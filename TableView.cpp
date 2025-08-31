#include "TableView.h"

TableView::TableView(QWidget *parent)
    : QWidget(parent), isDarkTheme(false)
{
    setupUI();
    styleTable();
}

void TableView::setupUI()
{
    // Main layout
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create header with table name and toolbar
    createHeader();
    
    // Create main splitter for table and properties
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setHandleWidth(8); // Un poco más grande para facilitar el agarre
    mainSplitter->setChildrenCollapsible(true); // Permitir colapsar completamente
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
            "background-color: #E5E7EB;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 4px;"
            "margin: 2px;"
        "}"
        "QSplitter::handle:hover {"
            "background-color: #D1D5DB;"
            "border-color: #9CA3AF;"
        "}"
        "QSplitter::handle:pressed {"
            "background-color: #9CA3AF;"
        "}"
    );
    
    // Create table view area (top)
    createTableArea();
    
    // Create properties area (bottom) - initially hidden
    createPropertiesArea();
    propertiesArea->hide(); // Hide by default
    
    // Add areas to splitter
    mainSplitter->addWidget(tableArea);
    mainSplitter->addWidget(propertiesArea);
    
    // Set initial sizes - tabla más grande, propiedades más pequeñas
    mainSplitter->setSizes({500, 300}); // Tabla 500px, propiedades 300px cuando se muestra (increased from 150)
    
    // Add to main layout
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(mainSplitter);
    
    // Apply initial theme
    updateTheme(false);
}

void TableView::setTableName(const QString &tableName)
{
    currentTableName = tableName;
    tableNameLabel->setText(tableName);
}

void TableView::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Update widget background
    QString bgColor = isDark ? "#111827" : "#FFFFFF";
    QString propertiesBg = isDark ? "#1F2937" : "#F9FAFB";
    QString borderColor = isDark ? "#374151" : "#E5E7EB";
    
    setStyleSheet(QString("QWidget { background-color: %1; }").arg(bgColor));
    
    // Update header
    if (headerWidget) {
        headerWidget->setStyleSheet(QString("QWidget { background-color: %1; border-bottom: 1px solid %2; }").arg(bgColor, borderColor));
    }
    
    // Update properties area
    if (propertiesArea) {
        propertiesArea->setStyleSheet(QString("QWidget { background-color: %1; border-top: 1px solid %2; }").arg(propertiesBg, borderColor));
    }
    
    // Update table name label
    QString titleColor = isDark ? "#F9FAFB" : "#111827";
    if (tableNameLabel) {
        tableNameLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(titleColor));
    }
    
    // Update table style
    styleTable();
}

void TableView::styleTable()
{
    QString bgColor = isDarkTheme ? "#111827" : "#FFFFFF";
    QString borderColor = isDarkTheme ? "#374151" : "#E5E7EB";
    QString headerBgColor = isDarkTheme ? "#1F2937" : "#F9FAFB";
    QString textColor = isDarkTheme ? "#F9FAFB" : "#111827";
    QString headerTextColor = isDarkTheme ? "#F3F4F6" : "#374151";
    QString selectionColor = isDarkTheme ? "#374151" : "#EBF4FF";
    QString alternateRowColor = isDarkTheme ? "#1F2937" : "#F8FAFC";
    
    tableWidget->setStyleSheet(QString(
        "QTableWidget {"
            "background-color: %1;"
            "border: 1px solid %2;"
            "border-radius: 8px;"
            "gridline-color: %2;"
            "color: %3;"
            "selection-background-color: %6;"
            "alternate-background-color: %7;"
        "}"
        "QTableWidget::item {"
            "padding: 8px 12px;"
            "border-bottom: 1px solid %2;"
        "}"
        "QTableWidget::item:selected {"
            "background-color: %6;"
            "color: %3;"
        "}"
        "QHeaderView::section {"
            "background-color: %4;"
            "color: %5;"
            "padding: 12px 16px;"
            "border: none;"
            "border-bottom: 2px solid %2;"
            "border-right: 1px solid %2;"
            "font-weight: 600;"
        "}"
        "QHeaderView::section:horizontal {"
            "border-top-left-radius: 8px;"
            "border-top-right-radius: 8px;"
        "}"
        "QScrollBar:vertical {"
            "background-color: %1;"
            "width: 8px;"
            "border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
            "background-color: %2;"
            "border-radius: 4px;"
            "min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
            "background-color: %5;"
        "}"
        "QScrollBar:horizontal {"
            "background-color: %1;"
            "height: 8px;"
            "border-radius: 4px;"
        "}"
        "QScrollBar::handle:horizontal {"
            "background-color: %2;"
            "border-radius: 4px;"
            "min-width: 20px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
            "background-color: %5;"
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line {"
            "border: none;"
            "background: none;"
        "}"
    ).arg(bgColor)
     .arg(borderColor)
     .arg(textColor)
     .arg(headerBgColor)
     .arg(headerTextColor)
     .arg(selectionColor)
     .arg(alternateRowColor));
}

void TableView::createHeader()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(60); // Más pequeño
    headerWidget->setStyleSheet("QWidget { background-color: #FFFFFF; border-bottom: 1px solid #E5E7EB; }");
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 8, 16, 8); // Márgenes más pequeños
    headerLayout->setSpacing(12);
    
    // Table name
    tableNameLabel = new QLabel();
    tableNameLabel->setFont(QFont("Inter", 18, QFont::Bold)); // Fuente más pequeña
    tableNameLabel->setStyleSheet("QLabel { color: #111827; }");
    headerLayout->addWidget(tableNameLabel);
    
    // Toolbar with icons (moved next to table name)
    QWidget *toolbar = new QWidget();
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(6); // Espaciado más pequeño
    
    // Add common table operation buttons with red theme
    addToolbarButton(toolbarLayout, "➕", "Agregar fila");
    addToolbarButton(toolbarLayout, "✏️", "Editar");
    addToolbarButton(toolbarLayout, "🗑️", "Eliminar");
    addToolbarButton(toolbarLayout, "💾", "Guardar");
    addToolbarButton(toolbarLayout, "🔄", "Actualizar");
    addToolbarButton(toolbarLayout, "📋", "Copiar");
    addToolbarButton(toolbarLayout, "📄", "Exportar");
    
    headerLayout->addWidget(toolbar);
    headerLayout->addStretch(); // Push everything to the left
}

