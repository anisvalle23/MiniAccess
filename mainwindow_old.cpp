#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), sidebarExpanded(false)
{
    setupUI();
}

MainWindow::~MainWindow()
{
    // Qt handles cleanup automatically
}

void MainWindow::setupUI()
{
    // Configure main window
    setWindowTitle("MiniAccess");
    setFixedSize(1200, 750);
    
    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - 1200) / 2;
    int y = (screenGeometry.height() - 750) / 2;
    move(x, y);
    
    // Main central widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout (vertical: header + body)
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create components
    createHeader();
    createMainContent();
    createSidebar(); // Sidebar as overlay
    
    // Add to main layout
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(mainContainer);
    
    // Setup animations
    setupSidebarAnimation();
    
    // Apply styles
    styleComponents();
}

void MainWindow::createHeader()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(64);
    headerWidget->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-top: 2px solid rgba(164, 55, 58, 0.18);"
            "border-bottom: 1px solid #E5E7EB;"
        "}"
    );
    
    headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(32, 0, 32, 0);
    headerLayout->setSpacing(20);
    
    // MiniAccess brand label
    miniAccessLabel = new QLabel("MiniAccess");
    miniAccessLabel->setFont(QFont("Inter", 16, QFont::Bold));
    miniAccessLabel->setStyleSheet("QLabel { color: #161616; }");
    headerLayout->addWidget(miniAccessLabel);
    
    // Vertical separator
    headerSeparator = new QFrame();
    headerSeparator->setFrameShape(QFrame::VLine);
    headerSeparator->setFrameShadow(QFrame::Sunken);
    headerSeparator->setStyleSheet("QFrame { color: #E5E7EB; border: none; background-color: #E5E7EB; }");
    headerSeparator->setFixedWidth(1);
    headerSeparator->setFixedHeight(20);
    headerLayout->addWidget(headerSeparator);
    
    // Project name with dropdown
    QWidget *projectWidget = new QWidget();
    QHBoxLayout *projectLayout = new QHBoxLayout(projectWidget);
    projectLayout->setContentsMargins(0, 0, 0, 0);
    projectLayout->setSpacing(8);
    
    projectNameLabel = new QLabel("proyecto1");
    projectNameLabel->setFont(QFont("Inter", 15, QFont::Medium));
    projectNameLabel->setStyleSheet("QLabel { color: #161616; }");
    
    projectDropdownBtn = new QPushButton("▾");
    projectDropdownBtn->setFixedSize(20, 20);
    projectDropdownBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "color: #6B7280;"
            "font-size: 12px;"
            "border-radius: 4px;"
        "}"
        "QPushButton:hover {"
            "background-color: rgba(164, 55, 58, 0.08);"
            "color: #A4373A;"
        "}"
    );
    
    projectLayout->addWidget(projectNameLabel);
    projectLayout->addWidget(projectDropdownBtn);
    headerLayout->addWidget(projectWidget);
    
    // Flexible spacer
    headerLayout->addStretch();
    
    // Settings button with rotation animation
    settingsBtn = new QPushButton("⚙");
    settingsBtn->setFixedSize(40, 40);
    settingsBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "border-radius: 8px;"
            "font-size: 20px;"
            "color: #6B7280;"
        "}"
        "QPushButton:hover {"
            "background-color: rgba(164, 55, 58, 0.08);"
            "color: #A4373A;"
        "}"
    );
    
    // Connect hover events for settings button
    connect(settingsBtn, &QPushButton::entered, this, &MainWindow::onSettingsButtonHover);
    connect(settingsBtn, &QPushButton::left, this, &MainWindow::onSettingsButtonLeave);
    
    headerLayout->addWidget(settingsBtn);
}

