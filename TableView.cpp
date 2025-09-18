#include "TableView.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QIntValidator>

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
    comboBox->addItems({"Números", "Sí / No", "Texto corto", "Texto largo", "moneda", "fecha"});
    comboBox->setStyleSheet(
        "QComboBox {"
        "background-color: white;"
        "border: 1px solid #D1D5DB;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 14px;"
        "color: #111827;"
        "}"
        "QComboBox:focus {"
        "border-color: #3b82f6;"
        "background-color: white;"
        "outline: none;"
        "}"
        "QComboBox::drop-down {"
        "border: none;"
        "width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "image: none;"
        "border: 2px solid #6b7280;"
        "width: 8px;"
        "height: 8px;"
        "border-top: none;"
        "border-left: none;"
        "margin-right: 5px;"
        "transform: rotate(45deg);"
        "}"
    );
    
    return comboBox;
}

void DataTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        comboBox->setCurrentText(value.isEmpty() ? "TEXTO CORTO" : value);
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
    
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setStyleSheet(
        "QLineEdit {"
        "background-color: white;"
        "border: 1px solid #D1D5DB;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 14px;"
        "color: #111827;"
        "}"
        "QLineEdit:focus {"
        "border-color: #3b82f6;"
        "background-color: white;"
        "outline: none;"
        "}"
    );
    
    return lineEdit;
}

void DescriptionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        lineEdit->setText(value);
    }
}

void DescriptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        model->setData(index, lineEdit->text(), Qt::EditRole);
    }
}

void DescriptionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                             const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QSize DescriptionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(200, 30);
}

void DescriptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

// FieldNameDelegate Implementation
FieldNameDelegate::FieldNameDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *FieldNameDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setStyleSheet(
        "QLineEdit {"
        "background-color: white;"
        "border: 1px solid #D1D5DB;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 14px;"
        "color: #111827;"
        "}"
        "QLineEdit:focus {"
        "border-color: #3b82f6;"
        "background-color: white;"
        "outline: none;"
        "}"
    );
    
    return lineEdit;
}

void FieldNameDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        lineEdit->setText(value);
    }
}

void FieldNameDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        model->setData(index, lineEdit->text(), Qt::EditRole);
    }
}

void FieldNameDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
static QString normalizeDataType(const QString& raw) {
    QString t = raw.trimmed();

    // Normaliza variantes que escribes en la tabla
    if (t.compare("Texto largo / Párrafo", Qt::CaseInsensitive) == 0)
        return "Texto largo";
    if (t.compare("Texto corto (hasta N caracteres)", Qt::CaseInsensitive) == 0
        || t.compare("Texto corto", Qt::CaseInsensitive) == 0)
        return "Texto corto";
    if (t.compare("Sí / No", Qt::CaseInsensitive) == 0
        || t.compare("Si / No", Qt::CaseInsensitive) == 0
        || t.compare("Booleano", Qt::CaseInsensitive) == 0)
        return "Sí / No";
    if (t.compare("Decimal", Qt::CaseInsensitive) == 0
        || t.compare("Decimales", Qt::CaseInsensitive) == 0
        || t.compare("Entero", Qt::CaseInsensitive) == 0
        || t.compare("Números", Qt::CaseInsensitive) == 0
        || t.compare("Numero", Qt::CaseInsensitive) == 0)
        return "Números";
    if (t.compare("Moneda", Qt::CaseInsensitive) == 0
        || t.compare("moneda", Qt::CaseInsensitive) == 0)
        return "moneda";
    if (t.compare("Fecha", Qt::CaseInsensitive) == 0
        || t.compare("fecha", Qt::CaseInsensitive) == 0)
        return "fecha";

    // Por defecto deja igual
    return t;
}

// TableView Implementation - SOLO PARA VISTA DISEÑO
TableView::TableView(QWidget *parent) : QWidget(parent)
{
    // Inicializar variables
    currentSelectedRow = -1;
    primaryKeyRow = -1; // No hay llave primaria inicialmente
    foreignKeyRows.clear(); // No hay foreign keys inicialmente
    uniqueKeyRows.clear(); // No hay campos únicos inicialmente
    isDarkTheme = false;
    currentTableName = "Nueva Tabla";
    
    // Inicializar lista de formatos de moneda
    fieldCurrencyFormats.clear();
    fieldMillaresDecimals.clear();
    fieldTextSizes.clear(); // Inicializar lista de tamaños de texto;
    fieldNumberTypes.clear(); // Inicializar lista de tipos específicos de números
    fieldDateFormats.clear(); // Inicializar lista de formatos de fecha
    
    // Crear la interfaz
    createInterface();
}

void TableView::createInterface()
{
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Crear header
    createHeader();
    
    // Crear splitter vertical (tabla arriba + propiedades abajo)
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setChildrenCollapsible(false); // Evitar que se colapsen completamente
    mainSplitter->setHandleWidth(8); // Hacer el handle más grueso para fácil redimensionamiento
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
        "background-color: #cbd5e1;"
        "border: 1px solid #9ca3af;"
        "}"
        "QSplitter::handle:hover {"
        "background-color: #9ca3af;"
        "}"
    );
    mainLayout->addWidget(mainSplitter);
    
    // Crear área de tabla
    createTableArea();
    
    // Crear área de propiedades
    createPropertiesArea();
    
    // Agregar al splitter
    mainSplitter->addWidget(tableArea);
    mainSplitter->addWidget(propertiesArea);
    
    // Configurar tamaños del splitter (60% tabla, 40% propiedades) para dar más espacio a las propiedades
    mainSplitter->setSizes({400, 300});
    mainSplitter->setStretchFactor(0, 1); // La tabla puede estirarse
    mainSplitter->setStretchFactor(1, 0); // Las propiedades mantienen su tamaño preferido
    
    // Configurar tabla inicial
    setupDesignTable();
}

void TableView::createHeader()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(70);
    headerWidget->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8fafc, stop:1 #e2e8f0);"
        "border-bottom: 2px solid #cbd5e1;"
        "}"
    );
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(30, 15, 25, 15);
    headerLayout->setSpacing(20);
    
    // Título de la tabla (solo el nombre)
    tableNameLabel = new QLabel(currentTableName);
    tableNameLabel->setFont(QFont("Inter", 18, QFont::Bold));
    tableNameLabel->setStyleSheet(
        "QLabel { "
            "color: #1e293b; "
            "padding: 2px 0px; "
            "min-width: 150px; "
        "}"
    );
    tableNameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    tableNameLabel->setWordWrap(false);
    tableNameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    
    headerLayout->addWidget(tableNameLabel);
    headerLayout->addStretch();
    
    // Contenedor para los botones
    QWidget *buttonContainer = new QWidget();
    buttonContainer->setStyleSheet(
        "QWidget {"
        "background: transparent;"
        "border: 1px solid #cbd5e1;"
        "border-radius: 8px;"
        "}"
    );
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    
    // Botón Vista Diseño (activo por defecto)
    QPushButton *designViewBtn = new QPushButton("🎨 Vista Diseño");
    designViewBtn->setFixedSize(135, 38);
    designViewBtn->setCursor(Qt::PointingHandCursor);
    designViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3b82f6, stop:1 #2563eb);"
        "color: white;"
        "border: 1px solid #1d4ed8;"
        "border-top-left-radius: 8px;"
        "border-bottom-left-radius: 8px;"
        "border-top-right-radius: 0px;"
        "border-bottom-right-radius: 0px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2563eb, stop:1 #1d4ed8);"
        "box-shadow: 0 2px 4px rgba(59, 130, 246, 0.3);"
        "}"
    );
    
    // Botón Vista Datos
    dataViewBtn = new QPushButton("📊 Vista Datos");
    dataViewBtn->setFixedSize(135, 38);
    dataViewBtn->setCursor(Qt::PointingHandCursor);
    dataViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f4f6, stop:1 #e5e7eb);"
        "color: #374151;"
        "border: 1px solid #d1d5db;"
        "border-top-left-radius: 0px;"
        "border-bottom-left-radius: 0px;"
        "border-top-right-radius: 8px;"
        "border-bottom-right-radius: 8px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e5e7eb, stop:1 #d1d5db);"
        "color: #1f2937;"
        "box-shadow: 0 2px 4px rgba(107, 114, 128, 0.2);"
        "}"
        "QPushButton:pressed {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d1d5db, stop:1 #9ca3af);"
        "}"
    );
    
    // Agregar botones al contenedor
    buttonLayout->addWidget(designViewBtn);
    buttonLayout->addWidget(dataViewBtn);
    
    // Conectar señales para ambos botones
    connect(dataViewBtn, &QPushButton::clicked, this, &TableView::onDataViewClicked);
    connect(designViewBtn, &QPushButton::clicked, this, &TableView::onDesignViewClicked);
    
    headerLayout->addWidget(buttonContainer);
    
    // Agregar espacio entre botones de vista y botones de fila
    headerLayout->addSpacing(30);
    
    // Contenedor para botones de agregar/eliminar fila
    QWidget *rowButtonContainer = new QWidget();
    rowButtonContainer->setStyleSheet(
        "QWidget {"
        "background: transparent;"
        "}"
    );
    QHBoxLayout *rowButtonLayout = new QHBoxLayout(rowButtonContainer);
    rowButtonLayout->setContentsMargins(0, 0, 0, 0);
    rowButtonLayout->setSpacing(10);
    
    // Botón Agregar Fila
    QPushButton *addRowBtn = new QPushButton("➕ Agregar Fila");
    addRowBtn->setFixedSize(130, 38);
    addRowBtn->setCursor(Qt::PointingHandCursor);
    addRowBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #059669);"
        "color: white;"
        "border: 1px solid #047857;"
        "border-radius: 8px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "padding: 8px 12px;"
        "text-align: left;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #059669, stop:1 #047857);"
        "border: 1px solid #065f46;"
        "transform: translateY(-1px);"
        "box-shadow: 0 4px 8px rgba(16, 185, 129, 0.3);"
        "}"
        "QPushButton:pressed {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #047857, stop:1 #065f46);"
        "transform: translateY(1px);"
        "box-shadow: 0 2px 4px rgba(16, 185, 129, 0.2);"
        "}"
    );
    addRowBtn->setToolTip("Agregar una nueva fila después de la seleccionada");
    
    // Botón Eliminar Fila
    QPushButton *deleteRowBtn = new QPushButton("🗑️ Eliminar Fila");
    deleteRowBtn->setFixedSize(130, 38);
    deleteRowBtn->setCursor(Qt::PointingHandCursor);
    deleteRowBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #dc2626);"
        "color: white;"
        "border: 1px solid #b91c1c;"
        "border-radius: 8px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "padding: 8px 12px;"
        "text-align: left;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dc2626, stop:1 #b91c1c);"
        "border: 1px solid #991b1b;"
        "transform: translateY(-1px);"
        "box-shadow: 0 4px 8px rgba(239, 68, 68, 0.3);"
        "}"
        "QPushButton:pressed {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #b91c1c, stop:1 #991b1b);"
        "transform: translateY(1px);"
        "box-shadow: 0 2px 4px rgba(239, 68, 68, 0.2);"
        "}"
    );
    deleteRowBtn->setToolTip("Eliminar la fila seleccionada");
    
    // Agregar botones al contenedor
    rowButtonLayout->addWidget(addRowBtn);
    rowButtonLayout->addWidget(deleteRowBtn);
    
    // Conectar señales
    connect(addRowBtn, &QPushButton::clicked, this, &TableView::onAddRowClicked);
    connect(deleteRowBtn, &QPushButton::clicked, this, &TableView::onDeleteRowClicked);
    
    headerLayout->addWidget(rowButtonContainer);
    
    mainLayout->addWidget(headerWidget);
}

void TableView::createTableArea()
{
    tableArea = new QWidget();
    QVBoxLayout *tableLayout = new QVBoxLayout(tableArea);
    tableLayout->setContentsMargins(10, 10, 10, 10);
    
    // Crear tabla
    tableWidget = new QTableWidget();
    tableWidget->setColumnCount(3);
    tableWidget->setRowCount(1); // Solo una fila inicial
    
    // Headers
    QStringList headers;
    headers << "Nombre del Campo" << "Tipo de Datos" << "Descripción";
    tableWidget->setHorizontalHeaderLabels(headers);
    
    // Configurar anchos - más compactos como TableData
    tableWidget->setColumnWidth(0, 150);
    tableWidget->setColumnWidth(1, 120);
    // La columna de descripción se estirará para llenar el espacio restante
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    
    // Configurar altura de filas - más compacta como TableData
    tableWidget->verticalHeader()->setDefaultSectionSize(35);
    tableWidget->verticalHeader()->setMinimumSectionSize(35);
    
    // Estilo de la tabla
    tableWidget->setStyleSheet(getTableStyle());
    
    // Configurar comportamiento
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setAlternatingRowColors(true);
    
    // Conectar señales
    connect(tableWidget, &QTableWidget::cellChanged, this, &TableView::onCellChanged);
    connect(tableWidget, &QTableWidget::currentCellChanged, this, &TableView::onCellSelectionChanged);
    
    tableLayout->addWidget(tableWidget);
}

