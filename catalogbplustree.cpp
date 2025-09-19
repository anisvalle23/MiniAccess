#include "catalogbplustree.h"
#include <fstream>
#include <algorithm>
#include <cstring>

#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QFileInfo>
#include <QHash>
#include <QDir>

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

std::vector<TableMeta> CatalogBPlusTree::getAllTables() const {
    std::vector<TableMeta> out;
    if (!root) return out;

    CatalogNode* leaf = root;
    while (leaf && !leaf->isLeaf) leaf = leaf->children[0];

    while (leaf) {
        for (const auto& rec : leaf->records) {
            out.push_back(rec);
        }
        leaf = leaf->next;
    }
    return out;
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

static bool writeStringToFile(const QString& path, const QString& content, QString* qerr=nullptr) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (qerr) *qerr = "No se pudo escribir: " + path;
        return false;
    }
    QTextStream ts(&f);
    ts << content;
    return true;
}

static bool appendLineToFile(const QString& path, const QString& line, QString* qerr=nullptr) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (qerr) *qerr = "No se pudo abrir para append: " + path;
        return false;
    }
    QTextStream ts(&f);
    ts << line << "\n";
    return true;
}

static QString tableMetaPathQS(const std::string& tablesDir, const std::string& tableName) {
    return QString::fromStdString(tablesDir) + "/" + QString::fromStdString(tableName) + ".meta";
}
static QString tableMadPathQS(const std::string& tablesDir, const std::string& tableName) {
    return QString::fromStdString(tablesDir) + "/" + QString::fromStdString(tableName) + ".mad";
}

// ===== Implementación de la API sencilla en el árbol =====
bool CatalogBPlusTree::createTableJson(const std::string& tablesDir,
                                       const std::string& catalogMetaPath,
                                       const std::string& tableName,
                                       const std::string& fieldsJsonPretty,
                                       std::string* err)
{
    // 1) Construir objeto JSON de meta (root con tableName, version, fields)
    QJsonObject root;
    root["tableName"] = QString::fromStdString(tableName);
    root["version"]   = 1;

    // fieldsJsonPretty debe ser al menos algo como: {"fields":[ ... ]}
    QJsonArray fieldsArray;

    // Intentar parsear como objeto completo
    QJsonParseError perr{};
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(fieldsJsonPretty), &perr);
    if (perr.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject in = doc.object();
        if (in.contains("fields") && in["fields"].isArray()) {
            fieldsArray = in["fields"].toArray();
        }
    }
    // Si no era objeto con "fields", intentar leer como ARRAY directamente
    if (fieldsArray.isEmpty()) {
        QJsonDocument docArr = QJsonDocument::fromJson(QByteArray::fromStdString(fieldsJsonPretty), &perr);
        if (perr.error == QJsonParseError::NoError && docArr.isArray()) {
            fieldsArray = docArr.array();
        }
    }

    // Si sigue vacío, hacer un esquema mínimo (id entero)
    if (fieldsArray.isEmpty()) {
        QJsonObject id;
        id["name"] = "id";
        id["type"] = "number";
        id["desc"] = "clave primaria";
        id["numberKind"] = "integer";
        id["allowNull"] = false;
        fieldsArray.push_back(id);
    }
    root["fields"] = fieldsArray;

    // 2) Escribir <tabla>.meta (JSON bonito)
    QJsonDocument outDoc(root);
    QString metaPath = tableMetaPathQS(tablesDir, tableName);
    QString qerr;
    if (!writeStringToFile(metaPath, QString::fromUtf8(outDoc.toJson(QJsonDocument::Indented)), &qerr)) {
        if (err) *err = qerr.toStdString();
        return false;
    }

    // 3) Crear <tabla>.mad vacío (NDJSON)
    QString madPath = tableMadPathQS(tablesDir, tableName);
    if (!writeStringToFile(madPath, QString(), &qerr)) {  // archivo vacío
        if (err) *err = qerr.toStdString();
        return false;
    }

    // 4) Insertar en árbol y persistir catalog.meta binario (como ya hacías)
    TableMeta tm{};
    std::memset(tm.name, 0, sizeof(tm.name));
    std::strncpy(tm.name, tableName.c_str(), sizeof(tm.name)-1);
    tm.fieldCount = fieldsArray.size();
    std::memset(tm.dataFile, 0, sizeof(tm.dataFile));
    // Ej: guarda el .mad asociado como texto dentro de dataFile (opcional)
    std::strncpy(tm.dataFile, (tableName + ".mad").c_str(), sizeof(tm.dataFile)-1);

    insert(tm);
    saveToFile(catalogMetaPath);

    return true;
}

