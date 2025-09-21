#include "FormulariosView.h"
#include "mainwindow.h"
#include "ThemeTokens.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QJsonParseError>
#include <QFile>
#include <QDate>

FormulariosView::FormulariosView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
    , currentRecordIndex(-1)
    , isEditMode(false)
{
    setupUI();
    updateTheme();
}

FormulariosView::~FormulariosView()
{
    // Cleanup será manejado automáticamente por Qt
}

void FormulariosView::setupUI()
{
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    createHeaderSection();
    createToolbar();
    createTableSelector();
    
    // Crear splitter principal
    mainSplitter = new QSplitter(Qt::Horizontal);
    
    createFormArea();
    createDataViewer();
    
    // Agregar widgets al splitter
    mainSplitter->addWidget(formAreaWidget);
    mainSplitter->addWidget(dataViewerWidget);
    mainSplitter->setSizes({400, 600}); // Proporción inicial
    
    // Agregar widgets al layout principal
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(navigationWidget);
    mainLayout->addWidget(tableSelectorWidget);
    mainLayout->addWidget(mainSplitter, 1); // El splitter toma el espacio restante
}

void FormulariosView::createHeaderSection()
{
    headerWidget = new QWidget();
    headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    
    // Título principal
    titleLabel = new QLabel("Formularios de Datos");
    titleLabel->setFont(QFont("Inter", 28, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignLeft);
    
    // Descripción
    descriptionLabel = new QLabel("Crea y edita registros usando formularios generados automáticamente");
    descriptionLabel->setFont(QFont("Inter", 14, QFont::Normal));
    descriptionLabel->setAlignment(Qt::AlignLeft);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
}

void FormulariosView::createToolbar()
{
    // Main toolbar with CRUD operations
    toolbarWidget = new QWidget();
    toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(12);
    
    // CRUD buttons
    newRecordBtn = new QPushButton("Nuevo");
    newRecordBtn->setFixedHeight(40);
    newRecordBtn->setFont(QFont("Inter", 12, QFont::Medium));
    newRecordBtn->setCursor(Qt::PointingHandCursor);
    
    editRecordBtn = new QPushButton("Editar");
    editRecordBtn->setFixedHeight(40);
    editRecordBtn->setFont(QFont("Inter", 12, QFont::Medium));
    editRecordBtn->setCursor(Qt::PointingHandCursor);
    editRecordBtn->setEnabled(false);
    
    deleteRecordBtn = new QPushButton("Eliminar");
    deleteRecordBtn->setFixedHeight(40);
    deleteRecordBtn->setFont(QFont("Inter", 12, QFont::Medium));
    deleteRecordBtn->setCursor(Qt::PointingHandCursor);
    deleteRecordBtn->setEnabled(false);
    
    saveRecordBtn = new QPushButton("Guardar");
    saveRecordBtn->setFixedHeight(40);
    saveRecordBtn->setFont(QFont("Inter", 12, QFont::Medium));
    saveRecordBtn->setCursor(Qt::PointingHandCursor);
    saveRecordBtn->setEnabled(false);
    
    clearFormBtn = new QPushButton("Limpiar");
    clearFormBtn->setFixedHeight(40);
    clearFormBtn->setFont(QFont("Inter", 12, QFont::Normal));
    clearFormBtn->setCursor(Qt::PointingHandCursor);
    clearFormBtn->setEnabled(false);
    
    refreshDataBtn = new QPushButton("Actualizar");
    refreshDataBtn->setFixedHeight(40);
    refreshDataBtn->setFont(QFont("Inter", 12, QFont::Normal));
    refreshDataBtn->setCursor(Qt::PointingHandCursor);
    
    // Add buttons to toolbar
    toolbarLayout->addWidget(newRecordBtn);
    toolbarLayout->addWidget(editRecordBtn);
    toolbarLayout->addWidget(deleteRecordBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(saveRecordBtn);
    toolbarLayout->addWidget(clearFormBtn);
    toolbarLayout->addWidget(refreshDataBtn);
    
    // Connect signals
    connect(newRecordBtn, &QPushButton::clicked, this, &FormulariosView::onNewRecordClicked);
    connect(editRecordBtn, &QPushButton::clicked, this, &FormulariosView::onEditRecordClicked);
    connect(deleteRecordBtn, &QPushButton::clicked, this, &FormulariosView::onDeleteRecordClicked);
    connect(saveRecordBtn, &QPushButton::clicked, this, &FormulariosView::onSaveRecordClicked);
    connect(clearFormBtn, &QPushButton::clicked, this, &FormulariosView::onClearFormClicked);
    connect(refreshDataBtn, &QPushButton::clicked, this, &FormulariosView::onRefreshDataClicked);
    
    // Navigation toolbar
    navigationWidget = new QWidget();
    navigationLayout = new QHBoxLayout(navigationWidget);
    navigationLayout->setContentsMargins(0, 0, 0, 0);
    navigationLayout->setSpacing(8);
    
    firstRecordBtn = new QPushButton("<<");
    firstRecordBtn->setFixedSize(40, 35);
    firstRecordBtn->setFont(QFont("Inter", 11, QFont::Bold));
    firstRecordBtn->setCursor(Qt::PointingHandCursor);
    firstRecordBtn->setEnabled(false);
    
    previousRecordBtn = new QPushButton("<");
    previousRecordBtn->setFixedSize(40, 35);
    previousRecordBtn->setFont(QFont("Inter", 11, QFont::Bold));
    previousRecordBtn->setCursor(Qt::PointingHandCursor);
    previousRecordBtn->setEnabled(false);
    
    nextRecordBtn = new QPushButton(">");
    nextRecordBtn->setFixedSize(40, 35);
    nextRecordBtn->setFont(QFont("Inter", 11, QFont::Bold));
    nextRecordBtn->setCursor(Qt::PointingHandCursor);
    nextRecordBtn->setEnabled(false);
    
    lastRecordBtn = new QPushButton(">>");
    lastRecordBtn->setFixedSize(40, 35);
    lastRecordBtn->setFont(QFont("Inter", 11, QFont::Bold));
    lastRecordBtn->setCursor(Qt::PointingHandCursor);
    lastRecordBtn->setEnabled(false);
    
    currentRecordLabel = new QLabel("Registro: 0 de 0");
    currentRecordLabel->setFont(QFont("Inter", 11, QFont::Normal));
    currentRecordLabel->setAlignment(Qt::AlignCenter);
    currentRecordLabel->setMinimumWidth(120);
    
    recordCountLabel = new QLabel("Total: 0 registros");
    recordCountLabel->setFont(QFont("Inter", 11, QFont::Normal));
    
    navigationLayout->addWidget(firstRecordBtn);
    navigationLayout->addWidget(previousRecordBtn);
    navigationLayout->addWidget(currentRecordLabel);
    navigationLayout->addWidget(nextRecordBtn);
    navigationLayout->addWidget(lastRecordBtn);
    navigationLayout->addStretch();
    navigationLayout->addWidget(recordCountLabel);
    
    // Connect navigation signals
    connect(firstRecordBtn, &QPushButton::clicked, this, &FormulariosView::onFirstRecordClicked);
    connect(previousRecordBtn, &QPushButton::clicked, this, &FormulariosView::onPreviousRecordClicked);
    connect(nextRecordBtn, &QPushButton::clicked, this, &FormulariosView::onNextRecordClicked);
    connect(lastRecordBtn, &QPushButton::clicked, this, &FormulariosView::onLastRecordClicked);
}

void FormulariosView::createTableSelector()
{
    tableSelectorWidget = new QWidget();
    tableSelectorLayout = new QHBoxLayout(tableSelectorWidget);
    tableSelectorLayout->setContentsMargins(0, 0, 0, 0);
    tableSelectorLayout->setSpacing(15);
    
    tableSelectorLabel = new QLabel("Seleccionar Tabla:");
    tableSelectorLabel->setFont(QFont("Inter", 14, QFont::Medium));
    
    tableComboBox = new QComboBox();
    tableComboBox->setFixedHeight(40);
    tableComboBox->setFont(QFont("Inter", 12, QFont::Normal));
    tableComboBox->setMinimumWidth(250);
    
    tableSelectorLayout->addWidget(tableSelectorLabel);
    tableSelectorLayout->addWidget(tableComboBox);
    tableSelectorLayout->addStretch(); // Empuja todo hacia la izquierda
    
    // Conectar señal
    connect(tableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FormulariosView::onTableSelected);
}

void FormulariosView::createFormArea()
{
    // Área del formulario (lado izquierdo)
    formAreaWidget = new QWidget();
    formAreaWidget->setMinimumWidth(350);
    formAreaWidget->setMaximumWidth(500);
    formAreaLayout = new QVBoxLayout(formAreaWidget);
    formAreaLayout->setContentsMargins(0, 0, 0, 0);
    formAreaLayout->setSpacing(15);
    
    formTitleLabel = new QLabel("Formulario de Entrada");
    formTitleLabel->setFont(QFont("Inter", 16, QFont::Bold));
    
    // Scroll area para el formulario
    formScrollArea = new QScrollArea();
    formScrollArea->setWidgetResizable(true);
    formScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    formScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    formContentWidget = new QWidget();
    formLayout = new QFormLayout(formContentWidget);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(15, 15, 15, 15);
    
    formScrollArea->setWidget(formContentWidget);
    
    // Estado vacío inicial
    emptyStateWidget = new QWidget();
    emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setAlignment(Qt::AlignCenter);
    
    emptyStateLabel = new QLabel("Selecciona una tabla");
    emptyStateLabel->setFont(QFont("Inter", 16, QFont::Medium));
    emptyStateLabel->setAlignment(Qt::AlignCenter);
    
    emptyStateDescription = new QLabel("Selecciona una tabla del menú desplegable para generar su formulario");
    emptyStateDescription->setFont(QFont("Inter", 12, QFont::Normal));
    emptyStateDescription->setAlignment(Qt::AlignCenter);
    emptyStateDescription->setWordWrap(true);
    
    emptyStateLayout->addWidget(emptyStateLabel);
    emptyStateLayout->addWidget(emptyStateDescription);
    
    formAreaLayout->addWidget(formTitleLabel);
    formAreaLayout->addWidget(emptyStateWidget);
}

void FormulariosView::createDataViewer()
{
    // Área del visualizador de datos (lado derecho)
    dataViewerWidget = new QWidget();
    dataViewerLayout = new QVBoxLayout(dataViewerWidget);
    dataViewerLayout->setContentsMargins(0, 0, 0, 0);
    dataViewerLayout->setSpacing(15);
    
    dataViewerLabel = new QLabel("Registros Existentes");
    dataViewerLabel->setFont(QFont("Inter", 16, QFont::Bold));
    
    // Tabla para mostrar los datos
    dataTable = new QTableWidget();
    dataTable->setAlternatingRowColors(true);
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable->horizontalHeader()->setStretchLastSection(true);
    dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    dataViewerLayout->addWidget(dataViewerLabel);
    dataViewerLayout->addWidget(dataTable);
    
    // Conectar selección de registros
    connect(dataTable, &QTableWidget::itemSelectionChanged, this, &FormulariosView::onRecordSelected);
}

void FormulariosView::styleComponents()
{
    // El styling se aplicará en updateTheme()
    updateTheme();
}

void FormulariosView::loadAvailableTables()
{
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        qDebug() << "FormulariosView: MainWindow o catalog no disponible";
        return;
    }
    
    tableComboBox->clear();
    tableComboBox->addItem("-- Seleccionar tabla --");
    
    // Obtener todas las tablas del catálogo
    std::vector<TableMeta> tables = m_mainWindow->catalog()->getAllTables();
    
    for (const auto& table : tables) {
        QString tableName = QString::fromStdString(table.name);
        tableComboBox->addItem(tableName);
        qDebug() << "FormulariosView: Tabla encontrada:" << tableName;
    }
}

void FormulariosView::generateFormForTable(const QString& tableName)
{
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        qDebug() << "FormulariosView: MainWindow o catalog no disponible";
        return;
    }
    
    if (tableName.isEmpty() || tableName == "-- Seleccionar tabla --") {
        return;
    }
    
    // Limpiar formulario anterior
    clearForm();
    
    // Cargar metadatos de la tabla
    QString metaPath = QString::fromStdString(m_mainWindow->tablesDir()) + "/" + tableName + ".meta";
    QFile metaFile(metaPath);
    
    if (!metaFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", QString("No se pudo abrir el archivo de metadatos para la tabla '%1'").arg(tableName));
        return;
    }
    
    QByteArray metaData = metaFile.readAll();
    metaFile.close();
    
    QJsonParseError parseError;
    QJsonDocument metaDoc = QJsonDocument::fromJson(metaData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "Error", QString("Error al parsear metadatos: %1").arg(parseError.errorString()));
        return;
    }
    
    QJsonObject metaObj = metaDoc.object();
    currentTableFields = metaObj.value("fields").toArray();
    
    if (currentTableFields.isEmpty()) {
        QMessageBox::information(this, "Información", QString("La tabla '%1' no tiene campos definidos").arg(tableName));
        return;
    }
    
    // Mostrar el área del formulario y ocultar el estado vacío
    formAreaLayout->removeWidget(emptyStateWidget);
    emptyStateWidget->setVisible(false);
    
    if (formAreaLayout->indexOf(formScrollArea) == -1) {
        formAreaLayout->addWidget(formScrollArea);
    }
    formScrollArea->setVisible(true);
    
    // Actualizar título del formulario
    formTitleLabel->setText(QString("Formulario - %1").arg(tableName));
    
    // Generar campos del formulario
    fieldNames.clear();
    formWidgets.clear();
    
    for (int i = 0; i < currentTableFields.size(); ++i) {
        QJsonObject field = currentTableFields[i].toObject();
        QString fieldName = field.value("name").toString();
        
        if (fieldName.isEmpty()) continue;
        
        fieldNames.append(fieldName);
        
        QWidget* fieldWidget = createFieldWidget(field);
        if (fieldWidget) {
            QString labelText = fieldName;
            
            // Agregar indicadores visuales
            if (field.value("isPrimaryKey").toBool()) {
                labelText += " 🔑";
            }
            if (field.value("isForeignKey").toBool()) {
                labelText += " 🔗";
            }
            if (!field.value("allowNull").toBool()) {
                labelText += " *";
            }
            
            formLayout->addRow(labelText + ":", fieldWidget);
            formWidgets.append(fieldWidget);
        }
    }
    
    // Habilitar botones
    saveRecordBtn->setEnabled(true);
    clearFormBtn->setEnabled(true);
    
    // Cargar datos existentes
    loadTableData(tableName);
    
    qDebug() << "FormulariosView: Formulario generado para tabla" << tableName << "con" << fieldNames.size() << "campos";
}