void TableView::createPropertiesArea()
{
    propertiesArea = new QWidget();
    propertiesArea->setMinimumHeight(300); // Cambiar de setFixedHeight a setMinimumHeight para que sea redimensionable
    propertiesArea->setStyleSheet(
        "QWidget {"
        "background-color: #f8fafc;"
        "border-top: 1px solid #e2e8f0;"
        "}"
    );
    
    QHBoxLayout *propLayout = new QHBoxLayout(propertiesArea);
    propLayout->setContentsMargins(10, 10, 10, 10); // Reducir márgenes para más espacio
    propLayout->setSpacing(20);
    
    // Crear scroll area para el contenido
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
        "background-color: #f1f5f9;"
        "width: 12px;"
        "border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "background-color: #cbd5e1;"
        "border-radius: 6px;"
        "min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "background-color: #9ca3af;"
        "}"
    );
    
    // Título y formulario en un contenedor que irá dentro del scroll area
    QWidget *formContainer = new QWidget();
    QVBoxLayout *formMainLayout = new QVBoxLayout(formContainer);
    formMainLayout->setSpacing(15);
    
    // Título
    QLabel *title = new QLabel("Propiedades del Campo");
    title->setFont(QFont("Arial", 14, QFont::Bold));
    title->setStyleSheet("QLabel { color: #1e293b; margin-bottom: 10px; }");
    formMainLayout->addWidget(title);
    
    // Contenedor horizontal para los campos
    QWidget *fieldsContainer = new QWidget();
    QHBoxLayout *fieldsLayout = new QHBoxLayout(fieldsContainer);
    fieldsLayout->setSpacing(30);
    
    // Columna izquierda
    QVBoxLayout *leftColumn = new QVBoxLayout();
    leftColumn->setSpacing(10);
    
    // Nombre del campo
    QLabel *nameLabel = new QLabel("Nombre:");
    nameLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    leftColumn->addWidget(nameLabel);
    
    fieldNameEdit = new QLineEdit();
    fieldNameEdit->setStyleSheet(getInputStyle());
    fieldNameEdit->setPlaceholderText("Ingrese el nombre del campo");
    leftColumn->addWidget(fieldNameEdit);
    
    // Tipo de datos
    QLabel *typeLabel = new QLabel("Tipo de Datos:");
    typeLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    leftColumn->addWidget(typeLabel);
    
    dataTypeCombo = new QComboBox();
    dataTypeCombo->addItems({"Números", "Sí / No", "Texto corto", "Texto largo", "moneda", "fecha"});
    dataTypeCombo->setStyleSheet(getComboStyle());
    leftColumn->addWidget(dataTypeCombo);
    
    // Columna centro
    QVBoxLayout *centerColumn = new QVBoxLayout();
    centerColumn->setSpacing(10);
    
    // Valor por defecto
    QLabel *defaultLabel = new QLabel("Valor por Defecto:");
    defaultLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    centerColumn->addWidget(defaultLabel);
    
    defaultValueEdit = new QLineEdit();
    defaultValueEdit->setStyleSheet(getInputStyle());
    defaultValueEdit->setPlaceholderText("Valor opcional por defecto");
    centerColumn->addWidget(defaultValueEdit);
    
    // Campo requerido
    requiredCheck = new QCheckBox("Primary Key");
    requiredCheck->setStyleSheet(
        "QCheckBox {"
        "color: #475569;"
        "font-weight: bold;"
        "margin-top: 20px;"
        "}"
        "QCheckBox::indicator {"
        "width: 18px;"
        "height: 18px;"
        "}"
    );
    centerColumn->addWidget(requiredCheck);
    
    // Campo Foreign Key
    foreignKeyCheck = new QCheckBox("Foreign Key");
    foreignKeyCheck->setStyleSheet(
        "QCheckBox {"
        "color: #475569;"
        "font-weight: bold;"
        "margin-top: 10px;"
        "}"
        "QCheckBox::indicator {"
        "width: 18px;"
        "height: 18px;"
        "}"
    );
    centerColumn->addWidget(foreignKeyCheck);
    
    // Campo Unique
    uniqueCheck = new QCheckBox("Unique");
    uniqueCheck->setStyleSheet(
        "QCheckBox {"
        "color: #475569;"
        "font-weight: bold;"
        "margin-top: 10px;"
        "}"
        "QCheckBox::indicator {"
        "width: 18px;"
        "height: 18px;"
        "}"
    );
    centerColumn->addWidget(uniqueCheck);
    
    // Columna derecha
    QVBoxLayout *rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(10);
    
    // Descripción
    QLabel *descLabel = new QLabel("Descripción:");
    descLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    rightColumn->addWidget(descLabel);
    
    descriptionEdit = new QTextEdit();
    descriptionEdit->setMaximumHeight(100);
    descriptionEdit->setStyleSheet(getTextEditStyle());
    descriptionEdit->setPlaceholderText("Descripción del campo");
    rightColumn->addWidget(descriptionEdit);
    
    // Agregar columnas al contenedor de campos
    fieldsLayout->addLayout(leftColumn);
    fieldsLayout->addLayout(centerColumn);
    fieldsLayout->addLayout(rightColumn);
    
    formMainLayout->addWidget(fieldsContainer);
    
    // Crear widgets para propiedades específicas
    createSpecificPropertiesWidgets();
    formMainLayout->addWidget(specificPropertiesWidget);
    
    // Configurar el scroll area
    scrollArea->setWidget(formContainer);
    propLayout->addWidget(scrollArea);
    
    // Conectar señales
    connect(fieldNameEdit, &QLineEdit::textChanged, this, &TableView::onFieldNameChanged);
    connect(dataTypeCombo, &QComboBox::currentTextChanged, this, &TableView::onDataTypeChanged);
    connect(defaultValueEdit, &QLineEdit::textChanged, this, &TableView::onDefaultValueChanged);
    connect(requiredCheck, &QCheckBox::toggled, this, &TableView::onRequiredChanged);
    connect(foreignKeyCheck, &QCheckBox::toggled, this, &TableView::onForeignKeyChanged);
    connect(uniqueCheck, &QCheckBox::toggled, this, &TableView::onUniqueChanged);
    connect(descriptionEdit, &QTextEdit::textChanged, this, &TableView::onDescriptionChanged);
}

void TableView::setupDesignTable()
{
    // Limpiar tabla
    tableWidget->clearContents();
    
    // Crear delegates
    fieldNameDelegate = new FieldNameDelegate(this);
    dataTypeDelegate = new DataTypeDelegate(this);
    descriptionDelegate = new DescriptionDelegate(this);
    
    tableWidget->setItemDelegateForColumn(0, fieldNameDelegate);
    tableWidget->setItemDelegateForColumn(1, dataTypeDelegate);
    tableWidget->setItemDelegateForColumn(2, descriptionDelegate);
    
    // Crear solo la primera fila con el campo ID
    for (int col = 0; col < tableWidget->columnCount(); col++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        
        // Configurar fuente más grande para mejor legibilidad
        QFont itemFont = item->font();
        itemFont.setPointSize(14);
        item->setFont(itemFont);
        
        // Todas las celdas son editables desde el inicio
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        
        if (col == 0) {
            item->setText("Id"); // Campo ID por defecto
            item->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco
        } else if (col == 1) {
            item->setText("Números"); // Tipo por defecto para ID
            item->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco
        } else {
            // Columna de descripción también habilitada
            item->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco
        }
        
        tableWidget->setItem(0, col, item);
    }
    
    // Conectar señal para habilitación progresiva
    connect(tableWidget, &QTableWidget::itemChanged, this, &TableView::onFieldItemChanged);
}

