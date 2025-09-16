#include "TableEditor.h"
#include "TableView.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

TableEditor::TableEditor(QWidget *parent)
    : QWidget(parent), isDarkTheme(false)
{
    setupUI();
    styleComponents();
}

void TableEditor::setupUI()
{
    // Main layout
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create splitter for resizable panels
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(0); // Sin separador visible
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
            "background-color: transparent;"
            "width: 0px;"
        "}"
    );
    
    createLeftPanel();
    createRightPanel();
    createTableCreationPanel();
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    
    // Set initial sizes (left panel smaller)
    mainSplitter->setSizes({220, 1080}); // Panel izquierdo más pequeño, más espacio para el contenido principal
    mainSplitter->setCollapsible(0, false);
    mainSplitter->setCollapsible(1, false);
    
    mainLayout->addWidget(mainSplitter);
}

void TableEditor::createLeftPanel()
{
    leftPanel = new QWidget();
    leftPanel->setFixedWidth(220);
    leftPanel->setStyleSheet("QWidget { background-color: #FAFAFA; }");

    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(12, 16, 12, 16);
    leftPanelLayout->setSpacing(12);

    // Botón nueva tabla
    newTableBtn = new QPushButton("Nueva Tabla");
    leftPanelLayout->addWidget(newTableBtn);

    // Table list section
    updateTableList();

    // <<< ARI es donde lo pones >>>
    connect(tableTree, &QTreeWidget::itemClicked,
            this, &TableEditor::onSidebarItemClicked,
            Qt::UniqueConnection);
    
    connect(tableTree, &QTreeWidget::customContextMenuRequested,
            this, &TableEditor::showTableContextMenu);

    leftPanelLayout->addStretch();

    connect(newTableBtn, &QPushButton::clicked, this, &TableEditor::onNewTableClicked);
}


void TableEditor::updateTableList()
{
    if (tableListSection) {
        leftPanelLayout->removeWidget(tableListSection);
        tableListSection->deleteLater();
        tableListSection = nullptr;
    }

    tableListSection = new QWidget();
    QVBoxLayout *tableListLayout = new QVBoxLayout(tableListSection);
    tableListLayout->setContentsMargins(0, 0, 0, 0);
    tableListLayout->setSpacing(8);
    
    // Search and filter header
    QWidget *searchHeader = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchHeader);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    
    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Buscar tablas");
    searchBox->setFont(QFont("Inter", 12));
    searchBox->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 6px 10px;"
            "font-size: 13px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
    );
    
    searchLayout->addWidget(searchBox);
    tableListLayout->addWidget(searchHeader);
    
    // Tables tree
    tableTree = new QTreeWidget();
    tableTree->setHeaderHidden(true);
    tableTree->setRootIsDecorated(false);
    tableTree->setContextMenuPolicy(Qt::CustomContextMenu);
    tableTree->setStyleSheet(
        "QTreeWidget {"
            "background-color: transparent;"
            "border: none;"
            "outline: none;"
            "font-family: 'Inter';"
            "font-size: 13px;"
        "}"
        "QTreeWidget::item {"
            "height: 32px;"
            "padding: 4px 8px;"
            "border-radius: 6px;"
            "color: #374151;"
        "}"
        "QTreeWidget::item:hover {"
            "background-color: #F3F4F6;"
        "}"
        "QTreeWidget::item:selected {"
            "background-color: #EFF6FF;"
            "color: #1D4ED8;"
        "}"
    );
    
    tableListLayout->addWidget(tableTree);
    
    // Connect context menu for this new tableTree instance
    connect(tableTree, &QTreeWidget::customContextMenuRequested,
            this, &TableEditor::showTableContextMenu);
    
    leftPanelLayout->addWidget(tableListSection);
}

void TableEditor::createRightPanel()
{
    rightPanel = new QWidget();
    rightPanel->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
        "}"
    );
    
    rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);
    
    // Solo crear el área principal, sin toolbar duplicado
    createMainTableArea();
}

void TableEditor::createMainTableArea()
{
    mainContentArea = new QWidget();
    mainContentArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    
    mainContentLayout = new QVBoxLayout(mainContentArea);
    mainContentLayout->setContentsMargins(0, 0, 0, 0); // Sin márgenes para aprovechar todo el espacio
    mainContentLayout->setSpacing(0); // Sin espaciado
    
    // Create central card similar to the second image
    createTableCard = new ClickableWidget();
    createTableCard->setFixedSize(450, 100);
    createTableCard->setStyleSheet(
        "ClickableWidget {"
            "background-color: #1F2937;"
            "border-radius: 8px;"
            "border: 1px solid #374151;"
        "}"
        "ClickableWidget:hover {"
            "background-color: #374151;"
            "border-color: #4B5563;"
        "}"
    );
    createTableCard->setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout *cardLayout = new QHBoxLayout(createTableCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(12);
    cardLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // Icon (similar to the database icon in the image)
    QLabel *iconLabel = new QLabel("🗄️");
    iconLabel->setFont(QFont("Inter", 32));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("QLabel { color: #60A5FA; }");
    
    // Text content
    QWidget *textWidget = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);
    
    QLabel *cardTitle = new QLabel("Crear una tabla");
    cardTitle->setFont(QFont("Inter", 16, QFont::Bold));
    cardTitle->setStyleSheet("QLabel { color: #FFFFFF; }");
    
    QLabel *cardDesc = new QLabel("Diseña y crea una nueva tabla de base de datos");
    cardDesc->setFont(QFont("Inter", 12));
    cardDesc->setStyleSheet("QLabel { color: #9CA3AF; }");
    cardDesc->setWordWrap(true);
    
    textLayout->addWidget(cardTitle);
    textLayout->addWidget(cardDesc);
    
    cardLayout->addWidget(iconLabel);
    cardLayout->addWidget(textWidget);
    cardLayout->addStretch();
    
    // Center the card horizontally and vertically
    QVBoxLayout *centerVerticalLayout = new QVBoxLayout();
    centerVerticalLayout->addStretch();
    
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(createTableCard);
    centerLayout->addStretch();
    
    centerVerticalLayout->addLayout(centerLayout);
    centerVerticalLayout->addStretch();
    
    mainContentLayout->addLayout(centerVerticalLayout);
    
    // Connect card click event
    connect(createTableCard, &ClickableWidget::clicked, this, &TableEditor::onNewTableClicked);
    
    rightPanelLayout->addWidget(mainContentArea);
}

