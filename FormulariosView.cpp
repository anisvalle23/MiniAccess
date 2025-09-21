#include "FormulariosView.h"
#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QDir>

FormulariosView::FormulariosView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
    , currentRecordIndex(-1)
    , isEditMode(false)
{
    qDebug() << "FormulariosView: Constructor iniciado";
    setupUI();
    updateTheme();
    qDebug() << "FormulariosView: Constructor terminado";
}

FormulariosView::~FormulariosView()
{
}

void FormulariosView::setupUI()
{
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    // Título
    titleLabel = new QLabel("Formularios de Datos");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);
    
    // Selector de tabla
    QWidget *selectorWidget = new QWidget();
    selectorWidget->setStyleSheet(
        "QWidget {"
        "background-color: white;"
        "border: 1px solid #e0e0e0;"
        "border-radius: 8px;"
        "padding: 15px;"
        "}"
    );
    QHBoxLayout *selectorLayout = new QHBoxLayout(selectorWidget);
    selectorLayout->setContentsMargins(15, 15, 15, 15);
    selectorLayout->setSpacing(15);
    
    QLabel *selectorLabel = new QLabel("Seleccionar Tabla:");
    selectorLabel->setStyleSheet(
        "font-size: 16px;"
        "font-weight: 600;"
        "color: #2c3e50;"
        "border: none;"
        "background: transparent;"
    );
    
    tableComboBox = new QComboBox();
    tableComboBox->setFixedHeight(40);
    tableComboBox->setMinimumWidth(250);
    tableComboBox->setStyleSheet(
        "QComboBox {"
        "border: 2px solid #e9ecef;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 14px;"
        "background-color: white;"
        "color: #495057;"
        "}"
        "QComboBox:focus {"
        "border-color: #007bff;"
        "outline: none;"
        "}"
        "QComboBox::drop-down {"
        "border: none;"
        "width: 30px;"
        "}"
        "QComboBox::down-arrow {"
        "image: none;"
        "border: 3px solid #6c757d;"
        "width: 8px;"
        "height: 8px;"
        "border-top: none;"
        "border-left: none;"
        "margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "border: 1px solid #dee2e6;"
        "background-color: white;"
        "selection-background-color: #e3f2fd;"
        "}"
    );
    
    selectorLayout->addWidget(selectorLabel);
    selectorLayout->addWidget(tableComboBox);
    selectorLayout->addStretch();
    
    mainLayout->addWidget(selectorWidget);
    
    // Botones simples
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    
    newRecordBtn = new QPushButton("Nuevo Registro");
    editRecordBtn = new QPushButton("Editar");
    deleteRecordBtn = new QPushButton("Eliminar");
    saveRecordBtn = new QPushButton("Guardar");
    
    // Estilo mejorado para los botones
    QString primaryButtonStyle = 
        "QPushButton {"
        "background-color: #007bff;"
        "color: white;"
        "border: none;"
        "border-radius: 6px;"
        "padding: 8px 16px;"
        "font-size: 14px;"
        "font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "background-color: #004085;"
        "}"
        "QPushButton:disabled {"
        "background-color: #6c757d;"
        "color: #fff;"
        "}";
    
    QString secondaryButtonStyle = 
        "QPushButton {"
        "background-color: #6c757d;"
        "color: white;"
        "border: none;"
        "border-radius: 6px;"
        "padding: 8px 16px;"
        "font-size: 14px;"
        "font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "background-color: #545b62;"
        "}"
        "QPushButton:pressed {"
        "background-color: #3e444a;"
        "}"
        "QPushButton:disabled {"
        "background-color: #e9ecef;"
        "color: #6c757d;"
        "}";
    
    QString dangerButtonStyle = 
        "QPushButton {"
        "background-color: #dc3545;"
        "color: white;"
        "border: none;"
        "border-radius: 6px;"
        "padding: 8px 16px;"
        "font-size: 14px;"
        "font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "background-color: #c82333;"
        "}"
        "QPushButton:pressed {"
        "background-color: #bd2130;"
        "}"
        "QPushButton:disabled {"
        "background-color: #e9ecef;"
        "color: #6c757d;"
        "}";
    
    newRecordBtn->setStyleSheet(primaryButtonStyle);
    saveRecordBtn->setStyleSheet(primaryButtonStyle);
    editRecordBtn->setStyleSheet(secondaryButtonStyle);
    deleteRecordBtn->setStyleSheet(dangerButtonStyle);
    
    newRecordBtn->setFixedHeight(40);
    editRecordBtn->setFixedHeight(40);
    deleteRecordBtn->setFixedHeight(40);
    saveRecordBtn->setFixedHeight(40);
    
    editRecordBtn->setEnabled(false);
    deleteRecordBtn->setEnabled(false);
    saveRecordBtn->setEnabled(false);
    
    buttonLayout->addWidget(newRecordBtn);
    buttonLayout->addWidget(editRecordBtn);
    buttonLayout->addWidget(deleteRecordBtn);
    buttonLayout->addWidget(saveRecordBtn);
    buttonLayout->addStretch();
    
    mainLayout->addWidget(buttonWidget);
    
    // Área principal con splitter
    mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Formulario (lado izquierdo)
    formAreaWidget = new QWidget();
    formAreaWidget->setStyleSheet(
        "QWidget {"
        "background-color: white;"
        "border: 1px solid #e0e0e0;"
        "border-radius: 8px;"
        "}"
    );
    formAreaLayout = new QVBoxLayout(formAreaWidget);
    formAreaLayout->setContentsMargins(20, 20, 20, 20);
    formAreaLayout->setSpacing(15);
    
    QLabel *formTitle = new QLabel("Formulario de Entrada");
    formTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "margin-bottom: 10px;"
        "border: none;"
    );
    formAreaLayout->addWidget(formTitle);
    
    formScrollArea = new QScrollArea();
    formScrollArea->setStyleSheet(
        "QScrollArea {"
        "border: none;"
        "background-color: transparent;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "background-color: transparent;"
        "}"
    );
    formContentWidget = new QWidget();
    formLayout = new QFormLayout(formContentWidget);
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formScrollArea->setWidget(formContentWidget);
    formScrollArea->setWidgetResizable(true);
    formAreaLayout->addWidget(formScrollArea);
    
    // Estado vacío
    emptyStateWidget = new QWidget();
    emptyStateWidget->setStyleSheet(
        "QWidget {"
        "background-color: #f8f9fa;"
        "border: 2px dashed #dee2e6;"
        "border-radius: 8px;"
        "}"
    );
    emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setContentsMargins(40, 40, 40, 40);
    
    emptyStateLabel = new QLabel("Selecciona una tabla");
    emptyStateLabel->setAlignment(Qt::AlignCenter);
    emptyStateLabel->setStyleSheet(
        "font-size: 18px;"
        "color: #6c757d;"
        "font-weight: 500;"
        "border: none;"
        "background: transparent;"
    );
    
    QLabel *emptyStateSubtitle = new QLabel("Elige una tabla del menú desplegable para comenzar a trabajar con los formularios");
    emptyStateSubtitle->setAlignment(Qt::AlignCenter);
    emptyStateSubtitle->setWordWrap(true);
    emptyStateSubtitle->setStyleSheet(
        "font-size: 14px;"
        "color: #8e9aaf;"
        "border: none;"
        "background: transparent;"
        "margin-top: 10px;"
    );
    
    emptyStateLayout->addWidget(emptyStateLabel);
    emptyStateLayout->addWidget(emptyStateSubtitle);
    formAreaLayout->addWidget(emptyStateWidget);
    
    // Tabla de datos (lado derecho)
    dataViewerWidget = new QWidget();
    dataViewerWidget->setStyleSheet(
        "QWidget {"
        "background-color: white;"
        "border: 1px solid #e0e0e0;"
        "border-radius: 8px;"
        "}"
    );
    dataViewerLayout = new QVBoxLayout(dataViewerWidget);
    dataViewerLayout->setContentsMargins(20, 20, 20, 20);
    dataViewerLayout->setSpacing(15);
    
    QLabel *dataTitle = new QLabel("Registros Existentes");
    dataTitle->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "margin-bottom: 10px;"
        "border: none;"
    );
    dataViewerLayout->addWidget(dataTitle);
    
    dataTable = new QTableWidget();
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable->setAlternatingRowColors(true);
    dataTable->setStyleSheet(
        "QTableWidget {"
        "border: 1px solid #dee2e6;"
        "border-radius: 6px;"
        "background-color: white;"
        "gridline-color: #e9ecef;"
        "font-size: 14px;"
        "}"
        "QTableWidget::item {"
        "padding: 8px;"
        "border-bottom: 1px solid #e9ecef;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #e3f2fd;"
        "color: #1976d2;"
        "}"
        "QHeaderView::section {"
        "background-color: #f8f9fa;"
        "border: 1px solid #dee2e6;"
        "padding: 10px 8px;"
        "font-weight: 600;"
        "color: #495057;"
        "font-size: 14px;"
        "}"
        "QTableWidget::item:alternate {"
        "background-color: #f8f9fa;"
        "}"
    );
    dataViewerLayout->addWidget(dataTable);
    
    // Agregar al splitter
    mainSplitter->addWidget(formAreaWidget);
    mainSplitter->addWidget(dataViewerWidget);
    mainSplitter->setSizes({400, 600});
    
    mainLayout->addWidget(mainSplitter, 1);
    
    // Conectar señales
    connect(tableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &FormulariosView::onTableSelected);
    connect(newRecordBtn, &QPushButton::clicked, this, &FormulariosView::onNewRecordClicked);
    connect(editRecordBtn, &QPushButton::clicked, this, &FormulariosView::onEditRecordClicked);
    connect(deleteRecordBtn, &QPushButton::clicked, this, &FormulariosView::onDeleteRecordClicked);
    connect(saveRecordBtn, &QPushButton::clicked, this, &FormulariosView::onSaveRecordClicked);
    connect(dataTable, &QTableWidget::itemSelectionChanged, this, &FormulariosView::onRecordSelected);
    
    // Cargar tablas
    qDebug() << "FormulariosView: Llamando loadAvailableTables() desde setupUI()";
    loadAvailableTables();
}