// Event Handlers
void TableView::onCellChanged(int row, int column)
{
    // Mantén tu lógica actual para nombre (crear fila vacía, etc.)
    if (column == 0) {
        ensureEmptyRowExists();
    }

    // Si no es la fila seleccionada, no sincronizamos propiedades
    if (row != currentSelectedRow) return;

    // Nombre → fieldNameEdit
    if (column == 0) {
        if (auto *nameItem = tableWidget->item(row, 0)) {
            QString name = nameItem->text();
            // Limpia emojis si los tuviera:
            if (name.startsWith("🔑🔗 ")) name = name.mid(4);
            else if (name.startsWith("🔑 ")) name = name.mid(3);
            else if (name.startsWith("🔗 ")) name = name.mid(3);

            fieldNameEdit->blockSignals(true);
            fieldNameEdit->setText(name);
            fieldNameEdit->blockSignals(false);
        }
    }

    // Tipo → dataTypeCombo + propiedades específicas
    // --- Sincronizar: cuando el tipo cambia en la tabla, reflejarlo en Propiedades ---
    if (column == 1) {
        // Asegura que la fila actual sea la seleccionada (por si el editor no movió selección)
        currentSelectedRow = row;

        if (auto *typeItem = tableWidget->item(row, 1)) {
            const QString norm = normalizeDataType(typeItem->text());

            // Actualiza el combo de propiedades sin disparar su slot
            dataTypeCombo->blockSignals(true);
            int idx = dataTypeCombo->findText(norm, Qt::MatchExactly);
            if (idx >= 0) dataTypeCombo->setCurrentIndex(idx);
            dataTypeCombo->blockSignals(false);

            // Muestra el panel específico correcto
            updateSpecificProperties(norm);
        }
    }


    // Descripción → descriptionEdit
    if (column == 2) {
        if (auto *descItem = tableWidget->item(row, 2)) {
            descriptionEdit->blockSignals(true);
            descriptionEdit->setPlainText(descItem->text());
            descriptionEdit->blockSignals(false);
        }
    }

    // Mantén tu emisión si la necesitas
    // emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onCellSelectionChanged()
{
    currentSelectedRow = tableWidget->currentRow();
    
    // Validar integridad de llave primaria
    validatePrimaryKeyIntegrity();
    
    if (currentSelectedRow >= 0) {
        updatePropertiesForRow(currentSelectedRow);
    }
}

void TableView::updatePropertiesForRow(int row)
{
    // Bloquear señales
    fieldNameEdit->blockSignals(true);
    dataTypeCombo->blockSignals(true);
    descriptionEdit->blockSignals(true);
    defaultValueEdit->blockSignals(true);
    requiredCheck->blockSignals(true);
    foreignKeyCheck->blockSignals(true);
    uniqueCheck->blockSignals(true);

    // Obtener datos de la fila
    QTableWidgetItem *nameItem = tableWidget->item(row, 0);
    QTableWidgetItem *typeItem = tableWidget->item(row, 1);
    QTableWidgetItem *descItem = tableWidget->item(row, 2);

    // Nombre (limpiar iconos 🔑 / 🔗 / 🔶)
    QString displayName = nameItem ? nameItem->text() : "";
    if (displayName.startsWith("🔑🔗🔶 ")) displayName = displayName.mid(5);
    else if (displayName.startsWith("🔑🔗 ")) displayName = displayName.mid(4);
    else if (displayName.startsWith("🔑🔶 ")) displayName = displayName.mid(4);
    else if (displayName.startsWith("🔗🔶 ")) displayName = displayName.mid(4);
    else if (displayName.startsWith("🔑 "))   displayName = displayName.mid(3);
    else if (displayName.startsWith("🔗 "))   displayName = displayName.mid(3);
    else if (displayName.startsWith("🔶 "))   displayName = displayName.mid(3);
    fieldNameEdit->setText(displayName);

    // Tipo (normalizado) → combo
    QString dataType = typeItem ? normalizeDataType(typeItem->text()) : QString("Texto largo");
    int idx = dataTypeCombo->findText(dataType, Qt::MatchExactly);
    if (idx >= 0) dataTypeCombo->setCurrentIndex(idx);

    // Descripción
    descriptionEdit->setPlainText(descItem ? descItem->text() : "");

    // Valor por defecto (por ahora no se usa, pero mantenemos limpio el UI)
    defaultValueEdit->setText("");

    // Checks PK / FK / Unique
    const bool isPrimaryKey = (primaryKeyRow == row);
    const bool isForeignKey = foreignKeyRows.contains(row);
    const bool isUnique     = uniqueKeyRows.contains(row);
    requiredCheck->setChecked(isPrimaryKey);
    foreignKeyCheck->setChecked(isForeignKey);
    uniqueCheck->setChecked(isUnique);

    // Propiedades específicas según tipo
    updateSpecificProperties(dataType);

    // Reactivar señales
    fieldNameEdit->blockSignals(false);
    dataTypeCombo->blockSignals(false);
    descriptionEdit->blockSignals(false);
    defaultValueEdit->blockSignals(false);
    requiredCheck->blockSignals(false);
    foreignKeyCheck->blockSignals(false);
    uniqueCheck->blockSignals(false);
}

void TableView::onFieldNameChanged(const QString &text)
{
    if (currentSelectedRow >= 0) {
        QTableWidgetItem *item = tableWidget->item(currentSelectedRow, 0);
        if (item) {
            if (!text.isEmpty()) {
                // Verificar el estado de Primary Key y Foreign Key
                bool isPrimaryKey = (primaryKeyRow == currentSelectedRow);
                bool isForeignKey = foreignKeyRows.contains(currentSelectedRow);
                
                // Aplicar los iconos apropiados
                if (isPrimaryKey && isForeignKey) {
                    item->setText("🔑🔗 " + text);
                    item->setToolTip("Campo Primary Key con Foreign Key - Clave única que también referencia otra tabla");
                } else if (isPrimaryKey) {
                    item->setText("🔑 " + text);
                    item->setToolTip("Campo Llave Primaria - Requerido y único");
                } else if (isForeignKey) {
                    item->setText("🔗 " + text);
                    item->setToolTip("Campo Foreign Key - Referencia a otra tabla");
                } else {
                    item->setText(text);
                    item->setToolTip("");
                }
            } else {
                item->setText(text);
                item->setToolTip("");
            }
        }
        ensureEmptyRowExists();
    }
}

void TableView::onDataTypeChanged(const QString &dataType)
{
    if (currentSelectedRow >= 0) {
        QTableWidgetItem *item = tableWidget->item(currentSelectedRow, 1);
        if (item) {
            item->setText(dataType);
        }
    }
    
    // Actualizar propiedades específicas según el tipo seleccionado
    updateSpecificProperties(dataType);
    
    // Emitir señal para actualizar vista de datos (no actualizar ejemplos en vista diseño)
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onDescriptionChanged()
{
    if (currentSelectedRow >= 0) {
        QTableWidgetItem *item = tableWidget->item(currentSelectedRow, 2);
        if (item) {
            item->setText(descriptionEdit->toPlainText());
        }
    }
}

void TableView::onDefaultValueChanged(const QString &value)
{
    Q_UNUSED(value)
    // Por ahora no guardamos el valor por defecto en la tabla
}

void TableView::onRequiredChanged(bool required)
{
    if (currentSelectedRow < 0) return;
    
    if (required) {
        // Si se está intentando marcar como requerido (llave primaria)
        if (primaryKeyRow != -1 && primaryKeyRow != currentSelectedRow) {
            // Ya existe otra llave primaria
            QTableWidgetItem *existingPrimaryKeyItem = tableWidget->item(primaryKeyRow, 0);
            QString existingFieldName = existingPrimaryKeyItem ? existingPrimaryKeyItem->text() : QString("Fila %1").arg(primaryKeyRow + 1);
            
            // Remover icono de llave del nombre para mostrar mensaje más limpio
            if (existingFieldName.startsWith("🔑 ")) {
                existingFieldName = existingFieldName.mid(3);
            }
            
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("🔑 Llave Primaria Duplicada");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(QString("<h3>Solo Una Llave Primaria</h3>"));
            msgBox.setInformativeText(QString("Ya existe una llave primaria en el campo <b>'%1'</b>.<br><br>"
                                             "⚠️ <b>Restricción:</b> Solo puede haber una llave primaria por tabla.<br><br>"
                                             "Para cambiar la llave primaria, primero desmarque el campo existente.")
                                             .arg(existingFieldName));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.button(QMessageBox::Ok)->setText("Entendido");
            
            // Estilo simple y muy visible
            msgBox.setStyleSheet(
                "QMessageBox {"
                    "background-color: white;"
                    "min-width: 400px;"
                    "min-height: 200px;"
                "}"
                "QMessageBox QLabel {"
                    "color: black;"
                    "font-size: 16px;"
                "}"
                "QPushButton {"
                    "background-color: blue;"
                    "color: white;"
                    "font-size: 16px;"
                    "font-weight: bold;"
                    "min-width: 120px;"
                    "min-height: 50px;"
                    "border: 2px solid black;"
                    "padding: 10px;"
                "}"
                "QPushButton:hover {"
                    "background-color: darkblue;"
                "}"
            );
            
            msgBox.exec();
            
            // Desmarcar el checkbox sin activar la señal
            requiredCheck->blockSignals(true);
            requiredCheck->setChecked(false);
            requiredCheck->blockSignals(false);
            return;
        }
        
        // Marcar este campo como llave primaria
        primaryKeyRow = currentSelectedRow;
        
        // Agregar el icono de llave al nombre del campo en la tabla
        QTableWidgetItem *fieldNameItem = tableWidget->item(currentSelectedRow, 0);
        if (fieldNameItem) {
            QString fieldName = fieldNameItem->text();
            // Obtener el nombre limpio del campo
            QString cleanFieldName = fieldName;
            
            // Remover todos los iconos posibles de manera robusta
            bool hasChanges = true;
            while (hasChanges) {
                hasChanges = false;
                
                if (cleanFieldName.startsWith("��🔗 ")) {
                    cleanFieldName = cleanFieldName.mid(4);
                    hasChanges = true;
                } else if (cleanFieldName.startsWith("� ")) {
                    cleanFieldName = cleanFieldName.mid(3);
                    hasChanges = true;
                } else if (cleanFieldName.startsWith("� ")) {
                    cleanFieldName = cleanFieldName.mid(3);
                    hasChanges = true;
                }
            }
            
            // Limpiar espacios extra
            cleanFieldName = fieldName;
            
            // Método simple y robusto para limpiar todos los iconos
            cleanFieldName = cleanFieldName.remove("🔑🔗🔶");
            cleanFieldName = cleanFieldName.remove("🔑🔗");
            cleanFieldName = cleanFieldName.remove("🔑🔶");
            cleanFieldName = cleanFieldName.remove("🔗🔶");
            cleanFieldName = cleanFieldName.remove("🔑");  
            cleanFieldName = cleanFieldName.remove("🔗");
            cleanFieldName = cleanFieldName.remove("🔶");
            cleanFieldName = cleanFieldName.trimmed();
            
            // Verificar si también es Foreign Key y/o Unique
            bool isForeignKey = foreignKeyRows.contains(currentSelectedRow);
            bool isUnique = uniqueKeyRows.contains(currentSelectedRow);
            
            // Construir el texto con todos los iconos apropiados
            QString newText = cleanFieldName;
            QString toolTip = "";
            
            if (isForeignKey && isUnique) {
                // PK + FK + Unique
                newText = "🔑🔗🔶 " + cleanFieldName;
                toolTip = "Campo Primary Key, Foreign Key y Unique";
            } else if (isForeignKey) {
                // PK + FK
                newText = "🔑🔗 " + cleanFieldName;
                toolTip = "Campo Primary Key y Foreign Key - Clave única que referencia otra tabla";
            } else if (isUnique) {
                // PK + Unique
                newText = "🔑🔶 " + cleanFieldName;
                toolTip = "Campo Primary Key y Unique";
            } else {
                // Solo Primary Key
                newText = "🔑 " + cleanFieldName;
                toolTip = "Campo Llave Primaria - Requerido y único";
            }
            
            // Bloquear señales del tableWidget para evitar bucles infinitos
            tableWidget->blockSignals(true);
            fieldNameItem->setText(newText);
            fieldNameItem->setToolTip(toolTip);
            tableWidget->blockSignals(false);
            
            qDebug() << "DEBUG: Campo marcado como llave primaria:" << cleanFieldName << "en fila:" << currentSelectedRow;
        }
        
    } else {
        // Si se está desmarcando como requerido
        if (primaryKeyRow == currentSelectedRow) {
            // Remover la llave primaria
            primaryKeyRow = -1;
            
            // Remover el icono de llave del nombre del campo
            QTableWidgetItem *fieldNameItem = tableWidget->item(currentSelectedRow, 0);
            if (fieldNameItem) {
                QString fieldName = fieldNameItem->text();
                // Obtener el nombre limpio del campo
                QString cleanFieldName = fieldName;
                
                // Limpiar todos los iconos
                cleanFieldName = cleanFieldName.remove("🔑🔗🔶");
                cleanFieldName = cleanFieldName.remove("🔑🔗");
                cleanFieldName = cleanFieldName.remove("🔑🔶");
                cleanFieldName = cleanFieldName.remove("🔗🔶");
                cleanFieldName = cleanFieldName.remove("🔑");  
                cleanFieldName = cleanFieldName.remove("🔗");
                cleanFieldName = cleanFieldName.remove("🔶");
                cleanFieldName = cleanFieldName.trimmed();
                
                // Verificar si también era Foreign Key y/o Unique
                bool wasForeignKey = foreignKeyRows.contains(currentSelectedRow);
                bool wasUnique = uniqueKeyRows.contains(currentSelectedRow);
                
                // Actualizar el campo según corresponda
                QString newText = cleanFieldName;
                QString toolTip = "";
                
                if (wasForeignKey && wasUnique) {
                    // Era PK + FK + Unique, ahora es FK + Unique
                    newText = "🔗🔶 " + cleanFieldName;
                    toolTip = "Campo Foreign Key y Unique";
                } else if (wasForeignKey) {
                    // Era PK + FK, ahora es solo FK
                    newText = "🔗 " + cleanFieldName;
                    toolTip = "Campo Foreign Key - Referencia a otra tabla";
                } else if (wasUnique) {
                    // Era PK + Unique, ahora es solo Unique
                    newText = "🔶 " + cleanFieldName;
                    toolTip = "Campo Unique - Valores únicos, no se permiten duplicados";
                }
                
                // Bloquear señales del tableWidget para evitar bucles infinitos
                tableWidget->blockSignals(true);
                fieldNameItem->setText(newText);
                fieldNameItem->setToolTip(toolTip);
                tableWidget->blockSignals(false);
                
                qDebug() << "DEBUG: Llave primaria removida del campo:" << cleanFieldName;
            }
        }
    }
}

void TableView::onForeignKeyChanged(bool isForeignKey)
{
    if (currentSelectedRow < 0) return;
    
    QTableWidgetItem *fieldNameItem = tableWidget->item(currentSelectedRow, 0);
    if (!fieldNameItem) return;
    
    QString fieldName = fieldNameItem->text();
    
    // Obtener el nombre limpio del campo (sin iconos)
    QString cleanFieldName = fieldName;
    
    // Remover todos los iconos posibles de manera robusta
    bool hasChanges = true;
    while (hasChanges) {
        hasChanges = false;
        
        if (cleanFieldName.startsWith("��🔗 ")) {
            cleanFieldName = cleanFieldName.mid(4);
            hasChanges = true;
        } else if (cleanFieldName.startsWith("� ")) {
            cleanFieldName = cleanFieldName.mid(3);
            hasChanges = true;
        } else if (cleanFieldName.startsWith("� ")) {
            cleanFieldName = cleanFieldName.mid(3);
            hasChanges = true;
        }
    }
    
    // Limpiar espacios extra
    cleanFieldName = fieldName;
    
    // Método simple y robusto para limpiar todos los iconos
    cleanFieldName = cleanFieldName.remove("🔑🔗🔶");
    cleanFieldName = cleanFieldName.remove("🔑🔗");
    cleanFieldName = cleanFieldName.remove("🔑🔶");
    cleanFieldName = cleanFieldName.remove("🔗🔶");
    cleanFieldName = cleanFieldName.remove("🔑");  
    cleanFieldName = cleanFieldName.remove("🔗");
    cleanFieldName = cleanFieldName.remove("🔶");
    cleanFieldName = cleanFieldName.trimmed();
    
    // Verificar si este campo es también Primary Key y/o Unique
    bool isPrimaryKey = (primaryKeyRow == currentSelectedRow);
    bool isUnique = uniqueKeyRows.contains(currentSelectedRow);
    
    if (isForeignKey) {
        // Agregar a la lista de Foreign Keys si no está ya
        if (!foreignKeyRows.contains(currentSelectedRow)) {
            foreignKeyRows.append(currentSelectedRow);
        }
        
        // Mostrar el icono apropiado
        QString newText = cleanFieldName;
        QString toolTip = "";
        
        if (isPrimaryKey && isUnique) {
            // PK + FK + Unique
            newText = "🔑🔗🔶 " + cleanFieldName;
            toolTip = "Campo Primary Key, Foreign Key y Unique";
        } else if (isPrimaryKey) {
            // PK + FK
            newText = "🔑🔗 " + cleanFieldName;
            toolTip = "Campo Primary Key con Foreign Key - Clave única que también referencia otra tabla";
        } else if (isUnique) {
            // FK + Unique
            newText = "🔗🔶 " + cleanFieldName;
            toolTip = "Campo Foreign Key y Unique - Referencia única a otra tabla";
        } else {
            // Solo Foreign Key
            newText = "🔗 " + cleanFieldName;
            toolTip = "Campo Foreign Key - Referencia a otra tabla";
        }
        
        // Bloquear señales del tableWidget para evitar bucles infinitos
        tableWidget->blockSignals(true);
        fieldNameItem->setText(newText);
        fieldNameItem->setToolTip(toolTip);
        tableWidget->blockSignals(false);
        
        qDebug() << "DEBUG: Campo marcado como Foreign Key:" << cleanFieldName << "en fila:" << currentSelectedRow;
        
    } else {
        // Remover de la lista de Foreign Keys
        foreignKeyRows.removeAll(currentSelectedRow);
        
        // Remover el icono de foreign key
        QString newText = cleanFieldName;
        QString toolTip = "";
        
        if (isPrimaryKey && isUnique) {
            // Era PK + FK + Unique, ahora es PK + Unique
            newText = "🔑🔶 " + cleanFieldName;
            toolTip = "Campo Primary Key y Unique";
        } else if (isPrimaryKey) {
            // Era PK + FK, ahora es solo PK
            newText = "🔑 " + cleanFieldName;
            toolTip = "Campo Primary Key - Clave única e irrepetible";
        } else if (isUnique) {
            // Era FK + Unique, ahora es solo Unique
            newText = "🔶 " + cleanFieldName;
            toolTip = "Campo Unique - Valores únicos, no se permiten duplicados";
        }
        // Si no es ni PK ni Unique, se queda solo con el nombre limpio
        
        // Bloquear señales del tableWidget para evitar bucles infinitos
        tableWidget->blockSignals(true);
        fieldNameItem->setText(newText);
        fieldNameItem->setToolTip(toolTip);
        tableWidget->blockSignals(false);
        
        qDebug() << "DEBUG: Foreign Key removida del campo:" << cleanFieldName;
        
        // Emitir señal para notificar que se eliminó una Foreign Key
        emit foreignKeyRemoved(currentTableName, cleanFieldName);
    }
}

void TableView::onUniqueChanged(bool isUnique)
{
    if (currentSelectedRow < 0) {
        qDebug() << "DEBUG: No hay fila seleccionada para cambiar unique";
        return;
    }
    
    if (!tableWidget) {
        qDebug() << "ERROR: tableWidget is null";
        return;
    }
    
    if (currentSelectedRow >= tableWidget->rowCount()) {
        qDebug() << "ERROR: currentSelectedRow out of bounds:" << currentSelectedRow << "rowCount:" << tableWidget->rowCount();
        return;
    }
    
    QTableWidgetItem *fieldNameItem = tableWidget->item(currentSelectedRow, 0);
    if (!fieldNameItem) {
        qDebug() << "ERROR: fieldNameItem is null for row:" << currentSelectedRow;
        return;
    }
    
    QString fieldName = fieldNameItem->text();
    
    // Verificar que el campo tenga un nombre
    if (fieldName.trimmed().isEmpty()) {
        qDebug() << "DEBUG: Campo sin nombre, no se puede marcar como unique";
        // Revertir el checkbox
        uniqueCheck->blockSignals(true);
        uniqueCheck->setChecked(false);
        uniqueCheck->blockSignals(false);
        return;
    }
    
    // Obtener el nombre limpio del campo (sin iconos)
    QString cleanFieldName = fieldName;
    
    // Método robusto para limpiar todos los iconos posibles
    cleanFieldName = cleanFieldName.remove("🔑🔗");
    cleanFieldName = cleanFieldName.remove("🔑");  
    cleanFieldName = cleanFieldName.remove("🔗");
    cleanFieldName = cleanFieldName.remove("🔶");  // Icono para unique
    cleanFieldName = cleanFieldName.trimmed();
    
    // Verificar si este campo es también Primary Key y/o Foreign Key
    bool isPrimaryKey = (primaryKeyRow == currentSelectedRow);
    bool isForeignKey = foreignKeyRows.contains(currentSelectedRow);
    
    if (isUnique) {
        qDebug() << "DEBUG: Marcando campo como unique:" << cleanFieldName;
        
        // Agregar a la lista de campos únicos si no está ya
        if (!uniqueKeyRows.contains(currentSelectedRow)) {
            uniqueKeyRows.append(currentSelectedRow);
            qDebug() << "DEBUG: Agregado a uniqueKeyRows:" << currentSelectedRow;
        }
        
        // Validar que no haya duplicados si este campo ya tiene datos
        qDebug() << "DEBUG: Verificando duplicados...";
        bool hasDuplicates = checkForDuplicates(cleanFieldName);
        qDebug() << "DEBUG: hasDuplicates =" << hasDuplicates;
        if (hasDuplicates) {
            // Mostrar mensaje de advertencia
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("⚠️ Datos Duplicados");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("<h3>Campo Unique con Datos Duplicados</h3>");
            msgBox.setInformativeText(
                QString("El campo <b>'%1'</b> contiene datos duplicados.<br><br>"
                        "⚠️ <b>Los campos marcados como Unique no pueden tener valores repetidos.</b><br><br>"
                        "Puede:<br>"
                        "• Quitar la marca de Unique para permitir duplicados<br>"
                        "• Modificar los datos para que sean únicos").arg(cleanFieldName)
            );
            msgBox.setStandardButtons(QMessageBox::Ok);
            
            // Estilo del mensaje
            msgBox.setStyleSheet(
                "QMessageBox {"
                "background-color: white;"
                "min-width: 460px;"
                "min-height: 180px;"
                "}"
                "QMessageBox QLabel {"
                "color: #0b0f19;"
                "font-size: 14px;"
                "}"
            );
            
            msgBox.exec();
        }
        
        // Mostrar el icono apropiado combinando con PK y FK
        QString newText = cleanFieldName;
        QString toolTip = "";
        
        if (isPrimaryKey && isForeignKey) {
            // PK + FK + Unique
            newText = "🔑🔗🔶 " + cleanFieldName;
            toolTip = "Campo Primary Key, Foreign Key y Unique - Clave única que también referencia otra tabla";
        } else if (isPrimaryKey) {
            // PK + Unique (redundante pero lo mostramos)
            newText = "🔑🔶 " + cleanFieldName;
            toolTip = "Campo Primary Key y Unique - Clave única e irrepetible";
        } else if (isForeignKey) {
            // FK + Unique
            newText = "🔗🔶 " + cleanFieldName;
            toolTip = "Campo Foreign Key y Unique - Referencia única a otra tabla";
        } else {
            // Solo Unique
            newText = "🔶 " + cleanFieldName;
            toolTip = "Campo Unique - Valores únicos, no se permiten duplicados";
        }
        
        // Bloquear señales del tableWidget para evitar bucles infinitos
        tableWidget->blockSignals(true);
        fieldNameItem->setText(newText);
        fieldNameItem->setToolTip(toolTip);
        tableWidget->blockSignals(false);
        
        qDebug() << "DEBUG: Campo marcado como Unique:" << cleanFieldName << "en fila:" << currentSelectedRow;
        
    } else {
        qDebug() << "DEBUG: Desmarcando campo unique:" << cleanFieldName;
        
        // Remover de la lista de campos únicos
        uniqueKeyRows.removeAll(currentSelectedRow);
        qDebug() << "DEBUG: Removido de uniqueKeyRows:" << currentSelectedRow;
        
        // Remover el icono de unique y actualizar el texto
        QString newText = cleanFieldName;
        QString toolTip = "";
        
        if (isPrimaryKey && isForeignKey) {
            // PK + FK (sin Unique)
            newText = "🔑🔗 " + cleanFieldName;
            toolTip = "Campo Primary Key con Foreign Key - Clave única que también referencia otra tabla";
        } else if (isPrimaryKey) {
            // Solo PK
            newText = "🔑 " + cleanFieldName;
            toolTip = "Campo Primary Key - Clave única e irrepetible";
        } else if (isForeignKey) {
            // Solo FK
            newText = "🔗 " + cleanFieldName;
            toolTip = "Campo Foreign Key - Referencia a otra tabla";
        } else {
            // Sin propiedades especiales
            newText = cleanFieldName;
            toolTip = "";
        }
        
        // Bloquear señales del tableWidget para evitar bucles infinitos
        tableWidget->blockSignals(true);
        fieldNameItem->setText(newText);
        fieldNameItem->setToolTip(toolTip);
        tableWidget->blockSignals(false);
        
        qDebug() << "DEBUG: Unique removido del campo:" << cleanFieldName;
    }
}

void TableView::onDataViewClicked()
{
    qDebug() << "DEBUG: Intentando cambiar a Vista Datos";
    
    // Validación: verificar si existe una Primary Key
    if (primaryKeyRow == -1) {
        // No hay Primary Key definida, mostrar mensaje de validación
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("⚠️ Primary Key Requerida");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("<h3>Primary Key Requerida</h3>");
        msgBox.setInformativeText(
            "Esta tabla no tiene definida una Primary Key.<br><br>"
            "⚠️ <b>Debe seleccionar un campo como Primary Key antes de continuar.</b><br><br>"
            "Para definir una Primary Key:<br>"
            "1. Seleccione un campo en la tabla<br>"
            "2. Marque la casilla 'Primary Key' en las propiedades<br>"
            "3. Intente cambiar a Vista de Datos nuevamente"
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.button(QMessageBox::Ok)->setText("Entendido");
        
        // Estilo del mensaje
        msgBox.setStyleSheet(
            "QMessageBox {"
                "background-color: white;"
                "min-width: 450px;"
                "min-height: 200px;"
            "}"
            "QMessageBox QLabel {"
                "color: #0b0f19;"
                "font-size: 14px;"
            "}"
            "QPushButton {"
                "background-color: #f59e0b;"
                "color: white;"
                "font-size: 14px;"
                "font-weight: 600;"
                "min-width: 120px;"
                "min-height: 36px;"
                "padding: 6px 12px;"
                "border-radius: 10px;"
                "border: 2px solid #d97706;"
            "}"
            "QPushButton:hover {"
                "background-color: #d97706;"
                "border-color: #b45309;"
            "}"
            "QPushButton:pressed {"
                "background-color: #b45309;"
            "}"
        );
        
        msgBox.exec();
        
        qDebug() << "DEBUG: Cambio a Vista Datos bloqueado - No hay Primary Key";
        return; // No permitir el cambio
    }
    
    qDebug() << "DEBUG: Primary Key encontrada en fila:" << primaryKeyRow << "- Cambiando a Vista Datos";
    emit switchToDataView();
}

void TableView::onDesignViewClicked()
{
    qDebug() << "DEBUG: Ya estoy en Vista Diseño";
    // No necesita hacer nada ya que estamos en la vista de diseño
}

void TableView::onAddRowClicked()
{
    qDebug() << "DEBUG: Agregar fila después de la seleccionada";

    int selectedRow = tableWidget->currentRow();
    int insertRow;

    if (selectedRow == -1) {
        // Si no hay fila seleccionada, agregar al final
        insertRow = tableWidget->rowCount();
        qDebug() << "DEBUG: No hay fila seleccionada, agregando al final en posición:" << insertRow;
    } else {
        // Agregar después de la fila seleccionada
        insertRow = selectedRow + 1;
        qDebug() << "DEBUG: Fila seleccionada:" << selectedRow << ", insertando en posición:" << insertRow;
    }

    // Insertar nueva fila
    tableWidget->insertRow(insertRow);

    // Ajustar primaryKeyRow si es necesario
    if (primaryKeyRow != -1 && primaryKeyRow >= insertRow) {
        primaryKeyRow++;
        qDebug() << "DEBUG: Ajustando primaryKeyRow a:" << primaryKeyRow;
    }

    // Ajustar foreignKeyRows si es necesario
    for (int i = 0; i < foreignKeyRows.size(); i++) {
        if (foreignKeyRows[i] >= insertRow) {
            foreignKeyRows[i]++;
        }
    }

    // Ajustar uniqueKeyRows si es necesario
    for (int i = 0; i < uniqueKeyRows.size(); i++) {
        if (uniqueKeyRows[i] >= insertRow) {
            uniqueKeyRows[i]++;
        }
    }

    // Ajustar listas de configuración de campos
    fieldCurrencyFormats.insert(insertRow, "Lempiras (Lps)");
    fieldNumberTypes.insert(insertRow, "Entero");
    fieldDateFormats.insert(insertRow, "DD-MM-YY");
    fieldMillaresDecimals.insert(insertRow, "2");
    fieldTextSizes.insert(insertRow, "255");

    // Crear items para la nueva fila
    for (int col = 0; col < tableWidget->columnCount(); col++) {
        QTableWidgetItem *item = new QTableWidgetItem("");

        // Configurar fuente más grande para mejor legibilidad
        QFont itemFont = item->font();
        itemFont.setPointSize(14);
        item->setFont(itemFont);

        // Solo la primera columna está habilitada inicialmente
        if (col == 0) {
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            item->setBackground(QBrush(QColor(255, 255, 255)));
        } else {
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setBackground(QBrush(QColor(245, 245, 245))); // Deshabilitada
        }

        tableWidget->setItem(insertRow, col, item);
    }

    // Seleccionar la nueva fila
    tableWidget->setCurrentCell(insertRow, 0);

    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());

    qDebug() << "DEBUG: Fila agregada exitosamente en posición:" << insertRow;
}

void TableView::onDeleteRowClicked()
{
    qDebug() << "DEBUG: Eliminar fila seleccionada";

    int selectedRow = tableWidget->currentRow();

    // --- Caso: no hay selección ---
    if (selectedRow == -1) {
        QMessageBox box(this);
        box.setWindowTitle("Eliminar Fila");
        box.setIcon(QMessageBox::Information);
        box.setText("<h3>Ninguna Fila Seleccionada</h3>");
        box.setInformativeText("Por favor seleccione una fila para eliminar.");
        box.setStandardButtons(QMessageBox::Ok);

        // Estilo general (como tu captura)
        box.setStyleSheet(
            "QMessageBox {"
            "background-color: white;"
            "min-width: 420px;"
            "min-height: 160px;"
            "}"
            "QMessageBox QLabel {"
            "color: #0b0f19;"
            "font-size: 14px;"
            "}"
            );

        if (QAbstractButton* okBtn = box.button(QMessageBox::Ok)) {
            okBtn->setText("Entendido");
            okBtn->setStyleSheet(
                "QPushButton {"
                "background-color: #2563eb;"
                "color: white;"
                "font-size: 14px;"
                "font-weight: 600;"
                "min-width: 120px;"
                "min-height: 36px;"
                "padding: 6px 12px;"
                "border-radius: 10px;"
                "border: 2px solid #1e40af;"
                "}"
                "QPushButton:hover {"
                "background-color: #1e40af;"
                "border-color: #1e3a8a;"
                "}"
                "QPushButton:pressed {"
                "background-color: #1d4ed8;"
                "}"
                );
        }

        box.exec();
        return;
    }

    // --- ¿Fila vacía? ---
    QTableWidgetItem* firstItem = tableWidget->item(selectedRow, 0);
    bool isEmptyRow = !firstItem || firstItem->text().trimmed().isEmpty();

    // --- Confirmación especial si es la llave primaria y no está vacía ---
    if (primaryKeyRow == selectedRow && !isEmptyRow) {
        QString fieldName = firstItem->text();
        if (fieldName.startsWith("🔑 ")) fieldName = fieldName.mid(3);

        QMessageBox box(this);
        box.setWindowTitle("🔑 Eliminar Llave Primaria");
        box.setIcon(QMessageBox::Warning);
        box.setText("<h3>Eliminar Campo Llave Primaria</h3>");
        box.setInformativeText(QString(
                                   "Está a punto de eliminar el campo llave primaria <b>'%1'</b>.<br><br>"
                                   "⚠️ <b>Advertencia:</b> Este campo es único e irrecuperable.<br>"
                                   "¿Está seguro de que desea continuar?")
                                   .arg(fieldName));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);

        // Estilo general (texto como tu UI)
        box.setStyleSheet(
            "QMessageBox {"
            "background-color: white;"
            "min-width: 460px;"
            "min-height: 180px;"
            "}"
            "QMessageBox QLabel {"
            "color: #0b0f19;"
            "font-size: 14px;"
            "}"
            );

        // Botón Sí (rojo compacto)
        if (QAbstractButton* yesBtn = box.button(QMessageBox::Yes)) {
            yesBtn->setText("Sí, Eliminar");
            yesBtn->setStyleSheet(
                "QPushButton {"
                "background-color: #dc2626;"
                "color: white;"
                "font-size: 14px;"
                "font-weight: 600;"
                "min-width: 120px;"
                "min-height: 36px;"
                "padding: 6px 12px;"
                "border-radius: 10px;"
                "border: 2px solid #991b1b;"
                "}"
                "QPushButton:hover {"
                "background-color: #b91c1c;"
                "border-color: #7f1d1d;"
                "}"
                "QPushButton:pressed {"
                "background-color: #991b1b;"
                "}"
                );
        }
        // Botón No (verde compacto)
        if (QAbstractButton* noBtn = box.button(QMessageBox::No)) {
            noBtn->setText("No, Cancelar");
            noBtn->setStyleSheet(
                "QPushButton {"
                "background-color: #16a34a;"
                "color: white;"
                "font-size: 14px;"
                "font-weight: 600;"
                "min-width: 120px;"
                "min-height: 36px;"
                "padding: 6px 12px;"
                "border-radius: 10px;"
                "border: 2px solid #15803d;"
                "}"
                "QPushButton:hover {"
                "background-color: #15803d;"
                "border-color: #166534;"
                "}"
                "QPushButton:pressed {"
                "background-color: #166534;"
                "}"
                );
        }

        if (box.exec() == QMessageBox::No) return;

        primaryKeyRow = -1;
        qDebug() << "DEBUG: Llave primaria eliminada";
    }

    // --- Confirmación de eliminación (manteniendo tu estilo de mensaje) ---
    QString fieldName = firstItem ? firstItem->text() : "";
    if (fieldName.startsWith("🔑 ")) fieldName = fieldName.mid(3);

    QString title, message;
    if (isEmptyRow) {
        title = "Eliminar Fila Vacía";
        message = "¿Está seguro de que desea eliminar esta fila vacía?";
    } else {
        title = "Confirmar Eliminación";
        message = QString("¿Está seguro de que desea eliminar el campo <b>'%1'</b>?<br><br>"
                          "Esta acción no se puede deshacer.")
                      .arg(fieldName.isEmpty() ? QString("Fila %1").arg(selectedRow + 1) : fieldName);
    }

    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setIcon(isEmptyRow ? QMessageBox::Information : QMessageBox::Question);
    if (isEmptyRow) {
        box.setText(message);
    } else {
        box.setText("<h3>Eliminar Campo</h3>");
        box.setInformativeText(message);
    }
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);

    // Estilo general (igual look)
    box.setStyleSheet(
        "QMessageBox {"
        "background-color: white;"
        "min-width: 460px;"
        "min-height: 180px;"
        "}"
        "QMessageBox QLabel {"
        "color: #0b0f19;"
        "font-size: 14px;"
        "}"
        );

    // Botones compactos (sólidos, sin transparencia)
    if (QAbstractButton* yesBtn = box.button(QMessageBox::Yes)) {
        yesBtn->setText("Sí, Eliminar");
        yesBtn->setStyleSheet(
            "QPushButton {"
            "background-color: #dc2626;"
            "color: white;"
            "font-size: 14px;"
            "font-weight: 600;"
            "min-width: 120px;"
            "min-height: 36px;"
            "padding: 6px 12px;"
            "border-radius: 10px;"
            "border: 2px solid #991b1b;"
            "}"
            "QPushButton:hover {"
            "background-color: #b91c1c;"
            "border-color: #7f1d1d;"
            "}"
            "QPushButton:pressed {"
            "background-color: #991b1b;"
            "}"
            );
    }
    if (QAbstractButton* noBtn = box.button(QMessageBox::No)) {
        noBtn->setText("Cancelar");
        noBtn->setStyleSheet(
            "QPushButton {"
            "background-color: #16a34a;"
            "color: white;"
            "font-size: 14px;"
            "font-weight: 600;"
            "min-width: 120px;"
            "min-height: 36px;"
            "padding: 6px 12px;"
            "border-radius: 10px;"
            "border: 2px solid #15803d;"
            "}"
            "QPushButton:hover {"
            "background-color: #15803d;"
            "border-color: #166534;"
            "}"
            "QPushButton:pressed {"
            "background-color: #166534;"
            "}"
            );
    }

    if (box.exec() == QMessageBox::No) return;

    // --- Ajustes tras eliminar ---
    if (primaryKeyRow != -1) {
        if (primaryKeyRow > selectedRow)      primaryKeyRow--;
        else if (primaryKeyRow == selectedRow) primaryKeyRow = -1;
        qDebug() << "DEBUG: Ajustando primaryKeyRow a:" << primaryKeyRow;
    }

    // Ajustar las filas de Foreign Keys
    for (int i = foreignKeyRows.size() - 1; i >= 0; i--) {
        if (foreignKeyRows[i] > selectedRow) {
            foreignKeyRows[i]--;  // Ajustar índice hacia arriba
        } else if (foreignKeyRows[i] == selectedRow) {
            foreignKeyRows.removeAt(i);  // Remover la FK que se está eliminando
        }
    }
    qDebug() << "DEBUG: Foreign Keys tras eliminar fila:" << foreignKeyRows;

    // Ajustar las filas de Unique Keys
    for (int i = uniqueKeyRows.size() - 1; i >= 0; i--) {
        if (uniqueKeyRows[i] > selectedRow) {
            uniqueKeyRows[i]--;  // Ajustar índice hacia arriba
        } else if (uniqueKeyRows[i] == selectedRow) {
            uniqueKeyRows.removeAt(i);  // Remover el Unique que se está eliminando
        }
    }
    qDebug() << "DEBUG: Unique Keys tras eliminar fila:" << uniqueKeyRows;

    // Ajustar listas de configuración de campos
    if (selectedRow < fieldCurrencyFormats.size()) {
        fieldCurrencyFormats.removeAt(selectedRow);
    }
    if (selectedRow < fieldNumberTypes.size()) {
        fieldNumberTypes.removeAt(selectedRow);
    }
    if (selectedRow < fieldDateFormats.size()) {
        fieldDateFormats.removeAt(selectedRow);
    }
    if (selectedRow < fieldMillaresDecimals.size()) {
        fieldMillaresDecimals.removeAt(selectedRow);
    }
    if (selectedRow < fieldTextSizes.size()) {
        fieldTextSizes.removeAt(selectedRow);
    }

    tableWidget->removeRow(selectedRow);
    ensureEmptyRowExists();

    int newSelection = selectedRow;
    if (newSelection >= tableWidget->rowCount()) newSelection = tableWidget->rowCount() - 1;
    if (newSelection >= 0) {
        tableWidget->setCurrentCell(newSelection, 0);
        updatePropertiesForRow(newSelection);
    }

    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
    qDebug() << "DEBUG: Fila eliminada exitosamente. Nueva selección:" << newSelection;
}

