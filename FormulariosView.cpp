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
    
    // Rediseño de botones: TODOS en una sola fila más compactos
    QWidget *buttonContainer = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(20, 15, 20, 15);
    buttonLayout->setSpacing(8); // Espaciado más pequeño entre botones
    
    // Crear TODOS los botones en una sola fila
    editRecordBtn = new QPushButton("🆕 Nuevo");
    newRecordBtn = new QPushButton("🖊️ Editar");
    deleteRecordBtn = new QPushButton("🗑️ Eliminar");
    saveRecordBtn = new QPushButton("💾 Guardar");
    firstRecordBtn = new QPushButton("|<");
    prevRecordBtn = new QPushButton("◀");
    nextRecordBtn = new QPushButton("▶");
    lastRecordBtn = new QPushButton(">|");
    
    // Estilo moderno y atractivo para todos los botones
    QString compactButtonStyle = 
        "QPushButton {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f8f9fa);"
        "color: #495057;"
        "border: 2px solid #e3f2fd;"
        "border-radius: 8px;"
        "padding: 10px 16px;"
        "font-size: 14px;"
        "font-weight: 600;"
        "min-width: 90px;"
        "max-height: 42px;"
        "box-shadow: 0 2px 4px rgba(0,0,0,0.1);"
        "}"
        "QPushButton:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e3f2fd, stop:1 #bbdefb);"
        "border-color: #2196f3;"
        "color: #1976d2;"
        "transform: translateY(-1px);"
        "box-shadow: 0 4px 8px rgba(33,150,243,0.2);"
        "}"
        "QPushButton:pressed {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #bbdefb, stop:1 #90caf9);"
        "transform: translateY(0px);"
        "box-shadow: 0 2px 4px rgba(33,150,243,0.3);"
        "}"
        "QPushButton:disabled {"
        "background: #f5f5f5;"
        "color: #bdbdbd;"
        "border-color: #e0e0e0;"
        "box-shadow: none;"
        "}";
    
    // Estilo especial para botones de navegación (más simples como antes)
    QString navButtonStyle = 
        "QPushButton {"
        "background-color: #f8f9fa;"
        "color: #495057;"
        "border: 2px solid #dee2e6;"
        "border-radius: 6px;"
        "padding: 8px;"
        "font-size: 14px;"
        "font-weight: 600;"
        "min-width: 35px;"
        "max-width: 35px;"
        "min-height: 35px;"
        "max-height: 35px;"
        "}"
        "QPushButton:hover {"
        "background-color: #e9ecef;"
        "border-color: #007bff;"
        "color: #007bff;"
        "}"
        "QPushButton:pressed {"
        "background-color: #dee2e6;"
        "}"
        "QPushButton:disabled {"
        "background-color: #e9ecef;"
        "color: #6c757d;"
        "border-color: #dee2e6;"
        "}";
    
    // Aplicar estilos
    editRecordBtn->setStyleSheet(compactButtonStyle);
    newRecordBtn->setStyleSheet(compactButtonStyle);
    deleteRecordBtn->setStyleSheet(compactButtonStyle);
    saveRecordBtn->setStyleSheet(compactButtonStyle);
    
    firstRecordBtn->setStyleSheet(navButtonStyle);
    prevRecordBtn->setStyleSheet(navButtonStyle);
    nextRecordBtn->setStyleSheet(navButtonStyle);
    lastRecordBtn->setStyleSheet(navButtonStyle);
    
    // Estado inicial de botones
    editRecordBtn->setEnabled(true);  // "Nuevo" siempre habilitado
    newRecordBtn->setEnabled(false);  // "Editar" se habilita cuando hay datos
    deleteRecordBtn->setEnabled(false);
    saveRecordBtn->setEnabled(false);
    
    // Botones de navegación inicialmente deshabilitados
    firstRecordBtn->setEnabled(false);
    prevRecordBtn->setEnabled(false);
    nextRecordBtn->setEnabled(false);
    lastRecordBtn->setEnabled(false);
    
    // Agregar TODOS los botones en una sola fila
    buttonLayout->addWidget(editRecordBtn);   // Nuevo
    buttonLayout->addWidget(newRecordBtn);    // Editar
    buttonLayout->addWidget(deleteRecordBtn); // Eliminar
    buttonLayout->addWidget(saveRecordBtn);   // Guardar
    
    // Separador visual
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet("QFrame { color: #dee2e6; margin: 5px; }");
    buttonLayout->addWidget(separator);
    
    buttonLayout->addWidget(firstRecordBtn);  // ⏮
    buttonLayout->addWidget(prevRecordBtn);   // ◀
    buttonLayout->addWidget(nextRecordBtn);   // ▶
    buttonLayout->addWidget(lastRecordBtn);   // ⏭
    
    buttonLayout->addStretch(); // Espacio flexible al final
    
    mainLayout->addWidget(buttonContainer);
    
    // Contador de registros con diseño más atractivo
    recordCounterLabel = new QLabel("", this);
    recordCounterLabel->setAlignment(Qt::AlignCenter);
    recordCounterLabel->setStyleSheet(
        "QLabel {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e3f2fd, stop:1 #f3e5f5);"
        "color: #1976d2;"
        "font-size: 16px;"
        "font-weight: 600;"
        "padding: 12px 24px;"
        "border: 2px solid #bbdefb;"
        "border-radius: 20px;"
        "margin: 8px 100px;"
        "}"
    );
    mainLayout->addWidget(recordCounterLabel);
    
    // Crear área del formulario con diseño ESPECTACULAR - TEMA BURGUNDY MÁS ROJO
    formScrollArea = new QScrollArea();
    formScrollArea->setStyleSheet(
        "QScrollArea {"
        "border: none;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "stop:0 rgba(139, 0, 0, 0.25), "
        "stop:0.25 rgba(178, 34, 34, 0.22), "
        "stop:0.5 rgba(220, 20, 60, 0.2), "
        "stop:0.75 rgba(139, 69, 19, 0.18), "
        "stop:1 rgba(128, 0, 0, 0.25));"
        "border-radius: 20px;"
        "min-height: 500px;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "background-color: transparent;"
        "}"
    );
    
    formContentWidget = new QWidget();
    formContentWidget->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 240, 240, 0.98), "
        "stop:0.5 rgba(250, 225, 225, 0.96), "
        "stop:1 rgba(245, 210, 210, 0.98));"
        "border: 2px solid rgba(139, 0, 0, 0.4);"
        "border-radius: 16px;"
        "margin: 10px 10px 30px 10px;"
        "min-height: 450px;"
        "}"
    );
    
    formLayout = new QFormLayout(formContentWidget);
    formLayout->setSpacing(25);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setContentsMargins(30, 30, 30, 30);
    
    // 🎨 ESTILO ESPECTACULAR para las etiquetas del formulario - TEMA BURGUNDY
    formContentWidget->setStyleSheet(
        "QFormLayout QLabel {"
        "font-size: 18px;"
        "font-weight: 600;"
        "color: #8B0000;"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 rgba(139, 69, 19, 0.15), "
        "stop:1 rgba(178, 34, 34, 0.12));"
        "border: 2px solid transparent;"
        "border-radius: 8px;"
        "padding: 8px 12px;"
        "margin-right: 15px;"
        "min-width: 140px;"
        "}"
        "QFormLayout QLabel:hover {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 rgba(139, 69, 19, 0.25), "
        "stop:1 rgba(178, 34, 34, 0.22));"
        "border: 2px solid rgba(178, 34, 34, 0.4);"
        "color: #B22222;"
        "}"
    );
    
    formScrollArea->setWidget(formContentWidget);
    formScrollArea->setWidgetResizable(true);
    
    // Crear widget de estado vacío con diseño atractivo
    emptyStateWidget = new QWidget();
    emptyStateWidget->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8f9fa, stop:1 #e9ecef);"
        "border: 3px dashed #bbdefb;"
        "border-radius: 16px;"
        "}"
    );
    QVBoxLayout *emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setContentsMargins(50, 50, 50, 50);
    
    QLabel *emptyStateLabel = new QLabel("📋 Selecciona una tabla");
    emptyStateLabel->setAlignment(Qt::AlignCenter);
    emptyStateLabel->setStyleSheet(
        "font-size: 22px;"
        "color: #1976d2;"
        "font-weight: 700;"
        "border: none;"
        "background: transparent;"
        "margin-bottom: 10px;"
    );
    
    QLabel *emptyStateSubtitle = new QLabel("Elige una tabla del menú desplegable para comenzar a trabajar con los formularios de datos");
    emptyStateSubtitle->setAlignment(Qt::AlignCenter);
    emptyStateSubtitle->setWordWrap(true);
    emptyStateSubtitle->setStyleSheet(
        "font-size: 16px;"
        "color: #64b5f6;"
        "border: none;"
        "background: transparent;"
        "margin-top: 15px;"
        "line-height: 1.4;"
    );
    
    emptyStateLayout->addWidget(emptyStateLabel);
    emptyStateLayout->addWidget(emptyStateSubtitle);
    
    mainLayout->addWidget(formScrollArea);
    mainLayout->addWidget(emptyStateWidget);
    mainLayout->addStretch();
    
    // Conectar señales
    connect(tableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &FormulariosView::onTableSelected);
    connect(newRecordBtn, &QPushButton::clicked, this, &FormulariosView::onNewRecordClicked);
    connect(editRecordBtn, &QPushButton::clicked, this, &FormulariosView::onEditRecordClicked);
    connect(deleteRecordBtn, &QPushButton::clicked, this, &FormulariosView::onDeleteRecordClicked);
    connect(saveRecordBtn, &QPushButton::clicked, this, &FormulariosView::onSaveRecordClicked);
    // TEMPORALMENTE COMENTADO: connect(dataTable, &QTableWidget::itemSelectionChanged, this, &FormulariosView::onRecordSelected);
    
    // Conectar señales de navegación
    connect(firstRecordBtn, &QPushButton::clicked, this, &FormulariosView::onFirstRecordClicked);
    connect(prevRecordBtn, &QPushButton::clicked, this, &FormulariosView::onPrevRecordClicked);
    connect(nextRecordBtn, &QPushButton::clicked, this, &FormulariosView::onNextRecordClicked);
    connect(lastRecordBtn, &QPushButton::clicked, this, &FormulariosView::onLastRecordClicked);
    
    // Cargar tablas
    qDebug() << "FormulariosView: Llamando loadAvailableTables() desde setupUI()";
    loadAvailableTables();
    
    // Mostrar estado vacío inicialmente
    formScrollArea->hide();
    emptyStateWidget->show();
}