void TableView::addToolbarButton(QHBoxLayout *layout, const QString &icon, const QString &tooltip)
{
    QPushButton *btn = new QPushButton(icon);
    btn->setFixedSize(36, 36);
    btn->setToolTip("");  // Disable tooltip to prevent floating popups
    btn->setStyleSheet(
        "QPushButton {"
            "background-color: #FEF2F2;"
            "border: 1px solid #FCA5A5;"
            "border-radius: 8px;"
            "font-size: 16px;"
            "padding: 6px;"
            "color: #DC2626;"
        "}"
        "QPushButton:hover {"
            "background-color: #FEE2E2;"
            "border-color: #F87171;"
        "}"
        "QPushButton:pressed {"
            "background-color: #FECACA;"
            "border-color: #EF4444;"
        "}"
    );
    layout->addWidget(btn);
}

void TableView::createTableArea()
{
    tableArea = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tableArea);
    tableLayout->setContentsMargins(12, 6, 12, 6); // Márgenes más pequeños
    tableLayout->setSpacing(6); // Espaciado más pequeño
    
    // Crear botones de vista como pestañas pequeñas
    QWidget *viewButtonsWidget = new QWidget();
    QHBoxLayout *viewButtonsLayout = new QHBoxLayout(viewButtonsWidget);
    viewButtonsLayout->setContentsMargins(0, 0, 0, 0);
    viewButtonsLayout->setSpacing(4); // Espaciado pequeño entre botones
    
    // Botón Vista de Diseño (activo por defecto)
    designViewBtn = new QPushButton("Vista de Diseño");
    designViewBtn->setFixedHeight(28); // Altura pequeña
    designViewBtn->setFont(QFont("Inter", 11, QFont::Medium)); // Fuente pequeña
    designViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #EFF6FF;" // Fondo azul claro (activo)
            "color: #1D4ED8;" // Texto azul
            "border: 1px solid #BFDBFE;"
            "border-radius: 6px;"
            "padding: 4px 12px;" // Padding pequeño
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #DBEAFE;"
        "}"
    );
    
    // Botón Vista Editor
    editorViewBtn = new QPushButton("Vista Editor");
    editorViewBtn->setFixedHeight(28); // Altura pequeña
    editorViewBtn->setFont(QFont("Inter", 11, QFont::Medium)); // Fuente pequeña
    editorViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;" // Fondo gris claro (inactivo)
            "color: #6B7280;" // Texto gris
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;" // Padding pequeño
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    // Botón Vista Datos (opcional)
    dataViewBtn = new QPushButton("Vista Datos");
    dataViewBtn->setFixedHeight(28); // Altura pequeña
    dataViewBtn->setFont(QFont("Inter", 11, QFont::Medium)); // Fuente pequeña
    dataViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;" // Fondo gris claro (inactivo)
            "color: #6B7280;" // Texto gris
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;" // Padding pequeño
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    // Conectar señales de los botones
    connect(designViewBtn, &QPushButton::clicked, this, &TableView::onDesignViewClicked);
    connect(editorViewBtn, &QPushButton::clicked, this, &TableView::onEditorViewClicked);
    connect(dataViewBtn, &QPushButton::clicked, this, &TableView::onDataViewClicked);
    
    // Agregar botones al layout
    viewButtonsLayout->addWidget(designViewBtn);
    viewButtonsLayout->addWidget(editorViewBtn);
    viewButtonsLayout->addWidget(dataViewBtn);
    viewButtonsLayout->addStretch(); // Empujar botones a la izquierda
    
    tableLayout->addWidget(viewButtonsWidget);
    
    // Create table widget for field design
    tableWidget = new QTableWidget();
    
    // Set up table structure for field design (like Access design view)
    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(1); // Solo mostrar una fila inicialmente
    
    QStringList headers;
    headers << "Nombre del Campo" << "Tipo de Datos" << "Descripción (Opcional)";
    tableWidget->setHorizontalHeaderLabels(headers);
    
    // Set column widths - proporciones más elegantes
    tableWidget->setColumnWidth(0, 250); // Campo nombre un poco más ancho
    tableWidget->setColumnWidth(1, 180); // Tipo de datos un poco más ancho
    tableWidget->setColumnWidth(2, 400); // Descripción amplia para texto largo
    
    // Configure column resize behavior
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); // Campo nombre fijo
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed); // Tipo de datos fijo  
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // Descripción se expande
    
    // Enable horizontal scrolling
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Add default first field with "Id" as field name
    QTableWidgetItem *idItem = new QTableWidgetItem("Id");
    tableWidget->setItem(0, 0, idItem);
    
    QTableWidgetItem *typeItem = new QTableWidgetItem("int");
    tableWidget->setItem(0, 1, typeItem);
    
    QTableWidgetItem *descItem = new QTableWidgetItem("Identificador único");
    tableWidget->setItem(0, 2, descItem);
    
    // Connect signal para agregar filas automáticamente
    connect(tableWidget, &QTableWidget::itemChanged, this, &TableView::onTableItemChanged);
    
    // Set custom delegate for data type column (column 1)
    DataTypeDelegate *dataTypeDelegate = new DataTypeDelegate(this);
    tableWidget->setItemDelegateForColumn(1, dataTypeDelegate);
    
    // Set custom delegate for description column (column 2) 
    DescriptionDelegate *descriptionDelegate = new DescriptionDelegate(this);
    tableWidget->setItemDelegateForColumn(2, descriptionDelegate);
    
    // Configure table properties for design view
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setGridStyle(Qt::SolidLine);
    tableWidget->setShowGrid(true);
    
    // Style the table with clear borders and better editing visibility - Excel/Access-like appearance
    tableWidget->setStyleSheet(
        "QTableWidget {"
            "background-color: #FFFFFF;"
            "border: 2px solid #D1D5DB;" // Thicker border like Excel
            "border-radius: 4px;" // Smaller radius for more professional look
            "gridline-color: #E5E7EB;"
            "selection-background-color: #EBF4FF;"
            "alternate-background-color: #FAFBFC;"
            "font-family: 'Segoe UI', 'Inter';" // More Excel-like font
            "font-size: 13px;"
            "outline: none;" // Remove focus outline
        "}"
        "QTableWidget::item {"
            "padding: 12px 16px;" // More Excel-like padding
            "border-right: 1px solid #E5E7EB;"
            "border-bottom: 1px solid #E5E7EB;"
            "font-size: 13px;"
            "color: #1F2937;"
            "background-color: #FFFFFF;"
            "min-height: 35px;" // Excel-like row height
        "}"
        "QTableWidget::item:selected {"
            "background-color: #DBEAFE;" // Excel-like selection color
            "color: #1E40AF;"
            "border: 2px solid #3B82F6;" // Thicker selection border
        "}"
        "QTableWidget::item:focus {"
            "background-color: #FFFFFF;"
            "color: #1F2937;"
            "border: 3px solid #3B82F6;" // Thick focus border like Excel
            "border-radius: 2px;"
        "}"
        "QTableWidget::item:hover {"
            "background-color: #F8FAFC;" // Subtle hover effect
        "}"
        "QLineEdit {" // Styling for in-cell editing
            "background-color: #FFFFFF;"
            "color: #1F2937;"
            "font-size: 13px;"
            "font-family: 'Segoe UI', 'Inter';"
            "border: 3px solid #3B82F6;"
            "border-radius: 3px;"
            "padding: 8px 12px;"
            "min-height: 20px;"
        "}"
        "QTextEdit {" // Styling for multi-line cell editing
            "background-color: #FFFFFF;"
            "color: #1F2937;"
            "font-size: 13px;"
            "font-family: 'Segoe UI', 'Inter';"
            "border: 3px solid #3B82F6;"
            "border-radius: 3px;"
            "padding: 8px 12px;"
            "min-height: 60px;"
        "}"
        "QHeaderView::section {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F8FAFC, stop:1 #E2E8F0);" // Excel-like gradient
            "color: #475569;"
            "padding: 16px 20px;"
            "border: none;"
            "border-right: 1px solid #CBD5E1;"
            "border-bottom: 2px solid #CBD5E1;" // Thicker bottom border
            "font-weight: 700;" // Bolder text like Excel
            "font-size: 12px;" // Smaller header font
            "font-family: 'Segoe UI', 'Inter';"
            "text-transform: none;" // Remove uppercase transform
            "letter-spacing: 0em;"
            "min-height: 40px;" // Excel-like header height
        "}"
        "QHeaderView::section:first {"
            "border-top-left-radius: 4px;"
        "}"
        "QHeaderView::section:last {"
            "border-top-right-radius: 4px;"
            "border-right: none;"
        "}"
        "QHeaderView::section:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E2E8F0, stop:1 #CBD5E1);" // Darker on hover
        "}"
        "QHeaderView::section:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #CBD5E1, stop:1 #94A3B8);" // Even darker when pressed
        "}"
        // Excel-like scrollbars
        "QScrollBar:vertical {"
            "background-color: #F1F5F9;"
            "width: 16px;" // Wider scrollbar like Excel
            "border: 1px solid #E2E8F0;"
            "border-radius: 0px;" // No radius for Excel-like appearance
        "}"
        "QScrollBar::handle:vertical {"
            "background-color: #CBD5E1;"
            "border: 1px solid #94A3B8;"
            "border-radius: 0px;"
            "min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
            "background-color: #94A3B8;"
        "}"
        "QScrollBar::handle:vertical:pressed {"
            "background-color: #64748B;"
        "}"
        "QScrollBar:horizontal {"
            "background-color: #F1F5F9;"
            "height: 16px;" // Wider scrollbar like Excel
            "border: 1px solid #E2E8F0;"
            "border-radius: 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
            "background-color: #CBD5E1;"
            "border: 1px solid #94A3B8;"
            "border-radius: 0px;"
            "min-width: 30px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
            "background-color: #94A3B8;"
        "}"
        "QScrollBar::handle:horizontal:pressed {"
            "background-color: #64748B;"
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line {"
            "border: none;"
            "background: none;"
        "}"
        "QScrollBar::add-page, QScrollBar::sub-page {"
            "background: none;"
        "}"
    );
    
    // Set row height for Excel-like appearance
    tableWidget->verticalHeader()->setDefaultSectionSize(45); // Excel-like row height
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed); // Fixed height for consistency
    tableWidget->verticalHeader()->setMinimumSectionSize(45); // Minimum height
    
    tableWidget->verticalHeader()->setVisible(true);
    
    // Custom header for showing primary key icon in first row
    QStringList verticalHeaders;
    verticalHeaders << "🔑"; // Primary key icon for first row
    for (int i = 1; i < 20; i++) { // Add numbers for other rows
        verticalHeaders << QString::number(i + 1);
    }
    tableWidget->setVerticalHeaderLabels(verticalHeaders);
    
    tableWidget->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F8FAFC, stop:1 #E2E8F0);" // Excel-like gradient
            "color: #64748B;"
            "padding: 8px 12px;"
            "border: none;"
            "border-right: 2px solid #CBD5E1;" // Thicker border like Excel
            "border-bottom: 1px solid #E5E7EB;"
            "font-size: 12px;" // Smaller font like Excel
            "font-weight: 600;" // Bold like Excel
            "width: 45px;" // Wider like Excel
            "text-align: center;"
            "font-family: 'Segoe UI', 'Inter';"
        "}"
        "QHeaderView::section:first {" // Special styling for primary key row
            "font-size: 16px;"
            "color: #DC2626;" // Red color for primary key
            "font-weight: 700;"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FEF2F2, stop:1 #FEE2E2);"
        "}"
        "QHeaderView::section:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #E2E8F0, stop:1 #CBD5E1);" // Darker on hover
        "}"
        "QHeaderView::section:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #CBD5E1, stop:1 #94A3B8);" // Even darker when pressed
        "}"
    );
    
    // Connect selection change to update properties
    connect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCellSelectionChanged);
    
    // Connect double click to toggle properties visibility
    connect(tableWidget, &QTableWidget::cellDoubleClicked, this, &TableView::onCellDoubleClicked);
    
    tableLayout->addWidget(tableWidget);
}

