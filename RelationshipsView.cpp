#include "RelationshipsView.h"
#include "ThemeManager.h"
#include "TableEditor.h"
#include "mainwindow.h"
#include "catalogbplustree.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QTimer>
#include <cmath>
#include <QStyleFactory>
#include <QAbstractButton>
#include <QPalette>
#include <QDir>
#include <QFileInfoList>

static QStringList readFieldsFromMeta(const QString& tablesDir, const QString& tableName) {
    QStringList out;

    const QString metaPath = QDir(tablesDir).filePath(tableName + ".meta");
    QFile f(metaPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return out;

    const QJsonDocument d = QJsonDocument::fromJson(f.readAll());
    if (!d.isObject()) return out;
    const QJsonObject o = d.object();
    const QJsonArray fields = o.value("fields").toArray();

    // Leer relaciones para identificar foreign keys DEL PROYECTO ACTUAL
    QSet<QString> foreignKeyFields;
    const QString relationshipsPath = QDir(tablesDir).filePath("../relationships.json");
    QFile relFile(relationshipsPath);
    if (relFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QJsonDocument relDoc = QJsonDocument::fromJson(relFile.readAll());
        if (relDoc.isObject()) {
            const QJsonArray relationships = relDoc.object().value("relationships").toArray();
            for (const auto& rel : relationships) {
                const QJsonObject relObj = rel.toObject();
                const QString targetTable = relObj.value("targetTable").toString();
                const QString targetField = relObj.value("targetField").toString();
                if (targetTable == tableName && !targetField.isEmpty()) {
                    foreignKeyFields.insert(targetField);
                }
            }
        }
    }

    for (const auto& v : fields) {
        const QJsonObject fo = v.toObject();
        const QString fname  = fo.value("name").toString().trimmed();
        const bool isPk      = fo.value("isPrimaryKey").toBool(false);
        const bool isFk      = foreignKeyFields.contains(fname);
        
        if (!fname.isEmpty()) {
            QString displayName = fname;
            if (isPk && isFk) {
                displayName = QStringLiteral("🔑🔗 ") + fname;
            } else if (isPk) {
                displayName = QStringLiteral("🔑 ") + fname;
            } else if (isFk) {
                displayName = QStringLiteral("🔗 ") + fname;
            }
            out << displayName;
        }
    }
    return out;
}

// --- Helper: une, deduplica y ordena una lista de tablas ---
static QStringList dedupSorted(QStringList lst) {
    lst.removeDuplicates();
    std::sort(lst.begin(), lst.end(), [](const QString& a, const QString& b){
        return a.localeAwareCompare(b) < 0;
    });
    return lst;
}

RelationshipsView::RelationshipsView(QWidget *parent)
    : QWidget(parent), isDarkTheme(false), tableEditor(nullptr)
{
    setupUI();
    styleComponents();
    
    // Connect theme manager
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](ThemeManager::Theme theme) {
                updateTheme(theme == ThemeManager::Theme::Dark);
            });
    
    // NO cargar tablas aquí porque tableEditor aún es nullptr
    // loadTables(); // Se llamará en setTableEditor()
    loadRelationships();
    
    // *** NUEVO: Mostrar tablas automáticamente y cargar estado guardado del diseñador ***
    // Esto también se moverá a setTableEditor()
}

void RelationshipsView::setTableEditor(TableEditor *editor)
{
    qDebug() << "DEBUG[setTableEditor]: =============CONFIGURANDO TABLE EDITOR=============";
    tableEditor = editor;
    
    // Reload tables when editor is set

    if (tableEditor) {
        qDebug() << "DEBUG[setTableEditor]: TableEditor válido, configurando conexiones";
        // 🔹 Esto ya lo usas para refrescar campos cuando cambian
        connect(tableEditor, &TableEditor::tableFieldsChanged,
                this, &RelationshipsView::onTableFieldsChanged, Qt::UniqueConnection);

        // 🔹 Aquí añadís la conexión para rename
        connect(tableEditor, &TableEditor::tableRenamed,
                this, &RelationshipsView::applyTableRenameImmediate, Qt::UniqueConnection);
        
        // 🔹 Nueva conexión para manejar tabla eliminada
        connect(tableEditor, &TableEditor::tableDeleted,
                this, &RelationshipsView::onTableDeleted, Qt::UniqueConnection);
    } else {
        qDebug() << "DEBUG[setTableEditor]: TableEditor es null";
    }

    // Cargar tablas ahora que tenemos el editor configurado
    qDebug() << "DEBUG[setTableEditor]: Llamando loadTables()...";
    loadTables();
    
    // Mostrar tablas automáticamente y cargar estado guardado con más delay
    qDebug() << "DEBUG[setTableEditor]: Programando loadDesignerState()...";
    QTimer::singleShot(1000, this, [this]() { // Aumentado a 1 segundo
        // NO mostrar automáticamente en diseñador - las tablas deben aparecer solo en la lista lateral
        // qDebug() << "DEBUG[setTableEditor]: Ejecutando showAllTablesAndRelationships()...";
        // showAllTablesAndRelationships();
        qDebug() << "DEBUG[setTableEditor]: Ejecutando loadDesignerState()...";
        loadDesignerState();
    });
    qDebug() << "DEBUG[setTableEditor]: =============FIN CONFIGURAR TABLE EDITOR=============";
}

void RelationshipsView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    qDebug() << "DEBUG[showEvent]: RelationshipsView se volvió visible - refrescando tablas";
    // Cuando la vista se vuelve visible, intentar cargar las tablas nuevamente
    // Esto ayuda con problemas de timing
    if (tableEditor) {
        QTimer::singleShot(100, this, [this]() {
            qDebug() << "DEBUG[showEvent]: Ejecutando loadTables() después de 100ms";
            loadTables();
        });
    }
}

// En RelationshipsView.cpp
void RelationshipsView::applyTableRenameImmediate(const QString& oldName, const QString& newName)
{
    if (oldName == newName || oldName.isEmpty() || newName.isEmpty()) return;

    // 1) Cache internos
    int idxAvail = availableTables.indexOf(oldName);
    if (idxAvail >= 0) availableTables[idxAvail] = newName;

    if (tableFields.contains(oldName)) {
        tableFields[newName] = tableFields.take(oldName); // mueve la entrada
    }

    // 2) Listado de tablas (texto y UserRole)
    for (int i = 0; i < tablesListWidget->count(); ++i) {
        QListWidgetItem *it = tablesListWidget->item(i);
        if (!it) continue;
        if (it->data(Qt::UserRole).toString() == oldName) {
            it->setData(Qt::UserRole, newName);
            it->setText(newName);
        } else if (it->text() == oldName) {
            it->setText(newName);
        }
    }

    // 3) Combos (mantener selección si aplica)
    auto replaceCombo = [](QComboBox* c, const QString& oldT, const QString& newT){
        if (!c) return;
        int idx = c->findText(oldT);
        if (idx >= 0) c->setItemText(idx, newT);
        // reafirma selección si ya estaba en ese índice
        if (c->currentIndex() == idx) c->setCurrentIndex(idx);
    };
    replaceCombo(sourceTableCombo, oldName, newName);
    replaceCombo(targetTableCombo, oldName, newName);

    // 4) Items del diseñador (solo cambia el título visible)
    for (auto *item : tableItems) {
        if (item && item->getTableName() == oldName) {
            item->setTableName(newName); // actualiza nombre visual
            item->update();              // repinta
        }
    }

    // 5) Relaciones visuales: reposicionar por si cambió el ancho del texto
    for (auto *line : relationshipLines) {
        if (line) line->updatePosition();
    }

    // 6) Lista de relaciones: actualizar nombres de tablas en el texto (SIN ELIMINAR relaciones)
    qDebug() << "DEBUG: Actualizando" << relationshipsListWidget->count() << "relaciones para cambio de nombre:" << oldName << "->" << newName;
    
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *it = relationshipsListWidget->item(i);
        if (!it) continue;

        QString txt = it->text(); // "A → B (tipo)"
        QString originalText = txt;
        
        // Parsear la relación para actualizar los nombres de tabla de forma precisa
        int arrowPos = txt.indexOf(" → ");
        if (arrowPos >= 0) {
            QString leftPart = txt.left(arrowPos).trimmed();
            QString rightPart = txt.mid(arrowPos + 3).trimmed();
            
            // Extraer la tabla destino (antes del paréntesis)
            int parenPos = rightPart.lastIndexOf("(");
            QString targetTable = parenPos >= 0 ? rightPart.left(parenPos).trimmed() : rightPart;
            QString typeAndParen = parenPos >= 0 ? rightPart.mid(parenPos) : "";
            
            // Actualizar los nombres si coinciden
            bool updated = false;
            if (leftPart == oldName) {
                leftPart = newName;
                updated = true;
            }
            if (targetTable == oldName) {
                targetTable = newName;
                updated = true;
            }
            
            // Reconstruir el texto si se actualizó
            if (updated) {
                QString newText = QString("%1 → %2 %3").arg(leftPart, targetTable, typeAndParen);
                it->setText(newText);
                qDebug() << "DEBUG: Relación actualizada de:" << originalText << "a:" << newText;
            }
        }
    }

    // 7) NO eliminar relaciones - solo actualizar referencias visuales
    // (Comentado el código de eliminación para preservar las relaciones existentes)

    // 8) Si hay items en diseño, forzá repintado (suave y sin parpadeo)
    if (designerScene) designerScene->update();

    qDebug() << "DEBUG: applyTableRenameImmediate OK:" << oldName << "->" << newName;
}

void RelationshipsView::refreshAvailableTablesFromStorage()
{
    availableTables.clear();

    qDebug() << "DEBUG[RefreshTables]: =============INICIANDO CARGA DE TABLAS=============";
    qDebug() << "DEBUG[RefreshTables]: tableEditor =" << (tableEditor ? "válido" : "null");
    
    // USAR LA MISMA LÓGICA QUE TABLEEDITOR - SIMPLE Y DIRECTO
    if (tableEditor && tableEditor->mainWindow() && tableEditor->mainWindow()->catalog()) {
        qDebug() << "DEBUG[RefreshTables]: Usando catálogo B+ (igual que TableEditor)";
        CatalogBPlusTree* cat = tableEditor->mainWindow()->catalog();
        const std::vector<TableMeta> metas = cat->getAllTables();
        qDebug() << "DEBUG[RefreshTables]: Catálogo tiene" << metas.size() << "tablas";
        
        for (const auto& tm : metas) {
            QString name = QString::fromLatin1(tm.name).trimmed();
            if (!name.isEmpty()) {
                availableTables << name;
                qDebug() << "DEBUG[RefreshTables]: Agregada desde catálogo:" << name;
            }
        }
    } else {
        qDebug() << "DEBUG[RefreshTables]: No se puede acceder al catálogo";
        if (!tableEditor) qDebug() << "DEBUG[RefreshTables]: - tableEditor es null";
        else if (!tableEditor->mainWindow()) qDebug() << "DEBUG[RefreshTables]: - mainWindow es null";
        else if (!tableEditor->mainWindow()->catalog()) qDebug() << "DEBUG[RefreshTables]: - catalog es null";
    }

    // Normalizar/ordenar (igual que TableEditor)
    availableTables.removeDuplicates();
    std::sort(availableTables.begin(), availableTables.end(),
              [](const QString& a, const QString& b){ return a.localeAwareCompare(b) < 0; });

    qDebug() << "DEBUG[RelationshipsView]: ============RESULTADO FINAL============";
    qDebug() << "DEBUG[RelationshipsView]: tablas disponibles FINAL =" << availableTables;
    qDebug() << "DEBUG[RelationshipsView]: =====================================";
}

void RelationshipsView::forceRefreshTables()
{
    qDebug() << "DEBUG[forceRefreshTables]: Forzando refresh de tablas";
    loadTables();
}

void RelationshipsView::showAllTablesInDesigner()
{
    // NEW: refrescar desde storage (árbol o disco)
    refreshAvailableTablesFromStorage();

    // Mostrar todas las tablas disponibles en el diseñador visual
    int tableCount = availableTables.size();
    if (tableCount == 0) {
        QMessageBox::information(this, "📋 Sin Tablas",
                                 "No hay tablas creadas para mostrar.\n"
                                 "Cree tablas primero en la vista de diseño de tablas.");
        return;
    }

    // ... el resto de tu función igual ...
    int cols = static_cast<int>(std::ceil(std::sqrt(tableCount)));
    int spacing = 200;
    int startX = 50;
    int startY = 50;

    for (int i = 0; i < tableCount; ++i) {
        const QString &tableName = availableTables[i];

        bool alreadyExists = false;
        for (auto *item : tableItems) {
            if (item && item->getTableName() == tableName) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            int row = i / cols;
            int col = i % cols;
            QPointF position(startX + col * spacing, startY + row * spacing);
            addTableToDesigner(tableName, position);
        }
    }

    if (!tableItems.isEmpty()) {
        designerView->fitInView(designerScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }

    qDebug() << "DEBUG: Mostradas" << tableCount << "tablas en el diseñador visual";
}

void RelationshipsView::showAllTablesAndRelationships()
{
    // IMPORTANTE: refrescar desde storage primero
    refreshAvailableTablesFromStorage();
    
    // Mostrar TODAS las tablas disponibles en el diseñador visual
    int tableCount = availableTables.size();
    if (tableCount == 0) {
        QMessageBox::information(this, "📋 Sin Tablas", 
            "No hay tablas creadas para mostrar.\n"
            "Cree tablas primero en la vista de diseño de tablas.");
        return;
    }
    
    // Configuración para posicionamiento automático
    int cols = static_cast<int>(std::ceil(std::sqrt(tableCount))); // Número de columnas en grid
    int spacing = 200; // Espaciado entre tablas
    int startX = 50;
    int startY = 50;
    
    // Agregar TODAS las tablas que no estén ya en el diseñador
    for (int i = 0; i < tableCount; ++i) {
        const QString &tableName = availableTables[i];
        
        // Verificar si la tabla ya existe en el diseñador
        bool alreadyExists = false;
        for (auto *item : tableItems) {
            if (item && item->getTableName() == tableName) {
                alreadyExists = true;
                break;
            }
        }
        
        // Solo agregar si no existe
        if (!alreadyExists) {
            int row = i / cols;
            int col = i % cols;
            QPointF position(startX + col * spacing, startY + row * spacing);
            
            addTableToDesigner(tableName, position);
        }
    }
    
    // Mostrar TODAS las relaciones existentes entre las tablas
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (!item) continue;
        
        QString relationshipText = item->text();
        
        // Parsear la relación para extraer información
        int arrowPos = relationshipText.indexOf(" → ");
        if (arrowPos >= 0) {
            QString sourceTable = relationshipText.left(arrowPos).trimmed();
            QString rightPart = relationshipText.mid(arrowPos + 3).trimmed();
            
            int parenPos = rightPart.lastIndexOf("(");
            QString targetTable = parenPos >= 0 ? rightPart.left(parenPos).trimmed() : rightPart;
            QString type = parenPos >= 0 ? rightPart.mid(parenPos + 1).replace(")", "").trimmed() : "";
            
            // Crear relación visual si ambas tablas existen
            if (!sourceTable.isEmpty() && !targetTable.isEmpty() && !type.isEmpty()) {
                // Verificar que no exista ya esta relación visual
                bool relationExists = false;
                for (auto *line : relationshipLines) {
                    if (line && line->getSourceTable() && line->getTargetTable()) {
                        QString lineSource = line->getSourceTable()->getTableName();
                        QString lineTarget = line->getTargetTable()->getTableName();
                        QString lineType = line->getRelationshipType();
                        
                        if ((lineSource == sourceTable && lineTarget == targetTable && lineType == type) ||
                            (lineSource == targetTable && lineTarget == sourceTable && lineType == type)) {
                            relationExists = true;
                            break;
                        }
                    }
                }
                
                // Solo crear si no existe
                if (!relationExists) {
                    createRelationshipBetweenTables(sourceTable, targetTable, type);
                }
            }
        }
    }
    
    // NO hacer zoom ni escalado - mantener tamaño normal
    // Solo centrar la vista en las tablas
    if (!tableItems.isEmpty()) {
        designerView->ensureVisible(designerScene->itemsBoundingRect());
    }
    
    qDebug() << "DEBUG: Mostradas" << tableCount << "tablas con todas sus relaciones en el diseñador visual";
}

void RelationshipsView::saveDesignerState()
{
    qDebug() << "DEBUG[saveDesignerState]: ========== INICIANDO GUARDADO DE RELACIONES ==========";
    QString filePath = getProjectRelationshipsPath();
    qDebug() << "DEBUG[saveDesignerState]: Ruta obtenida:" << filePath;
    if (filePath.isEmpty()) {
        qDebug() << "ERROR[saveDesignerState]: Ruta de relaciones está vacía";
        return;
    }
    
    QJsonObject designerState;
    QJsonArray tablesArray;
    QJsonArray relationshipsArray;
    
    // Guardar posiciones de las tablas
    for (auto *item : tableItems) {
        if (!item) continue;
        
        QJsonObject tableObj;
        tableObj["name"] = item->getTableName();
        tableObj["x"] = item->pos().x();
        tableObj["y"] = item->pos().y();
        tablesArray.append(tableObj);
    }
    
    // Guardar relaciones creadas
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (!item) continue;
        
        QString relationshipText = item->text();
        QJsonObject relationshipObj;
        relationshipObj["description"] = relationshipText;
        
        // Parsear la relación para extraer información estructurada
        int arrowPos = relationshipText.indexOf(" → ");
        if (arrowPos >= 0) {
            QString sourceTable = relationshipText.left(arrowPos).trimmed();
            QString rightPart = relationshipText.mid(arrowPos + 3).trimmed();
            
            int parenPos = rightPart.lastIndexOf("(");
            QString targetTable = parenPos >= 0 ? rightPart.left(parenPos).trimmed() : rightPart;
            QString type = parenPos >= 0 ? rightPart.mid(parenPos + 1).replace(")", "").trimmed() : "";
            
            relationshipObj["sourceTable"] = sourceTable;
            relationshipObj["targetTable"] = targetTable;
            relationshipObj["type"] = type;
        }
        
        relationshipsArray.append(relationshipObj);
    }
    
    designerState["tables"] = tablesArray;
    designerState["relationships"] = relationshipsArray;
    designerState["version"] = "1.0";
    
    // Escribir archivo
    QJsonDocument doc(designerState);
    QFile file(filePath);
    
    qDebug() << "DEBUG[saveDesignerState]: Intentando abrir archivo:" << filePath;
    qDebug() << "DEBUG[saveDesignerState]: Directorio padre:" << QFileInfo(filePath).absolutePath();
    
    // Asegurar que el directorio existe
    QDir dir = QFileInfo(filePath).absoluteDir();
    if (!dir.exists()) {
        qDebug() << "DEBUG[saveDesignerState]: Creando directorio:" << dir.absolutePath();
        dir.mkpath(".");
    }
    
    if (file.open(QIODevice::WriteOnly)) {
        qint64 bytesWritten = file.write(doc.toJson());
        file.close();
        qDebug() << "DEBUG[saveDesignerState]: ✅ Estado guardado exitosamente en:" << filePath;
        qDebug() << "DEBUG[saveDesignerState]: Bytes escritos:" << bytesWritten;
        qDebug() << "DEBUG[saveDesignerState]: Relaciones guardadas:" << relationshipsListWidget->count();
    } else {
        qWarning() << "ERROR[saveDesignerState]: ❌ No se pudo abrir archivo para escritura:" << filePath;
        qWarning() << "ERROR[saveDesignerState]: Error:" << file.errorString();
    }
    qDebug() << "DEBUG[saveDesignerState]: ========== FIN GUARDADO DE RELACIONES ==========";
}

