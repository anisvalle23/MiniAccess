#include "TableView.h"
#include <QDebug>

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
    comboBox->addItems({"Entero", "Decimales", "Sí / No", "Texto corto (hasta N caracteres)", "Texto largo / Párrafo", "moneda", "fecha"});
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

// TableView Implementation - SOLO PARA VISTA DISEÑO
TableView::TableView(QWidget *parent) : QWidget(parent)
{
    // Inicializar variables
    currentSelectedRow = -1;
    isDarkTheme = false;
    currentTableName = "Nueva Tabla";
    
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
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8fafc, stop:1 #e2e8f0);"
        "border-bottom: 1px solid #cbd5e1;"
        "}"
    );
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // Título de la tabla (solo el nombre)
    tableNameLabel = new QLabel(currentTableName);
    tableNameLabel->setFont(QFont("Arial", 18, QFont::Bold));
    tableNameLabel->setStyleSheet("QLabel { color: #1e293b; }");
    
    headerLayout->addWidget(tableNameLabel);
    headerLayout->addStretch();
    
    // Contenedor para los botones
    QWidget *buttonContainer = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    
    // Botón Vista Diseño (activo por defecto)
    QPushButton *designViewBtn = new QPushButton("Vista Diseño");
    designViewBtn->setFixedSize(120, 35);
    designViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3b82f6, stop:1 #2563eb);"
        "color: white;"
        "border: none;"
        "border-top-left-radius: 6px;"
        "border-bottom-left-radius: 6px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2563eb, stop:1 #1d4ed8);"
        "}"
    );
    
    // Botón Vista Datos
    dataViewBtn = new QPushButton("Vista Datos");
    dataViewBtn->setFixedSize(120, 35);
    dataViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e5e7eb, stop:1 #d1d5db);"
        "color: #374151;"
        "border: none;"
        "border-top-right-radius: 6px;"
        "border-bottom-right-radius: 6px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
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
    dataTypeCombo->addItems({"Entero", "Decimales", "Sí / No", "Texto corto (hasta N caracteres)", "Texto largo / Párrafo", "moneda", "fecha"});
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
    requiredCheck = new QCheckBox("Campo Requerido");
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
            item->setText("Entero"); // Tipo por defecto para ID
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
    // Si se escribió en la primera columna, asegurar que hay filas vacías
    if (column == 0) {
        ensureEmptyRowExists();
    }
}

void TableView::onCellSelectionChanged()
{
    currentSelectedRow = tableWidget->currentRow();
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
    
    // Obtener datos de la fila
    QTableWidgetItem *nameItem = tableWidget->item(row, 0);
    QTableWidgetItem *typeItem = tableWidget->item(row, 1);
    QTableWidgetItem *descItem = tableWidget->item(row, 2);
    
    // Actualizar propiedades
    fieldNameEdit->setText(nameItem ? nameItem->text() : "");
    
    QString dataType = typeItem ? typeItem->text() : "TEXT";
    int index = dataTypeCombo->findText(dataType);
    if (index >= 0) {
        dataTypeCombo->setCurrentIndex(index);
    }
    
    descriptionEdit->setPlainText(descItem ? descItem->text() : "");
    defaultValueEdit->setText("");
    
    // Campo requerido si es ID
    QString fieldName = nameItem ? nameItem->text().toLower() : "";
    requiredCheck->setChecked(fieldName == "id");
    
    // Actualizar propiedades específicas según el tipo de dato
    updateSpecificProperties(dataType);
    
    // Reactivar señales
    fieldNameEdit->blockSignals(false);
    dataTypeCombo->blockSignals(false);
    descriptionEdit->blockSignals(false);
    defaultValueEdit->blockSignals(false);
    requiredCheck->blockSignals(false);
    
    // Actualizar propiedades específicas según el tipo de dato seleccionado
    updateSpecificProperties(dataType);
}

