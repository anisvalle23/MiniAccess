#include "CreateProject.h"
#include "ThemeManager.h"
#include "ThemeTokens.h"
#include <QDebug>

CreateProject::CreateProject(QWidget *parent)
    : QMainWindow(parent), isGridView(true)
{
    qDebug() << "DEBUG: CreateProject constructor iniciado";
    
    try {
        qDebug() << "DEBUG: Creando themePopover";
        // Crear popover de temas
        themePopover = new ThemePopover(this);
        qDebug() << "DEBUG: themePopover creado exitosamente";
        
        qDebug() << "DEBUG: Llamando setupUI()";
        setupUI();
        qDebug() << "DEBUG: setupUI() completado";
        
        qDebug() << "DEBUG: Llamando centerWindow()";
        centerWindow();
        qDebug() << "DEBUG: centerWindow() completado";
        
        qDebug() << "DEBUG: Conectando señales de tema";
        // Conectar al sistema de temas para actualizar estilos
        connect(&ThemeManager::instance(), &ThemeManager::themeChanged, 
                this, [this](ThemeManager::Theme) {
            qDebug() << "DEBUG: Theme changed signal received";
            styleComponents();
            QApplication::processEvents();
            update();
        });
        qDebug() << "DEBUG: Señales conectadas exitosamente";
        
    } catch (const std::exception& e) {
        qDebug() << "DEBUG: Excepción en CreateProject constructor:" << e.what();
    } catch (...) {
        qDebug() << "DEBUG: Excepción desconocida en CreateProject constructor";
    }
    
    qDebug() << "DEBUG: CreateProject constructor terminado";
}

CreateProject::~CreateProject()
{
    // Qt maneja la limpieza automáticamente
}

void CreateProject::setupUI()
{
    qDebug() << "DEBUG: CreateProject::setupUI() iniciado";
    
    try {
        qDebug() << "DEBUG: Configurando ventana principal";
        // Configurar ventana principal
        setWindowTitle("Mini Access - Proyectos");
        setFixedSize(1200, 750);
        
        qDebug() << "DEBUG: Creando widget central";
        // Widget central principal
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        qDebug() << "DEBUG: Creando layout principal";
        // Layout principal vertical
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        
        qDebug() << "DEBUG: Llamando createHeader()";
        // Crear header
        createHeader();
        qDebug() << "DEBUG: createHeader() completado";
        
        qDebug() << "DEBUG: Llamando createMainContent()";
        // Crear contenido principal
        createMainContent();
        qDebug() << "DEBUG: createMainContent() completado";
        
        qDebug() << "DEBUG: Agregando widgets al layout";
        // Añadir elementos al layout principal
        mainLayout->addWidget(headerWidget);
        mainLayout->addWidget(headerLine);
        mainLayout->addWidget(contentWidget);
        
        qDebug() << "DEBUG: Llamando styleComponents()";
        // Aplicar estilos
        styleComponents();
        qDebug() << "DEBUG: styleComponents() completado";
        
    } catch (const std::exception& e) {
        qDebug() << "DEBUG: Excepción en setupUI():" << e.what();
    } catch (...) {
        qDebug() << "DEBUG: Excepción desconocida en setupUI()";
    }
    
    qDebug() << "DEBUG: CreateProject::setupUI() terminado";
}

void CreateProject::createHeader()
{
    // Widget contenedor del header
    headerWidget = new QWidget(centralWidget);
    headerWidget->setFixedHeight(50);
    headerWidget->setStyleSheet("background: transparent;");
    
    // Layout horizontal del header
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(40, 0, 40, 0);
    headerLayout->setSpacing(0);
    
    // Espaciador para empujar el botón a la derecha
    headerLayout->addStretch();
    
    // Botón de configuraciones con ícono gear
    settingsButton = new QPushButton();
    settingsButton->setFixedSize(32, 32);
    settingsButton->setText("⚙");
    settingsButton->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    border: none;"
        "    color: #A4373A;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    border-radius: 16px;"
        "    padding: 0;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(164, 55, 58, 0.08);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(164, 55, 58, 0.15);"
        "    transform: translateY(0px);"
        "}"
    );
    
    // Tooltip para el botón
    settingsButton->setToolTip("Configuraciones");
    
    // Agregar botón al layout
    headerLayout->addWidget(settingsButton);
    
    // Línea divisoria ultrafina
    headerLine = new QFrame();
    headerLine->setFrameShape(QFrame::HLine);
    headerLine->setFixedHeight(1);
    headerLine->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 rgba(164, 55, 58, 0.0), "
        "        stop:0.5 rgba(164, 55, 58, 0.25), "
        "        stop:1 rgba(164, 55, 58, 0.0));"
        "    border: none;"
        "    margin: 0 60px;"
        "}"
    );
    
    // Conectar señales
    connect(settingsButton, &QPushButton::clicked, this, &CreateProject::onSettingsClicked);
    
    // Configurar popover para usar este botón
    themePopover->setSettingsButton(settingsButton);
}