void TableEditor::createTableCreationPanel()
{
    createTablePanel = new QWidget(this);
    createTablePanel->setFixedWidth(240); // Mucho más pequeño
    createTablePanel->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-left: 1px solid #E5E7EB;"
        "}"
    );
    
    createTablePanelLayout = new QVBoxLayout(createTablePanel);
    createTablePanelLayout->setContentsMargins(20, 20, 20, 20); // Márgenes más pequeños
    createTablePanelLayout->setSpacing(20); // Espaciado más compacto
    
    // Header section - Solo título
    QLabel *titleLabel = new QLabel("Nueva Tabla");
    titleLabel->setFont(QFont("Inter", 20, QFont::Bold));
    titleLabel->setStyleSheet("QLabel { color: #111827; }");
    createTablePanelLayout->addWidget(titleLabel);
    
    // Table name input
    QWidget *nameWidget = new QWidget();
    QVBoxLayout *nameLayout = new QVBoxLayout(nameWidget);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(8);
    
    QLabel *nameLabel = new QLabel("Nombre de la tabla");
    nameLabel->setFont(QFont("Inter", 14, QFont::Medium));
    nameLabel->setStyleSheet("QLabel { color: #374151; }");
    
    tableNameInput = new QLineEdit();
    tableNameInput->setObjectName("tableNameInput");
    tableNameInput->setPlaceholderText("Ingresa el nombre de la tabla");
    tableNameInput->setFont(QFont("Inter", 14));
    tableNameInput->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 12px;"
            "font-size: 14px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
        "QLineEdit:focus {"
            "border-color: #A4373A;"
            "outline: none;"
        "}"
    );
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(tableNameInput);
    createTablePanelLayout->addWidget(nameWidget);
    
    // Añadir espacio flexible para empujar los botones hacia abajo
    createTablePanelLayout->addStretch();
    
    // Bottom buttons
    QWidget *bottomButtons = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomButtons);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(12);
    
    cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setFont(QFont("Inter", 10, QFont::Medium));
    cancelBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F3F4F6;"
            "color: #374151;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 12px 24px;"
        "}"
        "QPushButton:hover {"
            "background-color: #E5E7EB;"
        "}"
    );
    
    saveBtn = new QPushButton("Guardar");
    saveBtn->setFont(QFont("Inter", 10, QFont::Medium));
    saveBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #059669;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 6px;"
            "padding: 12px 24px;"
        "}"
        "QPushButton:hover {"
            "background-color: #047857;"
        "}"
    );
    
    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(saveBtn);
    createTablePanelLayout->addWidget(bottomButtons);
    
    // Initially hide the panel
    createTablePanel->setGeometry(width(), 0, 280, height());
    createTablePanel->hide();
    
    // Connect signals
    connect(cancelBtn, &QPushButton::clicked, this, &TableEditor::onCancelClicked);
    connect(saveBtn, &QPushButton::clicked, this, &TableEditor::onSaveClicked);
    
    // Setup animation
    panelAnimation = new QPropertyAnimation(createTablePanel, "geometry");
    panelAnimation->setDuration(300);
    panelAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TableEditor::styleComponents()
{
    updateTheme(isDarkTheme);
}

void TableEditor::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateLeftPanelTheme(isDark);
    updateRightPanelTheme(isDark);
    updateToolbarTheme(isDark);
    updateTableTheme(isDark);
}

void TableEditor::updateLeftPanelTheme(bool isDark)
{
    QString bgColor = isDark ? "#1F2937" : "#FAFAFA";
    
    leftPanel->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
        "}"
    ).arg(bgColor));
    
    // Update New Table button theme
    QString newTableBtnStyle = QString(
        "QPushButton {"
            "background-color: #A4373A;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 12px 16px;"
            "font-weight: 500;"
            "text-align: center;"
        "}"
        "QPushButton:hover {"
            "background-color: #8B2635;"
        "}"
        "QPushButton:pressed {"
            "background-color: #6D1D29;"
        "}"
    );
    newTableBtn->setStyleSheet(newTableBtnStyle);
    
    // Update search box and filter button
    updateSearchComponentsTheme(isDark);
    
    // Update tree widget theme
    updateTreeWidgetTheme(isDark);
    
    // Update empty state labels
    updateEmptyStateTheme(isDark);
}

void TableEditor::updateRightPanelTheme(bool isDark)
{
    QString bgColor = isDark ? "#111827" : "#FFFFFF";
    
    rightPanel->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
        "}"
    ).arg(bgColor));
    
    mainContentArea->setStyleSheet(QString("QWidget { background-color: %1; }").arg(bgColor));
    
    // Update text colors
    QString titleColor = isDark ? "#F9FAFB" : "#111827";
    QString descColor = isDark ? "#9CA3AF" : "#6B7280";
    
    // Update all labels in the main content area
    QList<QLabel*> allLabels = mainContentArea->findChildren<QLabel*>();
    for (QLabel* label : allLabels) {
        if (label->font().pointSize() >= 18) {
            // Title labels
            label->setStyleSheet(QString("QLabel { color: %1; margin-top: 32px; }").arg(titleColor));
        } else {
            // Description labels
            label->setStyleSheet(QString("QLabel { color: %1; }").arg(descColor));
        }
    }
}

void TableEditor::updateToolbarTheme(bool isDark)
{
    // Toolbar eliminado - ya no se necesita actualizar tema
    Q_UNUSED(isDark)
}

void TableEditor::updateTableTheme(bool isDark)
{
    // Actualizar tema de todas las vistas de tabla existentes
    for (auto it = tableViews.begin(); it != tableViews.end(); ++it) {
        if (it.value()) {
            it.value()->updateTheme(isDark);
        }
    }
    
    // Actualizar tema de todas las vistas de datos existentes
    for (auto it = tableDatas.begin(); it != tableDatas.end(); ++it) {
        if (it.value()) {
            it.value()->updateTheme(isDark);
        }
    }
}

