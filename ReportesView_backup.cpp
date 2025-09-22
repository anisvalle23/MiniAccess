#include "ReportesView.h"
#include "mainwindow.h"
#include "ThemeTokens.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDate>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCheckBox>
#include <QDialog>
#include <QScrollArea>
#include <QFile>
#include <QDir>
#include <QTextStream>

ReportesView::ReportesView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
    , currentSelectedReport("")
    , headerShadow(nullptr)
    , toolbarShadow(nullptr)
    , fadeAnimation(nullptr)
{
    setupUI();
    styleComponents();
    loadReports();
}

ReportesView::~ReportesView()
{
    // Cleanup será manejado automáticamente por Qt
}

void ReportesView::setupUI()
{
    // Layout principal sin márgenes para ocupar toda la ventana
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    createHeaderSection();
    createReportContent();
    
    // Agregar widgets al layout principal
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(reportContentWidget, 1); // El contenido toma el espacio restante
}

void ReportesView::createHeaderSection()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(120);
    headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(40, 30, 40, 20);
    headerLayout->setSpacing(10);
    
    // Título principal con estilo moderno
    titleLabel = new QLabel("Reportes");
    titleLabel->setFont(QFont("Inter", 32, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignLeft);
    
    // Descripción con mejor espaciado
    descriptionLabel = new QLabel("Visualiza y analiza la información de tus datos");
    descriptionLabel->setFont(QFont("Inter", 16, QFont::Normal));
    descriptionLabel->setAlignment(Qt::AlignLeft);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
}

void ReportesView::createReportContent()
{
    reportContentWidget = new QWidget();
    reportContentLayout = new QVBoxLayout(reportContentWidget);
    reportContentLayout->setContentsMargins(40, 20, 40, 40);
    reportContentLayout->setSpacing(30);
    
    // Crear tarjeta principal del reporte
    createReportCard();
    
    reportContentLayout->addWidget(reportCardWidget);
    reportContentLayout->addStretch();
}

void ReportesView::createReportCard()
{
    reportCardWidget = new QWidget();
    reportCardWidget->setMinimumHeight(400);
    reportCardLayout = new QVBoxLayout(reportCardWidget);
    reportCardLayout->setContentsMargins(30, 25, 30, 30);
    reportCardLayout->setSpacing(20);
    
    // Título del reporte
    reportTitleLabel = new QLabel("Reporte de Clientes por Ciudad");
    reportTitleLabel->setFont(QFont("Inter", 24, QFont::Bold));
    reportTitleLabel->setAlignment(Qt::AlignLeft);
    
    // Contenedor para los datos del reporte
    reportDataWidget = new QWidget();
    reportDataLayout = new QVBoxLayout(reportDataWidget);
    reportDataLayout->setContentsMargins(0, 0, 0, 0);
    reportDataLayout->setSpacing(15);
    
    // Crear elementos de datos individuales
    createReportDataItems();
    
    // Agregar total al final
    createTotalSection();
    
    reportCardLayout->addWidget(reportTitleLabel);
    reportCardLayout->addWidget(reportDataWidget);
    reportCardLayout->addStretch();
}

void ReportesView::createReportDataItems()
{
    // Datos de ejemplo basados en la imagen
    QStringList cities = {"San Pedro Sula", "Tegucigalpa"};
    QStringList clientCounts = {"25", "15"};
    
    for (int i = 0; i < cities.size(); ++i) {
        createDataRow(cities[i], clientCounts[i]);
    }
}

void ReportesView::createDataRow(const QString& city, const QString& count)
{
    QWidget* rowWidget = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 8, 0, 8);
    rowLayout->setSpacing(0);
    
    // Nombre de la ciudad
    QLabel* cityLabel = new QLabel(city);
    cityLabel->setFont(QFont("Inter", 16, QFont::Medium));
    
    // Contador de clientes
    QLabel* countLabel = new QLabel(count);
    countLabel->setFont(QFont("Inter", 16, QFont::Normal));
    countLabel->setAlignment(Qt::AlignRight);
    
    rowLayout->addWidget(cityLabel);
    rowLayout->addStretch();
    rowLayout->addWidget(countLabel);
    
    reportDataLayout->addWidget(rowWidget);
}

