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
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
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

private:
    void setupUI();
    void createHeader();
    void createMainContent();
    void styleComponents();
    void centerWindow();

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
    
    // Estado de vista
    bool isGridView;
};

#endif // CREATEPROJECT_H