void TableEditor::updateSearchComponentsTheme(bool isDark)
{
    QString inputBg = isDark ? "#374151" : "#FFFFFF";
    QString inputBorder = isDark ? "#4B5563" : "#D1D5DB";
    QString inputText = isDark ? "#F9FAFB" : "#111827";
    QString placeholderText = isDark ? "#9CA3AF" : "#6B7280";
    
    if (searchBox) {
        searchBox->setStyleSheet(QString(
            "QLineEdit {"
                "background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 8px;"
                "padding: 8px 12px;"
                "font-size: 14px;"
                "color: %3;"
            "}"
            "QLineEdit::placeholder {"
                "color: %4;"
            "}"
        ).arg(inputBg, inputBorder, inputText, placeholderText));
    }
    
    if (filterBtn) {
        QString filterBtnStyle = QString(
            "QPushButton {"
                "background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 8px;"
                "padding: 8px 12px;"
                "color: %3;"
            "}"
            "QPushButton:hover {"
                "background-color: %4;"
            "}"
        ).arg(inputBg, inputBorder, inputText, isDark ? "#4B5563" : "#F3F4F6");
        filterBtn->setStyleSheet(filterBtnStyle);
    }
}

void TableEditor::updateTreeWidgetTheme(bool isDark)
{
    if (!tableList) return;
    
    QString bgColor = isDark ? "#1F2937" : "#FAFAFA";
    QString textColor = isDark ? "#F9FAFB" : "#111827";
    QString selectionBg = isDark ? "#374151" : "#EFF6FF";
    QString selectionText = isDark ? "#F9FAFB" : "#1D4ED8";
    
    tableList->setStyleSheet(QString(
        "QTreeWidget {"
            "background-color: %1;"
            "border: none;"
            "outline: none;"
            "color: %2;"
        "}"
        "QTreeWidget::item {"
            "height: 32px;"
            "padding: 4px 8px;"
            "border-radius: 6px;"
            "margin: 1px 4px;"
        "}"
        "QTreeWidget::item:hover {"
            "background-color: %3;"
        "}"
        "QTreeWidget::item:selected {"
            "background-color: %3;"
            "color: %4;"
        "}"
    ).arg(bgColor, textColor, selectionBg, selectionText));
}

void TableEditor::updateEmptyStateTheme(bool isDark)
{
    QString textColor = isDark ? "#9CA3AF" : "#6B7280";
    
    // Find and update empty state labels in the left panel
    QList<QLabel*> emptyLabels = leftPanel->findChildren<QLabel*>();
    for (QLabel* label : emptyLabels) {
        if (label->text().contains("Sin tablas") || label->text().contains("se listará aquí")) {
            label->setStyleSheet(QString("QLabel { color: %1; }").arg(textColor));
        }
    }
}

void TableEditor::onNewTableClicked()
{
    showCreateTablePanel();
}

void TableEditor::onCancelClicked()
{
    hideCreateTablePanel();
}

void TableEditor::onSaveClicked()
{
    if (!tableNameInput || tableNameInput->text().trimmed().isEmpty()) {
        showStyledMessageBox("Error", "Por favor ingresa un nombre para la tabla.");
        return;
    }

    QString tableName = tableNameInput->text().trimmed();
    
    // Validar que el nombre sea válido
    if (!isValidTableName(tableName)) {
        showStyledMessageBox("Nombre Inválido", 
            "El nombre de la tabla debe:\n"
            "• Contener solo letras, números, espacios y guiones bajos\n"
            "• No comenzar con un número\n"
            "• No estar vacío\n\n"
            "Por favor elige un nombre válido.");
        return;
    }
    
    // Validar que no exista una tabla con el mismo nombre (ignorando mayúsculas/minúsculas)
    QStringList existingTables = getCreatedTables();
    for (const QString &existingTable : existingTables) {
        if (existingTable.toLower() == tableName.toLower()) {
            showStyledMessageBox("Nombre Duplicado", 
                QString("Ya existe una tabla con el nombre '%1'.\n\nPor favor elige un nombre diferente.").arg(existingTable));
            return; // No cerrar el panel, permitir al usuario cambiar el nombre
        }
    }

    hideCreateTablePanel();

    // Agregar la tabla al sidebar
    addTableToSidebar(tableName);
    // Emit signal that a new table was created
    emit tableCreated(tableName);

    showTableView(tableName);
    tableNameInput->clear();
}