void TableView::onFieldItemChanged(QTableWidgetItem *item)
{
    if (!item) return;
    
    int row = item->row();
    int col = item->column();
    
    qDebug() << "DEBUG: Campo cambiado en fila:" << row << "columna:" << col;
    
    // Validación para nombres de campos duplicados (columna 0)
    if (col == 0 && !item->text().trimmed().isEmpty()) {
        QString fieldName = item->text().trimmed();
        
        // Si este campo tiene el icono de llave primaria, removerlo temporalmente para la comparación
        QString cleanFieldName = fieldName;
        if (cleanFieldName.startsWith("🔑 ")) {
            cleanFieldName = cleanFieldName.mid(3);
        }
        
        // Buscar duplicados (sin importar mayúsculas/minúsculas)
        for (int checkRow = 0; checkRow < tableWidget->rowCount(); checkRow++) {
            if (checkRow == row) continue; // Saltar la fila actual
            
            QTableWidgetItem *checkItem = tableWidget->item(checkRow, 0);
            if (checkItem && !checkItem->text().trimmed().isEmpty()) {
                QString existingName = checkItem->text().trimmed();
                
                // Remover icono de llave si existe para comparación
                if (existingName.startsWith("🔑 ")) {
                    existingName = existingName.mid(3);
                }
                
                // Comparación sin considerar mayúsculas/minúsculas
                if (cleanFieldName.toLower() == existingName.toLower()) {
                    // Mostrar mensaje de error mejorado
                    QMessageBox msgBox(this);
                    msgBox.setWindowTitle("❌ Campo Duplicado");
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setText(QString("<h3>Nombre de Campo Duplicado</h3>"));
                    msgBox.setInformativeText(QString("Ya existe un campo con el nombre <b>'%1'</b>.<br><br>"
                                                     "⚠️ <b>Restricción:</b> Los nombres de los campos deben ser únicos<br>"
                                                     "(sin importar mayúsculas/minúsculas).<br><br>"
                                                     "Por favor elija un nombre diferente.")
                                                     .arg(existingName));
                    msgBox.setStandardButtons(QMessageBox::Ok);
                    msgBox.button(QMessageBox::Ok)->setText("Entendido");
                    
                    // Estilo para botones visibles
                    msgBox.setStyleSheet(
                        "QMessageBox {"
                        "background-color: white;"
                        "color: #1f2937;"
                        "}"
                        "QPushButton {"
                        "background-color: #f3f4f6;"
                        "color: #1f2937;"
                        "border: 2px solid #d1d5db;"
                        "border-radius: 6px;"
                        "padding: 8px 16px;"
                        "font-weight: bold;"
                        "min-width: 80px;"
                        "}"
                        "QPushButton:hover {"
                        "background-color: #e5e7eb;"
                        "border-color: #9ca3af;"
                        "}"
                        "QPushButton:pressed {"
                        "background-color: #d1d5db;"
                        "border-color: #6b7280;"
                        "}"
                    );
                    
                    msgBox.exec();
                    
                    // Limpiar el campo duplicado usando blockSignals del widget table
                    tableWidget->blockSignals(true);
                    item->setText("");
                    tableWidget->blockSignals(false);
                    
                    // Enfocar en el campo para que el usuario pueda escribir un nombre diferente
                    tableWidget->setCurrentItem(item);
                    tableWidget->editItem(item);
                    
                    qDebug() << "DEBUG: Campo duplicado detectado:" << cleanFieldName << "vs" << existingName;
                    return; // Salir sin procesar más
                }
            }
        }
        
        // Si llegamos aquí, no hay duplicados. Restaurar todos los iconos apropiados
        if (!cleanFieldName.isEmpty()) {
            bool isPrimaryKey = (primaryKeyRow == row);
            bool isForeignKey = foreignKeyRows.contains(row);
            bool isUnique = uniqueKeyRows.contains(row);
            
            QString newText = cleanFieldName;
            QString toolTip = "";
            
            // Construir el texto con todos los iconos apropiados
            if (isPrimaryKey && isForeignKey && isUnique) {
                newText = "🔑🔗🔶 " + cleanFieldName;
                toolTip = "Campo Primary Key, Foreign Key y Unique";
            } else if (isPrimaryKey && isForeignKey) {
                newText = "🔑🔗 " + cleanFieldName;
                toolTip = "Campo Primary Key con Foreign Key";
            } else if (isPrimaryKey && isUnique) {
                newText = "🔑🔶 " + cleanFieldName;
                toolTip = "Campo Primary Key y Unique";
            } else if (isForeignKey && isUnique) {
                newText = "�🔶 " + cleanFieldName;
                toolTip = "Campo Foreign Key y Unique";
            } else if (isPrimaryKey) {
                newText = "🔑 " + cleanFieldName;
                toolTip = "Campo Llave Primaria - Requerido y único";
            } else if (isForeignKey) {
                newText = "🔗 " + cleanFieldName;
                toolTip = "Campo Foreign Key - Referencia a otra tabla";
            } else if (isUnique) {
                newText = "🔶 " + cleanFieldName;
                toolTip = "Campo Unique - Valores únicos, no se permiten duplicados";
            }
            
            item->setText(newText);
            item->setToolTip(toolTip);
        }
    }
    
    // Si se escribió algo en una celda, habilitar la siguiente celda en la misma fila
    if (!item->text().trimmed().isEmpty()) {
        // Habilitar la siguiente columna en la misma fila
        if (col + 1 < tableWidget->columnCount()) {
            QTableWidgetItem *nextItem = tableWidget->item(row, col + 1);
            if (nextItem) {
                nextItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                nextItem->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco cuando se habilita
                
                // Configurar fuente más grande para mejor legibilidad
                QFont itemFont = nextItem->font();
                itemFont.setPointSize(14);
                nextItem->setFont(itemFont);
                
                // Establecer valores por defecto según la columna
                if (col == 0 && nextItem->text().isEmpty()) {
                    nextItem->setText("Texto largo"); // <-- coincide con el combo
                }
            }
        }
        
        // Si completamos una fila, agregar una nueva fila
        if (col == 0) {
            ensureEmptyRowExists();
        }

    } else {
        // Si se borró el contenido, deshabilitar las celdas siguientes en la misma fila
        for (int nextCol = col + 1; nextCol < tableWidget->columnCount(); nextCol++) {
            QTableWidgetItem *nextItem = tableWidget->item(row, nextCol);
            if (nextItem) {
                nextItem->setText(""); // Limpiar contenido
                nextItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                nextItem->setBackground(QBrush(QColor(245, 245, 245))); // Volver a gris
            }
        }
    }
    
    // Emitir señal de cambio de diseño de tabla
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::addNewRow()
{
    int newRow = tableWidget->rowCount();
    tableWidget->setRowCount(newRow + 1);
    
    for (int col = 0; col < tableWidget->columnCount(); col++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        
        // Configurar fuente más grande para mejor legibilidad
        QFont itemFont = item->font();
        itemFont.setPointSize(14);
        item->setFont(itemFont);
        
        // Solo la primera columna está habilitada inicialmente
        if (col == 0) {
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            item->setBackground(QBrush(QColor(255, 255, 255)));
        } else {
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setBackground(QBrush(QColor(245, 245, 245))); // Deshabilitada
        }
        
        tableWidget->setItem(newRow, col, item);
    }
}

void TableView::ensureEmptyRowExists()
{
    // Validar integridad de llave primaria antes de manipular filas
    validatePrimaryKeyIntegrity();
    
    // Verificar si necesitamos más filas vacías
    bool needNewRow = true;
    for (int row = 0; row < tableWidget->rowCount(); row++) {
        QTableWidgetItem *item = tableWidget->item(row, 0);
        if (!item || item->text().trimmed().isEmpty()) {
            needNewRow = false;
            break;
        }
    }
    
    if (needNewRow) {
        int newRow = tableWidget->rowCount();
        tableWidget->setRowCount(newRow + 1);
        
        for (int col = 0; col < tableWidget->columnCount(); col++) {
            QTableWidgetItem *item = new QTableWidgetItem("");
            
            // Configurar fuente más grande para mejor legibilidad
            QFont itemFont = item->font();
            itemFont.setPointSize(14);
            item->setFont(itemFont);
            
            // Solo la primera columna está habilitada inicialmente
            if (col == 0) {
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                item->setBackground(QBrush(QColor(255, 255, 255)));
            } else {
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                item->setBackground(QBrush(QColor(245, 245, 245))); // Deshabilitada
            }
            
            tableWidget->setItem(newRow, col, item);
        }
    }
}

// Utility Methods
void TableView::setTableName(const QString &tableName)
{
    currentTableName = tableName;
    if (tableNameLabel) {
        tableNameLabel->setText(tableName);
    }
}

void TableView::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Reaplica los estilos del header si existe
    if (headerWidget && tableNameLabel) {
        headerWidget->setStyleSheet(
            "QWidget {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop:0 #ffffff, stop:1 #f8f9fa);"
            "border-bottom: 2px solid #e9ecef;"
            "margin: 0px;"
            "padding: 0px;"
            "}"
        );
        
        tableNameLabel->setStyleSheet(
            "QLabel {"
            "font-family: 'Inter', 'SF Pro Display', 'Helvetica Neue', Arial, sans-serif;"
            "font-size: 20px;"
            "font-weight: 600;"
            "color: #2d3748;"
            "background: transparent;"
            "border: none;"
            "padding: 0px;"
            "margin: 0px;"
            "min-width: 150px;"
            "}"
        );
        
        // Fuerza una actualización del widget
        headerWidget->update();
        tableNameLabel->update();
    }
}