void TableView::createPropertiesArea()
{
    propertiesArea = new QWidget();
    propertiesArea->setStyleSheet(
        "QWidget { "
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FAFBFC, stop:1 #F8FAFC); "
            "border-top: 2px solid #E2E8F0; "
            "border-radius: 0 0 8px 8px;"
        "}"
    );
    propertiesArea->setMaximumHeight(400); // Increased maximum height
    propertiesArea->setMinimumHeight(250); // Increased minimum height to prevent cut-off
    propertiesArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesArea);
    propertiesLayout->setContentsMargins(20, 20, 20, 20); // Increased margins
    propertiesLayout->setSpacing(16); // Increased spacing
    
    // Properties title
    QLabel *propertiesTitle = new QLabel("Propiedades del Campo");
    propertiesTitle->setFont(QFont("Inter", 15, QFont::DemiBold)); // Larger font
    propertiesTitle->setStyleSheet(
        "QLabel { "
            "color: #1E293B; "
            "margin-bottom: 12px; "
            "padding: 12px 0px; "
            "border-bottom: 2px solid #E2E8F0;"
        "}"
    );
    propertiesLayout->addWidget(propertiesTitle);
    
    // Create scroll area for properties form to prevent cut-off
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(
        "QScrollArea {"
            "background-color: transparent;"
            "border: none;"
        "}"
        "QScrollBar:vertical {"
            "background-color: #F1F5F9;"
            "width: 12px;"
            "border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
            "background-color: #CBD5E1;"
            "border-radius: 6px;"
            "min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
            "background-color: #94A3B8;"
        "}"
    );
    
    // Create 3-column grid layout exactly like the reference image
    QWidget *propertiesForm = new QWidget();
    propertiesForm->setStyleSheet("QWidget { background-color: transparent; }");
    QGridLayout *formLayout = new QGridLayout(propertiesForm);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(20); // Increased spacing
    formLayout->setVerticalSpacing(15); // Increased spacing
    
    // Set column stretch to make better use of space
    formLayout->setColumnStretch(0, 1);
    formLayout->setColumnStretch(1, 1);
    formLayout->setColumnStretch(2, 1);
    
    // First row: Nombre, Tamaño, Valor Default
    formLayout->addWidget(createPropertyLabel("Nombre:"), 0, 0);
    formLayout->addWidget(createPropertyLabel("Tamaño:"), 0, 1);
    formLayout->addWidget(createPropertyLabel("Valor Default:"), 0, 2);
    
    fieldNameEdit = new QLineEdit();
    fieldNameEdit->setStyleSheet(getEnhancedInputStyle());
    fieldNameEdit->setMinimumHeight(40); // Increased height
    fieldNameEdit->setMaximumHeight(40);
    fieldNameEdit->setToolTip("");  // Disable tooltip
    fieldNameEdit->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    fieldNameEdit->setCompleter(nullptr);  // Disable autocomplete
    formLayout->addWidget(fieldNameEdit, 1, 0);
    
    fieldSizeEdit = new QLineEdit();
    fieldSizeEdit->setStyleSheet(getEnhancedInputStyle());
    fieldSizeEdit->setMinimumHeight(40); // Increased height
    fieldSizeEdit->setMaximumHeight(40);
    fieldSizeEdit->setText("Entero largo");  // Set as default text instead of placeholder
    fieldSizeEdit->setToolTip("");  // Disable tooltip
    fieldSizeEdit->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    fieldSizeEdit->setCompleter(nullptr);  // Disable autocomplete
    formLayout->addWidget(fieldSizeEdit, 1, 1);
    
    defaultValueEdit = new QLineEdit();
    defaultValueEdit->setStyleSheet(getEnhancedInputStyle());
    defaultValueEdit->setMinimumHeight(40); // Increased height
    defaultValueEdit->setMaximumHeight(40);
    defaultValueEdit->setText("AutoNumber");  // Set as default text instead of placeholder
    defaultValueEdit->setToolTip("");  // Disable tooltip
    defaultValueEdit->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    defaultValueEdit->setCompleter(nullptr);  // Disable autocomplete
    formLayout->addWidget(defaultValueEdit, 1, 2);
    
    // Second row: Tipo, Requerido, Descripción  
    formLayout->addWidget(createPropertyLabel("Tipo:"), 2, 0);
    formLayout->addWidget(createPropertyLabel("Requerido:"), 2, 1);
    formLayout->addWidget(createPropertyLabel("Descripción:"), 2, 2);
    
    dataTypeCombo = new QComboBox();
    dataTypeCombo->addItems({"Texto", "Número", "Fecha/Hora", "Moneda", "AutoNumber", "Sí/No", "Objeto OLE", "Hipervínculo"});
    dataTypeCombo->setCurrentText("Texto");
    dataTypeCombo->setStyleSheet(getEnhancedComboStyle());
    dataTypeCombo->setMinimumHeight(40); // Increased height
    dataTypeCombo->setMaximumHeight(40);
    dataTypeCombo->setToolTip("");  // Disable tooltip
    dataTypeCombo->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    dataTypeCombo->setEditable(false);  // Ensure not editable
    dataTypeCombo->view()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);  // Remove popup decorations
    dataTypeCombo->view()->setAttribute(Qt::WA_X11NetWmWindowTypeCombo, false);  // Disable system popup
    dataTypeCombo->setMaxVisibleItems(5);  // Limit visible items
    formLayout->addWidget(dataTypeCombo, 3, 0);
    
    // Checkbox styled like in the reference image
    requiredCheck = new QCheckBox();
    requiredCheck->setChecked(true);  // Set checked by default like in the image
    requiredCheck->setStyleSheet(
        "QCheckBox {"
            "spacing: 5px;"
        "}"
        "QCheckBox::indicator {"
            "width: 16px;"
            "height: 16px;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 3px;"
            "background-color: #FFFFFF;"
        "}"
        "QCheckBox::indicator:checked {"
            "background-color: #EC4899;"
            "border-color: #EC4899;"
        "}"
        "QCheckBox::indicator:checked:after {"
            "content: '✓';"
            "color: white;"
            "font-weight: bold;"
            "font-size: 12px;"
        "}"
    );
    
    // Add label next to checkbox like in the image
    QWidget *requiredWidget = new QWidget();
    QHBoxLayout *requiredLayout = new QHBoxLayout(requiredWidget);
    requiredLayout->setContentsMargins(0, 0, 0, 0);
    requiredLayout->setSpacing(5);
    requiredLayout->addWidget(requiredCheck);
    
    QLabel *requiredLabel = new QLabel("Campo Requerido");
    requiredLabel->setStyleSheet("QLabel { color: #374151; font-size: 12px; }");
    requiredLayout->addWidget(requiredLabel);
    requiredLayout->addStretch();
    
    formLayout->addWidget(requiredWidget, 3, 1);
    
    descriptionEdit = new QLineEdit();
    descriptionEdit->setStyleSheet(getEnhancedInputStyle());
    descriptionEdit->setMinimumHeight(40); // Increased height
    descriptionEdit->setMaximumHeight(40);
    descriptionEdit->setText("Identificador único");  // Set as default text instead of placeholder
    descriptionEdit->setToolTip("");  // Disable tooltip
    descriptionEdit->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    descriptionEdit->setCompleter(nullptr);  // Disable autocomplete
    formLayout->addWidget(descriptionEdit, 3, 2);
    
    // Hidden combo for field size options (keep for compatibility)
    fieldSizeCombo = new QComboBox();
    fieldSizeCombo->setStyleSheet(getEnhancedComboStyle());
    fieldSizeCombo->setToolTip("");  // Disable tooltip
    fieldSizeCombo->setAttribute(Qt::WA_InputMethodEnabled, false);  // Disable input method
    fieldSizeCombo->setEditable(false);  // Ensure not editable
    fieldSizeCombo->hide();
    fieldSizeCombo->setEnabled(false);  // Completely disable it
    
    // Set the form widget to the scroll area
    scrollArea->setWidget(propertiesForm);
    propertiesLayout->addWidget(scrollArea);
}

