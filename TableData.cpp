#include "TableData.h"
#include "RelationshipsView.h"
#include "TableEditor.h"
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
#include <QApplication>
#include <QScreen>

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
        
        // Obtener el formato de fecha para esta columna
        QString dateFormat = "dd-MM-yyyy"; // formato por defecto
        if (owner && index.column() < owner->getSavedDateFormats().size()) {
            QString savedFormat = owner->getSavedDateFormats().at(index.column());
            if (savedFormat == "DD-MM-YY") {
                dateFormat = "dd-MM-yy";
            } else if (savedFormat == "DD/MM/YY") {
                dateFormat = "dd/MM/yy";
            } else if (savedFormat == "DD/MESTEXTO/YYYY") {
                // Para el editor, usamos formato numérico normal, 
                // la conversión a texto se hace al mostrar
                dateFormat = "dd/MM/yyyy";
            }
        }
        
        dateEdit->setDisplayFormat(dateFormat);
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
        const TableData *owner = qobject_cast<const TableData*>(this->parent());
        
        // Usar el formato específico de la columna
        QString formattedDate;
        if (owner && index.column() < owner->getSavedDateFormats().size()) {
            QString savedFormat = owner->getSavedDateFormats().at(index.column());
            formattedDate = owner->formatDateWithTextMonth(d, savedFormat);
        } else {
            formattedDate = d.toString("dd-MM-yyyy");
        }
        
        model->setData(index, formattedDate, Qt::EditRole);
        if (owner)
            const_cast<TableData*>(owner)->clearCellError(index.row(), index.column());
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

        if (type == "Entero" || type == "Byte") {
            bool ok=false; 
            int intVal = newText.toInt(&ok);
            if (!ok) return softReject("Este campo es Entero.");
            if (type == "Byte" && (intVal < 0 || intVal > 255)) {
                return softReject("Byte debe estar entre 0 y 255.");
            }
        } else if (type == "Decimales" || type == "Decimal" || type == "Doble") {
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
                
                // Aplicar el formato específico de esta columna
                if (owner && index.column() < owner->getSavedDateFormats().size()) {
                    QString savedFormat = owner->getSavedDateFormats().at(index.column());
                    newText = owner->formatDateWithTextMonth(d, savedFormat);
                } else {
                    newText = d.toString("dd-MM-yyyy");
                }
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
    relationshipsView = nullptr; // Inicializar como nullptr
    tableEditor = nullptr; // Inicializar como nullptr
    
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
    
    // Crear controles de filtro
    createFilterControls();
    
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

void TableData::createFilterControls()
{
    // Widget contenedor para filtros
    QWidget *filterWidget = new QWidget();
    filterWidget->setFixedHeight(50);
    filterWidget->setStyleSheet(
        "QWidget {"
        "background-color: #f8f9fa;"
        "border-bottom: 1px solid #dee2e6;"
        "}"
    );
    
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(20, 8, 20, 8);
    filterLayout->setSpacing(15);
    
    // Etiqueta de búsqueda
    QLabel *searchLabel = new QLabel("🔍 Buscar:");
    searchLabel->setFont(QFont("Inter", 12, QFont::Medium));
    searchLabel->setStyleSheet("QLabel { color: #374151; }");
    
    // Campo de búsqueda
    searchField = new QLineEdit();
    searchField->setPlaceholderText("Escribe para filtrar datos...");
    searchField->setMaximumWidth(200);
    searchField->setStyleSheet(
        "QLineEdit {"
        "background-color: white;"
        "border: 1px solid #d1d5db;"
        "border-radius: 4px;"
        "padding: 6px 10px;"
        "font-size: 12px;"
        "}"
        "QLineEdit:focus {"
        "border-color: #3b82f6;"
        "outline: none;"
        "}"
    );
    
    // Filtro numérico
    QLabel *numberLabel = new QLabel("🔢 Números:");
    numberLabel->setFont(QFont("Inter", 12, QFont::Medium));
    numberLabel->setStyleSheet("QLabel { color: #374151; }");
    
    numberCondition = new QComboBox();
    numberCondition->addItems({"Sin filtro", "Mayor que", "Menor que", "Igual a", "Entre"});
    numberCondition->setMaximumWidth(100);
    numberCondition->setStyleSheet(
        "QComboBox {"
        "background-color: white;"
        "border: 1px solid #d1d5db;"
        "border-radius: 4px;"
        "padding: 4px 8px;"
        "font-size: 11px;"
        "}"
    );
    
    numberValue1 = new QLineEdit();
    numberValue1->setPlaceholderText("Valor");
    numberValue1->setMaximumWidth(70);
    numberValue1->setStyleSheet(searchField->styleSheet());
    
    numberValue2 = new QLineEdit();
    numberValue2->setPlaceholderText("Hasta");
    numberValue2->setMaximumWidth(70);
    numberValue2->setStyleSheet(searchField->styleSheet());
    numberValue2->setVisible(false); // Solo visible para "Entre"
    
    // Etiqueta de ordenamiento
    QLabel *sortLabel = new QLabel("📊 Ordenar por:");
    sortLabel->setFont(QFont("Inter", 12, QFont::Medium));
    sortLabel->setStyleSheet("QLabel { color: #374151; }");
    
    // Combo de columnas
    sortColumnCombo = new QComboBox();
    sortColumnCombo->setMaximumWidth(120);
    sortColumnCombo->setStyleSheet(
        "QComboBox {"
        "background-color: white;"
        "border: 1px solid #d1d5db;"
        "border-radius: 4px;"
        "padding: 4px 8px;"
        "font-size: 12px;"
        "}"
    );
    
    // Combo de orden
    sortOrderCombo = new QComboBox();
    sortOrderCombo->addItems({"↑ Ascendente", "↓ Descendente"});
    sortOrderCombo->setMaximumWidth(110);
    sortOrderCombo->setStyleSheet(
        "QComboBox {"
        "background-color: white;"
        "border: 1px solid #d1d5db;"
        "border-radius: 4px;"
        "padding: 4px 8px;"
        "font-size: 12px;"
        "}"
    );
    
    // Botón limpiar filtros
    clearFiltersBtn = new QPushButton("🗑️ Limpiar");
    clearFiltersBtn->setMaximumWidth(80);
    clearFiltersBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #6b7280;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "padding: 6px 12px;"
        "font-size: 12px;"
        "font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "background-color: #4b5563;"
        "}"
        "QPushButton:pressed {"
        "background-color: #374151;"
        "}"
    );
    
    // Agregar widgets al layout
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(searchField);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(numberLabel);
    filterLayout->addWidget(numberCondition);
    filterLayout->addWidget(numberValue1);
    filterLayout->addWidget(numberValue2);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(sortLabel);
    filterLayout->addWidget(sortColumnCombo);
    filterLayout->addWidget(sortOrderCombo);
    filterLayout->addSpacing(10);
    filterLayout->addWidget(clearFiltersBtn);
    filterLayout->addStretch();
    
    // Conectar señales
    connect(searchField, &QLineEdit::textChanged, this, &TableData::applyFilters);
    connect(numberCondition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        numberValue2->setVisible(index == 4); // "Entre" es el índice 4
        applyFilters();
    });
    connect(numberValue1, &QLineEdit::textChanged, this, &TableData::applyFilters);
    connect(numberValue2, &QLineEdit::textChanged, this, &TableData::applyFilters);
    connect(sortColumnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TableData::applyFilters);
    connect(sortOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TableData::applyFilters);
    connect(clearFiltersBtn, &QPushButton::clicked, this, &TableData::clearFilters);
    
    mainLayout->addWidget(filterWidget);
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
    
    // Actualizar combo de ordenamiento con nuevos campos
    if (sortColumnCombo) {
        sortColumnCombo->blockSignals(true);
        sortColumnCombo->clear();
        for (const QString &fieldName : fieldNames) {
            // Limpiar iconos de los nombres de campo para el combo
            QString cleanName = fieldName;
            cleanName = cleanName.remove("🔑🔗🔶")
                                 .remove("🔑🔗")
                                 .remove("🔑🔶")
                                 .remove("🔗🔶")
                                 .remove("🔑")
                                 .remove("🔗")
                                 .remove("🔶")
                                 .trimmed();
            sortColumnCombo->addItem(cleanName);
        }
        sortColumnCombo->blockSignals(false);
    }
    
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

