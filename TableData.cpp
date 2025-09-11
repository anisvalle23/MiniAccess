#include "TableData.h"
#include <QMessageBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QDate>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QTimer>
#include <QToolTip>

// Implementación del DataFieldDelegate
QWidget *DataFieldDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem & /*option*/,
                                         const QModelIndex &index) const
{
    // Tipo de la columna
    const TableData *owner = qobject_cast<const TableData*>(this->parent());
    const QString type = owner ? owner->fieldTypeForColumn(index.column()) : QString();

    // --- FECHA: QDateEdit con popup de calendario ---
    if (type == "fecha") {
        auto *dateEdit = new QDateEdit(parent);
        dateEdit->setAttribute(Qt::WA_StyledBackground, true);
        dateEdit->setAutoFillBackground(true);
        dateEdit->setMinimumHeight(48);
        dateEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("dd-MM-yyyy");
        dateEdit->setDate(QDate::currentDate());
        dateEdit->setMinimumDate(QDate(1900,1,1));
        dateEdit->setMaximumDate(QDate(2100,12,31));

        if (auto *cal = dateEdit->calendarWidget())
            cal->setGridVisible(true);

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

        // Guardar y cerrar al cambiar fecha
        QObject::connect(dateEdit, &QDateEdit::dateChanged, this, [this, dateEdit](const QDate&){
            auto *that = const_cast<DataFieldDelegate*>(this);
            Q_EMIT that->commitData(dateEdit);
            Q_EMIT that->closeEditor(dateEdit, QAbstractItemDelegate::NoHint);
        });

        // Guardar valor previo por si hay que revertir
        dateEdit->setProperty("prevText", index.model()->data(index, Qt::EditRole));
        return dateEdit;
    }

    // --- SÍ / NO: QComboBox ---
    if (type == "Sí / No") {
        auto *combo = new QComboBox(parent);
        combo->setEditable(false);
        combo->addItems({QStringLiteral("Sí"), QStringLiteral("No")});
        combo->setAttribute(Qt::WA_StyledBackground, true);
        combo->setAutoFillBackground(true);
        combo->setMinimumHeight(48);
        combo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        combo->setStyleSheet(
            "QComboBox {"
            "  background-color: #ffffff;"
            "  border: 1px solid #93c5fd;"
            "  border-radius: 6px;"
            "  padding: 6px 10px;"
            "  font-size: 16px;"
            "  color: #111827;"
            "}"
            "QComboBox:focus {"
            "  border: 1px solid #3b82f6;"
            "}"
            "QComboBox QAbstractItemView {"
            "  font-size: 16px;"
            "}"
            );

        // Commit & close al seleccionar
        QObject::connect(combo, QOverload<int>::of(&QComboBox::activated),
                         this, [this, combo](int){
                             auto *that = const_cast<DataFieldDelegate*>(this);
                             Q_EMIT that->commitData(combo);
                             Q_EMIT that->closeEditor(combo, QAbstractItemDelegate::NoHint);
                         });

        // Guardar valor previo
        combo->setProperty("prevText", index.model()->data(index, Qt::EditRole));
        return combo;
    }

    // --- RESTO DE TIPOS: QLineEdit con validación suave (no bloquea) ---
    auto *lineEdit = new QLineEdit(parent);
    lineEdit->setAttribute(Qt::WA_StyledBackground, true);
    lineEdit->setAutoFillBackground(true);
    lineEdit->setMinimumHeight(48);
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    lineEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  border: 1px solid #93c5fd;"
        "  border-radius: 6px;"
        "  padding: 12px;"
        "  font-size: 14px;"
        "  color: #111827;"
        "  selection-background-color: #3b82f6;"
        "}"
        "QLineEdit:focus {"
        "  background-color: #ffffff;"
        "  border: 1px solid #3b82f6;"
        "}"
        );

    // Commit & close al terminar edición
    QObject::connect(lineEdit, &QLineEdit::editingFinished, this, [this, lineEdit](){
        auto *that = const_cast<DataFieldDelegate*>(this);
        Q_EMIT that->commitData(lineEdit);
        Q_EMIT that->closeEditor(lineEdit, QAbstractItemDelegate::NoHint);
    });

    // --- VALIDACIÓN SUAVE EN VIVO (NO BLOQUEA) ---
    const int row = index.row();
    const int col = index.column();
    QObject::connect(lineEdit, &QLineEdit::textChanged, this,
                     [this, lineEdit, row, col, type=type](const QString& txt)
                     {
                         auto *tbl = qobject_cast<TableData*>(this->parent());
                         if (!tbl) return;

                         const bool ok = tbl->isValueValidForType(type, txt);
                         if (!ok) {
                             tbl->showSoftWarning(row, col, QString("Valor incompatible para '%1'").arg(type));
                         } else {
                             tbl->clearCellError(row, col);
                             QToolTip::hideText(); // opcional: oculta tooltip si corrige
                         }
                     });

    // Guardar valor previo por si hay que revertir en setModelData
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
            const QChar sep = txt.contains('/') ? QChar('/') : (txt.contains('-') ? QChar('-') : QChar());
            if (sep.isNull()) {
                d = QDate::currentDate();
            } else {
                const QString fmt = (txt.count(sep)==2 && txt.split(sep).last().size()==4)
                ? QString("dd%1MM%1yyyy").arg(sep)
                : QString("dd%1MM%1yy").arg(sep);
                d = QDate::fromString(txt, fmt);
            }
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
        QString currentText = index.model()->data(index, Qt::EditRole).toString();
        
        // Si es un campo de moneda, extraer solo el número para edición
        const TableData *owner = qobject_cast<const TableData*>(this->parent());
        const QString type = owner ? owner->fieldTypeForColumn(index.column()) : QString();
        
        if (type == "moneda" && !currentText.isEmpty()) {
            // Extraer solo el número del texto formateado (quitar prefijos como "Lps ", "$", etc.)
            QString cleanText = currentText;
            cleanText.remove(QRegularExpression("^(Lps|\\$|€)\\s*"));  // Quitar prefijos
            cleanText.remove(QRegularExpression("[,\\s]"));  // Quitar comas y espacios
            qDebug() << "DEBUG: Texto original:" << currentText << "-> Texto limpio para editar:" << cleanText;
            line->setText(cleanText);
        } else {
            line->setText(currentText);
        }
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
        QString newText = line->text().trimmed();
        TableData *owner = qobject_cast<TableData*>(this->parent());
        const QString type = owner ? owner->fieldTypeForColumn(index.column()) : QString();
        const QString oldText = line->property("prevText").toString();

        auto softReject = [&](const QString& msg){
            if (owner) {
                owner->showSoftWarning(index.row(), index.column(), msg);
                model->setData(index, oldText, Qt::EditRole);  // revertir
            }
        };

        if (type == "Entero") {
            bool ok=false; newText.toInt(&ok);
            if (!ok) return softReject("Este campo es Entero.");
        } else if (type == "Decimales") {
            bool ok=false; newText.toDouble(&ok);
            if (!ok) return softReject("Este campo es Decimal (ej. 12.34).");
        } else if (type == "moneda") {
            if (newText.isEmpty()) {
                model->setData(index, "", Qt::EditRole);
                if (owner) owner->clearCellError(index.row(), index.column());
                return;
            }
            bool ok=false; newText.toDouble(&ok);
            if (!ok) return softReject("Moneda inválida. Ingresa un número.");
            // >>> formateo visual aquí <<<
            if (owner) {
                QString format = owner->getCurrencyFormatForColumn(index.column());
                QString decimals = owner->getMillaresDecimalsForColumn(index.column());
                newText = owner->formatCurrencyWithFormatAndDecimals(newText, format, decimals);
            }
        } else if (type == "fecha") {
            if (!owner->isValueValidForType(type, newText)) {
                return softReject("Fecha inválida. Usa dd-MM-aaaa o dd/MM/aaaa.");
            } else {
                const QChar sep = newText.contains('/') ? QChar('/') : (newText.contains('-') ? QChar('-') : QChar());
                const QString fmt = (newText.count(sep)==2 && newText.split(sep).last().size()==4)
                                        ? QString("dd%1MM%1yyyy").arg(sep)
                                        : QString("dd%1MM%1yy").arg(sep);
                const QDate d = QDate::fromString(newText, fmt);
                newText = d.toString("dd-MM-yyyy");
            }
        } else if (type == "Texto corto" || type == "Texto largo") {
            // Validación de tamaño de texto
            if (owner) {
                QString maxSizeStr = owner->getTextSizeForColumn(index.column());
                if (!maxSizeStr.isEmpty() && maxSizeStr != "Sin límite" && maxSizeStr != "ilimitado") {
                    bool ok = false;
                    int maxSize = maxSizeStr.toInt(&ok);
                    if (ok && newText.length() > maxSize) {
                        return softReject(QString("Texto demasiado largo. Máximo %1 caracteres permitidos.").arg(maxSize));
                    }
                }
            }
        }
        // Texto: sin extra

        model->setData(index, newText, Qt::EditRole);
        if (owner) owner->clearCellError(index.row(), index.column());
        return;
    }

}


