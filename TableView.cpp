#include "TableView.h"
#include <QDebug>
#include <exception>

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
    mainSplitter->setHandleWidth(12); // Better handle size for easier resizing
    mainSplitter->setChildrenCollapsible(true);
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #F1F5F9, stop:0.5 #E2E8F0, stop:1 #CBD5E1);"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "margin: 2px 4px;"
        "}"
        "QSplitter::handle:hover {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #E2E8F0, stop:0.5 #CBD5E1, stop:1 #9CA3AF);"
            "border-color: #9CA3AF;"
        "}"
        "QSplitter::handle:pressed {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #CBD5E1, stop:0.5 #9CA3AF, stop:1 #6B7280);"
            "border-color: #6B7280;"
        "}"
    );
    
    // Create table view area (top)
    createTableArea();
    
    // Create properties area (bottom) - visible by default
    createPropertiesArea();
    // propertiesArea->hide(); // Remove this line to show by default
    
    // Add areas to splitter
    mainSplitter->addWidget(tableArea);
    mainSplitter->addWidget(propertiesArea);
    
    // Set initial sizes with better proportions for compact design
    mainSplitter->setSizes({700, 250}); // Table larger, properties more compact
    // Set minimum sizes for better resizing behavior
    tableArea->setMinimumHeight(300);
    propertiesArea->setMinimumHeight(180); // Reduced minimum
    
    // Add to main layout
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(mainSplitter);
    
    // Apply initial theme
    updateTheme(false);
    
    // Configurar la tabla inicial como Vista de Diseño con campo predeterminado
    QTimer::singleShot(50, this, [this]() {
        setupDesignView();
    });
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
    
    // Botón Vista Datos
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
    connect(dataViewBtn, &QPushButton::clicked, this, &TableView::onDataViewClicked);
    
    // Agregar botones al layout (solo Vista de Diseño y Vista Datos)
    viewButtonsLayout->addWidget(designViewBtn);
    viewButtonsLayout->addWidget(dataViewBtn);
    viewButtonsLayout->addStretch(); // Empujar botones a la izquierda
    
    tableLayout->addWidget(viewButtonsWidget);
    
    // Create table widget for field design
    tableWidget = new QTableWidget();
    
    // Set up table structure for field design (like Access design view)
    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(1); // Solo la primera fila (Id) por defecto
    
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
    idItem->setToolTip(""); // Disable tooltip
    tableWidget->setItem(0, 0, idItem);

    QTableWidgetItem *typeItem = new QTableWidgetItem("int");
    typeItem->setToolTip(""); // Disable tooltip
    tableWidget->setItem(0, 1, typeItem);

    QTableWidgetItem *descItem = new QTableWidgetItem("Identificador único");
    descItem->setToolTip(""); // Disable tooltip
    tableWidget->setItem(0, 2, descItem);
    
    // NO agregar filas vacías predeterminadas - solo la primera fila (Id)
    // Las filas se agregarán automáticamente cuando el usuario escriba
    
    // Connect signal para agregar filas automáticamente
    connect(tableWidget, &QTableWidget::itemChanged, this, &TableView::onTableItemChanged);
    
    // Enable safe data type delegate for dropdown selection
    DataTypeDelegate *dataTypeDelegate = new DataTypeDelegate(this);
    tableWidget->setItemDelegateForColumn(1, dataTypeDelegate);
    
    // Keep description delegate disabled to prevent floating text editors
    // DescriptionDelegate *descriptionDelegate = new DescriptionDelegate(this);
    // tableWidget->setItemDelegateForColumn(2, descriptionDelegate);
    
    // Configure table properties for design view
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setGridStyle(Qt::SolidLine);
    tableWidget->setShowGrid(true);
    
    // Enable cell editing but with basic triggers only
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    
    // Disable tooltips for the table to prevent floating popups
    tableWidget->setAttribute(Qt::WA_AlwaysShowToolTips, false);
    tableWidget->setToolTip("");
    
    // Disable context menu but allow editing
    tableWidget->setContextMenuPolicy(Qt::NoContextMenu);
    
    // Disable mouse tracking to prevent hover tooltips
    tableWidget->setMouseTracking(false);
    
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
    verticalHeaders << "🔑"; // Primary key icon for first row only
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
    
    // Re-enable double click but with safer implementation
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
    propertiesArea->setMaximumHeight(350); // Reduced from 600
    propertiesArea->setMinimumHeight(180); // Reduced from 200
    propertiesArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesArea);
    propertiesLayout->setContentsMargins(16, 16, 16, 16); // Reduced from 24
    propertiesLayout->setSpacing(12); // Reduced from 20
    
    // Properties title - smaller
    QLabel *propertiesTitle = new QLabel("Propiedades del Campo");
    propertiesTitle->setFont(QFont("Inter", 14, QFont::DemiBold)); // Reduced from 16
    propertiesTitle->setStyleSheet(
        "QLabel { "
            "color: #1E293B; "
            "margin-bottom: 10px; " // Reduced from 16
            "padding: 8px 0px; " // Reduced from 12
            "border-bottom: 2px solid #E2E8F0;"
        "}"
    );
    propertiesLayout->addWidget(propertiesTitle);
    
    // Create scroll area for properties form
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
            "width: 8px;" // Reduced from 12
            "border-radius: 4px;" // Reduced from 6
        "}"
        "QScrollBar::handle:vertical {"
            "background-color: #CBD5E1;"
            "border-radius: 4px;" // Reduced from 6
            "min-height: 20px;" // Reduced from 30
        "}"
        "QScrollBar::handle:vertical:hover {"
            "background-color: #94A3B8;"
        "}"
    );
    
    // Create improved grid layout with tighter spacing
    QWidget *propertiesForm = new QWidget();
    propertiesForm->setStyleSheet("QWidget { background-color: transparent; }");
    QGridLayout *formLayout = new QGridLayout(propertiesForm);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(16); // Reduced from 24
    formLayout->setVerticalSpacing(12); // Reduced from 20
    
    // Set column stretch for responsive design
    formLayout->setColumnStretch(0, 1);
    formLayout->setColumnStretch(1, 1);
    formLayout->setColumnStretch(2, 1);
    
    // Row 1: Nombre, Tamaño, Valor Default
    formLayout->addWidget(createPropertyLabel("Nombre:"), 0, 0);
    formLayout->addWidget(createPropertyLabel("Tamaño:"), 0, 1);
    formLayout->addWidget(createPropertyLabel("Valor Default:"), 0, 2);
    
    // Create compact input fields
    fieldNameEdit = new QLineEdit();
    fieldNameEdit->setStyleSheet(getCompactInputStyle()); // Use compact style
    fieldNameEdit->setMinimumHeight(32); // Reduced from 44
    fieldNameEdit->setMaximumHeight(32);
    fieldNameEdit->setToolTip("");
    fieldNameEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    fieldNameEdit->setCompleter(nullptr);
    formLayout->addWidget(fieldNameEdit, 1, 0);
    
    fieldSizeEdit = new QLineEdit();
    fieldSizeEdit->setStyleSheet(getCompactInputStyle()); // Use compact style
    fieldSizeEdit->setMinimumHeight(32);
    fieldSizeEdit->setMaximumHeight(32);
    fieldSizeEdit->setText("255");
    fieldSizeEdit->setToolTip("");
    fieldSizeEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    fieldSizeEdit->setCompleter(nullptr);
    formLayout->addWidget(fieldSizeEdit, 1, 1);
    
    defaultValueEdit = new QLineEdit();
    defaultValueEdit->setStyleSheet(getCompactInputStyle()); // Use compact style
    defaultValueEdit->setMinimumHeight(32);
    defaultValueEdit->setMaximumHeight(32);
    defaultValueEdit->setText("0.0");
    defaultValueEdit->setToolTip("");
    defaultValueEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    defaultValueEdit->setCompleter(nullptr);
    formLayout->addWidget(defaultValueEdit, 1, 2);
    
    // Row 2: Tipo, Requerido, Descripción  
    formLayout->addWidget(createPropertyLabel("Tipo:"), 2, 0);
    formLayout->addWidget(createPropertyLabel("Requerido:"), 2, 1);
    formLayout->addWidget(createPropertyLabel("Descripción:"), 2, 2);
    
    // Compact data type combo - completely disabled dropdown
    dataTypeCombo = new QComboBox();
    dataTypeCombo->addItems({"Texto", "Número", "Fecha/Hora", "Moneda", "AutoNumber", "Sí/No", "Objeto OLE", "Hipervínculo"});
    dataTypeCombo->setCurrentText("Texto");
    dataTypeCombo->setStyleSheet(getCompactComboStyle()); // Use compact style
    dataTypeCombo->setMinimumHeight(32); // Reduced from 44
    dataTypeCombo->setMaximumHeight(32);
    dataTypeCombo->setToolTip("");
    dataTypeCombo->setAttribute(Qt::WA_InputMethodEnabled, false);
    dataTypeCombo->setEditable(false);
    // Completely disable any dropdown behavior
    dataTypeCombo->setEnabled(false); // Disable completely
    dataTypeCombo->hide(); // Hide it completely
    
    // Replace with a simple label showing the type
    dataTypeLabel = new QLabel("Texto");
    dataTypeLabel->setStyleSheet(
        "QLabel {"
            "background-color: #F9FAFB;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 4px;"
            "padding: 6px 10px;"
            "color: #111827;"
            "font-size: 12px;"
            "font-family: 'Inter', system-ui;"
        "}"
    );
    dataTypeLabel->setMinimumHeight(32);
    dataTypeLabel->setMaximumHeight(32);
    formLayout->addWidget(dataTypeLabel, 3, 0);
    
    // Compact required checkbox
    requiredCheck = new QCheckBox();
    requiredCheck->setChecked(false); // Changed to unchecked as in image
    requiredCheck->setStyleSheet(
        "QCheckBox {"
            "spacing: 6px;" // Reduced from 8
            "font-size: 12px;" // Reduced from 14
        "}"
        "QCheckBox::indicator {"
            "width: 16px;" // Reduced from 18
            "height: 16px;" // Reduced from 18
            "border: 2px solid #D1D5DB;"
            "border-radius: 3px;" // Reduced from 4
            "background-color: #FFFFFF;"
        "}"
        "QCheckBox::indicator:checked {"
            "background-color: #EC4899;"
            "border-color: #EC4899;"
            "image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTAiIGhlaWdodD0iOCIgdmlld0JveD0iMCAwIDEwIDgiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxwYXRoIGQ9Ik05IDFMMy41IDYuNUwxIDQiIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMS41IiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+);" // Smaller checkmark
        "}"
        "QCheckBox::indicator:hover {"
            "border-color: #9CA3AF;"
        "}"
    );
    
    QWidget *requiredWidget = new QWidget();
    QHBoxLayout *requiredLayout = new QHBoxLayout(requiredWidget);
    requiredLayout->setContentsMargins(0, 0, 0, 0);
    requiredLayout->setSpacing(6); // Reduced from 8
    requiredLayout->addWidget(requiredCheck);
    
    QLabel *requiredLabel = new QLabel("Campo Requerido");
    requiredLabel->setStyleSheet("QLabel { color: #374151; font-size: 12px; }"); // Reduced font
    requiredLayout->addWidget(requiredLabel);
    requiredLayout->addStretch();
    
    formLayout->addWidget(requiredWidget, 3, 1);
    
    descriptionEdit = new QLineEdit();
    descriptionEdit->setStyleSheet(getCompactInputStyle()); // Use compact style
    descriptionEdit->setMinimumHeight(32);
    descriptionEdit->setMaximumHeight(32);
    descriptionEdit->setText("");
    descriptionEdit->setToolTip("");
    descriptionEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    descriptionEdit->setCompleter(nullptr);
    formLayout->addWidget(descriptionEdit, 3, 2);
    
    // Completely disable hidden combo to prevent any floating behavior
    fieldSizeCombo = new QComboBox();
    fieldSizeCombo->setVisible(false);
    fieldSizeCombo->setEnabled(false);
    fieldSizeCombo->setAttribute(Qt::WA_DeleteOnClose);
    fieldSizeCombo->setParent(nullptr); // Remove from parent to prevent any rendering
    
    // Set the form widget to the scroll area
    scrollArea->setWidget(propertiesForm);
    propertiesLayout->addWidget(scrollArea);
    
    // Conectar señales para sincronizar cambios del área de propiedades con la tabla
    connect(fieldNameEdit, &QLineEdit::textChanged, this, &TableView::onFieldNameChanged);
    connect(descriptionEdit, &QLineEdit::textChanged, this, &TableView::onDescriptionChanged);
    connect(requiredCheck, &QCheckBox::toggled, this, &TableView::onRequiredChanged);
}

