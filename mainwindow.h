#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QPropertyAnimation>
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QScrollArea>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QStackedWidget>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class TableEditor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setProjectName(const QString &projectName);

private slots:
    void onSidebarEnter();
    void onSidebarLeave();
    void onSettingsButtonHover();
    void onSettingsButtonLeave();
    void onSidebarItemClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void createHeader();
    void createSidebar();
    void createMainContent();
    void createHomeView();
    void createWelcomeSection();
    void createDatabaseSection();
    void createExploreSection();
    void switchToView(int viewIndex);
    void updateSidebarSelection(int selectedIndex);
    void styleComponents();
    void setupSidebarAnimation();
    void updateSidebarItems(bool expanded);
    void updateThemeStyles();
    void updateHeaderTheme(bool isDark);
    void updateSidebarTheme(bool isDark);
    void updateMainContentTheme(bool isDark);
    
    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    
        // Header components
    QWidget *headerWidget;
    QHBoxLayout *headerLayout;
    QLabel *projectNameLabel;
    QPushButton *settingsBtn;
    QPropertyAnimation *settingsRotationAnimation;
    
    // Sidebar (overlay)
    QWidget *sidebarOverlay;
    QWidget *sidebarWidget;
    QVBoxLayout *sidebarLayout;
    QPropertyAnimation *sidebarWidthAnimation;
    QPropertyAnimation *sidebarOpacityAnimation;
    QParallelAnimationGroup *sidebarAnimationGroup;
    QGraphicsOpacityEffect *sidebarOpacityEffect;
    QGraphicsDropShadowEffect *sidebarShadowEffect;
    bool sidebarExpanded;
    
    // Sidebar buttons
    QList<QPushButton*> sidebarButtons;
    QList<QLabel*> sidebarLabels;
    QList<QLabel*> sidebarLabelsWidgets;
    
    // Main Content Area (doesn't move when sidebar expands)
    QWidget *mainContainer;
    QVBoxLayout *mainContainerLayout;
    QStackedWidget *stackedWidget;
    
    // Home view
    QWidget *homeView;
    QVBoxLayout *homeViewLayout;
    QWidget *contentArea;
    QVBoxLayout *contentLayout;
    QLabel *welcomeLabel;
    QLabel *descriptionLabel;
    
    // Table Editor view
    TableEditor *tableEditorView;
    
    // Current view tracking
    int currentViewIndex;
    
    // Project data
    QString currentProjectName;
};

#endif // MAINWINDOW_H