QLabel* TableView::createPropertyLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setFont(QFont("Segoe UI", 13, QFont::Medium)); // Larger font with Excel-like typeface
    label->setStyleSheet(
        "QLabel { "
            "color: #374151; "
            "font-weight: 600; " // Bolder text
            "margin-bottom: 8px; " // Increased margin
            "padding: 0px;"
        "}"
    );
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return label;
}

QString TableView::getInputStyle()
{
    return "QLineEdit {"
           "background-color: #FFFFFF;"
           "border: 1px solid #D1D5DB;"
           "border-radius: 6px;"
           "padding: 8px 12px;"
           "color: #111827;"
           "font-size: 13px;"
           "}"
           "QLineEdit:focus {"
           "border-color: #A4373A;"
           "outline: none;"
           "}";
}

QString TableView::getCompactInputStyle()
{
    return "QLineEdit {"
           "background-color: #FFFFFF;"
           "border: 1px solid #D1D5DB;"
           "border-radius: 4px;" // Radio más pequeño
           "padding: 4px 6px;" // Padding más pequeño
           "color: #111827;"
           "font-size: 11px;" // Fuente más pequeña
           "}"
           "QLineEdit:focus {"
           "border-color: #A4373A;"
           "outline: none;"
           "}";
}

QString TableView::getComboStyle()
{
    return "QComboBox {"
           "background-color: #FFFFFF;"
           "border: 1px solid #D1D5DB;"
           "border-radius: 6px;"
           "padding: 8px 12px;"
           "color: #111827;"
           "font-size: 13px;"
           "}"
           "QComboBox:focus {"
           "border-color: #A4373A;"
           "}"
           "QComboBox::drop-down {"
           "border: none;"
           "}"
           "QComboBox::down-arrow {"
           "image: none;"
           "border: none;"
           "}";
}

