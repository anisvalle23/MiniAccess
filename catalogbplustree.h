#ifndef CATALOGBPLUSTREE_H
#define CATALOGBPLUSTREE_H

#pragma once
#include <iostream>
#include <vector>
#include <string>

// Estructura de cada tabla
struct TableMeta {
    char name[30];       // nombre de la tabla
    int  fieldCount;     // cantidad de campos
    char dataFile[50];   // archivo .mad/.mat asociado (por ahora lo puedes dejar vacío)
};

// Nodo del B+
class CatalogNode {
public:
    bool isLeaf;
    std::vector<std::string> keys;   // claves = nombre de tabla
    std::vector<CatalogNode*> children;
    std::vector<TableMeta> records;  // solo en hojas
    CatalogNode* next;

    CatalogNode(bool leaf);
};

// Árbol B+ para catálogo
class CatalogBPlusTree {
private:
    CatalogNode* root;
    int t; // grado mínimo

public:
    explicit CatalogBPlusTree(int grado);

    // --- Operaciones principales ---
    TableMeta* find(const std::string& tableName, CatalogNode* node = nullptr);
    void insert(const TableMeta& tmeta);
    bool remove(const std::string& tableName);
    void listAll();

    // --- Persistencia ---
    void saveToFile(const std::string& filepath);
    void loadFromFile(const std::string& filepath);

private:
    // --- Funciones internas ---
    void splitLeaf(CatalogNode* leaf);
    void splitInternal(CatalogNode* node);
    CatalogNode* findParent(CatalogNode* current, CatalogNode* child);
    void insertIntoParent(CatalogNode* leftChild, const std::string& key, CatalogNode* rightChild);
};

// >>> ADD: helpers muy básicos (no modifican la clase)
bool CreateTableFiles(const std::string& baseDir, const std::string& tableName);
bool AddTableToCatalogAndSave(CatalogBPlusTree& catalog, const std::string& tableName,
                              const std::string& catalogMetaPath);

#endif // CATALOGBPLUSTREE_H
