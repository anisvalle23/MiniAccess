#include "homescreen.h"
#include "ThemeManager.h"
#include "ThemeTokens.h"
#include <QDebug>

// Implementación de AnimatedBackground
AnimatedBackground::AnimatedBackground(QWidget *parent)
    : QWidget(parent), animationTime(0), blob1X(0), blob1Y(0), blob2X(0), blob2Y(0)
{
    setMouseTracking(true);
    resize(1200, 750);
    
    // Inicializar partículas flotantes
    for (int i = 0; i < 8; ++i) {
        particles.append(QPointF(
            qrand() % 1200,
            qrand() % 750
        ));
        particleOpacity.append((qrand() % 20 + 10) / 100.0f); // 0.1 - 0.3
    }
    
    // Timer para animaciones suaves a 60 FPS
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &AnimatedBackground::updateAnimation);
    animationTimer->start(16);
}

void AnimatedBackground::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    bool isDark = ThemeManager::instance().isDark();
    
    // Fondo base según el tema
    QColor baseColor = isDark ? QColor(15, 15, 17) : QColor(255, 255, 255);
    painter.fillRect(rect(), baseColor);
    
    // Degradado radial sutil según el tema
    QRadialGradient gradient(width() / 2, height() / 2.5, qMax(width(), height()) / 3);
    
    if (isDark) {
        // Tema oscuro: degradado muy sutil
        gradient.setColorAt(0, QColor(164, 55, 58, 8));      // Centro muy sutil
        gradient.setColorAt(0.5, QColor(164, 55, 58, 3));    // Transición
        gradient.setColorAt(1, QColor(164, 55, 58, 0));      // Transparente
    } else {
        // Tema claro: degradado original pero más sutil
        gradient.setColorAt(0, QColor(164, 55, 58, 15));     // Centro sutil
        gradient.setColorAt(0.5, QColor(164, 55, 58, 8));    // Transición
        gradient.setColorAt(1, QColor(164, 55, 58, 0));      // Transparente
    }
    
    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(width()/2 - 400, height()/2 - 300, 800, 600);
    
    // Grid parallax ultra suave
    int gridOpacity = isDark ? 3 : 6;
    painter.setPen(QPen(QColor(164, 55, 58, gridOpacity), 0.5));
    int gridSize = 50;
    int offsetX = (int)(mousePos.x() * 0.001) % gridSize;
    int offsetY = (int)(mousePos.y() * 0.001) % gridSize;
    
    for (int x = -offsetX; x < width() + gridSize; x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = -offsetY; y < height() + gridSize; y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }
    
    // Blobs borrosos flotantes orgánicos adaptados al tema
    painter.setPen(Qt::NoPen);
    
    int blob1Opacity = isDark ? 12 : 20;
    int blob2Opacity = isDark ? 10 : 16;
    
    // Blob 1 - Izquierda superior
    QRadialGradient blob1Gradient(blob1X, blob1Y, 180);
    blob1Gradient.setColorAt(0, QColor(164, 55, 58, blob1Opacity));
    blob1Gradient.setColorAt(0.6, QColor(164, 55, 58, blob1Opacity / 2));
    blob1Gradient.setColorAt(1, QColor(164, 55, 58, 0));
    painter.setBrush(QBrush(blob1Gradient));
    painter.drawEllipse(QPointF(blob1X, blob1Y), 180, 120);
    
    // Blob 2 - Derecha inferior
    QRadialGradient blob2Gradient(blob2X, blob2Y, 150);
    blob2Gradient.setColorAt(0, QColor(164, 55, 58, blob2Opacity));
    blob2Gradient.setColorAt(0.6, QColor(164, 55, 58, blob2Opacity / 2));
    blob2Gradient.setColorAt(1, QColor(164, 55, 58, 0));
    painter.setBrush(QBrush(blob2Gradient));
    painter.drawEllipse(QPointF(blob2X, blob2Y), 150, 100);
    
    // Partículas mínimas ascendentes
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < particles.size(); ++i) {
        int particleOpacityValue = isDark ? (int)(particleOpacity[i] * 120) : (int)(particleOpacity[i] * 180);
        painter.setBrush(QColor(164, 55, 58, particleOpacityValue));
        painter.drawEllipse(particles[i], 2, 2);
    }
}

