#include "splashscreen.h"
#include "mainwindow.h"

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent), mainWindow(nullptr)
{
    setupUI();
    
    // Procesar eventos para asegurar que la ventana se dibuje
    QApplication::processEvents();
    
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
    // Usar flags más seguras
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, false); // No eliminar automáticamente
    
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
    // Centrar la ventana usando un método más compatible con Qt Creator
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.x() + (screenGeometry.width() - width()) / 2;
        int y = screenGeometry.y() + (screenGeometry.height() - height()) / 2;
        move(x, y);
    } else {
        // Fallback si no se puede obtener la pantalla
        move(100, 100);
    }
    
    // Asegurar que la ventana esté visible y al frente
    show();
    raise();
    activateWindow();
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
    mainWindow->raise();
    mainWindow->activateWindow();
    
    // Ocultar el splash screen en lugar de cerrarlo inmediatamente
    this->hide();
    
    // Cerrar la pantalla de bienvenida después de un delay más largo
    QTimer::singleShot(500, [this]() {
        this->deleteLater();
    });
}