void FormulariosView::loadAvailableTables()
{
    qDebug() << "FormulariosView: loadAvailableTables() iniciado";
    
    qDebug() << "FormulariosView: Punto 1 - Antes de clear()";
    
    // Bloquear señales temporalmente para evitar que se dispare onTableSelected
    tableComboBox->blockSignals(true);
    
    tableComboBox->clear();
    qDebug() << "FormulariosView: Punto 2 - Después de clear()";
    
    tableComboBox->addItem("-- Seleccionar tabla --");
    qDebug() << "FormulariosView: Punto 3 - Después de addItem()";
    
    // Desbloquear señales
    tableComboBox->blockSignals(false);
    qDebug() << "FormulariosView: Punto 3.5 - Señales desbloqueadas";
    
    if (!m_mainWindow) {
        qDebug() << "FormulariosView: MainWindow no disponible";
        return;
    }
    qDebug() << "FormulariosView: Punto 4 - MainWindow OK";
    
    // Obtener información del proyecto actual
    QString currentProject = m_mainWindow->getCurrentProjectName();
    qDebug() << "FormulariosView: Punto 5 - Proyecto actual:" << currentProject;
    
    // Usar la ruta correcta del directorio de tablas desde MainWindow
    QString tablesPath = QString::fromStdString(m_mainWindow->tablesDir());
    qDebug() << "FormulariosView: Punto 6 - tablesPath obtenido:" << tablesPath;
    
    QDir tablesDir(tablesPath);
    qDebug() << "FormulariosView: Punto 7 - QDir creado";
    
    qDebug() << "FormulariosView: Buscando tablas en:" << tablesPath;
    qDebug() << "FormulariosView: Directorio existe:" << tablesDir.exists();
    
    if (!tablesDir.exists()) {
        qDebug() << "FormulariosView: Directorio de tablas no existe:" << tablesPath;
        tableComboBox->blockSignals(false); // Desbloquear antes de return
        return;
    }
    qDebug() << "FormulariosView: Punto 8 - Directorio existe";
    
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    qDebug() << "FormulariosView: Punto 9 - entryList obtenido";
    qDebug() << "FormulariosView: Archivos .meta encontrados:" << metaFiles;
    
    for (const QString &metaFile : metaFiles) {
        qDebug() << "FormulariosView: Punto 10 - Procesando archivo:" << metaFile;
        QString tableName = metaFile;
        tableName.remove(".meta");
        qDebug() << "FormulariosView: Punto 11 - Nombre tabla:" << tableName;
        tableComboBox->addItem(tableName);
        qDebug() << "FormulariosView: Punto 12 - Tabla agregada al combobox:" << tableName;
    }
    
    // Desbloquear señales al final
    tableComboBox->blockSignals(false);
    qDebug() << "FormulariosView: Punto 12.5 - Señales finalmente desbloqueadas";
    
    if (metaFiles.isEmpty()) {
        qDebug() << "FormulariosView: No se encontraron tablas en" << tablesPath;
    } else {
        qDebug() << "FormulariosView: Se encontraron" << metaFiles.size() << "tablas";
        qDebug() << "FormulariosView: Items en combobox:" << tableComboBox->count();
    }
    qDebug() << "FormulariosView: Punto 13 - loadAvailableTables() TERMINADO";
}