// Style Methods
QString TableView::getTableStyle()
{
    return "QTableWidget {"
           "background-color: white;"
           "gridline-color: #e2e8f0;"
           "border: 1px solid #cbd5e1;"
           "selection-background-color: #dbeafe;"
           "font-size: 14px;"
           "}"
           "QTableWidget::item {"
           "padding: 8px;"
           "border-bottom: 1px solid #f1f5f9;"
           "}"
           "QTableWidget::item:selected {"
           "background-color: #bfdbfe;"
           "color: #1e40af;"
           "}"
           "QTableWidget::item:focus {"
           "background-color: white;"
           "border: 1px solid #1d4ed8;"
           "outline: none;"
           "}"
           "QHeaderView::section {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f1f5f9, stop:1 #e2e8f0);"
           "color: #374151;"
           "padding: 8px;"
           "border: 1px solid #cbd5e1;"
           "font-weight: bold;"
           "font-size: 14px;"
           "}";
}

QString TableView::getInputStyle()
{
    return "QLineEdit {"
           "background-color: white;"
           "border: 2px solid #e5e7eb;"
           "border-radius: 6px;"
           "padding: 8px 12px;"
           "color: #111827;"
           "font-size: 13px;"
           "}"
           "QLineEdit:focus {"
           "border-color: #3b82f6;"
           "}";
}

QString TableView::getComboStyle()
{
    return "QComboBox {"
           "background-color: white;"
           "border: 2px solid #e5e7eb;"
           "border-radius: 6px;"
           "padding: 8px 12px;"
           "color: #111827;"
           "font-size: 13px;"
           "min-height: 20px;" // Altura mínima para evitar que se vea cortado
           "}"
           "QComboBox:focus {"
           "border-color: #3b82f6;"
           "}"
           "QComboBox::drop-down {"
           "border: none;"
           "width: 20px;"
           "}"
           "QComboBox::down-arrow {"
           "image: none;"
           "border: 2px solid #6b7280;"
           "width: 8px;"
           "height: 8px;"
           "border-top: none;"
           "border-left: none;"
           "margin-right: 5px;"
           "transform: rotate(45deg);"
           "}";
}

QString TableView::getTextEditStyle()
{
    return "QTextEdit {"
           "background-color: white;"
           "border: 2px solid #e5e7eb;"
           "border-radius: 6px;"
           "padding: 8px 12px;"
           "color: #111827;"
           "font-size: 13px;"
           "}"
           "QTextEdit:focus {"
           "border-color: #3b82f6;"
           "}";
}

QStringList TableView::getCurrentFieldNames() const
{
    QStringList fieldNames;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows";
        return fieldNames;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = tableWidget->item(row, 0);
        if (item && !item->text().trimmed().isEmpty()) {
            QString fieldName = item->text().trimmed();
            
            // Remover el icono de llave si existe
            if (fieldName.startsWith("🔑 ")) {
                fieldName = fieldName.mid(3);
            }
            
            if (!fieldName.isEmpty()) {
                fieldNames << fieldName;
                qDebug() << "DEBUG: Added field name:" << fieldName;
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentFieldNames() returning:" << fieldNames;
    return fieldNames;
}

QStringList TableView::getAllFieldNames() const
{
    QStringList fieldNames;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows";
        return fieldNames;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = tableWidget->item(row, 0);
        if (item && !item->text().trimmed().isEmpty()) {
            QString fieldName = item->text().trimmed();
            
            // NO remover el icono de llave, mantenerlo para mostrar en relaciones
            if (!fieldName.isEmpty()) {
                fieldNames << fieldName;
                qDebug() << "DEBUG: Added field name with key:" << fieldName;
            }
        }
    }
    
    qDebug() << "DEBUG: getAllFieldNames() returning:" << fieldNames;
    return fieldNames;
}

QStringList TableView::getPrimaryKeyFieldNames() const
{
    QStringList primaryKeyFields;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        return primaryKeyFields;
    }
    
    // Si hay una fila marcada como Primary Key
    if (primaryKeyRow >= 0 && primaryKeyRow < tableWidget->rowCount()) {
        QTableWidgetItem *item = tableWidget->item(primaryKeyRow, 0);
        if (item && !item->text().trimmed().isEmpty()) {
            QString fieldName = item->text().trimmed();
            
            // Remover el icono de llave si existe para obtener el nombre limpio
            if (fieldName.startsWith("🔑 ")) {
                fieldName = fieldName.mid(3);
            }
            
            if (!fieldName.isEmpty()) {
                primaryKeyFields << fieldName;
            }
        }
    }
    
    return primaryKeyFields;
}

QStringList TableView::getForeignKeyFieldNames() const
{
    QStringList foreignKeyFields;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        return foreignKeyFields;
    }
    
    // Recorrer todas las filas marcadas como Foreign Key
    for (int row : foreignKeyRows) {
        if (row >= 0 && row < tableWidget->rowCount()) {
            QTableWidgetItem *item = tableWidget->item(row, 0);
            if (item && !item->text().trimmed().isEmpty()) {
                QString fieldName = item->text().trimmed();
                
                // Remover el icono de llave si existe para obtener el nombre limpio
                if (fieldName.startsWith("🔗 ")) {
                    fieldName = fieldName.mid(3);
                } else if (fieldName.startsWith("🔑🔗 ")) {
                    fieldName = fieldName.mid(5);
                }
                
                if (!fieldName.isEmpty()) {
                    foreignKeyFields << fieldName;
                }
            }
        }
    }
    
    return foreignKeyFields;
}