QString TableData::formatDateWithTextMonth(const QDate &date, const QString &format) const
{
    if (!date.isValid()) return "";
    
    // Arreglo con nombres de meses en español
    QStringList monthNames = {
        "", "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio",
        "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"
    };
    
    if (format == "DD/MESTEXTO/YYYY") {
        int day = date.day();
        int month = date.month();
        int year = date.year();
        
        if (month >= 1 && month <= 12) {
            return QString("%1/%2/%3")
                .arg(day, 2, 10, QChar('0'))  // DD con padding de ceros
                .arg(monthNames[month])        // Mes en texto
                .arg(year);                    // YYYY
        }
    }
    
    // Para otros formatos, usar el formato Qt estándar
    if (format == "DD-MM-YY") {
        return date.toString("dd-MM-yy");
    } else if (format == "DD/MM/YY") {
        return date.toString("dd/MM/yy");
    }
    
    return date.toString("dd-MM-yyyy"); // Formato por defecto
}

void TableData::applyDateFormats()
{
    qDebug() << "DEBUG: Aplicando formatos de fecha específicos";
    
    if (savedDateFormats.isEmpty() || savedFieldTypes.isEmpty()) {
        qDebug() << "DEBUG: No hay formatos de fecha o tipos de campo guardados";
        return;
    }
    
    for (int col = 0; col < savedFieldTypes.size() && col < savedDateFormats.size(); ++col) {
        if (savedFieldTypes.at(col) == "fecha") {
            QString format = savedDateFormats.at(col);
            qDebug() << "DEBUG: Aplicando formato de fecha" << format << "a columna" << col;
            
            dataTable->blockSignals(true);
            for (int row = 0; row < dataTable->rowCount(); ++row) {
                QTableWidgetItem *item = dataTable->item(row, col);
                if (!item) continue;
                
                // Saltar fila de ejemplo
                QTableWidgetItem *firstItem = dataTable->item(row, 0);
                if (firstItem && firstItem->toolTip().contains("Ejemplo")) continue;

                const QString text = item->text().trimmed();
                if (!text.isEmpty()) {
                    // Intentar parsear la fecha actual
                    QDate date = QDate::fromString(text, "dd-MM-yyyy");
                    if (!date.isValid()) {
                        date = QDate::fromString(text, "dd/MM/yyyy");
                    }
                    if (!date.isValid()) {
                        date = QDate::fromString(text, "dd-MM-yy");
                    }
                    if (!date.isValid()) {
                        date = QDate::fromString(text, "dd/MM/yy");
                    }
                    
                    if (date.isValid()) {
                        QString formattedDate = formatDateWithTextMonth(date, format);
                        item->setText(formattedDate);
                        qDebug() << "DEBUG: Fecha" << text << "convertida a" << formattedDate;
                    }
                }
            }
            dataTable->blockSignals(false);
        }
    }
    
    // Forzar actualización visual de la tabla
    qDebug() << "DEBUG: Forzando actualización visual de fechas";
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
    qDebug() << "DEBUG: Tamaños guardados en savedTextSizes:" << savedTextSizes;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos de moneda después de la configuración básica
    applyCurrencyFormats();
}