TableData::TableData(QWidget *parent) : QWidget(parent)
{
    currentTableName = "Nueva Tabla";
    primaryKeyColumnIndex = -1; // No Primary Key por defecto
    
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
    
    // Contenedor para los botones de vista
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
    
    // Botón Vista Diseño (inactivo)
    designViewBtn = new QPushButton("🎨 Vista Diseño");
    designViewBtn->setFixedSize(135, 38);
    designViewBtn->setCursor(Qt::PointingHandCursor);
    designViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f3f4f6, stop:1 #e5e7eb);"
        "color: #374151;"
        "border: 1px solid #d1d5db;"
        "border-top-left-radius: 8px;"
        "border-bottom-left-radius: 8px;"
        "border-top-right-radius: 0px;"
        "border-bottom-right-radius: 0px;"
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
    
    // Botón Vista Datos (activo)
    QPushButton *dataViewBtn = new QPushButton("📊 Vista Datos");
    dataViewBtn->setFixedSize(135, 38);
    dataViewBtn->setCursor(Qt::PointingHandCursor);
    dataViewBtn->setStyleSheet(
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3b82f6, stop:1 #2563eb);"
        "color: white;"
        "border: 1px solid #1d4ed8;"
        "border-top-left-radius: 0px;"
        "border-bottom-left-radius: 0px;"
        "border-top-right-radius: 8px;"
        "border-bottom-right-radius: 8px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2563eb, stop:1 #1d4ed8);"
        "box-shadow: 0 2px 4px rgba(59, 130, 246, 0.3);"
        "}"
    );
    
    // Agregar botones al contenedor
    buttonLayout->addWidget(designViewBtn);
    buttonLayout->addWidget(dataViewBtn);
    
    // Conectar señales para ambos botones
    connect(designViewBtn, &QPushButton::clicked, this, &TableData::onDesignViewClicked);
    
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
    connect(addRowBtn, &QPushButton::clicked, this, &TableData::addNewRow);
    connect(deleteRowBtn, &QPushButton::clicked, this, &TableData::deleteSelectedRow);
    
    headerLayout->addWidget(rowButtonContainer);
    
    mainLayout->addWidget(headerWidget);
}