QStringList TableView::getCurrentFieldTypes() const
{
    QStringList fieldTypes;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for field types";
        return fieldTypes;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns";
            continue;
        }
        
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        
        if (typeItem && !typeItem->text().trimmed().isEmpty()) {
            QString fieldType = typeItem->text().trimmed();
            
            // Si el tipo es "Números", obtener el tipo específico de número
            if (fieldType == "Números") {
                // Asegurar que la lista de tipos de números tenga el tamaño correcto
                if (row < fieldNumberTypes.size() && !fieldNumberTypes[row].isEmpty()) {
                    fieldType = fieldNumberTypes[row];
                    qDebug() << "DEBUG: Reemplazando 'Números' con tipo específico:" << fieldType;
                } else {
                    // Valor por defecto si no se ha seleccionado un tipo específico
                    fieldType = "Entero";
                    qDebug() << "DEBUG: Usando tipo por defecto 'Entero' para campo Números";
                }
            }
            
            if (!fieldType.isEmpty()) {
                fieldTypes << fieldType;
                qDebug() << "DEBUG: Added field type:" << fieldType;
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentFieldTypes() returning:" << fieldTypes;
    return fieldTypes;
}

QStringList TableView::getCurrentCurrencyFormats() const
{
    QStringList currencyFormats;
    
    qDebug() << "DEBUG: getCurrentCurrencyFormats() - Lista guardada:" << fieldCurrencyFormats;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for currency formats";
        return currencyFormats;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns for currency formats";
            continue;
        }
        
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        
        if (typeItem && !typeItem->text().trimmed().isEmpty()) {
            QString fieldType = typeItem->text().trimmed();
            if (!fieldType.isEmpty()) {
                // Si es un campo de moneda, obtener el formato guardado
                if (fieldType == "moneda") {
                    QString format = "Lempiras (Lps)"; // Valor por defecto
                    
                    // Usar el formato guardado si existe
                    if (row < fieldCurrencyFormats.size() && !fieldCurrencyFormats[row].isEmpty()) {
                        format = fieldCurrencyFormats[row];
                    }
                    
                    currencyFormats << format;
                    qDebug() << "DEBUG: Added currency format:" << format << "for row:" << row;
                } else {
                    // Para campos que no son moneda, agregar cadena vacía para mantener índices
                    currencyFormats << "";
                }
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentCurrencyFormats() returning:" << currencyFormats;
    return currencyFormats;
}

QStringList TableView::getCurrentNumberTypes() const
{
    QStringList numberTypes;
    
    qDebug() << "DEBUG: getCurrentNumberTypes() - Lista guardada:" << fieldNumberTypes;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for number types";
        return numberTypes;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns for number types";
            continue;
        }
        
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        
        if (typeItem && !typeItem->text().trimmed().isEmpty()) {
            QString fieldType = typeItem->text().trimmed();
            if (!fieldType.isEmpty()) {
                // Si es un campo de números, obtener el tipo guardado
                if (fieldType == "Números") {
                    QString numberType = "Entero"; // Valor por defecto
                    
                    // Usar el tipo guardado si existe
                    if (row < fieldNumberTypes.size() && !fieldNumberTypes[row].isEmpty()) {
                        numberType = fieldNumberTypes[row];
                    }
                    
                    numberTypes << numberType;
                    qDebug() << "DEBUG: Added number type:" << numberType << "for row:" << row;
                } else {
                    // Para campos que no son números, agregar cadena vacía para mantener índices
                    numberTypes << "";
                }
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentNumberTypes() returning:" << numberTypes;
    return numberTypes;
}

QStringList TableView::getCurrentDateFormats() const
{
    QStringList dateFormats;
    
    qDebug() << "DEBUG: getCurrentDateFormats() - Lista guardada:" << fieldDateFormats;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for date formats";
        return dateFormats;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns for date formats";
            continue;
        }
        
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        
        if (typeItem && !typeItem->text().trimmed().isEmpty()) {
            QString fieldType = typeItem->text().trimmed();
            if (!fieldType.isEmpty()) {
                // Si es un campo de fecha, obtener el formato guardado
                if (fieldType == "fecha") {
                    QString dateFormat = "DD-MM-YY"; // Valor por defecto
                    
                    // Usar el formato guardado si existe
                    if (row < fieldDateFormats.size() && !fieldDateFormats[row].isEmpty()) {
                        dateFormat = fieldDateFormats[row];
                    }
                    
                    dateFormats << dateFormat;
                    qDebug() << "DEBUG: Added date format:" << dateFormat << "for row:" << row;
                } else {
                    // Para campos que no son fecha, agregar cadena vacía para mantener índices
                    dateFormats << "";
                }
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentDateFormats() returning:" << dateFormats;
    return dateFormats;
}

QStringList TableView::getCurrentMillaresDecimals() const
{
    QStringList millaresDecimals;
    
    qDebug() << "DEBUG: getCurrentMillaresDecimals() - Lista guardada:" << fieldMillaresDecimals;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for millares decimals";
        return millaresDecimals;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns for millares decimals";
            continue;
        }
        
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        
        if (typeItem && !typeItem->text().trimmed().isEmpty()) {
            QString fieldType = typeItem->text().trimmed();
            if (!fieldType.isEmpty()) {
                // Si es un campo de moneda, obtener los decimales guardados
                if (fieldType == "moneda") {
                    QString decimals = "2"; // Valor por defecto
                    
                    // Usar los decimales guardados si existen
                    if (row < fieldMillaresDecimals.size() && !fieldMillaresDecimals[row].isEmpty()) {
                        decimals = fieldMillaresDecimals[row];
                    }
                    
                    millaresDecimals << decimals;
                    qDebug() << "DEBUG: Added millares decimals:" << decimals << "for row:" << row;
                } else {
                    // Para campos que no son moneda, agregar cadena vacía para mantener índices
                    millaresDecimals << "";
                }
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentMillaresDecimals() returning:" << millaresDecimals;
    return millaresDecimals;
}

int TableView::getPrimaryKeyColumnIndex() const
{
    // Retorna el índice de la columna que es Primary Key, o -1 si no hay Primary Key
    if (primaryKeyRow == -1) {
        qDebug() << "DEBUG: No hay Primary Key definida";
        return -1;
    }
    
    // En nuestro diseño, primaryKeyRow indica qué fila de la tabla de diseño es la Primary Key
    // Pero en la vista de datos, esa fila se convierte en una columna
    // Por lo tanto, primaryKeyRow en diseño = índice de columna en datos
    qDebug() << "DEBUG: Primary Key está en la fila de diseño:" << primaryKeyRow << "-> Columna de datos:" << primaryKeyRow;
    return primaryKeyRow;
}

QList<int> TableView::getUniqueKeyColumnIndexes() const
{
    // Retorna la lista de índices de columnas que son campos únicos
    qDebug() << "DEBUG: Campos únicos en filas de diseño:" << uniqueKeyRows;
    
    // En nuestro diseño, uniqueKeyRows indica qué filas de la tabla de diseño son campos únicos
    // Pero en la vista de datos, esas filas se convierten en columnas
    // Por lo tanto, uniqueKeyRows en diseño = índices de columnas en datos
    QList<int> uniqueColumns = uniqueKeyRows;
    qDebug() << "DEBUG: Campos únicos como columnas de datos:" << uniqueColumns;
    
    return uniqueColumns;
}

QStringList TableView::getCurrentTextSizes() const
{
    QStringList textSizes;
    
    qDebug() << "DEBUG: getCurrentTextSizes() - Lista guardada:" << fieldTextSizes;
    
    // Verificar que la tabla existe y tiene filas
    if (!tableWidget || tableWidget->rowCount() == 0) {
        qDebug() << "DEBUG: TableWidget is null or has no rows for text sizes";
        return textSizes;
    }
    
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        // Verificar que ambas columnas existen
        if (tableWidget->columnCount() < 2) {
            qDebug() << "DEBUG: Table doesn't have enough columns";
            continue;
        }
        
        QTableWidgetItem *nameItem = tableWidget->item(row, 0);
        QTableWidgetItem *typeItem = tableWidget->item(row, 1);
        
        if (nameItem && typeItem && !nameItem->text().trimmed().isEmpty()) {
            QString dataType = typeItem->text();
            
            if (dataType == "Texto corto" || dataType == "Texto largo") {
                if (row < fieldTextSizes.size() && !fieldTextSizes[row].isEmpty()) {
                    QString size = fieldTextSizes[row];
                    textSizes << size;
                    qDebug() << "DEBUG: Added text size:" << size << "for row:" << row;
                } else {
                    // Valor por defecto según el tipo
                    QString defaultSize = (dataType == "Texto corto") ? "255" : "Sin límite";
                    textSizes << defaultSize;
                    qDebug() << "DEBUG: Added default text size:" << defaultSize << "for row:" << row;
                }
            } else {
                // Para campos que no son texto, agregar cadena vacía para mantener índices
                textSizes << "";
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentTextSizes() returning:" << textSizes;
    return textSizes;
}

void TableView::onNumberDecimalsChanged(const QString &dec)
{
    qDebug() << "DEBUG: Decimales cambiados a:" << dec
             << " | Fila:" << currentSelectedRow;

    if (currentSelectedRow < 0) return;

    // Asegurar storage por fila (default "2")
    while (fieldDecimalPlaces.size() <= currentSelectedRow)
        fieldDecimalPlaces.append("2");

    // Guardar solo si hay cambio real
    if (fieldDecimalPlaces[currentSelectedRow] != dec) {
        fieldDecimalPlaces[currentSelectedRow] = dec;
        qDebug() << "DEBUG: Guardado decimales =" << dec
                 << " para fila " << currentSelectedRow;

        // Refrescar vistas / sincronizar como haces en otros cambios
        emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
        emit tableDesignChangedWithAllFormats(
            getCurrentFieldNames(),
            getCurrentFieldTypes(),
            getCurrentCurrencyFormats(),
            getCurrentMillaresDecimals(),
            getCurrentNumberTypes(),
            getCurrentDateFormats()
            // Si luego quieres propagar también los decimales de números,
            // puedes extender esta señal para incluir getCurrentDecimalPlaces().
            );
    }
}


void TableView::createSpecificPropertiesWidgets()
{
    // Widget contenedor para propiedades específicas con mejor distribución
    specificPropertiesWidget = new QWidget();
    specificPropertiesWidget->setMinimumHeight(100); // Más altura para que no se corte
    specificPropertiesLayout = new QVBoxLayout(specificPropertiesWidget);
    specificPropertiesLayout->setContentsMargins(0, 15, 0, 15); // Más margen vertical
    specificPropertiesLayout->setSpacing(15); // Más espaciado
    
    // Agregar título para las propiedades específicas
    QLabel *specificTitle = new QLabel("Propiedades Específicas:");
    specificTitle->setFont(QFont("Arial", 12, QFont::Bold));
    specificTitle->setStyleSheet("QLabel { color: #374151; margin-bottom: 8px; }");
    specificPropertiesLayout->addWidget(specificTitle);
    
    // Widget para propiedades de texto (char[N] y string)
    textPropertiesWidget = new QWidget();
    textPropertiesWidget->setMinimumHeight(70); // Más altura para acomodar la validación
    QVBoxLayout *textMainLayout = new QVBoxLayout(textPropertiesWidget);
    textMainLayout->setContentsMargins(0, 5, 0, 5);
    textMainLayout->setSpacing(8);
    
    // Layout horizontal para el campo de tamaño
    QWidget *textInputWidget = new QWidget();
    QHBoxLayout *textLayout = new QHBoxLayout(textInputWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(15);
    
    QLabel *textSizeLabel = new QLabel("Tamaño del Campo:");
    textSizeLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    textSizeLabel->setMinimumWidth(120); // Ancho fijo para alineación
    textLayout->addWidget(textSizeLabel);
    
    textSizeEdit = new QLineEdit();
    textSizeEdit->setStyleSheet(getInputStyle());
    textSizeEdit->setPlaceholderText("Máximo 255 caracteres");
    textSizeEdit->setText("255");
    textSizeEdit->setMaximumWidth(200); // Limitar ancho para mejor distribución
    textLayout->addWidget(textSizeEdit);
    textLayout->addStretch(); // Agregar stretch para ocupar espacio restante
    
    // Label para mostrar la validación
    textValidationLabel = new QLabel("(Máx. 255 caracteres)");
    textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
    
    textMainLayout->addWidget(textInputWidget);
    textMainLayout->addWidget(textValidationLabel);
    
    // Widget para propiedades de número (int y float)
    numberPropertiesWidget = new QWidget();
    numberPropertiesWidget->setMinimumHeight(70); // Más altura para información adicional
    QVBoxLayout *numberMainLayout = new QVBoxLayout(numberPropertiesWidget);
    numberMainLayout->setContentsMargins(0, 5, 0, 5);
    numberMainLayout->setSpacing(8);
    
    // Layout horizontal para el tipo de número
    QWidget *numberInputWidget = new QWidget();
    QHBoxLayout *numberLayout = new QHBoxLayout(numberInputWidget);
    numberLayout->setContentsMargins(0, 0, 0, 0);
    numberLayout->setSpacing(15);
    
    QLabel *numberTypeLabel = new QLabel("Tipo de Número:");
    numberTypeLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    numberTypeLabel->setMinimumWidth(120);
    numberLayout->addWidget(numberTypeLabel);
    
    numberTypeCombo = new QComboBox();
    numberTypeCombo->addItems({"Entero", "Decimal", "Doble", "Byte"});
    numberTypeCombo->setStyleSheet(getComboStyle());
    numberTypeCombo->setMaximumWidth(200);
    numberTypeCombo->setMinimumHeight(35); // Altura mínima para el combo
    numberLayout->addWidget(numberTypeCombo);
    numberLayout->addStretch();

    // ----- Decimales solo para Decimal/Doble -----
    numberDecimalsLabel = new QLabel("Decimales:");
    numberDecimalsLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    numberDecimalsLabel->setMinimumWidth(80);

    numberDecimalsCombo = new QComboBox();
    numberDecimalsCombo->addItems({"0","1","2","3","4","5","6"});
    numberDecimalsCombo->setCurrentText("2"); // default
    numberDecimalsCombo->setStyleSheet(getComboStyle());
    numberDecimalsCombo->setMaximumWidth(80);
    numberDecimalsCombo->setMinimumHeight(35);

    numberLayout->addWidget(numberDecimalsLabel);
    numberLayout->addWidget(numberDecimalsCombo);
    numberLayout->addStretch();

    // Conexión
    connect(numberDecimalsCombo, &QComboBox::currentTextChanged,
            this, &TableView::onNumberDecimalsChanged);
    
    // Label para mostrar información del tamaño del campo
    numberSizeLabel = new QLabel("Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)");
    numberSizeLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
    numberSizeLabel->setWordWrap(true);
    
    numberMainLayout->addWidget(numberInputWidget);
    numberMainLayout->addWidget(numberSizeLabel);
    
    // Widget para propiedades de moneda
    currencyPropertiesWidget = new QWidget();
    currencyPropertiesWidget->setMinimumHeight(70); // Más altura para acomodar decimales de millares
    QVBoxLayout *currencyMainLayout = new QVBoxLayout(currencyPropertiesWidget);
    currencyMainLayout->setContentsMargins(0, 5, 0, 5);
    currencyMainLayout->setSpacing(8);
    
    // Layout horizontal para el formato de moneda
    QWidget *currencyFormatWidget = new QWidget();
    QHBoxLayout *currencyLayout = new QHBoxLayout(currencyFormatWidget);
    currencyLayout->setContentsMargins(0, 0, 0, 0);
    currencyLayout->setSpacing(10); // Reducir espaciado para que esté más junto
    
    QLabel *currencyFormatLabel = new QLabel("Formato de Moneda:");
    currencyFormatLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    currencyFormatLabel->setMinimumWidth(120);
    currencyLayout->addWidget(currencyFormatLabel);
    
    currencyFormatCombo = new QComboBox();
    currencyFormatCombo->addItems({"Lempiras (Lps)", "Dollar ($)", "Euros (€)", "Millares"});
    currencyFormatCombo->setStyleSheet(getComboStyle());
    currencyFormatCombo->setMaximumWidth(200);
    currencyFormatCombo->setMinimumHeight(35); // Altura mínima para el combo
    currencyLayout->addWidget(currencyFormatCombo);
    
    // Combo para decimales de millares (al lado derecho del formato, más junto)
    millaresDecimalsLabel = new QLabel("Decimales:");
    millaresDecimalsLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    millaresDecimalsLabel->setMinimumWidth(80);
    
    millaresDecimalsCombo = new QComboBox();
    millaresDecimalsCombo->addItems({"0", "1", "2", "3", "4", "5", "6"});
    millaresDecimalsCombo->setCurrentText("2"); // Valor por defecto
    millaresDecimalsCombo->setStyleSheet(getComboStyle());
    millaresDecimalsCombo->setMaximumWidth(80);
    millaresDecimalsCombo->setMinimumHeight(35);
    
    // Agregar el combo de decimales al mismo layout horizontal
    currencyLayout->addWidget(millaresDecimalsLabel);
    currencyLayout->addWidget(millaresDecimalsCombo);
    currencyLayout->addStretch(); // Agregar stretch al final
    
    // Agregar widget al layout principal de moneda
    currencyMainLayout->addWidget(currencyFormatWidget);
    
    // Widget para propiedades de fecha
    datePropertiesWidget = new QWidget();
    datePropertiesWidget->setMinimumHeight(40); // Altura mínima
    QHBoxLayout *dateLayout = new QHBoxLayout(datePropertiesWidget);
    dateLayout->setContentsMargins(0, 5, 0, 5); // Margen vertical
    dateLayout->setSpacing(15);
    
    QLabel *dateFormatLabel = new QLabel("Formato de Fecha:");
    dateFormatLabel->setStyleSheet("QLabel { color: #475569; font-weight: bold; }");
    dateFormatLabel->setMinimumWidth(120);
    dateLayout->addWidget(dateFormatLabel);
    
    dateFormatCombo = new QComboBox();
    dateFormatCombo->addItems({"DD-MM-YY", "DD/MM/YY"});
    dateFormatCombo->setStyleSheet(getComboStyle());
    dateFormatCombo->setMaximumWidth(200);
    dateFormatCombo->setMinimumHeight(35); // Altura mínima para el combo
    dateLayout->addWidget(dateFormatCombo);
    dateLayout->addStretch();
    
    // Agregar todos los widgets al layout principal (inicialmente ocultos)
    specificPropertiesLayout->addWidget(textPropertiesWidget);
    specificPropertiesLayout->addWidget(numberPropertiesWidget);
    specificPropertiesLayout->addWidget(currencyPropertiesWidget);
    specificPropertiesLayout->addWidget(datePropertiesWidget);
    specificPropertiesLayout->addStretch(); // Agregar stretch al final para mejor distribución
    
    // Ocultar todos inicialmente
    textPropertiesWidget->hide();
    numberPropertiesWidget->hide();
    currencyPropertiesWidget->hide();
    datePropertiesWidget->hide();
    
    // Conectar señales
    connect(textSizeEdit, &QLineEdit::textChanged, this, &TableView::onTextSizeChanged);
    connect(numberTypeCombo, &QComboBox::currentTextChanged, this, &TableView::onNumberTypeChanged);
    connect(currencyFormatCombo, &QComboBox::currentTextChanged, this, &TableView::onCurrencyFormatChanged);
    connect(dateFormatCombo, &QComboBox::currentTextChanged, this, &TableView::onDateFormatChanged);
    connect(millaresDecimalsCombo, &QComboBox::currentTextChanged, this, &TableView::onMillaresDecimalsChanged);
}

void TableView::updateSpecificProperties(const QString &dataType)
{
    // Ocultar todos los widgets de propiedades específicas
    textPropertiesWidget->hide();
    numberPropertiesWidget->hide();
    currencyPropertiesWidget->hide();
    datePropertiesWidget->hide();
    
    // Mostrar el widget correspondiente según el tipo de dato
    if (dataType == "Texto corto" || dataType == "Texto largo") {
        textPropertiesWidget->show();
        
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldTextSizes.size() <= currentSelectedRow) {
            fieldTextSizes.append("255"); // Valor por defecto
        }
        
        // Bloquear señales para evitar ciclos
        textSizeEdit->blockSignals(true);
        
        if (dataType == "Texto corto") {
            textSizeEdit->setPlaceholderText("Máximo 255 caracteres (ej: 50)");
            
            // Cargar el valor guardado para esta fila
            QString savedSize = fieldTextSizes[currentSelectedRow];
            if (!savedSize.isEmpty()) {
                textSizeEdit->setText(savedSize);
            } else {
                textSizeEdit->setText("50");
                fieldTextSizes[currentSelectedRow] = "50";
            }
            
            textValidationLabel->setText("(Máx. 255 caracteres)");
            textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
        } else {
            textSizeEdit->setPlaceholderText("Tamaño ilimitado por defecto");
            
            // Cargar el valor guardado para esta fila
            QString savedSize = fieldTextSizes[currentSelectedRow];
            if (!savedSize.isEmpty()) {
                textSizeEdit->setText(savedSize);
            } else {
                textSizeEdit->setText("Sin límite");
                fieldTextSizes[currentSelectedRow] = "Sin límite";
            }
            
            textValidationLabel->setText("(Texto de longitud ilimitada)");
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-style: italic; }");
        }
        
        // Reactivar señales
        textSizeEdit->blockSignals(false);
    } else if (dataType == "Números") {
    numberPropertiesWidget->show();

    // Asegurar que la lista tenga el tamaño correcto
    while (fieldNumberTypes.size() <= currentSelectedRow) {
        fieldNumberTypes.append("Entero"); // Valor por defecto
    }

    // Bloquear señales para evitar ciclos
    numberTypeCombo->blockSignals(true);

    // Cargar el tipo guardado para esta fila
    if (currentSelectedRow >= 0 && currentSelectedRow < fieldNumberTypes.size()) {
        QString savedType = fieldNumberTypes[currentSelectedRow];
        if (!savedType.isEmpty()) {
            numberTypeCombo->setCurrentText(savedType);
        } else {
            numberTypeCombo->setCurrentText("Entero");
            fieldNumberTypes[currentSelectedRow] = "Entero";
        }
    } else {
        numberTypeCombo->setCurrentText("Entero");
    }

    // === Define currentType UNA sola vez y reutiliza ===
    QString currentType = numberTypeCombo->currentText();

    // Actualizar el label de información de tamaño según el tipo seleccionado
    QString sizeInfo;
    if (currentType == "Entero") {
        sizeInfo = "Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)";
    } else if (currentType == "Decimal") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión)";
    } else if (currentType == "Doble") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión, mayor rango)";
    } else if (currentType == "Byte") {
        sizeInfo = "Tamaño: 8 bits (0 a 255)";
    }
    numberSizeLabel->setText(sizeInfo);

    numberTypeCombo->blockSignals(false);

    // ----- Decimales (solo Decimal/Doble) -----
    while (fieldDecimalPlaces.size() <= currentSelectedRow) {
        fieldDecimalPlaces.append("2"); // default 2
    }

    const bool needsDecimals = (currentType == "Decimal" || currentType == "Doble");
    if (numberDecimalsLabel) numberDecimalsLabel->setVisible(needsDecimals);
    if (numberDecimalsCombo) {
        numberDecimalsCombo->blockSignals(true);
        numberDecimalsCombo->setVisible(needsDecimals);
        numberDecimalsCombo->setEnabled(needsDecimals);
        if (needsDecimals) {
            QString savedDec = fieldDecimalPlaces[currentSelectedRow];
            if (savedDec.isEmpty()) savedDec = "2";
            numberDecimalsCombo->setCurrentText(savedDec);
            numberDecimalsCombo->setToolTip("Número de decimales para este campo (0–6).");
        }
        numberDecimalsCombo->blockSignals(false);
    }


    } else if (dataType == "moneda") {
        currencyPropertiesWidget->show();
        
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldCurrencyFormats.size() <= currentSelectedRow) {
            fieldCurrencyFormats.append("Lempiras (Lps)");
        }
        
        // Asegurar que la lista de decimales tenga el tamaño correcto
        while (fieldMillaresDecimals.size() <= currentSelectedRow) {
            fieldMillaresDecimals.append("2");
        }
        
        // Bloquear señales para evitar ciclos
        currencyFormatCombo->blockSignals(true);
        millaresDecimalsCombo->blockSignals(true);
        
        // Cargar el formato guardado para esta fila
        if (currentSelectedRow >= 0 && currentSelectedRow < fieldCurrencyFormats.size()) {
            QString savedFormat = fieldCurrencyFormats[currentSelectedRow];
            if (!savedFormat.isEmpty()) {
                currencyFormatCombo->setCurrentText(savedFormat);
            } else {
                currencyFormatCombo->setCurrentText("Lempiras (Lps)");
                fieldCurrencyFormats[currentSelectedRow] = "Lempiras (Lps)";
            }
        } else {
            currencyFormatCombo->setCurrentText("Lempiras (Lps)");
        }
        
        // Mostrar combo de decimales para TODOS los tipos de moneda
        millaresDecimalsLabel->show();
        millaresDecimalsCombo->show();
        
        // Cargar decimales guardados
        if (currentSelectedRow < fieldMillaresDecimals.size()) {
            millaresDecimalsCombo->setCurrentText(fieldMillaresDecimals[currentSelectedRow]);
        }
        
        // Reactivar señales
        currencyFormatCombo->blockSignals(false);
        millaresDecimalsCombo->blockSignals(false);
    } else if (dataType == "fecha") {
        datePropertiesWidget->show();
        
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldDateFormats.size() <= currentSelectedRow) {
            fieldDateFormats.append("DD-MM-YY"); // Valor por defecto
        }
        
        // Bloquear señales para evitar ciclos
        dateFormatCombo->blockSignals(true);
        
        // Cargar el formato guardado para esta fila
        if (currentSelectedRow >= 0 && currentSelectedRow < fieldDateFormats.size()) {
            QString savedFormat = fieldDateFormats[currentSelectedRow];
            if (!savedFormat.isEmpty()) {
                dateFormatCombo->setCurrentText(savedFormat);
            } else {
                dateFormatCombo->setCurrentText("DD-MM-YY");
                fieldDateFormats[currentSelectedRow] = "DD-MM-YY";
            }
        } else {
            dateFormatCombo->setCurrentText("DD-MM-YY");
        }
        
        // Reactivar señales
        dateFormatCombo->blockSignals(false);
    }
}

