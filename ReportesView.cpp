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
#include <QFrame>

ReportesView::ReportesView(MainWindow *mainWindow, QWidget *parent)
    : QWidget(parent)
    , m_mainWindow(mainWindow)
    , currentSelectedReport("")
    , currentSelectedTable("")
    , headerShadow(nullptr)
    , toolbarShadow(nullptr)
    , fadeAnimation(nullptr)
    , mainLayout(nullptr)
    , scrollArea(nullptr)
    , headerWidget(nullptr)
    , headerLayout(nullptr)
    , titleLabel(nullptr)
    , descriptionLabel(nullptr)
    , tableSelectorWidget(nullptr)
    , tableSelectorLayout(nullptr)
    , tableSelectorLabel(nullptr)
    , tableComboBox(nullptr)
    , reportContentWidget(nullptr)
    , reportContentLayout(nullptr)
    , reportCardWidget(nullptr)
    , reportCardLayout(nullptr)
    , reportTitleLabel(nullptr)
    , reportDataWidget(nullptr)
    , reportDataLayout(nullptr)
    , emptyStateWidget(nullptr)
    , emptyStateLayout(nullptr)
    , emptyStateLabel(nullptr)
{
    qDebug() << "ReportesView: Constructor iniciado";
    setupUI();
    styleComponents();
    loadReports();
    qDebug() << "ReportesView: Constructor completado";
}

ReportesView::~ReportesView()
{
    // Cleanup será manejado automáticamente por Qt
}

void ReportesView::setupUI()
{
    // Layout principal con márgenes para evitar que se vea cortado
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20); // Márgenes para separación del borde
    mainLayout->setSpacing(15); // Espaciado entre secciones principales
    
    createHeaderSection();
    createTableSelector();
    createReportContent();
    
    // Crear scroll area solo para el contenido del reporte, no para todo
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Establecer el widget de contenido en el scroll area
    scrollArea->setWidget(reportContentWidget);
    
    // Agregar widgets al layout principal - header y selector fijos, solo contenido con scroll
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(tableSelectorWidget);
    mainLayout->addWidget(scrollArea, 1); // El scroll area toma el espacio restante
    
    // Cargar tablas disponibles
    qDebug() << "ReportesView: Llamando loadAvailableTables() desde setupUI()";
    loadAvailableTables();
}

void ReportesView::createHeaderSection()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(160); // Altura suficiente para ver todo el contenido
    headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(25, 30, 25, 30); // Márgenes internos generosos
    headerLayout->setSpacing(12);
    
    // Título principal con estilo moderno
    titleLabel = new QLabel("📊 Reportes");
    titleLabel->setFont(QFont("Inter", 30, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignLeft);
    
    // Descripción con mejor espaciado
    descriptionLabel = new QLabel("Visualiza y analiza la información de tus datos");
    descriptionLabel->setFont(QFont("Inter", 15, QFont::Normal));
    descriptionLabel->setAlignment(Qt::AlignLeft);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
}

void ReportesView::createTableSelector()
{
    tableSelectorWidget = new QWidget();
    tableSelectorWidget->setFixedHeight(90); // Altura ajustada
    tableSelectorLayout = new QHBoxLayout(tableSelectorWidget);
    tableSelectorLayout->setContentsMargins(25, 20, 25, 20); // Márgenes internos
    tableSelectorLayout->setSpacing(20);
    
    // Label para el selector
    tableSelectorLabel = new QLabel("Seleccionar tabla:");
    tableSelectorLabel->setFont(QFont("Inter", 16, QFont::Medium));
    
    // ComboBox para seleccionar tabla
    tableComboBox = new QComboBox();
    tableComboBox->setMinimumWidth(250);
    tableComboBox->setMinimumHeight(40); // Altura ajustada
    tableComboBox->setFont(QFont("Inter", 14));
    tableComboBox->addItem("-- Seleccionar tabla --");
    
    // Conectar señal
    connect(tableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportesView::onTableSelectionChanged);
    
    tableSelectorLayout->addWidget(tableSelectorLabel);
    tableSelectorLayout->addWidget(tableComboBox);
    tableSelectorLayout->addStretch();
}

void ReportesView::createReportContent()
{
    reportContentWidget = new QWidget();
    reportContentWidget->setMinimumHeight(500); // Altura mínima para asegurar espacio
    reportContentLayout = new QVBoxLayout(reportContentWidget);
    reportContentLayout->setContentsMargins(25, 25, 25, 30); // Márgenes balanceados
    reportContentLayout->setSpacing(25); // Espaciado entre elementos
    
    // Crear estado vacío
    createEmptyState();
    
    // Crear tarjeta principal del reporte
    createReportCard();
    
    reportContentLayout->addWidget(emptyStateWidget);
    reportContentLayout->addWidget(reportCardWidget);
    reportContentLayout->addStretch(); // Espacio flexible al final
    
    // Inicialmente mostrar estado vacío
    showEmptyState();
}