void TableEditor::showCreateTablePanel()
{
    if (!createTablePanel) return;
    
    createTablePanel->setVisible(true);
    createTablePanel->raise();
    
    // Animation to slide in from right
    QPropertyAnimation *animation = new QPropertyAnimation(createTablePanel, "geometry");
    animation->setDuration(400);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    QRect startRect = createTablePanel->geometry();
    startRect.moveLeft(this->width());
    createTablePanel->setGeometry(startRect);
    
    QRect endRect(this->width() - 300, 0, 280, this->height()); // Posición más a la izquierda y más pequeño
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    
    connect(animation, &QPropertyAnimation::finished, [this]() {
        // Focus on table name input after animation
        if (tableNameInput) {
            tableNameInput->setFocus();
        }
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TableEditor::hideCreateTablePanel()
{
    if (!createTablePanel) return;
    
    // Animation to slide out to right
    QPropertyAnimation *animation = new QPropertyAnimation(createTablePanel, "geometry");
    animation->setDuration(400);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    QRect startRect = createTablePanel->geometry();
    QRect endRect = startRect;
    endRect.moveLeft(this->width());
    
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    
    connect(animation, &QPropertyAnimation::finished, [this]() {
        createTablePanel->setVisible(false);
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TableEditor::showTableView(const QString &tableName)
{
    currentTableName = tableName;

    // 1) Crear o recuperar el TableView de cache
    TableView *view = nullptr;
    if (tableViews.contains(tableName)) {
        view = tableViews.value(tableName);
    } else {
        view = new TableView(this);
        view->setTableName(tableName);
        view->updateTheme(isDarkTheme);
        view->setProperty("tableName", tableName);

        // Conexiones SOLO al crearlo (UniqueConnection por seguridad)
        connect(view, &TableView::switchToDataView, this, [this]() {
            switchToDataView();
        }, Qt::UniqueConnection);

        connect(view, &TableView::tableDesignChanged, this,
                [this, tableName](const QStringList &fieldNames, const QStringList &fieldTypes) {
                    // Guardar diseño en "arreglos" por tabla
                    TableDesignData &d = tableDesigns[tableName];
                    d.fieldNames = fieldNames;
                    d.fieldTypes = fieldTypes;
                    // Si existe su TableData, sincronizar
                    if (tableDatas.contains(tableName) && tableDatas.value(tableName)) {
                        // Obtener el índice de Primary Key del TableView actual
                        int primaryKeyIndex = -1;
                        if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                            primaryKeyIndex = tableViews.value(tableName)->getPrimaryKeyColumnIndex();
                        }
                        tableDatas.value(tableName)->setupDataView(fieldNames, fieldTypes, primaryKeyIndex);
                    }
                    // Emitir señal de que los campos cambiaron
                    emit tableFieldsChanged(tableName);
                }, Qt::UniqueConnection);

        // Conectar señal específica para formatos de moneda
        connect(view, &TableView::tableDesignChangedWithFormats, this,
                [this, tableName](const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats) {
                    // Guardar diseño en "arreglos" por tabla
                    TableDesignData &d = tableDesigns[tableName];
                    d.fieldNames = fieldNames;
                    d.fieldTypes = fieldTypes;
                    // Si existe su TableData, sincronizar con formatos de moneda
                    if (tableDatas.contains(tableName) && tableDatas.value(tableName)) {
                        // Obtener el índice de Primary Key del TableView actual
                        int primaryKeyIndex = -1;
                        if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                            primaryKeyIndex = tableViews.value(tableName)->getPrimaryKeyColumnIndex();
                        }
                        tableDatas.value(tableName)->setupDataViewWithFormats(fieldNames, fieldTypes, currencyFormats, primaryKeyIndex);
                    }
                    // Emitir señal de que los campos cambiaron
                    emit tableFieldsChanged(tableName);
                }, Qt::UniqueConnection);

        // Conectar señal específica para formatos de moneda y decimales de millares
        connect(view, &TableView::tableDesignChangedWithFormatsAndDecimals, this,
                [this, tableName](const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals) {
                    // Guardar diseño en "arreglos" por tabla
                    TableDesignData &d = tableDesigns[tableName];
                    d.fieldNames = fieldNames;
                    d.fieldTypes = fieldTypes;
                    // Si existe su TableData, sincronizar con formatos de moneda y decimales
                    if (tableDatas.contains(tableName) && tableDatas.value(tableName)) {
                        // Obtener el índice de Primary Key del TableView actual
                        int primaryKeyIndex = -1;
                        if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                            primaryKeyIndex = tableViews.value(tableName)->getPrimaryKeyColumnIndex();
                        }
                        tableDatas.value(tableName)->setupDataViewWithFormatsAndDecimals(fieldNames, fieldTypes, currencyFormats, millaresDecimals, primaryKeyIndex);
                    }
                    // Emitir señal de que los campos cambiaron
                    emit tableFieldsChanged(tableName);
                }, Qt::UniqueConnection);
        
        // Conectar señal para Foreign Key eliminada
        connect(view, &TableView::foreignKeyRemoved, this,
                [this](const QString &tableName, const QString &fieldName) {
                    qDebug() << "DEBUG: Foreign Key eliminada en tabla:" << tableName << "campo:" << fieldName;
                    // Emitir señal para notificar a RelationshipsView
                    emit foreignKeyRemoved(tableName, fieldName);
                }, Qt::UniqueConnection);

        // Conectar señal para validar campos únicos
        connect(view, &TableView::checkUniqueFieldDuplicates, this,
                [this, tableName](const QString &fieldName, int columnIndex) {
                    bool hasDuplicates = false;
                    
                    // Verificar si existe la instancia de TableData para esta tabla
                    if (tableDatas.contains(tableName) && tableDatas.value(tableName)) {
                        hasDuplicates = tableDatas.value(tableName)->hasColumnDuplicates(columnIndex);
                    }
                    
                    // Responder al TableView con el resultado de la validación
                    if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                        tableViews.value(tableName)->setUniqueValidationResult(fieldName, hasDuplicates);
                    }
                }, Qt::UniqueConnection);

        tableViews.insert(tableName, view);
    }

    // 2) Crear o recuperar el TableData de cache (aunque no se muestre todavía)
    TableData *data = nullptr;
    if (tableDatas.contains(tableName)) {
        data = tableDatas.value(tableName);
    } else {
        data = new TableData(this);
        data->setTableName(tableName);
        data->setProperty("tableName", tableName);

        connect(data, &TableData::switchToDesignView, this, [this]() {
            switchToDesignView();
        }, Qt::UniqueConnection);

        // Si ya hay diseño guardado, aplicarlo
        if (tableDesigns.contains(tableName)) {
            const auto &d = tableDesigns.value(tableName);
            // Obtener el índice de Primary Key del TableView actual
            int primaryKeyIndex = -1;
            if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                primaryKeyIndex = tableViews.value(tableName)->getPrimaryKeyColumnIndex();
            }
            data->setupDataView(d.fieldNames, d.fieldTypes, primaryKeyIndex);
        }
        tableDatas.insert(tableName, data);
    }

    // 3) Mostrar SOLO el view (diseño) ahora
    //    Limpia layout principal y añade el que corresponde (sin borrar caches)
    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(view);
    view->show();
}


void TableEditor::addTableToSidebar(const QString &tableName)
{
    // Create new tree widget item for the table
    auto *tableItem = new QTreeWidgetItem(tableTree);
    
    // Create custom widget for this table item
    TableItemWidget *itemWidget = new TableItemWidget(tableName, this);
    
    // Connect the custom widget signals
    connect(itemWidget, &TableItemWidget::tableClicked, this, [this](const QString &name) {
        showTableView(name);
    });
    
    connect(itemWidget, &TableItemWidget::optionsClicked, this, 
            &TableEditor::showTableOptionsMenu);
    
    // Set the custom widget as the tree item widget
    tableTree->setItemWidget(tableItem, 0, itemWidget);
    
    // Set item data for identification
    tableItem->setData(0, Qt::UserRole, tableName);
}

void TableEditor::showTableDataView(const QString &tableName)
{
    currentTableName = tableName;

    // Asegura que existen en cache
    if (!tableViews.contains(tableName)) {
        tableViews.insert(tableName, new TableView(this));
        tableViews[tableName]->setTableName(tableName);
        tableViews[tableName]->updateTheme(isDarkTheme);
        connect(tableViews[tableName], &TableView::switchToDataView, this, [this]() {
            switchToDataView();
        }, Qt::UniqueConnection);
    }
    if (!tableDatas.contains(tableName)) {
        tableDatas.insert(tableName, new TableData(this));
        tableDatas[tableName]->setTableName(tableName);
        connect(tableDatas[tableName], &TableData::switchToDesignView, this, [this]() {
            switchToDesignView();
        }, Qt::UniqueConnection);

        if (tableDesigns.contains(tableName)) {
            const auto &d = tableDesigns.value(tableName);
            // Obtener el índice de Primary Key del TableView actual
            int primaryKeyIndex = -1;
            if (tableViews.contains(tableName) && tableViews.value(tableName)) {
                primaryKeyIndex = tableViews.value(tableName)->getPrimaryKeyColumnIndex();
            }
            tableDatas[tableName]->setupDataView(d.fieldNames, d.fieldTypes, primaryKeyIndex);
        }
    }

    // Mostrar solo Data
    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(tableDatas.value(tableName));
    tableDatas.value(tableName)->show();
}

void TableEditor::onDeleteColumnClicked()
{
    // This function is called from the delete button in createColumnRow
    // The actual deletion is handled in the lambda in createColumnRow
    qDebug() << "Column delete requested";
}

void TableEditor::onTableSelected()
{
    qDebug() << "Table selected";
    // Implementación básica para evitar error de linking
}

void TableEditor::onCreateTableClicked()
{
    qDebug() << "Create table clicked";
    // Implementación básica para evitar error de linking
}

void TableEditor::animateCreateTablePanel()
{
    qDebug() << "Animate create table panel";
    // Implementación básica para evitar error de linking
}

void TableEditor::switchToDataView()
{
    if (!tableDatas.contains(currentTableName)) return;
    TableData *data = tableDatas.value(currentTableName);
    if (!data) return;

    // Obtener formatos de moneda actualizados de la vista de diseño antes de mostrar vista de datos
    if (tableViews.contains(currentTableName)) {
        TableView *view = tableViews.value(currentTableName);
        if (view) {
            QStringList fieldNames = view->getCurrentFieldNames();
            QStringList fieldTypes = view->getCurrentFieldTypes();
            QStringList currencyFormats = view->getCurrentCurrencyFormats();
            QStringList millaresDecimals = view->getCurrentMillaresDecimals();
            QStringList textSizes = view->getCurrentTextSizes();
            
            qDebug() << "DEBUG: switchToDataView - Actualizando con formatos:";
            qDebug() << "DEBUG: fieldNames:" << fieldNames;
            qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
            qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
            qDebug() << "DEBUG: millaresDecimals:" << millaresDecimals;
            qDebug() << "DEBUG: textSizes:" << textSizes;
            
            // Obtener el índice de Primary Key
            int primaryKeyIndex = view->getPrimaryKeyColumnIndex();
            qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyIndex;
            
            // Actualizar la vista de datos con todos los formatos más recientes
            data->setupDataViewWithTextSizes(fieldNames, fieldTypes, currencyFormats, millaresDecimals, textSizes, primaryKeyIndex);
        }
    }

    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(data);
    data->show();
}

void TableEditor::switchToDesignView()
{
    if (!tableViews.contains(currentTableName)) return;
    TableView *view = tableViews.value(currentTableName);
    if (!view) return;

    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(view);
    view->show();
}

void TableEditor::onSidebarItemClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (!item) return;
    // Get table name from UserRole data instead of text
    const QString selectedTableName = item->data(0, Qt::UserRole).toString();
    if (!selectedTableName.isEmpty()) {
        showTableView(selectedTableName);   // o showTableDataView si quieres abrir en datos
    }
}

