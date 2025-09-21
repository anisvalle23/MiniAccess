#include "ReportesView.h"
#include "mainwindow.h"
#include "ThemeTokens.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDate>
#include <QHeaderView>

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
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    
    createHeaderSection();
    createToolbar();
    createReportFilters();
    
    // Crear splitter principal
    mainSplitter = new QSplitter(Qt::Horizontal);
    
    createReportsList();
    createReportViewer();
    
    // Agregar widgets al splitter
    mainSplitter->addWidget(reportsListWidget);
    mainSplitter->addWidget(reportViewerWidget);
    mainSplitter->setSizes({300, 700}); // Proporción inicial
    
    // Agregar widgets al layout principal
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(filtersWidget);
    mainLayout->addWidget(mainSplitter, 1); // El splitter toma el espacio restante
}

void ReportesView::createHeaderSection()
{
    headerWidget = new QWidget();
    headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    
    // Título principal
    titleLabel = new QLabel("Reportes");
    titleLabel->setFont(QFont("Inter", 28, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignLeft);
    
    // Descripción
    descriptionLabel = new QLabel("Genera y visualiza reportes personalizados de tus datos");
    descriptionLabel->setFont(QFont("Inter", 14, QFont::Normal));
    descriptionLabel->setAlignment(Qt::AlignLeft);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
}

void ReportesView::createToolbar()
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
    // TODO: Implementar carga de reportes desde el sistema de archivos
    // Por ahora, agregar algunos elementos de ejemplo
    reportsList->clear();
    
    // Reportes de ejemplo
    reportsList->addItem("Resumen Mensual de Ventas");
    reportsList->addItem("Inventario Actual");
    reportsList->addItem("Análisis de Clientes");
    reportsList->addItem("Productos Más Vendidos");
    reportsList->addItem("Reporte de Ingresos");
    
    qDebug() << "ReportesView: Reportes cargados (modo ejemplo)";
}

void ReportesView::updateReportsList()
{
    loadReports();
}

void ReportesView::generatePreview()
{
    if (!currentSelectedReport.isEmpty()) {
        qDebug() << "ReportesView: Generando preview para:" << currentSelectedReport;
        
        // Datos de ejemplo para la tabla
        reportTable->setRowCount(5);
        reportTable->setColumnCount(4);
        reportTable->setHorizontalHeaderLabels({"ID", "Producto", "Cantidad", "Valor"});
        
        // Llenar con datos de ejemplo
        for (int row = 0; row < 5; ++row) {
            reportTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
            reportTable->setItem(row, 1, new QTableWidgetItem(QString("Producto %1").arg(row + 1)));
            reportTable->setItem(row, 2, new QTableWidgetItem(QString::number((row + 1) * 10)));
            reportTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg((row + 1) * 100)));
        }
        
        // Resumen de ejemplo
        reportSummary->setText(QString(
            "Resumen del Reporte: %1\n\n"
            "Este reporte muestra datos de ejemplo generados automáticamente. "
            "En una implementación completa, aquí se mostrarían los datos reales "
            "basados en los filtros aplicados y la configuración del reporte."
        ).arg(currentSelectedReport));
        
        reportStackedWidget->setCurrentWidget(reportDisplayWidget);
    }
}

void ReportesView::updateTheme()
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
    setStyleSheet(QString("ReportesView { background-color: %1; }").arg(bgColor));
    
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
    
    createReportBtn->setStyleSheet(buttonStyle);
    generateReportBtn->setStyleSheet(buttonStyle);
    applyFiltersBtn->setStyleSheet(buttonStyle);
    
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
    
    editReportBtn->setStyleSheet(secondaryButtonStyle);
    deleteReportBtn->setStyleSheet(secondaryButtonStyle);
    exportReportBtn->setStyleSheet(secondaryButtonStyle);
    refreshBtn->setStyleSheet(secondaryButtonStyle);
    
    // Estilo de los filtros
    reportTypeLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    dateFromLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    dateToLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    
    QString comboStyle = QString(
        "QComboBox {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 4px;"
        "    padding: 5px;"
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
    
    reportTypeCombo->setStyleSheet(comboStyle);
    
    QString dateStyle = QString(
        "QDateEdit {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "    color: %3;"
        "}"
        "QDateEdit:hover {"
        "    border-color: %4;"
        "}"
    ).arg(bgColor).arg(borderColor).arg(textColor).arg(primaryColor);
    
    dateFromEdit->setStyleSheet(dateStyle);
    dateToEdit->setStyleSheet(dateStyle);
    
    // Estilo de la lista
    reportsListLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    
    QString listStyle = QString(
        "QListWidget {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "    color: %3;"
        "    padding: 5px;"
        "}"
        "QListWidget::item {"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "    margin: 1px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: %4;"
        "    color: white;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: %5;"
        "}"
    ).arg(bgColor)
     .arg(borderColor)
     .arg(textColor)
     .arg(primaryColor)
     .arg(hoverColor);
    
    reportsList->setStyleSheet(listStyle);
    
    // Estilo del estado vacío
    emptyStateLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    emptyStateDescription->setStyleSheet(QString("color: %1;").arg(secondaryTextColor));
    
    // Estilo del título del reporte
    reportTitleLabel->setStyleSheet(QString("color: %1;").arg(textColor));
    
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
    
    reportTable->setStyleSheet(tableStyle);
    
    // Estilo del resumen de texto
    QString textEditStyle = QString(
        "QTextEdit {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "    color: %3;"
        "    padding: 10px;"
        "}"
    ).arg(bgColor).arg(borderColor).arg(textColor);
    
    reportSummary->setStyleSheet(textEditStyle);
}

void ReportesView::refreshView()
{
    qDebug() << "ReportesView: Actualizando vista...";
    loadReports();
}

// Slots para los botones
void ReportesView::onCreateReportClicked()
{
    qDebug() << "ReportesView: Crear reporte clickeado";
    // TODO: Implementar creación de reportes
    QMessageBox::information(this, "Reportes", "Función de crear reporte será implementada próximamente.");
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
    if (!currentSelectedReport.isEmpty()) {
        qDebug() << "ReportesView: Editando reporte:" << currentSelectedReport;
        // TODO: Implementar edición de reportes
        QMessageBox::information(this, "Reportes", QString("Editar reporte '%1' será implementado próximamente.").arg(currentSelectedReport));
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
        QString fileName = QFileDialog::getSaveFileName(this, 
                                                       "Exportar Reporte", 
                                                       currentSelectedReport + ".pdf",
                                                       "PDF files (*.pdf);;CSV files (*.csv)");
        
        if (!fileName.isEmpty()) {
            qDebug() << "ReportesView: Exportando reporte a:" << fileName;
            // TODO: Implementar exportación de reportes
            QMessageBox::information(this, "Reportes", "Función de exportar reporte será implementada próximamente.");
        }
    }
}

void ReportesView::onReportTypeChanged()
{
    qDebug() << "ReportesView: Tipo de reporte cambiado a:" << reportTypeCombo->currentText();
    // TODO: Filtrar reportes por tipo
}