void FormulariosView::loadAvailableTables()
{
    qDebug() << "FormulariosView: loadAvailableTables() iniciado";
    
    tableComboBox->clear();
    tableComboBox->addItem("-- Seleccionar tabla --");
    
    if (!m_mainWindow) {
        qDebug() << "FormulariosView: MainWindow no disponible";
        return;
    }
    
    // Obtener información del proyecto actual
    QString currentProject = m_mainWindow->getCurrentProjectName();
    qDebug() << "FormulariosView: Proyecto actual:" << currentProject;
    
    // Usar la ruta correcta del directorio de tablas desde MainWindow
    QString tablesPath = QString::fromStdString(m_mainWindow->tablesDir());
    QDir tablesDir(tablesPath);
    
    qDebug() << "FormulariosView: Buscando tablas en:" << tablesPath;
    qDebug() << "FormulariosView: Directorio existe:" << tablesDir.exists();
    
    if (!tablesDir.exists()) {
        qDebug() << "FormulariosView: Directorio de tablas no existe:" << tablesPath;
        return;
    }
    
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    qDebug() << "FormulariosView: Archivos .meta encontrados:" << metaFiles;
    
    for (const QString &metaFile : metaFiles) {
        QString tableName = metaFile;
        tableName.remove(".meta");
        tableComboBox->addItem(tableName);
        qDebug() << "FormulariosView: Tabla agregada al combobox:" << tableName;
    }
    
    if (metaFiles.isEmpty()) {
        qDebug() << "FormulariosView: No se encontraron tablas en" << tablesPath;
    } else {
        qDebug() << "FormulariosView: Se encontraron" << metaFiles.size() << "tablas";
        qDebug() << "FormulariosView: Items en combobox:" << tableComboBox->count();
    }
}