void ReportesView::createEmptyState()
{
    emptyStateWidget = new QWidget();
    emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setContentsMargins(50, 80, 50, 80); // Más márgenes
    emptyStateLayout->setSpacing(30);
    
    // Ícono más grande
    QLabel *iconLabel = new QLabel("📊");
    iconLabel->setFont(QFont("Inter", 64));
    iconLabel->setAlignment(Qt::AlignCenter);
    
    emptyStateLabel = new QLabel("Selecciona una tabla para generar un reporte");
    emptyStateLabel->setFont(QFont("Inter", 20, QFont::Medium)); // Fuente más grande
    emptyStateLabel->setAlignment(Qt::AlignCenter);
    emptyStateLabel->setStyleSheet("color: #666666; background: transparent;");
    
    // Subtítulo
    QLabel *subtitleLabel = new QLabel("Los reportes te ayudarán a analizar y visualizar tus datos");
    subtitleLabel->setFont(QFont("Inter", 14));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #999999; background: transparent;");
    
    emptyStateLayout->addWidget(iconLabel);
    emptyStateLayout->addWidget(emptyStateLabel);
    emptyStateLayout->addWidget(subtitleLabel);
}

void ReportesView::createReportCard()
{
    reportCardWidget = new QWidget();
    reportCardWidget->setMinimumHeight(450); // Altura ajustada
    reportCardLayout = new QVBoxLayout(reportCardWidget);
    reportCardLayout->setContentsMargins(30, 25, 30, 25); // Márgenes balanceados
    reportCardLayout->setSpacing(20); // Espaciado ajustado
    
    // Título del reporte
    reportTitleLabel = new QLabel("Reporte de Datos");
    reportTitleLabel->setFont(QFont("Inter", 24, QFont::Bold));
    reportTitleLabel->setAlignment(Qt::AlignLeft);
    
    // Contenedor para los datos del reporte
    reportDataWidget = new QWidget();
    reportDataLayout = new QVBoxLayout(reportDataWidget);
    reportDataLayout->setContentsMargins(0, 0, 0, 0);
    reportDataLayout->setSpacing(18); // Espaciado entre elementos de datos
    
    // Crear elementos de datos individuales
    createReportDataItems();
    
    // Agregar total al final
    createTotalSection();
    
    reportCardLayout->addWidget(reportTitleLabel);
    reportCardLayout->addWidget(reportDataWidget);
    reportCardLayout->addStretch(); // Espacio flexible al final
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

void ReportesView::styleComponents()
{
    updateTheme();
}

void ReportesView::loadAvailableTables()
{
    qDebug() << "ReportesView: loadAvailableTables() iniciado";
    
    tableComboBox->clear();
    tableComboBox->addItem("-- Seleccionar tabla --");
    
    if (!m_mainWindow) {
        qDebug() << "ReportesView: MainWindow no disponible";
        return;
    }
    
    // Usar la misma lógica que FormulariosView - buscar archivos .meta
    QString tablesPath = QString::fromStdString(m_mainWindow->tablesDir());
    QDir tablesDir(tablesPath);
    
    qDebug() << "ReportesView: Buscando tablas en:" << tablesPath;
    qDebug() << "ReportesView: Directorio existe:" << tablesDir.exists();
    
    if (!tablesDir.exists()) {
        qDebug() << "ReportesView: Directorio de tablas no existe:" << tablesPath;
        return;
    }
    
    QStringList metaFiles = tablesDir.entryList(QStringList() << "*.meta", QDir::Files);
    qDebug() << "ReportesView: Archivos .meta encontrados:" << metaFiles;
    
    for (const QString &metaFile : metaFiles) {
        QString tableName = metaFile;
        tableName.remove(".meta");
        tableComboBox->addItem(tableName);
        qDebug() << "ReportesView: Tabla agregada al combobox:" << tableName;
    }
    
    if (metaFiles.isEmpty()) {
        qDebug() << "ReportesView: No se encontraron tablas en" << tablesPath;
    } else {
        qDebug() << "ReportesView: Se encontraron" << metaFiles.size() << "tablas";
        qDebug() << "ReportesView: Items en combobox:" << tableComboBox->count();
    }
}

void ReportesView::showEmptyState()
{
    emptyStateWidget->show();
    reportCardWidget->hide();
}

void ReportesView::generateReportForTable(const QString& tableName)
{
    if (tableName.isEmpty() || tableName == "-- Seleccionar tabla --") {
        showEmptyState();
        return;
    }
    
    currentSelectedTable = tableName;
    
    if (!m_mainWindow || !m_mainWindow->catalog()) {
        qDebug() << "ReportesView: MainWindow o catalog no disponible";
        showEmptyState();
        return;
    }
    
    // Leer metadatos de la tabla
    QJsonObject tableMeta = readTableMetadata(tableName.toStdString());
    
    // Usar el mismo sistema que FormulariosView para cargar datos
    std::string err;
    std::vector<std::string> lines = m_mainWindow->catalog()->readAllRecordsJson(
        m_mainWindow->tablesDir(), tableName.toStdString(), &err
    );
    
    QJsonArray records;
    int totalRecords = 0;
    
    if (!err.empty()) {
        qDebug() << "ReportesView: Error al cargar datos:" << QString::fromStdString(err);
        totalRecords = 0;
    } else {
        // Convertir cada línea JSON a QJsonObject y añadir al array
        for (const std::string& line : lines) {
            QByteArray ba = QByteArray::fromStdString(line);
            QJsonDocument doc = QJsonDocument::fromJson(ba);
            
            if (doc.isObject()) {
                records.append(doc.object());
            }
        }
        totalRecords = records.size();
        qDebug() << "ReportesView: Cargados" << totalRecords << "registros de" << tableName;
    }
    
    // Generar reporte avanzado y dinámico
    generateAdvancedReport(tableName, tableMeta, records, totalRecords);
}

void ReportesView::generateAdvancedReport(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records, int totalRecords)
{
    // Actualizar título del reporte
    reportTitleLabel->setText(QString("📊 Análisis de %1").arg(tableName));
    
    // Limpiar datos anteriores
    QLayoutItem* item;
    while ((item = reportDataLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // Mostrar tarjeta del reporte
    emptyStateWidget->hide();
    reportCardWidget->show();
    
    if (totalRecords == 0) {
        createNoDataState();
        return;
    }
    
    // === CREAR CARDS DE ESTADÍSTICAS PRINCIPALES ===
    createMainStatsCards(tableName, tableMeta, records, totalRecords);
    
    // === SEPARADOR ===
    createSeparator();
    
    // === ANÁLISIS DETALLADO POR CAMPO ===
    createDetailedFieldAnalysis(tableName, tableMeta, records);
    
    // === INSIGHTS Y RESUMEN ===
    createInsightsSection(tableName, totalRecords);
}

void ReportesView::createStatsCard(const QString& title, const QString& value, const QString& description, const QString& color)
{
    QWidget* card = new QWidget();
    card->setMinimumHeight(120);
    card->setMaximumHeight(120);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 15, 20, 15);
    cardLayout->setSpacing(8);
    
    // Título con emoji
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setFont(QFont("Inter", 14, QFont::Medium));
    titleLabel->setStyleSheet("color: #6b7280;");
    
    // Valor principal (grande y destacado)
    QLabel* valueLabel = new QLabel(value);
    valueLabel->setFont(QFont("Inter", 28, QFont::Bold));
    valueLabel->setStyleSheet(QString("color: %1;").arg(color));
    
    // Descripción
    QLabel* descLabel = new QLabel(description);
    descLabel->setFont(QFont("Inter", 12));
    descLabel->setStyleSheet("color: #9ca3af;");
    
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(valueLabel);
    cardLayout->addWidget(descLabel);
    cardLayout->addStretch();
    
    // Estilo de la tarjeta con gradiente y sombra
    card->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #ffffff, stop:1 #f8fafc);"
        "border: 1px solid #e5e7eb;"
        "border-radius: 12px;"
        "}"
        "QWidget:hover {"
        "border: 1px solid " + color + ";"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #ffffff, stop:1 #f0f9ff);"
        "}"
    );
    
    // Agregar la tarjeta al layout de datos del reporte
    reportDataLayout->addWidget(card);
}

void ReportesView::createChartSection(const QString& title, const QJsonArray& records, const QJsonObject& tableMeta)
{
    // Separador visual
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("border: 1px solid #e5e7eb; margin: 20px 0;");
    reportDataLayout->addWidget(separator);
    
    // Título de la sección
    QLabel* sectionTitle = new QLabel(title);
    sectionTitle->setFont(QFont("Inter", 20, QFont::Bold));
    sectionTitle->setStyleSheet("color: #1f2937; margin: 20px 0 15px 0;");
    reportDataLayout->addWidget(sectionTitle);
    
    // Contenedor de análisis por campos
    QWidget* analysisContainer = new QWidget();
    QVBoxLayout* analysisLayout = new QVBoxLayout(analysisContainer);
    analysisLayout->setSpacing(15);
    analysisLayout->setContentsMargins(0, 0, 0, 0);
    
    QJsonArray fields = tableMeta["fields"].toArray();
    int maxFieldsToShow = qMin(5, fields.size()); // Mostrar máximo 5 campos para no sobrecargar
    
    for (int i = 0; i < maxFieldsToShow; ++i) {
        QJsonObject field = fields[i].toObject();
        QString fieldName = field.value("name").toString();
        QString fieldType = field.value("type").toString().toLower();
        
        // Widget para cada campo
        QWidget* fieldWidget = new QWidget();
        QVBoxLayout* fieldLayout = new QVBoxLayout(fieldWidget);
        fieldLayout->setContentsMargins(20, 15, 20, 15);
        fieldLayout->setSpacing(10);
        
        // Título del campo
        QLabel* fieldTitle = new QLabel(QString("📊 %1").arg(fieldName));
        fieldTitle->setFont(QFont("Inter", 16, QFont::DemiBold));
        fieldTitle->setStyleSheet("color: #374151;");
        fieldLayout->addWidget(fieldTitle);
        
        if (fieldType.contains("texto") || fieldType.contains("text") || fieldType.contains("string")) {
            // Análisis para campos de texto: valores únicos
            QMap<QString, int> valueCount;
            for (const auto& recordValue : records) {
                QJsonObject record = recordValue.toObject();
                QString value = record.value(fieldName).toString().trimmed();
                if (!value.isEmpty()) {
                    valueCount[value]++;
                }
            }
            
            QLabel* analysisLabel = new QLabel(QString("🏷️ %1 valores únicos encontrados").arg(valueCount.size()));
            analysisLabel->setFont(QFont("Inter", 13));
            analysisLabel->setStyleSheet("color: #6b7280; margin-bottom: 8px;");
            fieldLayout->addWidget(analysisLabel);
            
            // Mostrar los valores más comunes (top 3)
            auto sortedValues = valueCount.keys();
            std::sort(sortedValues.begin(), sortedValues.end(), [&](const QString& a, const QString& b) {
                return valueCount[a] > valueCount[b];
            });
            
            for (int j = 0; j < qMin(3, sortedValues.size()); ++j) {
                QString value = sortedValues[j];
                int count = valueCount[value];
                double percentage = (double(count) / records.size()) * 100;
                
                QWidget* barWidget = new QWidget();
                QHBoxLayout* barLayout = new QHBoxLayout(barWidget);
                barLayout->setContentsMargins(0, 5, 0, 5);
                
                QLabel* valueLabel = new QLabel(QString("• %1").arg(value));
                valueLabel->setFont(QFont("Inter", 12));
                valueLabel->setMinimumWidth(120);
                
                QLabel* countLabel = new QLabel(QString("%1 (%2%)").arg(count).arg(QString::number(percentage, 'f', 1)));
                countLabel->setFont(QFont("Inter", 12, QFont::Medium));
                countLabel->setStyleSheet("color: #059669;");
                
                barLayout->addWidget(valueLabel);
                barLayout->addWidget(countLabel);
                barLayout->addStretch();
                
                fieldLayout->addWidget(barWidget);
            }
            
        } else if (fieldType.contains("entero") || fieldType.contains("number") || fieldType.contains("integer")) {
            // Análisis para campos numéricos: promedio, min, max
            QList<double> numbers;
            for (const auto& recordValue : records) {
                QJsonObject record = recordValue.toObject();
                bool ok;
                double value = record.value(fieldName).toString().toDouble(&ok);
                if (ok) {
                    numbers.append(value);
                }
            }
            
            if (!numbers.isEmpty()) {
                std::sort(numbers.begin(), numbers.end());
                double sum = std::accumulate(numbers.begin(), numbers.end(), 0.0);
                double average = sum / numbers.size();
                double min = numbers.first();
                double max = numbers.last();
                
                QLabel* statsLabel = new QLabel(QString("📈 %1 valores numéricos analizados").arg(numbers.size()));
                statsLabel->setFont(QFont("Inter", 13));
                statsLabel->setStyleSheet("color: #6b7280; margin-bottom: 8px;");
                fieldLayout->addWidget(statsLabel);
                
                // Grid de estadísticas
                QWidget* statsGrid = new QWidget();
                QGridLayout* gridLayout = new QGridLayout(statsGrid);
                gridLayout->setSpacing(10);
                
                auto addStat = [&](int row, const QString& label, double value, const QString& color) {
                    QLabel* labelWidget = new QLabel(label);
                    labelWidget->setFont(QFont("Inter", 11));
                    labelWidget->setStyleSheet("color: #6b7280;");
                    
                    QLabel* valueWidget = new QLabel(QString::number(value, 'f', 2));
                    valueWidget->setFont(QFont("Inter", 13, QFont::Bold));
                    valueWidget->setStyleSheet(QString("color: %1;").arg(color));
                    
                    gridLayout->addWidget(labelWidget, row, 0);
                    gridLayout->addWidget(valueWidget, row, 1);
                };
                
                addStat(0, "• Promedio:", average, "#3b82f6");
                addStat(0, "• Mínimo:", min, "#ef4444");
                addStat(0, "• Máximo:", max, "#10b981");
                
                fieldLayout->addWidget(statsGrid);
            }
        }
        
        // Estilo del widget del campo
        fieldWidget->setStyleSheet(
            "QWidget {"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
            "stop:0 #fefefe, stop:1 #f9fafb);"
            "border: 1px solid #e5e7eb;"
            "border-radius: 8px;"
            "margin: 5px 0;"
            "}"
        );
        
        analysisLayout->addWidget(fieldWidget);
    }
    
    reportDataLayout->addWidget(analysisContainer);
}

void ReportesView::generateBasicAnalysisFromArray(const QJsonArray& records, const QJsonObject& tableMeta)
{
    // Este método ahora es redundante ya que usamos generateAdvancedReport
    qDebug() << "ReportesView: generateBasicAnalysisFromArray() - usando generateAdvancedReport en su lugar";
}

void ReportesView::loadReports()
{
    // Este método ya no necesita hacer nada - las tablas se cargan en setupUI()
    qDebug() << "ReportesView: loadReports() llamado - tablas ya cargadas en setupUI()";
}

// Slots
void ReportesView::onTableSelectionChanged()
{
    QString selectedTable = tableComboBox->currentText();
    
    qDebug() << "ReportesView: onTableSelectionChanged() - tabla seleccionada:" << selectedTable;
    
    if (selectedTable == "-- Seleccionar tabla --" || selectedTable.isEmpty()) {
        qDebug() << "ReportesView: Mostrando estado vacío";
        showEmptyState();
        return;
    }
    
    currentSelectedTable = selectedTable;
    qDebug() << "ReportesView: Generando reporte para tabla:" << selectedTable;
    generateReportForTable(selectedTable);
}

void ReportesView::updateTheme()
{
    bool isDark = ThemeManager::instance().isDark();
    
    // Colores principales del nuevo diseño
    QString primaryColor = "#E53E3E"; // Rojo principal como en la imagen
    QString accentColor = "#E53E3E"; // Color de acento para selecciones
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
    
    // Estilo del header con fondo blanco y borde visible
    QString headerStyle = QString(
        "QWidget {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 12px;"
        "    margin: 0px;" // Sin márgenes externos que causen corte
        "}"
    ).arg(cardBgColor).arg(borderColor);
    
    headerWidget->setStyleSheet(headerStyle);
    
    // Estilo del título en color oscuro para contrastar con el fondo blanco
    titleLabel->setStyleSheet(QString("color: %1; background: transparent; font-weight: 700; padding: 0px;").arg(textColor));
    descriptionLabel->setStyleSheet(QString("color: %1; background: transparent; padding: 0px;").arg(secondaryTextColor));
    
    // Estilo del selector de tablas
    tableSelectorWidget->setStyleSheet(QString(
        "QWidget { "
        "    background-color: %1; "
        "    border: 1px solid %2; "
        "    border-radius: 8px; "
        "    margin: 0px;" // Sin márgenes externos
        "}"
    ).arg(bgColor).arg(borderColor));
    
    tableSelectorLabel->setStyleSheet(QString(
        "color: %1; background: transparent; font-weight: 600; padding: 0px;"
    ).arg(textColor));
    
    tableComboBox->setStyleSheet(QString(
        "QComboBox {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 8px;"
        "    padding: 10px 14px;"
        "    color: %3;"
        "    font-size: 14px;"
        "    font-weight: 500;"
        "}"
        "QComboBox:focus {"
        "    border-color: #007bff;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 5px solid transparent;"
        "    border-right: 5px solid transparent;"
        "    border-top: 5px solid %3;"
        "    margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    selection-background-color: %4;"
        "    color: %3;"
        "    padding: 6px;"
        "}"
    ).arg(cardBgColor).arg(borderColor).arg(textColor).arg(accentColor));
    
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
    
    // Estilo del scroll area
    if (scrollArea) {
        scrollArea->setStyleSheet(QString(
            "QScrollArea {"
            "    background-color: %1;"
            "    border: none;"
            "}"
            "QScrollBar:vertical {"
            "    background-color: %2;"
            "    width: 12px;"
            "    border-radius: 6px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background-color: %3;"
            "    border-radius: 6px;"
            "    min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "    background-color: %4;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "    height: 0px;"
            "}"
            "QScrollBar:horizontal {"
            "    background-color: %2;"
            "    height: 12px;"
            "    border-radius: 6px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background-color: %3;"
            "    border-radius: 6px;"
            "    min-width: 20px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "    background-color: %4;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;"
            "}"
        ).arg(bgColor).arg(borderColor).arg(secondaryTextColor).arg(textColor));
    }
}

void ReportesView::refreshView()
{
    qDebug() << "ReportesView: refreshView() llamado";
    loadAvailableTables();
    if (!currentSelectedTable.isEmpty()) {
        generateReportForTable(currentSelectedTable);
    }
}

// Slots simplificados para el nuevo diseño
void ReportesView::onCreateReportClicked()
{
    QMessageBox::information(this, "Crear Reporte", "Funcionalidad de crear reporte en desarrollo");
}

void ReportesView::onReportSelected()
{
    // No utilizado en el nuevo diseño
}

void ReportesView::onEditReportClicked()
{
    QMessageBox::information(this, "Editar Reporte", "Funcionalidad de editar reporte en desarrollo");
}

void ReportesView::onDeleteReportClicked()
{
    QMessageBox::information(this, "Eliminar Reporte", "Funcionalidad de eliminar reporte en desarrollo");
}

void ReportesView::onGenerateReportClicked()
{
    loadReports();
    QMessageBox::information(this, "Generar Reporte", "Reporte actualizado exitosamente");
}

void ReportesView::onExportReportClicked()
{
    QMessageBox::information(this, "Exportar Reporte", "Funcionalidad de exportar reporte en desarrollo");
}

void ReportesView::onReportTypeChanged()
{
    qDebug() << "ReportesView: Tipo de reporte cambiado";
}

QJsonObject ReportesView::readTableMetadata(const std::string& tableName)
{
    if (!m_mainWindow) {
        return QJsonObject();
    }
    
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

// Métodos vacíos para compatibilidad
void ReportesView::updateReportsList() {}
void ReportesView::generatePreview() {}
void ReportesView::generateSingleTableReport(const std::string& tableName) {}
void ReportesView::generateSummaryReport() {}
void ReportesView::generateCompleteReport() {}
void ReportesView::generatePreviewWithSelectedFields(const std::string& tableName, 
                                                    const QList<QCheckBox*>& checkboxes, 
                                                    const QJsonObject& tableMeta) {}
void ReportesView::exportToCSV(const QString& fileName) {}
void ReportesView::exportToHTML(const QString& fileName) {}
void ReportesView::exportToText(const QString& fileName) {}

// === NUEVOS MÉTODOS PARA DISEÑO MODERNO ===

void ReportesView::createNoDataState()
{
    QWidget* noDataWidget = new QWidget();
    QVBoxLayout* noDataLayout = new QVBoxLayout(noDataWidget);
    noDataLayout->setAlignment(Qt::AlignCenter);
    noDataLayout->setSpacing(25);
    noDataLayout->setContentsMargins(40, 60, 40, 60);
    
    QLabel* iconLabel = new QLabel("📭");
    iconLabel->setFont(QFont("Inter", 64));
    iconLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* messageLabel = new QLabel("No hay datos disponibles");
    messageLabel->setFont(QFont("Inter", 20, QFont::Bold));
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("color: #374151;");
    
    QLabel* subLabel = new QLabel("Agrega algunos registros para ver estadísticas increíbles");
    subLabel->setFont(QFont("Inter", 14));
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("color: #6b7280;");
    
    noDataLayout->addWidget(iconLabel);
    noDataLayout->addWidget(messageLabel);
    noDataLayout->addWidget(subLabel);
    
    noDataWidget->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #f9fafb, stop:1 #f3f4f6);"
        "border-radius: 16px;"
        "border: 2px dashed #d1d5db;"
        "}"
    );
    
    reportDataLayout->addWidget(noDataWidget);
}

void ReportesView::createMainStatsCards(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records, int totalRecords)
{
    // Título de la sección
    QLabel* sectionTitle = new QLabel("📈 Estadísticas Generales");
    sectionTitle->setFont(QFont("Inter", 20, QFont::Bold));
    sectionTitle->setStyleSheet("color: #1f2937; margin: 20px 0px 15px 0px;");
    reportDataLayout->addWidget(sectionTitle);
    
    // Container para las cards en grid
    QWidget* statsContainer = new QWidget();
    QGridLayout* statsGrid = new QGridLayout(statsContainer);
    statsGrid->setSpacing(20);
    statsGrid->setContentsMargins(0, 0, 0, 0);
    
    // Analizar campos
    QJsonArray fields = tableMeta["fields"].toArray();
    int numericFields = 0;
    int textFields = 0;
    
    for (const auto& fieldValue : fields) {
        QJsonObject field = fieldValue.toObject();
        QString fieldType = field.value("type").toString().toLower();
        
        if (fieldType.contains("number") || fieldType.contains("integer") || fieldType.contains("entero")) {
            numericFields++;
        } else if (fieldType.contains("text") || fieldType.contains("string") || fieldType.contains("texto")) {
            textFields++;
        }
    }
    
    // Calcular completitud de datos
    int completeRecords = 0;
    for (const auto& recordValue : records) {
        QJsonObject record = recordValue.toObject();
        bool isComplete = true;
        for (const auto& fieldValue : fields) {
            QJsonObject field = fieldValue.toObject();
            QString fieldName = field.value("name").toString();
            if (!record.contains(fieldName) || record.value(fieldName).toString().isEmpty()) {
                isComplete = false;
                break;
            }
        }
        if (isComplete) completeRecords++;
    }
    
    int completenessPercentage = totalRecords > 0 ? (completeRecords * 100) / totalRecords : 0;
    
    // Cards modernas
    QWidget* totalCard = createModernStatsCard("📋", "Total de Registros", QString::number(totalRecords), 
                                              "Registros almacenados", "#3b82f6");
    QWidget* numericCard = createModernStatsCard("🔢", "Campos Numéricos", QString::number(numericFields), 
                                                "Para análisis matemático", "#10b981");
    QWidget* textCard = createModernStatsCard("📝", "Campos de Texto", QString::number(textFields), 
                                             "Para análisis categórico", "#f59e0b");
    QWidget* completenessCard = createModernStatsCard("✅", "Datos Completos", QString("%1%").arg(completenessPercentage), 
                                                     QString("%1 de %2 registros").arg(completeRecords).arg(totalRecords), "#8b5cf6");
    
    // Agregar cards al grid (2x2)
    statsGrid->addWidget(totalCard, 0, 0);
    statsGrid->addWidget(numericCard, 0, 1);
    statsGrid->addWidget(textCard, 1, 0);
    statsGrid->addWidget(completenessCard, 1, 1);
    
    reportDataLayout->addWidget(statsContainer);
}

void ReportesView::createDetailedFieldAnalysis(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records)
{
    // Título de la sección
    QLabel* sectionTitle = new QLabel("🔍 Análisis Detallado por Campo");
    sectionTitle->setFont(QFont("Inter", 20, QFont::Bold));
    sectionTitle->setStyleSheet("color: #1f2937; margin: 30px 0px 15px 0px;");
    reportDataLayout->addWidget(sectionTitle);
    
    // Container para análisis de campos
    QWidget* fieldsContainer = new QWidget();
    QVBoxLayout* fieldsLayout = new QVBoxLayout(fieldsContainer);
    fieldsLayout->setSpacing(15);
    fieldsLayout->setContentsMargins(0, 0, 0, 0);
    
    QJsonArray fields = tableMeta["fields"].toArray();
    
    for (const auto& fieldValue : fields) {
        QJsonObject field = fieldValue.toObject();
        QString fieldName = field.value("name").toString();
        QString fieldType = field.value("type").toString();
        
        QWidget* fieldCard = createFieldAnalysisCard(fieldName, fieldType, records);
        fieldsLayout->addWidget(fieldCard);
    }
    
    reportDataLayout->addWidget(fieldsContainer);
}

void ReportesView::createInsightsSection(const QString& tableName, int totalRecords)
{
    // Título de la sección
    QLabel* sectionTitle = new QLabel("💡 Insights y Resumen");
    sectionTitle->setFont(QFont("Inter", 20, QFont::Bold));
    sectionTitle->setStyleSheet("color: #1f2937; margin: 30px 0px 15px 0px;");
    reportDataLayout->addWidget(sectionTitle);
    
    // Card de insights
    QWidget* insightCard = new QWidget();
    QVBoxLayout* insightLayout = new QVBoxLayout(insightCard);
    insightLayout->setContentsMargins(25, 20, 25, 20);
    insightLayout->setSpacing(15);
    
    QLabel* insightTitle = new QLabel("📊 Resumen del Análisis");
    insightTitle->setFont(QFont("Inter", 16, QFont::Bold));
    insightTitle->setStyleSheet("color: #374151;");
    
    QString insightText;
    if (totalRecords == 0) {
        insightText = "Esta tabla no contiene datos. Considera agregar registros para obtener insights valiosos.";
    } else if (totalRecords < 10) {
        insightText = QString("Con %1 registros, tienes una muestra pequeña pero manejable. "
                             "Considera expandir los datos para obtener análisis más robustos.").arg(totalRecords);
    } else if (totalRecords < 100) {
        insightText = QString("Excelente! Con %1 registros tienes una buena base de datos para análisis. "
                             "Los patrones en estos datos pueden ser muy reveladores.").arg(totalRecords);
    } else {
        insightText = QString("¡Impresionante! Con %1 registros tienes una base de datos robusta. "
                             "Esta cantidad permite análisis estadísticos confiables y detección de patrones complejos.").arg(totalRecords);
    }
    
    QLabel* insightContent = new QLabel(insightText);
    insightContent->setFont(QFont("Inter", 14));
    insightContent->setWordWrap(true);
    insightContent->setStyleSheet("color: #6b7280; line-height: 1.5;");
    
    insightLayout->addWidget(insightTitle);
    insightLayout->addWidget(insightContent);
    
    insightCard->setStyleSheet(
        "QWidget {"
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "stop:0 #fef3c7, stop:1 #fde68a);"
        "border-radius: 12px;"
        "border: 1px solid #f59e0b;"
        "}"
    );
    
    reportDataLayout->addWidget(insightCard);
}

void ReportesView::createSeparator()
{
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);
    separator->setStyleSheet("QFrame { background-color: #e5e7eb; border: none; margin: 20px 0px; }");
    reportDataLayout->addWidget(separator);
}