void ReportesView::createTotalSection()
{
    // Línea separadora
    QFrame* separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);
    separatorLine->setFixedHeight(1);
    
    // Total
    QWidget* totalWidget = new QWidget();
    QHBoxLayout* totalLayout = new QHBoxLayout(totalWidget);
    totalLayout->setContentsMargins(0, 15, 0, 0);
    totalLayout->setSpacing(0);
    
    QLabel* totalLabel = new QLabel("Total clientes:");
    totalLabel->setFont(QFont("Inter", 18, QFont::Bold));
    
    QLabel* totalCountLabel = new QLabel("40");
    totalCountLabel->setFont(QFont("Inter", 18, QFont::Bold));
    totalCountLabel->setAlignment(Qt::AlignRight);
    
    totalLayout->addWidget(totalLabel);
    totalLayout->addStretch();
    totalLayout->addWidget(totalCountLabel);
    
    reportDataLayout->addWidget(separatorLine);
    reportDataLayout->addWidget(totalWidget);
}
{
    toolbarWidget = new QWidget();
    toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(12);
    
    // Botones de acción
    createReportBtn = new QPushButton("Crear Reporte");
    createReportBtn->setFixedHeight(40);
    createReportBtn->setFont(QFont("Inter", 12, QFont::Medium));
    createReportBtn->setCursor(Qt::PointingHandCursor);
    
    editReportBtn = new QPushButton("Editar");
    editReportBtn->setFixedHeight(40);
    editReportBtn->setFont(QFont("Inter", 12, QFont::Normal));
    editReportBtn->setCursor(Qt::PointingHandCursor);
    editReportBtn->setEnabled(false);
    
    deleteReportBtn = new QPushButton("Eliminar");
    deleteReportBtn->setFixedHeight(40);
    deleteReportBtn->setFont(QFont("Inter", 12, QFont::Normal));
    deleteReportBtn->setCursor(Qt::PointingHandCursor);
    deleteReportBtn->setEnabled(false);
    
    generateReportBtn = new QPushButton("Generar");
    generateReportBtn->setFixedHeight(40);
    generateReportBtn->setFont(QFont("Inter", 12, QFont::Medium));
    generateReportBtn->setCursor(Qt::PointingHandCursor);
    generateReportBtn->setEnabled(false);
    
    exportReportBtn = new QPushButton("Exportar");
    exportReportBtn->setFixedHeight(40);
    exportReportBtn->setFont(QFont("Inter", 12, QFont::Normal));
    exportReportBtn->setCursor(Qt::PointingHandCursor);
    exportReportBtn->setEnabled(false);
    
    refreshBtn = new QPushButton("Actualizar");
    refreshBtn->setFixedHeight(40);
    refreshBtn->setFont(QFont("Inter", 12, QFont::Normal));
    refreshBtn->setCursor(Qt::PointingHandCursor);
    
    // Agregar botones al toolbar
    toolbarLayout->addWidget(createReportBtn);
    toolbarLayout->addWidget(editReportBtn);
    toolbarLayout->addWidget(deleteReportBtn);
    toolbarLayout->addWidget(generateReportBtn);
    toolbarLayout->addWidget(exportReportBtn);
    toolbarLayout->addStretch(); // Empuja el botón refresh hacia la derecha
    toolbarLayout->addWidget(refreshBtn);
    
    // Conectar señales
    connect(createReportBtn, &QPushButton::clicked, this, &ReportesView::onCreateReportClicked);
    connect(editReportBtn, &QPushButton::clicked, this, &ReportesView::onEditReportClicked);
    connect(deleteReportBtn, &QPushButton::clicked, this, &ReportesView::onDeleteReportClicked);
    connect(generateReportBtn, &QPushButton::clicked, this, &ReportesView::onGenerateReportClicked);
    connect(exportReportBtn, &QPushButton::clicked, this, &ReportesView::onExportReportClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &ReportesView::refreshView);
}

void ReportesView::createReportFilters()
{
    filtersWidget = new QWidget();
    filtersLayout = new QHBoxLayout(filtersWidget);
    filtersLayout->setContentsMargins(0, 0, 0, 0);
    filtersLayout->setSpacing(15);
    
    // Tipo de reporte
    reportTypeLabel = new QLabel("Tipo:");
    reportTypeLabel->setFont(QFont("Inter", 12, QFont::Medium));
    
    reportTypeCombo = new QComboBox();
    reportTypeCombo->setFixedHeight(35);
    reportTypeCombo->setFont(QFont("Inter", 11, QFont::Normal));
    reportTypeCombo->addItems({"Todos los tipos", "Resumen de Datos", "Análisis Detallado", "Estadísticas", "Personalizado"});
    
    // Filtros de fecha
    dateFromLabel = new QLabel("Desde:");
    dateFromLabel->setFont(QFont("Inter", 12, QFont::Medium));
    
    dateFromEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    dateFromEdit->setFixedHeight(35);
    dateFromEdit->setFont(QFont("Inter", 11, QFont::Normal));
    dateFromEdit->setCalendarPopup(true);
    
    dateToLabel = new QLabel("Hasta:");
    dateToLabel->setFont(QFont("Inter", 12, QFont::Medium));
    
    dateToEdit = new QDateEdit(QDate::currentDate());
    dateToEdit->setFixedHeight(35);
    dateToEdit->setFont(QFont("Inter", 11, QFont::Normal));
    dateToEdit->setCalendarPopup(true);
    
    applyFiltersBtn = new QPushButton("Aplicar Filtros");
    applyFiltersBtn->setFixedHeight(35);
    applyFiltersBtn->setFont(QFont("Inter", 11, QFont::Medium));
    applyFiltersBtn->setCursor(Qt::PointingHandCursor);
    
    // Agregar controles al layout
    filtersLayout->addWidget(reportTypeLabel);
    filtersLayout->addWidget(reportTypeCombo);
    filtersLayout->addWidget(dateFromLabel);
    filtersLayout->addWidget(dateFromEdit);
    filtersLayout->addWidget(dateToLabel);
    filtersLayout->addWidget(dateToEdit);
    filtersLayout->addWidget(applyFiltersBtn);
    filtersLayout->addStretch(); // Empuja todo hacia la izquierda
    
    // Conectar señales
    connect(reportTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReportesView::onReportTypeChanged);
    connect(applyFiltersBtn, &QPushButton::clicked, this, &ReportesView::generatePreview);
}