QStringList TableEditor::getCreatedTables() const
{
    QStringList tables;
    if (tableTree) {
        for (int i = 0; i < tableTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = tableTree->topLevelItem(i);
            if (item) {
                QString tableName = item->data(0, Qt::UserRole).toString();
                if (!tableName.isEmpty()) {
                    tables.append(tableName);
                }
            }
        }
    }
    return tables;
}

QStringList TableEditor::getTableFields(const QString &tableName) const
{
    if (tableDesigns.contains(tableName)) {
        return tableDesigns.value(tableName).fieldNames;
    }
    return QStringList();
}

QStringList TableEditor::getTableFieldsWithKeys(const QString &tableName) const {
    // Buscar la TableView correspondiente en el mapa tableViews
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            // Usar el método que incluye las llaves
            QStringList allFieldNames = tableView->getAllFieldNames();
            return allFieldNames;
        }
    }
    
    // Si no encontramos la TableView, retornar los campos sin llaves
    return getTableFields(tableName);
}

QStringList TableEditor::getTableForeignKeys(const QString &tableName) const {
    QStringList foreignKeys;
    
    // Buscar la TableView correspondiente en el mapa tableViews
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            // Obtener todos los nombres de campos con sus iconos
            QStringList allFieldNames = tableView->getAllFieldNames();
            
            // Filtrar solo los que tienen el icono de Foreign Key (🔗)
            for (const QString &fieldName : allFieldNames) {
                if (fieldName.startsWith("🔗 ")) {
                    // Remover el icono para obtener solo el nombre del campo
                    QString cleanFieldName = fieldName.mid(3); // Remover "🔗 "
                    foreignKeys.append(cleanFieldName);
                }
            }
        }
    }
    
    return foreignKeys;
}

QStringList TableEditor::getTablePrimaryKeys(const QString &tableName) const {
    QStringList primaryKeys;
    
    // Buscar la TableView correspondiente en el mapa tableViews
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            // Obtener todos los nombres de campos con sus iconos
            QStringList allFieldNames = tableView->getAllFieldNames();
            
            // Filtrar solo los que tienen el icono de Primary Key (🔑)
            for (const QString &fieldName : allFieldNames) {
                if (fieldName.startsWith("🔑 ")) {
                    // Remover el icono para obtener solo el nombre del campo
                    QString cleanFieldName = fieldName.mid(3); // Remover "🔑 "
                    primaryKeys.append(cleanFieldName);
                }
            }
        }
    }
    
    return primaryKeys;
}

QStringList TableEditor::getTablePrimaryAndForeignKeys(const QString &tableName) const {
    QStringList primaryAndForeignKeys;
    
    // Buscar la TableView correspondiente en el mapa tableViews
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            // Obtener todos los nombres de campos con sus iconos
            QStringList allFieldNames = tableView->getAllFieldNames();
            
            // Filtrar solo los que tienen AMBOS iconos: Primary Key (🔑) Y Foreign Key (🔗)
            for (const QString &fieldName : allFieldNames) {
                if (fieldName.contains("🔑") && fieldName.contains("🔗")) {
                    // Extraer el nombre del campo sin iconos
                    QString cleanFieldName = fieldName;
                    cleanFieldName.remove("🔑 ");
                    cleanFieldName.remove("🔗 ");
                    cleanFieldName = cleanFieldName.trimmed();
                    primaryAndForeignKeys.append(cleanFieldName);
                }
            }
        }
    }
    
    return primaryAndForeignKeys;
}

