#include "TableData.h"
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpressionValidator>
#include <QDate>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QTimer>

// Implementación del DataFieldDelegate
QWidget *DataFieldDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    const TableData *owner = qobject_cast<const TableData*>(this->parent());
    const QString type = owner ? owner->fieldTypeForColumn(index.column()) : QString();

    if (type == "fecha") {
        auto *dateEdit = new QDateEdit(parent);
        dateEdit->setAttribute(Qt::WA_StyledBackground, true);
        dateEdit->setAutoFillBackground(true);
        dateEdit->setMinimumHeight(48);
        dateEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        dateEdit->setCalendarPopup(true);                 // popup con flecha
        dateEdit->setDisplayFormat("dd-MM-yyyy");         // formato mostrado
        dateEdit->setDate(QDate::currentDate());          // valor inicial
        dateEdit->setMinimumDate(QDate(1900,1,1));
        dateEdit->setMaximumDate(QDate(2100,12,31));

        // Personalizar el calendario embebido (cuando lo abras con la flecha)
        auto *cal = dateEdit->calendarWidget();
        cal->setGridVisible(true);

        // Estilo similar al resto de editores
        dateEdit->setStyleSheet(
            "QDateEdit {"
            "  background-color: #ffffff;"
            "  border: 1px solid #93c5fd;"
            "  border-radius: 6px;"
            "  padding: 6px 10px;"
            "  font-size: 16px;"
            "  color: #111827;"
            "}"
            "QDateEdit:focus {"
            "  border: 1px solid #3b82f6;"
            "}"
            );

        // Guardar cambios al seleccionar fecha
        QObject::connect(dateEdit, &QDateEdit::dateChanged, this, [this, dateEdit](const QDate&){
            auto *that = const_cast<DataFieldDelegate*>(this);
            Q_EMIT that->commitData(dateEdit);
            Q_EMIT that->closeEditor(dateEdit, QAbstractItemDelegate::NoHint);
        });

        return dateEdit;
    }


    if (type == "Sí / No") {
        auto *combo = new QComboBox(parent);
        combo->addItems({QStringLiteral("Sí"), QStringLiteral("No")});
        combo->setAttribute(Qt::WA_StyledBackground, true);
        combo->setAutoFillBackground(true);
        combo->setMinimumHeight(48);
        combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 🔧 FIX: castear this para poder emitir signals desde un método const
        QObject::connect(combo, QOverload<int>::of(&QComboBox::activated),
                         this, [this, combo](int){
                             auto *that = const_cast<DataFieldDelegate*>(this);
                             Q_EMIT that->commitData(combo);
                             Q_EMIT that->closeEditor(combo, QAbstractItemDelegate::NoHint);
                         });

        return combo;
    }

    auto *lineEdit = new QLineEdit(parent);
    lineEdit->setAttribute(Qt::WA_StyledBackground, true);
    lineEdit->setAutoFillBackground(true);
    lineEdit->setMinimumHeight(48);
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    // 🔧 FIX: idem para QLineEdit
    QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit](){
        auto *that = const_cast<DataFieldDelegate*>(this);
        Q_EMIT that->commitData(lineEdit);
        Q_EMIT that->closeEditor(lineEdit, QAbstractItemDelegate::NoHint);
    });

    // Editor opaco, padding amplio y texto centrado vertical
    lineEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  border: 1px solid #93c5fd;"
        "  border-radius: 6px;"
        "  padding: 12px;"                 /* más grande */
        "  font-size: 14px;"               /* más legible */
        "  color: #111827;"
        "  selection-background-color: #3b82f6;"
        "}"
        "QLineEdit:focus {"
        "  background-color: #ffffff;"
        "  border: 1px solid #3b82f6;"
        "}"
        );


    if (type == "Entero") {
        auto *v = new QIntValidator(lineEdit);
        v->setBottom(std::numeric_limits<int>::min());
        v->setTop(std::numeric_limits<int>::max());
        lineEdit->setValidator(v);
    } else if (type == "Decimales" || type == "moneda") {
        auto *v = new QDoubleValidator(lineEdit);
        v->setNotation(QDoubleValidator::StandardNotation);
        if (type == "moneda") v->setDecimals(2);
        lineEdit->setValidator(v);
    } else if (type == "fecha") {
        QRegularExpression rx(R"(^(0?[1-9]|[12][0-9]|3[01])([/\-])(0?[1-9]|1[0-2])\2(\d{2}|\d{4})$)");
        lineEdit->setValidator(new QRegularExpressionValidator(rx, lineEdit));
    } else if (type.startsWith("Texto corto")) {
        lineEdit->setMaxLength(255);
    }
    // Texto largo / Párrafo: sin restricción

    // Guardar valor previo por si hay que revertir
    lineEdit->setProperty("prevText", index.model()->data(index, Qt::EditRole));
    return lineEdit;
    }

void DataFieldDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option,
                                             const QModelIndex &) const
{
    const int margin = 2;
    editor->setGeometry(option.rect.adjusted(margin, margin, -margin, -margin));
}


void DataFieldDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (auto *dateEdit = qobject_cast<QDateEdit*>(editor)) {
        // Acepta dd-mm-aaaa o dd/mm/aaaa (y año de 2 dígitos)
        QString txt = index.model()->data(index, Qt::EditRole).toString().trimmed();
        QDate d;
        if (!txt.isEmpty()) {
            const QChar sep = txt.contains('/') ? '/' : '-';
            const QString fmt = (txt.count(sep)==2 && txt.split(sep).last().size()==4)
                                    ? QString("dd%1MM%1yyyy").arg(sep)
                                    : QString("dd%1MM%1yy").arg(sep);
            d = QDate::fromString(txt, fmt);
        }
        if (!d.isValid()) d = QDate::currentDate();
        dateEdit->setDate(d);
        return;
    }

    if (auto *combo = qobject_cast<QComboBox*>(editor)) {
        auto normalize = [](QString v)->QString {
            v = v.trimmed().toLower();
            if (v=="si" || v=="sí" || v=="true" || v=="1") return "Sí";
            if (v=="no" || v=="false" || v=="0")          return "No";
            return "Sí";
        };
        const QString current = normalize(index.model()->data(index, Qt::EditRole).toString());
        int i = combo->findText(current, Qt::MatchExactly);
        combo->setCurrentIndex(i >= 0 ? i : 0);
        return;
    }

    if (auto *line = qobject_cast<QLineEdit*>(editor)) {
        line->setText(index.model()->data(index, Qt::EditRole).toString());
    }
}

void DataFieldDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    if (auto *dateEdit = qobject_cast<QDateEdit*>(editor)) {
        const QDate d = dateEdit->date();
        model->setData(index, d.toString("dd-MM-yyyy"), Qt::EditRole);
        if (auto *owner = qobject_cast<TableData*>(this->parent()))
            owner->clearCellError(index.row(), index.column());
        return;
    }

    if (auto *combo = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, combo->currentText(), Qt::EditRole);
        if (auto *owner = qobject_cast<TableData*>(this->parent()))
            owner->clearCellError(index.row(), index.column());
        return;
    }

    if (auto *line = qobject_cast<QLineEdit*>(editor)) {
        // tu validación/normalización (moneda, fecha, etc.)
        model->setData(index, line->text().trimmed(), Qt::EditRole);
        if (auto *owner = qobject_cast<TableData*>(this->parent()))
            owner->clearCellError(index.row(), index.column());
    }
}


TableData::TableData(QWidget *parent) : QWidget(parent)
{
    currentTableName = "Nueva Tabla";
    
    // Crear delegate para estilo consistente
    dataFieldDelegate = new DataFieldDelegate(this);
    
    createUI();
    setupTableForPersonData();
}

TableData::~TableData()
{
    // Destructor
}

