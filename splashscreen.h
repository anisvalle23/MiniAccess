#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class MainWindow;

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

private slots:
    void showMainWindow();

private:
    void setupUI();
    void centerWindow();
    void startAnimation();
    
    // UI Components
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    
    // Animation
    QPropertyAnimation *fadeAnimation;
    QGraphicsOpacityEffect *opacityEffect;
    
    // Timer
    QTimer *splashTimer;
    
    // Main window reference
    MainWindow *mainWindow;
};

#endif // SPLASHSCREEN_H