void FormulariosView::onTableSelected()
{
    QString selectedTable = tableComboBox->currentText();
    
    if (selectedTable == "-- Seleccionar tabla --" || selectedTable.isEmpty()) {
        showEmptyState();
        return;
    }
    
    currentTableName = selectedTable;
    generateFormForTable(selectedTable);
    loadDataFromJson();  // Cargar datos existentes desde JSON
}

void FormulariosView::showEmptyState()
{
    formScrollArea->hide();
    emptyStateWidget->show();
    dataTable->clear();
    dataTable->setRowCount(0);
    dataTable->setColumnCount(0);
    
    editRecordBtn->setEnabled(false);
    deleteRecordBtn->setEnabled(false);
    saveRecordBtn->setEnabled(false);
}

void FormulariosView::generateFormForTable(const QString& tableName)
{
    if (tableName.isEmpty()) return;
    
    // Ocultar estado vacío y mostrar formulario
    emptyStateWidget->hide();
    formScrollArea->show();
    
    // Limpiar formulario anterior
    clearForm();
    
    // Cargar metadatos de la tabla usando la ruta correcta
    QString metaPath = QString::fromStdString(m_mainWindow->tablesDir()) + "/" + tableName + ".meta";
    QFile metaFile(metaPath);
    
    if (!metaFile.open(QIODevice::ReadOnly)) {
        qDebug() << "FormulariosView: No se pudo abrir archivo meta:" << metaPath;
        return;
    }
    
    QByteArray metaData = metaFile.readAll();
    metaFile.close();
    
    QJsonParseError parseError;
    QJsonDocument metaDoc = QJsonDocument::fromJson(metaData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "FormulariosView: Error al parsear meta JSON:" << parseError.errorString();
        return;
    }
    
    QJsonObject metaObj = metaDoc.object();
    currentTableFields = metaObj.value("fields").toArray();
    
    // Crear campos del formulario
    for (int i = 0; i < currentTableFields.size(); ++i) {
        QJsonObject field = currentTableFields[i].toObject();
        QString fieldName = field.value("name").toString();
        QString fieldType = field.value("type").toString();
        
        QWidget* inputWidget = createFieldWidget(field);
        if (inputWidget) {
            // Crear contenedor para input + mensaje de error
            QWidget* fieldContainer = new QWidget();
            QVBoxLayout* fieldLayout = new QVBoxLayout(fieldContainer);
            fieldLayout->setContentsMargins(0, 0, 0, 0);
            fieldLayout->setSpacing(5);
            
            // Agregar el widget de entrada
            fieldLayout->addWidget(inputWidget);
            
            // Crear etiqueta de error
            QLabel* errorLabel = new QLabel();
            errorLabel->setStyleSheet(
                "QLabel {"
                "color: #dc3545;"
                "font-size: 12px;"
                "font-weight: 500;"
                "background-color: #f8d7da;"
                "border: 1px solid #f5c6cb;"
                "border-radius: 4px;"
                "padding: 4px 8px;"
                "margin-top: 2px;"
                "}"
            );
            errorLabel->setWordWrap(true);
            errorLabel->hide(); // Ocultar inicialmente
            fieldLayout->addWidget(errorLabel);
            
            // Guardar referencia al label de error en el widget
            inputWidget->setProperty("errorLabel", QVariant::fromValue(errorLabel));
            
            formLayout->addRow(fieldName + ":", fieldContainer);
            formWidgets.append(inputWidget);
            fieldNames.append(fieldName);
            
            // Conectar validación en tiempo real
            setupRealTimeValidation(inputWidget, field);
        }
    }
    
    saveRecordBtn->setEnabled(true);
}