void TableData::createUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Crear header
    createHeader();
    
    // Crear área de tabla con contenido
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(10);
    
    // Crear tabla de datos
    dataTable = new QTableWidget();
    dataTable->setStyleSheet(getTableStyle());
    
    // Configurar comportamiento de la tabla (igual que TableView)
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Cambiar a filas completas como TableView
    dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
    dataTable->setAlternatingRowColors(true);
    
    // Configurar altura de filas (igual que TableView)
    dataTable->verticalHeader()->setDefaultSectionSize(50); // Filas más altas para mejor visibilidad del texto
    dataTable->verticalHeader()->setMinimumSectionSize(50);
    dataTable->verticalHeader()->show(); // Mostrar números de fila para mejor organización
    
    // Mejorar el comportamiento de edición
    dataTable->setEditTriggers(QAbstractItemView::DoubleClicked | 
                              QAbstractItemView::SelectedClicked | 
                              QAbstractItemView::AnyKeyPressed);
    
    // Conectar señales
    connect(dataTable, &QTableWidget::itemChanged, this, &TableData::onPersonDataChanged);
    
    contentLayout->addWidget(dataTable);
    mainLayout->addWidget(contentWidget);
}

void TableData::createHeader()
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
    
    // Botón Vista Diseño (inactivo)
    designViewBtn = new QPushButton("Vista Diseño");
    designViewBtn->setFixedSize(120, 35);
    designViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e5e7eb, stop:1 #d1d5db);"
        "color: #374151;"
        "border: none;"
        "border-top-left-radius: 6px;"
        "border-bottom-left-radius: 6px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #d1d5db, stop:1 #9ca3af);"
        "}"
    );
    
    // Botón Vista Datos (activo)
    QPushButton *dataViewBtn = new QPushButton("Vista Datos");
    dataViewBtn->setFixedSize(120, 35);
    dataViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3b82f6, stop:1 #2563eb);"
        "color: white;"
        "border: none;"
        "border-top-right-radius: 6px;"
        "border-bottom-right-radius: 6px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2563eb, stop:1 #1d4ed8);"
        "}"
    );
    
    // Agregar botones al contenedor
    buttonLayout->addWidget(designViewBtn);
    buttonLayout->addWidget(dataViewBtn);
    
    // Conectar señales
    connect(designViewBtn, &QPushButton::clicked, this, &TableData::onDesignViewClicked);
    
    headerLayout->addWidget(buttonContainer);
    
    mainLayout->addWidget(headerWidget);
}

void TableData::setupTableForPersonData()
{
    // Configuración para entrada de datos reales - empezar solo con Id
    QStringList defaultFields = {"Id"};
    QStringList defaultTypes = {"Entero"};
    
    setupDataView(defaultFields, defaultTypes);
}