void ReportesView::createReportsList()
{
    // Lista de reportes (lado izquierdo)
    reportsListWidget = new QWidget();
    reportsListWidget->setMinimumWidth(280);
    reportsListWidget->setMaximumWidth(400);
    reportsListLayout = new QVBoxLayout(reportsListWidget);
    reportsListLayout->setContentsMargins(0, 0, 0, 0);
    reportsListLayout->setSpacing(10);
    
    reportsListLabel = new QLabel("Reportes Disponibles");
    reportsListLabel->setFont(QFont("Inter", 14, QFont::Medium));
    
    reportsList = new QListWidget();
    reportsList->setFont(QFont("Inter", 12, QFont::Normal));
    
    reportsListLayout->addWidget(reportsListLabel);
    reportsListLayout->addWidget(reportsList);
    
    // Conectar selección
    connect(reportsList, &QListWidget::itemSelectionChanged, this, &ReportesView::onReportSelected);
}

void ReportesView::createReportViewer()
{
    // Área del visualizador de reportes (lado derecho)
    reportViewerWidget = new QWidget();
    reportViewerLayout = new QVBoxLayout(reportViewerWidget);
    reportViewerLayout->setContentsMargins(0, 0, 0, 0);
    reportViewerLayout->setSpacing(15);
    
    // Stacked widget para diferentes vistas
    reportStackedWidget = new QStackedWidget();
    
    // Estado vacío/bienvenida
    emptyStateWidget = new QWidget();
    emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setAlignment(Qt::AlignCenter);
    
    emptyStateLabel = new QLabel("Selecciona un reporte");
    emptyStateLabel->setFont(QFont("Inter", 18, QFont::Medium));
    emptyStateLabel->setAlignment(Qt::AlignCenter);
    
    emptyStateDescription = new QLabel("Selecciona un reporte de la lista para visualizarlo o crear uno nuevo");
    emptyStateDescription->setFont(QFont("Inter", 12, QFont::Normal));
    emptyStateDescription->setAlignment(Qt::AlignCenter);
    emptyStateDescription->setWordWrap(true);
    
    emptyStateLayout->addWidget(emptyStateLabel);
    emptyStateLayout->addWidget(emptyStateDescription);
    
    // Vista de reporte
    reportDisplayWidget = new QWidget();
    reportDisplayLayout = new QVBoxLayout(reportDisplayWidget);
    reportDisplayLayout->setContentsMargins(0, 0, 0, 0);
    reportDisplayLayout->setSpacing(15);
    
    // Título del reporte
    reportTitleLabel = new QLabel();
    reportTitleLabel->setFont(QFont("Inter", 16, QFont::Bold));
    reportTitleLabel->setAlignment(Qt::AlignLeft);
    
    // Área de scroll para el contenido del reporte
    reportScrollArea = new QScrollArea();
    reportScrollArea->setWidgetResizable(true);
    reportScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    reportScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    reportContentWidget = new QWidget();
    reportContentLayout = new QVBoxLayout(reportContentWidget);
    reportContentLayout->setContentsMargins(10, 10, 10, 10);
    reportContentLayout->setSpacing(20);
    
    // Tabla para mostrar datos
    reportTable = new QTableWidget();
    reportTable->setAlternatingRowColors(true);
    reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    reportTable->horizontalHeader()->setStretchLastSection(true);
    reportTable->setMinimumHeight(300);
    
    // Área de resumen/texto
    reportSummary = new QTextEdit();
    reportSummary->setMaximumHeight(150);
    reportSummary->setReadOnly(true);
    reportSummary->setFont(QFont("Inter", 11, QFont::Normal));
    
    reportContentLayout->addWidget(reportTable);
    reportContentLayout->addWidget(reportSummary);
    reportContentLayout->addStretch();
    
    reportScrollArea->setWidget(reportContentWidget);
    
    reportDisplayLayout->addWidget(reportTitleLabel);
    reportDisplayLayout->addWidget(reportScrollArea);
    
    // Agregar widgets al stacked widget
    reportStackedWidget->addWidget(emptyStateWidget);
    reportStackedWidget->addWidget(reportDisplayWidget);
    
    reportViewerLayout->addWidget(reportStackedWidget);
}

void ReportesView::styleComponents()
{
    // El styling se aplicará en updateTheme()
    updateTheme();
}