void RelationshipsView::loadDesignerState()
{
    QString filePath = getProjectRelationshipsPath();
    if (filePath.isEmpty() || !QFile::exists(filePath)) return;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ERROR: No se pudo leer el archivo de estado:" << filePath;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;
    
    QJsonObject designerState = doc.object();
    
    // Cargar posiciones de tablas
    QJsonArray tablesArray = designerState["tables"].toArray();
    QMap<QString, QPointF> tablePositions;
    
    for (const QJsonValue &value : tablesArray) {
        QJsonObject tableObj = value.toObject();
        QString tableName = tableObj["name"].toString();
        QPointF position(tableObj["x"].toDouble(), tableObj["y"].toDouble());
        tablePositions[tableName] = position;
    }
    
    // Aplicar posiciones a las tablas existentes en el diseñador
    for (auto *item : tableItems) {
        if (!item) continue;
        QString tableName = item->getTableName();
        if (tablePositions.contains(tableName)) {
            item->setPos(tablePositions[tableName]);
        }
    }
    
    // Cargar relaciones
    QJsonArray relationshipsArray = designerState["relationships"].toArray();
    
    // Limpiar lista de relaciones actual
    relationshipsListWidget->clear();
    
    // Limpiar líneas de relación visuales
    for (auto *line : relationshipLines) {
        if (line) {
            designerScene->removeItem(line);
            delete line;
        }
    }
    relationshipLines.clear();
    
    // Recrear relaciones
    for (const QJsonValue &value : relationshipsArray) {
        QJsonObject relationshipObj = value.toObject();
        QString description = relationshipObj["description"].toString();
        QString sourceTable = relationshipObj["sourceTable"].toString();
        QString targetTable = relationshipObj["targetTable"].toString();
        QString type = relationshipObj["type"].toString();
        
        // Agregar a la lista de relaciones
        relationshipsListWidget->addItem(description);
        
        // Crear línea visual si ambas tablas existen en el diseñador
        if (!sourceTable.isEmpty() && !targetTable.isEmpty() && !type.isEmpty()) {
            createRelationshipBetweenTables(sourceTable, targetTable, type);
        }
    }
    
    qDebug() << "DEBUG: Estado del diseñador cargado desde:" << filePath;
    qDebug() << "DEBUG: Cargadas" << tablesArray.size() << "posiciones de tablas y" << relationshipsArray.size() << "relaciones";
}

QString RelationshipsView::getProjectRelationshipsPath()
{
    qDebug() << "DEBUG[getProjectRelationshipsPath]: ========== OBTENIENDO RUTA ==========";
    // Usar el directorio de tablas del tableEditor activo para determinar el proyecto actual
    if (!tableEditor) {
        qDebug() << "WARNING[getProjectRelationshipsPath]: No hay tableEditor disponible";
        return QString();
    }
    
    QString tablesDir = QString::fromStdString(tableEditor->mainWindow()->tablesDir());
    qDebug() << "DEBUG[getProjectRelationshipsPath]: tablesDir obtenido:" << tablesDir;
    if (tablesDir.isEmpty()) {
        qDebug() << "WARNING[getProjectRelationshipsPath]: tablesDir está vacío";
        return QString();
    }
    
    // La ruta de relaciones está en el directorio padre del directorio de tablas
    QString relationshipsFile = QDir(tablesDir).filePath("../relationships.json");
    QString absolutePath = QDir(relationshipsFile).absolutePath() + "/relationships.json";
    
    qDebug() << "DEBUG[getProjectRelationshipsPath]: Ruta calculada:" << absolutePath;
    qDebug() << "DEBUG[getProjectRelationshipsPath]: ========== FIN OBTENER RUTA ==========";
    return absolutePath;
}

void RelationshipsView::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create toolbar
    createToolbar();
    
    // Create main area
    createMainArea();
}

void RelationshipsView::createToolbar()
{
    toolbarWidget = new QWidget();
    toolbarWidget->setFixedHeight(60);
    toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(20, 10, 20, 10);
    
    // Title
    titleLabel = new QLabel("🔗 Relaciones de Base de Datos");
    titleLabel->setFont(QFont("Inter", 20, QFont::Bold));
    titleLabel->setStyleSheet("color: #2C3E50; margin: 5px 0;");
    
    // Spacer
    toolbarLayout->addWidget(titleLabel);
    toolbarLayout->addStretch();
    
    // Buttons
    createRelationshipBtn = new QPushButton("✨ Nueva Relación");
    deleteRelationshipBtn = new QPushButton("🗑️ Eliminar");
    showAllTablesBtn = new QPushButton("📋 Mostrar Todas las Tablas");
    
    createRelationshipBtn->setFixedSize(160, 40);
    deleteRelationshipBtn->setFixedSize(120, 40);
    showAllTablesBtn->setFixedSize(180, 40);
    
    // Style buttons with better design
    QString primaryButtonStyle = 
        "QPushButton {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498DB, stop:1 #2980B9);"
            "color: white;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 10px 16px;"
            "font-weight: 600;"
            "font-size: 13px;"
        "}"
        "QPushButton:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5DADE2, stop:1 #3498DB);"
        "}"
        "QPushButton:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2980B9, stop:1 #1F618D);"
        "}";
    
    QString secondaryButtonStyle = 
        "QPushButton {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #95A5A6, stop:1 #7F8C8D);"
            "color: white;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 10px 16px;"
            "font-weight: 600;"
            "font-size: 13px;"
        "}"
        "QPushButton:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #BDC3C7, stop:1 #95A5A6);"
        "}";
    
    createRelationshipBtn->setStyleSheet(primaryButtonStyle);
    deleteRelationshipBtn->setStyleSheet(secondaryButtonStyle);
    showAllTablesBtn->setStyleSheet(primaryButtonStyle);
    
    toolbarLayout->addWidget(createRelationshipBtn);
    toolbarLayout->addWidget(showAllTablesBtn);
    toolbarLayout->addWidget(deleteRelationshipBtn);
    
    mainLayout->addWidget(toolbarWidget);
    
    // Connect signals
    connect(createRelationshipBtn, &QPushButton::clicked, this, &RelationshipsView::onNewRelationshipClicked);
    connect(showAllTablesBtn, &QPushButton::clicked, this, &RelationshipsView::showAllTablesAndRelationships);
    connect(deleteRelationshipBtn, &QPushButton::clicked, this, &RelationshipsView::onDeleteRelationship);
}

void RelationshipsView::createMainArea()
{
    // Main splitter (horizontal)
    mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left splitter (vertical) for tables and relationships lists
    leftSplitter = new QSplitter(Qt::Vertical);
    
    // Create panels
    createRelationshipsList();
    createRelationshipDesigner();
    createPropertiesPanel();
    
    // Add panels to splitters
    leftSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(designerPanel);
    mainSplitter->addWidget(propertiesPanel);
    
    // Set sizes
    mainSplitter->setSizes({250, 600, 300});
    
    mainLayout->addWidget(mainSplitter);
}

void RelationshipsView::createRelationshipsList()
{
    leftPanel = new QWidget();
    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(10, 10, 10, 10);
    
    // Tables section - SIMPLIFICADO
    tablesGroup = new QGroupBox("📋 Tablas");
    tablesGroup->setFont(QFont("Inter", 12, QFont::Bold));
    tablesLayout = new QVBoxLayout(tablesGroup);
    tablesLayout->setSpacing(5);
    
    tablesListWidget = new QListWidget();
    tablesListWidget->setDragDropMode(QAbstractItemView::DragOnly);
    tablesListWidget->setDefaultDropAction(Qt::CopyAction);
    
    // Habilitar drag iniciado por mouse
    connect(tablesListWidget, &QListWidget::itemPressed, [this](QListWidgetItem *item) {
        if (item && QApplication::mouseButtons() & Qt::LeftButton) {
            // Only allow drag if the item has UserRole data (real table, not info message)
            QString tableName = item->data(Qt::UserRole).toString();
            if (!tableName.isEmpty() && availableTables.contains(tableName)) {
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;
                mimeData->setText(tableName);
                drag->setMimeData(mimeData);
                drag->exec(Qt::CopyAction);
            }
        }
    });
    
    tablesListWidget->setMaximumHeight(120);
    tablesListWidget->setStyleSheet(
        "QListWidget {"
            "border: 1px solid #E8F4FD;"
            "border-radius: 6px;"
            "background: #F8FCFF;"
            "padding: 3px;"
        "}"
        "QListWidget::item {"
            "padding: 6px 10px;"
            "margin: 1px 0;"
            "border-radius: 4px;"
            "background: transparent;"
            "font-size: 12px;"
        "}"
        "QListWidget::item:hover {"
            "background: #E3F2FD;"
            "color: #1976D2;"
        "}"
        "QListWidget::item:selected {"
            "background: #2196F3;"
            "color: white;"
            "font-weight: 500;"
        "}"
    );
    tablesLayout->addWidget(tablesListWidget);
    
    // Relationships section - SIMPLIFICADO
    relationshipsGroup = new QGroupBox("🔗 Relaciones");
    relationshipsGroup->setFont(QFont("Inter", 12, QFont::Bold));
    relationshipsLayout = new QVBoxLayout(relationshipsGroup);
    relationshipsLayout->setSpacing(5);
    
    relationshipsListWidget = new QListWidget();
    relationshipsListWidget->setStyleSheet(
        "QListWidget {"
            "border: 1px solid #FFF3E0;"
            "border-radius: 6px;"
            "background: #FFFBF5;"
            "padding: 3px;"
        "}"
        "QListWidget::item {"
            "padding: 6px 10px;"
            "margin: 1px 0;"
            "border-radius: 4px;"
            "background: transparent;"
            "font-size: 11px;"
        "}"
        "QListWidget::item:hover {"
            "background: #FFF3E0;"
            "color: #F57C00;"
        "}"
        "QListWidget::item:selected {"
            "background: #FF9800;"
            "color: white;"
            "font-weight: 500;"
        "}"
    );
    relationshipsLayout->addWidget(relationshipsListWidget);
    
    leftPanelLayout->addWidget(tablesGroup);
    leftPanelLayout->addWidget(relationshipsGroup);
    
    // Connect signals
    connect(tablesListWidget, &QListWidget::itemSelectionChanged, 
            this, &RelationshipsView::onTableSelectionChanged);
    connect(relationshipsListWidget, &QListWidget::itemSelectionChanged,
            this, &RelationshipsView::onRelationshipSelectionChanged);
    connect(relationshipsListWidget, &QListWidget::itemDoubleClicked,
            this, &RelationshipsView::onRelationshipDoubleClicked);
}

void RelationshipsView::createRelationshipDesigner()
{
    designerPanel = new QWidget();
    designerLayout = new QVBoxLayout(designerPanel);
    designerLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title - MÁS SIMPLE
    QLabel *designerTitle = new QLabel("🎨 Diseñador Visual");
    designerTitle->setFont(QFont("Inter", 14, QFont::Bold));
    designerTitle->setStyleSheet("color: #2C3E50; margin: 5px 0;");
    designerLayout->addWidget(designerTitle);
    
    // Instrucciones SIMPLES y CLARAS
    QWidget *instructionCard = new QWidget();
    instructionCard->setStyleSheet(
        "QWidget {"
            "background: #FFEBEE;"
            "border: 1px solid #FFCDD2;"
            "border-radius: 6px;"
            "padding: 8px;"
        "}"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(instructionCard);
    cardLayout->setMargin(8);
    
    QLabel *instructionText = new QLabel("💡 Usa el botón 'Mostrar Todas las Tablas' para ver todas las tablas y sus relaciones. Crea nuevas relaciones en el panel derecho.");
    instructionText->setStyleSheet("color: #C62828; font-size: 11px; font-weight: 500;");
    instructionText->setWordWrap(true);
    
    cardLayout->addWidget(instructionText);
    designerLayout->addWidget(instructionCard);
    
    // Graphics view MEJORADO para drag & drop
    designerScene = new QGraphicsScene();
    designerView = new RelationshipDesignerView(designerScene, this);
    designerView->setDragMode(QGraphicsView::RubberBandDrag);
    designerView->setRenderHint(QPainter::Antialiasing);
    designerView->setAcceptDrops(true);
    designerView->setStyleSheet(
        "QGraphicsView {"
            "border: 2px dashed #CCCCCC;"
            "border-radius: 8px;"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FAFAFA, stop:1 #F0F0F0);"
        "}"
        "QGraphicsView:hover {"
            "border-color: #2196F3;"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F8FCFF, stop:1 #E3F2FD);"
        "}"
    );
    
    // Agregar texto de ayuda en el centro cuando esté vacío
    if (tableItems.isEmpty()) {
        QLabel *dropHintLabel = new QLabel("Arrastra tablas aquí para crear relaciones");
        dropHintLabel->setStyleSheet(
            "QLabel {"
                "color: #9E9E9E;"
                "font-size: 14px;"
                "font-style: italic;"
                "background: transparent;"
            "}"
        );
        dropHintLabel->setAlignment(Qt::AlignCenter);
        
        // Posicionar en el centro del view
        QGraphicsProxyWidget *proxyWidget = designerScene->addWidget(dropHintLabel);
        proxyWidget->setPos(200, 150);
    }
    
    designerLayout->addWidget(designerView);
}

void RelationshipsView::createPropertiesPanel()
{
    propertiesPanel = new QWidget();
    propertiesPanel->setFixedWidth(280);
    propertiesLayout = new QVBoxLayout(propertiesPanel);
    propertiesLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create a container for the title and info button
    QWidget *titleContainer = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(8);
    
    // Title label
    QLabel *titleLabel = new QLabel("⚙️ Nueva Relación");
    titleLabel->setFont(QFont("Inter", 12, QFont::Bold));
    titleLabel->setStyleSheet("color: #C62828; background: transparent;");
    
    // Info button - más visible y enfocado
    // Info button - más visible y enfocado con emoji
    QPushButton *infoButton = new QPushButton(QString::fromUtf8("ℹ️"));  // emoji info
    infoButton->setFixedSize(36, 36);

    // Aumentar un poco el tamaño de la fuente para que se vea centrado
    QFont f = infoButton->font();
    f.setPointSize(16);
    f.setBold(true);
    infoButton->setFont(f);

    infoButton->setStyleSheet(
        "QPushButton {"
        "background: #FF9800;"
        "color: white;"
        "border: 3px solid #F57C00;"
        "border-radius: 18px;"
        "font-size: 16px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background: #F57C00;"
        "border: 3px solid #E65100;"
        "}"
        "QPushButton:pressed {"
        "background: #E65100;"
        "}"
        );
    infoButton->setToolTip(QString::fromUtf8("💡 Guía de relaciones y validaciones"));

    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(infoButton);
    titleLayout->addStretch();
    
    propertiesGroup = new QGroupBox();
    propertiesGroup->setStyleSheet(
        "QGroupBox {"
            "font-weight: bold;"
            "border: 1px solid #FFEBEE;"
            "border-radius: 8px;"
            "margin-top: 12px;"
            "background: #FFFAFA;"
            "padding-top: 10px;"
        "}"
    );
    QVBoxLayout *groupLayout = new QVBoxLayout(propertiesGroup);
    groupLayout->setSpacing(12);
    
    // Add title container as first element
    groupLayout->addWidget(titleContainer);
    
    // SOLO LO ESENCIAL - Tipo de relación
    relationshipTypeLabel = new QLabel("Tipo:");
    relationshipTypeLabel->setFont(QFont("Inter", 11, QFont::Medium));
    relationshipTypeLabel->setStyleSheet("color: #2C3E50;");
    
    relationshipTypeCombo = new QComboBox();
    relationshipTypeCombo->addItems({
        "👫 Uno a Uno (1:1)",
        "👤➡️👥 Uno a Muchos (1:N)", 
        "👥↔️👥 Muchos a Muchos (N:M)"
    });
    relationshipTypeCombo->setStyleSheet(
        "QComboBox {"
            "border: 1px solid #E3F2FD;"
            "border-radius: 4px;"
            "padding: 6px 10px;"
            "background: white;"
            "font-size: 11px;"
        "}"
    );
    
    // Tablas origen y destino
    sourceTableLabel = new QLabel("De:");
    sourceTableLabel->setStyleSheet("color: #2C3E50; font-weight: 500;");
    sourceTableCombo = new QComboBox();
    
    targetTableLabel = new QLabel("A:");
    targetTableLabel->setStyleSheet("color: #2C3E50; font-weight: 500;");
    targetTableCombo = new QComboBox();
    
    // Campos de origen y destino
    sourceFieldLabel = new QLabel("Campo de:");
    sourceFieldLabel->setStyleSheet("color: #2C3E50; font-weight: 500;");
    sourceFieldCombo = new QComboBox();
    
    targetFieldLabel = new QLabel("A campo:");
    targetFieldLabel->setStyleSheet("color: #2C3E50; font-weight: 500;");
    targetFieldCombo = new QComboBox();
    
    QString comboStyle = 
        "QComboBox {"
            "border: 1px solid #E3F2FD;"
            "border-radius: 4px;"
            "padding: 6px 10px;"
            "background: white;"
            "font-size: 11px;"
        "}";
        
    sourceTableCombo->setStyleSheet(comboStyle);
    targetTableCombo->setStyleSheet(comboStyle);
    sourceFieldCombo->setStyleSheet(comboStyle);
    targetFieldCombo->setStyleSheet(comboStyle);
    
    // Botón para crear
    applyChangesBtn = new QPushButton("✅ Crear Relación");
    applyChangesBtn->setStyleSheet(
        "QPushButton {"
            "background: #4CAF50;"
            "color: white;"
            "border: none;"
            "border-radius: 6px;"
            "padding: 10px 16px;"
            "font-weight: 600;"
            "font-size: 12px;"
        "}"
        "QPushButton:hover {"
            "background: #45A049;"
        "}"
    );
    
    // Información simple - REMOVIDA DEL LAYOUT PRINCIPAL
    // Se mostrará en ventana emergente al hacer clic en el botón de información
    
    // Agregar al layout
    groupLayout->addWidget(relationshipTypeLabel);
    groupLayout->addWidget(relationshipTypeCombo);
    groupLayout->addSpacing(8);
    groupLayout->addWidget(sourceTableLabel);
    groupLayout->addWidget(sourceTableCombo);
    groupLayout->addSpacing(4);
    groupLayout->addWidget(sourceFieldLabel);
    groupLayout->addWidget(sourceFieldCombo);
    groupLayout->addSpacing(8);
    groupLayout->addWidget(targetTableLabel);
    groupLayout->addWidget(targetTableCombo);
    groupLayout->addSpacing(4);
    groupLayout->addWidget(targetFieldLabel);
    groupLayout->addWidget(targetFieldCombo);
    groupLayout->addSpacing(12);
    groupLayout->addWidget(applyChangesBtn);
    groupLayout->addStretch();
    
    propertiesLayout->addWidget(propertiesGroup);
    
    // Connect info button
    connect(infoButton, &QPushButton::clicked, this, &RelationshipsView::onInfoButtonClicked);
    
    // Connect table combo changes to update field combos - SIMPLIFICADO
    connect(sourceTableCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &RelationshipsView::updateSourceFields);
    
    connect(targetTableCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &RelationshipsView::updateTargetFields);
    
    // Connect apply button to create relationship function
    connect(applyChangesBtn, &QPushButton::clicked, this, &RelationshipsView::onCreateRelationship);
}

void RelationshipsView::styleComponents()
{
    updateTheme(ThemeManager::instance().isDark());
}

void RelationshipsView::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Main colors based on theme
    QString backgroundColor = isDark ? "#2B2B2B" : "#FFFFFF";
    QString textColor = isDark ? "#FFFFFF" : "#000000";
    QString borderColor = isDark ? "#404040" : "#E0E0E0";
    
    // Main widget
    setStyleSheet(QString(
        "RelationshipsView {"
            "background-color: %1;"
            "color: %2;"
        "}"
    ).arg(backgroundColor, textColor));
    
    // Toolbar - keep the existing styled buttons
    toolbarWidget->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
            "border-bottom: 1px solid %3;"
        "}"
    ).arg(backgroundColor, textColor, borderColor));
    
    // Update title color for theme
    if (isDark) {
        titleLabel->setStyleSheet("color: #FFFFFF; margin: 5px 0;");
    } else {
        titleLabel->setStyleSheet("color: #2C3E50; margin: 5px 0;");
    }
    
    // Update table items theme
    for (auto *item : tableItems) {
        item->updateTheme(isDark);
    }
    
    // Update relationship lines theme
    for (auto *line : relationshipLines) {
        line->updateTheme(isDark);
    }
    
    // Update designer view for theme
    if (isDark) {
        designerView->setStyleSheet(
            "QGraphicsView {"
                "border: 2px solid #404040;"
                "border-radius: 10px;"
                "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #353535, stop:1 #2B2B2B);"
            "}"
        );
    } else {
        designerView->setStyleSheet(
            "QGraphicsView {"
                "border: 2px solid #E0E0E0;"
                "border-radius: 10px;"
                "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FAFAFA, stop:1 #F5F5F5);"
            "}"
        );
    }
}

