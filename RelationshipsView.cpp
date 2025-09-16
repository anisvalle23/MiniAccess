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
    
    createRelationshipBtn->setFixedSize(160, 40);
    deleteRelationshipBtn->setFixedSize(120, 40);
    
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
    
    toolbarLayout->addWidget(createRelationshipBtn);
    toolbarLayout->addWidget(deleteRelationshipBtn);
    
    mainLayout->addWidget(toolbarWidget);
    
    // Connect signals
    connect(createRelationshipBtn, &QPushButton::clicked, this, &RelationshipsView::onNewRelationshipClicked);
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
        "• <b>N:M</b> - Muchos a muchos<br><br>"
        "<b>Validaciones de Llaves:</b><br>"
        "• <b>1:1</b> → Un campo puede ser Primary Key y Foreign Key al mismo tiempo.<br>"
        "• <b>1:N</b> → La Foreign Key no debe ser Primary Key en el lado muchos.<br>"
        "• <b>N:M</b> → Las Foreign Keys en la tabla intermedia pueden formar una Primary Key compuesta."
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
            QListWidgetItem *item = new QListWidgetItem(tableName);
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

void RelationshipsView::onNewRelationshipClicked()
{
    // Limpiar el área de diseño (canvas) para preparar una nueva relación
    clearDesignerArea();
    
    // Limpiar selecciones de los combos para que el usuario seleccione nuevas tablas
    sourceTableCombo->setCurrentIndex(-1);
    targetTableCombo->setCurrentIndex(-1);
    
    // Mostrar mensaje de confirmación
    QMessageBox::information(this, "Área Limpiada", 
        "El área de diseño ha sido limpiada.\n"
        "Puede arrastrar tablas desde la lista izquierda para crear una nueva relación.");
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
            // Relación 1:1: PK puede ser también FK - ESTO ES VÁLIDO
            bool sourceHasValidFK = !sourceForeignKeys.isEmpty();
            bool targetHasValidFK = !targetForeignKeys.isEmpty();
            
            if (!sourceHasValidFK && !targetHasValidFK) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación 1:1 - Foreign Key Requerida");
                msgBox.setText("<h3>Foreign Key Requerida para Relación 1:1</h3>");
                msgBox.setInformativeText(
                    QString("Para establecer una relación 1:1, una de las tablas debe contener una Foreign Key que apunte a la Primary Key de la otra.<br><br>"
                           "⚠️ <b>Estado actual:</b><br>"
                           "• Tabla <b>'%1'</b>: %2 Foreign Keys<br>"
                           "• Tabla <b>'%3'</b>: %4 Foreign Keys<br><br>"
                           "✅ <b>Nota:</b> En relaciones 1:1, un campo puede ser Primary Key y Foreign Key al mismo tiempo.<br><br>"
                           "<b>Solución:</b><br>"
                           "1. Vaya a la vista de diseño de una de las tablas<br>"
                           "2. Seleccione el campo de referencia<br>"
                           "3. Marque la casilla 'Foreign Key' en las propiedades<br>"
                           "4. Regrese e intente crear la relación nuevamente")
                           .arg(sourceTable).arg(sourceForeignKeys.size())
                           .arg(targetTable).arg(targetForeignKeys.size())
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
            
            // Información adicional si hay campos PK+FK (esto es válido en 1:1)
            if (!sourcePKandFK.isEmpty() || !targetPKandFK.isEmpty()) {
                qDebug() << "DEBUG: Relación 1:1 válida - Se encontraron campos PK+FK en:" 
                         << "Tabla" << sourceTable << ":" << sourcePKandFK 
                         << "Tabla" << targetTable << ":" << targetPKandFK;
            }
            
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
            // Relación N:M: Se requiere una tabla intermedia con dos Foreign Keys
            QStringList allTables = tableEditor->getCreatedTables();
            bool foundIntermediateTable = false;
            QString intermediateTableName;
            
            // Buscar una tabla intermedia que tenga FK a ambas tablas
            for (const QString &tableName : allTables) {
                if (tableName == sourceTable || tableName == targetTable) continue;
                
                QStringList tableFKs = tableEditor->getTableForeignKeys(tableName);
                
                // Verificar si tiene FK suficientes (al menos 2 para una relación N:M)
                if (tableFKs.size() >= 2) {
                    foundIntermediateTable = true;
                    intermediateTableName = tableName;
                    break;
                }
            }
            
            if (!foundIntermediateTable) {
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("🔗 Relación N:M - Tabla Intermedia Requerida");
                msgBox.setText("<h3>Tabla Intermedia Requerida para Relación N:M</h3>");
                msgBox.setInformativeText(
                    QString("Para establecer una relación N:M, se requiere una tabla intermedia con dos Foreign Keys que apunten a las Primary Keys de ambas tablas.<br><br>"
                           "⚠️ <b>Problema:</b><br>"
                           "• Tabla <b>'%1'</b>: Tabla origen<br>"
                           "• Tabla <b>'%2'</b>: Tabla destino<br>"
                           "• <b>Tabla intermedia</b>: ❌ No encontrada<br><br>"
                           "<b>Solución:</b><br>"
                           "1. Cree una nueva tabla intermedia (ej: '%3_%4')<br>"
                           "2. Agregue un campo Foreign Key que apunte a '%5'<br>"
                           "3. Agregue otro campo Foreign Key que apunte a '%6'<br>"
                           "4. Marque ambos campos como 'Foreign Key' en las propiedades<br>"
                           "5. Regrese e intente crear la relación nuevamente")
                           .arg(sourceTable, targetTable, sourceTable, targetTable, sourceTable, targetTable)
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
        
        // Si llegamos aquí, todas las validaciones pasaron
        qDebug() << "DEBUG: Validaciones de" << shortType << "pasaron correctamente entre" << sourceTable << "y" << targetTable;
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
        // Obtener el texto de la relación seleccionada
        QListWidgetItem *selectedItem = relationshipsListWidget->item(currentRow);
        QString relationshipText = selectedItem->text();
        
        // Eliminar de la lista primero
        delete relationshipsListWidget->takeItem(currentRow);
        
        // Limpiar completamente el área del diseñador (como "Nueva Relación")
        clearDesignerArea();
        
        // Limpiar selecciones de los combos
        sourceTableCombo->setCurrentIndex(-1);
        targetTableCombo->setCurrentIndex(-1);
        
        // Mostrar mensaje de confirmación
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle("🗑️ Relación Eliminada");
        msgBox.setText("<h3>Relación Eliminada Exitosamente</h3>");
        msgBox.setInformativeText(QString("La relación '%1' ha sido eliminada.\n\nEl área de diseño ha sido limpiada y está lista para crear una nueva relación.").arg(relationshipText));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido");
        msgBox.setStyleSheet(
            "QMessageBox { background-color: white; min-width: 400px; min-height: 200px; }"
            "QMessageBox QLabel { color: black; font-size: 14px; }"
            "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; font-weight: bold; min-width: 100px; min-height: 40px; border: none; border-radius: 6px; padding: 8px; }"
            "QPushButton:hover { background-color: #45A049; }"
        );
        msgBox.exec();
        
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
