#include "splashscreen.h"
#include "mainwindow.h"

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent), mainWindow(nullptr)
{
    setupUI();
    startAnimation();
    
    // Timer para mostrar la ventana principal después de 4 segundos
    splashTimer = new QTimer(this);
    splashTimer->setSingleShot(true);
    connect(splashTimer, &QTimer::timeout, this, &SplashScreen::showMainWindow);
    splashTimer->start(4000); // 4 segundos para dar tiempo a la animación
    
    // Centrar la ventana después de configurar todo
    centerWindow();
}

SplashScreen::~SplashScreen()
{
    // Qt maneja la limpieza automáticamente
}

void SplashScreen::setupUI()
{
    // Configurar ventana con el mismo tamaño que MainWindow
    setWindowTitle("MiniAccess");
    setFixedSize(1200, 750);
    setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
    
    // Layout principal
    mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Título principal - Solo "MINI ACCESS" (inicialmente invisible)
    titleLabel = new QLabel("MINI ACCESS");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Segoe UI", 60, QFont::Bold));
    titleLabel->setStyleSheet("color: rgba(255, 255, 255, 0); margin: 0px;"); // Invisible al inicio
    
    // Agregar solo el título al layout
    mainLayout->addWidget(titleLabel);
    
    // Estilo de fondo - Color Microsoft Access RGB(164, 55, 58)
    setStyleSheet(
        "QWidget {"
        "    background-color: rgb(164, 55, 58);"
        "    border: none;"
        "}"
    );
}

void SplashScreen::centerWindow()
{
    // Centrar la ventana en la pantalla usando el método más robusto
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.x() + (screenGeometry.width() - width()) / 2;
        int y = screenGeometry.y() + (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}

void SplashScreen::startAnimation()
{
    // Iniciar con el texto invisible
    titleLabel->setStyleSheet("color: rgba(255, 255, 255, 0); margin: 0px;");
    
    // Efecto de opacidad para el texto
    opacityEffect = new QGraphicsOpacityEffect(titleLabel);
    titleLabel->setGraphicsEffect(opacityEffect);
    
    // Animación de fade in para el texto - más lenta y suave
    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(2000); // 2 segundos para una aparición más dramática
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    
    // Cuando termine la animación, establecer el color blanco final
    connect(fadeAnimation, &QPropertyAnimation::finished, [this]() {
        titleLabel->setStyleSheet("color: white; margin: 0px;");
    });
    
    // Iniciar la animación después de 500ms para dar tiempo al fondo
    QTimer::singleShot(500, [this]() {
        fadeAnimation->start();
    });
}

void SplashScreen::showMainWindow()
{
    // Crear y mostrar la ventana principal
    mainWindow = new MainWindow();
    mainWindow->show();
    
    // Cerrar la pantalla de bienvenida
    this->close();
}