QString TableView::getCompactComboStyle()
{
    return "QComboBox {"
           "background-color: #FFFFFF;"
           "border: 1px solid #D1D5DB;"
           "border-radius: 4px;" // Radio más pequeño
           "padding: 4px 6px 4px 6px;" // Padding más pequeño
           "color: #111827;"
           "font-size: 11px;" // Fuente más pequeña
           "padding-right: 20px;" // Espacio para la flecha
           "}"
           "QComboBox:focus {"
           "border-color: #A4373A;"
           "}"
           "QComboBox::drop-down {"
           "subcontrol-origin: padding;"
           "subcontrol-position: top right;"
           "width: 18px;"
           "border-left: 1px solid #D1D5DB;"
           "border-top-right-radius: 4px;"
           "border-bottom-right-radius: 4px;"
           "background-color: #F9FAFB;"
           "}"
           "QComboBox::drop-down:hover {"
           "background-color: #F3F4F6;"
           "}"
           "QComboBox::down-arrow {"
           "image: none;"
           "border: none;"
           "width: 0;"
           "height: 0;"
           "border-left: 4px solid transparent;"
           "border-right: 4px solid transparent;"
           "border-top: 5px solid #6B7280;"
           "margin: 0px;"
           "}"
           "QComboBox::down-arrow:hover {"
           "border-top-color: #374151;"
           "}"
           "QComboBox QAbstractItemView {"
           "background-color: #FFFFFF;"
           "border: 1px solid #D1D5DB;"
           "border-radius: 4px;"
           "selection-background-color: #EFF6FF;"
           "color: #111827;"
           "font-size: 11px;"
           "padding: 2px;"
           "outline: none;"
           "}"
           "QComboBox QAbstractItemView::item {"
           "padding: 6px 8px;"
           "min-height: 16px;"
           "}"
           "QComboBox QAbstractItemView::item:selected {"
           "background-color: #DBEAFE;"
           "color: #1E40AF;"
           "}";
}

QString TableView::getEnhancedInputStyle()
{
    return "QLineEdit {"
           "background-color: #FFFFFF;"
           "border: 2px solid #E2E8F0;"
           "border-radius: 6px;" // Slightly smaller radius for more professional look
           "padding: 12px 16px;" // Increased padding for better appearance
           "color: #1F2937;"
           "font-size: 13px;"
           "font-family: 'Segoe UI', 'Inter';" // Excel-like font
           "font-weight: 400;"
           "}"
           "QLineEdit:focus {"
           "border-color: #3B82F6;"
           "outline: none;"
           "box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1);"
           "}"
           "QLineEdit:hover {"
           "border-color: #CBD5E1;"
           "}";
}