void AnimatedBackground::mouseMoveEvent(QMouseEvent *event)
{
    mousePos = event->pos();
    update(); // Actualizar parallax
}

void AnimatedBackground::updateAnimation()
{
    animationTime += 0.01f; // Velocidad lenta y orgánica
    
    // Movimiento orgánico de blobs (30-40s ciclo completo)
    blob1X = width() * 0.15f + qSin(animationTime * 0.03f) * 100;
    blob1Y = height() * 0.25f + qCos(animationTime * 0.025f) * 80;
    
    blob2X = width() * 0.85f + qSin(animationTime * 0.02f) * 120;
    blob2Y = height() * 0.75f + qCos(animationTime * 0.035f) * 90;
    
    // Movimiento ascendente muy lento de partículas
    for (int i = 0; i < particles.size(); ++i) {
        particles[i].setY(particles[i].y() - 0.3f);
        if (particles[i].y() < -10) {
            particles[i].setY(height() + 10);
            particles[i].setX(qrand() % width());
            particleOpacity[i] = (qrand() % 20 + 10) / 100.0f;
        }
    }
    
    update();
}

// Implementación de HomeScreen
HomeScreen::HomeScreen(QWidget *parent)
    : QMainWindow(parent), animationsStarted(false), createProjectWindow(nullptr)
{
        // Crear popover de temas
    themePopover = new ThemePopover(this);
    themePopover->setSettingsButton(settingsButton);
    
    setupUI();
    centerWindow();
    
    // Conectar al sistema de temas para actualizar estilos
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, 
            this, [this](ThemeManager::Theme) {
        styleComponents();
        updateLayout();
        
        // Procesar eventos pendientes y actualizar
        QApplication::processEvents();
        update();
    });
    
    // Iniciar animaciones de entrada con delay
    QTimer::singleShot(500, this, &HomeScreen::startEntranceAnimations);
}

HomeScreen::~HomeScreen()
{
    // Qt maneja la limpieza automáticamente
}

void HomeScreen::setupUI()
{
    // Configurar ventana principal - mantener tamaño actual
    setWindowTitle("Mini Access");
    setFixedSize(1200, 750);
    
    // Widget central principal
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Layout principal vertical
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Crear fondo animado como capa base
    backgroundWidget = new AnimatedBackground(centralWidget);
    backgroundWidget->setGeometry(0, 0, 1200, 750);
    backgroundWidget->lower();
    
    // Crear header ultrafino
    createHeader();
    
    // Crear hero section
    createHeroSection();
    setupAnimations();
    
    // Añadir elementos al layout principal
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(heroContainer);
    
    // Aplicar estilos
    styleComponents();
    
    // Conectar señales
    connect(ctaButton, &QPushButton::clicked, this, &HomeScreen::onStartProjectClicked);
}