void TableView::onFieldNameChanged(const QString &text)
{
    if (currentSelectedRow >= 0) {
        QTableWidgetItem *item = tableWidget->item(currentSelectedRow, 0);
        if (item) {
            item->setText(text);
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
    Q_UNUSED(required)
    // Por ahora no mostramos si es requerido en la tabla
}

void TableView::onDataViewClicked()
{
    qDebug() << "DEBUG: Cambiando a Vista Datos";
    emit switchToDataView();
}

void TableView::onDesignViewClicked()
{
    qDebug() << "DEBUG: Ya estoy en Vista Diseño";
    // No necesita hacer nada ya que estamos en la vista de diseño
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
        
        // Buscar duplicados (sin importar mayúsculas/minúsculas)
        for (int checkRow = 0; checkRow < tableWidget->rowCount(); checkRow++) {
            if (checkRow == row) continue; // Saltar la fila actual
            
            QTableWidgetItem *checkItem = tableWidget->item(checkRow, 0);
            if (checkItem && !checkItem->text().trimmed().isEmpty()) {
                QString existingName = checkItem->text().trimmed();
                
                // Comparación sin considerar mayúsculas/minúsculas
                if (fieldName.toLower() == existingName.toLower()) {
                    // Mostrar mensaje de error
                    QMessageBox::warning(this, "Campo Duplicado", 
                        QString("Ya existe un campo con el nombre '%1'.\n"
                                "Los nombres de los campos deben ser únicos (sin importar mayúsculas/minúsculas).")
                                .arg(existingName));
                    


                                
                    // Limpiar el campo duplicado usando blockSignals del widget table
                    tableWidget->blockSignals(true);
                    item->setText("");
                    tableWidget->blockSignals(false);
                    
                    // Enfocar en el campo para que el usuario pueda escribir un nombre diferente
                    tableWidget->setCurrentItem(item);
                    tableWidget->editItem(item);
                    
                    qDebug() << "DEBUG: Campo duplicado detectado:" << fieldName << "vs" << existingName;
                    return; // Salir sin procesar más
                }
            }
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
                if (col == 0 && nextItem->text().isEmpty()) { // Si se escribió en "Nombre del Campo"
                    nextItem->setText("Texto largo / Párrafo"); // Tipo de datos por defecto
                }
            }
        }
        
        // Si completamos una fila, agregar una nueva fila
        if (col == tableWidget->columnCount() - 1) { // Si es la última columna
            addNewRow();
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
    // Actualizar estilos según el tema
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
            if (!fieldName.isEmpty()) {
                fieldNames << fieldName;
                qDebug() << "DEBUG: Added field name:" << fieldName;
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentFieldNames() returning:" << fieldNames;
    return fieldNames;
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
            if (!fieldType.isEmpty()) {
                fieldTypes << fieldType;
                qDebug() << "DEBUG: Added field type:" << fieldType;
            }
        }
    }
    
    qDebug() << "DEBUG: getCurrentFieldTypes() returning:" << fieldTypes;
    return fieldTypes;
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
    
    // Label para mostrar información del tamaño del campo
    numberSizeLabel = new QLabel("Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)");
    numberSizeLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
    numberSizeLabel->setWordWrap(true);
    
    numberMainLayout->addWidget(numberInputWidget);
    numberMainLayout->addWidget(numberSizeLabel);
    
    // Widget para propiedades de moneda
    currencyPropertiesWidget = new QWidget();
    currencyPropertiesWidget->setMinimumHeight(40); // Altura mínima
    QHBoxLayout *currencyLayout = new QHBoxLayout(currencyPropertiesWidget);
    currencyLayout->setContentsMargins(0, 5, 0, 5); // Margen vertical
    currencyLayout->setSpacing(15);
    
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
    currencyLayout->addStretch();
    
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
    dateFormatCombo->addItems({"DD-MM-YY", "DD/MM/YY", "DD/MESTEXTO/YYYY"});
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
}

void TableView::updateSpecificProperties(const QString &dataType)
{
    // Ocultar todos los widgets de propiedades específicas
    textPropertiesWidget->hide();
    numberPropertiesWidget->hide();
    currencyPropertiesWidget->hide();
    datePropertiesWidget->hide();
    
    // Mostrar el widget correspondiente según el tipo de dato
    if (dataType == "Texto corto (hasta N caracteres)" || dataType == "Texto largo / Párrafo") {
        textPropertiesWidget->show();
        if (dataType == "Texto corto (hasta N caracteres)") {
            textSizeEdit->setPlaceholderText("Máximo 255 caracteres (ej: 50)");
            textSizeEdit->setText("50");
            textValidationLabel->setText("(Máx. 255 caracteres)");
            textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
        } else {
            textSizeEdit->setPlaceholderText("Tamaño ilimitado por defecto");
            textSizeEdit->setText("Sin límite");
            textValidationLabel->setText("(Texto de longitud ilimitada)");
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-style: italic; }");
        }
    } else if (dataType == "Entero" || dataType == "Decimales") {
        numberPropertiesWidget->show();
        if (dataType == "Entero") {
            numberTypeCombo->setCurrentText("Entero");
            numberSizeLabel->setText("Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)");
        } else {
            numberTypeCombo->setCurrentText("Decimal");
            numberSizeLabel->setText("Tamaño: 64 bits (15-17 dígitos de precisión)");
        }
    } else if (dataType == "moneda") {
        currencyPropertiesWidget->show();
        currencyFormatCombo->setCurrentText("Lempiras (Lps)");
    } else if (dataType == "fecha") {
        datePropertiesWidget->show();
        dateFormatCombo->setCurrentText("DD-MM-YY");
    }
}