void RelationshipsView::loadTables()
{
    qDebug() << "DEBUG[loadTables]: ============ INICIANDO loadTables() ============";
    qDebug() << "DEBUG[loadTables]: tableEditor =" << (tableEditor ? "válido" : "null");
    
    availableTables.clear();
    tableFields.clear();
    tablesListWidget->clear();
    sourceTableCombo->clear();
    targetTableCombo->clear();
    sourceFieldCombo->clear();
    targetFieldCombo->clear();

    qDebug() << "DEBUG[loadTables]: Widgets limpiados, llamando refreshAvailableTablesFromStorage()";

    // USAR LA MISMA LÓGICA QUE TABLEEDITOR - OBTENER DIRECTAMENTE DEL CATÁLOGO
    refreshAvailableTablesFromStorage();
    
    qDebug() << "DEBUG[loadTables]: Después de refreshAvailableTablesFromStorage():";
    qDebug() << "DEBUG[loadTables]: availableTables.size() =" << availableTables.size();
    qDebug() << "DEBUG[loadTables]: availableTables contenido =" << availableTables;

    // Si no hay tablas, mostrar mensaje
    if (availableTables.isEmpty()) {
        qDebug() << "DEBUG[loadTables]: NO HAY TABLAS - Mostrando mensaje 'No hay tablas creadas'";
        QListWidgetItem *item = new QListWidgetItem("📝 No hay tablas creadas");
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(QColor("#999999"));
        tablesListWidget->addItem(item);
        return;
    }

    qDebug() << "DEBUG[loadTables]: HAY TABLAS DISPONIBLES - Poblando lista lateral con:" << availableTables;

    // Poblar UI (lista y combos) y cargar campos
    QString tablesDir;
    if (tableEditor && tableEditor->mainWindow()) {
        tablesDir = QString::fromStdString(tableEditor->mainWindow()->tablesDir());
    }
    
    for (const QString& tableName : availableTables) {
        qDebug() << "DEBUG[loadTables]: Procesando tabla:" << tableName;
        
        // Agregar a lista lateral
        auto *item = new QListWidgetItem(tableName);
        item->setData(Qt::UserRole, tableName);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
        tablesListWidget->addItem(item);
        qDebug() << "DEBUG[loadTables]: Tabla agregada a tablesListWidget:" << tableName;

        // Agregar a combos
        sourceTableCombo->addItem(tableName);
        targetTableCombo->addItem(tableName);

        // Cargar campos (intentar del editor primero, luego del .meta)
        QStringList fieldsWithKeys;
        if (tableEditor) {
            fieldsWithKeys = tableEditor->getTableFieldsWithKeys(tableName);
        }

        if (fieldsWithKeys.isEmpty() && !tablesDir.isEmpty()) {
            // Leer del .meta si no está disponible en el editor
            fieldsWithKeys = readFieldsFromMeta(tablesDir, tableName);
        }

        // Limpiar campos vacíos
        QStringList validFields;
        for (QString f : fieldsWithKeys) {
            f = f.trimmed();
            if (!f.isEmpty()) validFields << f;
        }

        tableFields[tableName] = validFields;
        qDebug() << "DEBUG[loadTables]: Campos cargados para" << tableName << ":" << validFields;
    }
    
    qDebug() << "DEBUG[loadTables]: loadTables() completado exitosamente";
}

void RelationshipsView::loadRelationships()
{
    // NO limpiar la lista de relaciones existentes
    // Las relaciones se mantienen y solo se actualizan cuando sea necesario
    // relationshipsListWidget->clear(); // COMENTADO para preservar relaciones
    
    // Las relaciones se mostrarán solo cuando se vayan creando
    // No hay relaciones predeterminadas, todo será dinámico
}

void RelationshipsView::updateSourceFields(const QString &tableName)
{
    sourceFieldCombo->clear();
    
    if (tableName.isEmpty() || !tableFields.contains(tableName)) {
        return;
    }
    
    // *** PROTECCIÓN: Solo actualizar si tableEditor es válido ***
    if (!tableEditor) {
        qDebug() << "WARNING: tableEditor es null en updateSourceFields";
        return;
    }
    
    // Solo mostrar campos que sean Primary Key o Foreign Key
    try {
        QStringList primaryKeys = tableEditor->getTablePrimaryKeys(tableName);
        QStringList foreignKeys = tableEditor->getTableForeignKeys(tableName);
        
        // Agregar Primary Keys
        for (const QString &field : primaryKeys) {
            QString displayText = "🔑 " + field + " (PK)";
            sourceFieldCombo->addItem(displayText, field);
        }
        
        // Agregar Foreign Keys (que no sean también PK para evitar duplicados)
        for (const QString &field : foreignKeys) {
            if (!primaryKeys.contains(field)) {
                QString displayText = "🔗 " + field + " (FK)";
                sourceFieldCombo->addItem(displayText, field);
            }
        }
        
        // Agregar campos que sean tanto PK como FK
        QStringList pkAndFkFields = tableEditor->getTablePrimaryAndForeignKeys(tableName);
        for (const QString &field : pkAndFkFields) {
            QString displayText = "🔑🔗 " + field + " (PK+FK)";
            sourceFieldCombo->addItem(displayText, field);
        }
    } catch (...) {
        qDebug() << "ERROR: Excepción al actualizar campos source para tabla:" << tableName;
        // Continuar sin fallar
    }
    
    // Seleccionar el primer campo por defecto
    if (sourceFieldCombo->count() > 0) {
        sourceFieldCombo->setCurrentIndex(0);
    }
}

void RelationshipsView::updateTargetFields(const QString &tableName)
{
    targetFieldCombo->clear();
    
    if (tableName.isEmpty() || !tableFields.contains(tableName)) {
        return;
    }
    
    // *** PROTECCIÓN: Solo actualizar si tableEditor es válido ***
    if (!tableEditor) {
        qDebug() << "WARNING: tableEditor es null en updateTargetFields";
        return;
    }
    
    // Solo mostrar campos que sean Primary Key o Foreign Key
    try {
        QStringList primaryKeys = tableEditor->getTablePrimaryKeys(tableName);
        QStringList foreignKeys = tableEditor->getTableForeignKeys(tableName);
        
        // Agregar Primary Keys
        for (const QString &field : primaryKeys) {
            QString displayText = "🔑 " + field + " (PK)";
            targetFieldCombo->addItem(displayText, field);
        }
        
        // Agregar Foreign Keys (que no sean también PK para evitar duplicados)
        for (const QString &field : foreignKeys) {
            if (!primaryKeys.contains(field)) {
                QString displayText = "🔗 " + field + " (FK)";
                targetFieldCombo->addItem(displayText, field);
            }
        }
        
        // Agregar campos que sean tanto PK como FK
        QStringList pkAndFkFields = tableEditor->getTablePrimaryAndForeignKeys(tableName);
        for (const QString &field : pkAndFkFields) {
            QString displayText = "🔑🔗 " + field + " (PK+FK)";
            targetFieldCombo->addItem(displayText, field);
        }
    } catch (...) {
        qDebug() << "ERROR: Excepción al actualizar campos target para tabla:" << tableName;
        // Continuar sin fallar
    }
    
    // Seleccionar el primer campo por defecto
    if (targetFieldCombo->count() > 0) {
        targetFieldCombo->setCurrentIndex(0);
    }
}

void RelationshipsView::addTableToDesigner(const QString &tableName, const QPointF &position)
{
    // Verificar si la tabla ya existe en el diseñador
    for (auto *item : tableItems) {
        if (item->getTableName() == tableName) {
            // Ya existe, solo moverla
            item->setPos(position);
            return;
        }
    }
    
    // Remove hint text if this is the first table
    if (tableItems.isEmpty()) {
        QList<QGraphicsItem*> items = designerScene->items();
        for (QGraphicsItem *item : items) {
            QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget*>(item);
            if (proxy) {
                designerScene->removeItem(proxy);
                delete proxy;
                break;
            }
        }
    }
    
    // Crear nuevo item de tabla con tamaño ajustado para mostrar campos
    QRectF rect(position.x(), position.y(), 180, 120);
    TableGraphicsItem *tableItem = new TableGraphicsItem(tableName, rect);
    
    if (tableFields.contains(tableName)) {
        QStringList fields = tableFields[tableName];
        if (!fields.isEmpty() && tableEditor) {
            QStringList pkFields = tableEditor->getTablePrimaryKeys(tableName);
            QStringList fkFields = tableEditor->getTableForeignKeys(tableName);
            tableItem->setFieldsWithKeys(fields, pkFields, fkFields);
        } else if (!fields.isEmpty()) {
            tableItem->setFields(fields);
        }
    }
    
    tableItem->updateTheme(isDarkTheme);
    
    // Connect close button signal
    connect(tableItem, &TableGraphicsItem::closeRequested,
            this, &RelationshipsView::onTableCloseRequested);
    
    designerScene->addItem(tableItem);
    tableItems.append(tableItem);
}

void RelationshipsView::addTableToDesigner(const QString &tableName, const QPoint &position)
{
    // Check if table already exists in designer
    QList<QGraphicsItem*> items = designerScene->items();
    for (QGraphicsItem *item : items) {
        TableGraphicsItem *tableItem = dynamic_cast<TableGraphicsItem*>(item);
        if (tableItem && tableItem->getTableName() == tableName) {
            // Table already exists, just move it
            QPointF scenePos = designerView->mapToScene(position);
            tableItem->setPos(scenePos);
            return;
        }
    }
    
    // Create new table item
    TableGraphicsItem *newTableItem = new TableGraphicsItem(tableName);
    QPointF scenePos = designerView->mapToScene(position);
    newTableItem->setPos(scenePos);
    
    // Set fields for the new table item
    if (tableFields.contains(tableName)) {
        QStringList fields = tableFields[tableName];
        if (!fields.isEmpty() && tableEditor) {
            QStringList pkFields = tableEditor->getTablePrimaryKeys(tableName);
            QStringList fkFields = tableEditor->getTableForeignKeys(tableName);
            newTableItem->setFieldsWithKeys(fields, pkFields, fkFields);
        } else if (!fields.isEmpty()) {
            newTableItem->setFields(fields);
        }
    }
    
    newTableItem->updateTheme(isDarkTheme);
    
    // Connect close button signal
    connect(newTableItem, &TableGraphicsItem::closeRequested,
            this, &RelationshipsView::onTableCloseRequested);
    
    designerScene->addItem(newTableItem);
    tableItems.append(newTableItem);
    
    // Update combo boxes to reflect available tables in designer
    if (sourceTableCombo->findText(tableName) == -1) {
        sourceTableCombo->addItem(tableName);
    }
    if (targetTableCombo->findText(tableName) == -1) {
        targetTableCombo->addItem(tableName);
    }
}

void RelationshipsView::createRelationshipBetweenTables(const QString &table1, const QString &table2, 
                                                       const QString &relationship_type)
{
    TableGraphicsItem *source = nullptr;
    TableGraphicsItem *target = nullptr;
    
    // Find table items
    for (auto *item : tableItems) {
        if (item->getTableName() == table1) {
            source = item;
        }
        if (item->getTableName() == table2) {
            target = item;
        }
    }
    
    if (source && target) {
        RelationshipLine *relationshipLine = new RelationshipLine(source, target, relationship_type);
        relationshipLine->updateTheme(isDarkTheme);
        
        // Connect double click signal to delete relationship
        connect(relationshipLine, &RelationshipLine::doubleClicked,
                this, &RelationshipsView::onRelationshipLineDoubleClicked);
        
        designerScene->addItem(relationshipLine);
        relationshipLines.append(relationshipLine);
    }
}

void RelationshipsView::clearDesignerArea()
{
    // Limpiar todas las tablas del área de diseño
    for (auto *item : tableItems) {
        designerScene->removeItem(item);
        delete item;
    }
    tableItems.clear();
    
    // Limpiar todas las líneas de relación
    for (auto *line : relationshipLines) {
        designerScene->removeItem(line);
        delete line;
    }
    relationshipLines.clear();
    
    qDebug() << "DEBUG: Área de diseño limpiada";
}

void RelationshipsView::refreshTableList()
{
    // Store current selections to restore them if possible
    QString currentSourceTable = sourceTableCombo->currentText();
    QString currentTargetTable = targetTableCombo->currentText();
    QString currentSourceField = sourceFieldCombo->currentData().toString();
    QString currentTargetField = targetFieldCombo->currentData().toString();
    
    // Reload tables from TableEditor
    loadTables();
    loadRelationships();
    
    // Restore selections if the tables still exist
    int sourceIndex = sourceTableCombo->findText(currentSourceTable);
    if (sourceIndex >= 0) {
        sourceTableCombo->setCurrentIndex(sourceIndex);
        // Update fields for restored table selection
        updateSourceFields(currentSourceTable);
        // Restore field selection if it still exists
        int fieldIndex = sourceFieldCombo->findData(currentSourceField);
        if (fieldIndex >= 0) {
            sourceFieldCombo->setCurrentIndex(fieldIndex);
        }
    }
    
    int targetIndex = targetTableCombo->findText(currentTargetTable);
    if (targetIndex >= 0) {
        targetTableCombo->setCurrentIndex(targetIndex);
        // Update fields for restored table selection
        updateTargetFields(currentTargetTable);
        // Restore field selection if it still exists
        int fieldIndex = targetFieldCombo->findData(currentTargetField);
        if (fieldIndex >= 0) {
            targetFieldCombo->setCurrentIndex(fieldIndex);
        }
    }
    
    // Update the visual designer by clearing items that no longer exist
    QList<TableGraphicsItem*> itemsToRemove;
    for (auto *item : tableItems) {
        QString tableName = item->getTableName();
        if (!availableTables.contains(tableName)) {
            itemsToRemove.append(item);
        }
    }
    
    // Remove invalid table items
    for (auto *item : itemsToRemove) {
        designerScene->removeItem(item);
        tableItems.removeAll(item);
        delete item;
    }

    // NO eliminar líneas de relación automáticamente durante refresh
    // (Las relaciones se actualizan por applyTableRenameImmediate cuando hay rename)
    // Las líneas de relación solo se eliminan explícitamente por el usuario    // Update existing table items with new field information
    for (auto *item : tableItems) {
        QString tableName = item->getTableName();
        if (tableFields.contains(tableName)) {
            QStringList fields = tableFields[tableName];
            // Force update fields even if previously empty
            if (tableEditor) {
                QStringList pkFields = tableEditor->getTablePrimaryKeys(tableName);
                QStringList fkFields = tableEditor->getTableForeignKeys(tableName);
                item->setFieldsWithKeys(fields, pkFields, fkFields);
            } else {
                item->setFields(fields);
            }
        } else if (tableEditor) {
            // Try to get fields directly from tableEditor if not in our cache
            QStringList fields = tableEditor->getTableFields(tableName);
            QStringList validFields;
            for (const QString &field : fields) {
                QString cleanField = field.trimmed();
                if (!cleanField.isEmpty()) {
                    validFields << cleanField;
                }
            }
            if (!validFields.isEmpty()) {
                tableFields[tableName] = validFields;
                QStringList pkFields = tableEditor->getTablePrimaryKeys(tableName);
                QStringList fkFields = tableEditor->getTableForeignKeys(tableName);
                item->setFieldsWithKeys(validFields, pkFields, fkFields);
            }
        }
    }
}

void RelationshipsView::onNewRelationshipClicked()
{
    // *** LIMPIAR TODOS LOS CAMPOS ***
    
    // Limpiar selecciones de los combos
    sourceTableCombo->setCurrentIndex(-1);
    targetTableCombo->setCurrentIndex(-1);
    sourceFieldCombo->clear();
    targetFieldCombo->clear();
    
    // Restablecer tipo de relación al primero
    relationshipTypeCombo->setCurrentIndex(0);
    
    // Limpiar cualquier selección en las listas
    tablesListWidget->clearSelection();
    relationshipsListWidget->clearSelection();
    
    // *** LIMPIAR COMPLETAMENTE EL DISEÑADOR VISUAL ***
    
    // Eliminar todas las tablas del diseñador
    for (auto *item : tableItems) {
        if (item) {
            designerScene->removeItem(item);
            delete item;
        }
    }
    tableItems.clear();
    
    // Eliminar todas las líneas de relación
    for (auto *line : relationshipLines) {
        if (line) {
            designerScene->removeItem(line);
            delete line;
        }
    }
    relationshipLines.clear();
    
    // Limpiar la escena completamente
    designerScene->clear();
    
    qDebug() << "DEBUG: Diseñador visual limpiado completamente para nueva relación";
}