void HomeScreen::createHeroSection()
{
    // Contenedor principal del hero
    heroContainer = new QWidget();
    heroContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    heroContainer->setStyleSheet("background: transparent;");
    
    // Layout vertical centrado
    heroLayout = new QVBoxLayout(heroContainer);
    heroLayout->setAlignment(Qt::AlignCenter);
    heroLayout->setSpacing(0);
    heroLayout->setContentsMargins(50, 60, 50, 60);
    
    // Badge superior discreto "Mini Access" - REMOVIDO
    badgeLabel = new QLabel("");
    badgeLabel->setAlignment(Qt::AlignCenter);
    badgeLabel->setFont(QFont("Inter", 12, QFont::Medium));
    // Los estilos se aplicarán en styleComponents() según el tema
    badgeLabel->setFixedSize(0, 0);
    badgeLabel->setVisible(false);
    
    // Card/Glass container para el título (máx 800px)
    titleCard = new QWidget();
    titleCard->setMaximumWidth(800);
    // Los estilos se aplicarán en styleComponents() según el tema
    
    // Sombra muy ligera para la card (se configurará en styleComponents)
    cardShadow = new QGraphicsDropShadowEffect();
    titleCard->setGraphicsEffect(cardShadow);
    
    // Layout interno de la card
    QVBoxLayout *titleCardLayout = new QVBoxLayout(titleCard);
    titleCardLayout->setAlignment(Qt::AlignCenter);
    titleCardLayout->setSpacing(8);
    titleCardLayout->setContentsMargins(50, 40, 50, 40);
    
    // Título línea 1 - Extra bold muy grande
    titleLine1 = new QLabel("Mini Access");
    titleLine1->setAlignment(Qt::AlignCenter);
    titleLine1->setFont(QFont("Inter", 48, QFont::ExtraBold));
    titleLine1->setStyleSheet(
        "color: #161616; "
        "line-height: 1.1;"
    );
    titleLine1->setWordWrap(true);
    
    // Título línea 2 - REMOVIDO
    titleLine2 = new QLabel("");
    titleLine2->setAlignment(Qt::AlignCenter);
    titleLine2->setFont(QFont("Inter", 48, QFont::ExtraBold));
    titleLine2->setStyleSheet(
        "color: #161616; "
        "line-height: 1.1;"
    );
    titleLine2->setWordWrap(true);
    titleLine2->setVisible(false);
    
    // Agregar títulos a la card
    titleCardLayout->addWidget(titleLine1);
    titleCardLayout->addWidget(titleLine2);
    
    // Subcopy descriptivo
    descriptionLabel = new QLabel("La plataforma de base de datos. Crea tablas, relaciones y APIs.");
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setFont(QFont("Inter", 16, QFont::Normal));
    descriptionLabel->setStyleSheet(
        "color: #6B7280; "
        "line-height: 1.5;"
    );
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setMaximumWidth(650);
    
    // Botón CTA grande y centrado
    ctaButton = new QPushButton("Iniciar proyecto");
    ctaButton->setFixedSize(200, 56);
    ctaButton->setFont(QFont("Inter", 15, QFont::Medium));
    ctaButton->setCursor(Qt::PointingHandCursor);
    
    // Layout con espaciado amplio y elegante
    heroLayout->addStretch(1);
    heroLayout->addWidget(badgeLabel, 0, Qt::AlignCenter);
    heroLayout->addSpacing(50);
    heroLayout->addWidget(titleCard, 0, Qt::AlignCenter);
    heroLayout->addSpacing(40);
    heroLayout->addWidget(descriptionLabel, 0, Qt::AlignCenter);
    heroLayout->addSpacing(50);
    heroLayout->addWidget(ctaButton, 0, Qt::AlignCenter);
    heroLayout->addStretch(1);
}

void HomeScreen::setupAnimations()
{
    // Crear efectos de opacidad para animaciones de entrada
    badgeOpacity = new QGraphicsOpacityEffect(this);
    badgeOpacity->setOpacity(0);
    badgeLabel->setGraphicsEffect(badgeOpacity);
    
    titleLine1Opacity = new QGraphicsOpacityEffect(this);
    titleLine1Opacity->setOpacity(0);
    titleLine1->setGraphicsEffect(titleLine1Opacity);
    
    titleLine2Opacity = new QGraphicsOpacityEffect(this);
    titleLine2Opacity->setOpacity(0);
    titleLine2->setGraphicsEffect(titleLine2Opacity);
    
    descriptionOpacity = new QGraphicsOpacityEffect(this);
    descriptionOpacity->setOpacity(0);
    descriptionLabel->setGraphicsEffect(descriptionOpacity);
    
    ctaOpacity = new QGraphicsOpacityEffect(this);
    ctaOpacity->setOpacity(0);
    ctaButton->setGraphicsEffect(ctaOpacity);
}