bool CatalogBPlusTree::appendRecordJson(const std::string& tablesDir,
                                        const std::string& tableName,
                                        const std::string& recordJsonLine,
                                        std::string* err)
{
    // OPCIONAL: validar contra el .meta
    // Para mantenerlo simple, dejamos NDJSON “libre” (válido JSON objeto).
    QJsonParseError perr{};
    QJsonDocument d = QJsonDocument::fromJson(QByteArray::fromStdString(recordJsonLine), &perr);
    if (perr.error != QJsonParseError::NoError || !d.isObject()) {
        if (err) *err = "recordJsonLine debe ser un JSON objeto (ej: {\"id\":1,...})";
        return false;
    }

    QString madPath = tableMadPathQS(tablesDir, tableName);
    QString qerr;
    if (!appendLineToFile(madPath, QString::fromUtf8(QJsonDocument(d.object()).toJson(QJsonDocument::Compact)), &qerr)) {
        if (err) *err = qerr.toStdString();
        return false;
    }
    return true;
}

// Helpers ya usados antes…
static bool readJsonFile(const QString& path, QJsonObject* out, QString* qerr=nullptr) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (qerr) *qerr = "No se pudo abrir: " + path;
        return false;
    }
    auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        if (qerr) *qerr = "JSON inválido en: " + path;
        return false;
    }
    *out = doc.object();
    return true;
}

static bool writePrettyJson(const QString& path, const QJsonObject& obj, QString* qerr=nullptr) {
    QJsonDocument d(obj);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (qerr) *qerr = "No se pudo escribir: " + path;
        return false;
    }
    f.write(d.toJson(QJsonDocument::Indented));
    return true;
}

static QJsonArray parseFieldsArrayLoose(const std::string& s) {
    QJsonParseError perr{};
    QJsonDocument d = QJsonDocument::fromJson(QByteArray::fromStdString(s), &perr);
    if (perr.error==QJsonParseError::NoError) {
        if (d.isArray()) return d.array();
        if (d.isObject()) {
            auto o = d.object();
            if (o.contains("fields") && o["fields"].isArray())
                return o["fields"].toArray();
        }
    }
    return {}; // vacío
}

// textMax truncation (simple)
static QString truncateIfNeeded(const QString& s, int maxLen) {
    if (maxLen>0 && s.size()>maxLen) return s.left(maxLen);
    return s;
}