void TableData::setupDataView(const QStringList &fieldNames, const QStringList &fieldTypes)
{
    qDebug() << "DEBUG: Configurando vista de datos con campos:" << fieldNames;
    qDebug() << "DEBUG: Tipos de campos recibidos:" << fieldTypes;
    
    // Verificar que tengamos la misma cantidad de nombres y tipos
    if (fieldNames.size() != fieldTypes.size()) {
        qDebug() << "WARNING: Mismatch between field names (" << fieldNames.size() << ") and field types (" << fieldTypes.size() << ")";
        // Si hay más nombres que tipos, rellenar con "Texto corto (hasta N caracteres)"
        QStringList adjustedTypes = fieldTypes;
        while (adjustedTypes.size() < fieldNames.size()) {
            adjustedTypes << "Texto corto (hasta N caracteres)";
        }
        // Si hay más tipos que nombres, truncar los tipos
        while (adjustedTypes.size() > fieldNames.size()) {
            adjustedTypes.removeLast();
        }
        savedFieldTypes = adjustedTypes;
    } else {
        savedFieldTypes = fieldTypes;
    }
    
    // Guardar datos existentes antes de reconfigurar
    QList<QStringList> existingData;
    QStringList oldFieldNames = savedFieldNames;
    int oldRowCount = dataTable->rowCount();
    int oldColumnCount = dataTable->columnCount();
    
    // Solo guardar datos si ya había columnas configuradas
    if (oldColumnCount > 0 && oldRowCount > 0) {
        for (int row = 0; row < oldRowCount; row++) {
            // Verificar si esta fila es de ejemplo y saltarla
            QTableWidgetItem *firstItem = dataTable->item(row, 0);
            if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
                qDebug() << "DEBUG: Skipping example row" << row << "when saving existing data";
                continue;
            }
            
            QStringList rowData;
            bool hasData = false;
            
            for (int col = 0; col < oldColumnCount; col++) {
                QTableWidgetItem *item = dataTable->item(row, col);
                QString cellText = item ? item->text().trimmed() : "";
                rowData << cellText;
                
                if (!cellText.isEmpty()) {
                    hasData = true;
                }
            }
            
            // Solo guardar filas que tengan datos
            if (hasData) {
                existingData << rowData;
            }
        }
    }
    
    qDebug() << "DEBUG: Datos existentes guardados:" << existingData.size() << "filas";
    
    savedFieldNames = fieldNames;
    
    // Configurar tabla con nuevos campos
    dataTable->setColumnCount(fieldNames.size());
    dataTable->setHorizontalHeaderLabels(fieldNames);
    
    // Configurar el número inicial de filas - solo si hay datos existentes
    int initialRows = existingData.isEmpty() ? 0 : existingData.size(); // Sin filas si no hay datos existentes
    dataTable->setRowCount(initialRows);
    
    // Aplicar delegate a todas las columnas para estilo consistente
    for (int col = 0; col < dataTable->columnCount(); col++) {
        dataTable->setItemDelegateForColumn(col, dataFieldDelegate);
    }
    
    // Configurar anchos de columnas
    configureColumnWidths();
    
    // Configurar altura de filas después de configurar todo (igual que TableView)
    dataTable->verticalHeader()->setDefaultSectionSize(50); // Filas más altas para mejor visibilidad
    dataTable->verticalHeader()->setMinimumSectionSize(50);
    
    // Configurar ancho del header vertical (números de fila)
    dataTable->verticalHeader()->setFixedWidth(50);
    
    // Restaurar datos existentes y crear celdas solo para las filas que tienen datos
    for (int row = 0; row < initialRows; row++) {
        for (int col = 0; col < dataTable->columnCount(); col++) {
            QTableWidgetItem *item = new QTableWidgetItem("");
            
            // Configurar fuente más grande para mejor legibilidad
            QFont itemFont = item->font();
            itemFont.setPointSize(16); // Fuente más grande para consistencia con el editor
            item->setFont(itemFont);
            
            // Todas las celdas son editables desde el inicio
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            item->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco para todas las celdas
            
            // Restaurar datos si existen
            if (row < existingData.size() && !oldFieldNames.isEmpty()) {
                QString newFieldName = fieldNames.at(col);
                
                // Buscar si este campo existía antes
                int oldColIndex = oldFieldNames.indexOf(newFieldName);
                if (oldColIndex >= 0 && oldColIndex < existingData.at(row).size()) {
                    // Restaurar el dato existente
                    item->setText(existingData.at(row).at(oldColIndex));
                    qDebug() << "DEBUG: Restaurando dato en fila" << row << "columna" << col << ":" << item->text();
                }
                // Si es un campo nuevo, el item queda vacío
            }
            
            dataTable->setItem(row, col, item);
        }
    }
    
    // Agregar fila de ejemplo en gris al principio solo si no hay datos existentes
    if (existingData.isEmpty()) {
        updateExampleData();
    }
    
    // Solo agregar una fila vacía para empezar a escribir si no hay datos existentes Y no hay fila de ejemplo
    if (existingData.isEmpty() && dataTable->rowCount() <= 1) {
        addPersonRow();
    }
    
    qDebug() << "DEBUG: Vista de datos configurada exitosamente con" << dataTable->rowCount() << "filas y" << dataTable->columnCount() << "columnas";
}