void HomeScreen::startEntranceAnimations()
{
    // Verificar que las animaciones no se hayan iniciado ya
    if (animationsStarted) {
        return;
    }
    animationsStarted = true;
    
    // Grupo secuencial de animaciones elegantes
    entranceAnimationGroup = new QSequentialAnimationGroup(this);
    
    // 1. Badge fade-in desde arriba
    badgeAnimation = new QPropertyAnimation(badgeOpacity, "opacity");
    badgeAnimation->setDuration(800);
    badgeAnimation->setStartValue(0.0);
    badgeAnimation->setEndValue(1.0);
    badgeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 2. Títulos slide-up + fade en paralelo
    QParallelAnimationGroup *titleGroup = new QParallelAnimationGroup();
    
    titleLine1Animation = new QPropertyAnimation(titleLine1Opacity, "opacity");
    titleLine1Animation->setDuration(1000);
    titleLine1Animation->setStartValue(0.0);
    titleLine1Animation->setEndValue(1.0);
    titleLine1Animation->setEasingCurve(QEasingCurve::OutCubic);
    
    titleLine2Animation = new QPropertyAnimation(titleLine2Opacity, "opacity");
    titleLine2Animation->setDuration(1000);
    titleLine2Animation->setStartValue(0.0);
    titleLine2Animation->setEndValue(1.0);
    titleLine2Animation->setEasingCurve(QEasingCurve::OutCubic);
    
    titleGroup->addAnimation(titleLine1Animation);
    titleGroup->addAnimation(titleLine2Animation);
    
    // 3. Descripción fade suave
    descriptionAnimation = new QPropertyAnimation(descriptionOpacity, "opacity");
    descriptionAnimation->setDuration(700);
    descriptionAnimation->setStartValue(0.0);
    descriptionAnimation->setEndValue(1.0);
    descriptionAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 4. CTA con pequeño pop elegante
    ctaAnimation = new QPropertyAnimation(ctaOpacity, "opacity");
    ctaAnimation->setDuration(600);
    ctaAnimation->setStartValue(0.0);
    ctaAnimation->setEndValue(1.0);
    ctaAnimation->setEasingCurve(QEasingCurve::OutBack);
    
    // Secuencia con delays naturales
    entranceAnimationGroup->addAnimation(badgeAnimation);
    entranceAnimationGroup->addPause(300);
    entranceAnimationGroup->addAnimation(titleGroup);
    entranceAnimationGroup->addPause(400);
    entranceAnimationGroup->addAnimation(descriptionAnimation);
    entranceAnimationGroup->addPause(300);
    entranceAnimationGroup->addAnimation(ctaAnimation);
    
    // Iniciar la secuencia
    entranceAnimationGroup->start();
}

