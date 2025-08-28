#include "CreateProject.h"
#include "ThemeManager.h"
#include "ThemeTokens.h"
#include "mainwindow.h"
#include <QDebug>

CreateProject::CreateProject(QWidget *parent)
    : QMainWindow(parent), isGridView(true), isModalVisible(false), isAnimating(false)
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
    
    // Actualizar estilos del modal si está visible
    if (modalOverlay && modalOverlay->isVisible()) {
        styleModalComponents();
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
    qDebug() << "=== INICIO onNewProjectClicked() ===";
    qDebug() << "Estado actual - isModalVisible:" << isModalVisible << "isAnimating:" << isAnimating;
    
    // Prevenir múltiples clics mientras el modal está visible o animando
    if (isModalVisible || isAnimating) {
        qDebug() << "Modal ya está visible o animando, ignorando clic";
        qDebug() << "=== FIN onNewProjectClicked() (IGNORADO) ===";
        return;
    }
    
    qDebug() << "Iniciando animación del botón...";
    
    // Verificar que el botón existe
    if (!newProjectButton) {
        qDebug() << "ERROR: newProjectButton es nullptr!";
        qDebug() << "=== FIN onNewProjectClicked() (ERROR) ===";
        return;
    }
    
    qDebug() << "newProjectButton existe, geometry:" << newProjectButton->geometry();
    
    try {
        // Efecto de animación en el botón
        QPropertyAnimation *scaleAnimation = new QPropertyAnimation(newProjectButton, "geometry");
        if (!scaleAnimation) {
            qDebug() << "ERROR: No se pudo crear scaleAnimation";
            return;
        }
        
        scaleAnimation->setDuration(100);
        QRect originalGeometry = newProjectButton->geometry();
        QRect pressedGeometry = originalGeometry.adjusted(1, 1, -1, -1);
        
        qDebug() << "Geometrías - Original:" << originalGeometry << "Pressed:" << pressedGeometry;
        
        scaleAnimation->setStartValue(originalGeometry);
        scaleAnimation->setEndValue(pressedGeometry);
        scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);
        
        QPropertyAnimation *returnAnimation = new QPropertyAnimation(newProjectButton, "geometry");
        if (!returnAnimation) {
            qDebug() << "ERROR: No se pudo crear returnAnimation";
            delete scaleAnimation;
            return;
        }
        
        returnAnimation->setDuration(100);
        returnAnimation->setStartValue(pressedGeometry);
        returnAnimation->setEndValue(originalGeometry);
        returnAnimation->setEasingCurve(QEasingCurve::InCubic);
        
        QSequentialAnimationGroup *clickGroup = new QSequentialAnimationGroup();
        if (!clickGroup) {
            qDebug() << "ERROR: No se pudo crear clickGroup";
            delete scaleAnimation;
            delete returnAnimation;
            return;
        }
        
        clickGroup->addAnimation(scaleAnimation);
        clickGroup->addAnimation(returnAnimation);
        clickGroup->start(QAbstractAnimation::DeleteWhenStopped);
        
        qDebug() << "Animación del botón iniciada correctamente";
        
    } catch (const std::exception& e) {
        qDebug() << "EXCEPCIÓN en animación del botón:" << e.what();
    } catch (...) {
        qDebug() << "EXCEPCIÓN DESCONOCIDA en animación del botón";
    }
    
    qDebug() << "Llamando a showNewProjectModal()...";
    
    // Mostrar modal para crear proyecto
    try {
        showNewProjectModal();
        qDebug() << "showNewProjectModal() ejecutado correctamente";
    } catch (const std::exception& e) {
        qDebug() << "EXCEPCIÓN en showNewProjectModal():" << e.what();
    } catch (...) {
        qDebug() << "EXCEPCIÓN DESCONOCIDA en showNewProjectModal()";
    }
    
    qDebug() << "=== FIN onNewProjectClicked() ===";
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