void TableView::onTextSizeChanged(const QString &text)
{
    if (text.isEmpty()) return;
    
    // Validar que el valor no sea mayor a 255 para campos de texto
    bool ok;
    int size = text.toInt(&ok);
    
    if (ok && size > 255) {
        // Si el valor es mayor a 255, limitarlo a 255
        textSizeEdit->blockSignals(true);
        textSizeEdit->setText("255");
        textSizeEdit->blockSignals(false);
        
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
    } else if (!ok && !text.contains("Sin límite") && !text.contains("ilimitado")) {
        // Si no es un número válido y no es texto especial, mostrar error
        textValidationLabel->setText("⚠️ Ingrese un número válido");
        textValidationLabel->setStyleSheet("QLabel { color: #ef4444; font-size: 12px; font-weight: bold; }");
        
        textSizeEdit->setStyleSheet(getInputStyle() + 
            "QLineEdit { border: 2px solid #ef4444; background-color: #fef2f2; }");
        
        QTimer::singleShot(3000, [this]() {
            textSizeEdit->setStyleSheet(getInputStyle());
            textValidationLabel->setText("(Máx. 255 caracteres)");
            textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
        });
    } else {
        // Restablecer estilo normal si todo está bien
        textSizeEdit->setStyleSheet(getInputStyle());
        if (ok && size <= 255) {
            textValidationLabel->setText(QString("✅ Tamaño válido: %1 caracteres").arg(size));
            textValidationLabel->setStyleSheet("QLabel { color: #10b981; font-size: 12px; font-weight: bold; }");
            
            // Restaurar texto normal después de 2 segundos
            QTimer::singleShot(2000, [this]() {
                textValidationLabel->setText("(Máx. 255 caracteres)");
                textValidationLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 12px; font-style: italic; }");
            });
        }
        qDebug() << "DEBUG: Tamaño de texto cambiado a:" << text;
    }
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onNumberTypeChanged(const QString &text)
{
    qDebug() << "DEBUG: Tipo de número cambiado a:" << text;
    
    // Mostrar información sobre los tamaños según el tipo seleccionado
    QString sizeInfo = "";
    if (text == "Entero") {
        sizeInfo = "Tamaño: 32 bits (-2,147,483,648 a 2,147,483,647)";
    } else if (text == "Decimal") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión)";
    } else if (text == "Doble") {
        sizeInfo = "Tamaño: 64 bits (15-17 dígitos de precisión, mayor rango)";
    } else if (text == "Byte") {
        sizeInfo = "Tamaño: 8 bits (0 a 255)";
    }
    
    // Actualizar el label de información de tamaño
    if (!sizeInfo.isEmpty() && numberSizeLabel) {
        numberSizeLabel->setText(sizeInfo);
        qDebug() << "DEBUG: Información de tamaño actualizada:" << sizeInfo;
    }
    
    // También actualizar el tooltip del combo
    if (!sizeInfo.isEmpty()) {
        numberTypeCombo->setToolTip(sizeInfo);
    }
    
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onCurrencyFormatChanged(const QString &text)
{
    qDebug() << "DEBUG: Formato de moneda cambiado a:" << text;
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
}

void TableView::onDateFormatChanged(const QString &text)
{
    qDebug() << "DEBUG: Formato de fecha cambiado a:" << text;
    // Emitir señal para actualizar vista de datos
    emit tableDesignChanged(getCurrentFieldNames(), getCurrentFieldTypes());
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
    } else if (dataType == "Texto corto (hasta N caracteres)") {
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
    } else if (dataType == "Texto largo / Párrafo") {
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
        } else if (format == "DD/MESTEXTO/YYYY") {
            return "15/Agosto/2024";
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