void HomeScreen::styleComponents()
{
    bool isDark = ThemeManager::instance().isDark();
    QString accentColor = isDark ? ThemeTokens::Dark::ACCENT_ADJUSTED : ThemeTokens::ACCENT_PRIMARY;
    QString hoverColor = ThemeTokens::ThemeHelper::getButtonHoverColor(isDark);
    QString pressedColor = ThemeTokens::ThemeHelper::getButtonPressedColor(isDark);
    QString shadowColor = ThemeTokens::ThemeHelper::getShadowColor(isDark);
    QString surfaceColor = isDark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString textPrimary = isDark ? ThemeTokens::Dark::TEXT_PRIMARY : ThemeTokens::Light::TEXT_PRIMARY;
    QString textSecondary = isDark ? ThemeTokens::Dark::TEXT_SECONDARY : ThemeTokens::Light::TEXT_SECONDARY;
    
    // Reestablecer alineación del layout principal
    if (heroLayout) {
        heroLayout->setAlignment(Qt::AlignCenter);
    }
    
    // Asegurar que todos los elementos mantengan su alineación central
    if (badgeLabel) {
        badgeLabel->setAlignment(Qt::AlignCenter);
    }
    if (titleLine1) {
        titleLine1->setAlignment(Qt::AlignCenter);
    }
    if (titleLine2) {
        titleLine2->setAlignment(Qt::AlignCenter);
    }
    if (descriptionLabel) {
        descriptionLabel->setAlignment(Qt::AlignCenter);
    }
    
    // Estilo del badge según el tema
    badgeLabel->setStyleSheet(QString(
        "QLabel {"
        "    background-color: rgba(164, 55, 58, %1);"
        "    color: %2;"
        "    padding: 6px 14px;"
        "    border-radius: 18px;"
        "    border: 1px solid rgba(164, 55, 58, %3);"
        "    text-align: center;"
        "}"
    ).arg(isDark ? "0.12" : "0.08", accentColor, isDark ? "0.25" : "0.15"));
    
    // Estilo de la title card según el tema
    titleCard->setStyleSheet(QString(
        "QWidget {"
        "    background-color: %1;"
        "    border-radius: 22px;"
        "    border: 1px solid rgba(164, 55, 58, %2);"
        "}"
    ).arg(isDark ? "rgba(22, 22, 25, 0.8)" : "rgba(255, 247, 247, 0.7)", 
          isDark ? "0.15" : "0.08"));
    
    // Configurar sombra de la card según el tema
    if (cardShadow) {
        cardShadow->setBlurRadius(25);
        cardShadow->setColor(QColor(isDark ? 0 : 164, isDark ? 0 : 55, isDark ? 0 : 58, isDark ? 40 : 20));
        cardShadow->setOffset(0, isDark ? 8 : 6);
    }
    
    // Estilo de los títulos según el tema con alineación explícita
    titleLine1->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    line-height: 1.1;"
        "}"
    ).arg(textPrimary));
    // Forzar alineación central programáticamente
    titleLine1->setAlignment(Qt::AlignCenter);
    
    titleLine2->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    line-height: 1.1;"
        "}"
    ).arg(textPrimary));
    // Forzar alineación central programáticamente
    titleLine2->setAlignment(Qt::AlignCenter);
    
    // Estilo de la descripción según el tema con alineación explícita
    descriptionLabel->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    line-height: 1.5;"
        "}"
    ).arg(textSecondary));
    // Forzar alineación central programáticamente
    descriptionLabel->setAlignment(Qt::AlignCenter);
    
    // Estilo del botón CTA con microinteracciones
    ctaButton->setStyleSheet(QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-weight: 500;"
        "    padding: 16px 32px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
    ).arg(accentColor, hoverColor, pressedColor));
    
    // Sombra suave para el botón
    if (buttonShadow) {
        delete buttonShadow;
    }
    buttonShadow = new QGraphicsDropShadowEffect();
    buttonShadow->setBlurRadius(12);
    buttonShadow->setColor(QColor(isDark ? 0 : 164, isDark ? 0 : 55, isDark ? 0 : 58, isDark ? 60 : 35));
    buttonShadow->setOffset(0, 3);
    ctaButton->setGraphicsEffect(buttonShadow);
    
    // Actualizar el background animado
    if (backgroundWidget) {
        backgroundWidget->update();
    }
    
    // Forzar re-centrado de todos los elementos
    updateLayout();
}

void HomeScreen::updateLayout()
{
    // Reestablecer todas las alineaciones
    if (heroLayout) {
        heroLayout->setAlignment(Qt::AlignCenter);
        
        // Actualizar cada widget en el layout
        for (int i = 0; i < heroLayout->count(); ++i) {
            QLayoutItem *item = heroLayout->itemAt(i);
            if (item && item->widget()) {
                heroLayout->setAlignment(item->widget(), Qt::AlignCenter);
            }
        }
    }
    
    // Procesar eventos para asegurar que se apliquen los cambios
    QApplication::processEvents();
}

void HomeScreen::centerWindow()
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