void RelationshipsView::onCreateRelationship()
{
    qDebug() << "DEBUG[onCreateRelationship]: ======= INICIANDO CREACIÓN DE RELACIÓN =======";
    QString sourceTable = sourceTableCombo->currentText();
    QString targetTable = targetTableCombo->currentText();
    QString relationshipType = relationshipTypeCombo->currentText();
    
    qDebug() << "DEBUG[onCreateRelationship]: Tabla origen:" << sourceTable;
    qDebug() << "DEBUG[onCreateRelationship]: Tabla destino:" << targetTable;
    qDebug() << "DEBUG[onCreateRelationship]: Tipo:" << relationshipType;
    
    // Extract just the type part (1:1, 1:N, N:M)
    QString shortType;
    if (relationshipType.contains("1:1")) {
        shortType = "1:1";
    } else if (relationshipType.contains("1:N")) {
        shortType = "1:N";
    } else if (relationshipType.contains("N:M")) {
        shortType = "N:M";
    }

    if (sourceTable.isEmpty() || targetTable.isEmpty()) {
        QMessageBox m(this);
        m.setIcon(QMessageBox::Warning);
        m.setWindowTitle("Error");
        m.setText("Debe seleccionar ambas tablas");
        m.setStandardButtons(QMessageBox::Ok);

        // Forzar estilo no nativo
        m.setStyle(QStyleFactory::create("Fusion"));

        // Paleta blanca para que no herede gris del sistema
        QPalette pal = m.palette();
        pal.setColor(QPalette::Window, Qt::white);
        pal.setColor(QPalette::Base, Qt::white);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::WindowText, Qt::black);
        pal.setColor(QPalette::Button, QColor("#F3F4F6"));
        pal.setColor(QPalette::ButtonText, Qt::black);
        m.setPalette(pal);

        // Texto negro en labels del cuadro
        m.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QMessageBox QLabel { color: #111827; font-size: 14px; }"
            );

        // **Clave**: estilizar el botón directamente para ganar a tu stylesheet global
        if (QAbstractButton* ok = m.button(QMessageBox::Ok)) {
            ok->setStyleSheet(
                "color: #111827;"
                "background: #F3F4F6;"
                "border: 1px solid #D1D5DB;"
                "border-radius: 6px;"
                "padding: 6px 12px;"
                );
            // Por si el estilo global insiste:
            QPalette bp = ok->palette();
            bp.setColor(QPalette::ButtonText, Qt::black);
            bp.setColor(QPalette::WindowText, Qt::black);
            ok->setPalette(bp);
            ok->setAutoFillBackground(true);
        }

        m.exec();
        return;
    }
    
    if (sourceTable == targetTable) {
        QMessageBox m(this);
        m.setIcon(QMessageBox::Warning);
        m.setWindowTitle("Error");
        m.setText("No puede crear una relación de una tabla consigo misma");
        m.setStandardButtons(QMessageBox::Ok);

        m.setStyle(QStyleFactory::create("Fusion"));

        QPalette pal = m.palette();
        pal.setColor(QPalette::Window, Qt::white);
        pal.setColor(QPalette::Base, Qt::white);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::WindowText, Qt::black);
        pal.setColor(QPalette::Button, QColor("#F3F4F6"));
        pal.setColor(QPalette::ButtonText, Qt::black);
        m.setPalette(pal);

        m.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QMessageBox QLabel { color: #111827; font-size: 14px; }"
            );

        if (QAbstractButton* ok = m.button(QMessageBox::Ok)) {
            ok->setStyleSheet(
                "color: #111827;"
                "background: #F3F4F6;"
                "border: 1px solid #D1D5DB;"
                "border-radius: 6px;"
                "padding: 6px 12px;"
                );
            QPalette bp = ok->palette();
            bp.setColor(QPalette::ButtonText, Qt::black);
            bp.setColor(QPalette::WindowText, Qt::black);
            ok->setPalette(bp);
            ok->setAutoFillBackground(true);
        }

        m.exec();
        return;
    }
    if (sourceTable == targetTable) {
        QMessageBox m(this);
        m.setIcon(QMessageBox::Warning);
        m.setWindowTitle("Error");
        m.setText("No puede crear una relación de una tabla consigo misma");
        m.setStandardButtons(QMessageBox::Ok);

        m.setStyle(QStyleFactory::create("Fusion"));

        QPalette pal = m.palette();
        pal.setColor(QPalette::Window, Qt::white);
        pal.setColor(QPalette::Base, Qt::white);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::WindowText, Qt::black);
        pal.setColor(QPalette::Button, QColor("#F3F4F6"));
        pal.setColor(QPalette::ButtonText, Qt::black);
        m.setPalette(pal);

        m.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QMessageBox QLabel { color: #111827; font-size: 14px; }"
            );

        if (QAbstractButton* ok = m.button(QMessageBox::Ok)) {
            ok->setStyleSheet(
                "color: #111827;"
                "background: #F3F4F6;"
                "border: 1px solid #D1D5DB;"
                "border-radius: 6px;"
                "padding: 6px 12px;"
                );
            QPalette bp = ok->palette();
            bp.setColor(QPalette::ButtonText, Qt::black);
            bp.setColor(QPalette::WindowText, Qt::black);
            ok->setPalette(bp);
            ok->setAutoFillBackground(true);
        }

        m.exec();
        return;
    }
    
    // *** VALIDACIÓN DE RELACIONES DUPLICADAS ***
    // Verificar si ya existe una relación entre estas dos tablas
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        QString existingRelationText = item->text();
        
        // Verificar ambas direcciones de la relación
        bool isDuplicate = false;
        
        // Dirección 1: sourceTable → targetTable
        if (existingRelationText.contains(sourceTable) && existingRelationText.contains(targetTable)) {
            // Extraer las partes de la relación existente para verificar que sea la misma dirección
            QStringList parts = existingRelationText.split(" → ");
            if (parts.size() == 2) {
                QString existingSource = parts[0].trimmed();
                QString existingTargetPart = parts[1].trimmed();
                QString existingTarget = existingTargetPart.split(" (")[0].trimmed(); // Remover el tipo (1:1), (1:N), etc.
                
                if ((existingSource == sourceTable && existingTarget == targetTable) ||
                    (existingSource == targetTable && existingTarget == sourceTable)) {
                    isDuplicate = true;
                }
            }
        }
        
        if (isDuplicate) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setWindowTitle("⚠️ Relación Duplicada");
            msgBox.setText("<h3>Relación Ya Existente</h3>");
            msgBox.setInformativeText(
                QString("Ya existe una relación entre las tablas <b>'%1'</b> y <b>'%2'</b>.<br><br>"
                       "📋 <b>Relación existente:</b><br>"
                       "• %3<br><br>"
                       "💡 <b>Nota:</b> Solo se permite una relación entre cada par de tablas.<br>"
                       "Si desea cambiar el tipo de relación, primero elimine la relación existente.")
                       .arg(sourceTable, targetTable, existingRelationText)
            );
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->setText("Entendido");
            msgBox.setStyleSheet(
                "QMessageBox { background-color: white; min-width: 450px; min-height: 250px; }"
                "QMessageBox QLabel { color: black; font-size: 14px; }"
                "QPushButton { background-color: #FF9800; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                "QPushButton:hover { background-color: #F57C00; }"
            );
            msgBox.exec();
            return;
        }
    }
    
    // *** VALIDACIONES ESPECÍFICAS POR TIPO DE RELACIÓN ***
    if (tableEditor) {
        // Obtener Foreign Keys de ambas tablas
        QStringList sourceForeignKeys = tableEditor->getTableForeignKeys(sourceTable);
        QStringList targetForeignKeys = tableEditor->getTableForeignKeys(targetTable);
        
        // Obtener Primary Keys de ambas tablas (para validar referencias)
        QStringList sourcePrimaryKeys = tableEditor->getTablePrimaryKeys(sourceTable);
        QStringList targetPrimaryKeys = tableEditor->getTablePrimaryKeys(targetTable);
        
        // *** NUEVO: Obtener campos que sean tanto PK como FK ***
        QStringList sourcePKandFK = tableEditor->getTablePrimaryAndForeignKeys(sourceTable);
        QStringList targetPKandFK = tableEditor->getTablePrimaryAndForeignKeys(targetTable);
        
        // VALIDACIÓN SEGÚN TIPO DE RELACIÓN
        if (shortType == "1:1") {
            // Relación 1:1: Debe ser de Primary Key a Primary Key
            
            // Obtener los campos seleccionados de los combos
            QString selectedSourceField = sourceFieldCombo->currentText();
            QString selectedTargetField = targetFieldCombo->currentText();
            
            // Verificar que no sea la misma tabla
            if (sourceTable == targetTable) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación 1:1 - Tablas Diferentes Requeridas");
                msgBox.setText("<h3>No se puede crear relación con la misma tabla</h3>");
                msgBox.setInformativeText(
                    QString("Las relaciones 1:1 deben conectar tablas diferentes.<br><br>"
                           "⚠️ <b>Problema:</b><br>"
                           "Está intentando crear una relación entre la tabla '%1' y ella misma.<br><br>"
                           "<b>Solución:</b><br>"
                           "Seleccione una tabla diferente para el campo 'A'.")
                           .arg(sourceTable)
                );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 400px; min-height: 250px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #2196F3; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #1976D2; }"
                );
                msgBox.exec();
                return;
            }
            
            // Verificar que ambos campos sean Primary Keys (simplificado)
            // Si el campo contiene 🔑 y (PK), entonces es Primary Key
            bool sourceFieldIsPK = selectedSourceField.contains("🔑") && selectedSourceField.contains("(PK)");
            bool targetFieldIsPK = selectedTargetField.contains("🔑") && selectedTargetField.contains("(PK)");
            
            qDebug() << "DEBUG 1:1 Validation (Simplified):";
            qDebug() << "  - Source field:" << selectedSourceField << "-> isPK:" << sourceFieldIsPK;
            qDebug() << "  - Target field:" << selectedTargetField << "-> isPK:" << targetFieldIsPK;
            
            if (!sourceFieldIsPK || !targetFieldIsPK) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación 1:1 - Primary Keys Requeridas");
                msgBox.setText("<h3>Primary Keys Requeridas para Relación 1:1</h3>");
                msgBox.setInformativeText(
                    QString("Para establecer una relación 1:1, ambos campos deben ser Primary Keys de sus respectivas tablas.<br><br>"
                           "⚠️ <b>Estado actual:</b><br>"
                           "• Campo <b>'%1'</b> (en tabla '%2'): %3<br>"
                           "• Campo <b>'%4'</b> (en tabla '%5'): %6<br><br>"
                           "🔑 <b>Regla para relaciones 1:1:</b><br>"
                           "Las relaciones uno a uno deben conectar las Primary Keys de ambas tablas, "
                           "garantizando que cada registro se relacione únicamente con otro.<br><br>"
                           "<b>Solución:</b><br>"
                           "1. Seleccione el Primary Key de la tabla '%7'<br>"
                           "2. Seleccione el Primary Key de la tabla '%8'<br>"
                           "3. Intente crear la relación nuevamente")
                           .arg(selectedSourceField, sourceTable, sourceFieldIsPK ? "✅ Primary Key" : "❌ No es Primary Key")
                           .arg(selectedTargetField, targetTable, targetFieldIsPK ? "✅ Primary Key" : "❌ No es Primary Key")
                           .arg(sourceTable, targetTable)
                );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 500px; min-height: 300px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #2196F3; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #1976D2; }"
                );
                msgBox.exec();
                return;
            }
            
            // Si llegamos aquí, ambos campos son Primary Keys - relación válida
            qDebug() << "DEBUG: Relación 1:1 válida - Conectando PK a PK:" 
                     << "Tabla" << sourceTable << "Campo:" << selectedSourceField
                     << "Tabla" << targetTable << "Campo:" << selectedTargetField;
            
        } else if (shortType == "1:N") {
            // Relación 1:N: FK no puede ser PK en el lado muchos (target)
            if (targetForeignKeys.isEmpty()) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación 1:N - Foreign Key Requerida");
                msgBox.setText("<h3>Foreign Key Requerida para Relación 1:N</h3>");
                msgBox.setInformativeText(
                    QString("Para establecer una relación 1:N, la tabla del lado muchos debe contener una Foreign Key que apunte a la Primary Key de la tabla del lado uno.<br><br>"
                           "⚠️ <b>Problema:</b><br>"
                           "• Tabla <b>'%1'</b> (lado UNO): ✅ Origen válido<br>"
                           "• Tabla <b>'%2'</b> (lado MUCHOS): ❌ No tiene Foreign Keys<br><br>"
                           "<b>Solución:</b><br>"
                           "1. Vaya a la vista de diseño de la tabla '%3'<br>"
                           "2. Seleccione el campo que referenciará a '%4'<br>"
                           "3. Marque la casilla 'Foreign Key' en las propiedades<br>"
                           "4. Regrese e intente crear la relación nuevamente")
                           .arg(sourceTable, targetTable, targetTable, sourceTable)
                );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 520px; min-height: 300px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #FF9800; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #F57C00; }"
                );
                msgBox.exec();
                return;
            }
            
            // *** NUEVA VALIDACIÓN: FK no puede ser PK en el lado muchos ***
            if (!targetPKandFK.isEmpty()) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("⚠️ Relación 1:N - Violación de Integridad");
                msgBox.setText("<h3>Error: Foreign Key no puede ser Primary Key en el lado muchos</h3>");
                msgBox.setInformativeText(
                    QString("En una relación 1:N, el Foreign Key en la tabla del lado muchos NO puede ser también Primary Key.<br><br>"
                           "🚫 <b>Campo(s) problemático(s) en tabla '%1':</b><br>"
                           "• %2<br><br>"
                           "📘 <b>Explicación:</b><br>"
                           "Si el FK fuera PK en el lado muchos, solo podría haber un registro por cada valor de FK, "
                           "convirtiendo efectivamente la relación en 1:1 en lugar de 1:N.<br><br>"
                           "<b>Soluciones:</b><br>"
                           "1. <b>Remover Primary Key</b> del campo '%3' en tabla '%4'<br>"
                           "2. <b>Crear Primary Key diferente</b> (ej: campo 'Id' autoincremental)<br>"
                           "3. <b>Cambiar a relación 1:1</b> si esa es la intención")
                           .arg(targetTable)
                           .arg(targetPKandFK.join(", "))
                           .arg(targetPKandFK.first())
                           .arg(targetTable)
                );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 580px; min-height: 380px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #E53E3E; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #C53030; }"
                );
                msgBox.exec();
                return;
            }
            
        } else if (shortType == "N:M") {
            // Relación N:M: Se requiere una tabla intermedia con dos Foreign Keys específicos
            QStringList allTables = tableEditor->getCreatedTables();
            bool foundIntermediateTable = false;
            QString intermediateTableName;
            
            // Buscar una tabla intermedia que tenga FK a ambas tablas específicas
            for (const QString &tableName : allTables) {
                if (tableName == sourceTable || tableName == targetTable) continue;
                
                QStringList tableFKs = tableEditor->getTableForeignKeys(tableName);
                
                // Verificar si tiene FK que refieran a nuestras tablas específicas
                bool hasSourceFK = false;
                bool hasTargetFK = false;
                
                for (const QString &fk : tableFKs) {
                    QString fkLower = fk.toLower();
                    QString sourceLower = sourceTable.toLower();
                    QString targetLower = targetTable.toLower();
                    
                    // Limpiar el FK de emojis y etiquetas
                    QString cleanFK = fk;
                    cleanFK = cleanFK.remove(QRegExp("^[🔑🔗]\\s*")).remove(QRegExp("\\s*\\(PK\\)$")).remove(QRegExp("\\s*\\(FK\\)$")).trimmed();
                    
                    // Verificar si el FK podría referenciar a sourceTable (VALIDACIÓN SEMÁNTICA ESTRICTA)
                    if (validateForeignKeyNaming(cleanFK, sourceTable, false)) {
                        hasSourceFK = true;
                        qDebug() << "DEBUG N:M: ✅ FK" << cleanFK << "en tabla intermedia" << tableName << "SÍ referencia semánticamente a" << sourceTable;
                    }
                    
                    // Verificar si el FK podría referenciar a targetTable (VALIDACIÓN SEMÁNTICA ESTRICTA)
                    if (validateForeignKeyNaming(cleanFK, targetTable, false)) {
                        hasTargetFK = true;
                        qDebug() << "DEBUG N:M: ✅ FK" << cleanFK << "en tabla intermedia" << tableName << "SÍ referencia semánticamente a" << targetTable;
                    }
                }
                
                // Si encontramos una tabla con FKs a ambas tablas objetivo
                if (hasSourceFK && hasTargetFK && tableFKs.size() >= 2) {
                    foundIntermediateTable = true;
                    intermediateTableName = tableName;
                    break;
                }
            }
            
            if (!foundIntermediateTable) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación N:M - Tabla Intermedia Requerida");
                msgBox.setText("<h3>Tabla Intermedia Específica Requerida para Relación N:M</h3>");
                msgBox.setInformativeText(
                    QString("Para establecer una relación N:M entre <b>'%1'</b> y <b>'%2'</b>, se requiere una tabla intermedia con Foreign Keys específicos que apunten a ambas tablas.<br><br>"
                           "⚠️ <b>Problema:</b><br>"
                           "• Tabla origen: <b>'%3'</b><br>"
                           "• Tabla destino: <b>'%4'</b><br>"
                           "• <b>Tabla intermedia específica</b>: ❌ No encontrada<br><br>"
                           "✅ <b>Requisitos para la tabla intermedia:</b><br>"
                           "• Debe tener al menos 2 Foreign Keys<br>"
                           "• Un FK debe referenciar a '%5' (ej: %6_id, id_%7)<br>"
                           "• Otro FK debe referenciar a '%8' (ej: %9_id, id_%10)<br><br>"
                           "<b>Solución:</b><br>"
                           "1. Cree una nueva tabla intermedia (ej: '%11_%12' o '%13_%14')<br>"
                           "2. Agregue un campo Foreign Key que referencie a '%15'<br>"
                           "3. Agregue otro campo Foreign Key que referencie a '%16'<br>"
                           "4. Marque ambos campos como 'Foreign Key' en las propiedades<br>"
                           "5. Regrese e intente crear la relación nuevamente")
                           .arg(sourceTable, targetTable, sourceTable, targetTable, sourceTable, sourceTable, sourceTable, 
                                targetTable, targetTable, targetTable, sourceTable, targetTable, targetTable, sourceTable,
                                sourceTable, targetTable)
                );
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 550px; min-height: 350px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #9C27B0; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #7B1FA2; }"
                );
                msgBox.exec();
                return;
            } else {
                qDebug() << "DEBUG: Tabla intermedia encontrada:" << intermediateTableName << "con Foreign Keys válidas";
                
                // *** NUEVA VALIDACIÓN para N:M: FKs pueden ser PK compuesta en la tabla intermedia ***
                QStringList intermediatePKandFK = tableEditor->getTablePrimaryAndForeignKeys(intermediateTableName);
                QStringList intermediateFKs = tableEditor->getTableForeignKeys(intermediateTableName);
                QStringList intermediatePKs = tableEditor->getTablePrimaryKeys(intermediateTableName);
                
                // Verificar que los Foreign Keys en la tabla intermedia puedan formar una Primary Key compuesta válida
                bool validNMConfiguration = true;
                QString validationMessage = "";
                
                if (intermediateFKs.size() >= 2) {
                    // Caso ideal: Los FKs pueden ser parte de una PK compuesta
                    qDebug() << "DEBUG: Tabla intermedia" << intermediateTableName << "tiene" << intermediateFKs.size() << "Foreign Keys";
                    qDebug() << "DEBUG: Primary Keys en tabla intermedia:" << intermediatePKs;
                    qDebug() << "DEBUG: Campos PK+FK en tabla intermedia:" << intermediatePKandFK;
                    
                    // Verificar que al menos algunos de los FKs sean también PKs (para Primary Key compuesta)
                    if (intermediatePKs.size() >= 2) {
                        // Verificar que los PKs incluyan los FKs necesarios
                        int fksAsPks = 0;
                        for (const QString &fk : intermediateFKs) {
                            if (intermediatePKs.contains(fk)) {
                                fksAsPks++;
                            }
                        }
                        
                        if (fksAsPks >= 2) {
                            qDebug() << "DEBUG: ✅ Configuración N:M válida - Primary Key compuesta formada por Foreign Keys";
                        } else {
                            validationMessage = QString("La tabla intermedia '%1' debe tener los Foreign Keys como parte de una Primary Key compuesta para garantizar la unicidad en relaciones N:M.").arg(intermediateTableName);
                        }
                    } else if (intermediatePKs.size() == 1 && !intermediatePKandFK.isEmpty()) {
                        qDebug() << "DEBUG: ✅ Configuración N:M válida - Primary Key simple que también es Foreign Key";
                    } else {
                        validationMessage = QString("La tabla intermedia '%1' necesita una Primary Key compuesta formada por los Foreign Keys para garantizar la integridad de la relación N:M.").arg(intermediateTableName);
                    }
                }
                
                // Mostrar advertencia informativa si la configuración no es ideal (pero permitir continuar)
                if (!validationMessage.isEmpty()) {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Information);
                    msgBox.setWindowTitle("💡 Relación N:M - Recomendación de Diseño");
                    msgBox.setText("<h3>Configuración de Tabla Intermedia</h3>");
                    msgBox.setInformativeText(
                        QString("La tabla intermedia '%1' fue encontrada y tiene Foreign Keys suficientes.<br><br>"
                               "💡 <b>Recomendación:</b><br>"
                               "%2<br><br>"
                               "✅ <b>Configuración actual encontrada:</b><br>"
                               "• Tabla intermedia: <b>%3</b><br>"
                               "• Foreign Keys: %4<br>"
                               "• Primary Keys: %5<br><br>"
                               "📘 <b>Mejores prácticas para N:M:</b><br>"
                               "• Los Foreign Keys deberían formar una Primary Key compuesta<br>"
                               "• Esto garantiza que no haya relaciones duplicadas<br><br>"
                               "<b>¿Desea continuar creando la relación?</b>")
                               .arg(intermediateTableName)
                               .arg(validationMessage)
                               .arg(intermediateTableName)
                               .arg(intermediateFKs.join(", "))
                               .arg(intermediatePKs.join(", "))
                    );
                    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    msgBox.button(QMessageBox::Yes)->setText("Sí, continuar");
                    msgBox.button(QMessageBox::No)->setText("No, revisar diseño");
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    msgBox.setStyleSheet(
                        "QMessageBox { background-color: white; min-width: 600px; min-height: 400px; }"
                        "QMessageBox QLabel { color: black; font-size: 14px; }"
                        "QPushButton { font-size: 14px; font-weight: bold; min-width: 120px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                        "QPushButton:default { background-color: #10B981; color: white; }"
                        "QPushButton:default:hover { background-color: #059669; }"
                        "QPushButton:!default { background-color: #6B7280; color: white; }"
                        "QPushButton:!default:hover { background-color: #4B5563; }"
                    );
                    
                    int result = msgBox.exec();
                    if (result == QMessageBox::No) {
                        return; // Usuario decidió no continuar
                    }
                }
            }
        }
        
        // *** NUEVA VALIDACIÓN: TIPOS DE DATOS COMPATIBLES ***
        // Validar que las Primary Keys y Foreign Keys tengan tipos de datos compatibles
        if (!validateDataTypeCompatibility(sourceTable, targetTable, shortType)) {
            // La función validateDataTypeCompatibility mostrará el mensaje de error apropiado
            return;
        }
        
        // *** NUEVA VALIDACIÓN: NOMENCLATURA DE FOREIGN KEYS ***
        // Validar que los Foreign Keys tengan nombres similares a las tablas que referencian
        QString selectedSourceField = sourceFieldCombo->currentText();
        QString selectedTargetField = targetFieldCombo->currentText();
        
        // Limpiar los iconos de los nombres de campos para obtener solo el nombre
        selectedSourceField = selectedSourceField.remove(QRegExp("^[🔑🔗]\\s*")).remove(QRegExp("\\s*\\(PK\\)$")).remove(QRegExp("\\s*\\(FK\\)$")).trimmed();
        selectedTargetField = selectedTargetField.remove(QRegExp("^[🔑🔗]\\s*")).remove(QRegExp("\\s*\\(PK\\)$")).remove(QRegExp("\\s*\\(FK\\)$")).trimmed();
        
        // Debug: mostrar los nombres limpiados
        qDebug() << "DEBUG FK Validation - Source field cleaned:" << selectedSourceField;
        qDebug() << "DEBUG FK Validation - Target field cleaned:" << selectedTargetField;
        qDebug() << "DEBUG FK Validation - Source table:" << sourceTable;
        qDebug() << "DEBUG FK Validation - Target table:" << targetTable;
        
        // *** VALIDACIÓN ESTRICTA DE NOMENCLATURA SEMÁNTICA ***
        // Solo validar que los Foreign Keys apunten a la tabla correcta
        bool namingValidationPassed = true;
        if (shortType == "1:N") {
            // SOLO VALIDAR FOREIGN KEYS PARA RELACIONES 1:N
            // Las relaciones 1:1 usan PK a PK, no requieren validación semántica de FK
            bool validRelationship = false;
            QString problematicFK = "";
            QString fkTable = "";
            QString referencedTable = "";
            
            // Obtener listas de FKs primero
            QStringList sourceFKs = tableEditor->getTableForeignKeys(sourceTable);
            QStringList targetFKs = tableEditor->getTableForeignKeys(targetTable);
            
            qDebug() << "DEBUG SEMANTIC VALIDATION:";
            qDebug() << "  - Campo seleccionado DE:" << selectedSourceField;
            qDebug() << "  - Campo seleccionado A:" << selectedTargetField;
            qDebug() << "  - Tabla fuente:" << sourceTable;
            qDebug() << "  - Tabla destino:" << targetTable;
            qDebug() << "  - FKs en tabla fuente:" << sourceFKs;
            qDebug() << "  - FKs en tabla destino:" << targetFKs;
            
            // Verificar si el campo seleccionado "DE" (source) es un FK
            bool isSourceFieldFK = false;
            
            for (const QString &fk : sourceFKs) {
                QString cleanFK = fk;
                // Limpiar emojis tanto en formato visual como en códigos Unicode
                cleanFK = cleanFK.remove(QRegExp("^[🔑🔗\\uDD11\\uDD17]\\s*"))
                                 .remove(QRegExp("\\s*\\(PK\\)$"))
                                 .remove(QRegExp("\\s*\\(FK\\)$"))
                                 .remove(QRegExp("^\\uDD11\\s*"))  // Código Unicode para 🔑
                                 .remove(QRegExp("^\\uDD17\\s*"))  // Código Unicode para 🔗
                                 .trimmed();
                
                // También limpiar el campo seleccionado de la misma manera
                QString cleanSelectedSource = selectedSourceField;
                cleanSelectedSource = cleanSelectedSource.remove(QRegExp("^[🔑🔗\\uDD11\\uDD17]\\s*"))
                                                        .remove(QRegExp("\\s*\\(PK\\)$"))
                                                        .remove(QRegExp("\\s*\\(FK\\)$"))
                                                        .remove(QRegExp("^\\uDD11\\s*"))
                                                        .remove(QRegExp("^\\uDD17\\s*"))
                                                        .trimmed();
                
                qDebug() << "DEBUG FK CLEANUP: Original FK:" << fk << "-> Clean FK:" << cleanFK;
                qDebug() << "DEBUG FK CLEANUP: Selected source:" << selectedSourceField << "-> Clean:" << cleanSelectedSource;
                
                if (cleanFK == cleanSelectedSource) {
                    isSourceFieldFK = true;
                    qDebug() << "DEBUG: ✅ ENCONTRADO FK en source!" << cleanFK;
                    break;
                }
            }
            
            // Verificar si el campo seleccionado "A" (target) es un FK
            bool isTargetFieldFK = false;
            
            for (const QString &fk : targetFKs) {
                QString cleanFK = fk;
                // Limpiar emojis tanto en formato visual como en códigos Unicode
                cleanFK = cleanFK.remove(QRegExp("^[🔑🔗\\uDD11\\uDD17]\\s*"))
                                 .remove(QRegExp("\\s*\\(PK\\)$"))
                                 .remove(QRegExp("\\s*\\(FK\\)$"))
                                 .remove(QRegExp("^\\uDD11\\s*"))  // Código Unicode para 🔑
                                 .remove(QRegExp("^\\uDD17\\s*"))  // Código Unicode para 🔗
                                 .trimmed();
                
                // También limpiar el campo seleccionado de la misma manera
                QString cleanSelectedTarget = selectedTargetField;
                cleanSelectedTarget = cleanSelectedTarget.remove(QRegExp("^[🔑🔗\\uDD11\\uDD17]\\s*"))
                                                        .remove(QRegExp("\\s*\\(PK\\)$"))
                                                        .remove(QRegExp("\\s*\\(FK\\)$"))
                                                        .remove(QRegExp("^\\uDD11\\s*"))
                                                        .remove(QRegExp("^\\uDD17\\s*"))
                                                        .trimmed();
                
                qDebug() << "DEBUG FK CLEANUP: Original FK:" << fk << "-> Clean FK:" << cleanFK;
                qDebug() << "DEBUG FK CLEANUP: Selected target:" << selectedTargetField << "-> Clean:" << cleanSelectedTarget;
                
                if (cleanFK == cleanSelectedTarget) {
                    isTargetFieldFK = true;
                    qDebug() << "DEBUG: ✅ ENCONTRADO FK en target!" << cleanFK;
                    break;
                }
            }
            
            qDebug() << "DEBUG: isSourceFieldFK =" << isSourceFieldFK << ", isTargetFieldFK =" << isTargetFieldFK;
            
            // VALIDACIÓN: Si hay un FK, debe apuntar a la tabla correcta
            if (isSourceFieldFK) {
                // El FK en sourceTable debe apuntar a targetTable
                qDebug() << "DEBUG: Validando FK" << selectedSourceField << "en" << sourceTable << "-> ¿apunta a" << targetTable << "?";
                
                if (validateForeignKeyNaming(selectedSourceField, targetTable, false)) {
                    validRelationship = true;
                    qDebug() << "DEBUG: ✅ FK" << selectedSourceField << "en" << sourceTable << "SÍ apunta correctamente a" << targetTable;
                } else {
                    qDebug() << "DEBUG: ❌ FK" << selectedSourceField << "en" << sourceTable << "NO apunta semánticamente a" << targetTable;
                    problematicFK = selectedSourceField;
                    fkTable = sourceTable;
                    referencedTable = targetTable;
                }
            }
            
            if (isTargetFieldFK && !validRelationship) {
                // El FK en targetTable debe apuntar a sourceTable
                qDebug() << "DEBUG: Validando FK" << selectedTargetField << "en" << targetTable << "-> ¿apunta a" << sourceTable << "?";
                
                if (validateForeignKeyNaming(selectedTargetField, sourceTable, false)) {
                    validRelationship = true;
                    qDebug() << "DEBUG: ✅ FK" << selectedTargetField << "en" << targetTable << "SÍ apunta correctamente a" << sourceTable;
                } else {
                    qDebug() << "DEBUG: ❌ FK" << selectedTargetField << "en" << targetTable << "NO apunta semánticamente a" << sourceTable;
                    if (problematicFK.isEmpty()) { // Solo si no encontramos problema en el primer FK
                        problematicFK = selectedTargetField;
                        fkTable = targetTable;
                        referencedTable = sourceTable;
                    }
                }
            }
            
            // NUEVA VALIDACIÓN: Si no hay ningún FK, también es error
            if (!isSourceFieldFK && !isTargetFieldFK) {
                qDebug() << "DEBUG: ❌ No hay Foreign Keys en ninguno de los campos seleccionados";
                validRelationship = false;
                problematicFK = "Sin FK";
                fkTable = "Ambas tablas";
                referencedTable = "Cualquiera";
            }
            
            // Mostrar error cuando no hay relación válida
            if (!validRelationship) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setWindowTitle("🚫 Conexión Bloqueada - Relación Inválida");
                msgBox.setText("<h3>No se puede crear la relación</h3>");
                
                QString errorMessage;
                if (problematicFK == "Sin FK") {
                    errorMessage = QString(
                        "❌ <b>CONEXIÓN RECHAZADA:</b> No se puede establecer una relación sin Foreign Keys adecuados.<br><br>"
                        "🔍 <b>Campos seleccionados:</b><br>"
                        "• <b>Campo DE:</b> '%1' (en tabla '%2') - %3<br>"
                        "• <b>Campo A:</b> '%4' (en tabla '%5') - %6<br><br>"
                        "🚫 <b>Problema:</b><br>"
                        "Para establecer una relación válida, al menos uno de los campos debe ser un Foreign Key que apunte a la otra tabla.<br><br>"
                        "💡 <b>Ejemplo del problema actual:</b><br>"
                        "• Campo <b>id_maestro</b> (FK) → Tabla <b>seccion</b> ❌<br>"
                        "• Campo <b>Id_seccion</b> (PK) → No es FK ❌<br><br>"
                        "✅ <b>Solución:</b><br>"
                        "1. Vaya a la vista de diseño de una de las tablas<br>"
                        "2. Cree un Foreign Key que tenga relación semántica con la otra tabla<br>"
                        "3. Asegúrese de que el FK contenga el nombre de la tabla que referencia<br>"
                        "4. Regrese e intente crear la relación nuevamente")
                        .arg(selectedSourceField, sourceTable, isSourceFieldFK ? "FK" : "PK")
                        .arg(selectedTargetField, targetTable, isTargetFieldFK ? "FK" : "PK");
                } else {
                    errorMessage = QString(
                        "❌ <b>CONEXIÓN RECHAZADA:</b> El Foreign Key seleccionado no corresponde semánticamente a la tabla que intenta referenciar.<br><br>"
                        "🔍 <b>Problema detectado:</b><br>"
                        "• <b>Foreign Key:</b> '%1' (en tabla '%2')<br>"
                        "• <b>Tabla que intenta referenciar:</b> '%3'<br><br>"
                        "🚫 <b>Error semántico:</b><br>"
                        "El FK <b>'%4'</b> NO está relacionado semánticamente con la tabla <b>'%5'</b>.<br><br>"
                        "💡 <b>Ejemplo:</b><br>"
                        "• FK <b>id_maestro</b> → Debe referenciar tabla <b>maestro</b> ✅<br>"
                        "• FK <b>id_maestro</b> → NO puede referenciar tabla <b>seccion</b> ❌<br><br>"
                        "✅ <b>Para que funcione la relación:</b><br>"
                        "• Use un FK que contenga '%6' (ej: id_%7, %8_id)<br>"
                        "• O cambie la tabla de destino para que coincida con el FK '%9'<br><br>"
                        "🎯 <b>Regla:</b><br>"
                        "Los Foreign Keys DEBEN contener el nombre de la tabla que referencian.")
                        .arg(problematicFK, fkTable, referencedTable, problematicFK, referencedTable)
                        .arg(referencedTable.toLower(), referencedTable.toLower(), referencedTable.toLower(), problematicFK);
                }
                
                msgBox.setInformativeText(errorMessage);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.button(QMessageBox::Ok)->setText("Entendido - Corregiré la relación");
                msgBox.setStyleSheet(
                    "QMessageBox { background-color: white; min-width: 700px; min-height: 500px; }"
                    "QMessageBox QLabel { color: black; font-size: 14px; }"
                    "QPushButton { background-color: #E53E3E; color: white; font-size: 14px; font-weight: bold; min-width: 200px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #C53030; }"
                );
                msgBox.exec();
                namingValidationPassed = false;
            }
            
        } else if (shortType == "N:M") {
            // En relaciones N:M, la validación ya se hace en la tabla intermedia
            // No necesitamos validación adicional de nomenclatura aquí
        }
        
        if (!namingValidationPassed) {
            // La función validateForeignKeyNaming ya habrá mostrado el mensaje de error
            return;
        }
        
        // Si llegamos aquí, todas las validaciones pasaron
        qDebug() << "DEBUG: Validaciones de" << shortType << "pasaron correctamente entre" << sourceTable << "y" << targetTable;
    }
    
    // *** CREAR RELACIÓN VISUAL SIEMPRE ***
    // Crear línea visual entre las tablas si ambas están en el diseñador
    QStringList tablesInDesigner;
    for (auto *item : tableItems) {
        if (item) {
            tablesInDesigner.append(item->getTableName());
        }
    }
    
    // Solo crear línea visual si ambas tablas están en el diseñador
    if (tablesInDesigner.contains(sourceTable) && tablesInDesigner.contains(targetTable)) {
        createRelationshipBetweenTables(sourceTable, targetTable, shortType);
    }
    
    // Add to relationships list
    QString relationshipDesc = QString("%1 → %2 (%3)").arg(sourceTable, targetTable, shortType);
    relationshipsListWidget->addItem(relationshipDesc);
    
    qDebug() << "DEBUG[CREAR RELACIÓN]: ✅ Relación agregada a la lista:" << relationshipDesc;
    qDebug() << "DEBUG[CREAR RELACIÓN]: Total relaciones en lista:" << relationshipsListWidget->count();
    qDebug() << "DEBUG[CREAR RELACIÓN]: Llamando saveDesignerState()...";
    
    // *** NUEVO: Guardar estado después de crear relación ***
    saveDesignerState();
    
    {
        QMessageBox m(this);
        m.setIcon(QMessageBox::Information);
        m.setWindowTitle("Éxito");
        m.setText("Relación creada correctamente");
        m.setStandardButtons(QMessageBox::Ok);

        m.setStyle(QStyleFactory::create("Fusion"));

        QPalette pal = m.palette();
        pal.setColor(QPalette::Window, Qt::white);
        pal.setColor(QPalette::Base, Qt::white);
        pal.setColor(QPalette::Text, Qt::black);
        pal.setColor(QPalette::WindowText, Qt::black);
        pal.setColor(QPalette::Button, QColor("#ECFDF5"));
        pal.setColor(QPalette::ButtonText, Qt::black);
        m.setPalette(pal);

        m.setStyleSheet(
            "QMessageBox { background-color: white; }"
            "QMessageBox QLabel { color: #111827; font-size: 14px; }"
            );

        if (QAbstractButton* ok = m.button(QMessageBox::Ok)) {
            ok->setStyleSheet(
                "color: #111827;"
                "background: #ECFDF5;"
                "border: 1px solid #A7F3D0;"
                "border-radius: 6px;"
                "padding: 6px 12px;"
                );
            QPalette bp = ok->palette();
            bp.setColor(QPalette::ButtonText, Qt::black);
            bp.setColor(QPalette::WindowText, Qt::black);
            ok->setPalette(bp);
            ok->setAutoFillBackground(true);
        }

        m.exec();
    }

}