QString TableView::getEnhancedComboStyle()
{
    return "QComboBox {"
           "background-color: #FFFFFF;"
           "border: 2px solid #E2E8F0;"
           "border-radius: 6px;" // Smaller radius for professional look
           "padding: 12px 16px 12px 16px;" // Increased padding
           "color: #1F2937;"
           "font-size: 13px;"
           "font-family: 'Segoe UI', 'Inter';" // Excel-like font
           "font-weight: 400;"
           "padding-right: 35px;"
           "}"
           "QComboBox:focus {"
           "border-color: #3B82F6;"
           "}"
           "QComboBox:hover {"
           "border-color: #CBD5E1;"
           "}"
           "QComboBox::drop-down {"
           "subcontrol-origin: padding;"
           "subcontrol-position: top right;"
           "width: 30px;"
           "border-left: 1px solid #E2E8F0;"
           "border-top-right-radius: 6px;"
           "border-bottom-right-radius: 6px;"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F8FAFC, stop:1 #F1F5F9);"
           "}"
           "QComboBox::drop-down:hover {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F1F5F9, stop:1 #E2E8F0);"
           "}"
           "QComboBox::down-arrow {"
           "image: none;"
           "border: none;"
           "width: 0;"
           "height: 0;"
           "border-left: 6px solid transparent;"
           "border-right: 6px solid transparent;"
           "border-top: 8px solid #64748B;"
           "margin: 0px;"
           "}"
           "QComboBox::down-arrow:hover {"
           "border-top-color: #475569;"
           "}"
           "QComboBox QAbstractItemView {"
           "background-color: #FFFFFF;"
           "border: 2px solid #E2E8F0;"
           "border-radius: 6px;"
           "selection-background-color: #EFF6FF;"
           "color: #1F2937;"
           "font-size: 13px;"
           "padding: 6px;" // Increased padding
           "outline: none;"
           "margin-top: 2px;"
           "}"
           "QComboBox QAbstractItemView::item {"
           "padding: 12px 16px;" // Increased padding for items
           "min-height: 25px;" // Increased height
           "border-radius: 4px;"
           "margin: 2px 4px;"
           "}"
           "QComboBox QAbstractItemView::item:selected {"
           "background-color: #DBEAFE;"
           "color: #1E40AF;"
           "}";
}

QString TableView::getEnhancedTextEditStyle()
{
    return "QTextEdit {"
           "background-color: #FFFFFF;"
           "border: 2px solid #E2E8F0;"
           "border-radius: 8px;"
           "padding: 10px 14px;"
           "color: #1F2937;"
           "font-size: 13px;"
           "font-family: 'Inter';"
           "font-weight: 400;"
           "}"
           "QTextEdit:focus {"
           "border-color: #3B82F6;"
           "outline: none;"
           "}"
           "QTextEdit:hover {"
           "border-color: #CBD5E1;"
           "}";
}

void TableView::onCellSelectionChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    
    // Only update properties if properties area is visible
    if (!propertiesArea->isHidden() && currentRow >= 0 && currentColumn >= 0) {
        // Update properties based on selected field
        if (currentRow < tableWidget->rowCount()) {
            QTableWidgetItem *fieldNameItem = tableWidget->item(currentRow, 0);
            QTableWidgetItem *dataTypeItem = tableWidget->item(currentRow, 1);
            QTableWidgetItem *descriptionItem = tableWidget->item(currentRow, 2);
            
            if (fieldNameItem) {
                fieldNameEdit->setText(fieldNameItem->text());
            } else {
                fieldNameEdit->clear();
            }
            
            if (dataTypeItem) {
                QString dataType = dataTypeItem->text();
                int index = dataTypeCombo->findText(dataType, Qt::MatchFixedString);
                if (index >= 0) {
                    dataTypeCombo->setCurrentIndex(index);
                } else {
                    dataTypeCombo->setCurrentText(dataType);
                }
                
                // Connect signal to update properties when data type changes
                disconnect(dataTypeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), 
                          this, nullptr); // Disconnect previous connection
                connect(dataTypeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
                       this, [this, currentRow](const QString &newDataType) {
                    // Update the table item
                    QTableWidgetItem *typeItem = tableWidget->item(currentRow, 1);
                    if (typeItem) {
                        typeItem->setText(newDataType);
                    }
                    // Update properties
                    QTableWidgetItem *nameItem = tableWidget->item(currentRow, 0);
                    updateFieldProperties(currentRow, nameItem ? nameItem->text() : "");
                });
            } else {
                dataTypeCombo->setCurrentIndex(0);
            }
            
            if (descriptionItem) {
                descriptionEdit->setText(descriptionItem->text());
            } else {
                descriptionEdit->clear();
            }
            
            // Set sample properties based on data type and field
            updateFieldProperties(currentRow, fieldNameItem ? fieldNameItem->text() : "");
        }
    }
}

void TableView::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    
    // Toggle properties visibility on double click
    if (propertiesArea->isHidden()) {
        propertiesArea->show();
        mainSplitter->setSizes({350, 180}); // Mostrar propiedades con buen tamaño
        
        // Update properties for the current selection
        int currentRow = tableWidget->currentRow();
        if (currentRow >= 0) {
            QTableWidgetItem *fieldNameItem = tableWidget->item(currentRow, 0);
            QTableWidgetItem *dataTypeItem = tableWidget->item(currentRow, 1);
            QTableWidgetItem *descriptionItem = tableWidget->item(currentRow, 2);
            
            if (fieldNameItem) {
                fieldNameEdit->setText(fieldNameItem->text());
            } else {
                fieldNameEdit->clear();
            }
            
            if (dataTypeItem) {
                QString dataType = dataTypeItem->text();
                int index = dataTypeCombo->findText(dataType, Qt::MatchFixedString);
                if (index >= 0) {
                    dataTypeCombo->setCurrentIndex(index);
                } else {
                    dataTypeCombo->setCurrentText(dataType);
                }
            } else {
                dataTypeCombo->setCurrentIndex(0);
            }
            
            if (descriptionItem) {
                descriptionEdit->setText(descriptionItem->text());
            } else {
                descriptionEdit->clear();
            }
            
            updateFieldProperties(currentRow, fieldNameItem ? fieldNameItem->text() : "");
        }
    } else {
        propertiesArea->hide();
        mainSplitter->setSizes({500, 0}); // Dar todo el espacio a la tabla
    }
}

