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
#include "ThemeManager.h"

class MainWindow;

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

private:
    void setupUI();
    void createHeaderSection();
    void createToolbar();
    void createReportsList();
    void createReportViewer();
    void createReportFilters();
    void styleComponents();
    void loadReports();
    void updateReportsList();
    void generatePreview();

    // UI Components
    QVBoxLayout *mainLayout;
    
    // Header
    QWidget *headerWidget;
    QVBoxLayout *headerLayout;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    
    // Toolbar
    QWidget *toolbarWidget;
    QHBoxLayout *toolbarLayout;
    QPushButton *createReportBtn;
    QPushButton *editReportBtn;
    QPushButton *deleteReportBtn;
    QPushButton *generateReportBtn;
    QPushButton *exportReportBtn;
    QPushButton *refreshBtn;
    
    // Content Area
    QWidget *contentWidget;
    QHBoxLayout *contentLayout;
    QSplitter *mainSplitter;
    
    // Reports List (lado izquierdo)
    QWidget *reportsListWidget;
    QVBoxLayout *reportsListLayout;
    QLabel *reportsListLabel;
    QListWidget *reportsList;
    
    // Report Viewer Area (lado derecho)
    QWidget *reportViewerWidget;
    QVBoxLayout *reportViewerLayout;
    
    // Filters Section
    QWidget *filtersWidget;
    QHBoxLayout *filtersLayout;
    QLabel *reportTypeLabel;
    QComboBox *reportTypeCombo;
    QLabel *dateFromLabel;
    QDateEdit *dateFromEdit;
    QLabel *dateToLabel;
    QDateEdit *dateToEdit;
    QPushButton *applyFiltersBtn;
    
    // Report Content
    QStackedWidget *reportStackedWidget;
    
    // Empty state
    QWidget *emptyStateWidget;
    QVBoxLayout *emptyStateLayout;
    QLabel *emptyStateLabel;
    QLabel *emptyStateDescription;
    
    // Report preview/display
    QWidget *reportDisplayWidget;
    QVBoxLayout *reportDisplayLayout;
    QLabel *reportTitleLabel;
    QScrollArea *reportScrollArea;
    QWidget *reportContentWidget;
    QVBoxLayout *reportContentLayout;
    
    // Report data display
    QTableWidget *reportTable;
    QTextEdit *reportSummary;
    
    // Reference to main window
    MainWindow *m_mainWindow;
    
    // Current selection
    QString currentSelectedReport;
    
    // Effects and animations
    QGraphicsDropShadowEffect *headerShadow;
    QGraphicsDropShadowEffect *toolbarShadow;
    QPropertyAnimation *fadeAnimation;
};

#endif // REPORTESVIEW_H