void ReportesView::loadReports()
{
    // Para el nuevo diseño, simplemente mostramos un reporte de ejemplo
    // En una implementación real, aquí cargarías los datos desde la base de datos
    
    // Actualizar el título del reporte
    reportTitleLabel->setText("Reporte de Clientes por Ciudad");
    
    // Limpiar datos anteriores
    QLayoutItem* item;
    while ((item = reportDataLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // Recrear los elementos de datos
    createReportDataItems();
    createTotalSection();
    
    // Aplicar el tema actualizado
    updateTheme();
}

void ReportesView::updateReportsList()
{
    loadReports();
}

void ReportesView::generatePreview()
{
    if (currentSelectedReport.isEmpty() || !m_mainWindow || !m_mainWindow->catalog()) {
        reportStackedWidget->setCurrentWidget(emptyStateWidget);
        return;
    }
    
    try {
        // Determinar el tipo de reporte
        if (currentSelectedReport.startsWith("Reporte de ")) {
            // Reporte individual de tabla
            QString tableName = currentSelectedReport.mid(11); // Quitar "Reporte de "
            generateSingleTableReport(tableName.toStdString());
        } else if (currentSelectedReport == "Resumen General de Datos") {
            generateSummaryReport();
        } else if (currentSelectedReport == "Reporte Completo del Proyecto") {
            generateCompleteReport();
        }
        
        reportStackedWidget->setCurrentWidget(reportDisplayWidget);
        
    } catch (const std::exception& e) {
        qDebug() << "ReportesView: Error al generar preview:" << e.what();
        reportSummary->setText(QString("Error al generar el reporte: %1").arg(e.what()));
        reportStackedWidget->setCurrentWidget(reportDisplayWidget);
    }
}

void ReportesView::generateSingleTableReport(const std::string& tableName)
{
    if (!m_mainWindow || !m_mainWindow->catalog()) return;
    
    qDebug() << "ReportesView: Generando reporte para tabla:" << QString::fromStdString(tableName);
    
    // Leer metadatos de la tabla desde archivo .meta
    QJsonObject tableMeta = readTableMetadata(tableName);
    if (tableMeta.isEmpty()) {
        reportSummary->setText("No se pudo encontrar la tabla especificada.");
        return;
    }
    
    QJsonArray fields = tableMeta["fields"].toArray();
    if (fields.isEmpty()) {
        reportSummary->setText("La tabla no tiene campos definidos.");
        return;
    }
    
    // Leer datos de la tabla
    std::string err;
    auto records = m_mainWindow->catalog()->readAllRecordsJson(
        m_mainWindow->tablesDir(), tableName, &err);
    
    if (!err.empty()) {
        reportSummary->setText(QString("Error al leer datos: %1").arg(QString::fromStdString(err)));
        return;
    }
    
    // Configurar la tabla
    reportTable->setRowCount(records.size());
    reportTable->setColumnCount(fields.size());
    
    // Configurar headers
    QStringList headers;
    for (const auto& fieldValue : fields) {
        QJsonObject field = fieldValue.toObject();
        headers << field["name"].toString();
    }
    reportTable->setHorizontalHeaderLabels(headers);
    
    // Llenar datos
    for (int row = 0; row < records.size(); ++row) {
        try {
            QJsonDocument doc = QJsonDocument::fromJson(records[row].c_str());
            QJsonObject obj = doc.object();
            
            for (int col = 0; col < fields.size(); ++col) {
                QJsonObject field = fields[col].toObject();
                QString fieldName = field["name"].toString();
                QString value = obj[fieldName].toVariant().toString();
                reportTable->setItem(row, col, new QTableWidgetItem(value));
            }
        } catch (...) {
            qDebug() << "Error procesando registro" << row;
        }
    }
    
    // Generar resumen
    QString summary = QString(
        "Reporte de la tabla '%1'\n\n"
        "Número total de registros: %2\n"
        "Número de campos: %3\n\n"
        "Campos disponibles:\n"
    ).arg(QString::fromStdString(tableName))
     .arg(records.size())
     .arg(fields.size());
    
    for (const auto& fieldValue : fields) {
        QJsonObject field = fieldValue.toObject();
        QString fieldName = field["name"].toString();
        QString fieldType = field["type"].toString();
        
        if (fieldType == "number") {
            fieldType += QString(" (%1)").arg(field["numberKind"].toString());
        }
        
        QString extras = "";
        if (field["isPrimaryKey"].toBool()) extras += " [PK]";
        if (field["isUnique"].toBool()) extras += " [Unique]";
        if (!field["allowNull"].toBool()) extras += " [Required]";
        
        summary += QString("• %1 (%2)%3\n")
                   .arg(fieldName)
                   .arg(fieldType)
                   .arg(extras);
    }
    
    reportSummary->setText(summary);
}

QJsonObject ReportesView::readTableMetadata(const std::string& tableName)
{
    QString metaPath = QString::fromStdString(m_mainWindow->tablesDir()) + "/" + 
                       QString::fromStdString(tableName) + ".meta";
    
    QFile metaFile(metaPath);
    if (!metaFile.open(QIODevice::ReadOnly)) {
        qDebug() << "ReportesView: No se pudo abrir archivo meta:" << metaPath;
        return QJsonObject();
    }
    
    QByteArray metaData = metaFile.readAll();
    metaFile.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(metaData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "ReportesView: Error al parsear JSON meta:" << parseError.errorString();
        return QJsonObject();
    }
    
    return doc.object();
}

void ReportesView::generateSummaryReport()
{
    if (!m_mainWindow || !m_mainWindow->catalog()) return;
    
    // Obtener directorio de tablas
    QDir tablesDir(QString::fromStdString(m_mainWindow->tablesDir()));
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    
    // Configurar tabla con resumen por tabla
    reportTable->setRowCount(metaFiles.size());
    reportTable->setColumnCount(3);
    reportTable->setHorizontalHeaderLabels({"Tabla", "Registros", "Campos"});
    
    QString summary = "Resumen General del Proyecto\n\n";
    int totalRecords = 0;
    
    for (int i = 0; i < metaFiles.size(); ++i) {
        QString tableName = metaFiles[i];
        tableName.remove(".meta");
        
        // Contar registros
        std::string err;
        auto records = m_mainWindow->catalog()->readAllRecordsJson(
            m_mainWindow->tablesDir(), tableName.toStdString(), &err);
        
        int recordCount = err.empty() ? records.size() : 0;
        totalRecords += recordCount;
        
        // Contar campos
        QJsonObject tableMeta = readTableMetadata(tableName.toStdString());
        QJsonArray fields = tableMeta["fields"].toArray();
        int fieldCount = fields.size();
        
        reportTable->setItem(i, 0, new QTableWidgetItem(tableName));
        reportTable->setItem(i, 1, new QTableWidgetItem(QString::number(recordCount)));
        reportTable->setItem(i, 2, new QTableWidgetItem(QString::number(fieldCount)));
    }
    
    summary += QString("Total de tablas: %1\n").arg(metaFiles.size());
    summary += QString("Total de registros: %1\n\n").arg(totalRecords);
    summary += "Este resumen muestra la información básica de todas las tablas en el proyecto.";
    
    reportSummary->setText(summary);
}

void ReportesView::generateCompleteReport()
{
    if (!m_mainWindow || !m_mainWindow->catalog()) return;
    
    // Obtener directorio de tablas
    QDir tablesDir(QString::fromStdString(m_mainWindow->tablesDir()));
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    
    // Para el reporte completo, mostraremos solo la primera tabla con todos sus datos
    // y un resumen de las demás
    if (!metaFiles.empty()) {
        QString firstTableName = metaFiles[0];
        firstTableName.remove(".meta");
        
        generateSingleTableReport(firstTableName.toStdString());
        
        QString currentSummary = reportSummary->toPlainText();
        currentSummary += "\n\n--- Otras tablas en el proyecto ---\n";
        
        for (int i = 1; i < metaFiles.size(); ++i) {
            QString tableName = metaFiles[i];
            tableName.remove(".meta");
            
            std::string err;
            auto records = m_mainWindow->catalog()->readAllRecordsJson(
                m_mainWindow->tablesDir(), tableName.toStdString(), &err);
            
            currentSummary += QString("• %1: %2 registros\n")
                              .arg(tableName)
                              .arg(err.empty() ? records.size() : 0);
        }
        
        reportSummary->setText(currentSummary);
    }
}

void ReportesView::updateTheme()
{
    bool isDark = ThemeManager::instance().isDark();
    
    // Colores principales del nuevo diseño
    QString primaryColor = "#E53E3E"; // Rojo principal como en la imagen
    QString bgColor = isDark ? "#1a1a1a" : "#ffffff";
    QString cardBgColor = isDark ? "#2d2d2d" : "#ffffff";
    QString textColor = isDark ? "#ffffff" : "#333333";
    QString secondaryTextColor = isDark ? "#b3b3b3" : "#666666";
    QString borderColor = isDark ? "#404040" : "#e8e8e8";
    QString shadowColor = isDark ? "rgba(0,0,0,0.3)" : "rgba(0,0,0,0.1)";
    
    // Estilo del widget principal con gradiente sutil
    QString mainWidgetStyle = QString(
        "ReportesView {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 %1, stop:1 %2);"
        "}"
    ).arg(bgColor).arg(isDark ? "#0f0f0f" : "#f8f9fa");
    
    setStyleSheet(mainWidgetStyle);
    
    // Estilo del header con el color primario
    QString headerStyle = QString(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 %1, stop:1 %2);"
        "    border: none;"
        "}"
    ).arg(primaryColor).arg("#C53030"); // Gradiente rojo
    
    headerWidget->setStyleSheet(headerStyle);
    
    // Estilo del título en blanco para contrastar con el fondo rojo
    titleLabel->setStyleSheet("color: white; background: transparent;");
    descriptionLabel->setStyleSheet("color: rgba(255,255,255,0.9); background: transparent;");
    
    // Estilo de la tarjeta principal con sombra moderna
    QString cardStyle = QString(
        "QWidget {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 16px;"
        "    padding: 0px;"
        "}"
    ).arg(cardBgColor).arg(borderColor);
    
    reportCardWidget->setStyleSheet(cardStyle);
    
    // Aplicar sombra a la tarjeta
    QGraphicsDropShadowEffect *cardShadow = new QGraphicsDropShadowEffect();
    cardShadow->setBlurRadius(20);
    cardShadow->setXOffset(0);
    cardShadow->setYOffset(4);
    cardShadow->setColor(QColor(shadowColor));
    reportCardWidget->setGraphicsEffect(cardShadow);
    
    // Estilo del título del reporte
    reportTitleLabel->setStyleSheet(QString(
        "color: %1; background: transparent; padding: 0px;"
    ).arg(textColor));
    
    // Estilo para las filas de datos
    QString dataRowStyle = QString(
        "QLabel { color: %1; background: transparent; }"
    ).arg(textColor);
    
    // Aplicar estilo a todos los labels de datos
    QList<QLabel*> dataLabels = reportDataWidget->findChildren<QLabel*>();
    for (QLabel* label : dataLabels) {
        label->setStyleSheet(dataRowStyle);
    }
    
    // Estilo para la línea separadora
    QList<QFrame*> separators = reportDataWidget->findChildren<QFrame*>();
    for (QFrame* separator : separators) {
        separator->setStyleSheet(QString(
            "QFrame { background-color: %1; border: none; }"
        ).arg(borderColor));
    }
}

void ReportesView::refreshView()
{
    qDebug() << "ReportesView: Actualizando vista...";
    loadReports();
}

// Slots para los botones
void ReportesView::onCreateReportClicked()
{
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        QMessageBox::warning(this, "Reportes", "No hay un proyecto abierto.");
        return;
    }
    
    // Obtener directorio de tablas
    QDir tablesDir(QString::fromStdString(m_mainWindow->tablesDir()));
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    
    if (metaFiles.isEmpty()) {
        QMessageBox::information(this, "Reportes", "No hay tablas disponibles para crear reportes.");
        return;
    }
    
    // Crear lista de nombres de tablas
    QStringList tableNames;
    for (const QString &metaFile : metaFiles) {
        QString tableName = metaFile;
        tableName.remove(".meta");
        tableNames << tableName;
    }
    
    bool ok;
    QString selectedTable = QInputDialog::getItem(this, 
                                                 "Crear Reporte",
                                                 "Selecciona la tabla para el reporte:",
                                                 tableNames, 
                                                 0, 
                                                 false, 
                                                 &ok);
    
    if (ok && !selectedTable.isEmpty()) {
        QString reportName = QString("Reporte de %1").arg(selectedTable);
        
        // Verificar si ya existe
        bool exists = false;
        for (int i = 0; i < reportsList->count(); ++i) {
            if (reportsList->item(i)->text() == reportName) {
                exists = true;
                break;
            }
        }
        
        if (exists) {
            QMessageBox::information(this, "Reportes", 
                                   QString("Ya existe un reporte para la tabla '%1'.").arg(selectedTable));
        } else {
            reportsList->addItem(reportName);
            QMessageBox::information(this, "Reportes", 
                                   QString("Reporte para '%1' creado exitosamente.").arg(selectedTable));
        }
    }
}