QWidget* FormulariosView::createFieldWidget(const QJsonObject& fieldMeta)
{
    QString fieldType = fieldMeta.value("type").toString().toLower();
    QString fieldName = fieldMeta.value("name").toString();
    bool allowNull = fieldMeta.value("allowNull").toBool(true);
    bool isPrimaryKey = fieldMeta.value("isPrimaryKey").toBool(false);
    
    QWidget* widget = nullptr;
    
    if (fieldType == "number") {
        QString numberKind = fieldMeta.value("numberKind").toString().toLower();
        
        if (numberKind == "integer") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setRange(-999999999, 999999999);
            spinBox->setValue(0);
            
            // Si es clave primaria, lo hacemos de solo lectura (autoincrement)
            if (isPrimaryKey) {
                spinBox->setReadOnly(true);
                spinBox->setSpecialValueText("Auto");
                spinBox->setValue(spinBox->minimum());
            }
            
            widget = spinBox;
        } else {
            QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox();
            doubleSpinBox->setRange(-999999999.99, 999999999.99);
            doubleSpinBox->setDecimals(2);
            doubleSpinBox->setValue(0.0);
            widget = doubleSpinBox;
        }
    }
    else if (fieldType == "text") {
        int maxSize = fieldMeta.value("textMaxSize").toInt(255);
        
        if (maxSize > 100) {
            QTextEdit* textEdit = new QTextEdit();
            textEdit->setMaximumHeight(100);
            widget = textEdit;
        } else {
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setMaxLength(maxSize);
            widget = lineEdit;
        }
    }
    else if (fieldType == "date") {
        QDateEdit* dateEdit = new QDateEdit();
        dateEdit->setDate(QDate::currentDate());
        dateEdit->setCalendarPopup(true);
        widget = dateEdit;
    }
    else if (fieldType == "boolean") {
        QCheckBox* checkBox = new QCheckBox();
        widget = checkBox;
    }
    else {
        // Tipo por defecto: texto
        QLineEdit* lineEdit = new QLineEdit();
        widget = lineEdit;
    }
    
    if (widget) {
        widget->setFont(QFont("Inter", 11, QFont::Normal));
        widget->setFixedHeight(35);
        
        // Tooltip con información del campo
        QString tooltip = QString("Campo: %1\nTipo: %2").arg(fieldName, fieldType);
        if (!allowNull) tooltip += "\nCampo requerido";
        if (isPrimaryKey) tooltip += "\nClave primaria";
        
        widget->setToolTip(tooltip);
    }
    
    return widget;
}