QWidget* FormulariosView::createFieldWidget(const QJsonObject& fieldMeta)
{
    QString fieldType = fieldMeta.value("type").toString().toLower();
    QString fieldName = fieldMeta.value("name").toString();
    bool isPrimaryKey = fieldMeta.value("isPrimaryKey").toBool(false);
    bool allowNull = fieldMeta.value("allowNull").toBool(true);
    
    // Estilo común para inputs
    QString inputStyle = 
        "QLineEdit, QSpinBox, QDoubleSpinBox, QDateEdit {"
        "border: 2px solid #e9ecef;"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 14px;"
        "background-color: white;"
        "color: #495057;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {"
        "border-color: #007bff;"
        "outline: none;"
        "background-color: #ffffff;"
        "}"
        "QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QDateEdit:disabled {"
        "background-color: #f8f9fa;"
        "color: #6c757d;"
        "border-color: #dee2e6;"
        "}"
        "QCheckBox {"
        "font-size: 14px;"
        "color: #495057;"
        "}"
        "QCheckBox::indicator {"
        "width: 18px;"
        "height: 18px;"
        "border: 2px solid #dee2e6;"
        "border-radius: 3px;"
        "background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "background-color: #007bff;"
        "border-color: #007bff;"
        "image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iOSIgdmlld0JveD0iMCAwIDEyIDkiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxwYXRoIGQ9Ik0xMC42IDEuNEw0LjMgNy43TDEuNCA0LjgiIHN0cm9rZT0id2hpdGUiIHN0cm9rZS13aWR0aD0iMS41IiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+);"
        "}";
    
    if (fieldType == "text") {
        QLineEdit* lineEdit = new QLineEdit();
        lineEdit->setFixedHeight(40);
        lineEdit->setStyleSheet(inputStyle);
        lineEdit->setPlaceholderText(QString("Ingresa %1").arg(fieldName));
        return lineEdit;
    }
    else if (fieldType == "number") {
        QString numberKind = fieldMeta.value("numberKind").toString().toLower();
        
        if (numberKind == "integer") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setRange(0, 999999999);  // Rango normal para todos los campos
            spinBox->setFixedHeight(40);
            spinBox->setStyleSheet(inputStyle);
            return spinBox;
        } else {
            QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox();
            doubleSpinBox->setRange(0.0, 999999999.99);
            doubleSpinBox->setDecimals(2);
            doubleSpinBox->setFixedHeight(40);
            doubleSpinBox->setStyleSheet(inputStyle);
            return doubleSpinBox;
        }
    }
    else if (fieldType == "date") {
        QDateEdit* dateEdit = new QDateEdit();
        dateEdit->setDate(QDate::currentDate());
        dateEdit->setCalendarPopup(true);
        dateEdit->setFixedHeight(40);
        dateEdit->setStyleSheet(inputStyle);
        dateEdit->setDisplayFormat("dd/MM/yyyy");
        return dateEdit;
    }
    else if (fieldType == "boolean") {
        QCheckBox* checkBox = new QCheckBox("Activado");
        checkBox->setStyleSheet(inputStyle);
        return checkBox;
    }
    
    // Default: text field
    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setFixedHeight(40);
    lineEdit->setStyleSheet(inputStyle);
    lineEdit->setPlaceholderText(QString("Ingresa %1").arg(fieldName));
    return lineEdit;
}

void FormulariosView::loadDataFromJson()
{
    if (currentTableName.isEmpty()) return;
    
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        qDebug() << "FormulariosView: MainWindow o catalog no disponible";
        return;
    }
    
    // Limpiar datos previos
    allRecords.clear();
    currentRecordIndex = -1;
    
    // Usar el mismo sistema que TableData para cargar datos
    std::string err;
    std::vector<std::string> lines = m_mainWindow->catalog()->readAllRecordsJson(
        m_mainWindow->tablesDir(), currentTableName.toStdString(), &err
    );
    
    if (!err.empty()) {
        qDebug() << "FormulariosView: Error al cargar datos:" << QString::fromStdString(err);
        // Mostrar estado vacío si hay error
        showEmptyState();
        return;
    }
    
    // Convertir cada línea JSON a QJsonObject
    for (const std::string& line : lines) {
        QByteArray ba = QByteArray::fromStdString(line);
        QJsonDocument doc = QJsonDocument::fromJson(ba);
        
        if (doc.isObject()) {
            allRecords.append(doc.object());
        }
    }
    
    qDebug() << "FormulariosView: Cargados" << allRecords.size() << "registros";
    
    // Actualizar tabla de datos
    updateDataTable();
    
    // Si hay registros, mostrar el primero
    if (!allRecords.isEmpty()) {
        goToRecord(0);
        formScrollArea->show();
        emptyStateWidget->hide();
    } else {
        // No hay registros - mostrar formulario vacío pero habilitado para crear nuevo
        formScrollArea->show();
        emptyStateWidget->hide();
        clearFormInputs();
        currentRecordIndex = -1;
        isEditMode = true;
        setFormEnabled(true);
    }
    
    // Actualizar botones de navegación
    updateNavigationState();
}