void ReportesView::onReportSelected()
{
    auto selectedItems = reportsList->selectedItems();
    if (!selectedItems.isEmpty()) {
        currentSelectedReport = selectedItems.first()->text();
        editReportBtn->setEnabled(true);
        deleteReportBtn->setEnabled(true);
        generateReportBtn->setEnabled(true);
        exportReportBtn->setEnabled(true);
        
        reportTitleLabel->setText(currentSelectedReport);
        
        qDebug() << "ReportesView: Reporte seleccionado:" << currentSelectedReport;
        
        // Generar preview automáticamente
        generatePreview();
    } else {
        currentSelectedReport = "";
        editReportBtn->setEnabled(false);
        deleteReportBtn->setEnabled(false);
        generateReportBtn->setEnabled(false);
        exportReportBtn->setEnabled(false);
        reportStackedWidget->setCurrentWidget(emptyStateWidget);
    }
}

void ReportesView::onEditReportClicked()
{
    if (currentSelectedReport.isEmpty()) return;
    
    if (!currentSelectedReport.startsWith("Reporte de ")) {
        QMessageBox::information(this, "Reportes", "Solo se pueden editar reportes individuales de tabla.");
        return;
    }
    
    QString tableName = currentSelectedReport.mid(11); // Quitar "Reporte de "
    
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        QMessageBox::warning(this, "Reportes", "No hay un proyecto abierto.");
        return;
    }
    
    // Obtener metadatos de la tabla
    QJsonObject tableMeta = readTableMetadata(tableName.toStdString());
    if (tableMeta.isEmpty()) {
        QMessageBox::warning(this, "Reportes", "No se pudo encontrar la tabla especificada.");
        return;
    }
    
    QJsonArray fields = tableMeta["fields"].toArray();
    if (fields.isEmpty()) {
        QMessageBox::warning(this, "Reportes", "La tabla no tiene campos definidos.");
        return;
    }
    
    // Crear diálogo de selección de campos
    QDialog dialog(this);
    dialog.setWindowTitle(QString("Editar Reporte - %1").arg(tableName));
    dialog.setModal(true);
    dialog.resize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *label = new QLabel("Selecciona los campos a incluir en el reporte:");
    label->setFont(QFont("Inter", 12, QFont::Medium));
    layout->addWidget(label);
    
    // Lista de checkboxes para cada campo
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *checkboxWidget = new QWidget();
    QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxWidget);
    
    QList<QCheckBox*> checkboxes;
    for (const auto& fieldValue : fields) {
        QJsonObject field = fieldValue.toObject();
        QCheckBox *cb = new QCheckBox();
        cb->setChecked(true); // Por defecto todos seleccionados
        cb->setFont(QFont("Inter", 11, QFont::Normal));
        
        // Agregar información del tipo
        QString fieldName = field["name"].toString();
        QString fieldType = field["type"].toString();
        if (fieldType == "number") {
            fieldType += QString(" (%1)").arg(field["numberKind"].toString());
        }
        
        QString extras = "";
        if (field["isPrimaryKey"].toBool()) extras += " [PK]";
        if (field["isUnique"].toBool()) extras += " [Unique]";
        if (!field["allowNull"].toBool()) extras += " [Required]";
        
        cb->setText(QString("%1 (%2)%3").arg(fieldName).arg(fieldType).arg(extras));
        
        checkboxes.append(cb);
        checkboxLayout->addWidget(cb);
    }
    
    scrollArea->setWidget(checkboxWidget);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    
    // Botones
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *selectAllBtn = new QPushButton("Seleccionar Todo");
    QPushButton *deselectAllBtn = new QPushButton("Deseleccionar Todo");
    QPushButton *okBtn = new QPushButton("Aplicar");
    QPushButton *cancelBtn = new QPushButton("Cancelar");
    
    buttonLayout->addWidget(selectAllBtn);
    buttonLayout->addWidget(deselectAllBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    
    layout->addLayout(buttonLayout);
    
    // Conectar botones
    connect(selectAllBtn, &QPushButton::clicked, [&checkboxes]() {
        for (auto cb : checkboxes) cb->setChecked(true);
    });
    
    connect(deselectAllBtn, &QPushButton::clicked, [&checkboxes]() {
        for (auto cb : checkboxes) cb->setChecked(false);
    });
    
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // Aplicar estilos
    QString buttonStyle = QString(
        "QPushButton { background-color: #0078d4; color: white; border: none; "
        "border-radius: 4px; padding: 8px 16px; font-weight: 500; }"
        "QPushButton:hover { background-color: #106ebe; }"
    );
    
    okBtn->setStyleSheet(buttonStyle);
    selectAllBtn->setStyleSheet(buttonStyle);
    
    QString secondaryStyle = QString(
        "QPushButton { background-color: transparent; color: #0078d4; border: 1px solid #0078d4; "
        "border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #f5f5f5; }"
    );
    
    deselectAllBtn->setStyleSheet(secondaryStyle);
    cancelBtn->setStyleSheet(secondaryStyle);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Verificar que al menos un campo esté seleccionado
        bool hasSelection = false;
        for (auto cb : checkboxes) {
            if (cb->isChecked()) {
                hasSelection = true;
                break;
            }
        }
        
        if (!hasSelection) {
            QMessageBox::warning(this, "Reportes", "Debe seleccionar al menos un campo.");
            return;
        }
        
        // Regenerar el reporte con los campos seleccionados
        generatePreviewWithSelectedFields(tableName.toStdString(), checkboxes, tableMeta);
        
        QMessageBox::information(this, "Reportes", "Configuración del reporte actualizada.");
    }
}