QWidget* ReportesView::createModernStatsCard(const QString& icon, const QString& title, const QString& value, const QString& subtitle, const QString& color)
{
    QWidget* card = new QWidget();
    card->setFixedHeight(140);
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(8);
    
    // Header con ícono
    QWidget* header = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* iconLabel = new QLabel(icon);
    iconLabel->setFont(QFont("Inter", 24));
    
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setFont(QFont("Inter", 12, QFont::Medium));
    titleLabel->setStyleSheet("color: #6b7280;");
    
    headerLayout->addWidget(iconLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel);
    
    // Valor principal
    QLabel* valueLabel = new QLabel(value);
    valueLabel->setFont(QFont("Inter", 28, QFont::Bold));
    valueLabel->setStyleSheet(QString("color: %1;").arg(color));
    valueLabel->setAlignment(Qt::AlignCenter);
    
    // Subtítulo
    QLabel* subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setFont(QFont("Inter", 11));
    subtitleLabel->setStyleSheet("color: #9ca3af;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    
    layout->addWidget(header);
    layout->addWidget(valueLabel);
    layout->addWidget(subtitleLabel);
    layout->addStretch();
    
    card->setStyleSheet(
        "QWidget {"
        "background-color: white;"
        "border-radius: 12px;"
        "border: 1px solid #e5e7eb;"
        "}"
        "QWidget:hover {"
        "border-color: " + color + ";"
        "}"
    );
    
    // Efecto de sombra
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setXOffset(0);
    shadow->setYOffset(2);
    shadow->setColor(QColor(0, 0, 0, 20));
    card->setGraphicsEffect(shadow);
    
    return card;
}

QWidget* ReportesView::createFieldAnalysisCard(const QString& fieldName, const QString& fieldType, const QJsonArray& records)
{
    QWidget* card = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(card);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);
    
    // Información del campo
    QWidget* fieldInfo = new QWidget();
    QVBoxLayout* infoLayout = new QVBoxLayout(fieldInfo);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);
    
    QLabel* nameLabel = new QLabel(fieldName);
    nameLabel->setFont(QFont("Inter", 14, QFont::Bold));
    nameLabel->setStyleSheet("color: #1f2937;");
    
    QLabel* typeLabel = new QLabel(fieldType);
    typeLabel->setFont(QFont("Inter", 12));
    typeLabel->setStyleSheet("color: #6b7280;");
    
    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(typeLabel);
    
    // Estadísticas del campo
    QWidget* stats = new QWidget();
    QHBoxLayout* statsLayout = new QHBoxLayout(stats);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(20);
    
    // Contar valores únicos y no vacíos
    QSet<QString> uniqueValues;
    int nonEmptyValues = 0;
    
    for (const auto& recordValue : records) {
        QJsonObject record = recordValue.toObject();
        QString value = record.value(fieldName).toString();
        if (!value.isEmpty()) {
            uniqueValues.insert(value);
            nonEmptyValues++;
        }
    }
    
    // Estadística: Valores únicos
    QLabel* uniqueLabel = new QLabel(QString("🔗 %1 únicos").arg(uniqueValues.size()));
    uniqueLabel->setFont(QFont("Inter", 11));
    uniqueLabel->setStyleSheet("color: #059669; background-color: #d1fae5; padding: 4px 8px; border-radius: 4px;");
    
    // Estadística: Completitud
    int completeness = records.size() > 0 ? (nonEmptyValues * 100) / records.size() : 0;
    QLabel* completenessLabel = new QLabel(QString("✅ %1% completo").arg(completeness));
    completenessLabel->setFont(QFont("Inter", 11));
    QString completenessColor = completeness >= 80 ? "#059669" : (completeness >= 50 ? "#d97706" : "#dc2626");
    QString completenessBackground = completeness >= 80 ? "#d1fae5" : (completeness >= 50 ? "#fef3c7" : "#fee2e2");
    completenessLabel->setStyleSheet(QString("color: %1; background-color: %2; padding: 4px 8px; border-radius: 4px;").arg(completenessColor).arg(completenessBackground));
    
    statsLayout->addWidget(uniqueLabel);
    statsLayout->addWidget(completenessLabel);
    statsLayout->addStretch();
    
    layout->addWidget(fieldInfo);
    layout->addStretch();
    layout->addWidget(stats);
    
    card->setStyleSheet(
        "QWidget {"
        "background-color: #f9fafb;"
        "border-radius: 8px;"
        "border: 1px solid #e5e7eb;"
        "}"
    );
    
    return card;
}