void TableData::setupDataViewWithUniqueFields(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, const QStringList &numberTypes, const QStringList &dateFormats, const QList<int> &uniqueColumns, int primaryKeyColumn)
{
    qDebug() << "DEBUG: setupDataViewWithUniqueFields llamado con:";
    qDebug() << "DEBUG: fieldNames:" << fieldNames;
    qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
    qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
    qDebug() << "DEBUG: millaresDecimals:" << millaresDecimals;
    qDebug() << "DEBUG: textSizes:" << textSizes;
    qDebug() << "DEBUG: numberTypes:" << numberTypes;
    qDebug() << "DEBUG: dateFormats:" << dateFormats;    qDebug() << "DEBUG: uniqueColumns:" << uniqueColumns;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar los formatos de moneda, decimales, tamaños de texto, formatos de fecha y campos únicos
    savedCurrencyFormats = currencyFormats;
    savedMillaresDecimals = millaresDecimals;
    savedTextSizes = textSizes;
    savedDateFormats = dateFormats;
    savedUniqueColumns = uniqueColumns;
    qDebug() << "DEBUG: Decimales guardados en savedMillaresDecimals:" << savedMillaresDecimals;
    qDebug() << "DEBUG: Tamaños guardados en savedTextSizes:" << savedTextSizes;
    qDebug() << "DEBUG: Formatos de fecha guardados en savedDateFormats:" << savedDateFormats;
    qDebug() << "DEBUG: Campos únicos guardados en savedUniqueColumns:" << savedUniqueColumns;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos de moneda después de la configuración básica
    applyCurrencyFormats();
    
    // Aplicar formatos específicos de fecha después de la configuración básica
    applyDateFormats();
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
    
    // *** VALIDACIÓN DE CAMPOS ÚNICOS ***
    if (savedUniqueColumns.contains(col)) {
        QString newValue = item->text().trimmed();
        if (!newValue.isEmpty()) {
            // Buscar si ya existe este valor en otra fila de la misma columna
            for (int r = 0; r < dataTable->rowCount(); r++) {
                if (r == row) continue; // Saltar la fila actual
                
                QTableWidgetItem *otherItem = dataTable->item(r, col);
                if (otherItem && !otherItem->toolTip().contains("Ejemplo")) {
                    QString otherValue = otherItem->text().trimmed();
                    if (otherValue == newValue) {
                        // ¡Valor duplicado encontrado en campo único!
                        QString fieldName = (col < savedFieldNames.size()) ? savedFieldNames.at(col) : QString("Campo %1").arg(col + 1);
                        
                        // Usar QTimer::singleShot para mover el mensaje al main thread
                        QTimer::singleShot(0, this, [this, newValue, fieldName, item]() {
                            QMessageBox msgBox(this);
                            msgBox.setWindowTitle("Campo Único duplicado");
                            msgBox.setIcon(QMessageBox::Warning);
                            msgBox.setText(QString("El valor '%1' ya existe en el campo '%2'.\n"
                                                  "Los campos marcados como Únicos no pueden tener valores repetidos.\n\n"
                                                  "Por favor, ingrese un valor diferente.")
                                                  .arg(newValue, fieldName));
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
                                "background-color: #f59e0b;"
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
                                "background-color: #d97706;"
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
            qDebug() << "DEBUG: Unique field value '" << newValue << "' in column" << col << "is unique - OK";
        }
    }
    
    // *** VALIDACIÓN DE LLAVES FORÁNEAS ***
    if (col < savedFieldNames.size()) {
        QString fieldName = savedFieldNames.at(col);
        QString newValue = item->text().trimmed();
        
        qDebug() << "DEBUG: Validando campo" << fieldName << "con valor" << newValue;
        
        if (isFieldForeignKey(fieldName)) {
            qDebug() << "DEBUG: Campo" << fieldName << "identificado como FK";
            
            if (!newValue.isEmpty()) {
                // Verificar si hay relación establecida
                if (!hasEstablishedRelationship(fieldName)) {
                    qDebug() << "DEBUG: Campo FK" << fieldName << "no tiene relación establecida";
                    
                    // Usar QTimer::singleShot para mostrar mensaje de "sin conexión"
                    QTimer::singleShot(0, this, [this, fieldName, newValue, item]() {
                        QMessageBox msgBox(this);
                        msgBox.setWindowTitle("Campo sin Conexión");
                        msgBox.setIcon(QMessageBox::Warning);
                        msgBox.setText(QString("El campo '%1' está marcado como llave foránea pero no tiene conexión a ninguna tabla.\n\n"
                                              "Para establecer una relación:\n"
                                              "1. Ve a la vista de Relaciones\n"
                                              "2. Crea una conexión entre esta tabla y la tabla referenciada\n\n"
                                              "O elimina la marca de llave foránea si no necesitas validación.")
                                              .arg(fieldName));
                        msgBox.setStandardButtons(QMessageBox::Ok);
                        msgBox.setStyleSheet(
                            "QMessageBox {"
                            "background-color: white;"
                            "min-width: 500px;"
                            "min-height: 250px;"
                            "}"
                            "QMessageBox QLabel {"
                            "color: black;"
                            "font-size: 16px;"
                            "padding: 10px;"
                            "}"
                            "QPushButton {"
                            "background-color: #f59e0b;"
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
                            "background-color: #d97706;"
                            "}"
                        );
                        msgBox.exec();
                    });
                    
                    // Bloquear señales y restaurar valor anterior
                    dataTable->blockSignals(true);
                    item->setText(""); // Limpiar el campo
                    dataTable->blockSignals(false);
                    
                    // Enfocar el campo para facilitar corrección
                    QTimer::singleShot(100, this, [this, item]() {
                        dataTable->setCurrentItem(item);
                        dataTable->editItem(item);
                    });
                    return; // Salir sin procesar más
                }
                
                QString referencedTable = getReferencedTable(fieldName);
                QString referencedField = getReferencedField(fieldName);
                
                qDebug() << "DEBUG: FK referencia" << referencedTable << "." << referencedField;
                
                if (!valueExistsInReferencedTable(referencedTable, referencedField, newValue)) {
                    qDebug() << "DEBUG: Valor" << newValue << "NO existe en" << referencedTable << "." << referencedField;
                    
                    // Obtener valores válidos para mostrar al usuario
                    QStringList validValues = getTableData(referencedTable, referencedField);
                    QString validValuesText = validValues.isEmpty() ? 
                        "No hay datos disponibles en la tabla referenciada." :
                        QString("Valores válidos: %1").arg(validValues.join(", "));
                    
                    // Usar QTimer::singleShot para mover el mensaje al main thread
                    QTimer::singleShot(0, this, [this, newValue, referencedTable, referencedField, fieldName, validValuesText, item]() {
                        QMessageBox msgBox(this);
                        msgBox.setWindowTitle("Error de Llave Foránea");
                        msgBox.setIcon(QMessageBox::Critical);
                        msgBox.setText(QString("El valor '%1' no existe en %2.%3\n\n"
                                              "El campo '%4' es una llave foránea y debe hacer referencia a un valor válido.\n\n"
                                              "%5\n\n"
                                              "Por favor, ingrese un valor que exista en la tabla referenciada.")
                                              .arg(newValue, referencedTable, referencedField, fieldName, validValuesText));
                        msgBox.setStandardButtons(QMessageBox::Ok);
                        msgBox.setStyleSheet(
                            "QMessageBox {"
                            "background-color: white;"
                            "min-width: 450px;"
                            "min-height: 220px;"
                            "}"
                            "QMessageBox QLabel {"
                            "color: black;"
                            "font-size: 16px;"
                            "padding: 10px;"
                            "}"
                            "QPushButton {"
                            "background-color: #dc2626;"
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
                            "background-color: #b91c1c;"
                            "}"
                        );
                        msgBox.exec();
                    });
                    
                    // Bloquear señales y restaurar valor anterior
                    dataTable->blockSignals(true);
                    item->setText(""); // Limpiar el campo
                    dataTable->blockSignals(false);
                    
                    // Enfocar el campo para facilitar corrección
                    QTimer::singleShot(100, this, [this, item]() {
                        dataTable->setCurrentItem(item);
                        dataTable->editItem(item);
                    });
                    return; // Salir sin procesar más
                } else {
                    qDebug() << "DEBUG: Valor" << newValue << "SÍ existe en" << referencedTable << "." << referencedField << "- Validación OK";
                }
            }
        } else {
            qDebug() << "DEBUG: Campo" << fieldName << "NO es FK - sin validación";
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

void TableData::setRelationshipsView(RelationshipsView *relationshipsView)
{
    this->relationshipsView = relationshipsView;
}

void TableData::setTableEditor(TableEditor *tableEditor)
{
    this->tableEditor = tableEditor;
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
    if (dataType == "Entero" || dataType == "Números") {
        return "12345";
    } else if (dataType == "Decimal" || dataType == "Decimales" || dataType == "Doble") {
        return "123.45";
    } else if (dataType == "Byte") {
        return "255";
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
        return QStringLiteral("%1").arg(formattedNumber);
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
    if (!m_warnLabel) {
        // SIN padre: evitamos pasar 'this' (que es const aquí)
        m_warnLabel = new QLabel(nullptr);
        m_warnLabel->setObjectName("softWarn");
        m_warnLabel->setStyleSheet(
            "QLabel#softWarn {"
            "  background: #111827;"
            "  color: white;"
            "  border: 1px solid #374151;"
            "  border-radius: 8px;"
            "  padding: 8px 12px;"
            "  font-family: 'Inter';"
            "  font-size: 13px;"
            "}"
            );
        m_warnLabel->setWindowFlags(Qt::ToolTip);
        m_warnLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    if (!m_warnTimer) {
        // cuelga el timer del label para limpiar juntos
        m_warnTimer = new QTimer(m_warnLabel);
        m_warnTimer->setSingleShot(true);
        QObject::connect(m_warnTimer, &QTimer::timeout, m_warnLabel, &QLabel::hide);
    }

    m_warnLabel->setText(msg);
    m_warnLabel->adjustSize();

    // Posicionar cerca de la celda
    if (dataTable) {
        const QModelIndex ix = dataTable->model()->index(row, col);
        const QRect cellRect = dataTable->visualRect(ix);
        QPoint global = dataTable->viewport()->mapToGlobal(cellRect.bottomRight());
        global += QPoint(-m_warnLabel->width(), 8);

        const QRect screenGeo = QApplication::primaryScreen()->availableGeometry();
        const int x = std::clamp(global.x(), screenGeo.left(),  screenGeo.right()  - m_warnLabel->width());
        const int y = std::clamp(global.y(), screenGeo.top(),   screenGeo.bottom() - m_warnLabel->height());
        m_warnLabel->move(QPoint(x, y));
    } else {
        m_warnLabel->move(QCursor::pos() + QPoint(12, 12));
    }

    m_warnLabel->show();
    m_warnTimer->start(2000); // fijo: 6 s
}

bool TableData::isValueValidForType(const QString& type, const QString& value) const {
    const QString v = value.trimmed();
    if (type == "Entero" || type == "Byte") {
        bool ok=false; int intVal = v.toInt(&ok);
        if (!ok && !v.isEmpty()) return false;
        if (type == "Byte" && !v.isEmpty() && (intVal < 0 || intVal > 255)) return false;
        return ok || v.isEmpty();
    }
    if (type == "Decimales" || type == "Decimal" || type == "Doble" || type == "moneda") {
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

bool TableData::hasColumnDuplicates(int columnIndex) const
{
    if (!dataTable || columnIndex < 0 || columnIndex >= dataTable->columnCount()) {
        return false;
    }
    
    QSet<QString> uniqueValues;
    
    for (int row = 0; row < dataTable->rowCount(); row++) {
        // Ignorar la fila de ejemplo
        QTableWidgetItem *firstItem = dataTable->item(row, 0);
        if (firstItem && firstItem->toolTip().contains("Ejemplo")) {
            continue;
        }
        
        QTableWidgetItem *item = dataTable->item(row, columnIndex);
        if (item) {
            QString value = item->text().trimmed();
            
            // Ignorar valores vacíos
            if (!value.isEmpty()) {
                if (uniqueValues.contains(value)) {
                    return true; // Se encontró un duplicado
                }
                uniqueValues.insert(value);
            }
        }
    }
    
    return false; // No se encontraron duplicados
}

void TableData::setupDataViewWithAllFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, const QStringList &numberTypes, const QStringList &dateFormats, const QList<int> &uniqueColumns, int primaryKeyColumn)
{
    qDebug() << "DEBUG: setupDataViewWithAllFormats llamado con:";
    qDebug() << "DEBUG: fieldNames:" << fieldNames;
    qDebug() << "DEBUG: fieldTypes:" << fieldTypes;
    qDebug() << "DEBUG: currencyFormats:" << currencyFormats;
    qDebug() << "DEBUG: millaresDecimals:" << millaresDecimals;
    qDebug() << "DEBUG: textSizes:" << textSizes;
    qDebug() << "DEBUG: numberTypes:" << numberTypes;
    qDebug() << "DEBUG: uniqueColumns:" << uniqueColumns;
    qDebug() << "DEBUG: Primary Key en columna:" << primaryKeyColumn;
    
    // Guardar todos los formatos incluyendo tipos de números
    savedCurrencyFormats = currencyFormats;
    savedMillaresDecimals = millaresDecimals;
    savedTextSizes = textSizes;
    qDebug() << "DEBUG: Formatos guardados en savedCurrencyFormats:" << savedCurrencyFormats;
    qDebug() << "DEBUG: Decimales guardados en savedMillaresDecimals:" << savedMillaresDecimals;
    qDebug() << "DEBUG: Tamaños guardados en savedTextSizes:" << savedTextSizes;
    qDebug() << "DEBUG: Tipos de números guardados en savedNumberTypes:" << savedNumberTypes;
    qDebug() << "DEBUG: Formatos de fecha guardados en savedDateFormats:" << savedDateFormats;    qDebug() << "DEBUG: Campos únicos guardados en savedUniqueColumns:" << savedUniqueColumns;
    
    // Llamar al método base para hacer la configuración normal
    setupDataView(fieldNames, fieldTypes, primaryKeyColumn);
    
    // Aplicar formatos específicos después de la configuración básica
    applyCurrencyFormats();
    applyNumberFormats(); // Nuevo: aplicar formatos de números
    applyDateFormats(); // Nuevo: aplicar formatos de fechas
}

void TableData::applyNumberFormats()
{
    qDebug() << "DEBUG: Aplicando formatos de números específicos";
    
    if (savedNumberTypes.isEmpty() || savedFieldTypes.isEmpty()) {
        qDebug() << "DEBUG: No hay tipos de números o tipos de campo guardados";
        return;
    }
    
    for (int col = 0; col < savedFieldTypes.size() && col < savedNumberTypes.size(); ++col) {
        if (savedFieldTypes.at(col) == "Entero" || savedFieldTypes.at(col) == "Byte") {
            qDebug() << "DEBUG: Aplicando formato de número a columna" << col;
            
            dataTable->blockSignals(true);
            for (int row = 0; row < dataTable->rowCount(); ++row) {
                QTableWidgetItem *item = dataTable->item(row, col);
                if (!item) continue;
                
                // Saltar fila de ejemplo
                QTableWidgetItem *firstItem = dataTable->item(row, 0);
                if (firstItem && firstItem->toolTip().contains("Ejemplo")) continue;

                const QString text = item->text().trimmed();
                if (!text.isEmpty()) {
                    bool ok = false;
                    int intValue = text.toInt(&ok);
                    if (ok) {
                        // Formatear como número entero
                        item->setText(QString::number(intValue));
                    } else {
                        // Si no es un entero válido, dejar el texto tal cual
                        qDebug() << "DEBUG: Valor no entero, sin formato:" << text;
                    }
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

// ==================== MÉTODOS DE VALIDACIÓN FK ====================

bool TableData::validateForeignKeyConstraints(int row)
{
    if (!relationshipsView || !dataTable) {
        return true; // Si no hay RelationshipsView, no validar FK
    }
    
    for (int col = 0; col < dataTable->columnCount(); ++col) {
        QString fieldName = savedFieldNames.value(col, "");
        if (fieldName.isEmpty()) continue;
        
        // Verificar si es FK
        if (isFieldForeignKey(fieldName)) {
            QTableWidgetItem *item = dataTable->item(row, col);
            QString value = item ? item->text().trimmed() : "";
            
            if (!value.isEmpty()) {
                QString referencedTable = getReferencedTable(fieldName);
                QString referencedField = getReferencedField(fieldName);
                
                if (!valueExistsInReferencedTable(referencedTable, referencedField, value)) {
                    // Mostrar error
                    QMessageBox::warning(this, "Error de Validación", 
                        QString("El valor '%1' no existe en %2.%3\n\nPor favor, ingrese un valor válido.")
                        .arg(value, referencedTable, referencedField));
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool TableData::isFieldForeignKey(const QString &fieldName)
{
    if (!relationshipsView || !tableEditor) return false;
    
    // Método mejorado: verificar si el campo está en la lista de FK de TableEditor
    QStringList foreignKeys = tableEditor->getTableForeignKeys(currentTableName);
    
    // Verificar si el campo está en la lista de foreign keys
    if (foreignKeys.contains(fieldName)) {
        qDebug() << "DEBUG: Campo" << fieldName << "es FK según TableEditor";
        return true;
    }
    
    // Verificación adicional: si el campo termina con "_id" o contiene iconos FK
    bool isFK = fieldName.endsWith("_id") || fieldName.contains("🔗");
    
    qDebug() << "DEBUG: Campo" << fieldName << "- Es FK:" << isFK;
    return isFK;
}

// Nuevo método para verificar si hay relación establecida
bool TableData::hasEstablishedRelationship(const QString &fieldName)
{
    qDebug() << "DEBUG TableData::hasEstablishedRelationship para campo:" << fieldName;
    qDebug() << "DEBUG: relationshipsView es" << (relationshipsView ? "válido" : "NULL");
    qDebug() << "DEBUG: tableEditor es" << (tableEditor ? "válido" : "NULL");
    
    if (!relationshipsView || !tableEditor) {
        qDebug() << "DEBUG: Falta referencia a relationshipsView o tableEditor - retornando false";
        return false;
    }
    
    // Usar el nuevo método de RelationshipsView para verificar relaciones reales
    bool hasRelation = relationshipsView->hasRelationshipForField(currentTableName, fieldName);
    
    qDebug() << "DEBUG: Campo" << fieldName << "en tabla" << currentTableName << "tiene relación establecida:" << hasRelation;
    
    return hasRelation;
}

QString TableData::getReferencedTable(const QString &fieldName)
{
    if (!relationshipsView || !tableEditor) return "";
    
    qDebug() << "DEBUG: getReferencedTable para campo" << fieldName;
    
    // Método mejorado: buscar en las relaciones creadas
    // TODO: Implementar acceso real a las relaciones guardadas en RelationshipsView
    
    // Por ahora, aproximación simple basada en convenciones de nomenclatura
    if (fieldName.endsWith("_id")) {
        QString tableName = fieldName;
        tableName.remove("_id");
        
        qDebug() << "DEBUG: Tabla inferida del campo FK:" << tableName;
        
        // Verificar si la tabla existe
        QStringList availableTables = tableEditor->getCreatedTables();
        qDebug() << "DEBUG: Tablas disponibles:" << availableTables;
        
        // Buscar tabla exacta primero
        for (const QString &table : availableTables) {
            if (table.toLower() == tableName.toLower()) {
                qDebug() << "DEBUG: FK" << fieldName << "referencia tabla exacta" << table;
                return table;
            }
        }
        
        // Si no se encuentra exacta, buscar coincidencias parciales
        for (const QString &table : availableTables) {
            if (table.toLower().contains(tableName.toLower()) ||
                tableName.toLower().contains(table.toLower())) {
                qDebug() << "DEBUG: FK" << fieldName << "referencia tabla parcial" << table;
                return table;
            }
        }
        
        qDebug() << "DEBUG: No se encontró tabla referenciada para FK" << fieldName;
        
        // Si no se encuentra, retornar el nombre inferido con capitalización correcta
        tableName[0] = tableName[0].toLower(); // Primera letra minúscula para coincidir con el ejemplo
        return tableName;
    }
    
    return "";
}

QString TableData::getReferencedField(const QString &fieldName)
{
    if (!tableEditor) return "Id";
    
    // Obtener la tabla referenciada
    QString referencedTable = getReferencedTable(fieldName);
    if (referencedTable.isEmpty()) return "Id";
    
    // Obtener los campos de la tabla referenciada
    QStringList fields = tableEditor->getTableFields(referencedTable);
    
    qDebug() << "DEBUG: getReferencedField - Campos de tabla" << referencedTable << ":" << fields;
    
    // Buscar el campo "Id" con diferentes variaciones de capitalización
    for (const QString &field : fields) {
        QString cleanField = field;
        cleanField = cleanField.replace("🔑", "").replace("🔗", "").trimmed();
        qDebug() << "DEBUG: Comparando campo limpio:" << cleanField;
        
        if (cleanField.toLower() == "id") {
            qDebug() << "DEBUG: Campo referenciado encontrado:" << cleanField;
            return cleanField;
        }
    }
    
    // Si no se encuentra "id", usar el primer campo
    if (!fields.isEmpty()) {
        QString firstField = fields.first();
        firstField = firstField.replace("🔑", "").replace("🔗", "").trimmed();
        qDebug() << "DEBUG: Usando primer campo como referencia:" << firstField;
        return firstField;
    }
    
    // Por defecto retornar "Id" (con mayúscula como en tu ejemplo)
    qDebug() << "DEBUG: Usando valor por defecto: Id";
    return "Id";
}

bool TableData::valueExistsInReferencedTable(const QString &tableName, const QString &fieldName, const QString &value)
{
    if (!relationshipsView) return true;
    
    // Obtener datos de la tabla referenciada
    QStringList tableData = getTableData(tableName, fieldName);
    return tableData.contains(value);
}

QStringList TableData::getTableData(const QString &tableName, const QString &fieldName)
{
    QStringList result;
    
    if (!tableEditor) {
        qDebug() << "DEBUG: TableEditor no disponible";
        return result;
    }
    
    qDebug() << "DEBUG: Buscando datos en tabla" << tableName << "campo" << fieldName;
    
    try {
        // Obtener datos reales de la tabla referenciada a través de TableEditor
        
        // Primer paso: verificar si la tabla existe
        QStringList availableTables = tableEditor->getCreatedTables();
        if (!availableTables.contains(tableName)) {
            qDebug() << "DEBUG: Tabla" << tableName << "no existe en el sistema";
            qDebug() << "DEBUG: Tablas disponibles:" << availableTables;
            return result;
        }
        
        // Segundo paso: obtener los campos de la tabla para verificar que el campo existe
        QStringList tableFields = tableEditor->getTableFields(tableName);
        int fieldIndex = -1;
        
        // Buscar el índice del campo
        for (int i = 0; i < tableFields.size(); ++i) {
            QString field = tableFields[i];
            // Limpiar el campo de iconos y espacios
            field = field.replace("🔑", "").replace("🔗", "").trimmed();
            if (field == fieldName) {
                fieldIndex = i;
                break;
            }
        }
        
        if (fieldIndex == -1) {
            qDebug() << "DEBUG: Campo" << fieldName << "no encontrado en tabla" << tableName;
            qDebug() << "DEBUG: Campos disponibles:" << tableFields;
            return result;
        }
        
        qDebug() << "DEBUG: Campo" << fieldName << "encontrado en índice" << fieldIndex << "de tabla" << tableName;
        
        // Tercer paso: obtener los datos reales desde TableEditor
        result = tableEditor->getTableColumnData(tableName, fieldName);
        
        // Si no hay datos reales, proporcionar algunos datos de ejemplo para testing
        if (result.isEmpty()) {
            qDebug() << "DEBUG: No hay datos reales, usando datos de ejemplo";
            if (tableName.toLower() == "clases") {
                // Generar algunos IDs de ejemplo para la tabla clases
                result << "1" << "2" << "3" << "4" << "5" << "101" << "102" << "103";
                qDebug() << "DEBUG: Datos simulados para tabla clases:" << result;
            } else if (tableName.toLower() == "maestro") {
                result << "1" << "2" << "3" << "4" << "5";
                qDebug() << "DEBUG: Datos simulados para tabla maestro:" << result;
            } else {
                // Para otras tablas, generar IDs básicos
                for (int i = 1; i <= 10; ++i) {
                    result << QString::number(i);
                }
                qDebug() << "DEBUG: Datos genéricos para tabla" << tableName << ":" << result;
            }
        } else {
            qDebug() << "DEBUG: Datos reales obtenidos de la tabla" << tableName << ":" << result;
        }
        
    } catch (...) {
        qDebug() << "DEBUG: Error al acceder a datos de tabla" << tableName;
    }
    
    qDebug() << "DEBUG: Valores encontrados para validación:" << result;
    return result;
}

void TableData::applyFilters()
{
    if (!dataTable) return;
    
    QString searchText = searchField ? searchField->text().trimmed().toLower() : "";
    
    // Obtener configuración de filtro numérico
    QString numCondition = numberCondition ? numberCondition->currentText() : "Sin filtro";
    QString numValue1Text = numberValue1 ? numberValue1->text().trimmed() : "";
    QString numValue2Text = numberValue2 ? numberValue2->text().trimmed() : "";
    
    bool hasNumberFilter = (numCondition != "Sin filtro" && !numValue1Text.isEmpty());
    double numVal1 = 0, numVal2 = 0;
    bool numVal1Ok = false, numVal2Ok = false;
    
    if (hasNumberFilter) {
        numVal1 = numValue1Text.toDouble(&numVal1Ok);
        if (numCondition == "Entre" && !numValue2Text.isEmpty()) {
            numVal2 = numValue2Text.toDouble(&numVal2Ok);
        }
    }
    
    // Aplicar filtros
    for (int row = 0; row < dataTable->rowCount(); ++row) {
        bool shouldShow = true;
        
        // Verificar si es la fila vacía (última fila para nuevos datos)
        bool isEmptyRow = true;
        for (int col = 0; col < dataTable->columnCount(); ++col) {
            QTableWidgetItem *item = dataTable->item(row, col);
            if (item && !item->text().trimmed().isEmpty()) {
                isEmptyRow = false;
                break;
            }
        }
        
        // Siempre mostrar la fila vacía para permitir agregar datos
        if (isEmptyRow && row == dataTable->rowCount() - 1) {
            dataTable->setRowHidden(row, false);
            continue;
        }
        
        // Filtro de texto
        if (!searchText.isEmpty()) {
            shouldShow = false;
            for (int col = 0; col < dataTable->columnCount(); ++col) {
                QTableWidgetItem *item = dataTable->item(row, col);
                if (item && item->text().toLower().contains(searchText)) {
                    shouldShow = true;
                    break;
                }
            }
        }
        
        // Filtro numérico (aplicar solo si pasa el filtro de texto)
        if (shouldShow && hasNumberFilter && numVal1Ok) {
            bool passesNumFilter = false;
            
            // Verificar cada columna para valores numéricos
            for (int col = 0; col < dataTable->columnCount(); ++col) {
                QTableWidgetItem *item = dataTable->item(row, col);
                if (!item || item->text().trimmed().isEmpty()) continue;
                
                bool ok = false;
                double cellValue = item->text().toDouble(&ok);
                if (!ok) continue; // No es un número, saltar esta celda
                
                bool matches = false;
                if (numCondition == "Mayor que") {
                    matches = cellValue > numVal1;
                } else if (numCondition == "Menor que") {
                    matches = cellValue < numVal1;
                } else if (numCondition == "Igual a") {
                    matches = qAbs(cellValue - numVal1) < 0.0001; // Comparación de flotantes
                } else if (numCondition == "Entre" && numVal2Ok) {
                    double minVal = qMin(numVal1, numVal2);
                    double maxVal = qMax(numVal1, numVal2);
                    matches = cellValue >= minVal && cellValue <= maxVal;
                }
                
                if (matches) {
                    passesNumFilter = true;
                    break;
                }
            }
            
            shouldShow = passesNumFilter;
        }
        
        dataTable->setRowHidden(row, !shouldShow);
    }
    
    // Aplicar ordenamiento si está seleccionado
    if (sortColumnCombo && sortColumnCombo->currentIndex() >= 0) {
        int column = sortColumnCombo->currentIndex();
        bool ascending = (sortOrderCombo && sortOrderCombo->currentIndex() == 0); // 0 = Ascendente, 1 = Descendente
        sortDataRowsOnly(column, ascending);
    }
}

void TableData::clearFilters()
{
    // Limpiar campo de búsqueda
    if (searchField) {
        searchField->clear();
    }
    
    // Limpiar filtros numéricos
    if (numberCondition) {
        numberCondition->setCurrentIndex(0); // "Sin filtro"
    }
    if (numberValue1) {
        numberValue1->clear();
    }
    if (numberValue2) {
        numberValue2->clear();
        numberValue2->setVisible(false);
    }
    
    // Resetear combos de ordenamiento
    if (sortColumnCombo) {
        sortColumnCombo->setCurrentIndex(0);
    }
    if (sortOrderCombo) {
        sortOrderCombo->setCurrentIndex(0);
    }
    
    // Mostrar todas las filas
    if (dataTable) {
        for (int row = 0; row < dataTable->rowCount(); ++row) {
            dataTable->setRowHidden(row, false);
        }
        
        // Restaurar orden original
        dataTable->sortItems(0, Qt::AscendingOrder);
    }
}

void TableData::sortByColumn(int column, Qt::SortOrder order)
{
    if (!dataTable || column < 0 || column >= dataTable->columnCount()) return;
    
    // Temporalmente desconectar señales para evitar loops
    dataTable->blockSignals(true);
    
    // Separar filas con datos de filas vacías
    QList<QStringList> rowsWithData;
    QList<QStringList> emptyRows;
    QList<int> rowsWithDataOriginalIndex;
    QList<int> emptyRowsOriginalIndex;
    
    for (int row = 0; row < dataTable->rowCount(); ++row) {
        if (dataTable->isRowHidden(row)) continue; // Saltar filas ocultas
        
        QStringList rowValues;
        bool hasData = false;
        
        for (int col = 0; col < dataTable->columnCount(); ++col) {
            QTableWidgetItem *item = dataTable->item(row, col);
            QString value = item ? item->text().trimmed() : "";
            rowValues << value;
            if (!value.isEmpty()) {
                hasData = true;
            }
        }
        
        if (hasData) {
            rowsWithData.append(rowValues);
            rowsWithDataOriginalIndex.append(row);
        } else {
            emptyRows.append(rowValues);
            emptyRowsOriginalIndex.append(row);
        }
    }
    
    // Ordenar solo las filas con datos
    std::sort(rowsWithData.begin(), rowsWithData.end(), [column, order](const QStringList &a, const QStringList &b) {
        if (column >= a.size() || column >= b.size()) return false;
        
        QString valA = a[column].trimmed();
        QString valB = b[column].trimmed();
        
        // Si uno está vacío y el otro no, el vacío va al final
        if (valA.isEmpty() && !valB.isEmpty()) return order == Qt::DescendingOrder;
        if (!valA.isEmpty() && valB.isEmpty()) return order == Qt::AscendingOrder;
        if (valA.isEmpty() && valB.isEmpty()) return false;
        
        // Intentar ordenamiento numérico si ambos son números
        bool aIsNum, bIsNum;
        double numA = valA.toDouble(&aIsNum);
        double numB = valB.toDouble(&bIsNum);
        
        if (aIsNum && bIsNum) {
            return (order == Qt::AscendingOrder) ? numA < numB : numA > numB;
        }
        
        // Ordenamiento alfabético
        int comparison = valA.compare(valB, Qt::CaseInsensitive);
        return (order == Qt::AscendingOrder) ? comparison < 0 : comparison > 0;
    });
    
    // Aplicar datos ordenados: primero filas con datos, luego filas vacías
    QList<QStringList> allSortedRows = rowsWithData + emptyRows;
    QList<int> allOriginalIndexes = rowsWithDataOriginalIndex + emptyRowsOriginalIndex;
    
    int targetRow = 0;
    for (int i = 0; i < allSortedRows.size(); ++i) {
        // Encontrar siguiente fila visible
        while (targetRow < dataTable->rowCount() && dataTable->isRowHidden(targetRow)) {
            targetRow++;
        }
        
        if (targetRow >= dataTable->rowCount()) break;
        
        const QStringList &rowValues = allSortedRows[i];
        
        // Aplicar valores
        for (int col = 0; col < rowValues.size() && col < dataTable->columnCount(); ++col) {
            QTableWidgetItem *item = dataTable->item(targetRow, col);
            if (item) {
                item->setText(rowValues[col]);
            }
        }
        targetRow++;
    }
    
    // Reconectar señales
    dataTable->blockSignals(false);
}

void TableData::sortDataRowsOnly(int column, bool ascending)
{
    if (!dataTable || column < 0 || column >= dataTable->columnCount()) return;
    
    qDebug() << "DEBUG: Ordenando solo filas con datos - Columna:" << column << "Ascendente:" << ascending;
    
    // Bloquear señales durante el ordenamiento
    dataTable->blockSignals(true);
    
    // Identificar filas con datos (excluyendo fila vacía al final)
    QList<QStringList> dataRows;
    QList<int> originalRowNumbers;
    int totalRows = dataTable->rowCount();
    
    for (int row = 0; row < totalRows; row++) {
        // Verificar si la fila tiene datos
        bool hasData = false;
        QStringList rowData;
        
        for (int col = 0; col < dataTable->columnCount(); col++) {
            QTableWidgetItem *item = dataTable->item(row, col);
            QString cellText = item ? item->text().trimmed() : "";
            rowData << cellText;
            if (!cellText.isEmpty()) {
                hasData = true;
            }
        }
        
        // Solo incluir filas con datos Y que estén visibles
        if (hasData && !dataTable->isRowHidden(row)) {
            dataRows << rowData;
            originalRowNumbers << row;
        }
    }
    
    qDebug() << "DEBUG: Encontradas" << dataRows.size() << "filas con datos para ordenar";
    
    // Ordenar las filas con datos
    std::sort(dataRows.begin(), dataRows.end(), [column, ascending](const QStringList &a, const QStringList &b) {
        if (column >= a.size() || column >= b.size()) return false;
        
        QString aVal = a[column];
        QString bVal = b[column];
        
        // Intentar comparación numérica
        bool aIsNum, bIsNum;
        double aNum = aVal.toDouble(&aIsNum);
        double bNum = bVal.toDouble(&bIsNum);
        
        if (aIsNum && bIsNum) {
            return ascending ? aNum < bNum : aNum > bNum;
        }
        
        // Comparación de texto
        int result = aVal.compare(bVal, Qt::CaseInsensitive);
        return ascending ? result < 0 : result > 0;
    });
    
    // Aplicar el nuevo orden solo a las filas con datos visibles
    int targetRow = 0;
    for (int i = 0; i < dataRows.size(); i++) {
        // Buscar la próxima fila visible
        while (targetRow < totalRows && dataTable->isRowHidden(targetRow)) {
            targetRow++;
        }
        
        if (targetRow >= totalRows) break;
        
        // Verificar si la fila objetivo tiene datos (no es la fila vacía)
        bool targetHasData = false;
        for (int col = 0; col < dataTable->columnCount(); col++) {
            QTableWidgetItem *item = dataTable->item(targetRow, col);
            if (item && !item->text().trimmed().isEmpty()) {
                targetHasData = true;
                break;
            }
        }
        
        // Solo actualizar si la fila objetivo debe tener datos
        if (targetHasData || targetRow < totalRows - 1) {
            const QStringList &rowData = dataRows[i];
            for (int col = 0; col < rowData.size() && col < dataTable->columnCount(); col++) {
                QTableWidgetItem *item = dataTable->item(targetRow, col);
                if (item) {
                    item->setText(rowData[col]);
                }
            }
        }
        targetRow++;
    }
    
    // Reconectar señales
    dataTable->blockSignals(false);
    
    qDebug() << "DEBUG: Ordenamiento de filas con datos completado";
}
