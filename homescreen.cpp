#include "homescreen.h"

HomeScreen::HomeScreen(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    centerWindow();
}

HomeScreen::~HomeScreen()
{
    // Qt se encarga de la limpieza automáticamente
}

void HomeScreen::setupUI()
{
    // Configurar ventana principal
    setWindowTitle("MiniAccess - Inicio");
    setFixedSize(1200, 750);
    
    // Widget central principal
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Layout principal horizontal
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Crear componentes
    createSidebar();
    createCentralArea();
    
    // Añadir al layout principal
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(centralArea);
    
    // Aplicar estilos
    styleComponents();
}

void HomeScreen::createSidebar()
{
    // Frame de la barra lateral
    sidebar = new QFrame();
    sidebar->setFixedWidth(200);
    sidebar->setFrameStyle(QFrame::NoFrame);
    
    // Layout vertical para la barra lateral
    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(15);
    sidebarLayout->setContentsMargins(20, 30, 20, 30);
    
    // Botón Inicio - Con emoji de casa
    btnInicio = new QPushButton("🏠  Inicio");
    btnInicio->setFixedHeight(55);
    btnInicio->setFont(QFont("Segoe UI", 14, QFont::Medium));
    btnInicio->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.15);"
        "    border: 1px solid rgba(255, 255, 255, 0.3);"
        "    color: white;"
        "    text-align: left;"
        "    padding-left: 20px;"
        "    border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.25);"
        "    border: 1px solid rgba(255, 255, 255, 0.4);"
        "}"
    );
    
    // Botón Abrir - Con emoji de carpeta
    btnAbrir = new QPushButton("📂  Abrir");
    btnAbrir->setFixedHeight(55);
    btnAbrir->setFont(QFont("Segoe UI", 14, QFont::Medium));
    btnAbrir->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    border: 1px solid transparent;"
        "    color: white;"
        "    text-align: left;"
        "    padding-left: 20px;"
        "    border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.1);"
        "    border: 1px solid rgba(255, 255, 255, 0.2);"
        "}"
    );
    
    // Agregar espacio
    sidebarLayout->addWidget(btnInicio);
    sidebarLayout->addWidget(btnAbrir);
    sidebarLayout->addSpacing(50);
    
    // Label "Opciones" en la parte inferior
    opcionesLabel = new QLabel("Opciones");
    opcionesLabel->setFont(QFont("Segoe UI", 11, QFont::Normal));
    opcionesLabel->setStyleSheet("color: rgba(255, 255, 255, 0.8); padding-left: 20px;");
    
    // Agregar espacio flexible y opciones al final
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(opcionesLabel);
}

void HomeScreen::createCentralArea()
{
    // Widget del área central
    centralArea = new QWidget();
    
    // Layout vertical para el área central
    centralLayout = new QVBoxLayout(centralArea);
    centralLayout->setAlignment(Qt::AlignTop);
    centralLayout->setSpacing(30);
    centralLayout->setContentsMargins(0, 50, 0, 60);
    
    // Contenedor para el saludo y la línea
    QWidget *greetingContainer = new QWidget();
    greetingContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *greetingContainerLayout = new QVBoxLayout(greetingContainer);
    greetingContainerLayout->setSpacing(15);
    greetingContainerLayout->setContentsMargins(40, 0, 40, 0);
    
    // Saludo "Buenos días"
    greetingLabel = new QLabel("Buenos días");
    greetingLabel->setAlignment(Qt::AlignLeft);
    greetingLabel->setFont(QFont("Segoe UI", 32, QFont::Normal));
    greetingLabel->setStyleSheet("color: #2c3e50; margin: 0px;");
    
    // Línea separadora debajo de "Buenos días" - Extendida completamente
    QFrame *separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Plain);
    separatorLine->setFixedHeight(3);
    separatorLine->setStyleSheet(
        "QFrame { "
        "    background-color: #6c757d; "
        "    border: none; "
        "    margin: 0px; "
        "}"
    );
    
    // Agregar al contenedor del saludo
    greetingContainerLayout->addWidget(greetingLabel);
    greetingContainerLayout->addWidget(separatorLine);
    
    // Sección de base de datos - Mejorada
    databaseSection = new QWidget();
    databaseSection->setFixedSize(350, 240);
    databaseSection->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 15px;"
        "    box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);"
        "}"
        "QWidget:hover {"
        "    border: 2px solid #adb5bd;"
        "    background-color: #f8f9fa;"
        "}"
    );
    
    // Layout para la sección de base de datos
    databaseLayout = new QVBoxLayout(databaseSection);
    databaseLayout->setAlignment(Qt::AlignCenter);
    databaseLayout->setSpacing(25);
    databaseLayout->setContentsMargins(40, 40, 40, 40);
    
    // Icono de base de datos - Con emoji de documento
    databaseIcon = new QLabel("📄");
    databaseIcon->setAlignment(Qt::AlignCenter);
    databaseIcon->setFont(QFont("Arial", 60, QFont::Bold));
    databaseIcon->setStyleSheet("color: #ffc107; margin-bottom: 5px;");
    
    // Texto "Base de datos en blanco" - Mejorado
    databaseLabel = new QLabel("Base de datos en blanco");
    databaseLabel->setAlignment(Qt::AlignCenter);
    databaseLabel->setFont(QFont("Segoe UI", 16, QFont::Medium));
    databaseLabel->setStyleSheet(
        "color: #343a40; "
        "line-height: 1.4; "
        "background: transparent; "
        "border: none;"
    );
    databaseLabel->setWordWrap(true);
    
    // Agregar elementos a la sección de base de datos
    databaseLayout->addWidget(databaseIcon);
    databaseLayout->addWidget(databaseLabel);
    
    // Agregar elementos al layout central
    centralLayout->addWidget(greetingContainer);
    centralLayout->addSpacing(20);
    
    // Contenedor para centrar la sección de base de datos
    QWidget *databaseContainer = new QWidget();
    QHBoxLayout *databaseContainerLayout = new QHBoxLayout(databaseContainer);
    databaseContainerLayout->setContentsMargins(40, 0, 40, 0);
    databaseContainerLayout->addWidget(databaseSection);
    databaseContainerLayout->addStretch();
    
    centralLayout->addWidget(databaseContainer);
    centralLayout->addStretch();
}

void HomeScreen::styleComponents()
{
    // Estilo de la barra lateral (color burdeos RGB(164, 55, 58))
    sidebar->setStyleSheet(
        "QFrame {"
        "    background-color: rgb(164, 55, 58);"
        "    border-right: 1px solid rgb(140, 45, 48);"
        "}"
    );
    
    // Estilo del área central
    centralArea->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "}"
    );
    
    // Estilo general de la ventana
    setStyleSheet(
        "QMainWindow {"
        "    background-color: white;"
        "}"
    );
}

void HomeScreen::centerWindow()
{
    // Centrar la ventana en la pantalla
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.x() + (screenGeometry.width() - width()) / 2;
        int y = screenGeometry.y() + (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}