void HomeScreen::onStartProjectClicked()
{
    qDebug() << "DEBUG: onStartProjectClicked() iniciado";
    
    // Efecto ripple sutil al hacer click
    QPropertyAnimation *liftAnimation = new QPropertyAnimation(ctaButton, "geometry");
    liftAnimation->setDuration(100);
    QRect originalGeometry = ctaButton->geometry();
    QRect liftedGeometry = originalGeometry.adjusted(0, -2, 0, -2);
    
    liftAnimation->setStartValue(originalGeometry);
    liftAnimation->setEndValue(liftedGeometry);
    liftAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // Volver a posición original
    QPropertyAnimation *returnAnimation = new QPropertyAnimation(ctaButton, "geometry");
    returnAnimation->setDuration(100);
    returnAnimation->setStartValue(liftedGeometry);
    returnAnimation->setEndValue(originalGeometry);
    returnAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    QSequentialAnimationGroup *clickGroup = new QSequentialAnimationGroup();
    clickGroup->addAnimation(liftAnimation);
    clickGroup->addAnimation(returnAnimation);
    clickGroup->start(QAbstractAnimation::DeleteWhenStopped);
    
    qDebug() << "DEBUG: Animación de botón configurada";
    
    try {
        qDebug() << "DEBUG: Intentando crear CreateProject window";
        // Crear y mostrar la ventana CreateProject
        createProjectWindow = new CreateProject(nullptr); // Cambiar parent a nullptr temporalmente
        qDebug() << "DEBUG: CreateProject window creada exitosamente";
        
        createProjectWindow->show();
        qDebug() << "DEBUG: CreateProject window mostrada";
        
        // Cerrar la ventana actual con un pequeño delay
        QTimer::singleShot(200, this, [this]() {
            qDebug() << "DEBUG: Cerrando HomeScreen";
            this->close();
        });
        
    } catch (const std::exception& e) {
        qDebug() << "DEBUG: Excepción capturada:" << e.what();
    } catch (...) {
        qDebug() << "DEBUG: Excepción desconocida capturada";
    }
    
    qDebug() << "DEBUG: onStartProjectClicked() terminado";
}

void HomeScreen::createHeader()
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
    headerLine = new QFrame(centralWidget);
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
    
    // Posicionar elementos
    headerWidget->setGeometry(0, 0, 1200, 50);
    headerLine->setGeometry(0, 49, 1200, 1);
    
    // Conectar señal
    connect(settingsButton, &QPushButton::clicked, this, &HomeScreen::onSettingsClicked);
}

void HomeScreen::onSettingsClicked()
{
    // Verificar si el popover está visible para hacer toggle
    if (themePopover->isPopoverVisible()) {
        themePopover->hidePopover();
        return;
    }
    
    // Crear efecto de rotación en el ícono
    QGraphicsRotation *rotation = new QGraphicsRotation();
    rotation->setOrigin(QVector3D(16, 16, 0)); // Centro del botón
    
    QPropertyAnimation *rotateAnimation = new QPropertyAnimation(rotation, "angle");
    rotateAnimation->setDuration(200);
    rotateAnimation->setStartValue(0);
    rotateAnimation->setEndValue(15);
    rotateAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // Crear efecto de lift sutil
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
    
    // Secuencia de animaciones
    QParallelAnimationGroup *clickGroup = new QParallelAnimationGroup();
    clickGroup->addAnimation(rotateAnimation);
    clickGroup->addAnimation(liftAnimation);
    
    QSequentialAnimationGroup *fullSequence = new QSequentialAnimationGroup();
    fullSequence->addAnimation(clickGroup);
    fullSequence->addAnimation(returnAnimation);
    fullSequence->start(QAbstractAnimation::DeleteWhenStopped);
    
    // Mostrar popover de tema
    QPoint popoverPosition = settingsButton->mapToGlobal(QPoint(0, settingsButton->height()));
    themePopover->showPopover(popoverPosition);
}
