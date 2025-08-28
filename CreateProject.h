#ifndef CREATEPROJECT_H
#define CREATEPROJECT_H

#include <QtWidgets>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QWidget>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QScreen>
#include <QApplication>
#include <QTimer>
#include "ThemePopover.h"

class CreateProject : public QMainWindow
{
    Q_OBJECT

public:
    explicit CreateProject(QWidget *parent = nullptr);
    ~CreateProject();

private slots:
    void onSettingsClicked();
    void onNewProjectClicked();
    void onGridViewClicked();
    void onListViewClicked();
    void onCreateProjectConfirmed();
    void onCreateProjectCancelled();

private:
    void setupUI();
    void createHeader();
    void createMainContent();
    void styleComponents();
    void centerWindow();
    void showNewProjectModal();
    void hideNewProjectModal();
    void styleModalComponents();
    void navigateToProjectView(const QString &projectName);
    void createMainContentArea(const QString &projectName, QHBoxLayout *parentLayout);
    void createDemoTable(QVBoxLayout *parentLayout);

    // Widgets principales
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    
    // Header
    QWidget *headerWidget;
    QPushButton *settingsButton;
    QFrame *headerLine;
    
    // Contenido principal
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    QLabel *titleLabel;
    
    // Toolbar
    QWidget *toolbarWidget;
    QHBoxLayout *toolbarLayout;
    QPushButton *newProjectButton;
    QLineEdit *searchBar;
    QPushButton *gridViewButton;
    QPushButton *listViewButton;
    
    // Theme popover
    ThemePopover *themePopover;
    
    // Modal para crear proyecto
    QWidget *modalOverlay;
    QWidget *modalDialog;
    QLabel *modalTitleLabel;
    QLabel *hintLabel;
    QLabel *inputLabel;
    QLineEdit *projectNameInput;
    QPushButton *createButton;
    QPushButton *cancelButton;
    QGraphicsBlurEffect *backgroundBlur;
    QGraphicsDropShadowEffect *cardShadow;
    
    // Estado de vista y modal
    bool isGridView;
    bool isModalVisible;
    bool isAnimating;
};

#endif // CREATEPROJECT_H