QLabel* TableView::createPropertyLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setFont(QFont("Inter", 11, QFont::Medium)); // Reduced from 13
    label->setStyleSheet(
        "QLabel { "
            "color: #374151; "
            "font-weight: 600; "
            "margin-bottom: 4px; " // Reduced from 8
            "padding: 1px 0px;" // Reduced from 2px
            "letter-spacing: 0.025em;"
        "}"
    );
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setMinimumHeight(16); // Reduced from 20
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
           "border-radius: 4px;"
           "padding: 6px 10px;" // Better padding for compact size
           "color: #111827;"
           "font-size: 12px;" // Good readable size
           "font-family: 'Inter', system-ui;"
           "}"
           "QLineEdit:focus {"
           "border-color: #A4373A;"
           "outline: none;"
           "box-shadow: 0 0 0 2px rgba(164, 55, 58, 0.1);" // Subtle focus effect
           "}"
           "QLineEdit:hover {"
           "border-color: #9CA3AF;"
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
           "border-radius: 4px;"
           "padding: 6px 10px 6px 10px;" // Better padding for readability
           "color: #111827;"
           "font-size: 12px;" // Slightly larger for better readability
           "font-family: 'Inter', system-ui;"
           "padding-right: 25px;" // Space for arrow
           "}"
           "QComboBox:focus {"
           "border-color: #A4373A;"
           "outline: none;"
           "box-shadow: 0 0 0 2px rgba(164, 55, 58, 0.1);"
           "}"
           "QComboBox:hover {"
           "border-color: #9CA3AF;"
           "}"
           "QComboBox::drop-down {"
           "subcontrol-origin: padding;"
           "subcontrol-position: top right;"
           "width: 20px;"
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
           "font-size: 12px;" // Match combo font size
           "padding: 2px;"
           "outline: none;"
           "}"
           "QComboBox QAbstractItemView::item {"
           "padding: 8px 12px;" // Comfortable padding
           "min-height: 18px;" // Adequate height
           "border-radius: 2px;"
           "margin: 1px 2px;"
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
    
    qDebug() << "DEBUG: onCellSelectionChanged llamado - fila:" << currentRow << "columna:" << currentColumn;
    
    // Verificar que los widgets del área de propiedades existen
    if (!fieldNameEdit || !dataTypeLabel || !descriptionEdit) {
        qDebug() << "WARNING: Widgets del área de propiedades no están inicializados";
        return;
    }
    
    // Always update properties when a cell is selected
    if (currentRow >= 0 && currentColumn >= 0) {
        // Update properties based on selected field
        if (currentRow < tableWidget->rowCount()) {
            QTableWidgetItem *fieldNameItem = tableWidget->item(currentRow, 0);
            QTableWidgetItem *dataTypeItem = tableWidget->item(currentRow, 1);
            QTableWidgetItem *descriptionItem = tableWidget->item(currentRow, 2);
            
            try {
                if (fieldNameItem) {
                    fieldNameEdit->setText(fieldNameItem->text());
                } else {
                    fieldNameEdit->clear();
                }
                
                if (dataTypeItem) {
                    QString dataType = dataTypeItem->text();
                    // Update the label instead of combo
                    dataTypeLabel->setText(dataType);
                } else {
                    // Set default type
                    dataTypeLabel->setText("Texto");
                }
                
                if (descriptionItem) {
                    descriptionEdit->setText(descriptionItem->text());
                } else {
                    descriptionEdit->clear();
                }
                
                // Set sample properties based on data type and field
                try {
                    qDebug() << "DEBUG: Llamando updateFieldProperties para fila" << currentRow;
                    updateFieldProperties(currentRow, fieldNameItem ? fieldNameItem->text() : "");
                    qDebug() << "DEBUG: updateFieldProperties completado exitosamente";
                } catch (...) {
                    qDebug() << "ERROR: Excepción en updateFieldProperties - CONTINUANDO";
                }
                
            } catch (...) {
                qDebug() << "ERROR: Excepción en onCellSelectionChanged al actualizar widgets";
            }
        }
    }
    
    qDebug() << "DEBUG: onCellSelectionChanged completado exitosamente";
}