void FormulariosView::onTableSelected()
{
    qDebug() << "FormulariosView: onTableSelected() INICIADO";
    
    QString selectedTable = tableComboBox->currentText();
    qDebug() << "FormulariosView: Tabla seleccionada:" << selectedTable;
    
    if (selectedTable == "-- Seleccionar tabla --" || selectedTable.isEmpty()) {
        qDebug() << "FormulariosView: Mostrando estado vacío";
        showEmptyState();
        qDebug() << "FormulariosView: Estado vacío mostrado, retornando";
        return;
    }
    
    qDebug() << "FormulariosView: Asignando currentTableName";
    currentTableName = selectedTable;
    
    qDebug() << "FormulariosView: Llamando generateFormForTable";
    generateFormForTable(selectedTable);
    
    qDebug() << "FormulariosView: Llamando loadDataFromJson";
    loadDataFromJson();  // Cargar datos existentes desde JSON
    
    qDebug() << "FormulariosView: onTableSelected() TERMINADO";
}

void FormulariosView::showEmptyState()
{
    qDebug() << "FormulariosView: showEmptyState() INICIADO";
    
    qDebug() << "FormulariosView: Ocultando formScrollArea";
    formScrollArea->hide();
    
    qDebug() << "FormulariosView: Mostrando emptyStateWidget";
    emptyStateWidget->show();
    
    // TEMPORALMENTE COMENTADO: dataTable->clear();
    // TEMPORALMENTE COMENTADO: dataTable->setRowCount(0);
    // TEMPORALMENTE COMENTADO: dataTable->setColumnCount(0);
    
    qDebug() << "FormulariosView: Deshabilitando botones";
    editRecordBtn->setEnabled(false);
    deleteRecordBtn->setEnabled(false);
    saveRecordBtn->setEnabled(false);
    
    qDebug() << "FormulariosView: showEmptyState() TERMINADO";
}