void RelationshipsView::onDeleteRelationship()
{
    int currentRow = relationshipsListWidget->currentRow();
    if (currentRow >= 0) {
        // Obtener el texto de la relación seleccionada
        QListWidgetItem *selectedItem = relationshipsListWidget->item(currentRow);
        QString relationshipText = selectedItem->text();
        
        // Parsear la relación para identificar las tablas involucradas
        QString sourceTable, targetTable, relationType;
        int arrowPos = relationshipText.indexOf(" → ");
        if (arrowPos >= 0) {
            sourceTable = relationshipText.left(arrowPos).trimmed();
            QString rightPart = relationshipText.mid(arrowPos + 3).trimmed();
            
            int parenPos = rightPart.lastIndexOf("(");
            targetTable = parenPos >= 0 ? rightPart.left(parenPos).trimmed() : rightPart;
            relationType = parenPos >= 0 ? rightPart.mid(parenPos + 1).replace(")", "").trimmed() : "";
        }
        
        // *** CAMBIO: NO limpiar toda el área, solo eliminar la línea de relación específica ***
        // Buscar y eliminar la línea de relación visual específica
        QList<RelationshipLine*> linesToRemove;
        for (auto *line : relationshipLines) {
            if (line && line->getSourceTable() && line->getTargetTable()) {
                QString lineSource = line->getSourceTable()->getTableName();
                QString lineTarget = line->getTargetTable()->getTableName();
                QString lineType = line->getRelationshipType();
                
                // Verificar si coincide (en cualquier dirección)
                bool matches = (lineSource == sourceTable && lineTarget == targetTable && lineType == relationType) ||
                              (lineSource == targetTable && lineTarget == sourceTable && lineType == relationType);
                
                if (matches) {
                    linesToRemove.append(line);
                }
            }
        }
        
        // Eliminar las líneas encontradas
        for (auto *line : linesToRemove) {
            designerScene->removeItem(line);
            relationshipLines.removeAll(line);
            delete line;
        }
        
        // Eliminar de la lista de relaciones
        delete relationshipsListWidget->takeItem(currentRow);
        
        // *** NUEVO: Guardar estado después de eliminar relación ***
        saveDesignerState();
        \
        // Mostrar mensaje de confirmación (texto botón negro)
        {
            QMessageBox m(this);
            m.setIcon(QMessageBox::Information);
            m.setWindowTitle(QString::fromUtf8("🗑️ Relación Eliminada"));
            m.setText(
                QString("La relación '%1' ha sido eliminada.\n\n"
                        "Las tablas permanecen en el diseñador para futuras relaciones.")
                    .arg(relationshipText)
                );
            m.setStandardButtons(QMessageBox::Ok);

            // Evitar diálogo nativo y forzar colores
            m.setStyle(QStyleFactory::create("Fusion"));

            QPalette pal = m.palette();
            pal.setColor(QPalette::Window, Qt::white);
            pal.setColor(QPalette::Base, Qt::white);
            pal.setColor(QPalette::Text, Qt::black);
            pal.setColor(QPalette::WindowText, Qt::black);
            pal.setColor(QPalette::Button, QColor("#ECFDF5"));
            pal.setColor(QPalette::ButtonText, Qt::black);
            m.setPalette(pal);

            // Labels negros
            m.setStyleSheet(
                "QMessageBox { background-color: white; }"
                "QMessageBox QLabel { color: #111827; font-size: 14px; }"
                );

            // Forzar estilo directo al botón para vencer cualquier stylesheet global
            if (QAbstractButton* ok = m.button(QMessageBox::Ok)) {
                ok->setStyleSheet(
                    "color: #111827;"
                    "background: #ECFDF5;"
                    "border: 1px solid #A7F3D0;"
                    "border-radius: 6px;"
                    "padding: 6px 12px;"
                    );
                QPalette bp = ok->palette();
                bp.setColor(QPalette::ButtonText, Qt::black);
                bp.setColor(QPalette::WindowText, Qt::black);
                ok->setPalette(bp);
                ok->setAutoFillBackground(true);
            }

            m.exec();
        }

        
    } else {
        // No hay relación seleccionada
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("⚠️ Selecciona una Relación");
        msgBox.setText("<h3>Ninguna Relación Seleccionada</h3>");
        msgBox.setInformativeText("Debes seleccionar una relación de la lista antes de poder eliminarla.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; min-width: 350px; min-height: 180px; }"
            "QMessageBox QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color: #FF9800; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #F57C00; }"
        );
        msgBox.exec();
    }
}