void FormulariosView::updateDataTable()
{
    dataTable->clear();
    dataTable->setRowCount(allRecords.size());
    dataTable->setColumnCount(fieldNames.size());
    dataTable->setHorizontalHeaderLabels(fieldNames);
    
    for (int row = 0; row < allRecords.size(); ++row) {
        QJsonObject record = allRecords[row];
        
        for (int col = 0; col < fieldNames.size(); ++col) {
            QString fieldName = cleanFieldNameUI(fieldNames[col]);  // Limpiar nombre
            QJsonValue value = record.value(fieldName);
            
            QString displayText;
            if (value.isString()) {
                displayText = value.toString();
            } else if (value.isDouble()) {
                displayText = QString::number(value.toDouble());
            } else if (value.isBool()) {
                displayText = value.toBool() ? "true" : "false";
            } else {
                displayText = "";
            }
            
            QTableWidgetItem* item = new QTableWidgetItem(displayText);
            dataTable->setItem(row, col, item);
        }
    }
    
    // Ajustar ancho de columnas
    dataTable->resizeColumnsToContents();
}

void FormulariosView::clearForm()
{
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

void FormulariosView::onNewRecordClicked()
{
    clearFormInputs();
    isEditMode = false;
    currentRecordIndex = -1;
    setFormEnabled(true);
    updateButtonStates();
    
    // Deseleccionar fila en tabla
    dataTable->blockSignals(true);
    dataTable->clearSelection();
    dataTable->blockSignals(false);
}

void FormulariosView::onEditRecordClicked()
{
    // Ya no es necesario activar edición porque el formulario siempre está editable
    // Solo asegurar que los botones estén en el estado correcto
    updateButtonStates();
}

void FormulariosView::onDeleteRecordClicked()
{
    if (currentRecordIndex < 0 || currentRecordIndex >= allRecords.size()) return;
    
    int reply = QMessageBox::question(this, "Confirmar eliminación", 
                                    "¿Está seguro de que desea eliminar este registro?",
                                    QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        allRecords.removeAt(currentRecordIndex);
        saveDataToJson();
        updateDataTable();
        
        // Ajustar índice actual
        if (currentRecordIndex >= allRecords.size()) {
            currentRecordIndex = allRecords.size() - 1;
        }
        
        if (currentRecordIndex >= 0) {
            goToRecord(currentRecordIndex);
        } else {
            clearFormInputs();
            currentRecordIndex = -1;
        }
        
        updateNavigationState();
    }
}

void FormulariosView::onSaveRecordClicked()
{
    // Validar campos antes de guardar
    if (!validateFormData()) {
        return;
    }
    
    QJsonObject recordData = getCurrentRecordData();
    
    if (recordData.isEmpty()) {
        QMessageBox::information(this, "Información", "No hay datos para guardar");
        return;
    }
    
    if (isEditMode && currentRecordIndex >= 0 && currentRecordIndex < allRecords.size()) {
        // Edit mode - update existing record
        allRecords[currentRecordIndex] = recordData;
        saveDataToJson();
        QMessageBox::information(this, "Éxito", "Registro actualizado exitosamente");
        
        // Permanecer en el mismo registro pero en modo lectura
        isEditMode = false;
        setFormEnabled(false);
        populateFormWithRecord(currentRecordIndex);
    } else {
        // New record mode - append new record
        allRecords.append(recordData);
        saveDataToJson();
        QMessageBox::information(this, "Éxito", "Registro guardado exitosamente");
        
        // Ir al nuevo registro recién creado
        int newIndex = allRecords.size() - 1;
        goToRecord(newIndex);
        isEditMode = false;
        setFormEnabled(false);
    }
    
    // Actualizar tabla y botones
    updateDataTable();
    updateNavigationState();
    hasUnsavedChanges = false;
}

void FormulariosView::onRecordSelected()
{
    int row = dataTable->currentRow();
    
    if (row < 0 || row >= allRecords.size()) {
        currentRecordIndex = -1;
        updateNavigationState();
        return;
    }
    
    // Solo cambiar si no estamos en modo edición
    if (!isEditMode) {
        goToRecord(row);
    }
}

void FormulariosView::clearFormInputs()
{
    for (QWidget* widget : formWidgets) {
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->clear();
        }
        else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
            spinBox->setValue(0);
        }
        else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
            dateEdit->setDate(QDate::currentDate());
        }
        else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(false);
        }
    }
}

void FormulariosView::setFormEnabled(bool enabled)
{
    for (QWidget* widget : formWidgets) {
        widget->setEnabled(enabled);
    }
}

void FormulariosView::updateButtonStates()
{
    bool hasTable = !currentTableName.isEmpty();
    bool hasSelection = currentRecordIndex >= 0 && currentRecordIndex < allRecords.size();
    
    newRecordBtn->setEnabled(hasTable);
    editRecordBtn->setEnabled(false);  // Ya no necesario - formulario siempre editable
    deleteRecordBtn->setEnabled(hasTable && hasSelection);
    saveRecordBtn->setEnabled(hasTable);  // Siempre habilitado si hay tabla
}