void FormulariosView::generateFormForTable(const QString& tableName)
{
    qDebug() << "FormulariosView: generateFormForTable() INICIADO para tabla:" << tableName;
    
    if (tableName.isEmpty()) {
        qDebug() << "FormulariosView: tableName está vacío, retornando";
        return;
    }
    
    qDebug() << "FormulariosView: Ocultando emptyStateWidget";
    // Ocultar estado vacío y mostrar formulario
    emptyStateWidget->hide();
    
    qDebug() << "FormulariosView: Mostrando formScrollArea";
    formScrollArea->show();
    
    qDebug() << "FormulariosView: Llamando clearForm()";
    // Limpiar formulario anterior
    clearForm();
    
    qDebug() << "FormulariosView: Preparando ruta del archivo meta";
    // Cargar metadatos de la tabla usando la ruta correcta
    QString metaPath = QString::fromStdString(m_mainWindow->tablesDir()) + "/" + tableName + ".meta";
    qDebug() << "FormulariosView: Ruta meta:" << metaPath;
    
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
    
    // Estilo ESPECTACULAR para inputs del formulario - TEMA BURGUNDY - SUPER REDONDOS
    QString inputStyle = 
        "QLineEdit, QSpinBox, QDoubleSpinBox, QDateEdit {"
        "border: 3px solid transparent;"
        "border-radius: 20px;"
        "padding: 15px 20px;"
        "font-size: 16px;"
        "font-weight: 500;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 250, 250, 0.95), "
        "stop:1 rgba(248, 240, 240, 0.9));"
        "color: #8B0000;"
        "selection-background-color: #B22222;"
        "min-height: 20px;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {"
        "border: 3px solid #B22222;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 245, 245, 1.0), "
        "stop:1 rgba(255, 235, 235, 1.0));"
        "color: #8B0000;"
        "}"
        "QLineEdit:hover, QSpinBox:hover, QDoubleSpinBox:hover, QDateEdit:hover {"
        "border: 3px solid #CD5C5C;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 248, 248, 0.98), "
        "stop:1 rgba(250, 235, 235, 0.95));"
        "}"
        "QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QDateEdit:disabled {"
        "background: rgba(245, 245, 245, 0.7);"
        "color: #9e9e9e;"
        "border: 3px solid #e0e0e0;"
        "}"
        "QCheckBox {"
        "font-size: 16px;"
        "font-weight: 500;"
        "color: #8B0000;"
        "}"
        "QCheckBox::indicator {"
        "width: 24px;"
        "height: 24px;"
        "border: 3px solid #CD5C5C;"
        "border-radius: 6px;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 250, 250, 0.9), "
        "stop:1 rgba(248, 240, 240, 0.9));"
        "}"
        "QCheckBox::indicator:checked {"
        "border: 3px solid #B22222;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(178, 34, 34, 0.2), "
        "stop:1 rgba(139, 0, 0, 0.3));"
        "image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTMuNSA4LjVMNi41IDExLjVMMTIuNSA0LjUiIHN0cm9rZT0iI0IyMjIyMiIgc3Ryb2tlLXdpZHRoPSIyIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+Cg==);"
        "}"
        "QCheckBox::indicator:hover {"
        "border: 3px solid #DC143C;"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255, 245, 245, 1.0), "
        "stop:1 rgba(255, 235, 235, 1.0));"
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
    
    // TEMPORALMENTE COMENTADO: updateDataTable();
    
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
    // TEMPORALMENTE COMENTADO: dataTable->blockSignals(true);
    // TEMPORALMENTE COMENTADO: dataTable->clearSelection();
    // TEMPORALMENTE COMENTADO: dataTable->blockSignals(false);
}