void FormulariosView::loadTableData(const QString& tableName)
{
    if (!m_mainWindow || !m_mainWindow->catalog() || tableName.isEmpty()) {
        return;
    }
    
    std::string error;
    std::vector<std::string> records = m_mainWindow->catalog()->readAllRecordsJson(
        m_mainWindow->tablesDir(), 
        tableName.toStdString(), 
        &error
    );
    
    if (!error.empty()) {
        qDebug() << "FormulariosView: Error al leer registros:" << QString::fromStdString(error);
        return;
    }
    
    // Configurar tabla
    dataTable->setRowCount(records.size());
    dataTable->setColumnCount(fieldNames.size());
    dataTable->setHorizontalHeaderLabels(fieldNames);
    
    // Llenar datos
    for (int row = 0; row < static_cast<int>(records.size()); ++row) {
        QJsonParseError parseError;
        QJsonDocument recordDoc = QJsonDocument::fromJson(records[row].c_str(), &parseError);
        
        if (parseError.error == QJsonParseError::NoError && recordDoc.isObject()) {
            QJsonObject recordObj = recordDoc.object();
            
            for (int col = 0; col < fieldNames.size(); ++col) {
                QString fieldName = fieldNames[col];
                QJsonValue value = recordObj.value(fieldName);
                
                QString displayText;
                if (value.isString()) {
                    displayText = value.toString();
                } else if (value.isDouble()) {
                    displayText = QString::number(value.toDouble());
                } else if (value.isBool()) {
                    displayText = value.toBool() ? "Sí" : "No";
                } else {
                    displayText = value.toVariant().toString();
                }
                
                QTableWidgetItem* item = new QTableWidgetItem(displayText);
                dataTable->setItem(row, col, item);
            }
        }
    }
    
    // Ajustar columnas
    dataTable->resizeColumnsToContents();
    
    // Update tableData array for navigation
    tableData = QJsonArray();
    for (int row = 0; row < static_cast<int>(records.size()); ++row) {
        QJsonParseError parseError;
        QJsonDocument recordDoc = QJsonDocument::fromJson(records[row].c_str(), &parseError);
        if (parseError.error == QJsonParseError::NoError && recordDoc.isObject()) {
            tableData.append(recordDoc.object());
        }
    }
    
    // Initialize navigation
    currentRecordIndex = tableData.size() > 0 ? 0 : -1;
    updateNavigationButtons();
    updateButtonStates();
    
    // Load first record if available
    if (currentRecordIndex >= 0) {
        loadRecordAtIndex(currentRecordIndex);
        dataTable->selectRow(currentRecordIndex);
    }
    
    qDebug() << "FormulariosView: Cargados" << records.size() << "registros para tabla" << tableName;
}