void TableData::configureColumnWidths()
{
    // Configurar anchos para tabla de datos (no de diseño)
    for (int col = 0; col < dataTable->columnCount(); col++) {
        if (col < savedFieldNames.size()) {
            QString fieldName = savedFieldNames.at(col).toLower();
            
            if (fieldName == "id") {
                dataTable->setColumnWidth(col, 80);
            } else if (fieldName.contains("nombre") || fieldName.contains("name")) {
                dataTable->setColumnWidth(col, 150);
            } else if (fieldName.contains("apellido")) {
                dataTable->setColumnWidth(col, 150);
            } else if (fieldName.contains("email") || fieldName.contains("correo")) {
                dataTable->setColumnWidth(col, 200);
            } else if (fieldName.contains("edad") || fieldName.contains("age")) {
                dataTable->setColumnWidth(col, 80);
            } else if (fieldName.contains("descripción") || fieldName.contains("description")) {
                dataTable->setColumnWidth(col, 250);
            } else {
                dataTable->setColumnWidth(col, 120);
            }
        }
    }
}

void TableData::addPersonRow(const QStringList &personData)
{
    int newRow = dataTable->rowCount();
    dataTable->setRowCount(newRow + 1);
    
    // Crear todas las celdas de la nueva fila - todas editables
    for (int col = 0; col < dataTable->columnCount(); col++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        
        // Configurar fuente más grande para mejor legibilidad
        QFont itemFont = item->font();
        itemFont.setPointSize(16); // Fuente más grande para consistencia con el editor
        item->setFont(itemFont);
        
        // Todas las celdas son editables
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        item->setBackground(QBrush(QColor(255, 255, 255))); // Fondo blanco
        
        dataTable->setItem(newRow, col, item);
    }
}

void TableData::addNewPersonRow()
{
    qDebug() << "DEBUG: Agregando nueva fila para persona";
    addPersonRow();
}

void TableData::removeEmptyRows()
{
    // Eliminar filas completamente vacías desde el final
    for (int row = dataTable->rowCount() - 1; row >= 0; row--) {
        bool isEmpty = true;
        for (int col = 0; col < dataTable->columnCount(); col++) {
            QTableWidgetItem *item = dataTable->item(row, col);
            if (item && !item->text().trimmed().isEmpty()) {
                isEmpty = false;
                break;
            }
        }
        
        if (isEmpty && dataTable->rowCount() > 1) {
            dataTable->removeRow(row);
        }
    }
}

void TableData::onPersonDataChanged(QTableWidgetItem *item)
{
    if (!item) return;
    
    // Ignorar cambios en la fila de ejemplo
    if (item->toolTip().contains("Ejemplo")) {
        qDebug() << "DEBUG: Ignoring changes to example row";
        return;
    }
    
    int row = item->row();
    int col = item->column();
    
    qDebug() << "DEBUG: Datos cambiados en fila:" << row << "columna:" << col;
    
    // Si el usuario empieza a escribir, eliminar la fila de ejemplo
    if (!item->text().trimmed().isEmpty()) {
        // Buscar y eliminar la fila de ejemplo
        for (int r = 0; r < dataTable->rowCount(); r++) {
            QTableWidgetItem *firstItem = dataTable->item(r, 0);
            if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
                qDebug() << "DEBUG: User started typing, removing example row";
                dataTable->blockSignals(true);
                dataTable->removeRow(r);
                dataTable->blockSignals(false);
                break;
            }
        }
    }
    
    // Verificar que los índices son válidos antes de acceder a savedFieldTypes
    if (col >= savedFieldTypes.size() || col >= savedFieldNames.size()) {
        qDebug() << "DEBUG: Column index" << col << "out of range. savedFieldTypes size:" << savedFieldTypes.size() << "savedFieldNames size:" << savedFieldNames.size();
        return;
    }
    
    // Aplicar formato automático para campos de moneda
    if (col < savedFieldTypes.size() && col < savedFieldNames.size() && savedFieldTypes.at(col) == "moneda") {
        QString text = item->text().trimmed();
        if (!text.isEmpty() && !text.startsWith("Lps ") && !text.startsWith("$") && !text.startsWith("€")) {
            // Bloquear señales para evitar bucle infinito
            dataTable->blockSignals(true);
            
            // Extraer solo los números y puntos decimales
            QString cleanNumber = "";
            for (int i = 0; i < text.length(); i++) {
                QChar c = text.at(i);
                if (c.isDigit() || c == '.' || c == ',') {
                    cleanNumber += c;
                }
            }
            
            if (!cleanNumber.isEmpty()) {
                item->setText("Lps " + cleanNumber);
            }
            
            dataTable->blockSignals(false);
        }
    }
    
    // Solo agregar nueva fila si estamos escribiendo en la última fila y hay contenido real
    if (row == dataTable->rowCount() - 1 && !item->text().trimmed().isEmpty()) {
        // Bloquear señales temporalmente para evitar bucles infinitos
        dataTable->blockSignals(true);
        addPersonRow();
        dataTable->blockSignals(false);
    }
}