QString TableEditor::getFieldType(const QString &tableName, const QString &fieldName) const {
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            QStringList fieldNames = tableView->getCurrentFieldNames();
            QStringList fieldTypes = tableView->getCurrentFieldTypes();
            
            // Buscar el índice del campo (limpiando iconos si es necesario)
            for (int i = 0; i < fieldNames.size(); ++i) {
                QString cleanFieldName = fieldNames[i];
                cleanFieldName.remove("🔑 ");
                cleanFieldName.remove("🔗 ");
                cleanFieldName = cleanFieldName.trimmed();
                
                if (cleanFieldName == fieldName && i < fieldTypes.size()) {
                    return fieldTypes[i];
                }
            }
        }
    }
    return QString(); // Retorna cadena vacía si no encuentra el campo
}

QStringList TableEditor::getTableFieldTypes(const QString &tableName) const {
    if (tableViews.contains(tableName)) {
        TableView* tableView = tableViews.value(tableName);
        if (tableView) {
            return tableView->getCurrentFieldTypes();
        }
    }
    return QStringList(); // Retorna lista vacía si no encuentra la tabla
}

void TableEditor::showTableContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = tableTree->itemAt(pos);
    if (!item) return;

    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 8px;"
            "padding: 6px;"
            "font-family: 'Inter';"
            "font-size: 13px;"
        "}"
        "QMenu::item {"
            "padding: 8px 12px;"
            "margin: 2px;"
            "border-radius: 6px;"
            "color: #374151;"
        "}"
        "QMenu::item:selected {"
            "background-color: #FEE2E2;"
            "color: #DC2626;"
        "}"
    );
    
    // Add delete action
    QAction *deleteAction = new QAction("🗑️ Eliminar Tabla", &contextMenu);
    
    connect(deleteAction, &QAction::triggered, this, [this, item]() {
        QString tableName = item->text(0);
        deleteTable(tableName);
    });
    
    contextMenu.addAction(deleteAction);
    
    // Show context menu at the clicked position
    contextMenu.exec(tableTree->mapToGlobal(pos));
}

void TableEditor::onDeleteTableClicked()
{
    // This can be used if you want to add a delete button elsewhere
    if (!tableTree->currentItem()) return;
    
    QString tableName = tableTree->currentItem()->text(0);
    deleteTable(tableName);
}

void TableEditor::deleteTable(const QString &tableName)
{
    // Show confirmation dialog
    QDialog dlg(this);
    dlg.setWindowTitle("Confirmar Eliminación");
    dlg.setModal(true);
    dlg.setSizeGripEnabled(false);

    // Layout principal
    QHBoxLayout *row = new QHBoxLayout;
    QVBoxLayout *col = new QVBoxLayout(&dlg);
    col->setContentsMargins(16,16,16,16);
    col->setSpacing(12);

    // Icono estilo "pregunta"
    QLabel *iconLbl = new QLabel;
    QPixmap px = style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(40, 40);
    iconLbl->setPixmap(px);
    iconLbl->setFixedSize(44,44);
    iconLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Texto
    QLabel *txt = new QLabel(
        QString("¿Está seguro de que desea eliminar la tabla '%1'?\n\n"
                "Esta acción no se puede deshacer.").arg(tableName));
    txt->setWordWrap(true);

    // Armar fila icono + texto
    row->addWidget(iconLbl, 0, Qt::AlignTop);
    row->addWidget(txt, 1);

    // Botonera
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    QPushButton *yesBtn = box->button(QDialogButtonBox::Yes);
    QPushButton *noBtn  = box->button(QDialogButtonBox::No);
    yesBtn->setText("Sí");
    noBtn->setText("No");
    box->setCenterButtons(false);

    // Estilos (BOTONES TEXTO NEGRO)
    dlg.setStyleSheet(
        "QDialog { background:#FFFFFF; }"
        "QLabel { color:#111827; font-family:'Inter'; font-size:14px; }"
        "QDialogButtonBox QPushButton {"
        "background:#F3F4F6; border:1px solid #D1D5DB; border-radius:6px;"
        "color:#111827; font-family:'Inter'; font-size:13px; font-weight:500;"
        "padding:8px 16px; min-width:80px; }"
        "QDialogButtonBox QPushButton:hover { background:#E5E7EB; }"
        "QDialogButtonBox QPushButton:pressed { background:#D1D5DB; }"
        "QDialogButtonBox QPushButton:disabled { background:#F9FAFB; color:#9CA3AF; border:1px solid #E5E7EB; }"
        );

    // Conexiones aceptar/cancelar
    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    col->addLayout(row);
    col->addWidget(box, 0, Qt::AlignRight);

    if (dlg.exec() != QDialog::Accepted) {
        return; // usuario canceló
    }
    
    // Remove from table tree
    for (int i = 0; i < tableTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tableTree->topLevelItem(i);
        if (item && item->data(0, Qt::UserRole).toString() == tableName) {
            delete tableTree->takeTopLevelItem(i);
            break;
        }
    }
    
    // Clean up table data
    if (tableViews.contains(tableName)) {
        TableView *tableView = tableViews.take(tableName);
        if (tableView) {
            tableView->deleteLater();
        }
    }
    
    if (tableDatas.contains(tableName)) {
        TableData *tableData = tableDatas.take(tableName);
        if (tableData) {
            tableData->deleteLater();
        }
    }
    
    // Remove table design data
    if (tableDesigns.contains(tableName)) {
        tableDesigns.remove(tableName);
    }
    
    // If the deleted table was currently displayed, show welcome content
    if (currentTableName == tableName) {
        currentTableName.clear();
        currentTableView = nullptr;
        currentTableData = nullptr;
        showWelcomeContent();
    }
    
    // Emit signal to notify other components
    emit tableDeleted(tableName);
    
    qDebug() << "Table deleted successfully:" << tableName;
}