void TableView::onTextSizeChanged(const QString &text)
{
    if (text.isEmpty()) return;
    
    // Asegurar que la lista tenga el tamaño correcto
    while (fieldTextSizes.size() <= currentSelectedRow) {
        fieldTextSizes.append("255"); // Valor por defecto
    }
    
    // Validar que el valor no sea mayor a 255 para campos de texto corto
    bool ok;
    int size = text.toInt(&ok);
    
    // Obtener el tipo de dato actual
    QTableWidgetItem *typeItem = tableWidget->item(currentSelectedRow, 1);
    QString dataType = typeItem ? typeItem->text() : "Texto corto";
    
    if (dataType == "Texto largo") {
        // Para texto largo, permitir "Sin límite" o números
        if (text.contains("Sin límite") || text.contains("ilimitado")) {
            fieldTextSizes[currentSelectedRow] = "Sin límite";
            textValidationLabel->setText("✅ Texto de longitud ilimitada");
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-weight: bold; }");
        } else if (ok && size > 0) {
            fieldTextSizes[currentSelectedRow] = text;
            textValidationLabel->setText(QString("✅ Tamaño válido: %1 caracteres").arg(size));
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-weight: bold; }");
        } else {
            textValidationLabel->setText("⚠️ Ingrese un número válido o 'Sin límite'");
            textValidationLabel->setStyleSheet("QLabel { color: #ef4444; font-size: 12px; font-weight: bold; }");
            return;
        }
    } else {
        // Para texto corto, validar límite de 255
        if (ok && size > 255) {
            // Si el valor es mayor a 255, limitarlo a 255
            textSizeEdit->blockSignals(true);
            textSizeEdit->setText("255");
            textSizeEdit->blockSignals(false);
            
            fieldTextSizes[currentSelectedRow] = "255";
            
            // Mostrar mensaje de error en el label de validación
            textValidationLabel->setText("⚠️ Máximo permitido: 255 caracteres");
            textValidationLabel->setStyleSheet("QLabel { color: #ef4444; font-size: 12px; font-weight: bold; }");
            
            // Cambiar el estilo del input
            textSizeEdit->setStyleSheet(getInputStyle() + 
                "QLineEdit { border: 2px solid #ef4444; background-color: #fef2f2; }");
            
            // Usar un timer para restaurar el estilo normal después de 3 segundos
            QTimer::singleShot(3000, [this]() {
                textSizeEdit->setStyleSheet(getInputStyle());
                textValidationLabel->setText("(Máx. 255 caracteres)");
                textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
            });
            
            qDebug() << "DEBUG: Tamaño de texto limitado a 255 caracteres";
        } else if (!ok) {
            // Si no es un número válido, mostrar error
            textValidationLabel->setText("⚠️ Ingrese un número válido (1-255)");
            textValidationLabel->setStyleSheet("QLabel { color: #ef4444; font-size: 12px; font-weight: bold; }");
            
            textSizeEdit->setStyleSheet(getInputStyle() + 
                "QLineEdit { border: 2px solid #ef4444; background-color: #fef2f2; }");
            
            QTimer::singleShot(3000, [this]() {
                textSizeEdit->setStyleSheet(getInputStyle());
                textValidationLabel->setText("(Máx. 255 caracteres)");
                textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
            });
            return;
        } else if (ok && size > 0 && size <= 255) {
            // Valor válido, guardarlo
            fieldTextSizes[currentSelectedRow] = text;
            
            // Restablecer estilo normal
            textSizeEdit->setStyleSheet(getInputStyle());
            textValidationLabel->setText(QString("✅ Tamaño válido: %1 caracteres").arg(size));
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-weight: bold; }");
            
            // Restaurar texto normal después de 2 segundos
            QTimer::singleShot(2000, [this]() {
                textValidationLabel->setText("(Máx. 255 caracteres)");
                textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
            });
        }
    }
    
    qDebug() << "DEBUG: Tamaño de texto cambiado a:" << text << "para fila:" << currentSelectedRow;
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onNumberTypeChanged(const QString &text)
{
    qDebug() << "DEBUG: Tipo de número cambiado a:" << text;
    qDebug() << "DEBUG: Fila actual seleccionada:" << currentSelectedRow;

    // ----- Info de tamaño según tipo -----
    QString sizeInfo;
    if (text == "Entero") {
        sizeInfo = "Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)";
    } else if (text == "Decimal") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión)";
    } else if (text == "Doble") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión, mayor rango)";
    } else if (text == "Byte") {
        sizeInfo = "Tamaño: 8 bits (0 a 255)";
    }

    if (!sizeInfo.isEmpty() && numberSizeLabel) {
        numberSizeLabel->setText(sizeInfo);
        qDebug() << "DEBUG: Información de tamaño actualizada:" << sizeInfo;
    }
    if (!sizeInfo.isEmpty() && numberTypeCombo) {
        numberTypeCombo->setToolTip(sizeInfo);
    }

    // ----- Guardar el tipo por fila -----
    if (currentSelectedRow >= 0) {
        while (fieldNumberTypes.size() <= currentSelectedRow)
            fieldNumberTypes.append("Entero");
        fieldNumberTypes[currentSelectedRow] = text;
        qDebug() << "DEBUG: Guardado tipo de número" << text << "para fila" << currentSelectedRow;
        qDebug() << "DEBUG: Lista completa de tipos de números:" << fieldNumberTypes;
    }

    // ----- NUEVO: Decimales visibles solo para Decimal/Doble -----
    const bool needsDecimals = (text == "Decimal" || text == "Doble");

    if (numberDecimalsLabel)
        numberDecimalsLabel->setVisible(needsDecimals);

    if (numberDecimalsCombo) {
        numberDecimalsCombo->setVisible(needsDecimals);
        numberDecimalsCombo->setEnabled(needsDecimals);

        if (needsDecimals && currentSelectedRow >= 0) {
            // Asegurar storage y cargar valor guardado
            while (fieldDecimalPlaces.size() <= currentSelectedRow)
                fieldDecimalPlaces.append("2"); // default 2

            QString savedDec = fieldDecimalPlaces[currentSelectedRow];
            if (savedDec.isEmpty()) savedDec = "2";

            numberDecimalsCombo->blockSignals(true);
            numberDecimalsCombo->setCurrentText(savedDec);
            numberDecimalsCombo->blockSignals(false);

            numberDecimalsCombo->setToolTip("Número de decimales para este campo (0–6).");
            qDebug() << "DEBUG: Decimales visibles. Valor para fila" << currentSelectedRow << ":" << savedDec;
        } else {
            qDebug() << "DEBUG: Decimales ocultos para tipo:" << text;
        }
    }

    // ----- Emitir señales existentes -----
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
    emit tableDesignChangedWithAllFormats(
        getCurrentFieldNames(),
        getCurrentFieldTypes(),
        getCurrentCurrencyFormats(),
        getCurrentMillaresDecimals(),
        getCurrentNumberTypes(),
        getCurrentDateFormats()
        // Nota: si luego quieres propagar también los decimales de números,
        // puedes extender este emit para incluir getCurrentDecimalPlaces().
        );
}

void TableView::onCurrencyFormatChanged(const QString &text)
{
    qDebug() << "DEBUG: Formato de moneda cambiado a:" << text;
    qDebug() << "DEBUG: Fila actual seleccionada:" << currentSelectedRow;
    
    // El combo de decimales siempre se muestra para todos los tipos de moneda
    millaresDecimalsLabel->show();
    millaresDecimalsCombo->show();
    
    // Asegurar que la lista de decimales tenga el tamaño correcto
    while (fieldMillaresDecimals.size() <= currentSelectedRow) {
        fieldMillaresDecimals.append("2"); // Valor por defecto: 2 decimales
    }
    
    // Cargar el valor guardado para esta fila
    if (currentSelectedRow >= 0 && currentSelectedRow < fieldMillaresDecimals.size()) {
        millaresDecimalsCombo->blockSignals(true);
        millaresDecimalsCombo->setCurrentText(fieldMillaresDecimals[currentSelectedRow]);
        millaresDecimalsCombo->blockSignals(false);
    }
    
    // Guardar el formato para el campo actual
    if (currentSelectedRow >= 0) {
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldCurrencyFormats.size() <= currentSelectedRow) {
            fieldCurrencyFormats.append("Lempiras (Lps)");
        }
        
        // Guardar el formato seleccionado para esta fila
        fieldCurrencyFormats[currentSelectedRow] = text;
        qDebug() << "DEBUG: Guardado formato" << text << "para fila" << currentSelectedRow;
        qDebug() << "DEBUG: Lista completa de formatos:" << fieldCurrencyFormats;
    }
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
    // Emitir señal específica con formatos de moneda y decimales
    emit tableDesignChangedWithFormatsAndDecimals(getCurrentFieldNames(), getCurrentFieldTypes(), getCurrentCurrencyFormats(), getCurrentMillaresDecimals());
}

