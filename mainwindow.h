#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QFont>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QScreen>
#include <QRect>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void createSidebar();
    void createCentralArea();
    void styleComponents();
    
    // UI Components
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    
    // Sidebar
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QPushButton *btnInicio;
    QPushButton *btnTablas;
    QPushButton *btnRegistros;
    QPushButton *btnConsultas;
    QPushButton *btnRelaciones;
    QPushButton *btnConfiguracion;
    
    // Central Area
    QWidget *centralArea;
    QVBoxLayout *centralLayout;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
};

#endif // MAINWINDOW_H