void FormulariosView::refreshView()
{
    qDebug() << "FormulariosView: refreshView() llamado";
    loadAvailableTables();
    if (!currentTableName.isEmpty()) {
        loadDataFromJson();
    }
}

void FormulariosView::onTableCreated(const QString& tableName)
{
    qDebug() << "FormulariosView: Nueva tabla creada:" << tableName;
    // Recargar la lista de tablas disponibles
    loadAvailableTables();
}

void FormulariosView::updateTheme()
{
    // Aplicar estilos básicos
    setStyleSheet("FormulariosView { background-color: #f5f5f5; }");
}

bool FormulariosView::validateFormData()
{
    // Limpiar errores previos
    clearValidationErrors();
    
    bool isValid = true;
    QStringList errors;
    
    for (int i = 0; i < formWidgets.size() && i < fieldNames.size() && i < currentTableFields.size(); ++i) {
        QString fieldName = fieldNames[i];
        QWidget* widget = formWidgets[i];
        QJsonObject fieldMeta = currentTableFields[i].toObject();
        
        bool allowNull = fieldMeta.value("allowNull").toBool(true);
        bool isPrimaryKey = fieldMeta.value("isPrimaryKey").toBool(false);
        QString fieldType = fieldMeta.value("type").toString().toLower();
        int maxLength = fieldMeta.value("maxLength").toInt(255);
        
        QString value = getWidgetValue(widget);
        bool isEmpty = value.trimmed().isEmpty();
        
        // 1. Validar campos requeridos (no nulos)
        if (!allowNull && !isPrimaryKey && isEmpty) {
            errors.append(QString("El campo '%1' es obligatorio y no puede estar vacío.").arg(fieldName));
            setFieldError(widget, true);
            isValid = false;
            continue;
        }
        
        // Si el campo está vacío y permite nulos, no validar más
        if (isEmpty && allowNull) {
            continue;
        }
        
        // 2. Validar tipos de datos
        if (!isEmpty) {
            if (fieldType == "number" || fieldType == "int" || fieldType == "integer") {
                bool ok;
                value.toDouble(&ok);
                if (!ok) {
                    errors.append(QString("El campo '%1' debe contener un número válido.").arg(fieldName));
                    setFieldError(widget, true);
                    isValid = false;
                }
            }
            else if (fieldType == "email") {
                if (!isValidEmail(value)) {
                    errors.append(QString("El campo '%1' debe contener un email válido.").arg(fieldName));
                    setFieldError(widget, true);
                    isValid = false;
                }
            }
            else if (fieldType == "phone") {
                if (!isValidPhone(value)) {
                    errors.append(QString("El campo '%1' debe contener un teléfono válido.").arg(fieldName));
                    setFieldError(widget, true);
                    isValid = false;
                }
            }
            else if (fieldType == "date") {
                if (!isValidDate(value)) {
                    errors.append(QString("El campo '%1' debe contener una fecha válida (DD/MM/YYYY).").arg(fieldName));
                    setFieldError(widget, true);
                    isValid = false;
                }
            }
        }
        
        // 3. Validar longitud máxima
        if (value.length() > maxLength) {
            errors.append(QString("El campo '%1' no puede tener más de %2 caracteres.").arg(fieldName).arg(maxLength));
            setFieldError(widget, true);
            isValid = false;
        }
        
        // 4. Validaciones especiales para campos de texto
        if ((fieldType == "text" || fieldType == "varchar") && !value.isEmpty()) {
            if (value.contains(QRegExp("[<>\"'&]"))) {
                errors.append(QString("El campo '%1' contiene caracteres no permitidos.").arg(fieldName));
                setFieldError(widget, true);
                isValid = false;
            }
        }
    }
    
    // Mostrar errores si los hay
    if (!isValid) {
        QString errorMessage = "Se encontraron los siguientes errores:\n\n";
        for (int i = 0; i < errors.size(); ++i) {
            errorMessage += QString("• %1\n").arg(errors[i]);
        }
        
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Errores de Validación");
        msgBox.setText(errorMessage);
        msgBox.setStyleSheet(
            "QMessageBox {"
            "background-color: white;"
            "}"
            "QMessageBox QLabel {"
            "color: #721c24;"
            "font-size: 14px;"
            "}"
        );
        msgBox.exec();
        
        // Enfocar el primer campo con error
        for (QWidget* widget : formWidgets) {
            if (widget->property("hasError").toBool()) {
                widget->setFocus();
                break;
            }
        }
    }
    
    return isValid;
}

// Implementaciones básicas para funciones requeridas
void FormulariosView::onCreateFormClicked() {}
void FormulariosView::onClearFormClicked() { clearFormInputs(); }
void FormulariosView::onRefreshDataClicked() { refreshView(); }
void FormulariosView::onFirstRecordClicked() {}
void FormulariosView::onPreviousRecordClicked() {}
void FormulariosView::onNextRecordClicked() {}
void FormulariosView::onLastRecordClicked() {}
void FormulariosView::onRecordNavigationChanged() {}

