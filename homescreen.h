#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QTimer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QEasingCurve>
#include <QGraphicsDropShadowEffect>
#include <QPaintEvent>
#include <QBrush>
#include <QPen>
#include <QRadialGradient>
#include <QtMath>
#include <QFrame>
#include <QToolTip>
#include <QGraphicsRotation>
#include "ThemeManager.h"
#include "ThemePopover.h"
#include "CreateProject.h"

// Widget personalizado para el fondo animado
class AnimatedBackground : public QWidget
{
    Q_OBJECT

public:
    explicit AnimatedBackground(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateAnimation();

private:
    QTimer *animationTimer;
    QPointF mousePos;
    float blob1X, blob1Y, blob2X, blob2Y;
    float animationTime;
    QList<QPointF> particles;
    QList<float> particleOpacity;
};

class HomeScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeScreen(QWidget *parent = nullptr);
    ~HomeScreen();

private slots:
    void onStartProjectClicked();
    void startEntranceAnimations();
    void onSettingsClicked();

private:
    void setupUI();
    void createHeroSection();
    void createHeader();
    void styleComponents();
    void updateLayout(); // Nueva función para forzar re-alineación
    void centerWindow();
    void setupAnimations();
    
    // UI Components principales
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    AnimatedBackground *backgroundWidget;
    
    // Hero Section
    QWidget *heroContainer;
    QVBoxLayout *heroLayout;
    QLabel *badgeLabel;
    QWidget *titleCard;
    QLabel *titleLine1;
    QLabel *titleLine2;
    QLabel *descriptionLabel;
    QPushButton *ctaButton;
    
    // Header ultrafino
    QWidget *headerWidget;
    QFrame *headerLine;
    QPushButton *settingsButton;
    ThemePopover *themePopover;
    
    // Ventana de CreateProject
    CreateProject *createProjectWindow;
    
    // Efectos gráficos
    QGraphicsDropShadowEffect *cardShadow;
    QGraphicsDropShadowEffect *buttonShadow;
    
    // Animaciones
    QPropertyAnimation *badgeAnimation;
    QPropertyAnimation *titleLine1Animation;
    QPropertyAnimation *titleLine2Animation;
    QPropertyAnimation *descriptionAnimation;
    QPropertyAnimation *ctaAnimation;
    QSequentialAnimationGroup *entranceAnimationGroup;
    
    // Efectos de opacidad para animaciones
    QGraphicsOpacityEffect *badgeOpacity;
    QGraphicsOpacityEffect *titleLine1Opacity;
    QGraphicsOpacityEffect *titleLine2Opacity;
    QGraphicsOpacityEffect *descriptionOpacity;
    QGraphicsOpacityEffect *ctaOpacity;
    
    // Flag para controlar animaciones
    bool animationsStarted;
};

#endif // HOMESCREEN_H
