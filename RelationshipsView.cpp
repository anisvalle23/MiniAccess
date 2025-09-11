#include "RelationshipsView.h"
#include "ThemeManager.h"
#include "TableEditor.h"
#include <QApplication>
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
#include <cmath>

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
    
    // Load initial data
    loadTables();
    loadRelationships();
}

void RelationshipsView::setTableEditor(TableEditor *editor)
{
    tableEditor = editor;
    
    // Reload tables when editor is set
    loadTables();
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
    refreshBtn = new QPushButton("🔄 Actualizar");
    
    createRelationshipBtn->setFixedSize(160, 40);
    deleteRelationshipBtn->setFixedSize(120, 40);
    refreshBtn->setFixedSize(130, 40);
    
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
    refreshBtn->setStyleSheet(secondaryButtonStyle);
    
    toolbarLayout->addWidget(createRelationshipBtn);
    toolbarLayout->addWidget(deleteRelationshipBtn);
    toolbarLayout->addWidget(refreshBtn);
    
    mainLayout->addWidget(toolbarWidget);
    
    // Connect signals
    connect(createRelationshipBtn, &QPushButton::clicked, this, &RelationshipsView::onCreateRelationship);
    connect(deleteRelationshipBtn, &QPushButton::clicked, this, &RelationshipsView::onDeleteRelationship);
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        refreshTableList();
        // Show a brief confirmation
        QMessageBox::information(this, "Actualizado", "Las tablas y campos han sido actualizados correctamente.");
    });
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
    relationshipsGroup = new QGroupBox("� Relaciones");
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
            "background: #E8F5E8;"
            "border: 1px solid #C8E6C9;"
            "border-radius: 6px;"
            "padding: 8px;"
        "}"
    );
    QVBoxLayout *cardLayout = new QVBoxLayout(instructionCard);
    cardLayout->setMargin(8);
    
    QLabel *instructionText = new QLabel("💡 Arrastra tablas desde la lista izquierda aquí para conectarlas");
    instructionText->setStyleSheet("color: #2E7D32; font-size: 11px; font-weight: 500;");
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
    
    propertiesGroup = new QGroupBox("⚙️ Nueva Relación");
    propertiesGroup->setFont(QFont("Inter", 12, QFont::Bold));
    propertiesGroup->setStyleSheet(
        "QGroupBox {"
            "font-weight: bold;"
            "border: 1px solid #E8F5E8;"
            "border-radius: 8px;"
            "margin-top: 12px;"
            "background: #F9FFF9;"
        "}"
        "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "left: 12px;"
            "padding: 0 6px;"
            "color: #2E7D32;"
            "background: white;"
        "}"
    );
    QVBoxLayout *groupLayout = new QVBoxLayout(propertiesGroup);
    groupLayout->setSpacing(12);
    
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
    
    // Información simple
    QLabel *infoLabel = new QLabel(
        "<b>Tipos de Relaciones:</b><br>"
        "• <b>1:1</b> - Uno a uno<br>"
        "• <b>1:N</b> - Uno a muchos<br>"
        "• <b>N:M</b> - Muchos a muchos"
    );
    infoLabel->setStyleSheet("color: #666; font-size: 10px; margin: 8px 0;");
    infoLabel->setWordWrap(true);
    
    // Agregar al layout
    groupLayout->addWidget(relationshipTypeLabel);
    groupLayout->addWidget(relationshipTypeCombo);
    groupLayout->addSpacing(8);
    groupLayout->addWidget(sourceTableLabel);
    groupLayout->addWidget(sourceTableCombo);
    groupLayout->addSpacing(4);
    groupLayout->addWidget(targetTableLabel);
    groupLayout->addWidget(targetTableCombo);
    groupLayout->addSpacing(12);
    groupLayout->addWidget(applyChangesBtn);
    groupLayout->addSpacing(8);
    groupLayout->addWidget(infoLabel);
    groupLayout->addStretch();
    
    propertiesLayout->addWidget(propertiesGroup);
    
    // Connect table combo changes to update field combos - SIMPLIFICADO
    connect(sourceTableCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [this](const QString &tableName) {
                if (tableFields.contains(tableName)) {
                    // Auto-populate fields when table is selected
                }
            });
    
    connect(targetTableCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [this](const QString &tableName) {
                if (tableFields.contains(tableName)) {
                    // Auto-populate fields when table is selected
                }
            });
    
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
    availableTables.clear();
    tableFields.clear();
    tablesListWidget->clear();
    sourceTableCombo->clear();
    targetTableCombo->clear();
    
    // Get tables from TableEditor if available
    if (tableEditor) {
        QStringList createdTables = tableEditor->getCreatedTables();
        
        for (const QString &tableName : createdTables) {
            availableTables.append(tableName);
            
            // Create draggable item for tables list
            QListWidgetItem *item = new QListWidgetItem("📊 " + tableName);
            item->setData(Qt::UserRole, tableName);
            item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
            tablesListWidget->addItem(item);
            
            sourceTableCombo->addItem(tableName);
            targetTableCombo->addItem(tableName);
            
            // Get fields from TableEditor - con llaves incluidas para mostrar las primary keys
            QStringList fields = tableEditor->getTableFieldsWithKeys(tableName);
            
            // Filter out empty fields and ensure we only get actual field names
            QStringList validFields;
            for (const QString &field : fields) {
                QString cleanField = field.trimmed();
                if (!cleanField.isEmpty()) {
                    validFields << cleanField;
                }
            }
            
            // Always store the fields, even if empty
            tableFields[tableName] = validFields;
        }
        
        // If no tables found, show a helpful message
        if (createdTables.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem("📝 No hay tablas creadas");
            item->setFlags(Qt::NoItemFlags); // Make it non-selectable and non-draggable
            item->setForeground(QColor("#999999"));
            tablesListWidget->addItem(item);
        }
    }
    
    // Remove the fallback predefined tables - only show real tables from TableEditor
}