void FormulariosView::clearForm()
{
    // Limpiar widgets del formulario anterior
    while (formLayout->count() > 0) {
        QLayoutItem* item = formLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    formWidgets.clear();
    fieldNames.clear();
    currentTableFields = QJsonArray();
}

QJsonObject FormulariosView::getFormData()
{
    QJsonObject data;
    
    for (int i = 0; i < formWidgets.size() && i < fieldNames.size(); ++i) {
        QString fieldName = fieldNames[i];
        QWidget* widget = formWidgets[i];
        
        if (!widget) continue;
        
        QJsonObject fieldMeta = currentTableFields[i].toObject();
        QString fieldType = fieldMeta.value("type").toString().toLower();
        bool isPrimaryKey = fieldMeta.value("isPrimaryKey").toBool(false);
        
        // Si es clave primaria y es autoincrement, no incluir en los datos
        if (isPrimaryKey && fieldType == "number") {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
            if (spinBox && spinBox->value() == spinBox->minimum()) {
                continue; // Skip auto-increment fields
            }
        }
        
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            data[fieldName] = lineEdit->text();
        }
        else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
            data[fieldName] = textEdit->toPlainText();
        }
        else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
            data[fieldName] = spinBox->value();
        }
        else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
            data[fieldName] = doubleSpinBox->value();
        }
        else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
            data[fieldName] = dateEdit->date().toString(Qt::ISODate);
        }
        else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
            data[fieldName] = checkBox->isChecked();
        }
    }
    
    return data;
}