void MainWindow::createSidebar()
{
    // Sidebar overlay container (positioned absolutely)
    sidebarOverlay = new QWidget(centralWidget);
    sidebarOverlay->setGeometry(0, 64, 64, 750 - 64); // Start collapsed
    sidebarOverlay->setStyleSheet("background-color: transparent;");
    sidebarOverlay->raise(); // Ensure it's above main content
    
    // Actual sidebar widget
    sidebarWidget = new QWidget(sidebarOverlay);
    sidebarWidget->setFixedSize(64, 750 - 64);
    sidebarWidget->setStyleSheet(
        "QWidget {"
            "background-color: #F8F8F8;"
            "border-right: 1px solid #E5E7EB;"
        "}"
    );
    
    // Shadow effect for expanded state
    sidebarShadowEffect = new QGraphicsDropShadowEffect();
    sidebarShadowEffect->setBlurRadius(20);
    sidebarShadowEffect->setColor(QColor(0, 0, 0, 30));
    sidebarShadowEffect->setOffset(4, 0);
    sidebarShadowEffect->setEnabled(false);
    sidebarWidget->setGraphicsEffect(sidebarShadowEffect);
    
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(8, 20, 8, 16);
    sidebarLayout->setSpacing(6);
    
    // Sidebar items
    QStringList sidebarIcons = {"🏠", "📊", "🗃", "🔗", "🔐", "💾", "⚡", "📡", "📈", "📋", "📚", "🔌", "⚙"};
    QStringList sidebarLabels = {"Home", "Tablas", "SQL", "Database", "Auth", "Storage", "Edge Functions", "Realtime", "Reports", "Logs", "API Docs", "Integrations", "Settings"};
    
    for (int i = 0; i < sidebarIcons.size(); ++i) {
        QPushButton *btn = new QPushButton(sidebarIcons[i]);
        btn->setFixedSize(48, 44);
        btn->setProperty("labelText", sidebarLabels[i]);
        btn->setProperty("iconText", sidebarIcons[i]);
        
        QString buttonStyle = 
            "QPushButton {"
                "background-color: transparent;"
                "border: none;"
                "border-radius: 8px;"
                "font-size: 18px;"
                "color: #6B7280;"
                "text-align: center;"
            "}"
            "QPushButton:hover {"
                "background-color: rgba(164, 55, 58, 0.08);"
                "color: #A4373A;"
                "transform: translateY(-1px);"
            "}";
        
        // First button (Home) is active
        if (i == 0) {
            buttonStyle += 
                "QPushButton {"
                    "background-color: rgba(164, 55, 58, 0.1);"
                    "color: #A4373A;"
                "}";
        }
        
        btn->setStyleSheet(buttonStyle);
        sidebarButtons.append(btn);
        sidebarLayout->addWidget(btn);
        
        // Add dividers between groups
        if (i == 2 || i == 7 || i == 10) { // After SQL, Realtime, API Docs
            QFrame *divider = new QFrame();
            divider->setFrameShape(QFrame::HLine);
            divider->setStyleSheet("QFrame { color: #E5E7EB; background-color: #E5E7EB; border: none; }");
            divider->setFixedHeight(1);
            sidebarLayout->addWidget(divider);
        }
    }
    
    sidebarLayout->addStretch();
    
    // Install event filter for hover detection
    sidebarWidget->setMouseTracking(true);
    sidebarWidget->installEventFilter(this);
}

void MainWindow::createMainContent()
{
    // Main container (doesn't move when sidebar expands)
    mainContainer = new QWidget();
    mainContainer->setStyleSheet("QWidget { background-color: #FAFAFA; }");
    
    mainContainerLayout = new QVBoxLayout(mainContainer);
    mainContainerLayout->setContentsMargins(32, 32, 32, 32);
    mainContainerLayout->setSpacing(24);
    
    createWelcomeSection();
    createSkeletonTable();
}