bool CatalogBPlusTree::updateTableJson(const std::string& tablesDir,
                                       const std::string& catalogMetaPath,
                                       const std::string& tableName,
                                       const std::string& newFieldsJsonPretty,
                                       bool migrateData,
                                       std::string* err)
{
    const QString metaPath = tableMetaPathQS(tablesDir, tableName);
    const QString madPath  = tableMadPathQS (tablesDir, tableName);

    // 1) Cargar meta actual (si no existe, lo tratamos como vacío con id)
    QJsonObject oldMeta;
    QString qerr;
    QJsonArray oldFields;
    if (QFile::exists(metaPath)) {
        if (!readJsonFile(metaPath, &oldMeta, &qerr)) { if(err)*err=qerr.toStdString(); return false; }
        oldFields = oldMeta["fields"].toArray();
    }

    // 2) Parsear el NUEVO fields[]
    QJsonArray newFields = parseFieldsArrayLoose(newFieldsJsonPretty);
    if (newFields.isEmpty()) {
        // si viene vacío, deja al menos id
        QJsonObject id; id["name"]="id"; id["type"]="number"; id["numberKind"]="integer"; id["allowNull"]=false; id["desc"]="PK";
        newFields = QJsonArray{ id };
    }

    // 3) Escribir NUEVO .meta (tableName + version + fields)
    QJsonObject newMeta;
    newMeta["tableName"] = QString::fromStdString(tableName);
    newMeta["version"]   = 1;
    newMeta["fields"]    = newFields;
    if (!writePrettyJson(metaPath, newMeta, &qerr)) { if(err)*err=qerr.toStdString(); return false; }

    // 4) Migrar datos del .mad si se pidió (proyectar/llenar nulos)
    if (migrateData && QFile::exists(madPath)) {
        QFile in(madPath);
        if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) { if(err)*err="No se pudo abrir mad para migrar"; return false; }
        const QString tmpPath = madPath + ".tmp";
        QFile out(tmpPath);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { if(err)*err="No se pudo crear tmp"; return false; }
        QTextStream tin(&in), tout(&out);

        // Para validar/truncar texto según textMax y formateos simples
        auto getFieldSpec = [&](const QString& fname)->QJsonObject {
            for (const auto& v : newFields) {
                const auto o = v.toObject();
                if (o["name"].toString().compare(fname, Qt::CaseInsensitive)==0) return o;
            }
            return {};
        };

        while (!tin.atEnd()) {
            const QByteArray line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            auto d = QJsonDocument::fromJson(line);
            if (!d.isObject()) continue;
            QJsonObject oldRow = d.object();
            QJsonObject newRow;

            for (const auto& fv : newFields) {
                const QJsonObject f = fv.toObject();
                const QString fname = f["name"].toString();
                const QString ftype = f["type"].toString(); // number|bool|text|currency|date|datetime
                const bool allowNull = f["allowNull"].toBool(true);

                // Copiar si existe
                QJsonValue v = oldRow.value(fname);
                if (v.isUndefined() || v.isNull()) {
                    // dejar null o default vacío
                    newRow.insert(fname, QJsonValue::Null);
                } else {
                    // coerciones MUY simples (para principiante)
                    if (ftype=="text") {
                        int maxLen = f["textMax"].toInt(0);
                        newRow.insert(fname, truncateIfNeeded(v.toString(), maxLen));
                    } else if (ftype=="number" || ftype=="currency") {
                        // intentar a double; si numberKind=integer, redondeamos
                        const QString numberKind = f["numberKind"].toString("decimal");
                        bool ok=false;
                        double x = v.toVariant().toDouble(&ok);
                        if (!ok) { newRow.insert(fname, allowNull ? QJsonValue::Null : 0); }
                        else {
                            if (ftype=="number" && numberKind=="integer")
                                newRow.insert(fname, static_cast<qint64>(std::llround(x)));
                            else
                                newRow.insert(fname, x);
                        }
                    } else if (ftype=="bool") {
                        newRow.insert(fname, v.toBool());
                    } else if (ftype=="date" || ftype=="datetime") {
                        // dejamos el string como viene; el formateo lo validas en UI
                        newRow.insert(fname, v.toString());
                    } else {
                        newRow.insert(fname, v); // desconocido: copiar tal cual
                    }
                }
            }

            QJsonDocument outDoc(newRow);
            tout << outDoc.toJson(QJsonDocument::Compact) << "\n";
        }
        in.close(); out.close();

        // Reemplazar
        QFile::remove(madPath);
        QFile::rename(tmpPath, madPath);
    }

    // 5) Actualizar el registro en el B+ (fieldCount y dataFile si quieres)
    if (auto* tmeta = find(tableName)) {
        tmeta->fieldCount = newFields.size();
        // tmeta->dataFile ya apunta a "<tabla>.mad" si lo habías guardado
        saveToFile(catalogMetaPath);
    } else {
        // si no estaba, inserta
        TableMeta tm{}; std::memset(tm.name,0,sizeof(tm.name));
        std::strncpy(tm.name, tableName.c_str(), sizeof(tm.name)-1);
        tm.fieldCount = newFields.size();
        std::memset(tm.dataFile,0,sizeof(tm.dataFile));
        std::strncpy(tm.dataFile, (tableName + ".mad").c_str(), sizeof(tm.dataFile)-1);
        insert(tm);
        saveToFile(catalogMetaPath);
    }

    return true;
}