void TableView::onDateFormatChanged(const QString &text)
{
    qDebug() << "DEBUG: Formato de fecha cambiado a:" << text;
    qDebug() << "DEBUG: Fila actual seleccionada:" << currentSelectedRow;
    
    // Mostrar información sobre el formato seleccionado
    QString formatInfo = "";
    if (text == "DD-MM-YY") {
        formatInfo = "Formato: Día-Mes-Año (ejemplo: 25-12-23)";
    } else if (text == "DD/MM/YY") {
        formatInfo = "Formato: Día/Mes/Año (ejemplo: 25/12/23)";
    }
    
    qDebug() << "DEBUG: Información de formato:" << formatInfo;
    
    // Guardar el formato de fecha para el campo actual
    if (currentSelectedRow >= 0) {
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldDateFormats.size() <= currentSelectedRow) {
            fieldDateFormats.append("DD-MM-YY"); // Valor por defecto
        }
        
        // Guardar el formato seleccionado para esta fila
        fieldDateFormats[currentSelectedRow] = text;
        qDebug() << "DEBUG: Guardado formato de fecha" << text << "para fila" << currentSelectedRow;
        qDebug() << "DEBUG: Lista completa de formatos de fecha:" << fieldDateFormats;
    }
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
    // Emitir señal específica con todos los formatos incluyendo formatos de fecha
    emit tableDesignChangedWithAllFormats(getCurrentFieldNames(), getCurrentFieldTypes(), getCurrentCurrencyFormats(), getCurrentMillaresDecimals(), getCurrentNumberTypes(), getCurrentDateFormats());
}

void TableView::onMillaresDecimalsChanged(const QString &text)
{
    qDebug() << "DEBUG: Decimales de millares cambiado a:" << text;
    qDebug() << "DEBUG: Fila actual seleccionada:" << currentSelectedRow;
    
    // Guardar el número de decimales para el campo actual
    if (currentSelectedRow >= 0) {
        // Asegurar que la lista tenga el tamaño correcto
        while (fieldMillaresDecimals.size() <= currentSelectedRow) {
            fieldMillaresDecimals.append("2");
        }
        
        // Guardar el número de decimales seleccionado para esta fila
        fieldMillaresDecimals[currentSelectedRow] = text;
        qDebug() << "DEBUG: Guardados" << text << "decimales para fila" << currentSelectedRow;
        qDebug() << "DEBUG: Lista completa de decimales:" << fieldMillaresDecimals;
    }
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
    emit tableDesignChangedWithFormatsAndDecimals(getCurrentFieldNames(), getCurrentFieldTypes(), getCurrentCurrencyFormats(), getCurrentMillaresDecimals());
}

QString TableView::generateExampleData(const QString &dataType, int column)
{
    Q_UNUSED(column) // Por ahora no usamos la columna, pero puede ser útil en el futuro
    
    if (dataType == "Entero") {
        QString numberType = numberTypeCombo ? numberTypeCombo->currentText() : "Entero";
        if (numberType == "Byte") {
            return "123";
        } else if (numberType == "Entero") {
            return "12345";
        } else {
            return "12345";
        }
    } else if (dataType == "Decimales") {
        QString numberType = numberTypeCombo ? numberTypeCombo->currentText() : "Decimal";
        if (numberType == "Decimal") {
            return "123.45";
        } else if (numberType == "Doble") {
            return "123.456789";
        } else {
            return "123.45";
        }
    } else if (dataType == "Sí / No") {
        return "Sí";
    } else if (dataType == "Texto corto") {
        QString size = textSizeEdit ? textSizeEdit->text() : "50";
        if (size == "Sin límite" || size.contains("ilimitado")) {
            return "Texto de ejemplo";
        } else {
            bool ok;
            int maxSize = size.toInt(&ok);
            if (ok && maxSize > 0) {
                QString example = "Ejemplo de texto";
                if (example.length() > maxSize) {
                    return example.left(maxSize - 3) + "...";
                }
                return example;
            }
            return "Texto de ejemplo";
        }
    } else if (dataType == "Texto largo") {
        return "Este es un ejemplo de texto largo que puede contener múltiples líneas...";
    } else if (dataType == "moneda") {
        QString format = currencyFormatCombo ? currencyFormatCombo->currentText() : "Lempiras (Lps)";
        if (format.contains("Lempiras")) {
            return "Lps 1,500.00";
        } else if (format.contains("Dollar")) {
            return "$1,500.00";
        } else if (format.contains("Euros")) {
            return "€1,500.00";
        } else if (format.contains("Millares")) {
            return "1,500";
        } else {
            return "Lps 1,500.00";
        }
    } else if (dataType == "fecha") {
        QString format = dateFormatCombo ? dateFormatCombo->currentText() : "DD-MM-YY";
        if (format == "DD-MM-YY") {
            return "15-08-24";
        } else if (format == "DD/MM/YY") {
            return "15/08/24";
        } else {
            return "15-08-24";
        }
    }
    
    return "Ejemplo";
}

void TableView::updateExampleData()
{
    // Verificar que la tabla existe
    if (!tableWidget) {
        qDebug() << "DEBUG: TableWidget is null, cannot update example data";
        return;
    }
    
    // Bloquear señales para evitar bucles infinitos
    tableWidget->blockSignals(true);
    
    // Solo actualizar si hay al menos una fila de campo definido en la tabla
    if (tableWidget->rowCount() <= 1) {
        qDebug() << "DEBUG: Not enough rows to create example data";
        tableWidget->blockSignals(false);
        return;
    }
    
    // Buscar si ya existe una fila de ejemplo al principio
    bool hasExampleRow = false;
    if (tableWidget->rowCount() > 0) {
        QTableWidgetItem *firstItem = tableWidget->item(0, 0);
        if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
            hasExampleRow = true;
        }
    }
    
    // Si no hay fila de ejemplo, crear una
    if (!hasExampleRow) {
        // Insertar fila de ejemplo al principio
        tableWidget->insertRow(0);
        
        // Crear items para la fila de ejemplo
        for (int col = 0; col < 3; col++) { // 3 columnas: Nombre, Tipo, Descripción
            QTableWidgetItem *exampleItem = new QTableWidgetItem("");
            
            // Configurar estilo para datos de ejemplo
            QFont exampleFont = exampleItem->font();
            exampleFont.setPointSize(14);
            exampleFont.setItalic(true); // Cursiva para indicar que es ejemplo
            exampleItem->setFont(exampleFont);
            
            // Color gris para indicar que es ejemplo
            exampleItem->setForeground(QBrush(QColor(156, 163, 175))); // Color gris
            exampleItem->setBackground(QBrush(QColor(249, 250, 251))); // Fondo gris muy claro
            
            // No editable
            exampleItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            
            // Tooltip para identificar que es ejemplo
            exampleItem->setToolTip("Fila de ejemplo - muestra cómo se verán los datos");
            
            tableWidget->setItem(0, col, exampleItem);
        }
    }
    
    // Buscar el primer campo real (no de ejemplo) para generar los ejemplos
    int firstDataRow = hasExampleRow ? 1 : 0;
    
    // Actualizar el contenido de la fila de ejemplo basándose en el primer campo definido
    for (int dataRow = firstDataRow; dataRow < tableWidget->rowCount(); dataRow++) {
        QTableWidgetItem *fieldNameItem = tableWidget->item(dataRow, 0);
        QTableWidgetItem *dataTypeItem = tableWidget->item(dataRow, 1);
        
        if (fieldNameItem && dataTypeItem && !dataTypeItem->text().isEmpty()) {
            QString fieldName = fieldNameItem->text();
            QString dataType = dataTypeItem->text();
            
            // Actualizar nombre del campo en la fila de ejemplo basado en el primer campo real
            QTableWidgetItem *exampleNameItem = tableWidget->item(0, 0);
            if (exampleNameItem) {
                if (!fieldName.isEmpty()) {
                    QString example = generateExampleData(dataType, 0);
                    exampleNameItem->setText(example);
                } else {
                    exampleNameItem->setText("Ejemplo de dato");
                }
            }
            
            // Actualizar el tipo en la fila de ejemplo
            QTableWidgetItem *exampleTypeItem = tableWidget->item(0, 1);
            if (exampleTypeItem) {
                exampleTypeItem->setText("(Ejemplo)");
            }
            
            // Actualizar descripción en la fila de ejemplo
            QTableWidgetItem *exampleDescItem = tableWidget->item(0, 2);
            if (exampleDescItem) {
                exampleDescItem->setText(QString("Vista previa de tipo: %1").arg(dataType));
            }
            
            // Solo usamos el primer campo para generar el ejemplo, entonces salimos del bucle
            break;
        }
    }
    
    // Si no se encontró ningún campo definido, mostrar texto genérico
    bool foundDefinedField = false;
    for (int dataRow = firstDataRow; dataRow < tableWidget->rowCount(); dataRow++) {
        QTableWidgetItem *dataTypeItem = tableWidget->item(dataRow, 1);
        if (dataTypeItem && !dataTypeItem->text().isEmpty()) {
            foundDefinedField = true;
            break;
        }
    }
    
    if (!foundDefinedField) {
        QTableWidgetItem *exampleNameItem = tableWidget->item(0, 0);
        if (exampleNameItem) {
            exampleNameItem->setText("Ejemplo de dato");
        }
        
        QTableWidgetItem *exampleTypeItem = tableWidget->item(0, 1);
        if (exampleTypeItem) {
            exampleTypeItem->setText("(Ejemplo)");
        }
        
        QTableWidgetItem *exampleDescItem = tableWidget->item(0, 2);
        if (exampleDescItem) {
            exampleDescItem->setText("Vista previa de datos");
        }
    }
    
    // Restaurar señales
    tableWidget->blockSignals(false);
}

void TableView::validatePrimaryKeyIntegrity()
{
    // Si no hay llave primaria definida, no hay nada que validar
    if (primaryKeyRow == -1) {
        return;
    }
    
    // Verificar que la fila de llave primaria aún existe y tiene datos válidos
    if (primaryKeyRow >= tableWidget->rowCount()) {
        // La fila de llave primaria fue eliminada
        qDebug() << "DEBUG: Llave primaria fue eliminada. Reseteando primaryKeyRow.";
        primaryKeyRow = -1;
        return;
    }
    
    // Verificar que el item de llave primaria aún existe y no está vacío
    QTableWidgetItem *primaryKeyItem = tableWidget->item(primaryKeyRow, 0);
    if (!primaryKeyItem || primaryKeyItem->text().trimmed().isEmpty()) {
        // El campo de llave primaria está vacío
        qDebug() << "DEBUG: Campo de llave primaria está vacío. Reseteando primaryKeyRow.";
        primaryKeyRow = -1;
        return;
    }
    
    // Si llegamos aquí, la llave primaria es válida
    QString fieldName = primaryKeyItem->text();
    if (!fieldName.startsWith("🔑 ")) {
        // Restaurar el icono de llave si fue removido accidentalmente
        if (fieldName.startsWith("🔑 ")) {
            fieldName = fieldName.mid(3);
        }
        primaryKeyItem->setText("🔑 " + fieldName);
        primaryKeyItem->setToolTip("Campo Llave Primaria - Requerido y único");
        qDebug() << "DEBUG: Icono de llave primaria restaurado para:" << fieldName;
    }
}

bool TableView::checkForDuplicates(const QString &fieldName)
{
    qDebug() << "DEBUG: Verificando duplicados para campo:" << fieldName;
    
    // Encontrar el índice de la columna de este campo
    int fieldIndex = -1;
    QStringList currentFields = getCurrentFieldNames();
    
    for (int i = 0; i < currentFields.size(); i++) {
        if (currentFields[i] == fieldName) {
            fieldIndex = i;
            break;
        }
    }
    
    if (fieldIndex == -1) {
        qDebug() << "DEBUG: Campo no encontrado en la lista de campos actuales";
        return false;
    }
    
    // Emitir señal para que el TableEditor/TableData verifique duplicados
    emit checkUniqueFieldDuplicates(fieldName, fieldIndex);
    
    // Por ahora retornamos false y esperamos la respuesta asíncrona
    // La validación real se manejará en setUniqueValidationResult()
    return false;
}

void TableView::setUniqueValidationResult(const QString &fieldName, bool hasDuplicates)
{
    qDebug() << "DEBUG: Resultado de validación unique para" << fieldName << ":" << hasDuplicates;
    
    if (hasDuplicates) {
        // Buscar la fila del campo para desmarcarlo
        int fieldRow = -1;
        for (int row = 0; row < tableWidget->rowCount(); row++) {
            QTableWidgetItem *item = tableWidget->item(row, 0);
            if (item) {
                QString itemText = item->text();
                // Limpiar iconos para comparar
                QString cleanText = itemText;
                cleanText = cleanText.remove("🔑🔗🔶");
                cleanText = cleanText.remove("🔑🔗");
                cleanText = cleanText.remove("🔑🔶");
                cleanText = cleanText.remove("🔗🔶");
                cleanText = cleanText.remove("🔑");
                cleanText = cleanText.remove("🔗");
                cleanText = cleanText.remove("🔶");
                cleanText = cleanText.trimmed();
                
                if (cleanText == fieldName) {
                    fieldRow = row;
                    break;
                }
            }
        }
        
        if (fieldRow != -1) {
            // Mostrar mensaje de error más específico
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("⚠️ Datos Duplicados Detectados");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("<h3>Campo Unique con Valores Duplicados</h3>");
            msgBox.setInformativeText(
                QString("El campo <b>'%1'</b> contiene valores duplicados en los datos existentes.<br><br>"
                        "❌ <b>Los campos Unique no pueden tener valores repetidos.</b><br><br>"
                        "Opciones disponibles:<br>"
                        "• <b>Limpiar datos duplicados</b> y luego marcar como Unique<br>"
                        "• <b>Mantener los datos actuales</b> sin restricción de unicidad<br><br>"
                        "Se ha revertido la marca de Unique automáticamente.").arg(fieldName)
            );
            msgBox.setStandardButtons(QMessageBox::Ok);
            
            // Estilo del mensaje
            msgBox.setStyleSheet(
                "QMessageBox {"
                "background-color: white;"
                "min-width: 480px;"
                "min-height: 200px;"
                "}"
                "QMessageBox QLabel {"
                "color: #0b0f19;"
                "font-size: 14px;"
                "}"
            );
            
            msgBox.exec();
            
            // Revertir automáticamente la marca de unique
            uniqueKeyRows.removeAll(fieldRow);
            
            // Actualizar el checkbox en la interfaz
            if (fieldRow == currentSelectedRow) {
                uniqueCheck->blockSignals(true);
                uniqueCheck->setChecked(false);
                uniqueCheck->blockSignals(false);
            }
            
            // Actualizar el icono del campo
            QTableWidgetItem *fieldItem = tableWidget->item(fieldRow, 0);
            if (fieldItem) {
                bool isPrimaryKey = (primaryKeyRow == fieldRow);
                bool isForeignKey = foreignKeyRows.contains(fieldRow);
                
                QString newText = fieldName;
                QString toolTip = "";
                
                if (isPrimaryKey && isForeignKey) {
                    newText = "🔑🔗 " + fieldName;
                    toolTip = "Campo Primary Key con Foreign Key";
                } else if (isPrimaryKey) {
                    newText = "🔑 " + fieldName;
                    toolTip = "Campo Primary Key - Clave única e irrepetible";
                } else if (isForeignKey) {
                    newText = "🔗 " + fieldName;
                    toolTip = "Campo Foreign Key - Referencia a otra tabla";
                } else {
                    newText = fieldName;
                    toolTip = "";
                }
                
                tableWidget->blockSignals(true);
                fieldItem->setText(newText);
                fieldItem->setToolTip(toolTip);
                tableWidget->blockSignals(false);
            }
            
            qDebug() << "DEBUG: Campo" << fieldName << "revertido de unique debido a duplicados";
        }
    } else {
        qDebug() << "DEBUG: Campo" << fieldName << "validado como único correctamente";
    }
}