void MainWindow::createWelcomeSection()
{
    // Project title
    projectTitleLabel = new QLabel("proyecto1");
    projectTitleLabel->setFont(QFont("Inter", 32, QFont::ExtraBold)); // Weight 800
    projectTitleLabel->setStyleSheet("QLabel { color: #161616; }");
    mainContainerLayout->addWidget(projectTitleLabel);
    
    // Title separator
    titleSeparator = new QFrame();
    titleSeparator->setFrameShape(QFrame::HLine);
    titleSeparator->setStyleSheet("QFrame { color: #E5E7EB; background-color: #E5E7EB; border: none; }");
    titleSeparator->setFixedHeight(1);
    mainContainerLayout->addWidget(titleSeparator);
    
    // Welcome message
    welcomeLabel = new QLabel("Bienvenido a tu nuevo proyecto");
    welcomeLabel->setFont(QFont("Inter", 20, QFont::Bold));
    welcomeLabel->setStyleSheet("QLabel { color: #161616; margin-top: 4px; }");
    mainContainerLayout->addWidget(welcomeLabel);
    
    // Description
    descriptionLabel = new QLabel("Tu proyecto has been deployed on its own instance, with its own API all set up and ready to use.");
    descriptionLabel->setFont(QFont("Inter", 16));
    descriptionLabel->setStyleSheet("QLabel { color: #6B7280; line-height: 1.5; }");
    descriptionLabel->setWordWrap(true);
    mainContainerLayout->addWidget(descriptionLabel);
}

void MainWindow::createSkeletonTable()
{
    // Skeleton table container
    skeletonTableWidget = new QWidget();
    skeletonTableWidget->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border: 1px solid #EDEDED;"
            "border-radius: 16px;"
        "}"
    );
    skeletonTableWidget->setFixedHeight(280);
    
    QVBoxLayout *tableLayout = new QVBoxLayout(skeletonTableWidget);
    tableLayout->setContentsMargins(20, 18, 20, 18);
    tableLayout->setSpacing(14);
    
    // Table header
    QWidget *headerWidget = new QWidget();
    headerWidget->setStyleSheet("QWidget { background-color: #F9FAFB; border-radius: 8px; }");
    headerWidget->setFixedHeight(40);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    
    QLabel *idHeader = new QLabel("id");
    QLabel *taskHeader = new QLabel("task");
    QLabel *statusHeader = new QLabel("status");
    
    QString headerStyle = "QLabel { color: #6B7280; font-weight: 600; font-size: 14px; font-family: 'Inter'; }";
    idHeader->setStyleSheet(headerStyle);
    taskHeader->setStyleSheet(headerStyle);
    statusHeader->setStyleSheet(headerStyle);
    
    headerLayout->addWidget(idHeader, 1);
    headerLayout->addWidget(taskHeader, 4);
    headerLayout->addWidget(statusHeader, 2);
    
    tableLayout->addWidget(headerWidget);
    
    // Skeleton rows
    QStringList tasks = {"Create a project", "Read documentation", "Build application", "Connect Supabase", "Deploy project", "Get users"};
    QStringList statuses = {"Complete", "Complete", "In progress", "In progress", "Not started", "Not started"};
    QStringList statusColors = {"#059669", "#059669", "#D97706", "#D97706", "#6B7280", "#6B7280"};
    
    for (int i = 0; i < tasks.size(); ++i) {
        QWidget *row = new QWidget();
        row->setFixedHeight(36);
        
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(16, 0, 16, 0);
        
        QLabel *idLabel = new QLabel(QString::number(i + 1));
        QLabel *taskLabel = new QLabel(tasks[i]);
        QLabel *statusLabel = new QLabel(statuses[i]);
        
        QString rowStyle = "QLabel { color: #374151; font-size: 14px; font-family: 'Inter'; }";
        idLabel->setStyleSheet(rowStyle);
        taskLabel->setStyleSheet(rowStyle);
        statusLabel->setStyleSheet("QLabel { color: " + statusColors[i] + "; font-size: 14px; font-weight: 500; font-family: 'Inter'; }");
        
        rowLayout->addWidget(idLabel, 1);
        rowLayout->addWidget(taskLabel, 4);
        rowLayout->addWidget(statusLabel, 2);
        
        tableLayout->addWidget(row);
        
        // Row separator (except last)
        if (i < tasks.size() - 1) {
            QFrame *separator = new QFrame();
            separator->setFrameShape(QFrame::HLine);
            separator->setStyleSheet("QFrame { color: #F3F4F6; background-color: #F3F4F6; border: none; }");
            separator->setFixedHeight(1);
            tableLayout->addWidget(separator);
        }
    }
    
    mainContainerLayout->addWidget(skeletonTableWidget);
    mainContainerLayout->addStretch();
}