std::vector<std::string> CatalogBPlusTree::readAllRecordsJson(const std::string& tablesDir,
                                                              const std::string& tableName,
                                                              std::string* err)
{
    std::vector<std::string> out;
    QString madPath = tableMadPathQS(tablesDir, tableName);
    QFile f(madPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (err) *err = std::string("No se pudo abrir: ") + madPath.toStdString();
        return out;
    }
    while (!f.atEnd()) {
        QByteArray line = f.readLine().trimmed();
        if (line.isEmpty()) continue;
        // validamos que sea JSON objeto
        QJsonParseError perr{};
        QJsonDocument d = QJsonDocument::fromJson(line, &perr);
        if (perr.error == QJsonParseError::NoError && d.isObject()) {
            out.push_back(QString::fromUtf8(d.toJson(QJsonDocument::Compact)).toStdString());
        }
        // si no es objeto, lo ignoramos (simple/robusto para principiante)
    }
    return out;
}

// === Helpers para guardar NDJSON según el meta ===
static bool loadMetaFields(const QString& metaPath, QJsonArray* outFields, QString* qerr=nullptr) {
    QJsonObject meta;
    if (!readJsonFile(metaPath, &meta, qerr)) return false;
    *outFields = meta.value("fields").toArray();
    return true;
}

static QJsonObject findFieldSpec(const QJsonArray& fields, const QString& fname) {
    for (const auto& v : fields) {
        const auto o = v.toObject();
        if (o.value("name").toString().compare(fname, Qt::CaseInsensitive) == 0) return o;
    }
    return {};
}

static QJsonValue toNumberJson(const QString& s, bool integer, bool allowNull) {
    const QString t = s.trimmed();
    if (t.isEmpty()) return allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(0);
    if (integer) {
        bool ok=false; qlonglong iv = t.toLongLong(&ok);
        return ok ? QJsonValue(iv) : (allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(0));
    } else {
        // admite coma decimal
        QString t = s.trimmed();
        QString norm = t;
        norm.replace(',', '.');
        bool ok = false;
        double dv = norm.toDouble(&ok);
        return ok ? QJsonValue(dv) : (allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(0.0));
    }

}

static QJsonValue toBoolJson(const QString& s, bool allowNull) {
    const QString t = s.trimmed().toLower();
    if (t.isEmpty()) return allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(false);
    if (t=="1" || t=="true" || t=="sí" || t=="si" || t=="yes")  return QJsonValue(true);
    if (t=="0" || t=="false"|| t=="no")                         return QJsonValue(false);
    return allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(false);
}

static QJsonObject uiRowToJsonObject(const QStringList& fieldNamesOrdered,
                                     const QStringList& uiRow,
                                     const QJsonArray&  fieldsMeta)
{
    QJsonObject obj;
    for (int i=0; i<fieldNamesOrdered.size(); ++i) {
        const QString fname = fieldNamesOrdered[i];
        const QString s     = (i<uiRow.size() ? uiRow[i] : QString());

        const QJsonObject spec = findFieldSpec(fieldsMeta, fname);
        const QString ftype    = spec.value("type").toString(); // number|currency|text|date|datetime|bool
        const QString nKind    = spec.value("numberKind").toString("decimal");
        const int textMax      = spec.value("textMax").toInt(0);
        const bool allowNull   = spec.value("allowNull").toBool(true);

        if (ftype == "text") {
            obj.insert(fname, truncateIfNeeded(s, textMax));
        } else if (ftype == "number") {
            const bool integer = (nKind.compare("integer", Qt::CaseInsensitive) == 0);
            obj.insert(fname, toNumberJson(s, integer, allowNull));
        } else if (ftype == "currency") {
            // se guarda como número decimal; la moneda (HNL/USD) queda en el meta
            obj.insert(fname, toNumberJson(s, /*integer=*/false, allowNull));
        } else if (ftype == "bool") {
            obj.insert(fname, toBoolJson(s, allowNull));
        } else if (ftype == "date" || ftype == "datetime") {
            // guardamos string; validación/format se hace en UI
            obj.insert(fname, s.trimmed().isEmpty() && allowNull ? QJsonValue(QJsonValue::Null) : QJsonValue(s));
        } else {
            // desconocido: guarda como string (con trunc si se definió textMax)
            obj.insert(fname, textMax>0 ? truncateIfNeeded(s, textMax) : s);
        }
    }
    return obj;
}