void CreateProject::showNewProjectModal()
{
    qDebug() << "=== INICIO showNewProjectModal() ===";
    qDebug() << "Estado actual - isModalVisible:" << isModalVisible << "isAnimating:" << isAnimating;
    
    // Prevenir mostrar el modal si ya está visible o animando
    if (isModalVisible || isAnimating) {
        qDebug() << "Modal ya está visible o en proceso, saltando";
        qDebug() << "=== FIN showNewProjectModal() (SALTANDO) ===";
        return;
    }
    
    qDebug() << "Mostrando modal de crear proyecto";
    isAnimating = true;
    qDebug() << "isAnimating establecido a true";
    
    try {
        qDebug() << "Verificando si modalOverlay existe...";
        
        // Crear overlay modal si no existe
        if (!modalOverlay) {
            qDebug() << "modalOverlay no existe, creándolo...";
            
            modalOverlay = new QWidget(this);
            if (!modalOverlay) {
                qDebug() << "ERROR: No se pudo crear modalOverlay";
                isAnimating = false;
                return;
            }
            qDebug() << "modalOverlay creado correctamente";
            
            modalOverlay->setGeometry(0, 0, width(), height());
            modalOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.50);"); // 50% opacity
            modalOverlay->hide();
            qDebug() << "modalOverlay configurado - geometry:" << modalOverlay->geometry();
            
            qDebug() << "Creando modalDialog...";
            
            // Crear el diálogo modal con diseño limpio y minimalista
            modalDialog = new QWidget(modalOverlay);
            if (!modalDialog) {
                qDebug() << "ERROR: No se pudo crear modalDialog";
                isAnimating = false;
                return;
            }
            qDebug() << "modalDialog creado correctamente";
            
            modalDialog->setFixedSize(540, 320);
            modalDialog->setStyleSheet(
                "QWidget {"
                    "background-color: #FFFFFF;"
                    "border-radius: 22px;"
                "}"
            );
            qDebug() << "modalDialog configurado - size:" << modalDialog->size();
            
            qDebug() << "Creando cardShadow...";
            
            // Agregar sombra suave al card
            cardShadow = new QGraphicsDropShadowEffect();
            if (!cardShadow) {
                qDebug() << "ERROR: No se pudo crear cardShadow";
                isAnimating = false;
                return;
            }
            cardShadow->setBlurRadius(32);
            cardShadow->setXOffset(0);
            cardShadow->setYOffset(12);
            cardShadow->setColor(QColor(0, 0, 0, 31)); // 12% opacidad
            modalDialog->setGraphicsEffect(cardShadow);
            qDebug() << "cardShadow aplicado correctamente";
            
            // Centrar el diálogo
            int x = (modalOverlay->width() - modalDialog->width()) / 2;
            int y = (modalOverlay->height() - modalDialog->height()) / 2;
            modalDialog->move(x, y);
            qDebug() << "modalDialog centrado en posición:" << x << "," << y;
            
            qDebug() << "Creando layout principal...";
            
            // Layout principal del diálogo
            QVBoxLayout *dialogLayout = new QVBoxLayout(modalDialog);
            if (!dialogLayout) {
                qDebug() << "ERROR: No se pudo crear dialogLayout";
                isAnimating = false;
                return;
            }
            dialogLayout->setContentsMargins(30, 30, 30, 30);
            dialogLayout->setSpacing(0);
            qDebug() << "dialogLayout configurado correctamente";
            
            qDebug() << "Creando elementos del modal...";
            
            // Título centrado (sin pill ni decoraciones)
            modalTitleLabel = new QLabel("Crear nuevo proyecto");
            if (!modalTitleLabel) {
                qDebug() << "ERROR: No se pudo crear modalTitleLabel";
                isAnimating = false;
                return;
            }
            modalTitleLabel->setFont(QFont("SF Pro Display", 30, 800)); // Peso 800
            modalTitleLabel->setStyleSheet("QLabel { color: #111827; }");
            modalTitleLabel->setAlignment(Qt::AlignCenter);
            dialogLayout->addWidget(modalTitleLabel);
            dialogLayout->addSpacing(28);
            qDebug() << "modalTitleLabel creado correctamente";
            
            // Label del input
            inputLabel = new QLabel("Nombre del proyecto");
            if (!inputLabel) {
                qDebug() << "ERROR: No se pudo crear inputLabel";
                isAnimating = false;
                return;
            }
            inputLabel->setFont(QFont("SF Pro Text", 14, QFont::Medium));
        inputLabel->setStyleSheet("QLabel { color: #4B5563; }");
        dialogLayout->addWidget(inputLabel);
        dialogLayout->addSpacing(12);
        
        // Hint text (debajo del label)
        hintLabel = new QLabel("Ingresa el nombre para tu nuevo proyecto");
        hintLabel->setFont(QFont("SF Pro Text", 13));
        hintLabel->setStyleSheet("QLabel { color: #6B7280; }");
        hintLabel->setAlignment(Qt::AlignLeft);
        dialogLayout->addWidget(hintLabel);
        dialogLayout->addSpacing(10);
        
        // Campo de entrada con diseño refinado
        projectNameInput = new QLineEdit();
        projectNameInput->setPlaceholderText("Mi proyecto increíble");
        projectNameInput->setFixedHeight(50);
        projectNameInput->setFont(QFont("SF Pro Text", 16));
        projectNameInput->setStyleSheet(
            "QLineEdit {"
                "background-color: #FFFFFF;"
                "border: 1px solid #E5E7EB;"
                "border-radius: 13px;"
                "padding: 14px 16px;"
                "color: #111827;"
                "font-size: 16px;"
            "}"
            "QLineEdit::placeholder {"
                "color: #9CA3AF;"
            "}"
            "QLineEdit:focus {"
                "border: 1px solid #A4373A;"
                "outline: none;"
                "box-shadow: 0 0 0 3px rgba(164, 55, 58, 0.30);"
            "}"
        );
        
        dialogLayout->addWidget(projectNameInput);
        dialogLayout->addSpacing(22);
        
        // Espaciador flexible
        dialogLayout->addStretch();
        
        // Botonera alineada a la derecha
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(12);
        buttonLayout->addStretch();
        
        // Botón Cancelar (estilo refinado)
        cancelButton = new QPushButton("Cancelar");
        cancelButton->setFixedSize(100, 44);
        cancelButton->setFont(QFont("SF Pro Text", 15, QFont::Medium));
        cancelButton->setStyleSheet(
            "QPushButton {"
                "background-color: #FAFAFA;"
                "border: 1px solid #E5E7EB;"
                "border-radius: 12px;"
                "color: #374151;"
                "font-weight: 500;"
            "}"
            "QPushButton:hover {"
                "background-color: #F3F4F6;"
                "transform: translateY(-1px);"
            "}"
            "QPushButton:focus {"
                "outline: 2px solid #A4373A;"
                "outline-offset: 2px;"
            "}"
            "QPushButton:pressed {"
                "background-color: #E5E7EB;"
                "transform: translateY(0px);"
            "}"
        );
        
        // Botón Crear proyecto (primario con micro-interacciones)
        createButton = new QPushButton("Crear proyecto");
        createButton->setFixedSize(140, 44);
        createButton->setFont(QFont("SF Pro Text", 15, QFont::Medium));
        createButton->setStyleSheet(
            "QPushButton {"
                "background-color: #A4373A;"
                "border: none;"
                "border-radius: 12px;"
                "color: #FFFFFF;"
                "font-weight: 500;"
            "}"
            "QPushButton:hover {"
                "background-color: #8F2F33;"
                "transform: translateY(-2px);"
            "}"
            "QPushButton:focus {"
                "outline: 2px solid #A4373A;"
                "outline-offset: 2px;"
            "}"
            "QPushButton:pressed {"
                "background-color: #7A282C;"
                "transform: translateY(0px);"
            "}"
        );
        
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(createButton);
        
        dialogLayout->addLayout(buttonLayout);
        
        // Conectar señales
        connect(cancelButton, &QPushButton::clicked, this, &CreateProject::onCreateProjectCancelled);
        connect(createButton, &QPushButton::clicked, this, &CreateProject::onCreateProjectConfirmed);
        connect(projectNameInput, &QLineEdit::returnPressed, this, &CreateProject::onCreateProjectConfirmed);
        
        qDebug() << "Elementos del modal creados correctamente";
    }
    
    qDebug() << "Verificando backgroundBlur...";
    
    // Verificar que contentWidget existe antes de aplicar efectos
    if (!contentWidget) {
        qDebug() << "ERROR CRÍTICO: contentWidget es nullptr!";
        isAnimating = false;
        return;
    }
    qDebug() << "contentWidget existe correctamente";
    
    // Aplicar blur sutil al fondo
    // IMPORTANTE: Qt toma ownership de QGraphicsEffect cuando se asigna
    // No necesitamos eliminar manualmente el efecto anterior
    
    // Quitar cualquier efecto existente antes de aplicar uno nuevo
    contentWidget->setGraphicsEffect(nullptr);
    qDebug() << "Efectos previos limpiados";
    
    // Crear nuevo efecto de blur
    backgroundBlur = new QGraphicsBlurEffect();
    if (!backgroundBlur) {
        qDebug() << "ERROR: No se pudo crear backgroundBlur";
        isAnimating = false;
        return;
    }
    backgroundBlur->setBlurRadius(6); // Blur más sutil
    qDebug() << "backgroundBlur creado correctamente";
    
    qDebug() << "Aplicando backgroundBlur a contentWidget...";
    contentWidget->setGraphicsEffect(backgroundBlur);
    qDebug() << "backgroundBlur aplicado exitosamente";
    
    qDebug() << "Mostrando modal con animación...";
    
    // Verificar que modalOverlay y modalDialog existen
    if (!modalOverlay) {
        qDebug() << "ERROR CRÍTICO: modalOverlay es nullptr antes de mostrar!";
        isAnimating = false;
        return;
    }
    if (!modalDialog) {
        qDebug() << "ERROR CRÍTICO: modalDialog es nullptr antes de mostrar!";
        isAnimating = false;
        return;
    }
    
    // IMPORTANTE: Siempre recentrar el modal antes de mostrarlo
    // Esto asegura que siempre aparezca en el centro, sin importar cuántas veces se abra
    modalOverlay->setGeometry(0, 0, width(), height());
    int centerX = (modalOverlay->width() - modalDialog->width()) / 2;
    int centerY = (modalOverlay->height() - modalDialog->height()) / 2;
    modalDialog->move(centerX, centerY);
    qDebug() << "Modal recentrado en posición:" << centerX << "," << centerY;
    qDebug() << "Ventana size:" << width() << "x" << height();
    qDebug() << "Modal size:" << modalDialog->width() << "x" << modalDialog->height();
    
    // Mostrar modal con animación
    modalOverlay->show();
    modalOverlay->raise();
    qDebug() << "modalOverlay mostrado";
    
    // Animación de entrada con duraciones optimizadas
    QPropertyAnimation *fadeIn = new QPropertyAnimation(modalOverlay, "windowOpacity");
    if (!fadeIn) {
        qDebug() << "ERROR: No se pudo crear fadeIn animation";
        isAnimating = false;
        return;
    }
    fadeIn->setDuration(180);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    qDebug() << "fadeIn animation creada";
    
    QPropertyAnimation *scaleIn = new QPropertyAnimation(modalDialog, "geometry");
    if (!scaleIn) {
        qDebug() << "ERROR: No se pudo crear scaleIn animation";
        delete fadeIn;
        isAnimating = false;
        return;
    }
    scaleIn->setDuration(180);
    QRect finalGeometry = modalDialog->geometry();
    QRect startGeometry = QRect(finalGeometry.center(), QSize(0, 0));
    scaleIn->setStartValue(startGeometry);
    scaleIn->setEndValue(finalGeometry);
    scaleIn->setEasingCurve(QEasingCurve::OutBack);
    qDebug() << "scaleIn animation creada";
    
    QParallelAnimationGroup *entryAnimation = new QParallelAnimationGroup();
    if (!entryAnimation) {
        qDebug() << "ERROR: No se pudo crear entryAnimation";
        delete fadeIn;
        delete scaleIn;
        isAnimating = false;
        return;
    }
    entryAnimation->addAnimation(fadeIn);
    entryAnimation->addAnimation(scaleIn);
    entryAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    qDebug() << "entryAnimation iniciada";
    
    // Marcar modal como visible cuando la animación termina
    connect(entryAnimation, &QParallelAnimationGroup::finished, this, [this]() {
        isModalVisible = true;
        isAnimating = false;
        qDebug() << "Modal mostrado completamente";
    });
    
    // Enfocar el campo de entrada
    projectNameInput->setFocus();
    projectNameInput->selectAll();
    qDebug() << "Input enfocado";
    
    // Aplicar estilos según el tema
    styleModalComponents();
    qDebug() << "Estilos aplicados";
    
    qDebug() << "=== FIN showNewProjectModal() (EXITOSO) ===";
    
    } catch (const std::exception& e) {
        qDebug() << "EXCEPCIÓN en showNewProjectModal():" << e.what();
        isAnimating = false;
        qDebug() << "=== FIN showNewProjectModal() (ERROR) ===";
    } catch (...) {
        qDebug() << "EXCEPCIÓN DESCONOCIDA en showNewProjectModal()";
        isAnimating = false;
        qDebug() << "=== FIN showNewProjectModal() (ERROR DESCONOCIDO) ===";
    }
}