void MainWindow::setupSidebarAnimation()
{
    // Width animation
    sidebarWidthAnimation = new QPropertyAnimation(sidebarWidget, "minimumWidth");
    sidebarWidthAnimation->setDuration(200);
    sidebarWidthAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    // Opacity effect for smooth expansion
    sidebarOpacityEffect = new QGraphicsOpacityEffect();
    sidebarOpacityEffect->setOpacity(1.0);
    
    // Group animations
    sidebarAnimationGroup = new QParallelAnimationGroup();
    sidebarAnimationGroup->addAnimation(sidebarWidthAnimation);
}

void MainWindow::updateSidebarItems(bool expanded)
{
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        QString icon = btn->property("iconText").toString();
        QString label = btn->property("labelText").toString();
        
        if (expanded) {
            btn->setText(icon + "  " + label);
            btn->setFixedSize(220, 44);
            btn->setStyleSheet(btn->styleSheet() + "text-align: left; padding-left: 16px;");
        } else {
            btn->setText(icon);
            btn->setFixedSize(48, 44);
            QString style = btn->styleSheet();
            style.remove("text-align: left; padding-left: 16px;");
            btn->setStyleSheet(style);
        }
    }
}

void MainWindow::onSidebarEnter()
{
    if (!sidebarExpanded) {
        sidebarExpanded = true;
        
        // Resize overlay and widget
        sidebarOverlay->setFixedWidth(240);
        sidebarWidget->setFixedWidth(240);
        
        // Enable shadow
        sidebarShadowEffect->setEnabled(true);
        
        // Update button layout
        updateSidebarItems(true);
        
        // Animate width
        sidebarWidthAnimation->setStartValue(64);
        sidebarWidthAnimation->setEndValue(240);
        sidebarWidthAnimation->start();
    }
}

void MainWindow::onSidebarLeave()
{
    if (sidebarExpanded) {
        sidebarExpanded = false;
        
        // Disable shadow
        sidebarShadowEffect->setEnabled(false);
        
        // Update button layout
        updateSidebarItems(false);
        
        // Animate width back
        sidebarWidthAnimation->setStartValue(240);
        sidebarWidthAnimation->setEndValue(64);
        sidebarWidthAnimation->start();
        
        // Resize overlay and widget after animation
        QTimer::singleShot(200, [this]() {
            sidebarOverlay->setFixedWidth(64);
            sidebarWidget->setFixedWidth(64);
        });
    }
}

void MainWindow::onSettingsButtonHover()
{
    // Rotation animation for settings button
    settingsBtn->setStyleSheet(settingsBtn->styleSheet() + "transform: rotate(15deg) translateY(-1px);");
}

void MainWindow::onSettingsButtonLeave()
{
    // Reset settings button
    QString style = settingsBtn->styleSheet();
    style.remove("transform: rotate(15deg) translateY(-1px);");
    settingsBtn->setStyleSheet(style);
}

void MainWindow::styleComponents()
{
    // Main window styling
    setStyleSheet(
        "QMainWindow {"
            "background-color: #FAFAFA;"
            "font-family: 'Inter', sans-serif;"
        "}"
    );
}