void TableData::onDesignViewClicked()
{
    qDebug() << "DEBUG: Cambiando a Vista Diseño";
    emit switchToDesignView();
}

void TableData::setTableName(const QString &tableName)
{
    currentTableName = tableName;
    if (tableNameLabel) {
        tableNameLabel->setText(tableName);
    }
}

QList<QStringList> TableData::getAllPersonData() const
{
    QList<QStringList> allData;
    
    for (int row = 0; row < dataTable->rowCount(); row++) {
        // Ignorar la fila de ejemplo
        QTableWidgetItem *firstItem = dataTable->item(row, 0);
        if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
            continue;
        }
        
        QStringList rowData;
        bool hasData = false;
        
        for (int col = 0; col < dataTable->columnCount(); col++) {
            QTableWidgetItem *item = dataTable->item(row, col);
            QString cellText = item ? item->text().trimmed() : "";
            rowData << cellText;
            
            if (!cellText.isEmpty()) {
                hasData = true;
            }
        }
        
        // Solo agregar filas que tengan al menos un dato
        if (hasData) {
            allData << rowData;
        }
    }
    
    return allData;
}

void TableData::clearAllData()
{
    dataTable->clearContents();
    dataTable->setRowCount(0); // Empezar sin filas
    
    // Mostrar fila de ejemplo cuando no hay datos
    updateExampleData();
    
    // Agregar una fila vacía para empezar a escribir
    addPersonRow();
}

QString TableData::getTableStyle()
{
    return "QTableWidget {"
           "background-color: white;"
           "gridline-color: #e2e8f0;"
           "border: 1px solid #cbd5e1;"
           "selection-background-color: #dbeafe;"
           "font-size: 14px;"
           "}"
           "QTableWidget::item {"
           "padding: 16px 12px;" // Más padding para mejor visibilidad del texto (igual que el editor)
           "border-bottom: 1px solid #f1f5f9;"
           "min-height: 50px;" // Altura mínima para las celdas
           "font-size: 16px;" // Fuente más grande para mejor legibilidad
           "}"
           "QTableWidget::item:selected {"
           "background-color: #bfdbfe;" // Mismo color que TableView
           "color: #1e40af;"
           "}"
           "QTableWidget::item:focus {"
           "background-color: rgba(59, 130, 246, 0.1);" // Fondo muy sutil al hacer foco
           "border: 1px solid #3b82f6;" // Borde sutil para indicar foco
           "outline: none;"
           "}"
           "QHeaderView::section {"
           "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f1f5f9, stop:1 #e2e8f0);"
           "color: #374151;"
           "padding: 8px;" // Mismo padding que TableView
           "border: 1px solid #cbd5e1;"
           "font-weight: bold;"
           "font-size: 14px;"
           "}"
           "QHeaderView::section:vertical {"
           "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f1f5f9, stop:1 #e2e8f0);"
           "color: #374151;"
           "padding: 4px 8px;"
           "border: 1px solid #cbd5e1;"
           "font-weight: bold;"
           "font-size: 12px;"
           "min-width: 40px;"
           "}";
}