void TableEditor::showTableOptionsMenu(const QString &tableName, const QPoint &pos)
{
    QMenu optionsMenu(this);
    optionsMenu.setStyleSheet(
        "QMenu {"
            "background-color: #FFFFFF;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 8px;"
            "padding: 8px 0px;"
            "min-width: 180px;"
            "box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1);"
        "}"
        "QMenu::item {"
            "padding: 12px 16px;"
            "font-family: 'Inter';"
            "font-size: 14px;"
            "color: #374151;"
            "margin: 0px 4px;"
            "border-radius: 6px;"
        "}"
        "QMenu::item:selected {"
            "background-color: #F9FAFB;"
            "color: #111827;"
        "}"
        "QMenu::separator {"
            "height: 1px;"
            "background-color: #F3F4F6;"
            "margin: 8px 12px;"
        "}"
    );
    
    // Add edit name action
    QAction *editAction = new QAction("Editar nombre", &optionsMenu);
    connect(editAction, &QAction::triggered, this, [this, tableName]() {
        // ==== Dialogo personalizado (no nativo) ====
        QDialog dlg(this);
        dlg.setWindowTitle("Editar Nombre de Tabla");
        dlg.setModal(true);
        dlg.setSizeGripEnabled(false);

        QVBoxLayout *v = new QVBoxLayout(&dlg);
        v->setContentsMargins(16,16,16,16);
        v->setSpacing(12);

        QLabel *lbl = new QLabel("Nuevo nombre:");
        QLineEdit *edit = new QLineEdit(tableName);
        edit->setPlaceholderText("Ingresa el nuevo nombre");

        // Botonera OK/Cancel
        QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        QPushButton *okBtn = box->button(QDialogButtonBox::Ok);
        QPushButton *cancelBtn = box->button(QDialogButtonBox::Cancel);

        // Estilos: botones con TEXTO NEGRO
        dlg.setStyleSheet(
            "QDialog { background:#FFFFFF; }"
            "QLabel { color:#111827; font-family:'Inter'; font-size:14px; }"
            "QLineEdit { background:#FFFFFF; border:1px solid #D1D5DB; border-radius:6px; padding:10px; font-size:14px; color:#111827; }"
            "QLineEdit:focus { border-color:#A4373A; }"
            "QDialogButtonBox QPushButton {"
            "background:#F3F4F6; border:1px solid #D1D5DB; border-radius:6px;"
            "color:#111827; font-family:'Inter'; font-size:13px; font-weight:500; padding:8px 16px; }"
            "QDialogButtonBox QPushButton:hover { background:#E5E7EB; }"
            "QDialogButtonBox QPushButton:pressed { background:#D1D5DB; }"
            "QDialogButtonBox QPushButton:disabled { background:#F9FAFB; color:#9CA3AF; border:1px solid #E5E7EB; }"
            );

        v->addWidget(lbl);
        v->addWidget(edit);
        v->addWidget(box);

        // Validación igual a tu isValidTableName() + evitar duplicados
        auto validate = [this, edit, tableName]() {
            const QString newName = edit->text().trimmed();
            if (!isValidTableName(newName)) return false;
            // no permitir duplicados (ignorando mayúsculas)
            for (const auto &t : getCreatedTables())
                if (t.compare(newName, Qt::CaseInsensitive) == 0 && t != tableName) return false;
            return true;
        };

        okBtn->setEnabled(validate());

        connect(edit, &QLineEdit::textChanged, &dlg, [okBtn, validate]() {
            okBtn->setEnabled(validate());
        });
        connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            const QString newName = edit->text().trimmed();
            if (newName != tableName) {
                if (!isValidTableName(newName)) {
                    showStyledMessageBox("Nombre Inválido",
                                         "El nombre de la tabla debe:\n"
                                         "• Contener solo letras, números, espacios y guiones bajos\n"
                                         "• No comenzar con un número\n"
                                         "• No estar vacío\n\n"
                                         "Por favor elige un nombre válido.");
                    return;
                }
                // Duplicados otra vez por seguridad
                for (const auto &t : getCreatedTables()) {
                    if (t.compare(newName, Qt::CaseInsensitive) == 0) {
                        showStyledMessageBox("Nombre Duplicado",
                                             QString("Ya existe una tabla con el nombre '%1'.\n\nPor favor elige un nombre diferente.").arg(t));
                        return;
                    }
                }
                renameTable(tableName, newName);
            }
        }
    });
    
    // Add separator
    optionsMenu.addAction(editAction);
    optionsMenu.addSeparator();
    
    // Add delete action
    QAction *deleteAction = new QAction("🗑️  Eliminar tabla", &optionsMenu);
    deleteAction->setProperty("destructive", true);
    connect(deleteAction, &QAction::triggered, this, [this, tableName]() {
        deleteTable(tableName);
    });
    
    optionsMenu.addAction(deleteAction);
    
    // Style the destructive action differently
    deleteAction->setToolTip("Eliminar esta tabla permanentemente");
    
    // Show menu at the specified position
    optionsMenu.exec(pos);
}

void TableEditor::showWelcomeContent()
{
    // Clear the current content area
    QList<QWidget*> widgets = mainContentArea->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *widget : widgets) {
        if (widget != createTableCard) {
            widget->hide();
            mainContentLayout->removeWidget(widget);
        }
    }
    
    // Show the create table card if it's not already visible
    if (createTableCard && !createTableCard->isVisible()) {
        // Re-add the card to the layout if needed
        QVBoxLayout *centerVerticalLayout = new QVBoxLayout();
        centerVerticalLayout->addStretch();
        
        QHBoxLayout *centerLayout = new QHBoxLayout();
        centerLayout->addStretch();
        centerLayout->addWidget(createTableCard);
        centerLayout->addStretch();
        
        centerVerticalLayout->addLayout(centerLayout);
        centerVerticalLayout->addStretch();
        
        mainContentLayout->addLayout(centerVerticalLayout);
        createTableCard->show();
    }
}