void MainWindow::setProjectName(const QString &projectName)
{
    currentProjectName = projectName;
    setWindowTitle("MiniAccess - " + projectName);
    projectNameLabel->setText(projectName);
    projectTitleLabel->setText(projectName);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sidebarWidget) {
        if (event->type() == QEvent::Enter) {
            onSidebarEnter();
        } else if (event->type() == QEvent::Leave) {
            onSidebarLeave();
        }
    }
    return QMainWindow::eventFilter(obj, event);
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
    
    // Layout principal vertical
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Crear componentes
    createHeader();
    
    // Contenedor horizontal para sidebar + contenido
    QWidget *bodyWidget = new QWidget();
    QHBoxLayout *bodyLayout = new QHBoxLayout(bodyWidget);
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    
    createSidebar();
    createMainContent();
    
    bodyLayout->addWidget(sidebarWidget);
    bodyLayout->addWidget(contentArea);
    
    // Añadir al layout principal
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(bodyWidget);
    
    // Aplicar estilos
    styleComponents();
    
    // Configurar animación del sidebar
    sidebarAnimation = new QPropertyAnimation(sidebarWidget, "minimumWidth");
    sidebarAnimation->setDuration(200);
    sidebarAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void MainWindow::createHeader()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(60);
    headerWidget->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-bottom: 1px solid #E5E7EB;"
        "}"
    );
    
    headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 12, 24, 12);
    headerLayout->setSpacing(16);
    
    // Logo/Nombre MiniAccess
    miniAccessLabel = new QLabel("MiniAccess");
    miniAccessLabel->setFont(QFont("SF Pro Display", 18, QFont::Bold));
    miniAccessLabel->setStyleSheet("QLabel { color: #111827; }");
    headerLayout->addWidget(miniAccessLabel);
    
    // Separador vertical
    headerSeparator = new QFrame();
    headerSeparator->setFrameShape(QFrame::VLine);
    headerSeparator->setFrameShadow(QFrame::Sunken);
    headerSeparator->setStyleSheet("QFrame { color: #E5E7EB; }");
    headerSeparator->setFixedHeight(20);
    headerLayout->addWidget(headerSeparator);
    
    // Nombre del proyecto con dropdown
    QWidget *projectWidget = new QWidget();
    QHBoxLayout *projectLayout = new QHBoxLayout(projectWidget);
    projectLayout->setContentsMargins(0, 0, 0, 0);
    projectLayout->setSpacing(8);
    
    projectNameLabel = new QLabel("proyecto1");
    projectNameLabel->setFont(QFont("SF Pro Text", 16, QFont::Medium));
    projectNameLabel->setStyleSheet("QLabel { color: #374151; }");
    
    projectDropdownBtn = new QPushButton("▼");
    projectDropdownBtn->setFixedSize(24, 24);
    projectDropdownBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "color: #6B7280;"
            "font-size: 12px;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
            "border-radius: 4px;"
        "}"
    );
    
    projectLayout->addWidget(projectNameLabel);
    projectLayout->addWidget(projectDropdownBtn);
    headerLayout->addWidget(projectWidget);
    
    // Espaciador flexible
    headerLayout->addStretch();
    
    // Botón de configuración
    settingsBtn = new QPushButton("⚙️");
    settingsBtn->setFixedSize(40, 40);
    settingsBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "border-radius: 8px;"
            "font-size: 18px;"
            "color: #6B7280;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
        "}"
        "QPushButton:pressed {"
            "background-color: #E5E7EB;"
        "}"
    );
    
    headerLayout->addWidget(settingsBtn);
}

void MainWindow::createSidebar()
{
    sidebarWidget = new QWidget();
    sidebarWidget->setFixedWidth(60); // Ancho colapsado inicial
    sidebarWidget->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-right: 1px solid #E5E7EB;"
        "}"
    );
    
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(8, 16, 8, 16);
    sidebarLayout->setSpacing(8);
    
    // Crear botones del sidebar
    homeBtn = new QPushButton("🏠");
    analyticsBtn = new QPushButton("📊");
    settingsBtn2 = new QPushButton("⚙️");
    docsBtn = new QPushButton("📝");
    toolsBtn = new QPushButton("🔧");
    
    QList<QPushButton*> sidebarButtons = {homeBtn, analyticsBtn, settingsBtn2, docsBtn, toolsBtn};
    QStringList buttonLabels = {"Home", "Analytics", "Settings", "Docs", "Tools"};
    
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        btn->setFixedSize(44, 44);
        btn->setStyleSheet(
            "QPushButton {"
                "background-color: transparent;"
                "border: none;"
                "border-radius: 8px;"
                "font-size: 20px;"
                "color: #6B7280;"
            "}"
            "QPushButton:hover {"
                "background-color: #F3F4F6;"
                "color: #374151;"
            "}"
            "QPushButton:pressed {"
                "background-color: #E5E7EB;"
            "}"
        );
        
        // Marcar el primer botón (Home) como activo
        if (i == 0) {
            btn->setStyleSheet(btn->styleSheet() + 
                "QPushButton { background-color: #A4373A; color: white; }"
                "QPushButton:hover { background-color: #8F2F33; }"
            );
        }
        
        // Guardar el texto del label para expandir
        btn->setProperty("labelText", buttonLabels[i]);
        
        sidebarLayout->addWidget(btn);
    }
    
    sidebarLayout->addStretch();
    
    // Configurar eventos de hover para el sidebar
    sidebarWidget->setMouseTracking(true);
    sidebarWidget->installEventFilter(this);
}

