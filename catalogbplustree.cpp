#include "catalogbplustree.h"
#include <fstream>
#include <algorithm>
#include <cstring>

// Implementación de CatalogNode
CatalogNode::CatalogNode(bool leaf) : isLeaf(leaf), next(nullptr) {}

// Implementación de CatalogBPlusTree
CatalogBPlusTree::CatalogBPlusTree(int grado) {
    root = new CatalogNode(true);
    t = grado;
}

TableMeta* CatalogBPlusTree::find(const std::string& tableName, CatalogNode* node) {
    if (!node) node = root;
    int i = 0;
    while (i < (int)node->keys.size() && tableName > node->keys[i]) i++;

    if (node->isLeaf) {
        if (i < (int)node->keys.size() && node->keys[i] == tableName) {
            return &node->records[i];
        }
        return nullptr;
    } else {
        return find(tableName, node->children[i]);
    }
}

void CatalogBPlusTree::insert(const TableMeta& tmeta) {
    CatalogNode* leaf = root;
    while (!leaf->isLeaf) {
        int i = 0;
        while (i < (int)leaf->keys.size() && std::string(tmeta.name) > leaf->keys[i]) i++;
        leaf = leaf->children[i];
    }

    int pos = (int)(std::lower_bound(leaf->keys.begin(),
                                      leaf->keys.end(),
                                      std::string(tmeta.name)) - leaf->keys.begin());

    // >>> ADD: evitar duplicados; si ya existe, solo actualiza el record
    if (pos < (int)leaf->keys.size() && leaf->keys[pos] == std::string(tmeta.name)) {
        leaf->records[pos] = tmeta;
        return;
    }

    leaf->keys.insert(leaf->keys.begin() + pos, tmeta.name);
    leaf->records.insert(leaf->records.begin() + pos, tmeta);

    if ((int)leaf->keys.size() >= 2 * t) {
        splitLeaf(leaf);
    }
}

bool CatalogBPlusTree::remove(const std::string& tableName) {
    CatalogNode* leaf = root;
    while (!leaf->isLeaf) {
        int i = 0;
        while (i < (int)leaf->keys.size() && tableName > leaf->keys[i]) i++;
        leaf = leaf->children[i];
    }
    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == tableName) {
            leaf->keys.erase(leaf->keys.begin() + i);
            leaf->records.erase(leaf->records.begin() + i);
            return true;
        }
    }
    return false;
}

void CatalogBPlusTree::listAll() {
    CatalogNode* leaf = root;
    while (!leaf->isLeaf) leaf = leaf->children[0];

    std::cout << "\n--- Tablas definidas en el proyecto ---\n";
    while (leaf) {
        for (auto& rec : leaf->records) {
            std::cout << "Tabla: " << rec.name
                      << " | Campos: " << rec.fieldCount
                      << " | Archivo: " << rec.dataFile << "\n";
        }
        leaf = leaf->next;
    }
}

void CatalogBPlusTree::saveToFile(const std::string& filepath) {
    // >>> ADD: usa trunc para sobrescribir (evita duplicados por append accidental)
    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "No se pudo abrir archivo " << filepath << " para escritura\n";
        return;
    }

    CatalogNode* leaf = root;
    while (!leaf->isLeaf) leaf = leaf->children[0];

    while (leaf) {
        for (auto& rec : leaf->records) {
            out.write(reinterpret_cast<const char*>(&rec), sizeof(TableMeta));
        }
        leaf = leaf->next;
    }
    out.close();
}

void CatalogBPlusTree::loadFromFile(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        // No imprimir error: si no existe todavía, es normal en el primer arranque
        return;
    }

    TableMeta rec;
    while (in.read(reinterpret_cast<char*>(&rec), sizeof(TableMeta))) {
        insert(rec);
    }
    in.close();
}

void CatalogBPlusTree::splitLeaf(CatalogNode* leaf) {
    int mid = (int)leaf->keys.size() / 2;
    CatalogNode* newLeaf = new CatalogNode(true);

    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->records.assign(leaf->records.begin() + mid, leaf->records.end());

    leaf->keys.resize(mid);
    leaf->records.resize(mid);

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    if (leaf == root) {
        CatalogNode* newRoot = new CatalogNode(false);
        newRoot->keys.push_back(newLeaf->keys[0]);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
    } else {
        insertIntoParent(leaf, newLeaf->keys[0], newLeaf);
    }
}

void CatalogBPlusTree::splitInternal(CatalogNode* node) {
    int mid = (int)node->keys.size() / 2;
    std::string promoteKey = node->keys[mid];

    CatalogNode* newNode = new CatalogNode(false);
    newNode->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newNode->children.assign(node->children.begin() + mid + 1, node->children.end());

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    if (node == root) {
        CatalogNode* newRoot = new CatalogNode(false);
        newRoot->keys.push_back(promoteKey);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newNode);
        root = newRoot;
    } else {
        insertIntoParent(node, promoteKey, newNode);
    }
}

CatalogNode* CatalogBPlusTree::findParent(CatalogNode* current, CatalogNode* child) {
    if (current->isLeaf) return nullptr;
    for (auto* c : current->children) {
        if (c == child) return current;
        if (!c->isLeaf) {
            CatalogNode* p = findParent(c, child);
            if (p) return p;
        }
    }
    return nullptr;
}

void CatalogBPlusTree::insertIntoParent(CatalogNode* leftChild,
                                        const std::string& key,
                                        CatalogNode* rightChild) {
    CatalogNode* parent = findParent(root, leftChild);
    if (!parent) return;

    int pos = 0;
    while (pos < (int)parent->keys.size() && key > parent->keys[pos]) pos++;

    parent->keys.insert(parent->keys.begin() + pos, key);
    parent->children.insert(parent->children.begin() + pos + 1, rightChild);

    if ((int)parent->keys.size() >= 2 * t) {
        splitInternal(parent);
    }
}

// Crea archivos vacíos <tabla>.meta y <tabla>.mat enbaseDir
bool CreateTableFiles(const std::string& baseDir, const std::string& tableName) {
    const std::string metaPath = baseDir + "/" + tableName + ".meta";
    const std::string matPath  = baseDir + "/" + tableName + ".mat";

    std::ofstream fmeta(metaPath, std::ios::binary | std::ios::trunc);
    if (!fmeta) { std::cerr << "No se pudo crear " << metaPath << "\n"; return false; }
    fmeta.close();

    std::ofstream fmat(matPath, std::ios::binary | std::ios::trunc);
    if (!fmat) { std::cerr << "No se pudo crear " << matPath << "\n"; return false; }
    fmat.close();

    return true;
}

// Inserta solo el nombre en el árbol y guarda el catálogo encatalogMetaPath
bool AddTableToCatalogAndSave(CatalogBPlusTree& catalog, const std::string& tableName,
                              const std::string& catalogMetaPath) {
    TableMeta tm{};
    // Copiar nombre de forma segura (rellena con '\0' si sobra espacio)
    std::memset(tm.name, 0, sizeof(tm.name));
    std::strncpy(tm.name, tableName.c_str(), sizeof(tm.name) - 1);

    tm.fieldCount = 0; // por ahora no lo usas

    // Por ahora no necesitas dataFile; si quieres puedes dejarlo vacío:
    std::memset(tm.dataFile, 0, sizeof(tm.dataFile));

    catalog.insert(tm);
    catalog.saveToFile(catalogMetaPath);
    return true;
}