// TableItemWidget implementation
TableItemWidget::TableItemWidget(const QString &tableName, QWidget *parent)
    : QWidget(parent), tableName(tableName)
{
    setFixedHeight(36);
    
    // Add hover effect to the entire widget
    setStyleSheet(
        "TableItemWidget {"
            "background-color: transparent;"
            "border-radius: 6px;"
        "}"
        "TableItemWidget:hover {"
            "background-color: #F9FAFB;"
        "}"
    );
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 6, 8, 6);
    layout->setSpacing(8);
    
    // Icon
    QLabel *iconLabel = new QLabel("�");
    iconLabel->setFixedSize(18, 18);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFont(QFont("Inter", 14));
    
    // Table name label (clickable)
    QLabel *nameLabel = new QLabel(tableName);
    nameLabel->setFont(QFont("Inter", 13, QFont::Medium));
    nameLabel->setStyleSheet(
        "QLabel { "
            "color: #374151; "
            "padding: 2px 0px;"
        "}"
    );
    nameLabel->setCursor(Qt::PointingHandCursor);
    
    // Menu button (3 dots)
    menuButton = new QPushButton("•••");
    menuButton->setFixedSize(28, 24);
    menuButton->setFont(QFont("Inter", 12, QFont::Bold));
    menuButton->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: 1px solid transparent;"
            "color: #9CA3AF;"
            "border-radius: 6px;"
            "text-align: center;"
            "font-weight: bold;"
            "letter-spacing: 1px;"
            "padding: 2px;"
        "}"
        "QPushButton:hover {"
            "background-color: #F9FAFB;"
            "border-color: #E5E7EB;"
            "color: #6B7280;"
        "}"
        "QPushButton:pressed {"
            "background-color: #F3F4F6;"
            "border-color: #D1D5DB;"
            "color: #374151;"
        "}"
    );
    menuButton->setCursor(Qt::PointingHandCursor);
    menuButton->setToolTip("Opciones de tabla");
    
    layout->addWidget(iconLabel);
    layout->addWidget(nameLabel);
    layout->addStretch();
    layout->addWidget(menuButton);
    
    // Connect signals
    connect(menuButton, &QPushButton::clicked, this, [this, tableName]() {
        QPoint globalPos = menuButton->mapToGlobal(QPoint(menuButton->width() - 160, menuButton->height() + 2));
        emit optionsClicked(tableName, globalPos);
    });
    
    // Make the whole widget clickable (except menu button)
    this->setCursor(Qt::PointingHandCursor);
}

void TableItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if click was not on the menu button
        QPoint clickPos = event->pos();
        QRect menuButtonRect = menuButton->geometry();
        
        if (!menuButtonRect.contains(clickPos)) {
            emit tableClicked(tableName);
        }
    }
    QWidget::mousePressEvent(event);
}

void TableEditor::renameTable(const QString &oldName, const QString &newName)
{
    // Update the tree widget item
    for (int i = 0; i < tableTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tableTree->topLevelItem(i);
        if (item && item->data(0, Qt::UserRole).toString() == oldName) {
            // Get the custom widget and update it
            TableItemWidget *widget = qobject_cast<TableItemWidget*>(tableTree->itemWidget(item, 0));
            if (widget) {
                // Remove old widget and create new one with updated name
                tableTree->removeItemWidget(item, 0);
                TableItemWidget *newWidget = new TableItemWidget(newName, this);
                
                // Connect the new widget signals
                connect(newWidget, &TableItemWidget::tableClicked, this, [this](const QString &name) {
                    showTableView(name);
                });
                
                connect(newWidget, &TableItemWidget::optionsClicked, this, 
                        &TableEditor::showTableOptionsMenu);
                
                tableTree->setItemWidget(item, 0, newWidget);
                item->setData(0, Qt::UserRole, newName);
            }
            break;
        }
    }
    
    // Update table designs map
    if (tableDesigns.contains(oldName)) {
        TableDesignData designData = tableDesigns.take(oldName);
        tableDesigns.insert(newName, designData);
    }
    
    // Update table views map
    if (tableViews.contains(oldName)) {
        TableView *tableView = tableViews.take(oldName);
        if (tableView) {
            tableView->setTableName(newName);
            tableViews.insert(newName, tableView);
        }
    }
    
    // Update table datas map
    if (tableDatas.contains(oldName)) {
        TableData *tableData = tableDatas.take(oldName);
        if (tableData) {
            tableData->setTableName(newName);
            tableDatas.insert(newName, tableData);
        }
    }
    
    // Update current table name if it was the renamed table
    if (currentTableName == oldName) {
        currentTableName = newName;
    }
    
    // Emit signal to notify other components about the rename
    qDebug() << "DEBUG TableEditor: Emitiendo tableRenamed signal:" << oldName << "->" << newName;
    emit tableRenamed(oldName, newName);
    
    qDebug() << "Table renamed from" << oldName << "to" << newName;
}

void TableEditor::showStyledMessageBox(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.setModal(true);
    dlg.setSizeGripEnabled(false);

    // Layouts
    QVBoxLayout *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(12);

    QHBoxLayout *row = new QHBoxLayout;

    // Icono según 'icon'
    QLabel *iconLbl = new QLabel;
    QStyle::StandardPixmap sp = QStyle::SP_MessageBoxInformation;
    switch (icon) {
    case QMessageBox::Warning:  sp = QStyle::SP_MessageBoxWarning; break;
    case QMessageBox::Critical: sp = QStyle::SP_MessageBoxCritical; break;
    case QMessageBox::Question: sp = QStyle::SP_MessageBoxQuestion; break;
    case QMessageBox::Information:
    default: sp = QStyle::SP_MessageBoxInformation; break;
    }
    QPixmap px = style()->standardIcon(sp).pixmap(40, 40);
    iconLbl->setPixmap(px);
    iconLbl->setFixedSize(44,44);
    iconLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Texto
    QLabel *txt = new QLabel(message);
    txt->setWordWrap(true);

    row->addWidget(iconLbl, 0, Qt::AlignTop);
    row->addWidget(txt, 1);

    // Botón OK
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok);
    QPushButton *okBtn = box->button(QDialogButtonBox::Ok);
    okBtn->setText("OK");

    // Estilos (BOTONES TEXTO NEGRO)
    dlg.setStyleSheet(
        "QDialog { background:#FFFFFF; }"
        "QLabel { color:#111827; font-family:'Inter'; font-size:14px; }"
        "QDialogButtonBox QPushButton {"
        "background:#F3F4F6; border:1px solid #D1D5DB; border-radius:6px;"
        "color:#111827; font-family:'Inter'; font-size:13px; font-weight:500;"
        "padding:8px 16px; min-width:80px; }"
        "QDialogButtonBox QPushButton:hover { background:#E5E7EB; }"
        "QDialogButtonBox QPushButton:pressed { background:#D1D5DB; }"
        );

    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    root->addLayout(row);
    root->addWidget(box, 0, Qt::AlignRight);

    dlg.exec();
}

bool TableEditor::isValidTableName(const QString &name)
{
    // Verificar que el nombre no esté vacío
    if (name.trimmed().isEmpty()) {
        return false;
    }
    
    // Verificar que no contenga solo espacios
    QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty()) {
        return false;
    }
    
    // Verificar que no contenga caracteres especiales problemáticos
    // Permitir letras, números, guiones bajos y espacios
    QRegularExpression validChars("^[a-zA-Z0-9_\\s]+$");
    if (!validChars.match(trimmedName).hasMatch()) {
        return false;
    }
    
    // Verificar que no comience con número
    if (trimmedName.at(0).isDigit()) {
        return false;
    }
    
    return true;
}
