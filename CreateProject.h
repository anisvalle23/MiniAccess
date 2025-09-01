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
#include <QScrollArea>
#include <QGridLayout>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
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
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void createProjectsArea();
    void loadAndDisplayProjects();
    void createGridProjectCard(const QString &projectName, const QString &lastModified, int tableCount);
    void createListProjectItem(const QString &projectName, const QString &lastModified, int tableCount);
    void clearProjectsDisplay();
    void onProjectCardClicked(const QString &projectName);

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
    
    // Área de proyectos
    QWidget *projectsAreaWidget;
    QVBoxLayout *projectsAreaLayout;
    QScrollArea *projectsScrollArea;
    QWidget *projectsContainer;
    QGridLayout *projectsGridLayout;
    QVBoxLayout *projectsListLayout;
    
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