static bool writeNdjsonFileTruncate(const QString& path, const QVector<QJsonObject>& rows, QString* qerr=nullptr) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (qerr) *qerr = "No se pudo escribir: " + path;
        return false;
    }
    QTextStream ts(&f);
    for (const auto& o : rows) {
        ts << QJsonDocument(o).toJson(QJsonDocument::Compact) << "\n";
    }
    return true;
}

bool CatalogBPlusTree::rewriteMadFromRows(const std::string& tablesDir,
                                          const std::string& tableName,
                                          const QStringList& fieldNamesOrdered,
                                          const QList<QStringList>& rows,
                                          std::string* err)
{
    const QString metaPath = tableMetaPathQS(tablesDir, tableName);
    const QString madPath  = tableMadPathQS (tablesDir, tableName);

    QJsonArray fieldsMeta;
    QString qerr;
    if (!loadMetaFields(metaPath, &fieldsMeta, &qerr)) {
        if (err) *err = qerr.toStdString();
        return false;
    }

    QVector<QJsonObject> out;
    out.reserve(rows.size());
    for (const auto& r : rows) {
        out.push_back(uiRowToJsonObject(fieldNamesOrdered, r, fieldsMeta));
    }

    if (!writeNdjsonFileTruncate(madPath, out, &qerr)) {
        if (err) *err = qerr.toStdString();
        return false;
    }
    return true;
}

bool CatalogBPlusTree::appendMadFromRows(const std::string& tablesDir,
                                         const std::string& tableName,
                                         const QStringList& fieldNamesOrdered,
                                         const QList<QStringList>& rows,
                                         std::string* err)
{
    const QString metaPath = tableMetaPathQS(tablesDir, tableName);
    const QString madPath  = tableMadPathQS (tablesDir, tableName);

    QJsonArray fieldsMeta;
    QString qerr;
    if (!loadMetaFields(metaPath, &fieldsMeta, &qerr)) {
        if (err) *err = qerr.toStdString();
        return false;
    }

    QFile f(madPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (err) *err = std::string("No se pudo abrir para append: ") + madPath.toStdString();
        return false;
    }
    QTextStream ts(&f);

    for (const auto& r : rows) {
        const QJsonObject o = uiRowToJsonObject(fieldNamesOrdered, r, fieldsMeta);
        ts << QJsonDocument(o).toJson(QJsonDocument::Compact) << "\n";
    }
    return true;
}

static QString catalogAvlPathQS(const std::string& catalogMetaPath) {
    QFileInfo fi(QString::fromStdString(catalogMetaPath));
    return fi.dir().filePath(fi.baseName() + ".avl"); // p.ej. catalog.avl
}