void RelationshipsView::onTableSelectionChanged()
{
    QListWidgetItem *currentItem = tablesListWidget->currentItem();
    if (currentItem) {
        showTableDetails(currentItem->text());
    }
}

void RelationshipsView::onRelationshipSelectionChanged()
{
    QListWidgetItem *currentItem = relationshipsListWidget->currentItem();
    if (currentItem) {
        updatePropertiesPanel(currentItem->text());
    }
}

void RelationshipsView::onRelationshipDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    
    QString relationshipText = item->text();
    qDebug() << "DEBUG: Doble clic en relación:" << relationshipText;
    
    // Parsear la relación para obtener las tablas y el tipo
    // Formato esperado: "tabla1 → tabla2 (tipo)"
    QStringList parts = relationshipText.split(" → ");
    if (parts.size() != 2) {
        qDebug() << "ERROR: Formato de relación inválido";
        return;
    }
    
    QString sourceTable = parts[0].trimmed();
    QString targetPart = parts[1].trimmed();
    
    // Extraer tabla destino y tipo de relación
    QStringList targetParts = targetPart.split(" (");
    if (targetParts.size() != 2) {
        qDebug() << "ERROR: No se pudo extraer tabla destino y tipo";
        return;
    }
    
    QString targetTable = targetParts[0].trimmed();
    QString relationshipType = targetParts[1].replace(")", "").trimmed();
    
    qDebug() << "DEBUG: Mostrando relación -" << "Source:" << sourceTable << "Target:" << targetTable << "Type:" << relationshipType;
    
    // Limpiar el diseñador actual
    clearDesignerArea();
    
    // Agregar las tablas al diseñador en posiciones específicas
    QPointF sourcePos(50, 100);   // Posición de la tabla origen (izquierda)
    QPointF targetPos(300, 100);  // Posición de la tabla destino (derecha)
    
    addTableToDesigner(sourceTable, sourcePos);
    addTableToDesigner(targetTable, targetPos);
    
    // Crear la línea de relación visual
    createRelationshipBetweenTables(sourceTable, targetTable, relationshipType);
    
    // Actualizar los combos para reflejar la relación actual
    int sourceIndex = sourceTableCombo->findText(sourceTable);
    int targetIndex = targetTableCombo->findText(targetTable);
    
    if (sourceIndex >= 0) sourceTableCombo->setCurrentIndex(sourceIndex);
    if (targetIndex >= 0) targetTableCombo->setCurrentIndex(targetIndex);
    
    // Establecer el tipo de relación en el combo
    for (int i = 0; i < relationshipTypeCombo->count(); ++i) {
        QString comboText = relationshipTypeCombo->itemText(i);
        if (comboText.contains(relationshipType)) {
            relationshipTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    qDebug() << "DEBUG: Relación visualizada correctamente en el diseñador";
}

void RelationshipsView::onRelationshipLineDoubleClicked(RelationshipLine* line)
{
    if (!line) return;
    
    // Get the table names and relationship type from the line
    QString sourceTable = line->getSourceTable()->getTableName();
    QString targetTable = line->getTargetTable()->getTableName();
    QString relationshipType = line->getRelationshipType();
    
    qDebug() << "DEBUG: Doble clic en línea de relación -" << "Source:" << sourceTable << "Target:" << targetTable << "Type:" << relationshipType;
    
    // Find and remove the relationship from the list
    QString relationshipDesc = QString("%1 → %2 (%3)").arg(sourceTable, targetTable, relationshipType);
    
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (item && item->text() == relationshipDesc) {
            delete relationshipsListWidget->takeItem(i);
            break;
        }
    }
    
    // Remove the visual line from the scene
    designerScene->removeItem(line);
    relationshipLines.removeAll(line);
    delete line;
    
    // Show confirmation message
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("🗑️ Relación Eliminada");
    msgBox.setText("<h3>Relación Eliminada</h3>");
    msgBox.setInformativeText(
        QString("La relación '%1' ha sido eliminada exitosamente.\n\n"
               "✅ La línea de conexión fue removida del diseñador.\n"
               "✅ La relación fue eliminada de la lista.\n"
               "✅ Las tablas permanecen intactas en el diseñador.")
               .arg(relationshipDesc)
    );
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->setText("Entendido");
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; min-width: 400px; min-height: 200px; }"
        "QMessageBox QLabel { color: black; font-size: 14px; }"
        "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
        "QPushButton:hover { background-color: #45A049; }"
    );
    msgBox.exec();
    
    qDebug() << "DEBUG: Relación eliminada - Las tablas permanecen en el diseñador";
}

void RelationshipsView::onTableFieldsChanged(const QString &tableName)
{
    if (!tableEditor) return;
    
    // *** PROTECCIÓN: Evitar bucles infinitos durante actualizaciones ***
    static bool isUpdating = false;
    if (isUpdating) {
        qDebug() << "DEBUG: Evitando bucle infinito en onTableFieldsChanged para tabla:" << tableName;
        return;
    }
    isUpdating = true;
    
    try {
        // *** PROTECCIÓN CONTRA CRASHES: Validar relaciones existentes ***
    QStringList invalidRelationships;
    
    // Obtener los campos actuales de la tabla
    QStringList currentFields = tableEditor->getTableFields(tableName);
    QStringList currentPKFields = tableEditor->getTablePrimaryKeys(tableName);
    QStringList currentFKFields = tableEditor->getTableForeignKeys(tableName);
    
    // Silenciosamente validar y actualizar las relaciones sin mostrar mensajes al usuario
    for (int i = relationshipsListWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (!item) continue;
        
        QString relationshipText = item->text();
        
        // Verificar si esta relación involucra la tabla modificada
        if (relationshipText.contains(tableName)) {
            // Parsear la relación
            QStringList parts = relationshipText.split(" → ");
            if (parts.size() == 2) {
                QString sourceTable = parts[0].trimmed();
                QString rightPart = parts[1].trimmed();
                QString targetTable = rightPart.split(" (")[0].trimmed();
                
                // Si la tabla modificada está en esta relación, verificar que aún tenga FK válidos
                if (sourceTable == tableName || targetTable == tableName) {
                    bool hasValidFK = false;
                    
                    if (sourceTable == tableName) {
                        // Verificar que la tabla origen aún tenga FK
                        hasValidFK = !currentFKFields.isEmpty();
                    } else if (targetTable == tableName) {
                        // Verificar que la tabla destino aún tenga FK
                        hasValidFK = !currentFKFields.isEmpty();
                    }
                    
                    if (!hasValidFK) {
                        invalidRelationships << relationshipText;
                        // Solo registrar, no mostrar mensaje al usuario
                    }
                }
            }
        }
    }
    
    // Update the fields for this specific table - usando el método que incluye las llaves
    QStringList fields = tableEditor->getTableFieldsWithKeys(tableName);
    QStringList validFields;
    for (const QString &field : fields) {
        QString cleanField = field.trimmed();
        if (!cleanField.isEmpty()) {
            validFields << cleanField;
        }
    }
    
    // Update our local cache SAFELY
    tableFields[tableName] = validFields;
    
    // *** PROTECCIÓN: Update any existing table items in the designer SAFELY ***
    for (auto *item : tableItems) {
        if (item && item->getTableName() == tableName) {
            try {
                QStringList pkFields = tableEditor->getTablePrimaryKeys(tableName);
                QStringList fkFields = tableEditor->getTableForeignKeys(tableName);
                item->setFieldsWithKeys(validFields, pkFields, fkFields);
                // Force a scene update to show changes immediately
                item->update();
            } catch (...) {
                qDebug() << "ERROR: Excepción al actualizar tabla visual:" << tableName;
                // Continuar con las demás tablas en caso de error
            }
        }
    }
    
    // Update field combos if this table is currently selected, but preserve user selections
    if (sourceTableCombo->currentText() == tableName) {
        QString currentSourceSelection = sourceFieldCombo->currentData().toString();
        updateSourceFields(tableName);
        // Restore user selection if it still exists
        int sourceIndex = sourceFieldCombo->findData(currentSourceSelection);
        if (sourceIndex >= 0) {
            sourceFieldCombo->setCurrentIndex(sourceIndex);
        }
    }
    if (targetTableCombo->currentText() == tableName) {
        QString currentTargetSelection = targetFieldCombo->currentData().toString();
        updateTargetFields(tableName);
        // Restore user selection if it still exists
        int targetIndex = targetFieldCombo->findData(currentTargetSelection);
        if (targetIndex >= 0) {
            targetFieldCombo->setCurrentIndex(targetIndex);
        }
    }
    
    // Solo registrar en debug si hay relaciones afectadas, sin mostrar mensajes al usuario
    if (!invalidRelationships.isEmpty()) {
        qDebug() << "DEBUG: Relaciones actualizadas automáticamente para tabla:" << tableName;
    }
    
        qDebug() << "DEBUG: Actualizados los campos en" << tableName << "tras cambios en campos";
    } catch (const std::exception& e) {
        qDebug() << "ERROR: Excepción al procesar cambios de campos en tabla" << tableName << ":" << e.what();
    } catch (...) {
        qDebug() << "ERROR: Excepción desconocida al procesar cambios de campos en tabla" << tableName;
    }
    
    isUpdating = false;
}

void RelationshipsView::onForeignKeyRemoved(const QString &tableName, const QString &fieldName)
{
    qDebug() << "DEBUG RelationshipsView: Foreign Key eliminada en tabla:" << tableName << "campo:" << fieldName;
    
    // Lista para almacenar relaciones que se eliminarán
    QStringList relationshipsToRemove;
    QList<RelationshipLine*> linesToRemove;
    
    // Buscar relaciones que usen esta tabla y eliminarlas
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        QString relationshipText = item->text();
        
        // Verificar si la relación involucra la tabla afectada
        if (relationshipText.contains(tableName)) {
            relationshipsToRemove << relationshipText;
            qDebug() << "DEBUG: Marcando relación para eliminar:" << relationshipText;
        }
    }
    
    // Eliminar líneas de relación visual que involucren la tabla
    for (auto *line : relationshipLines) {
        QString sourceTable = line->getSourceTable()->getTableName();
        QString targetTable = line->getTargetTable()->getTableName();
        
        if (sourceTable == tableName || targetTable == tableName) {
            linesToRemove.append(line);
        }
    }
    
    // Eliminar las líneas visuales
    for (auto *line : linesToRemove) {
        designerScene->removeItem(line);
        relationshipLines.removeAll(line);
        delete line;
    }
    
    // Eliminar relaciones de la lista
    for (int i = relationshipsListWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (relationshipsToRemove.contains(item->text())) {
            delete relationshipsListWidget->takeItem(i);
        }
    }
    
    // Actualizar campos de la tabla en el cache y elementos visuales
    onTableFieldsChanged(tableName);
    
    // Mostrar mensaje informativo si se eliminaron relaciones
    if (!relationshipsToRemove.isEmpty()) {
        QString message = QString("Se eliminaron %1 relación(es) de la tabla '%2' debido a que se eliminó el campo Foreign Key '%3'.\n\n")
                         .arg(relationshipsToRemove.size())
                         .arg(tableName)
                         .arg(fieldName);
        
        message += "Relaciones eliminadas:\n";
        for (const QString &relation : relationshipsToRemove) {
            message += "• " + relation + "\n";
        }
        
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("🔗 Relaciones Eliminadas");
        msgBox.setText("<h3>Relaciones Eliminadas por Foreign Key</h3>");
        msgBox.setInformativeText(message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; min-width: 450px; min-height: 250px; }"
            "QMessageBox QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color: #FF5722; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #E64A19; }"
        );
        msgBox.exec();
    }
}

void RelationshipsView::onForeignKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName)
{
    qDebug() << "DEBUG RelationshipsView: Foreign Key renombrada en tabla:" << tableName 
             << "de:" << oldFieldName << "a:" << newFieldName;
    
    // *** PROTECCIÓN: Evitar procesamiento durante actualizaciones ***
    static bool isUpdating = false;
    if (isUpdating) {
        qDebug() << "DEBUG: Evitando bucle infinito en onForeignKeyRenamed";
        return;
    }
    isUpdating = true;
    
    try {
        // *** PROTECCIÓN: Actualizar referencias de relaciones existentes ***
        QStringList updatedRelationships;
        bool hasUpdates = false;
        
        // Buscar relaciones que usen este Foreign Key y actualizarlas
        for (int i = 0; i < relationshipsListWidget->count(); ++i) {
            QListWidgetItem *item = relationshipsListWidget->item(i);
            if (!item) continue;
            
            QString relationshipText = item->text();
            
            // Verificar si esta relación involucra la tabla con el FK renombrado
            if (relationshipText.contains(tableName)) {
                qDebug() << "DEBUG: Verificando relación para actualización:" << relationshipText;
                
                // Parsear la relación
                QStringList parts = relationshipText.split(" → ");
                if (parts.size() == 2) {
                    QString sourceTable = parts[0].trimmed();
                    QString rightPart = parts[1].trimmed();
                    QString targetTable = rightPart.split(" (")[0].trimmed();
                    QString relationType = rightPart.mid(rightPart.lastIndexOf("(") + 1).replace(")", "").trimmed();
                    
                    // Si la tabla de la relación es la tabla modificada, la relación podría estar afectada
                    if (sourceTable == tableName || targetTable == tableName) {
                        // Marcar que hay actualizaciones pero mantener la relación
                        hasUpdates = true;
                        updatedRelationships << relationshipText;
                        qDebug() << "DEBUG: Relación marcada para revisión:" << relationshipText;
                    }
                }
            }
        }
        
        // Actualizar la vista visual sin mostrar mensajes (con delay para evitar bucles)
        QTimer::singleShot(100, this, [this, tableName]() {
            try {
                onTableFieldsChanged(tableName);
            } catch (...) {
                qDebug() << "ERROR: Excepción al actualizar campos tras renombrar FK";
            }
        });
        
        // Log de actualización (sin molestar al usuario)
        if (hasUpdates) {
            qDebug() << "DEBUG: Se actualizaron" << updatedRelationships.size() << "relaciones tras renombrar FK";
        }
        
    } catch (...) {
        qDebug() << "ERROR: Excepción en onForeignKeyRenamed";
    }
    
    isUpdating = false;
}

void RelationshipsView::onPrimaryKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName)
{
    qDebug() << "DEBUG RelationshipsView: Primary Key renombrada en tabla:" << tableName 
             << "de:" << oldFieldName << "a:" << newFieldName;
    
    // *** PROTECCIÓN: Evitar procesamiento durante actualizaciones ***
    static bool isUpdating = false;
    if (isUpdating) {
        qDebug() << "DEBUG: Evitando bucle infinito en onPrimaryKeyRenamed";
        return;
    }
    isUpdating = true;
    
    try {
        // *** PROTECCIÓN: Actualizar referencias de relaciones existentes ***
        QStringList updatedRelationships;
        bool hasUpdates = false;
        
        // Buscar relaciones que referencien esta Primary Key
        for (int i = 0; i < relationshipsListWidget->count(); ++i) {
            QListWidgetItem *item = relationshipsListWidget->item(i);
            if (!item) continue;
            
            QString relationshipText = item->text();
            
            // Verificar si esta relación involucra la tabla con el PK renombrado
            if (relationshipText.contains(tableName)) {
                qDebug() << "DEBUG: Verificando relación para actualización de PK:" << relationshipText;
                
                // Parsear la relación
                QStringList parts = relationshipText.split(" → ");
                if (parts.size() == 2) {
                    QString sourceTable = parts[0].trimmed();
                    QString rightPart = parts[1].trimmed();
                    QString targetTable = rightPart.split(" (")[0].trimmed();
                    
                    // Si alguna tabla de la relación es la tabla con PK modificado, marcar para revisión
                    if (sourceTable == tableName || targetTable == tableName) {
                        hasUpdates = true;
                        updatedRelationships << relationshipText;
                        qDebug() << "DEBUG: Relación marcada para revisión de PK:" << relationshipText;
                    }
                }
            }
        }
        
        // Actualizar la vista visual sin mostrar mensajes (con delay para evitar bucles)
        QTimer::singleShot(100, this, [this, tableName]() {
            try {
                onTableFieldsChanged(tableName);
            } catch (...) {
                qDebug() << "ERROR: Excepción al actualizar campos tras renombrar PK";
            }
        });
        
        // Log de actualización (sin molestar al usuario)
        if (hasUpdates) {
            qDebug() << "DEBUG: Se actualizaron" << updatedRelationships.size() << "relaciones tras renombrar PK";
        }
        
    } catch (...) {
        qDebug() << "ERROR: Excepción en onPrimaryKeyRenamed";
    }
    
    isUpdating = false;
}

void RelationshipsView::showTableDetails(const QString &tableName)
{
    // Extract table name from display text (remove emoji)
    QString cleanTableName = tableName;
    if (cleanTableName.startsWith("📊 ")) {
        cleanTableName = cleanTableName.mid(2).trimmed();
    }
    
    // Only show details if it's a real table, not an info message
    if (!availableTables.contains(cleanTableName)) {
        return;
    }
    
    QString tableInfo = QString("Tabla seleccionada: %1\n\n").arg(cleanTableName);
    
    if (tableFields.contains(cleanTableName)) {
        QStringList fields = tableFields[cleanTableName];
        if (!fields.isEmpty()) {
            tableInfo += "Campos disponibles:\n";
            for (const QString &field : fields) {
                if (!field.trimmed().isEmpty()) {
                    tableInfo += QString("• %1\n").arg(field.trimmed());
                }
            }
        } else {
            tableInfo += "Esta tabla no tiene campos definidos.\n";
        }
    } else {
        tableInfo += "No se encontraron campos para esta tabla.\n";
    }
    
    tableInfo += "\nPara crear una relación:\n"
                "1. Selecciona el tipo de relación\n"
                "2. Elige las tablas origen y destino\n"
                "3. Haz clic en 'Crear Relación'";
    
    // descriptionEdit->setText(tableInfo);
}

void RelationshipsView::updatePropertiesPanel(const QString &selectedItem)
{
    // Parse relationship info and populate properties panel - SIMPLIFICADO
    // relationshipNameEdit->setText(selectedItem.split(" ").first());
    
    QString description;
    if (selectedItem.contains("1:1")) {
        description = "Relación Uno a Uno:\n\n"
                     "Cada registro en la tabla origen se relaciona con exactamente un registro en la tabla destino y viceversa.\n\n"
                     "Implementación: Se conectan las Primary Keys de ambas tablas directamente, garantizando que cada registro se relacione únicamente con otro.\n\n"
                     "Ejemplo: Un empleado tiene una sola credencial y cada credencial pertenece a un solo empleado.";
    } else if (selectedItem.contains("1:N")) {
        description = "Relación Uno a Muchos:\n\n"
                     "Un registro en la tabla origen puede relacionarse con múltiples registros en la tabla destino, pero cada registro destino se relaciona con solo uno en el origen.\n\n"
                     "Ejemplo: Un profesor puede impartir múltiples cursos, pero cada curso tiene un solo profesor.";
    } else if (selectedItem.contains("N:M")) {
        description = "Relación Muchos a Muchos:\n\n"
                     "Múltiples registros en la tabla origen pueden relacionarse con múltiples registros en la tabla destino.\n\n"
                     "Ejemplo: Los estudiantes pueden inscribirse en múltiples cursos y cada curso puede tener múltiples estudiantes.";
    } else {
        description = "Selecciona una relación para ver sus detalles.";
    }
    
    // descriptionEdit->setText(description);
}