void TableView::onCellDoubleClicked(int row, int column)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    
    // Simple implementation - just ensure properties are visible
    if (propertiesArea->isHidden()) {
        propertiesArea->show();
        // Set better proportional sizes for compact design
        QList<int> sizes = mainSplitter->sizes();
        int totalHeight = sizes[0] + sizes[1];
        int tableHeight = totalHeight * 0.75; // 75% for table
        int propertiesHeight = totalHeight * 0.25; // 25% for compact properties
        mainSplitter->setSizes({tableHeight, propertiesHeight});
    }
    
    // Force update the current selection without causing floating elements
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0) {
        onCellSelectionChanged(currentRow, tableWidget->currentColumn(), -1, -1);
    }
}

void TableView::updateFieldProperties(int row, const QString &fieldName)
{
    qDebug() << "DEBUG: updateFieldProperties iniciado para fila" << row << "campo" << fieldName;
    
    // Verificar que todos los widgets necesarios existen
    if (!fieldSizeEdit || !fieldSizeCombo || !requiredCheck || !defaultValueEdit) {
        qDebug() << "ERROR: Algunos widgets de propiedades no están inicializados";
        qDebug() << "fieldSizeEdit:" << (fieldSizeEdit ? "OK" : "NULL");
        qDebug() << "fieldSizeCombo:" << (fieldSizeCombo ? "OK" : "NULL");
        qDebug() << "requiredCheck:" << (requiredCheck ? "OK" : "NULL");
        qDebug() << "defaultValueEdit:" << (defaultValueEdit ? "OK" : "NULL");
        return;
    }
    
    // Get the data type for this field
    QTableWidgetItem *dataTypeItem = tableWidget->item(row, 1);
    QString dataType = dataTypeItem ? dataTypeItem->text() : "";
    qDebug() << "DEBUG: Tipo de dato encontrado:" << dataType;
    
    try {
        // Clear current properties - uno por uno con validación
        qDebug() << "DEBUG: Limpiando widgets de propiedades uno por uno...";
        
        try {
            qDebug() << "DEBUG: Limpiando fieldSizeEdit...";
            if (fieldSizeEdit) fieldSizeEdit->clear();
            qDebug() << "DEBUG: fieldSizeEdit limpiado OK";
        } catch (...) {
            qDebug() << "ERROR: Crash al limpiar fieldSizeEdit";
        }
        
        try {
            qDebug() << "DEBUG: Limpiando fieldSizeCombo...";
            if (fieldSizeCombo) fieldSizeCombo->clear();
            qDebug() << "DEBUG: fieldSizeCombo limpiado OK";
        } catch (...) {
            qDebug() << "ERROR: Crash al limpiar fieldSizeCombo";
        }
        
        try {
            qDebug() << "DEBUG: Configurando requiredCheck...";
            if (requiredCheck) requiredCheck->setChecked(false);
            qDebug() << "DEBUG: requiredCheck configurado OK";
        } catch (...) {
            qDebug() << "ERROR: Crash al configurar requiredCheck";
        }
        
        try {
            qDebug() << "DEBUG: Limpiando defaultValueEdit...";
            if (defaultValueEdit) defaultValueEdit->clear();
            qDebug() << "DEBUG: defaultValueEdit limpiado OK";
        } catch (...) {
            qDebug() << "ERROR: Crash al limpiar defaultValueEdit";
        }
        
        qDebug() << "DEBUG: Todos los widgets limpiados exitosamente";
    } catch (...) {
        qDebug() << "ERROR: Excepción al limpiar widgets";
        return;
    }
    
    // Keep all controls in their default state - no show/hide to prevent floating elements
    // Always keep fieldSizeCombo hidden to prevent floating "Entero" popup
    fieldSizeCombo->hide();
    fieldSizeEdit->show();
    
    // Set default values based on data type without showing/hiding widgets
    if (dataType == "int") {
        fieldSizeEdit->setText("Entero largo");
        if (fieldName.toLower() == "id") {
            requiredCheck->setChecked(true);
            defaultValueEdit->setText("AutoNumber");
        }
    }
    else if (dataType == "float") {
        fieldSizeEdit->setText("Decimal");
        defaultValueEdit->setText("0.0");
    }
    else if (dataType == "bool") {
        fieldSizeEdit->setText("Verdadero/Falso");
        fieldSizeEdit->setText("Sí/No");
        fieldSizeEdit->setReadOnly(true);
        defaultValueEdit->setPlaceholderText("Falso");
    }
    else if (dataType == "char[N]" || dataType == "string") {
        // fieldSizeEdit->show();  // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->hide(); // Eliminado para evitar elementos flotantes
        fieldSizeEdit->setReadOnly(false);
        fieldSizeEdit->setText(dataType == "char[N]" ? "50" : "255");
        fieldSizeEdit->setPlaceholderText("Tamaño de campo (Max 255)");
        if (fieldName.toLower().contains("nombre") || fieldName.toLower().contains("email")) {
            requiredCheck->setChecked(true);
        }
    }
    else if (dataType == "moneda") {
        // fieldSizeEdit->hide();  // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->show(); // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->addItems({"Moneda Lps", "Dollar", "Euros", "Millares"}); // Eliminado
        // fieldSizeCombo->setCurrentText("Moneda Lps"); // Eliminado
        fieldSizeEdit->setText("Moneda Lps");
        defaultValueEdit->setPlaceholderText("0.00");
    }
    else if (dataType == "fecha") {
        // fieldSizeEdit->hide();  // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->show(); // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->addItems({"DD-MM-YY", "DD/MM/YY", "DD/MESTEXTO/YYYY"}); // Eliminado
        // fieldSizeCombo->setCurrentText("DD-MM-YY"); // Eliminado
        fieldSizeEdit->setText("DD-MM-YY");
        defaultValueEdit->setPlaceholderText("Fecha()");
    }
    else {
        // Default case
        // fieldSizeEdit->show();  // Eliminado para evitar elementos flotantes
        // fieldSizeCombo->hide(); // Eliminado para evitar elementos flotantes
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
                fieldSizeEdit->setToolTip(""); // Remove tooltip
            }
            else if (dataType == "char[N]" || dataType == "string") {
                label->setText("Tamaño:");
                fieldSizeEdit->setToolTip(""); // Remove tooltip
            }
            else if (dataType == "moneda") {
                label->setText("Formato:");
                fieldSizeEdit->setToolTip(""); // Remove tooltip
            }
            else if (dataType == "fecha") {
                label->setText("Formato:");
                fieldSizeEdit->setToolTip(""); // Remove tooltip
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
    
    // Cambiar estilo de botón inactivo (Solo Vista Datos)
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
    
    // Cambiar la tabla para vista de diseño
    setupDesignView();
}

void TableView::onDataViewClicked()
{
    qDebug() << "DEBUG: onDataViewClicked() iniciado";
    
    try {
        // Cambiar estilo del botón activo
        if (dataViewBtn) {
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
            qDebug() << "DEBUG: Estilo de dataViewBtn actualizado";
        }
        
        // Cambiar estilo de botón inactivo (Solo Vista de Diseño)
        if (designViewBtn) {
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
            qDebug() << "DEBUG: Estilo de designViewBtn actualizado";
        }
        
        // Cambiar la tabla para vista de datos
        qDebug() << "DEBUG: Llamando a setupDataView()";
        setupDataView();
        qDebug() << "DEBUG: setupDataView() completado";
        
    } catch (const std::exception& e) {
        qDebug() << "ERROR en onDataViewClicked():" << e.what();
    } catch (...) {
        qDebug() << "ERROR desconocido en onDataViewClicked()";
    }
}

void TableView::onTableItemChanged(QTableWidgetItem *item)
{
    if (!item) return;
    
    int row = item->row();
    int column = item->column();
    
    // Manejar tanto Vista de Diseño (3 columnas) como Vista de Datos (múltiples columnas)
    bool isDesignView = (tableWidget->columnCount() == 3);
    bool isDataView = (tableWidget->columnCount() > 3);
    
    if (isDesignView) {
        // VISTA DE DISEÑO: Solo agregar fila cuando se escribe en la primera columna (nombre del campo)
        if (column == 0 && !item->text().trimmed().isEmpty()) {
            
            // Si está escribiendo en la última fila, agregar una nueva fila
            if (row == tableWidget->rowCount() - 1) {
                qDebug() << "DEBUG: [Diseño] Agregando nueva fila porque usuario escribió en la última fila";
                
                tableWidget->setRowCount(tableWidget->rowCount() + 1);
                int newRow = tableWidget->rowCount() - 1;
                
                // Crear items vacíos para la nueva fila
                for (int col = 0; col < 3; col++) {
                    QTableWidgetItem *newItem = new QTableWidgetItem("");
                    newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                    tableWidget->setItem(newRow, col, newItem);
                }
                
                qDebug() << "DEBUG: [Diseño] Nueva fila agregada. Total filas:" << tableWidget->rowCount();
            }
        }
        
        // Si el usuario borró el contenido de la primera columna, limpiar filas vacías del final
        if (column == 0 && item->text().trimmed().isEmpty()) {
            while (tableWidget->rowCount() > 2) {
                int lastRow = tableWidget->rowCount() - 1;
                QTableWidgetItem *lastNameItem = tableWidget->item(lastRow, 0);
                QTableWidgetItem *secondLastNameItem = tableWidget->item(lastRow - 1, 0);
                
                if ((!lastNameItem || lastNameItem->text().trimmed().isEmpty()) &&
                    (!secondLastNameItem || secondLastNameItem->text().trimmed().isEmpty())) {
                    tableWidget->setRowCount(tableWidget->rowCount() - 1);
                    qDebug() << "DEBUG: [Diseño] Fila vacía eliminada. Total filas:" << tableWidget->rowCount();
                } else {
                    break;
                }
            }
        }
    }
    else if (isDataView) {
        // VISTA DE DATOS: Agregar fila cuando se escribe en cualquier columna de la última fila
        if (!item->text().trimmed().isEmpty()) {
            
            // Si está escribiendo en la última fila, agregar una nueva fila
            if (row == tableWidget->rowCount() - 1) {
                qDebug() << "DEBUG: [Datos] Agregando nueva fila porque usuario escribió en la última fila";
                
                tableWidget->setRowCount(tableWidget->rowCount() + 1);
                int newRow = tableWidget->rowCount() - 1;
                
                // Crear items vacíos para la nueva fila (todas las columnas)
                for (int col = 0; col < tableWidget->columnCount(); col++) {
                    QTableWidgetItem *newItem = new QTableWidgetItem("");
                    newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                    tableWidget->setItem(newRow, col, newItem);
                }
                
                // Actualizar headers verticales numerados
                QStringList verticalHeaders;
                for (int i = 0; i < tableWidget->rowCount(); i++) {
                    verticalHeaders << QString::number(i + 1);
                }
                tableWidget->setVerticalHeaderLabels(verticalHeaders);
                
                qDebug() << "DEBUG: [Datos] Nueva fila agregada. Total filas:" << tableWidget->rowCount();
            }
        }
        
        // Si el usuario borró contenido, considerar eliminar filas vacías del final
        if (item->text().trimmed().isEmpty()) {
            // Verificar si toda la fila está vacía para posible eliminación
            while (tableWidget->rowCount() > 2) {
                int lastRow = tableWidget->rowCount() - 1;
                int secondLastRow = lastRow - 1;
                
                // Verificar si las dos últimas filas están completamente vacías
                bool lastRowEmpty = true;
                bool secondLastRowEmpty = true;
                
                for (int col = 0; col < tableWidget->columnCount(); col++) {
                    QTableWidgetItem *lastItem = tableWidget->item(lastRow, col);
                    QTableWidgetItem *secondLastItem = tableWidget->item(secondLastRow, col);
                    
                    if (lastItem && !lastItem->text().trimmed().isEmpty()) {
                        lastRowEmpty = false;
                    }
                    if (secondLastItem && !secondLastItem->text().trimmed().isEmpty()) {
                        secondLastRowEmpty = false;
                    }
                }
                
                if (lastRowEmpty && secondLastRowEmpty) {
                    tableWidget->setRowCount(tableWidget->rowCount() - 1);
                    
                    // Actualizar headers verticales
                    QStringList verticalHeaders;
                    for (int i = 0; i < tableWidget->rowCount(); i++) {
                        verticalHeaders << QString::number(i + 1);
                    }
                    tableWidget->setVerticalHeaderLabels(verticalHeaders);
                    
                    qDebug() << "DEBUG: [Datos] Fila vacía eliminada. Total filas:" << tableWidget->rowCount();
                } else {
                    break;
                }
            }
        }
    }
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
QString TableView::getModernInputStyle(){    return         "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 2px solid #E5E7EB;"
            "border-radius: 8px;"
            "padding: 12px 16px;"
            "font-size: 14px;"
            "font-family: 'Inter', system-ui;"
            "color: #1F2937;"
            "selection-background-color: #FDF2F8;"
        "}"
        "QLineEdit:focus {"
            "border: 2px solid #A4373A;"
            "outline: 0px;"
            "background-color: #FFFFFF;"
        "}"
        "QLineEdit:hover {"
            "border: 2px solid #D1D5DB;"
        "}";
}

QString TableView::getModernComboStyle()
{
    return 
        "QComboBox {"
            "background-color: #FFFFFF;"
            "border: 2px solid #E5E7EB;"
            "border-radius: 8px;"
            "padding: 12px 16px;"
            "font-size: 14px;"
            "font-family: 'Inter', system-ui;"
            "color: #1F2937;"
            "min-width: 160px;"
        "}"
        "QComboBox:focus {"
            "border: 2px solid #A4373A;"
            "outline: 0px;"
        "}"
        "QComboBox:hover {"
            "border: 2px solid #D1D5DB;"
        "}";
}

void TableView::setupDesignView()
{
    qDebug() << "=== DEBUG: setupDesignView() INICIADO - RESTAURAR VISTA DISEÑO ===";
    
    try {
        // Bloquear señales temporalmente
        tableWidget->blockSignals(true);
        
        // PASO 1: GUARDAR CAMPOS EXISTENTES SOLO SI VENIMOS DE VISTA DE DATOS
        QStringList existingFields;
        QStringList existingTypes;
        QStringList existingDescriptions;
        
        // Si tenemos más de 3 columnas, significa que venimos de Vista de Datos
        if (tableWidget->columnCount() > 3) {
            qDebug() << "DEBUG: Venimos de Vista de Datos, guardando campos definidos";
            
            // Los headers de Vista de Datos contienen los nombres de campos
            for (int col = 0; col < tableWidget->columnCount(); col++) {
                QTableWidgetItem *header = tableWidget->horizontalHeaderItem(col);
                if (header && !header->text().trimmed().isEmpty()) {
                    existingFields << header->text().trimmed();
                    existingTypes << "text"; // Tipo por defecto
                    existingDescriptions << ""; // Descripción vacía por defecto
                }
            }
        } else if (tableWidget->columnCount() == 3) {
            // Ya estamos en Vista de Diseño, leer campos existentes
            qDebug() << "DEBUG: Ya en Vista de Diseño, leyendo campos existentes";
            for (int row = 0; row < tableWidget->rowCount(); row++) {
                QTableWidgetItem *nameItem = tableWidget->item(row, 0);
                QTableWidgetItem *typeItem = tableWidget->item(row, 1);
                QTableWidgetItem *descItem = tableWidget->item(row, 2);
                
                if (nameItem && !nameItem->text().trimmed().isEmpty()) {
                    existingFields << nameItem->text().trimmed();
                    existingTypes << (typeItem ? typeItem->text().trimmed() : "text");
                    existingDescriptions << (descItem ? descItem->text().trimmed() : "");
                }
            }
        }
        
        // Si no hay campos, usar campo por defecto
        if (existingFields.isEmpty()) {
            existingFields << "Id";
            existingTypes << "int";
            existingDescriptions << "Identificador único";
        }
        
        qDebug() << "DEBUG: Campos a restaurar:" << existingFields.size();
        
        // PASO 2: RECONFIGURAR TABLA PARA VISTA DE DISEÑO
        tableWidget->setColumnCount(3);
        
        QStringList headers;
        headers << "Nombre del Campo" << "Tipo de Datos" << "Descripción (Opcional)";
        tableWidget->setHorizontalHeaderLabels(headers);
        
        // Configurar anchos de columna
        tableWidget->setColumnWidth(0, 250);
        tableWidget->setColumnWidth(1, 180);
        tableWidget->setColumnWidth(2, 400);
        
        tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        
        // PASO 3: RESTAURAR CAMPOS EN VISTA DE DISEÑO
        tableWidget->setRowCount(qMax(existingFields.size(), 1)); // Solo las filas necesarias
        
        for (int row = 0; row < existingFields.size(); row++) {
            // Nombre del Campo
            QTableWidgetItem *nameItem = new QTableWidgetItem(existingFields[row]);
            nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            tableWidget->setItem(row, 0, nameItem);
            
            // Tipo de Datos
            QTableWidgetItem *typeItem = new QTableWidgetItem(existingTypes[row]);
            typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            tableWidget->setItem(row, 1, typeItem);
            
            // Descripción
            QTableWidgetItem *descItem = new QTableWidgetItem(existingDescriptions[row]);
            descItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            tableWidget->setItem(row, 2, descItem);
        }
        
        // Solo agregar UNA fila vacía adicional si la última fila tiene contenido
        bool lastRowHasContent = false;
        int lastRow = tableWidget->rowCount() - 1;
        if (lastRow >= 0) {
            QTableWidgetItem *lastNameItem = tableWidget->item(lastRow, 0);
            if (lastNameItem && !lastNameItem->text().trimmed().isEmpty()) {
                lastRowHasContent = true;
            }
        }
        
        if (lastRowHasContent) {
            // Agregar solo UNA fila vacía para el siguiente campo
            tableWidget->setRowCount(tableWidget->rowCount() + 1);
            int newRow = tableWidget->rowCount() - 1;
            
            for (int col = 0; col < 3; col++) {
                QTableWidgetItem *item = new QTableWidgetItem("");
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                tableWidget->setItem(newRow, col, item);
            }
        }
        
        // Mostrar área de propiedades
        if (propertiesArea) {
            propertiesArea->show();
        }
        
        // Reactivar señales con timer
        QTimer::singleShot(100, this, [this]() {
            tableWidget->blockSignals(false);
            
            // Reconectar señales específicas para vista de diseño
            disconnect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCurrentCellChanged);
            connect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCellSelectionChanged);
            
            qDebug() << "=== DEBUG: setupDesignView() COMPLETADO - VISTA DISEÑO RESTAURADA ===";
        });
        
    } catch (...) {
        qDebug() << "CRITICAL ERROR: Excepción en setupDesignView()";
    }
}

