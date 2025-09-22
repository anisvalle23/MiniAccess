#ifndef REPORTESVIEW_H
#define REPORTESVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QFont>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QJsonObject>
#include <QJsonArray>
#include "ThemeManager.h"

// Forward declarations
class MainWindow;
class QCheckBox;

class ReportesView : public QWidget
{
    Q_OBJECT

public:
    explicit ReportesView(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~ReportesView();

    void updateTheme();

public slots:
    void refreshView();

private slots:
    void onCreateReportClicked();
    void onReportSelected();
    void onEditReportClicked();
    void onDeleteReportClicked();
    void onGenerateReportClicked();
    void onExportReportClicked();
    void onReportTypeChanged();
    void onTableSelectionChanged();

private:
    void setupUI();
    void createHeaderSection();
    void createTableSelector();
    void createReportContent();
    void createEmptyState();
    void createReportCard();
    void createReportDataItems();
    void createDataRow(const QString& city, const QString& count);
    void createTotalSection();
    void createToolbar();
    void createReportsList();
    void createReportViewer();
    void createReportFilters();
    void styleComponents();
    void loadReports();
    void loadAvailableTables();
    void generateReportForTable(const QString& tableName);
    void generateAdvancedReport(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records, int totalRecords);
    void createStatsCard(const QString& title, const QString& value, const QString& description, const QString& color);
    void createChartSection(const QString& title, const QJsonArray& records, const QJsonObject& tableMeta);
    void generateBasicAnalysis(const std::vector<QJsonObject>& records, const QJsonObject& tableMeta);
    void generateBasicAnalysisFromArray(const QJsonArray& records, const QJsonObject& tableMeta);
    
    // New modern design methods
    void createNoDataState();
    void createMainStatsCards(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records, int totalRecords);
    void createDetailedFieldAnalysis(const QString& tableName, const QJsonObject& tableMeta, const QJsonArray& records);
    void createInsightsSection(const QString& tableName, int totalRecords);
    void createSeparator();
    QWidget* createModernStatsCard(const QString& icon, const QString& title, const QString& value, const QString& subtitle, const QString& color);
    QWidget* createFieldAnalysisCard(const QString& fieldName, const QString& fieldType, const QJsonArray& records);
    void showEmptyState();
    void updateReportsList();
    void generatePreview();
    void generateSingleTableReport(const std::string& tableName);
    void generateSummaryReport();
    void generateCompleteReport();
    void generatePreviewWithSelectedFields(const std::string& tableName, 
                                          const QList<QCheckBox*>& checkboxes, 
                                          const QJsonObject& tableMeta);
    QJsonObject readTableMetadata(const std::string& tableName);
    
    // Export methods
    void exportToCSV(const QString& fileName);
    void exportToHTML(const QString& fileName);
    void exportToText(const QString& fileName);

    // UI Components
    QVBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    
    // Header
    QWidget *headerWidget;
    QVBoxLayout *headerLayout;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    
    // Table Selector
    QWidget *tableSelectorWidget;
    QHBoxLayout *tableSelectorLayout;
    QLabel *tableSelectorLabel;
    QComboBox *tableComboBox;
    
    // Report Content
    QWidget *reportContentWidget;
    QVBoxLayout *reportContentLayout;
    QWidget *reportCardWidget;
    QVBoxLayout *reportCardLayout;
    QLabel *reportTitleLabel;
    QWidget *reportDataWidget;
    QVBoxLayout *reportDataLayout;
    
    // Empty State
    QWidget *emptyStateWidget;
    QVBoxLayout *emptyStateLayout;
    QLabel *emptyStateLabel;
    
    // Reference to main window
    MainWindow *m_mainWindow;
    
    // Current selection
    QString currentSelectedReport;
    QString currentSelectedTable;
    
    // Effects and animations
    QGraphicsDropShadowEffect *headerShadow;
    QGraphicsDropShadowEffect *toolbarShadow;
    QPropertyAnimation *fadeAnimation;
};

#endif // REPORTESVIEW_H