void FormulariosView::populateFormWithRecord(const QJsonObject& record)
{
    for (int i = 0; i < formWidgets.size() && i < fieldNames.size(); ++i) {
        QString fieldName = fieldNames[i];
        QWidget* widget = formWidgets[i];
        
        if (!widget || !record.contains(fieldName)) continue;
        
        QJsonValue value = record.value(fieldName);
        
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setText(value.toString());
        }
        else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
            textEdit->setPlainText(value.toString());
        }
        else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
            spinBox->setValue(value.toInt());
        }
        else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
            doubleSpinBox->setValue(value.toDouble());
        }
        else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
            QString dateStr = value.toString();
            QDate date = QDate::fromString(dateStr, Qt::ISODate);
            if (date.isValid()) {
                dateEdit->setDate(date);
            }
        }
        else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(value.toBool());
        }
    }
}

void FormulariosView::updateTheme()
{
    bool isDark = ThemeManager::instance().isDark();
    
    // Colores base del tema
    QString bgColor = isDark ? "#1e1e1e" : "#ffffff";
    QString textColor = isDark ? "#ffffff" : "#000000";
    QString secondaryTextColor = isDark ? "#b3b3b3" : "#666666";
    QString borderColor = isDark ? "#404040" : "#e0e0e0";
    QString hoverColor = isDark ? "#2d2d2d" : "#f5f5f5";
    QString primaryColor = isDark ? "#007acc" : "#0078d4";
    QString buttonBgColor = isDark ? "#0e639c" : "#0078d4";
    QString buttonHoverColor = isDark ? "#1177bb" : "#106ebe";
    
    // Estilo del widget principal
    setStyleSheet(QString("FormulariosView { background-color: %1; }").arg(bgColor));
    
    // Estilo del header
    titleLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    descriptionLabel->setStyleSheet(QString("color: %1;").arg(secondaryTextColor));
    
    // Estilo de los botones del toolbar
    QString buttonStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
        "QPushButton:disabled {"
        "    background-color: %4;"
        "    color: %5;"
        "}"
    ).arg(buttonBgColor)
     .arg(buttonHoverColor)
     .arg(isDark ? "#0d5a94" : "#005a9e")
     .arg(isDark ? "#333333" : "#cccccc")
     .arg(isDark ? "#666666" : "#999999");
    
    saveRecordBtn->setStyleSheet(buttonStyle);
    
    QString secondaryButtonStyle = QString(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %3;"
        "}"
        "QPushButton:disabled {"
        "    color: %4;"
        "    border-color: %4;"
        "}"
    ).arg(textColor)
     .arg(borderColor)
     .arg(hoverColor)
     .arg(isDark ? "#666666" : "#cccccc");
    
    clearFormBtn->setStyleSheet(secondaryButtonStyle);
    refreshDataBtn->setStyleSheet(secondaryButtonStyle);
    
    // Estilo del selector de tabla
    tableSelectorLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    
    QString comboStyle = QString(
        "QComboBox {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 6px;"
        "    padding: 8px;"
        "    color: %3;"
        "}"
        "QComboBox:hover {"
        "    border-color: %4;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
        "QComboBox::down-arrow {"
        "    width: 12px;"
        "    height: 12px;"
        "}"
    ).arg(bgColor).arg(borderColor).arg(textColor).arg(primaryColor);
    
    tableComboBox->setStyleSheet(comboStyle);
    
    // Estilo de las etiquetas del formulario
    formTitleLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    dataViewerLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    
    // Estilo del estado vacío
    emptyStateLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    emptyStateDescription->setStyleSheet(QString("color: %1;").arg(secondaryTextColor));
    
    // Estilo de la tabla
    QString tableStyle = QString(
        "QTableWidget {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "    color: %3;"
        "    gridline-color: %2;"
        "}"
        "QTableWidget::item {"
        "    padding: 5px;"
        "    border-bottom: 1px solid %2;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: %4;"
        "    color: white;"
        "}"
        "QHeaderView::section {"
        "    background-color: %5;"
        "    color: %3;"
        "    padding: 8px;"
        "    border: 1px solid %2;"
        "    font-weight: bold;"
        "}"
    ).arg(bgColor)
     .arg(borderColor)
     .arg(textColor)
     .arg(primaryColor)
     .arg(isDark ? "#2d2d2d" : "#f8f9fa");
    
    dataTable->setStyleSheet(tableStyle);
    
    // Estilo del scroll area
    QString scrollStyle = QString(
        "QScrollArea {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "}"
    ).arg(bgColor).arg(borderColor);
    
    formScrollArea->setStyleSheet(scrollStyle);
}