void RelationshipsView::loadRelationships()
{
    relationshipsListWidget->clear();
    // Las relaciones se mostrarán solo cuando se vayan creando
    // No hay relaciones predeterminadas, todo será dinámico
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
        if (!fields.isEmpty()) {
            tableItem->setFields(fields);
        }
    }
    
    tableItem->updateTheme(isDarkTheme);
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
        if (!fields.isEmpty()) {
            newTableItem->setFields(fields);
        }
    }
    
    newTableItem->updateTheme(isDarkTheme);
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
        designerScene->addItem(relationshipLine);
        relationshipLines.append(relationshipLine);
    }
}

void RelationshipsView::refreshTableList()
{
    // Store current selections to restore them if possible
    QString currentSourceTable = sourceTableCombo->currentText();
    QString currentTargetTable = targetTableCombo->currentText();
    
    // Reload tables from TableEditor
    loadTables();
    loadRelationships();
    
    // Restore selections if the tables still exist
    int sourceIndex = sourceTableCombo->findText(currentSourceTable);
    if (sourceIndex >= 0) {
        sourceTableCombo->setCurrentIndex(sourceIndex);
    }
    
    int targetIndex = targetTableCombo->findText(currentTargetTable);
    if (targetIndex >= 0) {
        targetTableCombo->setCurrentIndex(targetIndex);
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
    
    // Also remove relationship lines that reference deleted tables
    QList<RelationshipLine*> linesToRemove;
    for (auto *line : relationshipLines) {
        QString sourceTableName = line->getSourceTable()->getTableName();
        QString targetTableName = line->getTargetTable()->getTableName();
        
        if (!availableTables.contains(sourceTableName) || !availableTables.contains(targetTableName)) {
            linesToRemove.append(line);
        }
    }
    
    // Remove invalid relationship lines
    for (auto *line : linesToRemove) {
        designerScene->removeItem(line);
        relationshipLines.removeAll(line);
        delete line;
    }
    
    // Update existing table items with new field information
    for (auto *item : tableItems) {
        QString tableName = item->getTableName();
        if (tableFields.contains(tableName)) {
            QStringList fields = tableFields[tableName];
            // Force update fields even if previously empty
            item->setFields(fields);
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
                item->setFields(validFields);
            }
        }
    }
}

void RelationshipsView::onCreateRelationship()
{
    QString sourceTable = sourceTableCombo->currentText();
    QString targetTable = targetTableCombo->currentText();
    QString relationshipType = relationshipTypeCombo->currentText();
    
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
        QMessageBox::warning(this, "Error", "Debe seleccionar ambas tablas");
        return;
    }
    
    if (sourceTable == targetTable) {
        QMessageBox::warning(this, "Error", "No puede crear una relación de una tabla consigo misma");
        return;
    }
    
    // Create visual representation
    createRelationshipBetweenTables(sourceTable, targetTable, shortType);
    
    // Add to relationships list
    QString relationshipDesc = QString("%1 → %2 (%3)").arg(sourceTable, targetTable, shortType);
    relationshipsListWidget->addItem(relationshipDesc);
    
    QMessageBox::information(this, "Éxito", "Relación creada correctamente");
}

void RelationshipsView::onDeleteRelationship()
{
    int currentRow = relationshipsListWidget->currentRow();
    if (currentRow >= 0) {
        relationshipsListWidget->takeItem(currentRow);
        QMessageBox::information(this, "Relación Eliminada", "La relación ha sido eliminada.");
    } else {
        QMessageBox::warning(this, "Error", "Selecciona una relación para eliminar.");
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

void RelationshipsView::onTableFieldsChanged(const QString &tableName)
{
    if (!tableEditor) return;
    
    // Update the fields for this specific table - usando el método que incluye las llaves
    QStringList fields = tableEditor->getTableFieldsWithKeys(tableName);
    QStringList validFields;
    for (const QString &field : fields) {
        QString cleanField = field.trimmed();
        if (!cleanField.isEmpty()) {
            validFields << cleanField;
        }
    }
    
    // Update our local cache
    tableFields[tableName] = validFields;
    
    // Update any existing table items in the designer
    for (auto *item : tableItems) {
        if (item->getTableName() == tableName) {
            item->setFields(validFields);
            // Force a scene update to show changes immediately
            item->update();
        }
    }
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

// TableGraphicsItem Implementation
TableGraphicsItem::TableGraphicsItem(const QString &tableName, const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent), tableName(tableName), isDarkTheme(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    
    nameText = new QGraphicsTextItem(tableName, this);
    nameText->setPos(rect.x() + 5, rect.y() + 5);
    QFont font = nameText->font();
    font.setBold(true);
    font.setPointSize(10);
    nameText->setFont(font);
}

TableGraphicsItem::TableGraphicsItem(const QString &tableName, QGraphicsItem *parent)
    : QGraphicsRectItem(QRectF(0, 0, 150, 100), parent), tableName(tableName), isDarkTheme(false)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    
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
    Q_UNUSED(event)
    // Handle table selection
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

void RelationshipsView::onTableRenamed(const QString &oldName, const QString &newName)
{
    qDebug() << "DEBUG RelationshipsView: Recibida signal tableRenamed:" << oldName << "->" << newName;
    
    // La forma más segura y efectiva es recargar toda la lista
    // Esto asegura que todo esté sincronizado correctamente
    refreshTableList();
    
    qDebug() << "DEBUG RelationshipsView: Lista de tablas actualizada después del renombrado";
}
