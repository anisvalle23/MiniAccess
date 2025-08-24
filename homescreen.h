#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QRect>

class HomeScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeScreen(QWidget *parent = nullptr);
    ~HomeScreen();

private:
    void setupUI();
    void createSidebar();
    void createCentralArea();
    void styleComponents();
    void centerWindow();
    
    // UI Components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    
    // Sidebar
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QPushButton *btnInicio;
    QPushButton *btnAbrir;
    QLabel *opcionesLabel;
    
    // Central Area
    QWidget *centralArea;
    QVBoxLayout *centralLayout;
    QLabel *greetingLabel;
    QWidget *databaseSection;
    QVBoxLayout *databaseLayout;
    QLabel *databaseIcon;
    QLabel *databaseLabel;
};

#endif // HOMESCREEN_H