void FormulariosView::refreshView()
{
    qDebug() << "FormulariosView: Actualizando vista...";
    loadAvailableTables();
    if (!currentTableName.isEmpty()) {
        loadTableData(currentTableName);
    }
}

// Slots
void FormulariosView::onCreateFormClicked()
{
    // Esta función ya no se usa en el nuevo diseño
    qDebug() << "FormulariosView: onCreateFormClicked - función obsoleta";
}

void FormulariosView::onTableSelected()
{
    QString selectedTable = tableComboBox->currentText();
    
    if (selectedTable == "-- Seleccionar tabla --") {
        currentTableName = "";
        clearForm();
        
        // Mostrar estado vacío
        if (formAreaLayout->indexOf(formScrollArea) != -1) {
            formAreaLayout->removeWidget(formScrollArea);
            formScrollArea->setVisible(false);
        }
        
        if (formAreaLayout->indexOf(emptyStateWidget) == -1) {
            formAreaLayout->addWidget(emptyStateWidget);
        }
        emptyStateWidget->setVisible(true);
        
        // Deshabilitar botones
        saveRecordBtn->setEnabled(false);
        clearFormBtn->setEnabled(false);
        
        // Limpiar tabla de datos
        dataTable->clear();
        dataTable->setRowCount(0);
        dataTable->setColumnCount(0);
        
        return;
    }
    
    currentTableName = selectedTable;
    generateFormForTable(selectedTable);
}