void TableView::updateFieldProperties(int row, const QString &fieldName)
{
    // Get the data type for this field
    QTableWidgetItem *dataTypeItem = tableWidget->item(row, 1);
    QString dataType = dataTypeItem ? dataTypeItem->text() : "";
    
    // Clear current properties
    fieldSizeEdit->clear();
    fieldSizeCombo->clear();
    requiredCheck->setChecked(false);
    defaultValueEdit->clear();
    
    // Hide/show appropriate controls based on data type
    if (dataType == "int") {
        fieldSizeEdit->hide();
        fieldSizeCombo->show();
        fieldSizeCombo->addItems({"Entero", "Decimal", "Doble", "Byte"});
        fieldSizeCombo->setCurrentText("Entero");
        if (fieldName.toLower() == "id") {
            requiredCheck->setChecked(true);
            defaultValueEdit->setText("AutoNumber");
        }
    }
    else if (dataType == "float") {
        fieldSizeEdit->hide();
        fieldSizeCombo->show();
        fieldSizeCombo->addItems({"Entero", "Decimal", "Doble", "Byte"});
        fieldSizeCombo->setCurrentText("Decimal");
        defaultValueEdit->setPlaceholderText("0.0");
    }
    else if (dataType == "bool") {
        fieldSizeEdit->show();
        fieldSizeCombo->hide();
        fieldSizeEdit->setText("Sí/No");
        fieldSizeEdit->setReadOnly(true);
        defaultValueEdit->setPlaceholderText("Falso");
    }
    else if (dataType == "char[N]" || dataType == "string") {
        fieldSizeEdit->show();
        fieldSizeCombo->hide();
        fieldSizeEdit->setReadOnly(false);
        fieldSizeEdit->setText(dataType == "char[N]" ? "50" : "255");
        fieldSizeEdit->setPlaceholderText("Tamaño de campo (Max 255)");
        if (fieldName.toLower().contains("nombre") || fieldName.toLower().contains("email")) {
            requiredCheck->setChecked(true);
        }
    }
    else if (dataType == "moneda") {
        fieldSizeEdit->hide();
        fieldSizeCombo->show();
        fieldSizeCombo->addItems({"Moneda Lps", "Dollar", "Euros", "Millares"});
        fieldSizeCombo->setCurrentText("Moneda Lps");
        defaultValueEdit->setPlaceholderText("0.00");
    }
    else if (dataType == "fecha") {
        fieldSizeEdit->hide();
        fieldSizeCombo->show();
        fieldSizeCombo->addItems({"DD-MM-YY", "DD/MM/YY", "DD/MESTEXTO/YYYY"});
        fieldSizeCombo->setCurrentText("DD-MM-YY");
        defaultValueEdit->setPlaceholderText("Fecha()");
    }
    else {
        // Default case - show text field
        fieldSizeEdit->show();
        fieldSizeCombo->hide();
        fieldSizeEdit->setReadOnly(false);
        fieldSizeEdit->setText("255");
    }
    
    // Update property labels based on data type
    updatePropertyLabels(dataType);
}

void TableView::updatePropertyLabels(const QString &dataType)
{
    // Find the property labels and update them
    QList<QLabel*> labels = propertiesArea->findChildren<QLabel*>();
    
    for (QLabel* label : labels) {
        if (label->text().startsWith("Tamaño:") || label->text().startsWith("Formato:")) {
            if (dataType == "int" || dataType == "float") {
                label->setText("Tamaño:");
                fieldSizeEdit->setToolTip("Opciones: Entero, Decimal, Doble o Byte");
            }
            else if (dataType == "char[N]" || dataType == "string") {
                label->setText("Tamaño:");
                fieldSizeEdit->setToolTip("Tamaño máximo: 255 caracteres");
            }
            else if (dataType == "moneda") {
                label->setText("Formato:");
                fieldSizeEdit->setToolTip("Formatos: Moneda Lps, Dollar, Euros y Millares");
            }
            else if (dataType == "fecha") {
                label->setText("Formato:");
                fieldSizeEdit->setToolTip("Formatos: DD-MM-YY, DD/MM/YY, DD/MESTEXTO/YYYY");
            }
            break;
        }
    }
}

void TableView::onDesignViewClicked()
{
    // Cambiar estilo del botón activo
    designViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #EFF6FF;" // Fondo azul claro (activo)
            "color: #1D4ED8;" // Texto azul
            "border: 1px solid #BFDBFE;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #DBEAFE;"
        "}"
    );
    
    // Cambiar estilo de botones inactivos
    editorViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    dataViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    // Aquí puedes agregar lógica para cambiar la vista a diseño
    // qDebug() << "Cambiando a Vista de Diseño";
}

void TableView::onEditorViewClicked()
{
    // Cambiar estilo del botón activo
    editorViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #EFF6FF;" // Fondo azul claro (activo)
            "color: #1D4ED8;" // Texto azul
            "border: 1px solid #BFDBFE;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #DBEAFE;"
        "}"
    );
    
    // Cambiar estilo de botones inactivos
    designViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    dataViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    // Aquí puedes agregar lógica para cambiar la vista a editor
    // qDebug() << "Cambiando a Vista Editor";
}

void TableView::onDataViewClicked()
{
    // Cambiar estilo del botón activo
    dataViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #EFF6FF;" // Fondo azul claro (activo)
            "color: #1D4ED8;" // Texto azul
            "border: 1px solid #BFDBFE;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #DBEAFE;"
        "}"
    );
    
    // Cambiar estilo de botones inactivos
    designViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    editorViewBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "color: #6B7280;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 6px;"
            "padding: 4px 12px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "color: #374151;"
        "}"
    );
    
    // Aquí puedes agregar lógica para cambiar la vista a datos
    // qDebug() << "Cambiando a Vista de Datos";
}