void CreateProject::hideNewProjectModal()
{
    if (!modalOverlay || !isModalVisible) {
        qDebug() << "Modal no está visible, saltando ocultación";
        return;
    }
    
    qDebug() << "Ocultando modal de crear proyecto";
    isAnimating = true;
    
    // Quitar blur del fondo de manera segura
    if (contentWidget) {
        qDebug() << "Removiendo efectos gráficos de contentWidget";
        contentWidget->setGraphicsEffect(nullptr);
        qDebug() << "Efectos removidos correctamente";
        // IMPORTANTE: No eliminar backgroundBlur manualmente
        // Qt toma ownership cuando se asigna con setGraphicsEffect()
        backgroundBlur = nullptr; // Solo resetear el puntero
        qDebug() << "backgroundBlur pointer reseteado";
    }
    
    // Animación de salida con duraciones optimizadas
    QPropertyAnimation *fadeOut = new QPropertyAnimation(modalOverlay, "windowOpacity");
    fadeOut->setDuration(160);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    
    QPropertyAnimation *scaleOut = new QPropertyAnimation(modalDialog, "geometry");
    scaleOut->setDuration(160);
    QRect currentGeometry = modalDialog->geometry();
    QRect endGeometry = QRect(currentGeometry.center(), QSize(0, 0));
    scaleOut->setStartValue(currentGeometry);
    scaleOut->setEndValue(endGeometry);
    scaleOut->setEasingCurve(QEasingCurve::InBack);
    
    QParallelAnimationGroup *exitAnimation = new QParallelAnimationGroup();
    exitAnimation->addAnimation(fadeOut);
    exitAnimation->addAnimation(scaleOut);
    
    connect(exitAnimation, &QAbstractAnimation::finished, [this]() {
        modalOverlay->hide();
        isModalVisible = false;
        isAnimating = false;
        qDebug() << "Modal ocultado completamente";
    });
    
    exitAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CreateProject::onCreateProjectConfirmed()
{
    QString projectName = projectNameInput->text().trimmed();
    
    if (projectName.isEmpty()) {
        // Highlight del campo si está vacío
        projectNameInput->setStyleSheet(projectNameInput->styleSheet() + " border: 2px solid #e74c3c;");
        
        QTimer::singleShot(2000, [this]() {
            styleModalComponents(); // Restaurar estilo normal
        });
        
        return;
    }
    
    // Crear el proyecto
    qDebug() << "Creando proyecto:" << projectName;
    
    // Cerrar el modal primero
    hideNewProjectModal();
    
    // Navegar a la vista del proyecto (MainWindow)
    navigateToProjectView(projectName);
    
    // Limpiar el campo para la próxima vez
    projectNameInput->clear();
}

void CreateProject::navigateToProjectView(const QString &projectName)
{
    qDebug() << "Navegando a la vista del proyecto:" << projectName;
    
    // Crear y mostrar MainWindow con el proyecto
    MainWindow *mainWindow = new MainWindow();
    mainWindow->setProjectName(projectName); // Pasaremos el nombre del proyecto
    mainWindow->show();
    
    // Cerrar la ventana actual de CreateProject
    this->close();
}

void CreateProject::onCreateProjectCancelled()
{
    hideNewProjectModal();
    
    // Limpiar el campo
    projectNameInput->clear();
}

void CreateProject::styleModalComponents()
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
    
    if (!modalDialog) return;
    
    // Overlay
    modalOverlay->setStyleSheet(QString(
        "background-color: rgba(0, 0, 0, %1);"
    ).arg(isDark ? "0.7" : "0.5"));
    
    // Diálogo principal
    modalDialog->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 1px solid %2;"
        "}"
    ).arg(backgroundColor, borderColor));
    
    // Título
    QLabel *titleLabel = modalDialog->findChild<QLabel*>();
    if (titleLabel && titleLabel->text().contains("Crear")) {
        titleLabel->setStyleSheet(QString(
            "QLabel {"
            "    color: %1;"
            "    font-weight: bold;"
            "}"
        ).arg(textPrimary));
    }
    
    // Descripción
    QList<QLabel*> labels = modalDialog->findChildren<QLabel*>();
    for (QLabel *label : labels) {
        if (label->text().contains("Ingresa") || label->text().contains("Nombre del")) {
            label->setStyleSheet(QString(
                "QLabel {"
                "    color: %1;"
                "}"
            ).arg(textSecondary));
        }
    }
    
    // Campo de entrada
    if (projectNameInput) {
        projectNameInput->setStyleSheet(QString(
            "QLineEdit {"
            "    background-color: %1;"
            "    border: 2px solid %2;"
            "    border-radius: 8px;"
            "    padding: 12px 16px;"
            "    color: %3;"
            "    font-size: 14px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: %4;"
            "}"
            "QLineEdit::placeholder {"
            "    color: %5;"
            "}"
        ).arg(surfaceColor, borderColor, textPrimary, accentColor, textSecondary));
    }
    
    // Botón cancelar
    if (cancelButton) {
        cancelButton->setStyleSheet(QString(
            "QPushButton {"
            "    background-color: %1;"
            "    color: %2;"
            "    border: 2px solid %3;"
            "    border-radius: 8px;"
            "    font-weight: 500;"
            "    padding: 8px 16px;"
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
              accentColor, pressedColor));
    }
    
    // Botón crear
    if (createButton) {
        createButton->setStyleSheet(QString(
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
    }
}