void TableView::setupDataView()
{
    qDebug() << "DEBUG: setupDataView() iniciado - CONFIGURAR VISTA DE DATOS CON CAMPOS";
    
    try {
        // Bloquear señales temporalmente
        tableWidget->blockSignals(true);
        
        // PASO 1: LEER CAMPOS DEFINIDOS EN VISTA DE DISEÑO
        QStringList fieldNames;
        qDebug() << "DEBUG: Leyendo campos definidos en la tabla";
        
        // Si la tabla tiene 3 columnas, estamos en modo diseño - leer campos
        if (tableWidget->columnCount() == 3) {
            // Leer campos fila por fila
            for (int row = 0; row < tableWidget->rowCount(); row++) {
                QTableWidgetItem *nameItem = tableWidget->item(row, 0);
                if (nameItem && !nameItem->text().trimmed().isEmpty()) {
                    fieldNames << nameItem->text().trimmed();
                    qDebug() << "DEBUG: Campo encontrado:" << nameItem->text().trimmed();
                }
            }
        } else {
            // Ya estamos en vista de datos, usar headers existentes
            for (int col = 0; col < tableWidget->columnCount(); col++) {
                QString header = tableWidget->horizontalHeaderItem(col) ? 
                                tableWidget->horizontalHeaderItem(col)->text() : 
                                QString("Campo_%1").arg(col + 1);
                fieldNames << header;
                qDebug() << "DEBUG: Header existente:" << header;
            }
        }
        
        // Si no hay campos, usar campo por defecto
        if (fieldNames.isEmpty()) {
            fieldNames << "Id";
            qDebug() << "DEBUG: Usando campo por defecto: Id";
        }
        
        qDebug() << "DEBUG: Total campos para vista de datos:" << fieldNames.size();
        
        // PASO 2: GUARDAR DATOS EXISTENTES (si hay filas de datos)
        QList<QStringList> existingData;
        if (tableWidget->columnCount() > 3) { // Solo si ya estamos en vista datos
            for (int row = 0; row < tableWidget->rowCount(); row++) {
                QStringList rowData;
                for (int col = 0; col < qMin(tableWidget->columnCount(), fieldNames.size()); col++) {
                    QTableWidgetItem *item = tableWidget->item(row, col);
                    rowData << (item ? item->text() : "");
                }
                existingData << rowData;
            }
            qDebug() << "DEBUG: Guardados" << existingData.size() << "filas de datos existentes";
        }
        
        // PASO 3: RECONFIGURAR TABLA PARA VISTA DE DATOS
        qDebug() << "DEBUG: Reconfigurando tabla para vista de datos";
        
        // Establecer número de columnas basado en los campos
        tableWidget->setColumnCount(fieldNames.size());
        
        // Establecer headers con nombres de campos
        tableWidget->setHorizontalHeaderLabels(fieldNames);
        
        // PASO 4: RESTAURAR DATOS EXISTENTES DE MANERA DINÁMICA
        int existingRows = existingData.size();
        
        // Solo configurar las filas que realmente tienen datos + UNA fila vacía
        int initialRows = qMax(1, existingRows + 1); // Al menos 1 fila para empezar a escribir
        
        tableWidget->setRowCount(initialRows);
        qDebug() << "DEBUG: Configuradas" << initialRows << "filas (" << existingRows << "con datos existentes)";
        
        // Restaurar datos existentes
        for (int row = 0; row < existingRows && row < existingData.size(); row++) {
            const QStringList &rowData = existingData[row];
            for (int col = 0; col < qMin(rowData.size(), fieldNames.size()); col++) {
                QTableWidgetItem *item = new QTableWidgetItem(rowData[col]);
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                tableWidget->setItem(row, col, item);
            }
        }
        
        // Crear solo UNA fila vacía para nuevos datos
        for (int row = existingRows; row < initialRows; row++) {
            for (int col = 0; col < fieldNames.size(); col++) {
                QTableWidgetItem *item = new QTableWidgetItem("");
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                tableWidget->setItem(row, col, item);
            }
        }
        
        // Configurar headers verticales numerados
        QStringList verticalHeaders;
        for (int i = 0; i < initialRows; i++) {
            verticalHeaders << QString::number(i + 1);
        }
        tableWidget->setVerticalHeaderLabels(verticalHeaders);
        
        // Ocultar área de propiedades en vista de datos
        if (propertiesArea) {
            propertiesArea->hide();
        }
        
        // Configurar anchos de columna
        for (int i = 0; i < fieldNames.size(); i++) {
            int width = qMax(100, fieldNames[i].length() * 8 + 30);
            tableWidget->setColumnWidth(i, width);
        }
        
        // Reactivar señales con timer
        QTimer::singleShot(100, this, [this]() {
            tableWidget->blockSignals(false);
            
            // Reconectar señales específicas para vista de datos
            disconnect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCellSelectionChanged);
            connect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCurrentCellChanged);
            
            qDebug() << "DEBUG: setupDataView() completado - VISTA DE DATOS CONFIGURADA";
        });
        
    } catch (...) {
        qDebug() << "ERROR en setupDataView()";
    }
}