void ReportesView::onDeleteReportClicked()
{
    if (!currentSelectedReport.isEmpty()) {
        int ret = QMessageBox::question(this, "Eliminar Reporte", 
                                       QString("¿Estás seguro de que quieres eliminar el reporte '%1'?").arg(currentSelectedReport),
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            qDebug() << "ReportesView: Eliminando reporte:" << currentSelectedReport;
            // TODO: Implementar eliminación de reportes
            QMessageBox::information(this, "Reportes", "Función de eliminar reporte será implementada próximamente.");
        }
    }
}

void ReportesView::onGenerateReportClicked()
{
    if (!currentSelectedReport.isEmpty()) {
        qDebug() << "ReportesView: Generando reporte:" << currentSelectedReport;
        generatePreview();
        QMessageBox::information(this, "Reportes", QString("Reporte '%1' generado exitosamente.").arg(currentSelectedReport));
    }
}

void ReportesView::onExportReportClicked()
{
    if (!currentSelectedReport.isEmpty()) {
        QStringList formats;
        formats << "Archivo CSV (*.csv)" << "Archivo de Texto (*.txt)" << "Archivo HTML (*.html)";
        
        QString fileName = QFileDialog::getSaveFileName(this, 
                                                       "Exportar Reporte", 
                                                       currentSelectedReport + ".csv",
                                                       formats.join(";;"));
        
        if (!fileName.isEmpty()) {
            qDebug() << "ReportesView: Exportando reporte a:" << fileName;
            
            if (fileName.endsWith(".csv")) {
                exportToCSV(fileName);
            } else if (fileName.endsWith(".html")) {
                exportToHTML(fileName);
            } else {
                exportToText(fileName);
            }
        }
    }
}