void FormulariosView::onSaveRecordClicked()
{
    if (currentTableName.isEmpty() || !m_mainWindow || !m_mainWindow->catalog()) {
        QMessageBox::warning(this, "Error", "No hay tabla seleccionada o catálogo no disponible");
        return;
    }
    
    QJsonObject recordData = getFormData();
    
    if (recordData.isEmpty()) {
        QMessageBox::information(this, "Información", "No hay datos para guardar");
        return;
    }
    
    // Convertir a JSON string
    QJsonDocument doc(recordData);
    QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    
    std::string error;
    bool success = false;
    
    if (isEditMode && currentRecordIndex >= 0 && currentRecordIndex < tableData.size()) {
        // Edit mode - update existing record
        tableData[currentRecordIndex] = recordData;
        saveTableData(currentTableName, tableData);
        success = true;
        QMessageBox::information(this, "Éxito", "Registro actualizado exitosamente");
    } else {
        // New record mode - append new record
        success = m_mainWindow->catalog()->appendRecordJson(
            m_mainWindow->tablesDir(),
            currentTableName.toStdString(),
            jsonString.toStdString(),
            &error
        );
        
        if (success) {
            QMessageBox::information(this, "Éxito", "Registro guardado exitosamente");
        } else {
            QMessageBox::critical(this, "Error", QString("Error al guardar registro: %1").arg(QString::fromStdString(error)));
        }
    }
    
    if (success) {
        // Reset edit mode
        isEditMode = false;
        setFormEnabled(false);
        
        // Reload data and update navigation
        loadTableData(currentTableName);
    }
}

void FormulariosView::onClearFormClicked()
{
    for (QWidget* widget : formWidgets) {
        if (!widget) continue;
        
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->clear();
        }
        else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
            textEdit->clear();
        }
        else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
            if (!spinBox->isReadOnly()) {
                spinBox->setValue(0);
            }
        }
        else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
            doubleSpinBox->setValue(0.0);
        }
        else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
            dateEdit->setDate(QDate::currentDate());
        }
        else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(false);
        }
    }
    
    // Reset states
    isEditMode = false;
    currentRecordIndex = -1;
    setFormEnabled(false);
    updateButtonStates();
    updateNavigationButtons();
    
    qDebug() << "FormulariosView: Formulario limpiado";
}

void FormulariosView::onRefreshDataClicked()
{
    refreshView();
}

void FormulariosView::onRecordSelected()
{
    QList<QTableWidgetItem*> selectedItems = dataTable->selectedItems();
    if (selectedItems.isEmpty()) return;
    
    int row = selectedItems.first()->row();
    
    // Crear objeto JSON con los datos de la fila seleccionada
    QJsonObject record;
    for (int col = 0; col < fieldNames.size() && col < dataTable->columnCount(); ++col) {
        QTableWidgetItem* item = dataTable->item(row, col);
        if (item) {
            QString fieldName = fieldNames[col];
            QString value = item->text();
            
            // Intentar convertir a tipo apropiado
            if (col < currentTableFields.size()) {
                QJsonObject fieldMeta = currentTableFields[col].toObject();
                QString fieldType = fieldMeta.value("type").toString().toLower();
                
                if (fieldType == "number") {
                    QString numberKind = fieldMeta.value("numberKind").toString().toLower();
                    if (numberKind == "integer") {
                        record[fieldName] = value.toInt();
                    } else {
                        record[fieldName] = value.toDouble();
                    }
                } else if (fieldType == "boolean") {
                    record[fieldName] = (value == "Sí" || value.toLower() == "true");
                } else {
                    record[fieldName] = value;
                }
            } else {
                record[fieldName] = value;
            }
        }
    }
    
    // Llenar el formulario con los datos del registro seleccionado
    populateFormWithRecord(record);
    
    // Update current record index
    currentRecordIndex = row;
    updateNavigationButtons();
    
    qDebug() << "FormulariosView: Registro seleccionado cargado en formulario";
}

void FormulariosView::onNewRecordClicked()
{
    clearForm();
    isEditMode = false;
    currentRecordIndex = -1;
    updateNavigationButtons();
    updateButtonStates();
    
    // Enable form editing
    setFormEnabled(true);
    
    qDebug() << "FormulariosView: Modo nuevo registro activado";
}

void FormulariosView::onEditRecordClicked()
{
    if (dataTable->currentRow() < 0) {
        qDebug() << "FormulariosView: No hay registro seleccionado para editar";
        return;
    }
    
    isEditMode = true;
    currentRecordIndex = dataTable->currentRow();
    updateButtonStates();
    
    // Enable form editing
    setFormEnabled(true);
    
    qDebug() << "FormulariosView: Modo edición activado para registro" << currentRecordIndex;
}