void RelationshipsView::onTableCloseRequested(TableGraphicsItem* table)
{
    if (!table) return;
    
    QString tableName = table->getTableName();
    
    // Remove any relationship lines connected to this table
    QList<RelationshipLine*> linesToRemove;
    for (auto *line : relationshipLines) {
        if (line && (line->getSourceTable() == table || line->getTargetTable() == table)) {
            linesToRemove.append(line);
        }
    }
    
    // Remove relationship lines from scene and list
    for (auto *line : linesToRemove) {
        designerScene->removeItem(line);
        relationshipLines.removeAll(line);
        
        // Also remove from relationships list widget
        QString sourceTable = line->getSourceTable()->getTableName();
        QString targetTable = line->getTargetTable()->getTableName();
        QString relationshipType = line->getRelationshipType();
        
        // Find and remove the corresponding item from relationships list
        for (int i = 0; i < relationshipsListWidget->count(); ++i) {
            QListWidgetItem *item = relationshipsListWidget->item(i);
            QString itemText = item->text();
            
            // Check if this relationship involves the table being closed
            if (itemText.contains(sourceTable) && itemText.contains(targetTable)) {
                delete relationshipsListWidget->takeItem(i);
                break;
            }
        }
        
        delete line;
    }
    
    // Remove table from scene and list
    designerScene->removeItem(table);
    tableItems.removeAll(table);
    delete table;
    
    qDebug() << "DEBUG: Tabla" << tableName << "cerrada individualmente del diseñador";
}

void RelationshipsView::onInfoButtonClicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("ℹ️ Guía de Relaciones");
    msgBox.setText("<h3>📚 Guía de Relaciones y Validaciones</h3>");
    
    QString detailedInfo = 
        "<h4>🔗 <b>Tipos de Relaciones:</b></h4>"
        "<p><b>• 1:1 (Uno a Uno)</b> - Cada registro se relaciona con exactamente uno<br>"
        "<b>• 1:N (Uno a Muchos)</b> - Un registro se relaciona con múltiples<br>"
        "<b>• N:M (Muchos a Muchos)</b> - Múltiples registros se relacionan con múltiples</p>"
        
        "<h4>🔑 <b>Validaciones por Tipo:</b></h4>"
        "<p><b>📍 Relación 1:1:</b> Conecta Primary Key a Primary Key directamente<br>"
        "<b>📍 Relación 1:N:</b> FK en lado 'muchos', no puede ser PK<br>"
        "<b>📍 Relación N:M:</b> Requiere tabla intermedia</p>"
        
        "<h4>✅ <b>Ejemplos:</b></h4>"
        "<p><b>🏢 1:1:</b> Empleado → Credencial<br>"
        "<b>👨‍🏫 1:N:</b> Profesor → Cursos<br>"
        "<b>📚 N:M:</b> Estudiantes ↔ Cursos</p>"
        
        "<h4>🚨 <b>Validaciones:</b></h4>"
        "<p>• Nomenclatura FK relacionada con tabla<br>"
        "• Tipos de datos compatibles<br>"
        "• No relaciones duplicadas</p>";
    
    msgBox.setInformativeText(detailedInfo);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.button(QMessageBox::Ok)->setText("Entendido");
    
    // Style the message box - más compacto
    msgBox.setStyleSheet(
        "QMessageBox {"
            "background-color: white;"
            "min-width: 450px;"
            "max-width: 500px;"
            "min-height: 350px;"
            "max-height: 400px;"
        "}"
        "QMessageBox QLabel {"
            "color: black;"
            "font-size: 12px;"
            "margin: 8px;"
        "}"
        "QMessageBox QPushButton {"
            "background-color: #2196F3;"
            "color: white;"
            "font-size: 13px;"
            "font-weight: bold;"
            "min-width: 100px;"
            "min-height: 35px;"
            "border: none;"
            "border-radius: 6px;"
            "padding: 6px;"
        "}"
        "QMessageBox QPushButton:hover {"
            "background-color: #1976D2;"
        "}"
    );
    
    msgBox.exec();
}

// TableGraphicsItem Implementation
TableGraphicsItem::TableGraphicsItem(const QString &tableName, const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent), tableName(tableName), isDarkTheme(false), isHovered(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true); // Enable hover events for close button
    
    nameText = new QGraphicsTextItem(tableName, this);
    nameText->setPos(rect.x() + 5, rect.y() + 5);
    QFont font = nameText->font();
    font.setBold(true);
    font.setPointSize(10);
    nameText->setFont(font);
}

TableGraphicsItem::TableGraphicsItem(const QString &tableName, QGraphicsItem *parent)
    : QGraphicsRectItem(QRectF(0, 0, 150, 100), parent), tableName(tableName), isDarkTheme(false), isHovered(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true); // Enable hover events for close button
    
    nameText = new QGraphicsTextItem(tableName, this);
    nameText->setPos(5, 5);
    QFont font = nameText->font();
    font.setBold(true);
    font.setPointSize(10);
    nameText->setFont(font);
}

void TableGraphicsItem::setTableName(const QString &name)
{
    tableName = name;
    nameText->setPlainText(name);
}

QString TableGraphicsItem::getTableName() const
{
    return tableName;
}

void TableGraphicsItem::setFields(const QStringList &fields)
{
    this->fields = fields;
    
    // Clear existing field texts
    for (auto *text : fieldTexts) {
        delete text;
    }
    fieldTexts.clear();
    
    // Add field texts
    QFont fieldFont;
    fieldFont.setPointSize(8);
    
    for (int i = 0; i < fields.size(); ++i) {
        QGraphicsTextItem *fieldText = new QGraphicsTextItem(fields[i], this);
        fieldText->setFont(fieldFont);
        fieldText->setPos(rect().x() + 10, rect().y() + 25 + i * 15);
        
        // Apply current theme to new field text
        if (isDarkTheme) {
            fieldText->setDefaultTextColor(QColor("#CCCCCC"));
        } else {
            fieldText->setDefaultTextColor(QColor("#666666"));
        }
        
        fieldTexts.append(fieldText);
    }
    
    // Adjust rectangle size based on content
    qreal height = qMax(30 + fields.size() * 15, 60); // Minimum height of 60
    setRect(rect().x(), rect().y(), rect().width(), height);
    
    // Force update to show changes immediately
    update();
}

void TableGraphicsItem::setFieldsWithKeys(const QStringList &fields, const QStringList &primaryKeys, const QStringList &foreignKeys)
{
    this->fields = fields;
    this->primaryKeys = primaryKeys;
    this->foreignKeys = foreignKeys;
    
    // Clear existing field texts
    for (auto *text : fieldTexts) {
        delete text;
    }
    fieldTexts.clear();
    
    // Add field texts with key indicators
    QFont fieldFont;
    fieldFont.setPointSize(8);
    
    for (int i = 0; i < fields.size(); ++i) {
        QString fieldName = fields[i];
        QString displayText = fieldName;
        
        // Add key indicators
        if (primaryKeys.contains(fieldName) && foreignKeys.contains(fieldName)) {
            displayText = "🔑🔗 " + fieldName; // PK + FK
        } else if (primaryKeys.contains(fieldName)) {
            displayText = "🔑 " + fieldName; // PK only
        } else if (foreignKeys.contains(fieldName)) {
            displayText = "🔗 " + fieldName; // FK only
        }
        
        QGraphicsTextItem *fieldText = new QGraphicsTextItem(displayText, this);
        fieldText->setFont(fieldFont);
        fieldText->setPos(rect().x() + 10, rect().y() + 25 + i * 15);
        
        // Apply current theme to new field text
        if (isDarkTheme) {
            fieldText->setDefaultTextColor(QColor("#CCCCCC"));
        } else {
            fieldText->setDefaultTextColor(QColor("#666666"));
        }
        
        fieldTexts.append(fieldText);
    }
    
    // Adjust rectangle size based on content
    qreal height = qMax(30 + fields.size() * 15, 60); // Minimum height of 60
    setRect(rect().x(), rect().y(), rect().width(), height);
    
    // Force update to show changes immediately
    update();
}

void TableGraphicsItem::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    QPen pen;
    QBrush brush;
    
    if (isDark) {
        pen.setColor(QColor("#606060"));
        brush.setColor(QColor("#404040"));
        nameText->setDefaultTextColor(QColor("#FFFFFF"));
        
        for (auto *text : fieldTexts) {
            text->setDefaultTextColor(QColor("#CCCCCC"));
        }
    } else {
        pen.setColor(QColor("#CCCCCC"));
        brush.setColor(QColor("#F8F9FA"));
        nameText->setDefaultTextColor(QColor("#000000"));
        
        for (auto *text : fieldTexts) {
            text->setDefaultTextColor(QColor("#666666"));
        }
    }
    
    setPen(pen);
    setBrush(brush);
}

void TableGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw table background
    if (isDarkTheme) {
        painter->setPen(QPen(QColor("#606060"), 1));
        painter->setBrush(QBrush(QColor("#404040")));
    } else {
        painter->setPen(QPen(QColor("#CCCCCC"), 1));
        painter->setBrush(QBrush(QColor("#F8F9FA")));
    }
    
    painter->drawRoundedRect(rect(), 5, 5);
    
    // Draw header separator
    painter->drawLine(rect().x(), rect().y() + 20, 
                     rect().x() + rect().width(), rect().y() + 20);
    
    // Draw close button (X) when hovered or always visible
    if (isHovered || true) { // Always show close button for better UX
        QRectF closeRect = getCloseButtonRect();
        
        // Draw close button background
        QColor closeButtonColor = isHovered ? QColor("#FF4444") : QColor("#CCCCCC");
        if (isDarkTheme) {
            closeButtonColor = isHovered ? QColor("#FF4444") : QColor("#666666");
        }
        
        painter->setBrush(QBrush(closeButtonColor));
        painter->setPen(QPen(closeButtonColor, 1));
        painter->drawEllipse(closeRect);
        
        // Draw X symbol
        painter->setPen(QPen(QColor("white"), 2));
        float margin = 3;
        painter->drawLine(closeRect.x() + margin, closeRect.y() + margin,
                         closeRect.x() + closeRect.width() - margin, 
                         closeRect.y() + closeRect.height() - margin);
        painter->drawLine(closeRect.x() + closeRect.width() - margin, closeRect.y() + margin,
                         closeRect.x() + margin, 
                         closeRect.y() + closeRect.height() - margin);
    }
}

QRectF TableGraphicsItem::getCloseButtonRect() const
{
    // Position close button at top-right corner
    float buttonSize = 16;
    float margin = 5;
    return QRectF(rect().x() + rect().width() - buttonSize - margin, 
                  rect().y() + margin, 
                  buttonSize, buttonSize);
}

void TableGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    isHovered = true;
    update(); // Trigger repaint to show hover effect
    QGraphicsRectItem::hoverEnterEvent(event);
}

void TableGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    isHovered = false;
    update(); // Trigger repaint to remove hover effect
    QGraphicsRectItem::hoverLeaveEvent(event);
}

QVariant TableGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // Update connected relationship lines
        QList<QGraphicsItem*> items = scene()->items();
        for (QGraphicsItem *item : items) {
            RelationshipLine *line = dynamic_cast<RelationshipLine*>(item);
            if (line && (line->getSourceTable() == this || line->getTargetTable() == this)) {
                line->updatePosition();
            }
        }
    }
    
    return QGraphicsRectItem::itemChange(change, value);
}

void TableGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if click is on close button
        QRectF closeRect = getCloseButtonRect();
        if (closeRect.contains(event->pos())) {
            // Emit signal to request table closure
            emit closeRequested(this);
            event->accept();
            return;
        }
    }
    
    // Handle normal table selection and movement
    QGraphicsRectItem::mousePressEvent(event);
}

// RelationshipLine Implementation
RelationshipLine::RelationshipLine(TableGraphicsItem *source, TableGraphicsItem *target, 
                                   const QString &relationshipType, QGraphicsItem *parent)
    : QGraphicsLineItem(parent), sourceTable(source), targetTable(target), 
      relationshipType(relationshipType), isDarkTheme(false)
{
    updatePosition();
    
    typeText = new QGraphicsTextItem(relationshipType, this);
    QFont font = typeText->font();
    font.setPointSize(10);
    font.setBold(true);
    typeText->setFont(font);
    
    // Style the text with a background
    typeText->setDefaultTextColor(QColor("#FFFFFF"));
}

void RelationshipLine::updatePosition()
{
    if (!sourceTable || !targetTable) return;
    
    // Get table rectangles in scene coordinates
    QRectF sourceRect = sourceTable->rect().translated(sourceTable->pos());
    QRectF targetRect = targetTable->rect().translated(targetTable->pos());
    
    // Calculate connection points (center of right edge for source, center of left edge for target)
    QPointF sourcePoint, targetPoint;
    
    if (sourceRect.center().x() < targetRect.center().x()) {
        // Source is to the left of target
        sourcePoint = QPointF(sourceRect.right(), sourceRect.center().y());
        targetPoint = QPointF(targetRect.left(), targetRect.center().y());
    } else {
        // Source is to the right of target
        sourcePoint = QPointF(sourceRect.left(), sourceRect.center().y());
        targetPoint = QPointF(targetRect.right(), targetRect.center().y());
    }
    
    setLine(QLineF(sourcePoint, targetPoint));
    
    // Position type text at the middle of the line
    if (typeText) {
        QPointF midPoint = (sourcePoint + targetPoint) / 2;
        typeText->setPos(midPoint.x() - typeText->boundingRect().width() / 2,
                        midPoint.y() - typeText->boundingRect().height() / 2);
    }
}

void RelationshipLine::setRelationshipType(const QString &type)
{
    relationshipType = type;
    if (typeText) {
        typeText->setPlainText(type);
    }
}

QString RelationshipLine::getRelationshipType() const
{
    return relationshipType;
}

TableGraphicsItem* RelationshipLine::getSourceTable() const
{
    return sourceTable;
}

TableGraphicsItem* RelationshipLine::getTargetTable() const
{
    return targetTable;
}

void RelationshipLine::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    if (typeText) {
        if (isDark) {
            typeText->setDefaultTextColor(QColor("#FFFFFF"));
        } else {
            typeText->setDefaultTextColor(QColor("#1976D2"));
        }
        
        // Add background to text for better visibility
        typeText->setHtml(QString("<div style='background-color: %1; padding: 2px 6px; border-radius: 4px; font-weight: bold;'>%2</div>")
                         .arg(isDark ? "#424242" : "#E3F2FD")
                         .arg(relationshipType));
    }
}

void RelationshipLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Set pen color and style based on theme
    QPen pen;
    if (isDarkTheme) {
        pen.setColor(QColor("#4FC3F7"));
        pen.setWidth(3);
    } else {
        pen.setColor(QColor("#1976D2"));
        pen.setWidth(3);
    }
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    
    QLineF l = line();
    painter->drawLine(l);
    
    // Draw arrow at the end with better styling
    double angle = std::atan2((l.p2().y() - l.p1().y()), (l.p2().x() - l.p1().x()));
    
    QPointF arrowP1 = l.p2() - QPointF(
        std::sin(angle + M_PI / 3) * 15,
        std::cos(angle + M_PI / 3) * 15);
    QPointF arrowP2 = l.p2() - QPointF(
        std::sin(angle + M_PI - M_PI / 3) * 15,
        std::cos(angle + M_PI - M_PI / 3) * 15);
    
    QPolygonF arrowHead;
    arrowHead << l.p2() << arrowP1 << arrowP2;
    
    // Fill arrow with solid color
    QBrush brush;
    if (isDarkTheme) {
        brush.setColor(QColor("#4FC3F7"));
    } else {
        brush.setColor(QColor("#1976D2"));
    }
    brush.setStyle(Qt::SolidPattern);
    painter->setBrush(brush);
    painter->drawPolygon(arrowHead);
}

void RelationshipLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit doubleClicked(this);
}

// Custom QGraphicsView class for drag and drop
RelationshipDesignerView::RelationshipDesignerView(QGraphicsScene *scene, RelationshipsView *parent)
    : QGraphicsView(scene), relationshipsView(parent) 
{
    setAcceptDrops(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHint(QPainter::Antialiasing);
}

void RelationshipDesignerView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void RelationshipDesignerView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void RelationshipDesignerView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText()) {
        QString tableName = event->mimeData()->text();
        QPointF scenePos = mapToScene(event->pos());
        relationshipsView->addTableToDesigner(tableName, scenePos);
        event->acceptProposedAction();
    }
}