void TableView::onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    
    // Solo manejar esto en vista de datos (cuando no hay area de propiedades visible)
    if (propertiesArea && propertiesArea->isVisible()) {
        return; // Estamos en vista de diseño
    }
    
    // Si el usuario navega a una celda que no existe, crear la fila
    if (currentRow >= 0 && currentColumn >= 0) {
        // Si no hay filas o el usuario está navegando a una fila que requiere creación
        if (tableWidget->rowCount() == 0 || currentRow >= tableWidget->rowCount()) {
            
            // Crear filas hasta la fila requerida + 1 fila extra
            int targetRows = qMax(1, currentRow + 2); // Siempre tener al menos 1 fila extra
            
            qDebug() << "DEBUG: Creando filas automáticamente hasta fila" << targetRows;
            
            // Bloquear señales temporalmente para evitar recursión
            tableWidget->blockSignals(true);
            
            int oldRowCount = tableWidget->rowCount();
            tableWidget->setRowCount(targetRows);
            
            // Crear items para las nuevas filas
            for (int row = oldRowCount; row < targetRows; row++) {
                for (int col = 0; col < tableWidget->columnCount(); col++) {
                    QTableWidgetItem *item = new QTableWidgetItem("");
                    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                    tableWidget->setItem(row, col, item);
                }
            }
            
            // Actualizar headers verticales
            QStringList verticalHeaders;
            for (int i = 0; i < tableWidget->rowCount(); i++) {
                verticalHeaders << QString::number(i + 1);
            }
            tableWidget->setVerticalHeaderLabels(verticalHeaders);
            
            // Reactivar señales
            tableWidget->blockSignals(false);
            
            qDebug() << "DEBUG: Filas creadas exitosamente, total:" << tableWidget->rowCount();
        }
    }
}