void FormulariosView::onEditRecordClicked()
{
    // Este botón funciona como "Nuevo Registro"
    clearFormInputs();
    currentRecordIndex = -1; // Indicar que estamos creando un nuevo registro
    isEditMode = true;
    
    // Calcular el siguiente número de registro
    int nextRecordNumber = allRecords.size() + 1;
    
    // Actualizar contador para mostrar el próximo registro a crear
    recordCounterLabel->setText(QString("🆕 Creando registro %1").arg(nextRecordNumber));
    
    // Si hay un campo 'id' en el formulario, pre-llenarlo con el siguiente número
    autoFillNextId();
    
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
        // TEMPORALMENTE COMENTADO: updateDataTable();
        
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
    // TEMPORALMENTE COMENTADO: updateDataTable();
    updateNavigationState();
    hasUnsavedChanges = false;
}

void FormulariosView::onRecordSelected()
{
    // TEMPORALMENTE COMENTADO: int row = dataTable->currentRow();
    int row = -1; // Temporal
    
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
    editRecordBtn->setEnabled(hasTable);  // Habilitar botón "Nuevo" cuando hay tabla
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
    // TEMPORALMENTE COMENTADO: dataTable->blockSignals(true);
    // TEMPORALMENTE COMENTADO: dataTable->selectRow(index);
    // TEMPORALMENTE COMENTADO: dataTable->blockSignals(false);
    
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
    newRecordBtn->setEnabled(hasSelection); // Editar solo si hay registro seleccionado
    
    // Actualizar botones de navegación
    if (hasRecords) {
        firstRecordBtn->setEnabled(currentRecordIndex > 0);
        prevRecordBtn->setEnabled(currentRecordIndex > 0);
        nextRecordBtn->setEnabled(currentRecordIndex < allRecords.size() - 1);
        lastRecordBtn->setEnabled(currentRecordIndex < allRecords.size() - 1);
    } else {
        firstRecordBtn->setEnabled(false);
        prevRecordBtn->setEnabled(false);
        nextRecordBtn->setEnabled(false);
        lastRecordBtn->setEnabled(false);
    }
    
    // Actualizar contador de registros
    if (hasRecords && hasSelection) {
        recordCounterLabel->setText(QString("Registro %1 de %2")
                                   .arg(currentRecordIndex + 1)
                                   .arg(allRecords.size()));
    } else if (hasRecords) {
        recordCounterLabel->setText(QString("Total: %1 registros").arg(allRecords.size()));
    } else {
        recordCounterLabel->setText("Sin registros");
    }
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

// Nuevos métodos de navegación

void FormulariosView::onFirstRecordClicked()
{
    if (!allRecords.isEmpty()) {
        goToRecord(0);
    }
}

void FormulariosView::onPrevRecordClicked()
{
    if (currentRecordIndex > 0) {
        goToRecord(currentRecordIndex - 1);
    }
}

void FormulariosView::onNextRecordClicked()
{
    if (currentRecordIndex < allRecords.size() - 1) {
        goToRecord(currentRecordIndex + 1);
    }
}

void FormulariosView::autoFillNextId()
{
    if (formWidgets.isEmpty() || fieldNames.isEmpty()) return;
    
    // Buscar si hay un campo que se llame 'id' o similar
    for (int i = 0; i < fieldNames.size(); ++i) {
        QString fieldName = fieldNames[i].toLower();
        
        // Si es un campo de ID, auto-llenar con el siguiente número
        if (fieldName.contains("id") || fieldName == "numero" || fieldName == "codigo") {
            QWidget* widget = formWidgets[i];
            
            // Calcular el siguiente ID basado en los registros existentes
            int nextId = 1;
            if (!allRecords.isEmpty()) {
                // Buscar el ID más alto existente
                int maxId = 0;
                for (const QJsonObject &record : allRecords) {
                    QString cleanFieldName = cleanFieldNameUI(fieldNames[i]);
                    QJsonValue value = record.value(cleanFieldName);
                    int currentId = value.toString().toInt();
                    if (currentId > maxId) {
                        maxId = currentId;
                    }
                }
                nextId = maxId + 1;
            }
            
            // Llenar el widget con el siguiente ID
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                lineEdit->setText(QString::number(nextId));
            }
            else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
                spinBox->setValue(nextId);
            }
            
            break; // Solo llenar el primer campo ID encontrado
        }
    }
}

void FormulariosView::onLastRecordClicked()
{
    if (!allRecords.isEmpty()) {
        goToRecord(allRecords.size() - 1);
    }
}