void CreateProject::createMainContent()
{
    // Widget contenedor del contenido principal
    contentWidget = new QWidget();
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Layout vertical del contenido
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 40, 40, 40);
    contentLayout->setSpacing(24);
    
    // Título principal "Proyectos"
    titleLabel = new QLabel("Proyectos");
    titleLabel->setFont(QFont("Inter", 28, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignLeft);
    
    // Toolbar con controles
    toolbarWidget = new QWidget();
    toolbarWidget->setFixedHeight(60);
    
    toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(20);
    
    // Botón "Nuevo Proyecto"
    newProjectButton = new QPushButton("Nuevo Proyecto");
    newProjectButton->setFixedSize(160, 40);
    newProjectButton->setFont(QFont("Inter", 14, QFont::Medium));
    newProjectButton->setCursor(Qt::PointingHandCursor);
    
    // Barra de búsqueda
    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Buscar proyecto...");
    searchBar->setFixedHeight(40);
    searchBar->setFont(QFont("Inter", 13));
    searchBar->setMinimumWidth(300);
    
    // Espaciador flexible
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // Botones de vista (cuadrícula/lista)
    gridViewButton = new QPushButton("⚏");
    gridViewButton->setFixedSize(40, 40);
    gridViewButton->setFont(QFont("Inter", 16, QFont::Bold));
    gridViewButton->setCursor(Qt::PointingHandCursor);
    gridViewButton->setToolTip("Vista en cuadrícula");
    
    listViewButton = new QPushButton("☰");
    listViewButton->setFixedSize(40, 40);
    listViewButton->setFont(QFont("Inter", 16, QFont::Bold));
    listViewButton->setCursor(Qt::PointingHandCursor);
    listViewButton->setToolTip("Vista en lista");
    
    // Agregar elementos al toolbar
    toolbarLayout->addWidget(searchBar);
    toolbarLayout->addWidget(spacer);
    toolbarLayout->addWidget(listViewButton);
    toolbarLayout->addWidget(gridViewButton);
    toolbarLayout->addWidget(newProjectButton);
    
    // Agregar título y toolbar al contenido principal
    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(toolbarWidget);
    
    // Espaciador para el resto del contenido (donde irán los proyectos)
    contentLayout->addStretch();
    
    // Conectar señales
    connect(newProjectButton, &QPushButton::clicked, this, &CreateProject::onNewProjectClicked);
    connect(gridViewButton, &QPushButton::clicked, this, &CreateProject::onGridViewClicked);
    connect(listViewButton, &QPushButton::clicked, this, &CreateProject::onListViewClicked);
}

void CreateProject::styleComponents()
{
    bool isDark = ThemeManager::instance().isDark();
    QString accentColor = isDark ? ThemeTokens::Dark::ACCENT_ADJUSTED : ThemeTokens::ACCENT_PRIMARY;
    QString hoverColor = ThemeTokens::ThemeHelper::getButtonHoverColor(isDark);
    QString pressedColor = ThemeTokens::ThemeHelper::getButtonPressedColor(isDark);
    QString surfaceColor = isDark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString borderColor = isDark ? ThemeTokens::Dark::BORDER : ThemeTokens::Light::BORDER;
    QString textPrimary = isDark ? ThemeTokens::Dark::TEXT_PRIMARY : ThemeTokens::Light::TEXT_PRIMARY;
    QString textSecondary = isDark ? ThemeTokens::Dark::TEXT_SECONDARY : ThemeTokens::Light::TEXT_SECONDARY;
    QString backgroundColor = isDark ? ThemeTokens::Dark::BACKGROUND : ThemeTokens::Light::BACKGROUND;
    
    // Fondo principal de la ventana
    centralWidget->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
    ).arg(backgroundColor, textPrimary));
    
    // Estilo del título principal
    titleLabel->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    margin-bottom: 20px;"
        "    margin-top: 10px;"
        "    font-weight: bold;"
        "}"
    ).arg(textPrimary));
    
    // Estilo del botón "Nuevo Proyecto"
    newProjectButton->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-weight: 500;"
        "    padding: 8px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
    ).arg(accentColor, hoverColor, pressedColor));
    
    // Estilo de la barra de búsqueda
    searchBar->setStyleSheet(QString(
        "QLineEdit {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    color: %3;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %4;"
        "}"
        "QLineEdit::placeholder {"
        "    color: %5;"
        "}"
    ).arg(surfaceColor, borderColor, textPrimary, accentColor, textSecondary));
    
    // Estilo de los botones de vista
    QString viewButtonStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "    border-radius: 8px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: %4;"
        "    border-color: %5;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %6;"
        "}"
    ).arg(surfaceColor, textSecondary, borderColor, 
          isDark ? ThemeTokens::Dark::SURFACE_HOVER : ThemeTokens::Light::SURFACE_HOVER,
          accentColor, pressedColor);
    
    // Aplicar estilos y estados de los botones de vista
    gridViewButton->setStyleSheet(viewButtonStyle);
    listViewButton->setStyleSheet(viewButtonStyle);
    
    // Resaltar el botón activo
    if (isGridView) {
        gridViewButton->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: white;"
            "    border: 1px solid %1;"
            "    border-radius: 8px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: %2;"
            "}"
        ).arg(accentColor, hoverColor));
    } else {
        listViewButton->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: white;"
            "    border: 1px solid %1;"
            "    border-radius: 8px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: %2;"
            "}"
        ).arg(accentColor, hoverColor));
    }
}