QString TableData::generateExampleData(const QString &dataType, int column)
{
    Q_UNUSED(column) // Por ahora no usamos la columna, pero puede ser útil en el futuro
    
    if (dataType == "Entero") {
        return "12345";
    } else if (dataType == "Decimales") {
        return "123.45";
    } else if (dataType == "Sí / No") {
        return "Sí";
    } else if (dataType == "Texto corto (hasta N caracteres)") {
        return "Texto de ejemplo";
    } else if (dataType == "Texto largo / Párrafo") {
        return "Este es un ejemplo de texto largo...";
    } else if (dataType == "moneda") {
        return "Lps 1,500.00";
    } else if (dataType == "fecha") {
        return "15-08-24";
    }
    
    return "Ejemplo";
}

void TableData::updateExampleData()
{
    // Verificar que la tabla existe
    if (!dataTable) {
        qDebug() << "DEBUG: DataTable is null, cannot update example data";
        return;
    }
    
    // Solo agregar ejemplo si hay campos definidos
    if (savedFieldNames.isEmpty() || savedFieldTypes.isEmpty()) {
        qDebug() << "DEBUG: No field names or types defined, skipping example data";
        return;
    }
    
    // Bloquear señales para evitar bucles infinitos
    dataTable->blockSignals(true);
    
    // Eliminar TODAS las filas de ejemplo existentes
    for (int row = dataTable->rowCount() - 1; row >= 0; row--) {
        QTableWidgetItem *firstItem = dataTable->item(row, 0);
        if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
            qDebug() << "DEBUG: Removing existing example row at index" << row;
            dataTable->removeRow(row);
        }
    }
    
    // Crear UNA nueva fila de ejemplo al principio (índice 0)
    dataTable->insertRow(0);
    
    // Crear items para la fila de ejemplo
    for (int col = 0; col < savedFieldNames.size(); col++) {
        QString fieldName = savedFieldNames.at(col);
        QString dataType = (col < savedFieldTypes.size()) ? savedFieldTypes.at(col) : "Texto corto (hasta N caracteres)";
        
        QTableWidgetItem *exampleItem = new QTableWidgetItem("");
        
        // Configurar estilo para datos de ejemplo
        QFont exampleFont = exampleItem->font();
        exampleFont.setPointSize(16);
        exampleFont.setItalic(true); // Cursiva para indicar que es ejemplo
        exampleItem->setFont(exampleFont);
        
        // Color gris para indicar que es ejemplo
        exampleItem->setForeground(QBrush(QColor(156, 163, 175))); // Color gris
        exampleItem->setBackground(QBrush(QColor(249, 250, 251))); // Fondo gris muy claro
        
        // NO EDITABLE - solo para mostrar ejemplo
        exampleItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        
        // Tooltip para identificar que es ejemplo
        exampleItem->setToolTip("Fila de ejemplo - muestra cómo se verán los datos");
        
        // Generar el ejemplo específico para este tipo de dato
        QString example = generateExampleData(dataType, col);
        exampleItem->setText(example);
        
        dataTable->setItem(0, col, exampleItem);
    }
    
    // Desbloquear señales
    dataTable->blockSignals(false);
    
    qDebug() << "DEBUG: Created single new example row at index 0";
}

// #include "TableData.moc"
QString TableData::fieldTypeForColumn(int col) const {
    if (col >= 0 && col < savedFieldTypes.size())
        return savedFieldTypes.at(col);
    return QStringLiteral("Texto corto (hasta N caracteres)");
}

void TableData::markCellInvalid(int row, int col, const QString& msg) const {
    if (!dataTable) return;
    if (auto *it = dataTable->item(row, col)) {
        it->setBackground(QBrush(QColor("#FEE2E2"))); // rojo suave
        it->setToolTip(msg);
    }
}

void TableData::clearCellError(int row, int col) const {
    if (!dataTable) return;
    if (auto *it = dataTable->item(row, col)) {
        it->setBackground(QBrush(QColor(255,255,255)));
        it->setToolTip({});
    }
}