void TableData::setupTableForPersonData()
{
    // Configuración para entrada de datos reales - empezar solo con Id
    QStringList defaultFields = {"Id"};
    QStringList defaultTypes = {"Entero"};
    
    setupDataView(defaultFields, defaultTypes);
}

void TableData::setupDataView(const QStringList &fieldNames, const QStringList &fieldTypes, int primaryKeyColumn)
{
    qDebug() << "DEBUG: Configurando vista de datos con campos:" << fieldNames;
    qDebug() << "DEBUG: Tipos de campos recibidos:" << fieldTypes;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar el índice de Primary Key
    primaryKeyColumnIndex = primaryKeyColumn;
    
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
    
    // Reformatea celdas existentes de columnas moneda
    for (int col = 0; col < savedFieldTypes.size(); ++col) {
        if (savedFieldTypes.at(col) == "moneda") {
            dataTable->blockSignals(true);
            for (int row = 0; row < dataTable->rowCount(); ++row) {
                QTableWidgetItem *it = dataTable->item(row, col);
                if (!it) continue;
                // saltar fila de ejemplo
                QTableWidgetItem *first = dataTable->item(row, 0);
                if (first && first->toolTip().contains("Ejemplo")) continue;

                const QString t = it->text().trimmed();
                if (!t.isEmpty()) {
                    QString format = getCurrencyFormatForColumn(col);
                    QString decimals = getMillaresDecimalsForColumn(col);
                    it->setText(formatCurrencyWithFormatAndDecimals(t, format, decimals));
                }
            }
            dataTable->blockSignals(false);
        }
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

void TableData::setupDataViewWithFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, int primaryKeyColumn)
{
    qDebug() << "DEBUG: setupDataViewWithFormats llamado con:";
    qDebug() << "DEBUG: fieldNames:" << fieldNames;
    qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
    qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar los formatos de moneda
    savedCurrencyFormats = currencyFormats;
    qDebug() << "DEBUG: Formatos guardados en savedCurrencyFormats:" << savedCurrencyFormats;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos de moneda después de la configuración básica
    applyCurrencyFormats();
}

void TableData::applyCurrencyFormats()
{
    qDebug() << "DEBUG: Aplicando formatos de moneda específicos";
    
    if (savedCurrencyFormats.isEmpty() || savedFieldTypes.isEmpty()) {
        qDebug() << "DEBUG: No hay formatos de moneda o tipos de campo guardados";
        return;
    }
    
    for (int col = 0; col < savedFieldTypes.size() && col < savedCurrencyFormats.size(); ++col) {
        if (savedFieldTypes.at(col) == "moneda") {
            QString format = savedCurrencyFormats.at(col);
            qDebug() << "DEBUG: Aplicando formato de moneda" << format << "a columna" << col;
            
            dataTable->blockSignals(true);
            for (int row = 0; row < dataTable->rowCount(); ++row) {
                QTableWidgetItem *item = dataTable->item(row, col);
                if (!item) continue;
                
                // Saltar fila de ejemplo
                QTableWidgetItem *firstItem = dataTable->item(row, 0);
                if (firstItem && firstItem->toolTip().contains("Ejemplo")) continue;

                const QString text = item->text().trimmed();
                if (!text.isEmpty()) {
                    // Aplicar formato específico según la selección
                    QString decimals = savedMillaresDecimals.size() > col ? savedMillaresDecimals.at(col) : "2";
                    QString formattedValue = formatCurrencyWithFormatAndDecimals(text, format, decimals);
                    item->setText(formattedValue);
                }
            }
            dataTable->blockSignals(false);
        }
    }
    
    // Forzar actualización visual de la tabla
    qDebug() << "DEBUG: Forzando actualización visual de la tabla";
    dataTable->viewport()->update();
    dataTable->repaint();
}

void TableData::setupDataViewWithFormatsAndDecimals(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, int primaryKeyColumn)
{
    qDebug() << "DEBUG: setupDataViewWithFormatsAndDecimals llamado con:";
    qDebug() << "DEBUG: fieldNames:" << fieldNames;
    qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
    qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
    qDebug() << "DEBUG: millaresDecimals:" << millaresDecimals;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar los formatos de moneda y decimales
    savedCurrencyFormats = currencyFormats;
    savedMillaresDecimals = millaresDecimals;
    qDebug() << "DEBUG: Formatos guardados en savedCurrencyFormats:" << savedCurrencyFormats;
    qDebug() << "DEBUG: Decimales guardados en savedMillaresDecimals:" << savedMillaresDecimals;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos de moneda después de la configuración básica
    applyCurrencyFormats();
}

void TableData::setupDataViewWithTextSizes(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, int primaryKeyColumn)
{
    qDebug() << "DEBUG: setupDataViewWithTextSizes llamado con:";
    qDebug() << "DEBUG: fieldNames:" << fieldNames;
    qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
    qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
    qDebug() << "DEBUG: millaresDecimals:" << millaresDecimals;
    qDebug() << "DEBUG: textSizes:" << textSizes;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar los formatos de moneda, decimales y tamaños de texto
    savedCurrencyFormats = currencyFormats;
    savedMillaresDecimals = millaresDecimals;
    savedTextSizes = textSizes;
    qDebug() << "DEBUG: Formatos guardados en savedCurrencyFormats:" << savedCurrencyFormats;
    qDebug() << "DEBUG: Decimales guardados en savedMillaresDecimals:" << savedMillaresDecimals;
    qDebug() << "DEBUG: Tamaños guardados en savedTextSizes:" << savedTextSizes;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos de moneda después de la configuración básica
    applyCurrencyFormats();
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
    
    // *** VALIDACIÓN DE PRIMARY KEY ÚNICO ***
    if (primaryKeyColumnIndex >= 0 && col == primaryKeyColumnIndex) {
        QString newValue = item->text().trimmed();
        if (!newValue.isEmpty()) {
            // Buscar si ya existe este valor en otra fila de la misma columna
            for (int r = 0; r < dataTable->rowCount(); r++) {
                if (r == row) continue; // Saltar la fila actual
                
                QTableWidgetItem *otherItem = dataTable->item(r, col);
                if (otherItem && !otherItem->toolTip().contains("Ejemplo")) {
                    QString otherValue = otherItem->text().trimmed();
                    if (otherValue == newValue) {
                        // ¡Valor duplicado encontrado!
                        // Usar QTimer::singleShot para mover el mensaje al main thread
                        QTimer::singleShot(0, this, [this, newValue, item]() {
                            QMessageBox msgBox(this);
                            msgBox.setWindowTitle("Primary Key duplicado");
                            msgBox.setIcon(QMessageBox::Warning);
                            msgBox.setText(QString("El valor '%1' ya existe en el campo Primary Key.\n"
                                                  "Los campos Primary Key deben ser únicos y no se pueden repetir.\n\n"
                                                  "Por favor, ingrese un valor diferente.")
                                                  .arg(newValue));
                            msgBox.setStandardButtons(QMessageBox::Ok);
                            msgBox.setStyleSheet(
                                "QMessageBox {"
                                "background-color: white;"
                                "min-width: 400px;"
                                "min-height: 200px;"
                                "}"
                                "QMessageBox QLabel {"
                                "color: black;"
                                "font-size: 16px;"
                                "padding: 10px;"
                                "}"
                                "QPushButton {"
                                "background-color: #ef4444;"
                                "color: white;"
                                "font-size: 16px;"
                                "font-weight: bold;"
                                "min-width: 120px;"
                                "min-height: 44px;"
                                "border: none;"
                                "padding: 10px 16px;"
                                "border-radius: 6px;"
                                "}"
                                "QPushButton:hover {"
                                "background-color: #dc2626;"
                                "}"
                            );
                            msgBox.exec();
                        });
                        
                        // Bloquear señales y restaurar valor anterior
                        dataTable->blockSignals(true);
                        item->setText(""); // Limpiar el campo
                        dataTable->blockSignals(false);
                        
                        // Enfocar el campo para facilitar corrección usando QTimer también
                        QTimer::singleShot(100, this, [this, item]() {
                            dataTable->setCurrentItem(item);
                            dataTable->editItem(item);
                        });
                        return; // Salir sin procesar más
                    }
                }
            }
            qDebug() << "DEBUG: Primary Key value '" << newValue << "' is unique - OK";
        }
    }
    
    // Aplicar formato automático para campos de moneda con formato dinámico
    if (col < savedFieldTypes.size() && col < savedFieldNames.size() && savedFieldTypes.at(col) == "moneda") {
        QString text = item->text().trimmed();
        if (!text.isEmpty()) {
            // Verificar si el texto ya tiene formato de moneda
            bool alreadyFormatted = text.startsWith("Lps ") || 
                                   text.startsWith("$") || 
                                   text.startsWith("€") ||
                                   text.contains("Lps") ||
                                   text.contains("$") ||
                                   text.contains("€");
            
            qDebug() << "DEBUG: Texto a verificar:" << text << "- Ya formateado:" << alreadyFormatted;
            
            // Obtener el formato correspondiente para esta columna
            QString format = "Lempiras (Lps)"; // Formato por defecto
            if (col < savedCurrencyFormats.size() && !savedCurrencyFormats.at(col).isEmpty()) {
                format = savedCurrencyFormats.at(col);
                qDebug() << "DEBUG: Usando formato guardado:" << format;
            } else {
                qDebug() << "DEBUG: Usando formato por defecto:" << format;
            }
            
            // Si ya está formateado, verificar si está en el formato correcto
            if (alreadyFormatted) {
                bool correctFormat = false;
                if (format.contains("Lempiras") || format.contains("Lps")) {
                    correctFormat = text.startsWith("Lps ") || text.contains("Lps");
                } else if (format.contains("Dollar") || format.contains("$")) {
                    correctFormat = text.startsWith("$");
                } else if (format.contains("Euros") || format.contains("€")) {
                    correctFormat = text.startsWith("€");
                }
                
                qDebug() << "DEBUG: Formato correcto aplicado:" << correctFormat;
                
                // Si no está en el formato correcto, reformatear
                if (!correctFormat) {
                    // Extraer el número y reformatear
                    QString cleanNumber = text;
                    cleanNumber.remove(QRegularExpression("^(Lps|\\$|€)\\s*"));
                    cleanNumber.remove(QRegularExpression("[,\\s]"));
                    
                    qDebug() << "DEBUG: Reformateando de" << text << "a formato" << format << "con número limpio:" << cleanNumber;
                    
                    dataTable->blockSignals(true);
                    QString decimals = savedMillaresDecimals.size() > col ? savedMillaresDecimals.at(col) : "2";
                    QString formattedText = formatCurrencyWithFormatAndDecimals(cleanNumber, format, decimals);
                    item->setText(formattedText);
                    dataTable->blockSignals(false);
                    
                    qDebug() << "DEBUG: Texto reformateado:" << formattedText;
                    
                    // Forzar actualización visual inmediata
                    dataTable->viewport()->update();
                }
            } else {
                // Texto sin formato - aplicar formato por primera vez
                qDebug() << "DEBUG: Aplicando formato por primera vez a:" << text;
                
                dataTable->blockSignals(true);
                QString decimals = savedMillaresDecimals.size() > col ? savedMillaresDecimals.at(col) : "2";
                QString formattedText = formatCurrencyWithFormatAndDecimals(text, format, decimals);
                qDebug() << "DEBUG: Texto original:" << text << "-> Texto formateado:" << formattedText;
                if (!formattedText.isEmpty()) {
                    item->setText(formattedText);
                }
                dataTable->blockSignals(false);
                
                // Forzar actualización visual inmediata
                dataTable->viewport()->update();
            }
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
        // Usar el formato correspondiente para esta columna si está disponible
        QString format = "Lempiras (Lps)"; // Formato por defecto
        if (column < savedCurrencyFormats.size() && !savedCurrencyFormats.at(column).isEmpty()) {
            format = savedCurrencyFormats.at(column);
        }
        
        // Generar ejemplo con el formato correcto
        if (format.contains("Lempiras") || format.contains("Lps")) {
            return "Lps 1,500.00";
        } else if (format.contains("Dollar") || format.contains("$")) {
            return "$1,500.00";
        } else if (format.contains("Euros") || format.contains("€")) {
            return "€1,500.00";
        } else if (format.contains("Millares")) {
            return "1,500";
        } else {
            return "Lps 1,500.00"; // Formato por defecto
        }
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

QString TableData::formatCurrency(const QString& raw) const {
    // Extrae dígitos, separadores y signo para poder parsear
    QString cleaned;
    cleaned.reserve(raw.size());
    for (QChar c : raw) {
        if (c.isDigit() || c == '.' || c == ',' || c == '-') cleaned.append(c);
    }
    if (cleaned.isEmpty()) return QString();

    // Normaliza decimal a punto para parseo
    QString normalized = cleaned;
    normalized.replace(',', '.');

    bool ok = false;
    const double v = normalized.toDouble(&ok);
    if (!ok) return raw; // Si no se pudo parsear, deja el texto tal cual

    // Formatea con separadores y 2 decimales
    QLocale loc(QLocale::Spanish, QLocale::Honduras); // "1,234.56" → en ES-HN se ve "1,234.56" o según configuración
    return QStringLiteral("Lps %1").arg(loc.toString(v, 'f', 2));
}

QString TableData::formatCurrencyWithFormat(const QString& raw, const QString& format) const {
    // Extrae dígitos, separadores y signo para poder parsear
    QString cleaned;
    cleaned.reserve(raw.size());
    for (QChar c : raw) {
        if (c.isDigit() || c == '.' || c == ',' || c == '-') cleaned.append(c);
    }
    if (cleaned.isEmpty()) return QString();

    // Normaliza decimal a punto para parseo
    QString normalized = cleaned;
    normalized.replace(',', '.');

    bool ok = false;
    const double v = normalized.toDouble(&ok);
    if (!ok) return raw; // Si no se pudo parsear, deja el texto tal cual

    // Formatea según el formato especificado
    QLocale loc(QLocale::Spanish, QLocale::Honduras);
    QString formattedNumber = loc.toString(v, 'f', 2);
    
    if (format.contains("Lempiras") || format.contains("Lps")) {
        return QStringLiteral("Lps %1").arg(formattedNumber);
    } else if (format.contains("Dollar") || format.contains("$")) {
        return QStringLiteral("$%1").arg(formattedNumber);
    } else if (format.contains("Euros") || format.contains("€")) {
        return QStringLiteral("€%1").arg(formattedNumber);
    } else if (format.contains("Millares")) {
        // Solo mostrar la parte entera con separadores de miles
        int integerPart = static_cast<int>(v);
        QString integerFormatted = loc.toString(integerPart);
        return integerFormatted;
    } else {
        // Formato por defecto (Lempiras)
        return QStringLiteral("Lps %1").arg(formattedNumber);
    }
}

QString TableData::formatCurrencyWithFormatAndDecimals(const QString& raw, const QString& format, const QString& decimals) const {
    // Extrae dígitos, separadores y signo para poder parsear
    QString cleaned;
    cleaned.reserve(raw.size());
    for (QChar c : raw) {
        if (c.isDigit() || c == '.' || c == ',' || c == '-') cleaned.append(c);
    }
    if (cleaned.isEmpty()) return QString();

    // Normaliza decimal a punto para parseo
    QString normalized = cleaned;
    normalized.replace(',', '.');

    bool ok = false;
    const double v = normalized.toDouble(&ok);
    if (!ok) return raw; // Si no se pudo parsear, deja el texto tal cual

    // Formatea según el formato especificado
    QLocale loc(QLocale::Spanish, QLocale::Honduras);
    
    // Usar el número de decimales especificado para todos los formatos
    bool decOk = false;
    int numDecimals = decimals.toInt(&decOk);
    if (!decOk || numDecimals < 0 || numDecimals > 6) {
        numDecimals = 2; // Valor por defecto
    }
    
    if (format.contains("Lempiras") || format.contains("Lps")) {
        QString formattedNumber = loc.toString(v, 'f', numDecimals);
        return QStringLiteral("Lps %1").arg(formattedNumber);
    } else if (format.contains("Dollar") || format.contains("$")) {
        QString formattedNumber = loc.toString(v, 'f', numDecimals);
        return QStringLiteral("$%1").arg(formattedNumber);
    } else if (format.contains("Euros") || format.contains("€")) {
        QString formattedNumber = loc.toString(v, 'f', numDecimals);
        return QStringLiteral("€%1").arg(formattedNumber);
    } else if (format.contains("Millares")) {
        QString formattedNumber = loc.toString(v, 'f', numDecimals);
        return formattedNumber;
    } else {
        // Formato por defecto (Lempiras)
        QString formattedNumber = loc.toString(v, 'f', numDecimals);
        return QStringLiteral("Lps %1").arg(formattedNumber);
    }
}

QString TableData::getCurrencyFormatForColumn(int column) const {
    qDebug() << "DEBUG: getCurrencyFormatForColumn llamado para columna:" << column;
    qDebug() << "DEBUG: savedCurrencyFormats disponibles:" << savedCurrencyFormats;
    
    // Verificar que la columna existe en los formatos guardados
    if (column >= 0 && column < savedCurrencyFormats.size()) {
        QString format = savedCurrencyFormats.at(column);
        if (!format.isEmpty()) {
            qDebug() << "DEBUG: Formato encontrado para columna" << column << ":" << format;
            return format;
        }
    }
    
    // Valor por defecto
    qDebug() << "DEBUG: Usando formato por defecto para columna:" << column;
    return "Lempiras (Lps)";
}

QString TableData::getMillaresDecimalsForColumn(int column) const {
    qDebug() << "DEBUG: getMillaresDecimalsForColumn llamado para columna:" << column;
    qDebug() << "DEBUG: savedMillaresDecimals disponibles:" << savedMillaresDecimals;
    
    // Verificar que la columna existe en los decimales guardados
    if (column >= 0 && column < savedMillaresDecimals.size()) {
        QString decimals = savedMillaresDecimals.at(column);
        if (!decimals.isEmpty()) {
            qDebug() << "DEBUG: Decimales encontrados para columna" << column << ":" << decimals;
            return decimals;
        }
    }
    
    // Valor por defecto
    qDebug() << "DEBUG: Usando decimales por defecto para columna:" << column;
    return "2";
}

QString TableData::getTextSizeForColumn(int column) const {
    qDebug() << "DEBUG: getTextSizeForColumn llamado para columna:" << column;
    qDebug() << "DEBUG: savedTextSizes disponibles:" << savedTextSizes;
    
    // Verificar que la columna existe en los tamaños guardados
    if (column >= 0 && column < savedTextSizes.size()) {
        QString textSize = savedTextSizes.at(column);
        if (!textSize.isEmpty()) {
            qDebug() << "DEBUG: Tamaño de texto encontrado para columna" << column << ":" << textSize;
            return textSize;
        }
    }
    
    // Valor por defecto según el tipo de campo
    if (column >= 0 && column < savedFieldTypes.size()) {
        QString fieldType = savedFieldTypes.at(column);
        if (fieldType == "Texto corto") {
            qDebug() << "DEBUG: Usando tamaño por defecto para Texto corto en columna:" << column;
            return "255";
        } else if (fieldType == "Texto largo") {
            qDebug() << "DEBUG: Usando tamaño por defecto para Texto largo en columna:" << column;
            return "Sin límite";
        }
    }
    
    // Valor por defecto general
    qDebug() << "DEBUG: Usando tamaño por defecto general para columna:" << column;
    return "255";
}

void TableData::showSoftWarning(int row, int col, const QString& msg) const {
    if (!dataTable) return;
    // marcar rojo suave
    markCellInvalid(row, col, msg);

    // calcular posición de la celda y mostrar tooltip no modal
    const QModelIndex idx = dataTable->model()->index(row, col);
    QRect vr = dataTable->visualRect(idx);
    QPoint pos = dataTable->viewport()->mapToGlobal(vr.center());
    QToolTip::showText(pos, msg, dataTable);
}

bool TableData::isValueValidForType(const QString& type, const QString& value) const {
    const QString v = value.trimmed();
    if (type == "Entero") {
        bool ok=false; v.toInt(&ok); return ok || v.isEmpty();
    }
    if (type == "Decimales" || type == "moneda") {
        bool ok=false; v.toDouble(&ok); return ok || v.isEmpty();
    }
    if (type == "fecha") {
        if (v.isEmpty()) return true;
        const QChar sep = v.contains('/') ? QChar('/') : (v.contains('-') ? QChar('-') : QChar());
        if (sep.isNull()) return false;
        const QString fmt = (v.count(sep)==2 && v.split(sep).last().size()==4)
                                ? QString("dd%1MM%1yyyy").arg(sep)
                                : QString("dd%1MM%1yy").arg(sep);
        return QDate::fromString(v, fmt).isValid();
    }
    return true;
}

void DataFieldDelegate::initStyleOption(QStyleOptionViewItem *option,
                                        const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);

    const TableData *owner = qobject_cast<const TableData*>(this->parent());
    const QString type = owner ? owner->fieldTypeForColumn(index.column()).trimmed().toLower() : QString();
    if (type != "moneda")
        return;

    // Texto crudo que devolvería el modelo para pintar
    QString raw = index.model()->data(index, Qt::DisplayRole).toString().trimmed();
    if (raw.isEmpty())
        return;

    // Obtener el formato específico para esta columna
    QString format = owner ? owner->getCurrencyFormatForColumn(index.column()) : "Lempiras (Lps)";
    qDebug() << "DEBUG: initStyleOption - Formato para columna" << index.column() << ":" << format;

    // Si ya viene formateado correctamente, no hacer nada
    if ((format.contains("Dollar") && raw.startsWith("$")) ||
        (format.contains("Euros") && raw.startsWith("€")) ||
        (format.contains("Lempiras") && raw.startsWith("Lps"))) {
        option->text = raw;
        qDebug() << "DEBUG: initStyleOption - Ya formateado correctamente:" << raw;
        return;
    }

    // Aplicar formato usando el método de TableData
    if (owner) {
        QString decimals = owner->getMillaresDecimalsForColumn(index.column());
        QString formattedText = owner->formatCurrencyWithFormatAndDecimals(raw, format, decimals);
        option->text = formattedText;
        qDebug() << "DEBUG: initStyleOption - Texto formateado:" << raw << "->" << formattedText;
    } else {
        option->text = raw;
    }
}

void TableData::addNewRow()
{
    if (!dataTable) return;
    
    int newRowIndex = dataTable->rowCount();
    dataTable->insertRow(newRowIndex);
    
    // Configurar la nueva fila
    for (int col = 0; col < dataTable->columnCount(); ++col) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        dataTable->setItem(newRowIndex, col, item);
    }
    
    // Seleccionar la nueva fila y enfocar la primera celda
    dataTable->setCurrentCell(newRowIndex, 0);
    dataTable->edit(dataTable->currentIndex());
    
    qDebug() << "DEBUG: Nueva fila agregada en posición" << newRowIndex;
}

void TableData::deleteSelectedRow()
{
    if (!dataTable) return;

    int currentRow = dataTable->currentRow();

    if (currentRow < 0) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Información");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Por favor selecciona una fila para eliminar.");
        msgBox.setStandardButtons(QMessageBox::Ok);

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
            "background-color: #2563eb;"   /* azul */
            "color: white;"
            "font-size: 16px;"
            "font-weight: bold;"
            "min-width: 120px;"
            "min-height: 44px;"
            "border: none;"
            "padding: 10px 16px;"
            "border-radius: 6px;"
            "}"
            "QPushButton:hover {"
            "background-color: #1e40af;"   /* azul oscuro */
            "}"
            "QPushButton:pressed {"
            "background-color: #1d4ed8;"
            "}"
            );

        msgBox.exec();
        return;
    }

    // Verificar si es una fila de ejemplo
    QTableWidgetItem *firstItem = dataTable->item(currentRow, 0);
    if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No se puede eliminar");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("No se puede eliminar la fila de ejemplo.");
        msgBox.setInformativeText("Esta fila muestra cómo se verán los datos y no se puede eliminar.");
        msgBox.setStandardButtons(QMessageBox::Ok);

        msgBox.setStyleSheet(
            "QMessageBox {"
            "background-color: white;"
            "min-width: 420px;"
            "min-height: 200px;"
            "}"
            "QMessageBox QLabel {"
            "color: #111827;"
            "font-size: 16px;"
            "}"
            "QPushButton {"
            "background-color: #f59e0b;"   /* naranja */
            "color: #111827;"
            "font-size: 16px;"
            "font-weight: 600;"
            "min-width: 120px;"
            "min-height: 44px;"
            "border: 2px solid #111827;"
            "padding: 10px 16px;"
            "border-radius: 6px;"
            "}"
            "QPushButton:hover {"
            "background-color: #d97706;"   /* naranja oscuro */
            "}"
            );

        msgBox.exec();
        return;
    }

    // Verificar si la fila tiene datos
    bool hasData = false;
    for (int col = 0; col < dataTable->columnCount(); ++col) {
        QTableWidgetItem *item = dataTable->item(currentRow, col);
        if (item && !item->text().trimmed().isEmpty()) {
            hasData = true;
            break;
        }
    }

    // Mensaje
    const QString message = hasData
                                ? "¿Estás seguro de que deseas eliminar esta fila con datos? Esta acción no se puede deshacer."
                                : "¿Estás seguro de que deseas eliminar esta fila vacía?";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirmar eliminación");
    msgBox.setIcon(hasData ? QMessageBox::Question : QMessageBox::Information);
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // Estilo general del cuadro
    msgBox.setStyleSheet(
        "QMessageBox {"
        "background-color: white;"
        "min-width: 520px;"
        "min-height: 260px;"
        "}"
        "QMessageBox QLabel {"
        "color: #111827;"
        "font-size: 18px;"
        "font-weight: 600;"
        "}"
        /* Estilo base por si no asignamos estilos por-botón */
        "QPushButton {"
        "font-size: 18px;"
        "font-weight: bold;"
        "min-width: 150px;"
        "min-height: 56px;"
        "border: 3px solid #111827;"
        "padding: 14px 18px;"
        "margin: 10px;"
        "border-radius: 8px;"
        "background-color: #e5e7eb;" /* gris claro */
        "color: #111827;"
        "}"
        "QPushButton:hover {"
        "background-color: #d1d5db;"
        "}"
        );

    // Obtener y personalizar botones (texto y color sólido)
    QAbstractButton *yesBtn = msgBox.button(QMessageBox::Yes);
    QAbstractButton *noBtn  = msgBox.button(QMessageBox::No);

    if (yesBtn) {
        yesBtn->setText("Sí, Eliminar");
        yesBtn->setStyleSheet(
            "QPushButton {"
            "background-color: #dc2626;"  /* rojo */
            "color: white;"
            "font-size: 18px;"
            "font-weight: bold;"
            "min-width: 150px;"
            "min-height: 56px;"
            "border: 3px solid #111827;"
            "padding: 14px 18px;"
            "margin: 10px;"
            "border-radius: 8px;"
            "}"
            "QPushButton:hover {"
            "background-color: #b91c1c;"  /* rojo oscuro */
            "}"
            "QPushButton:pressed {"
            "background-color: #991b1b;"
            "}"
            );
    }

    if (noBtn) {
        noBtn->setText("Cancelar");
        noBtn->setStyleSheet(
            "QPushButton {"
            "background-color: #16a34a;"  /* verde */
            "color: white;"
            "font-size: 18px;"
            "font-weight: bold;"
            "min-width: 150px;"
            "min-height: 56px;"
            "border: 3px solid #111827;"
            "padding: 14px 18px;"
            "margin: 10px;"
            "border-radius: 8px;"
            "}"
            "QPushButton:hover {"
            "background-color: #15803d;"  /* verde oscuro */
            "}"
            "QPushButton:pressed {"
            "background-color: #166534;"
            "}"
            );
    }

    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }

    dataTable->removeRow(currentRow);

    // Seleccionar la siguiente o la anterior
    if (dataTable->rowCount() > 0) {
        int newRow = (currentRow < dataTable->rowCount()) ? currentRow : currentRow - 1;
        if (newRow >= 0) {
            dataTable->setCurrentCell(newRow, 0);
        }
    }

    qDebug() << "DEBUG: Fila eliminada en posición" << currentRow;
    emit personDataChanged();
}

void TableData::updateTheme(bool isDark)
{
    // Reaplica los estilos del header si existe
    if (headerWidget && tableNameLabel) {
        headerWidget->setStyleSheet(
            "QWidget {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop:0 #f8fafc, stop:1 #e2e8f0);"
            "border-bottom: 2px solid #cbd5e1;"
            "}"
        );
        
        tableNameLabel->setStyleSheet(
            "QLabel { "
                "color: #1e293b; "
                "padding: 2px 0px; "
                "min-width: 150px; "
                "font-family: 'Inter', 'SF Pro Display', 'Helvetica Neue', Arial, sans-serif;"
                "font-size: 18px;"
                "font-weight: bold;"
            "}"
        );
        
        // Fuerza una actualización del layout
        headerWidget->update();
    }
}