void TableView::onTableItemChanged(QTableWidgetItem *item)
{
    if (!item) return;
    
    int row = item->row();
    int column = item->column();
    
    // Si el usuario está escribiendo en la última fila y en cualquier columna
    if (row == tableWidget->rowCount() - 1 && (column == 0 || column == 1 || column == 2)) {
        // Si hay contenido en el campo actual, agregar una nueva fila
        if (!item->text().trimmed().isEmpty()) {
            tableWidget->setRowCount(tableWidget->rowCount() + 1);
            
            // Configurar la nueva fila con valores vacíos para permitir edición
            QTableWidgetItem *nameItem = new QTableWidgetItem("");
            QTableWidgetItem *typeItem = new QTableWidgetItem("");
            QTableWidgetItem *descItem = new QTableWidgetItem("");
            
            int newRow = tableWidget->rowCount() - 1;
            tableWidget->setItem(newRow, 0, nameItem);
            tableWidget->setItem(newRow, 1, typeItem);
            tableWidget->setItem(newRow, 2, descItem);
            
            // Actualizar las etiquetas del header vertical para incluir la nueva fila
            QStringList verticalHeaders;
            verticalHeaders << "🔑"; // Primary key icon for first row
            for (int i = 1; i < tableWidget->rowCount(); i++) {
                verticalHeaders << QString::number(i + 1);
            }
            tableWidget->setVerticalHeaderLabels(verticalHeaders);
        }
    }
    
    // Force update of row heights when content changes
    tableWidget->resizeRowsToContents();
}

// DataTypeDelegate Implementation
DataTypeDelegate::DataTypeDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *DataTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QComboBox *comboBox = new QComboBox(parent);
    
    // Add specific data types as requested
    comboBox->addItem("int");      // Claves primarias, edades, IDs
    comboBox->addItem("float");    // Promedios, notas, precios
    comboBox->addItem("bool");     // Estados activos/inactivos
    comboBox->addItem("char[N]");  // Nombres, descripciones cortas
    comboBox->addItem("string");   // Direcciones, observaciones
    comboBox->addItem("moneda");   // Campos monetarios
    comboBox->addItem("fecha");    // Campos de fecha
    
    // Style the combo box with visible arrow
    comboBox->setStyleSheet(
        "QComboBox {"
            "background-color: #FFFFFF;"
            "border: 2px solid #3B82F6;"
            "border-radius: 4px;"
            "padding: 8px 12px 8px 12px;"
            "color: #111827;"
            "font-size: 14px;"
            "font-family: 'Inter';"
            "min-height: 20px;"
            "padding-right: 30px;" // Espacio para la flecha
        "}"
        "QComboBox:focus {"
            "border-color: #1D4ED8;"
        "}"
        "QComboBox::drop-down {"
            "subcontrol-origin: padding;"
            "subcontrol-position: top right;"
            "width: 25px;"
            "border-left: 1px solid #D1D5DB;"
            "border-top-right-radius: 4px;"
            "border-bottom-right-radius: 4px;"
            "background-color: #F3F4F6;"
        "}"
        "QComboBox::drop-down:hover {"
            "background-color: #E5E7EB;"
        "}"
        "QComboBox::down-arrow {"
            "image: none;"
            "border: none;"
            "width: 0;"
            "height: 0;"
            "border-left: 5px solid transparent;"
            "border-right: 5px solid transparent;"
            "border-top: 6px solid #6B7280;"
            "margin: 0px;"
        "}"
        "QComboBox::down-arrow:hover {"
            "border-top-color: #374151;"
        "}"
        "QComboBox QAbstractItemView {"
            "background-color: #FFFFFF;"
            "border: 2px solid #E2E8F0;"
            "border-radius: 8px;"
            "selection-background-color: #EFF6FF;"
            "color: #1F2937;"
            "font-size: 13px;"
            "padding: 4px;"
            "outline: none;"
            "margin-top: 2px;"
        "}"
        "QComboBox QAbstractItemView::item {"
            "padding: 12px 16px;"
            "min-height: 20px;"
            "border-radius: 4px;"
            "margin: 2px 4px;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
            "background-color: #DBEAFE;"
            "color: #1E40AF;"
        "}"
    );
    
    return comboBox;
}

void DataTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        int idx = comboBox->findText(value);
        if (idx >= 0) {
            comboBox->setCurrentIndex(idx);
        }
    }
}

void DataTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }
}

void DataTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

// DescriptionDelegate Implementation
DescriptionDelegate::DescriptionDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *DescriptionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QTextEdit *textEdit = new QTextEdit(parent);
    textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Style the text edit
    textEdit->setStyleSheet(
        "QTextEdit {"
            "background-color: #FFFFFF;"
            "border: 2px solid #3B82F6;"
            "border-radius: 4px;"
            "color: #111827;"
            "font-size: 14px;"
            "font-family: 'Inter';"
            "padding: 8px 12px;"
            "min-height: 60px;"
        "}"
        "QTextEdit:focus {"
            "border-color: #1D4ED8;"
        "}"
    );
    
    return textEdit;
}

void DescriptionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
    if (textEdit) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        textEdit->setPlainText(value);
    }
}

void DescriptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
    if (textEdit) {
        model->setData(index, textEdit->toPlainText(), Qt::EditRole);
    }
}

void DescriptionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(index)
    QRect rect = option.rect;
    rect.setHeight(qMax(rect.height(), 80)); // Altura mínima para el editor
    editor->setGeometry(rect);
}

QSize DescriptionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    
    if (text.isEmpty()) {
        return QSize(-1, 50); // Altura mínima para celdas vacías
    }
    
    // Calcular altura basada en el texto
    QFontMetrics fm(option.font);
    int width = 500; // Ancho de la columna descripción
    QRect textRect = fm.boundingRect(0, 0, width - 32, 0, Qt::TextWordWrap, text);
    int height = qMax(textRect.height() + 24, 50); // Padding + altura mínima
    
    return QSize(-1, height);
}

void DescriptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    
    // Draw background
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, QColor("#DBEAFE"));
    } else {
        painter->fillRect(opt.rect, QColor("#FFFFFF"));
    }
    
    // Draw text with word wrap
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    if (!text.isEmpty()) {
        painter->setPen(QColor("#111827"));
        painter->setFont(opt.font);
        
        QRect textRect = opt.rect.adjusted(12, 8, -12, -8); // Padding
        painter->drawText(textRect, Qt::TextWordWrap | Qt::AlignTop | Qt::AlignLeft, text);
    }
}