// POP: toma el último índice libre del .avl (LIFO). Retorna true si obtuvo uno.
static bool popFreeIdx(const QString& avlPath, int* outIdx) {
    QFile f(avlPath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QList<QByteArray> lines;
    while (!f.atEnd()) lines.push_back(f.readLine());
    f.close();
    while (!lines.isEmpty() && lines.last().trimmed().isEmpty()) lines.removeLast();
    if (lines.isEmpty()) return false;

    bool ok=false;
    int idx = QString::fromUtf8(lines.last()).trimmed().toInt(&ok);
    if (!ok) return false;
    lines.removeLast();

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
    for (auto& l : lines) f.write(l);
    f.close();

    *outIdx = idx;
    return true;
}

// PUSH: agrega un índice libre al final del .avl
static void pushFreeIdx(const QString& avlPath, int idx) {
    QFile f(avlPath);
    f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream ts(&f);
    ts << idx << "\n";
}

// Devuelve: mapa nombre->índice y total de slots actuales
static bool scanCatalogMetaQuick(const std::string& catalogMetaPath,
                                 QHash<QString,int>* slotOfName,
                                 int* totalSlots) {
    slotOfName->clear();
    QFile f(QString::fromStdString(catalogMetaPath));
    if (!f.exists()) { *totalSlots = 0; return true; }
    if (!f.open(QIODevice::ReadOnly)) return false;

    const qint64 recSize = sizeof(TableMeta);
    const qint64 sz = f.size();
    *totalSlots = int(sz / recSize);

    TableMeta rec{};
    for (int i=0; i<*totalSlots; ++i) {
        if (f.read(reinterpret_cast<char*>(&rec), recSize) != recSize) break;
        if (rec.name[0] != '\0') {
            slotOfName->insert(QString::fromLatin1(rec.name), i);
        }
    }
    return true;
}

bool CatalogBPlusTree::upsertCatalogRecordShallow(const std::string& catalogMetaPath,
                                                  const TableMeta& tm,
                                                  std::string* err)
{
    const QString metaQS = QString::fromStdString(catalogMetaPath);
    const QString avlQS  = catalogAvlPathQS(catalogMetaPath);

    QFile f(metaQS);
    if (!f.open(QIODevice::ReadWrite)) {
        // crear si no existe
        if (!f.open(QIODevice::WriteOnly)) { if(err)*err=("No se pudo abrir/crear: "+metaQS).toStdString(); return false; }
        f.close();
        if (!f.open(QIODevice::ReadWrite)) { if(err)*err=("No se pudo reabrir RW: "+metaQS).toStdString(); return false; }
    }

    QHash<QString,int> slotOfName; int total=0;
    if (!scanCatalogMetaQuick(catalogMetaPath, &slotOfName, &total)) {
        if (err) *err = "No se pudo escanear catalog.meta";
        return false;
    }

    const QString key = QString::fromLatin1(tm.name);
    // Si ya existe -> update in-place
    if (slotOfName.contains(key)) {
        const int idx = slotOfName.value(key);
        const qint64 pos = qint64(idx) * sizeof(TableMeta);
        if (!f.seek(pos)) { if(err)*err="seek() falló"; return false; }
        if (f.write(reinterpret_cast<const char*>(&tm), sizeof(TableMeta)) != sizeof(TableMeta)) {
            if(err)*err="Escritura incompleta (update)"; return false;
        }
        return true;
    }

    // Si no existe -> intentar reusar un slot libre del .avl
    int reuseIdx = -1;
    if (!popFreeIdx(avlQS, &reuseIdx)) {
        reuseIdx = total; // append
    }

    const qint64 pos = qint64(reuseIdx) * sizeof(TableMeta);
    if (!f.seek(pos)) { if(err)*err="seek() falló"; return false; }
    if (f.write(reinterpret_cast<const char*>(&tm), sizeof(TableMeta)) != sizeof(TableMeta)) {
        if(err)*err="Escritura incompleta (insert)"; return false;
    }
    return true;
}

bool CatalogBPlusTree::deleteCatalogRecordByNameShallow(const std::string& catalogMetaPath,
                                                        const std::string& tableName,
                                                        std::string* err)
{
    const QString metaQS = QString::fromStdString(catalogMetaPath);
    const QString avlQS  = catalogAvlPathQS(catalogMetaPath);

    QFile f(metaQS);
    if (!f.open(QIODevice::ReadWrite)) { if(err)*err=("No se pudo abrir: "+metaQS).toStdString(); return false; }

    QHash<QString,int> slotOfName; int total=0;
    if (!scanCatalogMetaQuick(catalogMetaPath, &slotOfName, &total)) { if(err)*err="No se pudo escanear catalog.meta"; return false; }

    const QString key = QString::fromStdString(tableName);
    if (!slotOfName.contains(key)) return true; // idempotente

    const int idx = slotOfName.value(key);
    const qint64 pos = qint64(idx) * sizeof(TableMeta);

    TableMeta rec{};
    if (!f.seek(pos)) { if(err)*err="seek() falló"; return false; }
    if (f.read(reinterpret_cast<char*>(&rec), sizeof(TableMeta)) != sizeof(TableMeta)) { if(err)*err="Lectura incompleta"; return false; }

    rec.name[0] = '\0'; // marca vacío
    if (!f.seek(pos)) { if(err)*err="seek() falló"; return false; }
    if (f.write(reinterpret_cast<const char*>(&rec), sizeof(TableMeta)) != sizeof(TableMeta)) { if(err)*err="Escritura incompleta (delete)"; return false; }

    pushFreeIdx(avlQS, idx);
    return true;
}