void MainWindow::createMainContent()
{
    contentArea = new QWidget();
    contentArea->setStyleSheet("QWidget { background-color: #FAFAFA; }");
    
    contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(32, 32, 32, 32);
    contentLayout->setSpacing(24);
    
    createProjectDashboard();
}

void MainWindow::createProjectDashboard()
{
    // Título del proyecto
    projectTitleLabel = new QLabel("proyecto1");
    projectTitleLabel->setFont(QFont("SF Pro Display", 32, QFont::Bold));
    projectTitleLabel->setStyleSheet("QLabel { color: #111827; }");
    contentLayout->addWidget(projectTitleLabel);
    
    // Línea separadora
    titleSeparator = new QFrame();
    titleSeparator->setFrameShape(QFrame::HLine);
    titleSeparator->setFrameShadow(QFrame::Sunken);
    titleSeparator->setStyleSheet("QFrame { color: #E5E7EB; }");
    contentLayout->addWidget(titleSeparator);
    
    // Descripción de bienvenida
    welcomeLabel = new QLabel("Bienvenido a tu nuevo proyecto");
    welcomeLabel->setFont(QFont("SF Pro Text", 20, QFont::Medium));
    welcomeLabel->setStyleSheet("QLabel { color: #374151; }");
    contentLayout->addWidget(welcomeLabel);
    
    descriptionLabel = new QLabel("Tu proyecto has been deployed on its own instance, with its own API all set up and ready to use.");
    descriptionLabel->setFont(QFont("SF Pro Text", 16));
    descriptionLabel->setStyleSheet("QLabel { color: #6B7280; }");
    descriptionLabel->setWordWrap(true);
    contentLayout->addWidget(descriptionLabel);
    
    // Tabla de demostración
    createDemoTable();
}