// Funciones auxiliares de validación
QString FormulariosView::getWidgetValue(QWidget* widget)
{
    if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
        return lineEdit->text();
    }
    else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
        return QString::number(spinBox->value());
    }
    else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
        return QString::number(doubleSpinBox->value());
    }
    else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
        return dateEdit->date().toString("dd/MM/yyyy");
    }
    else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
        return textEdit->toPlainText();
    }
    else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
        return comboBox->currentText();
    }
    
    return "";
}

void FormulariosView::setFieldError(QWidget* widget, bool hasError)
{
    widget->setProperty("hasError", hasError);
    
    QString errorStyle = hasError ? 
        "border: 2px solid #dc3545; background-color: #fff5f5;" :
        "";
    
    if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
        QString baseStyle = lineEdit->styleSheet();
        if (hasError) {
            lineEdit->setStyleSheet(baseStyle + errorStyle);
        } else {
            // Restaurar estilo original
            lineEdit->setStyleSheet(
                "QLineEdit {"
                "border: 2px solid #e9ecef;"
                "border-radius: 6px;"
                "padding: 8px 12px;"
                "font-size: 14px;"
                "background-color: white;"
                "color: #495057;"
                "}"
                "QLineEdit:focus {"
                "border-color: #007bff;"
                "outline: none;"
                "}"
            );
        }
    }
    else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
        if (hasError) {
            spinBox->setStyleSheet(spinBox->styleSheet() + errorStyle);
        }
    }
}

void FormulariosView::clearValidationErrors()
{
    for (QWidget* widget : formWidgets) {
        setFieldError(widget, false);
    }
}

bool FormulariosView::isValidEmail(const QString& email)
{
    QRegExp emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.exactMatch(email);
}

bool FormulariosView::isValidPhone(const QString& phone)
{
    QRegExp phoneRegex("^[+]?[0-9\\s\\-\\(\\)]{7,15}$");
    return phoneRegex.exactMatch(phone.trimmed());
}

bool FormulariosView::isValidDate(const QString& dateStr)
{
    QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
    return date.isValid();
}

// Funciones de navegación de registros
void FormulariosView::goToRecord(int index)
{
    if (index < 0 || index >= allRecords.size()) {
        currentRecordIndex = -1;
        clearFormInputs();
        return;
    }
    
    currentRecordIndex = index;
    populateFormWithRecord(index);
    
    // Sincronizar selección en tabla
    dataTable->blockSignals(true);
    dataTable->selectRow(index);
    dataTable->blockSignals(false);
    
    updateNavigationState();
}

void FormulariosView::populateFormWithRecord(int recordIndex)
{
    if (recordIndex < 0 || recordIndex >= allRecords.size()) return;
    
    QJsonObject record = allRecords[recordIndex];
    
    for (int i = 0; i < formWidgets.size() && i < fieldNames.size(); ++i) {
        QString fieldName = cleanFieldNameUI(fieldNames[i]);
        QWidget* widget = formWidgets[i];
        QJsonValue value = record.value(fieldName);
        
        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setText(value.toString());
        }
        else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
            spinBox->setValue(value.toInt());
        }
        else if (QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget)) {
            QString dateStr = value.toString();
            QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
            if (date.isValid()) {
                dateEdit->setDate(date);
            }
        }
    }
    
    // Mantener formulario editable para permitir modificaciones directas
    isEditMode = true;
    setFormEnabled(true);
    updateButtonStates();
}

void FormulariosView::updateNavigationState()
{
    bool hasRecords = !allRecords.isEmpty();
    bool hasSelection = currentRecordIndex >= 0 && currentRecordIndex < allRecords.size();
    
    editRecordBtn->setEnabled(hasSelection);
    deleteRecordBtn->setEnabled(hasSelection);
    saveRecordBtn->setEnabled(isEditMode);
    
    // TODO: Agregar botones de navegación (primero, anterior, siguiente, último)
    // y actualizar su estado aquí
}

QString FormulariosView::cleanFieldNameUI(const QString& fieldName)
{
    // Remover emojis de interfaz como 🔑, 🔗, etc.
    QString cleaned = fieldName;
    cleaned = cleaned.remove("🔑🔗🔶")
                    .remove("🔑🔗")
                    .remove("🔑🔶")
                    .remove("🔗🔶")
                    .remove("🔑")
                    .remove("🔗")
                    .remove("🔶")
                    .trimmed();
    return cleaned;
}

QJsonObject FormulariosView::getCurrentRecordData()
{
    QJsonObject record;
    
    for (int i = 0; i < formWidgets.size() && i < fieldNames.size(); ++i) {
        QString fieldName = cleanFieldNameUI(fieldNames[i]);
        QWidget* widget = formWidgets[i];
        QString value = getWidgetValue(widget);
        
        // Determinar tipo de campo basado en metadata
        if (i < currentTableFields.size()) {
            QJsonObject fieldMeta = currentTableFields[i].toObject();
            QString fieldType = fieldMeta.value("type").toString().toLower();
            
            if (fieldType == "number" || fieldType == "int" || fieldType == "integer") {
                bool ok;
                double numValue = value.toDouble(&ok);
                if (ok) {
                    record[fieldName] = numValue;
                } else {
                    record[fieldName] = value; // Guardar como string si no es número válido
                }
            } else {
                record[fieldName] = value;
            }
        } else {
            record[fieldName] = value;
        }
    }
    
    return record;
}