void FormulariosView::onDeleteRecordClicked()
{
    if (dataTable->currentRow() < 0) {
        qDebug() << "FormulariosView: No hay registro seleccionado para eliminar";
        return;
    }
    
    int reply = QMessageBox::question(this, "Confirmar eliminación", 
                                    "¿Está seguro de que desea eliminar este registro?",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Remove from data array
        tableData.removeAt(currentRecordIndex);
        
        // Update data file
        saveTableData(currentTableName, tableData);
        
        // Refresh view
        loadTableData(currentTableName);
        clearForm();
        isEditMode = false;
        currentRecordIndex = -1;
        updateNavigationButtons();
        updateButtonStates();
        
        qDebug() << "FormulariosView: Registro eliminado";
    }
}

void FormulariosView::onFirstRecordClicked()
{
    if (tableData.size() > 0) {
        currentRecordIndex = 0;
        loadRecordAtIndex(currentRecordIndex);
        updateNavigationButtons();
        dataTable->selectRow(currentRecordIndex);
    }
}

void FormulariosView::onPreviousRecordClicked()
{
    if (currentRecordIndex > 0) {
        currentRecordIndex--;
        loadRecordAtIndex(currentRecordIndex);
        updateNavigationButtons();
        dataTable->selectRow(currentRecordIndex);
    }
}

void FormulariosView::onNextRecordClicked()
{
    if (currentRecordIndex < tableData.size() - 1) {
        currentRecordIndex++;
        loadRecordAtIndex(currentRecordIndex);
        updateNavigationButtons();
        dataTable->selectRow(currentRecordIndex);
    }
}

void FormulariosView::onLastRecordClicked()
{
    if (tableData.size() > 0) {
        currentRecordIndex = tableData.size() - 1;
        loadRecordAtIndex(currentRecordIndex);
        updateNavigationButtons();
        dataTable->selectRow(currentRecordIndex);
    }
}

void FormulariosView::onRecordNavigationChanged()
{
    updateNavigationButtons();
}

void FormulariosView::updateNavigationButtons()
{
    bool hasRecords = tableData.size() > 0;
    bool isFirst = currentRecordIndex <= 0;
    bool isLast = currentRecordIndex >= tableData.size() - 1;
    
    firstRecordBtn->setEnabled(hasRecords && !isFirst);
    previousRecordBtn->setEnabled(hasRecords && !isFirst);
    nextRecordBtn->setEnabled(hasRecords && !isLast);
    lastRecordBtn->setEnabled(hasRecords && !isLast);
    
    if (hasRecords) {
        currentRecordLabel->setText(QString("Registro: %1 de %2")
                                  .arg(currentRecordIndex + 1)
                                  .arg(tableData.size()));
        recordCountLabel->setText(QString("Total: %1 registros").arg(tableData.size()));
    } else {
        currentRecordLabel->setText("Registro: 0 de 0");
        recordCountLabel->setText("Total: 0 registros");
    }
}

void FormulariosView::updateButtonStates()
{
    bool hasTable = !currentTableName.isEmpty();
    bool hasRecord = currentRecordIndex >= 0 && currentRecordIndex < tableData.size();
    bool inEditMode = isEditMode || currentRecordIndex == -1; // New record mode
    
    newRecordBtn->setEnabled(hasTable);
    editRecordBtn->setEnabled(hasTable && hasRecord && !inEditMode);
    deleteRecordBtn->setEnabled(hasTable && hasRecord && !inEditMode);
    saveRecordBtn->setEnabled(hasTable && inEditMode);
    clearFormBtn->setEnabled(hasTable);
    refreshDataBtn->setEnabled(hasTable);
}

void FormulariosView::setFormEnabled(bool enabled)
{
    for (QWidget* widget : formWidgets) {
        widget->setEnabled(enabled);
    }
}

void FormulariosView::loadRecordAtIndex(int index)
{
    if (index >= 0 && index < tableData.size()) {
        QJsonObject record = tableData[index].toObject();
        populateFormWithRecord(record);
        isEditMode = false;
        updateButtonStates();
    }
}

void FormulariosView::saveTableData(const QString& tableName, const QJsonArray& data)
{
    QString projectPath = QString("proyectos/%1").arg(getCurrentProject());
    QString tableFilePath = QString("%1/tables/%2.json").arg(projectPath, tableName);
    
    QJsonDocument doc(data);
    QFile file(tableFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        qDebug() << "FormulariosView: Datos guardados en" << tableFilePath;
    } else {
        qDebug() << "FormulariosView: Error al guardar datos en" << tableFilePath;
    }
}

QString FormulariosView::getCurrentProject()
{
    // This should return the current project name
    // For now, we'll use "proyecto1" as default
    return "proyecto1";
}