void ReportesView::onReportTypeChanged()
{
    qDebug() << "ReportesView: Tipo de reporte cambiado a:" << reportTypeCombo->currentText();
    // TODO: Filtrar reportes por tipo
}

void ReportesView::generatePreviewWithSelectedFields(const std::string& tableName, 
                                                    const QList<QCheckBox*>& checkboxes, 
                                                    const QJsonObject& tableMeta)
{
    if (!m_mainWindow || !m_mainWindow->catalog()) return;
    
    QJsonArray fields = tableMeta["fields"].toArray();
    
    // Obtener campos seleccionados
    std::vector<int> selectedFieldIndices;
    QStringList selectedHeaders;
    
    for (int i = 0; i < checkboxes.size() && i < fields.size(); ++i) {
        if (checkboxes[i]->isChecked()) {
            selectedFieldIndices.push_back(i);
            QJsonObject field = fields[i].toObject();
            selectedHeaders << field["name"].toString();
        }
    }
    
    if (selectedFieldIndices.empty()) return;
    
    // Leer datos de la tabla
    std::string err;
    auto records = m_mainWindow->catalog()->readAllRecordsJson(
        m_mainWindow->tablesDir(), tableName, &err);
    
    if (!err.empty()) {
        reportSummary->setText(QString("Error al leer datos: %1").arg(QString::fromStdString(err)));
        return;
    }
    
    // Configurar la tabla con solo los campos seleccionados
    reportTable->setRowCount(records.size());
    reportTable->setColumnCount(selectedFieldIndices.size());
    reportTable->setHorizontalHeaderLabels(selectedHeaders);
    
    // Llenar datos solo con campos seleccionados
    for (int row = 0; row < records.size(); ++row) {
        try {
            QJsonDocument doc = QJsonDocument::fromJson(records[row].c_str());
            QJsonObject obj = doc.object();
            
            for (int col = 0; col < selectedFieldIndices.size(); ++col) {
                int fieldIndex = selectedFieldIndices[col];
                QJsonObject field = fields[fieldIndex].toObject();
                QString fieldName = field["name"].toString();
                QString value = obj[fieldName].toVariant().toString();
                reportTable->setItem(row, col, new QTableWidgetItem(value));
            }
        } catch (...) {
            qDebug() << "Error procesando registro" << row;
        }
    }
    
    // Generar resumen actualizado
    QString summary = QString(
        "Reporte personalizado de la tabla '%1'\n\n"
        "Número total de registros: %2\n"
        "Campos mostrados: %3 de %4\n\n"
        "Campos incluidos en el reporte:\n"
    ).arg(QString::fromStdString(tableName))
     .arg(records.size())
     .arg(selectedFieldIndices.size())
     .arg(fields.size());
    
    for (int index : selectedFieldIndices) {
        QJsonObject field = fields[index].toObject();
        QString fieldName = field["name"].toString();
        QString fieldType = field["type"].toString();
        if (fieldType == "number") {
            fieldType += QString(" (%1)").arg(field["numberKind"].toString());
        }
        
        QString extras = "";
        if (field["isPrimaryKey"].toBool()) extras += " [PK]";
        if (field["isUnique"].toBool()) extras += " [Unique]";
        if (!field["allowNull"].toBool()) extras += " [Required]";
        
        summary += QString("• %1 (%2)%3\n")
                   .arg(fieldName)
                   .arg(fieldType)
                   .arg(extras);
    }
    
    reportSummary->setText(summary);
    reportStackedWidget->setCurrentWidget(reportDisplayWidget);
}