void TableView::onFieldNameChanged(const QString &text)
{
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < tableWidget->rowCount()) {
        QTableWidgetItem *nameItem = tableWidget->item(currentRow, 0);
        if (!nameItem) {
            nameItem = new QTableWidgetItem();
            tableWidget->setItem(currentRow, 0, nameItem);
        }
        nameItem->setText(text);
    }
}

void TableView::onDescriptionChanged(const QString &text)
{
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < tableWidget->rowCount()) {
        QTableWidgetItem *descItem = tableWidget->item(currentRow, 2);
        if (!descItem) {
            descItem = new QTableWidgetItem();
            tableWidget->setItem(currentRow, 2, descItem);
        }
        descItem->setText(text);
    }
}

void TableView::onRequiredChanged(bool checked)
{
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < tableWidget->rowCount()) {
        // Para el campo "Requerido", podríamos almacenar esta información
        // en una propiedad personalizada del item o en una estructura de datos separada
        QTableWidgetItem *nameItem = tableWidget->item(currentRow, 0);
        if (nameItem) {
            // Almacenar el estado de "requerido" como propiedad del item
            nameItem->setData(Qt::UserRole, checked);
            
            // Opcional: cambiar el estilo visual para indicar que es requerido
            if (checked) {
                nameItem->setFont(QFont(nameItem->font().family(), nameItem->font().pointSize(), QFont::Bold));
            } else {
                nameItem->setFont(QFont(nameItem->font().family(), nameItem->font().pointSize(), QFont::Normal));
            }
        }
    }
}