void MainWindow::createDemoTable()
{
    // Contenedor de la tabla
    QWidget *tableContainer = new QWidget();
    tableContainer->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 12px;"
        "}"
    );
    tableContainer->setMaximumHeight(300);
    
    QVBoxLayout *tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(16, 16, 16, 16);
    tableLayout->setSpacing(12);
    
    // Header de la tabla
    QWidget *tableHeader = new QWidget();
    tableHeader->setStyleSheet("QWidget { background-color: #F9FAFB; border-radius: 6px; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(tableHeader);
    headerLayout->setContentsMargins(12, 8, 12, 8);
    
    QLabel *idHeader = new QLabel("id");
    QLabel *taskHeader = new QLabel("task");
    QLabel *statusHeader = new QLabel("status");
    
    QString headerStyle = "QLabel { color: #6B7280; font-weight: 600; font-size: 14px; }";
    idHeader->setStyleSheet(headerStyle);
    taskHeader->setStyleSheet(headerStyle);
    statusHeader->setStyleSheet(headerStyle);
    
    headerLayout->addWidget(idHeader, 1);
    headerLayout->addWidget(taskHeader, 3);
    headerLayout->addWidget(statusHeader, 2);
    
    tableLayout->addWidget(tableHeader);
    
    // Filas de datos de demostración
    QStringList tasks = {"Create a project", "Read documentation", "Build application", "Connect Supabase", "Deploy project", "Get users"};
    QStringList statuses = {"Complete", "Complete", "In progress", "In progress", "Not started", "Not started"};
    
    for (int i = 0; i < tasks.size(); ++i) {
        QWidget *row = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 8, 12, 8);
        
        QLabel *idLabel = new QLabel(QString::number(i + 1));
        QLabel *taskLabel = new QLabel(tasks[i]);
        QLabel *statusLabel = new QLabel(statuses[i]);
        
        QString rowStyle = "QLabel { color: #374151; font-size: 14px; }";
        idLabel->setStyleSheet(rowStyle);
        taskLabel->setStyleSheet(rowStyle);
        
        // Diferentes colores para los estados
        QString statusColor = "#6B7280";
        if (statuses[i] == "Complete") statusColor = "#059669";
        else if (statuses[i] == "In progress") statusColor = "#D97706";
        
        statusLabel->setStyleSheet("QLabel { color: " + statusColor + "; font-size: 14px; font-weight: 500; }");
        
        rowLayout->addWidget(idLabel, 1);
        rowLayout->addWidget(taskLabel, 3);
        rowLayout->addWidget(statusLabel, 2);
        
        tableLayout->addWidget(row);
        
        // Línea separadora entre filas (excepto la última)
        if (i < tasks.size() - 1) {
            QFrame *rowSeparator = new QFrame();
            rowSeparator->setFrameShape(QFrame::HLine);
            rowSeparator->setStyleSheet("QFrame { color: #F3F4F6; }");
            tableLayout->addWidget(rowSeparator);
        }
    }
    
    contentLayout->addWidget(tableContainer);
    contentLayout->addStretch();
}

void MainWindow::onSidebarHover()
{
    if (!sidebarExpanded) {
        sidebarExpanded = true;
        sidebarAnimation->setStartValue(60);
        sidebarAnimation->setEndValue(200);
        sidebarAnimation->start();
        
        // Cambiar texto de los botones para mostrar labels
        QList<QPushButton*> buttons = {homeBtn, analyticsBtn, settingsBtn2, docsBtn, toolsBtn};
        QStringList icons = {"🏠", "📊", "⚙️", "📝", "🔧"};
        QStringList labels = {"Home", "Analytics", "Settings", "Docs", "Tools"};
        
        for (int i = 0; i < buttons.size(); ++i) {
            buttons[i]->setText(icons[i] + "  " + labels[i]);
            buttons[i]->setStyleSheet(buttons[i]->styleSheet() + "text-align: left; padding-left: 12px;");
        }
    }
}

void MainWindow::onSidebarLeave()
{
    if (sidebarExpanded) {
        sidebarExpanded = false;
        sidebarAnimation->setStartValue(200);
        sidebarAnimation->setEndValue(60);
        sidebarAnimation->start();
        
        // Restaurar solo iconos
        QList<QPushButton*> buttons = {homeBtn, analyticsBtn, settingsBtn2, docsBtn, toolsBtn};
        QStringList icons = {"🏠", "📊", "⚙️", "📝", "🔧"};
        
        for (int i = 0; i < buttons.size(); ++i) {
            buttons[i]->setText(icons[i]);
            QString style = buttons[i]->styleSheet();
            style.remove("text-align: left; padding-left: 12px;");
            buttons[i]->setStyleSheet(style);
        }
    }
}

void MainWindow::styleComponents()
{
    // Estilo general de la ventana
    setStyleSheet(
        "QMainWindow {"
            "background-color: #FAFAFA;"
        "}"
    );
}

void MainWindow::setProjectName(const QString &projectName)
{
    currentProjectName = projectName;
    setWindowTitle("MiniAccess - " + projectName);
    projectNameLabel->setText(projectName);
    projectTitleLabel->setText(projectName);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sidebarWidget) {
        if (event->type() == QEvent::Enter) {
            onSidebarHover();
        } else if (event->type() == QEvent::Leave) {
            onSidebarLeave();
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