void ReportesView::exportToCSV(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo crear el archivo.");
        return;
    }
    
    QTextStream out(&file);
    
    // Escribir encabezados
    QStringList headers;
    for (int col = 0; col < reportTable->columnCount(); ++col) {
        headers << reportTable->horizontalHeaderItem(col)->text();
    }
    out << headers.join(",") << "\n";
    
    // Escribir datos
    for (int row = 0; row < reportTable->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < reportTable->columnCount(); ++col) {
            QTableWidgetItem* item = reportTable->item(row, col);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join(",") << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Exportar", QString("Reporte exportado exitosamente a %1").arg(fileName));
}

void ReportesView::exportToHTML(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo crear el archivo.");
        return;
    }
    
    QTextStream out(&file);
    
    out << "<!DOCTYPE html>\n<html>\n<head>\n";
    out << "<title>" << currentSelectedReport << "</title>\n";
    out << "<style>\n";
    out << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    out << "table { border-collapse: collapse; width: 100%; }\n";
    out << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    out << "th { background-color: #f2f2f2; }\n";
    out << "tr:nth-child(even) { background-color: #f9f9f9; }\n";
    out << "</style>\n</head>\n<body>\n";
    
    out << "<h1>" << currentSelectedReport << "</h1>\n";
    out << "<table>\n<thead>\n<tr>\n";
    
    // Encabezados
    for (int col = 0; col < reportTable->columnCount(); ++col) {
        out << "<th>" << reportTable->horizontalHeaderItem(col)->text() << "</th>\n";
    }
    out << "</tr>\n</thead>\n<tbody>\n";
    
    // Datos
    for (int row = 0; row < reportTable->rowCount(); ++row) {
        out << "<tr>\n";
        for (int col = 0; col < reportTable->columnCount(); ++col) {
            QTableWidgetItem* item = reportTable->item(row, col);
            out << "<td>" << (item ? item->text() : "") << "</td>\n";
        }
        out << "</tr>\n";
    }
    
    out << "</tbody>\n</table>\n";
    out << "<br><p><strong>Resumen:</strong></p>\n";
    out << "<p>" << reportSummary->toPlainText().replace("\n", "<br>") << "</p>\n";
    out << "</body>\n</html>\n";
    
    file.close();
    QMessageBox::information(this, "Exportar", QString("Reporte exportado exitosamente a %1").arg(fileName));
}

void ReportesView::exportToText(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo crear el archivo.");
        return;
    }
    
    QTextStream out(&file);
    
    out << currentSelectedReport << "\n";
    out << "=" << QString("=").repeated(currentSelectedReport.length()) << "\n\n";
    
    // Escribir encabezados
    QStringList headers;
    for (int col = 0; col < reportTable->columnCount(); ++col) {
        headers << reportTable->horizontalHeaderItem(col)->text();
    }
    out << headers.join("\t") << "\n";
    out << QString("-").repeated(headers.join("\t").length()) << "\n";
    
    // Escribir datos
    for (int row = 0; row < reportTable->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < reportTable->columnCount(); ++col) {
            QTableWidgetItem* item = reportTable->item(row, col);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join("\t") << "\n";
    }
    
    out << "\n\nRESUMEN:\n";
    out << reportSummary->toPlainText() << "\n";
    
    file.close();
    QMessageBox::information(this, "Exportar", QString("Reporte exportado exitosamente a %1").arg(fileName));
}