bool RelationshipsView::validateDataTypeCompatibility(const QString &sourceTable, const QString &targetTable, const QString &relationshipType)
{
    if (!tableEditor) {
        return true; // Si no hay tableEditor, no podemos validar pero permitimos continuar
    }
    
    QStringList sourcePrimaryKeys = tableEditor->getTablePrimaryKeys(sourceTable);
    QStringList targetPrimaryKeys = tableEditor->getTablePrimaryKeys(targetTable);
    QStringList sourceForeignKeys = tableEditor->getTableForeignKeys(sourceTable);
    QStringList targetForeignKeys = tableEditor->getTableForeignKeys(targetTable);
    
    QStringList incompatibleFields;
    QString validationMessage;
    
    if (relationshipType == "1:1") {
        // En 1:1, verificar que las PKs/FKs que se van a relacionar tengan tipos compatibles
        for (const QString &sourcePK : sourcePrimaryKeys) {
            QString sourcePKType = tableEditor->getFieldType(sourceTable, sourcePK);
            
            for (const QString &targetFK : targetForeignKeys) {
                QString targetFKType = tableEditor->getFieldType(targetTable, targetFK);
                
                if (!sourcePKType.isEmpty() && !targetFKType.isEmpty() && sourcePKType != targetFKType) {
                    incompatibleFields.append(QString("🔑 %1.%2 (%3) ↔ 🔗 %4.%5 (%6)")
                                            .arg(sourceTable, sourcePK, sourcePKType, targetTable, targetFK, targetFKType));
                }
            }
        }
        
        // También verificar la dirección opuesta
        for (const QString &targetPK : targetPrimaryKeys) {
            QString targetPKType = tableEditor->getFieldType(targetTable, targetPK);
            
            for (const QString &sourceFK : sourceForeignKeys) {
                QString sourceFKType = tableEditor->getFieldType(sourceTable, sourceFK);
                
                if (!targetPKType.isEmpty() && !sourceFKType.isEmpty() && targetPKType != sourceFKType) {
                    incompatibleFields.append(QString("🔑 %1.%2 (%3) ↔ 🔗 %4.%5 (%6)")
                                            .arg(targetTable, targetPK, targetPKType, sourceTable, sourceFK, sourceFKType));
                }
            }
        }
        
    } else if (relationshipType == "1:N") {
        // En 1:N, verificar que la PK del lado "uno" sea compatible con las FKs del lado "muchos"
        for (const QString &sourcePK : sourcePrimaryKeys) {
            QString sourcePKType = tableEditor->getFieldType(sourceTable, sourcePK);
            
            for (const QString &targetFK : targetForeignKeys) {
                QString targetFKType = tableEditor->getFieldType(targetTable, targetFK);
                
                if (!sourcePKType.isEmpty() && !targetFKType.isEmpty() && sourcePKType != targetFKType) {
                    incompatibleFields.append(QString("🔑 %1.%2 (%3) ↔ 🔗 %4.%5 (%6)")
                                            .arg(sourceTable, sourcePK, sourcePKType, targetTable, targetFK, targetFKType));
                }
            }
        }
        
    } else if (relationshipType == "N:M") {
        // En N:M, buscar la tabla intermedia y verificar sus FKs con las PKs de ambas tablas
        QStringList allTables = tableEditor->getCreatedTables();
        
        for (const QString &tableName : allTables) {
            if (tableName == sourceTable || tableName == targetTable) continue;
            
            QStringList tableFKs = tableEditor->getTableForeignKeys(tableName);
            if (tableFKs.size() >= 2) {
                // Esta podría ser la tabla intermedia, verificar tipos
                for (const QString &fk : tableFKs) {
                    QString fkType = tableEditor->getFieldType(tableName, fk);
                    
                    // Verificar compatibilidad con sourceTable PKs
                    for (const QString &sourcePK : sourcePrimaryKeys) {
                        QString sourcePKType = tableEditor->getFieldType(sourceTable, sourcePK);
                        if (!fkType.isEmpty() && !sourcePKType.isEmpty() && fkType != sourcePKType) {
                            incompatibleFields.append(QString("🔑 %1.%2 (%3) ↔ 🔗 %4.%5 (%6)")
                                                    .arg(sourceTable, sourcePK, sourcePKType, tableName, fk, fkType));
                        }
                    }
                    
                    // Verificar compatibilidad con targetTable PKs
                    for (const QString &targetPK : targetPrimaryKeys) {
                        QString targetPKType = tableEditor->getFieldType(targetTable, targetPK);
                        if (!fkType.isEmpty() && !targetPKType.isEmpty() && fkType != targetPKType) {
                            incompatibleFields.append(QString("🔑 %1.%2 (%3) ↔ 🔗 %4.%5 (%6)")
                                                    .arg(targetTable, targetPK, targetPKType, tableName, fk, fkType));
                        }
                    }
                }
                break; // Solo verificar la primera tabla intermedia encontrada
            }
        }
    }
    
    // Si hay incompatibilidades, mostrar mensaje de error
    if (!incompatibleFields.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("❌ Error de Tipos de Datos");
        msgBox.setText("<h3>Tipos de Datos Incompatibles</h3>");
        msgBox.setInformativeText(
            QString("No se puede crear la relación <b>%1</b> porque los tipos de datos de los campos clave no son compatibles.<br><br>"
                   "🚫 <b>Incompatibilidades encontradas:</b><br>"
                   "• %2<br><br>"
                   "📘 <b>Explicación:</b><br>"
                   "Para establecer una relación válida, los campos Primary Key y Foreign Key "
                   "que se van a conectar deben tener exactamente el mismo tipo de dato.<br><br>"
                   "✅ <b>Solución:</b><br>"
                   "1. Vaya a la vista de diseño de las tablas afectadas<br>"
                   "2. Modifique los tipos de datos para que coincidan<br>"
                   "3. Guarde los cambios<br>"
                   "4. Regrese e intente crear la relación nuevamente<br><br>"
                   "<b>Ejemplos de tipos compatibles:</b><br>"
                   "• texto ↔ texto<br>"
                   "• número ↔ número<br>"
                   "• fecha ↔ fecha")
                   .arg(relationshipType)
                   .arg(incompatibleFields.join("<br>• "))
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; min-width: 650px; min-height: 450px; }"
            "QMessageBox QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color: #E53E3E; color: white; font-size: 14px; font-weight: bold; min-width: 120px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #C53030; }"
        );
        msgBox.exec();
        return false;
    }
    
    return true; // Todos los tipos son compatibles
}

void RelationshipsView::onTableRenamed(const QString &oldName, const QString &newName)
{
    qDebug() << "DEBUG RelationshipsView: Recibida signal tableRenamed:" << oldName << "->" << newName;
    
    // La forma más segura y efectiva es recargar toda la lista
    // Esto asegura que todo esté sincronizado correctamente
    refreshTableList();
    
    qDebug() << "DEBUG RelationshipsView: Lista de tablas actualizada después del renombrado";
}

void RelationshipsView::onTableDeleted(const QString &tableName)
{
    qDebug() << "DEBUG: Eliminando tabla del diseñador:" << tableName;
    
    // 1. PRIMERO: Eliminar las líneas de relación visuales de forma segura
    QList<RelationshipLine*> linesToRemove;
    for (auto *line : relationshipLines) {
        if (line && line->getSourceTable() && line->getTargetTable() &&
            (line->getSourceTable()->getTableName() == tableName || 
             line->getTargetTable()->getTableName() == tableName)) {
            linesToRemove.append(line);
        }
    }
    
    // Eliminar líneas de relación de forma segura
    for (auto *line : linesToRemove) {
        if (line && designerScene) {
            designerScene->removeItem(line);
            relationshipLines.removeAll(line);
            line->deleteLater(); // Usar deleteLater() para evitar crashes
        }
    }
    
    // 2. SEGUNDO: Eliminar items de tabla del diseñador de forma segura
    QList<TableGraphicsItem*> itemsToRemove;
    for (auto *item : tableItems) {
        if (item && item->getTableName() == tableName) {
            itemsToRemove.append(item);
        }
    }
    
    for (auto *item : itemsToRemove) {
        if (item && designerScene) {
            designerScene->removeItem(item);
            tableItems.removeAll(item);
            delete item; // Eliminar directamente ya que TableGraphicsItem no hereda de QObject
        }
    }
    
    // 3. Eliminar relaciones de la lista de relaciones
    QStringList relationshipsToRemove;
    for (int i = relationshipsListWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (item) {
            QString relationshipText = item->text();
            
            // Verificar si la relación involucra la tabla eliminada
            if (relationshipText.contains(tableName)) {
                // Parsear para obtener las tablas exactas
                QStringList parts = relationshipText.split(" → ");
                if (parts.size() == 2) {
                    QString sourceTable = parts[0].trimmed();
                    QString targetPart = parts[1].trimmed();
                    QString targetTable = targetPart.split(" (")[0].trimmed();
                    
                    if (sourceTable == tableName || targetTable == tableName) {
                        relationshipsToRemove << relationshipText;
                        delete relationshipsListWidget->takeItem(i);
                        qDebug() << "DEBUG: Relación eliminada:" << relationshipText;
                    }
                }
            }
        }
    }
    
    // 4. Eliminar la tabla de las listas internas
    availableTables.removeAll(tableName);
    tableFields.remove(tableName);
    
    // 5. Eliminar de la lista visual de tablas
    for (int i = tablesListWidget->count() - 1; i >= 0; --i) {
        QListWidgetItem *item = tablesListWidget->item(i);
        if (item && (item->text() == tableName || item->data(Qt::UserRole).toString() == tableName)) {
            delete tablesListWidget->takeItem(i);
        }
    }
    
    // 6. Eliminar de los combos de selección de forma segura
    int sourceIndex = sourceTableCombo->findText(tableName);
    if (sourceIndex >= 0) {
        sourceTableCombo->removeItem(sourceIndex);
    }
    
    int targetIndex = targetTableCombo->findText(tableName);
    if (targetIndex >= 0) {
        targetTableCombo->removeItem(targetIndex);
    }
    
    // 7. Agregar mensaje informativo si no quedan tablas
    if (tablesListWidget->count() == 0) {
        QListWidgetItem *item = new QListWidgetItem("📝 No hay tablas creadas");
        item->setFlags(Qt::NoItemFlags);
        item->setForeground(QColor("#999999"));
        tablesListWidget->addItem(item);
    }
    
    // 8. Actualizar la vista del diseñador de forma segura
    if (designerScene) {
        designerScene->update();
    }
    
    // 9. Mostrar mensaje de confirmación si se eliminaron relaciones (sin bloquear)
    if (!relationshipsToRemove.isEmpty()) {
        // Usar QTimer para mostrar el mensaje después de que se complete la eliminación
        QTimer::singleShot(100, this, [this, tableName, relationshipsToRemove]() {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowTitle("🗑️ Tabla y Relaciones Eliminadas");
            msgBox.setText("<h3>Tabla Eliminada Exitosamente</h3>");
            msgBox.setInformativeText(
                QString("La tabla '<b>%1</b>' ha sido eliminada junto con todas sus relaciones.<br><br>"
                       "🗑️ <b>Relaciones eliminadas (%2):</b><br>"
                       "• %3<br><br>"
                       "✅ La tabla fue removida del diseñador visual<br>"
                       "✅ Todas las conexiones fueron limpiadas<br>"
                       "✅ Las tablas restantes permanecen intactas")
                       .arg(tableName)
                       .arg(relationshipsToRemove.size())
                       .arg(relationshipsToRemove.join("<br>• "))
            );
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->setText("Entendido");
            msgBox.setStyleSheet(
                "QMessageBox { background-color: white; min-width: 450px; min-height: 300px; }"
                "QMessageBox QLabel { color: black; font-size: 14px; }"
                "QPushButton { background-color: #E53E3E; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
                "QPushButton:hover { background-color: #C53030; }"
            );
            msgBox.exec();
        });
    }
    
    qDebug() << "DEBUG: Tabla" << tableName << "eliminada del RelationshipsView";
}

bool RelationshipsView::validateForeignKeyNaming(const QString &foreignKeyField, const QString &referencedTable, bool showErrorMessage)
{
    if (foreignKeyField.isEmpty() || referencedTable.isEmpty()) {
        return false;
    }
    
    // Limpiar completamente el nombre del campo FK de cualquier emoji o texto extra
    QString cleanFK = foreignKeyField;
    cleanFK = cleanFK.remove(QRegExp("^[🔑🔗\\uDD11\\uDD17]\\s*"))  // Remover emojis al inicio (visual y Unicode)
                     .remove(QRegExp("\\s*\\(PK\\)$"))              // Remover (PK) al final
                     .remove(QRegExp("\\s*\\(FK\\)$"))              // Remover (FK) al final
                     .remove(QRegExp("^\\uDD11\\s*"))               // Código Unicode para 🔑
                     .remove(QRegExp("^\\uDD17\\s*"))               // Código Unicode para 🔗
                     .trimmed();                                    // Remover espacios
    
    // Convertir ambos nombres a minúsculas para comparación case-insensitive
    QString fkLower = cleanFK.toLower();
    QString tableLower = referencedTable.toLower();
    
    qDebug() << "DEBUG SEMANTIC validateForeignKeyNaming:";
    qDebug() << "  - Original FK field:" << foreignKeyField;
    qDebug() << "  - Cleaned FK field:" << cleanFK;
    qDebug() << "  - FK lowercase:" << fkLower;
    qDebug() << "  - Table lowercase:" << tableLower;
    
    // *** VALIDACIÓN SEMÁNTICA ESTRICTA ***
    // El FK debe contener semánticamente el nombre de la tabla que referencia
    // Ejemplo: para tabla "clases", FK válido = "id_clase", FK inválido = "id_maestro"
    
    bool isValid = false;
    
    // Extraer la parte semántica del FK (quitar prefijos/sufijos comunes)
    QString fkCore = fkLower;
    fkCore = fkCore.replace("id_", "").replace("_id", "").replace("id", "");
    
    // Crear versión singular de la tabla para comparación más flexible
    QString tableSingular = tableLower;
    if (tableLower.endsWith("s") && tableLower.length() > 2) {
        tableSingular = tableLower.left(tableLower.length() - 1);
    }
    
    qDebug() << "  - FK core (sem 'id'):" << fkCore;
    qDebug() << "  - Table singular:" << tableSingular;
    
    // VERIFICACIÓN SEMÁNTICA: El FK debe estar relacionado con la tabla
    if (!fkCore.isEmpty()) {
        // Verificar si el núcleo del FK coincide con la tabla (o su forma singular)
        if (fkCore == tableLower || fkCore == tableSingular) {
            isValid = true;
            qDebug() << "DEBUG: ✅ Relación semántica válida - FK core '" << fkCore << "' coincide con tabla '" << tableLower << "'";
        }
        // También permitir si la tabla contiene el core del FK
        else if (tableLower.contains(fkCore) || tableSingular.contains(fkCore)) {
            isValid = true;
            qDebug() << "DEBUG: ✅ Relación semántica válida - tabla contiene FK core '" << fkCore << "'";
        }
        // O si el FK contiene la tabla (para casos especiales)
        else if (fkCore.contains(tableLower) || fkCore.contains(tableSingular)) {
            isValid = true;
            qDebug() << "DEBUG: ✅ Relación semántica válida - FK core contiene tabla";
        }
    }
    
    // Verificación directa con patrones completos si no se encontró relación en el core
    if (!isValid) {
        // Verificar patrones directos con la tabla completa
        if (fkLower == tableLower + "_id" || fkLower == "id_" + tableLower || 
            fkLower == tableLower + "id" || fkLower == "id" + tableLower || 
            fkLower == tableLower) {
            isValid = true;
            qDebug() << "DEBUG: ✅ Patrón directo válido con tabla completa";
        }
        // Verificar patrones con forma singular
        else if (fkLower == tableSingular + "_id" || fkLower == "id_" + tableSingular || 
                 fkLower == tableSingular + "id" || fkLower == "id" + tableSingular || 
                 fkLower == tableSingular) {
            isValid = true;
            qDebug() << "DEBUG: ✅ Patrón directo válido con tabla singular";
        }
    }
    
    if (!isValid) {
        qDebug() << "DEBUG: ❌ RELACIÓN SEMÁNTICA INVÁLIDA";
        qDebug() << "DEBUG: ❌ FK '" << fkLower << "' NO está relacionado semánticamente con tabla '" << tableLower << "'";
        qDebug() << "DEBUG: ❌ FK core '" << fkCore << "' no coincide con '" << tableLower << "' ni '" << tableSingular << "'";
    }
    
    
    // Solo mostrar mensaje de error si se solicita explícitamente
    if (!isValid && showErrorMessage) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("🚫 Relación Bloqueada - Sin Relación Semántica");
        msgBox.setText("<h3>No se puede crear la relación</h3>");
        msgBox.setInformativeText(
            QString("❌ <b>La relación fue RECHAZADA</b> porque el Foreign Key no está relacionado semánticamente con la tabla.<br><br>"
                   "🚫 <b>Campo Foreign Key:</b> '%1'<br>"
                   "📋 <b>Tabla que intenta referenciar:</b> '%2'<br><br>"
                   "💡 <b>Problema detectado:</b><br>"
                   "El nombre '%3' no tiene relación semántica con '%4'.<br><br>"
                   "🔍 <b>Ejemplo del problema:</b><br>"
                   "• Tabla: <b>clases</b><br>"
                   "• FK incorrecto: <b>id_maestro</b> ❌ (no está relacionado)<br>"
                   "• FK correcto: <b>id_clase</b> ✅ (está relacionado)<br><br>"
                   "✅ <b>Para conectar con '%5', use nombres como:</b><br>"
                   "• %6_id<br>"
                   "• id_%7<br>"
                   "• %8<br><br>"
                   "🎯 <b>Regla fundamental:</b><br>"
                   "El Foreign Key DEBE contener el nombre de la tabla que referencia (no importa mayúsculas, minúsculas, singular o plural).<br><br>"
                   "⚠️ <b>Esta validación evita:</b><br>"
                   "• Conexiones sin sentido semántico<br>"
                   "• Errores de diseño en la base de datos<br>"
                   "• Confusión en el modelo de datos<br><br>"
                   "<b>✅ Solución:</b><br>"
                   "1. Vaya a la vista de diseño de la tabla<br>"
                   "2. Renombre el Foreign Key para que contenga '%9'<br>"
                   "3. Regrese e intente crear la relación nuevamente<br><br>"
                   "🚫 <b>La relación NO se creará hasta que haya relación semántica.</b>")
                   .arg(foreignKeyField, referencedTable, cleanFK, referencedTable, referencedTable, 
                        tableLower, tableLower, tableLower, referencedTable)
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido - Corregiré el nombre");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; min-width: 650px; min-height: 450px; }"
            "QMessageBox QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color: #E53E3E; color: white; font-size: 14px; font-weight: bold; min-width: 200px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #C53030; }"
        );
        msgBox.exec();
    }
    
    return isValid;
}

bool RelationshipsView::hasRelationshipForField(const QString &tableName, const QString &fieldName)
{
    qDebug() << "DEBUG RelationshipsView::hasRelationshipForField:" << tableName << fieldName;
    
    // Limpiar el nombre del campo de iconos y espacios
    QString cleanFieldName = fieldName;
    cleanFieldName = cleanFieldName.remove("🔑🔗🔶")
                                  .remove("🔑🔗")
                                  .remove("🔑🔶")
                                  .remove("🔗🔶")
                                  .remove("🔑")
                                  .remove("🔗")
                                  .remove("🔶")
                                  .trimmed();
    
    qDebug() << "DEBUG: Campo limpio para verificación:" << cleanFieldName;
    
    // Buscar en las relaciones existentes
    for (int i = 0; i < relationshipsListWidget->count(); ++i) {
        QListWidgetItem *item = relationshipsListWidget->item(i);
        if (!item) continue;
        
        QString relationshipText = item->text();
        qDebug() << "DEBUG: Verificando relación:" << relationshipText;
        
        // Parsear la relación: "tabla1 → tabla2 (tipo)"
        QStringList parts = relationshipText.split(" → ");
        if (parts.size() != 2) continue;
        
        QString sourceTable = parts[0].trimmed();
        QString rightPart = parts[1].trimmed();
        QString targetTable = rightPart.split(" (")[0].trimmed();
        
        qDebug() << "DEBUG: Relación parseada - Source:" << sourceTable << "Target:" << targetTable;
        
        // Verificar si esta relación involucra la tabla especificada
        if (sourceTable == tableName) {
            // Esta tabla es origen de la relación
            // Verificar si el campo está relacionado con la tabla destino
            QString tableCore = targetTable.toLower();
            QString fieldCore = cleanFieldName.toLower();
            
            // Remover prefijos/sufijos comunes del campo
            fieldCore = fieldCore.replace("id_", "").replace("_id", "").replace("id", "");
            
            // Crear versión singular de la tabla
            QString tableSingular = tableCore;
            if (tableCore.endsWith("s") && tableCore.length() > 2) {
                tableSingular = tableCore.left(tableCore.length() - 1);
            }
            
            qDebug() << "DEBUG: Verificando si field core" << fieldCore << "coincide con tabla" << tableCore << "o singular" << tableSingular;
            
            // Verificar coincidencia semántica
            if (fieldCore == tableCore || fieldCore == tableSingular ||
                tableCore.contains(fieldCore) || tableSingular.contains(fieldCore) ||
                fieldCore.contains(tableCore) || fieldCore.contains(tableSingular)) {
                qDebug() << "DEBUG: ✅ Relación encontrada para campo" << cleanFieldName << "en tabla" << tableName;
                return true;
            }
        }
        
        if (targetTable == tableName) {
            // Esta tabla es destino de la relación
            qDebug() << "DEBUG: Tabla" << tableName << "es destino de relación desde" << sourceTable;
            
            // Verificar si el campo FK apunta a la tabla source
            // Por ejemplo: maestro_id debería apuntar a la tabla "maestro"
            QString expectedSourceTable = cleanFieldName;
            if (expectedSourceTable.endsWith("_id")) {
                expectedSourceTable = expectedSourceTable.left(expectedSourceTable.length() - 3);
            } else if (expectedSourceTable.startsWith("id_")) {
                expectedSourceTable = expectedSourceTable.mid(3);
            }
            
            qDebug() << "DEBUG: Campo esperado de tabla source:" << expectedSourceTable << "Tabla source real:" << sourceTable;
            
            if (expectedSourceTable.compare(sourceTable, Qt::CaseInsensitive) == 0) {
                qDebug() << "DEBUG: ✅ Relación encontrada para campo" << cleanFieldName << "en tabla" << tableName;
                return true;
            }
        }
    }
    
    qDebug() << "DEBUG: ❌ No se encontró relación establecida para campo" << cleanFieldName << "en tabla" << tableName;
    return false;
}