void FormulariosView::saveDataToJson()
{
    if (currentTableName.isEmpty() || !m_mainWindow || !m_mainWindow->catalog()) {
        qDebug() << "FormulariosView: No se puede guardar - datos insuficientes";
        return;
    }
    
    // Convertir registros a formato para el catalog
    QStringList uiFieldNames;
    for (const QString& name : fieldNames) {
        uiFieldNames.append(cleanFieldNameUI(name));
    }
    
    // Convertir QJsonObjects a QList<QStringList>
    QList<QStringList> rows;
    for (const QJsonObject& record : allRecords) {
        QStringList row;
        for (const QString& fieldName : uiFieldNames) {
            QJsonValue value = record.value(fieldName);
            QString strValue;
            if (value.isString()) {
                strValue = value.toString();
            } else if (value.isDouble()) {
                strValue = QString::number(value.toDouble());
            } else if (value.isBool()) {
                strValue = value.toBool() ? "true" : "false";
            }
            row.append(strValue);
        }
        rows.append(row);
    }
    
    // Guardar usando el sistema de catalog (igual que TableData)
    std::string err;
    bool success = m_mainWindow->catalog()->rewriteMadFromRows(
        m_mainWindow->tablesDir(),
        currentTableName.toStdString(),
        uiFieldNames,
        rows,
        &err
    );
    
    if (!success) {
        qDebug() << "FormulariosView: Error al guardar datos:" << QString::fromStdString(err);
        QMessageBox::warning(this, "Error", 
            QString("Error al guardar datos: %1").arg(QString::fromStdString(err)));
    } else {
        qDebug() << "FormulariosView: Datos guardados exitosamente";
        hasUnsavedChanges = false;
    }
}

void FormulariosView::setupRealTimeValidation(QWidget* widget, const QJsonObject& fieldMeta)
{
    QString fieldType = fieldMeta.value("type").toString().toLower();
    QString fieldName = fieldMeta.value("name").toString();
    bool allowNull = fieldMeta.value("allowNull").toBool(true);
    bool isPrimaryKey = fieldMeta.value("isPrimaryKey").toBool(false);
    
    if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
        connect(lineEdit, &QLineEdit::textChanged, [this, lineEdit, fieldType, fieldName, allowNull, isPrimaryKey](const QString &text) {
            QString value = text.trimmed();
            
            // Validar campo requerido
            if (!allowNull && !isPrimaryKey && value.isEmpty()) {
                showFieldError(lineEdit, QString("El campo '%1' es obligatorio").arg(fieldName));
                return;
            }
            
            // Validar tipo de datos
            if (!value.isEmpty()) {
                if (fieldType == "number" || fieldType == "int" || fieldType == "integer") {
                    bool ok;
                    value.toDouble(&ok);
                    if (!ok) {
                        showFieldError(lineEdit, QString("Valor incompatible para 'Entero'"));
                        return;
                    }
                }
                else if (fieldType == "email") {
                    if (!isValidEmail(value)) {
                        showFieldError(lineEdit, QString("Formato de email inválido"));
                        return;
                    }
                }
                else if (fieldType == "phone") {
                    if (!isValidPhone(value)) {
                        showFieldError(lineEdit, QString("Formato de teléfono inválido"));
                        return;
                    }
                }
                else if (fieldType == "date") {
                    if (!isValidDate(value)) {
                        showFieldError(lineEdit, QString("Formato de fecha inválido (DD/MM/YYYY)"));
                        return;
                    }
                }
            }
            
            // Si llegamos aquí, el valor es válido
            hideFieldError(lineEdit);
        });
    }
    else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this, spinBox, allowNull, isPrimaryKey, fieldName](int value) {
            if (!allowNull && !isPrimaryKey && value == 0) {
                showFieldError(spinBox, QString("El campo '%1' es obligatorio").arg(fieldName));
            } else {
                hideFieldError(spinBox);
            }
        });
    }
    else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, doubleSpinBox, allowNull, isPrimaryKey, fieldName](double value) {
            if (!allowNull && !isPrimaryKey && value == 0.0) {
                showFieldError(doubleSpinBox, QString("El campo '%1' es obligatorio").arg(fieldName));
            } else {
                hideFieldError(doubleSpinBox);
            }
        });
    }
}

void FormulariosView::showFieldError(QWidget* widget, const QString& errorMessage)
{
    QLabel* errorLabel = widget->property("errorLabel").value<QLabel*>();
    if (errorLabel) {
        errorLabel->setText(errorMessage);
        errorLabel->show();
    }
    
    // Cambiar estilo del widget a error
    QString errorStyle = "border: 2px solid #dc3545; background-color: #fff5f5;";
    widget->setStyleSheet(widget->styleSheet() + errorStyle);
}

void FormulariosView::hideFieldError(QWidget* widget)
{
    QLabel* errorLabel = widget->property("errorLabel").value<QLabel*>();
    if (errorLabel) {
        errorLabel->hide();
    }
    
    // Restaurar estilo normal del widget
    QString baseStyle = widget->styleSheet();
    baseStyle = baseStyle.remove("border: 2px solid #dc3545; background-color: #fff5f5;");
    widget->setStyleSheet(baseStyle);
}