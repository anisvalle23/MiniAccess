#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow()
{
    // Qt maneja la limpieza automáticamente con el parent-child system
}

void MainWindow::setupUI()
{
    // Configurar ventana principal
    setWindowTitle("MiniAccess");
    setFixedSize(1200, 750);
    
    // Centrar la ventana en la pantalla
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - 1200) / 2;
    int y = (screenGeometry.height() - 750) / 2;
    move(x, y);
    
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

void MainWindow::createSidebar()
{
    // Frame de la barra lateral
    sidebar = new QFrame();
    sidebar->setFixedWidth(200);
    sidebar->setFrameStyle(QFrame::NoFrame);
    
    // Layout vertical para la barra lateral
    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(2);
    sidebarLayout->setContentsMargins(15, 20, 15, 20);
    
    // Crear botones con iconos simulados
    btnInicio = new QPushButton("🏠  Inicio");
    btnTablas = new QPushButton("📊  Tablas");
    btnRegistros = new QPushButton("📝  Registros");
    btnConsultas = new QPushButton("🔍  Consultas");
    btnRelaciones = new QPushButton("🔗  Relaciones");
    btnConfiguracion = new QPushButton("⚙️  Configuración");
    
    // Configurar propiedades de los botones
    QList<QPushButton*> buttons = {btnInicio, btnTablas, btnRegistros, 
                                   btnConsultas, btnRelaciones, btnConfiguracion};
    
    for (QPushButton* btn : buttons) {
        btn->setFixedHeight(45);
        btn->setFont(QFont("Segoe UI", 10, QFont::Normal));
        btn->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    border: none;"
            "    color: white;"
            "    text-align: left;"
            "    padding-left: 15px;"
            "    border-radius: 8px;"
            "}"
            "QPushButton:hover {"
            "    background-color: rgba(255, 255, 255, 0.1);"
            "    border: 1px solid rgba(255, 255, 255, 0.2);"
            "}"
            "QPushButton:pressed {"
            "    background-color: rgba(255, 255, 255, 0.2);"
            "}"
        );
        sidebarLayout->addWidget(btn);
    }
    
    // Agregar espacio flexible al final
    sidebarLayout->addStretch();
}

void MainWindow::createCentralArea()
{
    // Widget del área central
    centralArea = new QWidget();
    
    // Layout vertical para el área central
    centralLayout = new QVBoxLayout(centralArea);
    centralLayout->setAlignment(Qt::AlignCenter);
    centralLayout->setSpacing(10);
    centralLayout->setContentsMargins(50, 50, 50, 50);
    
    // Título principal
    titleLabel = new QLabel("MiniAccess");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Segoe UI", 48, QFont::Bold));
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    
    // Subtítulo
    subtitleLabel = new QLabel("Gestor de Base de Datos");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setFont(QFont("Segoe UI", 18, QFont::Normal));
    subtitleLabel->setStyleSheet("color: #7f8c8d; margin-top: 0px;");
    
    // Agregar al layout
    centralLayout->addWidget(titleLabel);
    centralLayout->addWidget(subtitleLabel);
    
    // Agregar espacio flexible
    centralLayout->addStretch();
}

void MainWindow::styleComponents()
{
    // Estilo de la barra lateral (color burdeos)
    sidebar->setStyleSheet(
        "QFrame {"
        "    background-color: #800020;"
        "    border-right: 1px solid #600015;"
        "}"
    );
    
    // Estilo del área central
    centralArea->setStyleSheet(
        "QWidget {"
        "    background-color: #f8f9fa;"
        "}"
    );
    
    // Estilo general de la ventana
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f8f9fa;"
        "}"
    );
}