void CreateProject::centerWindow()
{
    // Centrar ventana en pantalla
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - 1200) / 2;
        int y = (screenGeometry.height() - 750) / 2;
        move(x, y);
    }
}

void CreateProject::onSettingsClicked()
{
    // Verificar si el popover está visible para hacer toggle
    if (themePopover->isPopoverVisible()) {
        themePopover->hidePopover();
        return;
    }
    
    // Crear efecto de rotación en el ícono
    QRect originalGeometry = settingsButton->geometry();
    QRect liftedGeometry = originalGeometry.adjusted(0, -1, 0, -1);
    
    QPropertyAnimation *liftAnimation = new QPropertyAnimation(settingsButton, "geometry");
    liftAnimation->setDuration(150);
    liftAnimation->setStartValue(originalGeometry);
    liftAnimation->setEndValue(liftedGeometry);
    liftAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    QPropertyAnimation *returnAnimation = new QPropertyAnimation(settingsButton, "geometry");
    returnAnimation->setDuration(100);
    returnAnimation->setStartValue(liftedGeometry);
    returnAnimation->setEndValue(originalGeometry);
    returnAnimation->setEasingCurve(QEasingCurve::InQuad);
    
    QSequentialAnimationGroup *fullSequence = new QSequentialAnimationGroup();
    fullSequence->addAnimation(liftAnimation);
    fullSequence->addAnimation(returnAnimation);
    fullSequence->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Mostrar popover de tema
    QPoint popoverPosition = settingsButton->mapToGlobal(QPoint(0, settingsButton->height()));
    themePopover->showPopover(popoverPosition);
}

void CreateProject::onNewProjectClicked()
{
    // Efecto de animación en el botón
    QPropertyAnimation *scaleAnimation = new QPropertyAnimation(newProjectButton, "geometry");
    scaleAnimation->setDuration(100);
    QRect originalGeometry = newProjectButton->geometry();
    QRect pressedGeometry = originalGeometry.adjusted(1, 1, -1, -1);
    
    scaleAnimation->setStartValue(originalGeometry);
    scaleAnimation->setEndValue(pressedGeometry);
    scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    QPropertyAnimation *returnAnimation = new QPropertyAnimation(newProjectButton, "geometry");
    returnAnimation->setDuration(100);
    returnAnimation->setStartValue(pressedGeometry);
    returnAnimation->setEndValue(originalGeometry);
    returnAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    QSequentialAnimationGroup *clickGroup = new QSequentialAnimationGroup();
    clickGroup->addAnimation(scaleAnimation);
    clickGroup->addAnimation(returnAnimation);
    clickGroup->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Aquí agregar lógica para crear nuevo proyecto
    qDebug() << "Crear nuevo proyecto";
}

void CreateProject::onGridViewClicked()
{
    if (!isGridView) {
        isGridView = true;
        styleComponents(); // Actualizar estilos para resaltar el botón activo
        qDebug() << "Vista en cuadrícula activada";
    }
}

void CreateProject::onListViewClicked()
{
    if (isGridView) {
        isGridView = false;
        styleComponents(); // Actualizar estilos para resaltar el botón activo
        qDebug() << "Vista en lista activada";
    }
}
