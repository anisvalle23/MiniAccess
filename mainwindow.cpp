#include "mainwindow.h"
#include "ThemeManager.h"
#include "ThemeTokens.h"
#include "TableEditor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), sidebarExpanded(false), currentViewIndex(0)
{
    setupUI();
    
    // Conectar al sistema de temas para actualizar estilos
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, 
            this, [this](ThemeManager::Theme) {
        updateThemeStyles();
        QApplication::processEvents();
        update();
    });
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
    
    // Create header first
    createHeader();
    mainLayout->addWidget(headerWidget);
    
    // Create main content first
    createMainContent();
    mainLayout->addWidget(mainContainer);
    
    // Create sidebar (as overlay)
    createSidebar();
    
    // Setup animations
    setupSidebarAnimation();
    
    // Apply styles
    styleComponents();
}

void MainWindow::createHeader()
{
    headerWidget = new QWidget();
    headerWidget->setFixedHeight(68);
    headerWidget->setStyleSheet(
        "QWidget {"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #FFFFFF, stop:1 #FEFEFE);"
            "border-bottom: 1px solid #E5E7EB;"
            "border-bottom-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                "stop:0 #E5E7EB, stop:0.5 #D1D5DB, stop:1 #E5E7EB);"
        "}"
    );
    
    headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(32, 0, 32, 0);
    headerLayout->setSpacing(20);
    
    // Logo de base de datos (izquierda)
    QLabel *dbLogoLabel = new QLabel("🗄");
    dbLogoLabel->setFont(QFont("Inter", 20, QFont::Medium));
    dbLogoLabel->setStyleSheet("QLabel { color: #A4373A; }");
    headerLayout->addWidget(dbLogoLabel);
    
    // Línea diagonal separadora
    QLabel *separatorLabel = new QLabel("/");
    separatorLabel->setFont(QFont("Inter", 18, QFont::Light));
    separatorLabel->setStyleSheet("QLabel { color: #D1D5DB; margin: 0 4px; }");
    headerLayout->addWidget(separatorLabel);
    
    // Icono de proyecto
    QLabel *projectIconLabel = new QLabel("📁");
    projectIconLabel->setFont(QFont("Inter", 18));
    projectIconLabel->setStyleSheet("QLabel { color: #6B7280; }");
    headerLayout->addWidget(projectIconLabel);
    
    // Nombre del proyecto con dropdown
    QWidget *projectWidget = new QWidget();
    QHBoxLayout *projectLayout = new QHBoxLayout(projectWidget);
    projectLayout->setContentsMargins(0, 0, 0, 0);
    projectLayout->setSpacing(8);
    
    projectNameLabel = new QLabel("proyecto1");
    projectNameLabel->setFont(QFont("Inter", 17, QFont::DemiBold));
    projectNameLabel->setStyleSheet("QLabel { color: #1F2937; letter-spacing: -0.5px; }");
    
    projectDropdownBtn = new QPushButton("⌄");
    projectDropdownBtn->setFixedSize(24, 24);
    projectDropdownBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "color: #9CA3AF;"
            "font-size: 16px;"
            "border-radius: 6px;"
            "font-weight: normal;"
        "}"
        "QPushButton:hover {"
            "background-color: rgba(164, 55, 58, 0.1);"
            "color: #A4373A;"
            "transform: scale(1.1);"
        "}"
        "QPushButton:pressed {"
            "background-color: rgba(164, 55, 58, 0.15);"
            "transform: scale(0.95);"
        "}"
    );
    
    projectLayout->addWidget(projectNameLabel);
    projectLayout->addWidget(projectDropdownBtn);
    headerLayout->addWidget(projectWidget);
    
    // Espaciador flexible
    headerLayout->addStretch();
    
    // Botón de configuraciones (copiado de CreateProject)
    settingsBtn = new QPushButton();
    settingsBtn->setFixedSize(40, 40);
    settingsBtn->setText("⚙");
    settingsBtn->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #FFFFFF, stop:1 #F9FAFB);"
        "    border: 1px solid #E5E7EB;"
        "    color: #A4373A;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    border-radius: 20px;"
        "    padding: 0;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #FEFEFE, stop:1 rgba(164, 55, 58, 0.05));"
        "    border: 1px solid #A4373A;"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 4px 12px rgba(164, 55, 58, 0.15);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(164, 55, 58, 0.1);"
        "    border: 1px solid #A4373A;"
        "    transform: translateY(0px);"
        "    box-shadow: 0 2px 4px rgba(164, 55, 58, 0.2);"
        "}"
    );
    
    headerLayout->addWidget(settingsBtn);
    
    // Conectar el botón de configuraciones para cambiar tema
    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        ThemeManager::instance().toggleTheme();
    });
}

void MainWindow::createSidebar()
{
    // Sidebar overlay container (positioned absolutely)
    sidebarOverlay = new QWidget(centralWidget);
    sidebarOverlay->setGeometry(0, 68, 72, 750 - 68); // Start collapsed
    sidebarOverlay->setStyleSheet("background-color: transparent;");
    sidebarOverlay->raise(); // Ensure it's above main content
    
    // Actual sidebar widget
    sidebarWidget = new QWidget(sidebarOverlay);
    sidebarWidget->setFixedSize(72, 750 - 68);
    sidebarWidget->setStyleSheet(
        "QWidget {"
            "background: #A4373A;"  // Burgundy background
            "border-right: 1px solid #8B2635;"
        "}"
    );
    
    // Shadow effect for expanded state
    sidebarShadowEffect = new QGraphicsDropShadowEffect();
    sidebarShadowEffect->setBlurRadius(20);
    sidebarShadowEffect->setColor(QColor(0, 0, 0, 60));
    sidebarShadowEffect->setOffset(4, 0);
    sidebarShadowEffect->setEnabled(false);
    sidebarWidget->setGraphicsEffect(sidebarShadowEffect);
    
    // Enable mouse tracking for hover detection
    sidebarWidget->setMouseTracking(true);
    sidebarWidget->installEventFilter(this);
    
    sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(12, 16, 12, 16);
    sidebarLayout->setSpacing(8);
    
    // Sidebar items
    QStringList sidebarIcons = {"🏠", "📊", "�", "�", "🔐", "📁", "⚡", "📡", "�", "📋", "📚", "🔌", "⚙"};
    QStringList sidebarLabels = {"Project overview", "Table Editor", "SQL Editor", "Database", "Authentication", "Storage", "Edge Functions", "Realtime", "Advisors", "Reports", "Logs", "API Docs", "Integrations"};
    
    for (int i = 0; i < sidebarIcons.size(); ++i) {
        // Create container for each sidebar item
        QWidget *itemContainer = new QWidget();
        itemContainer->setFixedHeight(48);
        
        QHBoxLayout *itemLayout = new QHBoxLayout(itemContainer);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(12);
        
        // Icon button
        QPushButton *btn = new QPushButton(sidebarIcons[i]);
        btn->setFixedSize(48, 48);
        btn->setProperty("labelText", sidebarLabels[i]);
        btn->setProperty("iconText", sidebarIcons[i]);
        
        QString buttonStyle = 
            "QPushButton {"
                "background: transparent;"
                "border: none;"
                "border-radius: 8px;"
                "font-size: 20px;"
                "color: #FFFFFF;"  // White icons on burgundy
                "text-align: center;"
                "font-weight: 500;"
                "padding: 2px;"
            "}"
            "QPushButton:hover {"
                "background: rgba(255, 255, 255, 0.15);"  // White hover effect
                "color: #FFFFFF;"
                "border-radius: 8px;"
            "}";
        
        // First button (Inicio) is active
        if (i == 0) {
            buttonStyle = 
                "QPushButton {"
                    "background: rgba(255, 255, 255, 0.2);"  // More prominent white background
                    "border: none;"
                    "border-radius: 8px;"
                    "font-size: 20px;"
                    "color: #FFFFFF;"
                    "text-align: center;"
                    "font-weight: 600;"
                    "padding: 2px;"
                "}"
                "QPushButton:hover {"
                    "background: rgba(255, 255, 255, 0.25);"
                "}";
        }
        
        btn->setStyleSheet(buttonStyle);
        
        // Connect click signal
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            updateSidebarSelection(i);
            switchToView(i);
        });
        
        // Label (initially hidden)
        QLabel *label = new QLabel(sidebarLabels[i]);
        label->setFont(QFont("Inter", 14, QFont::Medium));
        label->setStyleSheet(
            "QLabel {"
                "color: #FFFFFF;"  // White text on burgundy
                "font-weight: 500;"
                "padding: 0;"
                "margin: 0;"
            "}"
        );
        label->setVisible(false);
        
        itemLayout->addWidget(btn);
        itemLayout->addWidget(label);
        itemLayout->addStretch();
        
        sidebarButtons.append(btn);
        sidebarLabelsWidgets.append(label);
        
        sidebarLayout->addWidget(itemContainer);
    }
    
    sidebarLayout->addStretch();
    
    // No need for mouse tracking since sidebar doesn't expand
}

void MainWindow::createMainContent()
{
    // Main container
    mainContainer = new QWidget();
    mainContainer->setStyleSheet(
        "QWidget { "
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 #FAFAFA, stop:1 #F5F5F5);"
        " }"
    );
    
    mainContainerLayout = new QVBoxLayout(mainContainer);
    mainContainerLayout->setContentsMargins(120, 0, 0, 0); // Left margin for sidebar, no top margin
    mainContainerLayout->setSpacing(0);
    
    // Create stacked widget for different views
    stackedWidget = new QStackedWidget();
    
    // Create home view
    createHomeView();
    
    // Create table editor view
    tableEditorView = new TableEditor();
    
    // Add views to stacked widget
    stackedWidget->addWidget(homeView);     // Index 0
    stackedWidget->addWidget(tableEditorView); // Index 1
    
    // Set initial view
    stackedWidget->setCurrentIndex(0);
    
    mainContainerLayout->addWidget(stackedWidget);
}

void MainWindow::createHomeView()
{
    homeView = new QWidget();
    homeViewLayout = new QVBoxLayout(homeView);
    homeViewLayout->setContentsMargins(40, 40, 40, 40);
    homeViewLayout->setSpacing(32);
    
    createWelcomeSection();
    createDatabaseSection();
    createExploreSection();
}

void MainWindow::createWelcomeSection()
{
    // Welcome message
    welcomeLabel = new QLabel("Bienvenido a tu nuevo proyecto");
    welcomeLabel->setFont(QFont("Inter", 32, QFont::Bold));
    welcomeLabel->setStyleSheet(
        "QLabel { "
            "color: #111827; "
            "letter-spacing: -1px; "
            "margin-bottom: 8px;"
        " }"
    );
    homeViewLayout->addWidget(welcomeLabel);
    
    // Description
    descriptionLabel = new QLabel("Tu proyecto has been deployed on its own instance, with its own API all set up and ready to use.");
    descriptionLabel->setFont(QFont("Inter", 18, QFont::Normal));
    descriptionLabel->setStyleSheet(
        "QLabel { "
            "color: #6B7280; "
            "line-height: 1.6; "
            "letter-spacing: -0.2px;"
        " }"
    );
    descriptionLabel->setWordWrap(true);
    homeViewLayout->addWidget(descriptionLabel);
}

void MainWindow::createDatabaseSection()
{
    // Database section title
    QLabel *dbTitle = new QLabel("Empieza construyendo tu base de datos");
    dbTitle->setFont(QFont("Inter", 24, QFont::Bold));
    dbTitle->setStyleSheet(
        "QLabel { "
            "color: #111827; "
            "margin-top: 24px; "
            "margin-bottom: 12px; "
            "letter-spacing: -0.5px;"
        " }"
    );
    homeViewLayout->addWidget(dbTitle);
    
    // Database description
    QLabel *dbDesc = new QLabel("Comienza construyendo tu aplicación creando tablas e insertando datos. Nuestro Editor de Tablas hace que Postgres sea tan fácil de usar como una hoja de cálculo, pero también está nuestro Editor SQL si necesitas algo más avanzado.");
    dbDesc->setFont(QFont("Inter", 16));
    dbDesc->setStyleSheet(
        "QLabel { "
            "color: #6B7280; "
            "line-height: 1.7; "
            "letter-spacing: -0.1px;"
        " }"
    );
    dbDesc->setWordWrap(true);
    homeViewLayout->addWidget(dbDesc);
}

void MainWindow::createExploreSection()
{
    // Explore section title
    QLabel *exploreTitle = new QLabel("Explora nuestros otros productos");
    exploreTitle->setFont(QFont("Inter", 24, QFont::Bold));
    exploreTitle->setStyleSheet(
        "QLabel { "
            "color: #111827; "
            "margin-top: 32px; "
            "margin-bottom: 12px; "
            "letter-spacing: -0.5px;"
        " }"
    );
    homeViewLayout->addWidget(exploreTitle);
    
    // Explore description
    QLabel *exploreDesc = new QLabel("MiniAccess proporciona todas las características de backend que necesitas para construir un producto. Puedes usarlo completamente, o solo las características que necesites.");
    exploreDesc->setFont(QFont("Inter", 16));
    exploreDesc->setStyleSheet(
        "QLabel { "
            "color: #6B7280; "
            "line-height: 1.7; "
            "letter-spacing: -0.1px;"
        " }"
    );
    exploreDesc->setWordWrap(true);
    homeViewLayout->addWidget(exploreDesc);
    
    homeViewLayout->addStretch();
}

void MainWindow::setupSidebarAnimation()
{
    // No animations needed since sidebar doesn't expand
}

void MainWindow::updateSidebarItems(bool expanded)
{
    // No expansion, buttons remain as icons only
}

void MainWindow::onSettingsButtonHover()
{
    // Settings button hover effect
}

void MainWindow::onSettingsButtonLeave()
{
    // Settings button leave effect
}

void MainWindow::styleComponents()
{
    updateThemeStyles();
}

void MainWindow::updateThemeStyles()
{
    bool isDark = ThemeManager::instance().isDark();
    
    // Main window styling - cambia según el tema
    QString backgroundColor = isDark ? "#0F0F11" : "#FAFAFA";
    QString backgroundGradient = isDark ? 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0F0F11, stop:1 #1A1A1D)" :
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FAFAFA, stop:1 #F5F5F5)";
    
    setStyleSheet(QString(
        "QMainWindow {"
            "background: %1;"
            "font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;"
        "}"
        "QLabel {"
            "font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;"
        "}"
        "QPushButton {"
            "font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;"
        "}"
    ).arg(backgroundGradient));
    
    // Actualizar header
    updateHeaderTheme(isDark);
    
    // Actualizar sidebar
    updateSidebarTheme(isDark);
    
    // Actualizar main content
    updateMainContentTheme(isDark);
}

void MainWindow::updateHeaderTheme(bool isDark)
{
    QString headerBackground = isDark ?
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1A1A1D, stop:1 #0F0F11)" :
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #FEFEFE)";
    
    QString borderColor = isDark ? "#2D2D30" : "#E5E7EB";
    
    headerWidget->setStyleSheet(QString(
        "QWidget {"
            "background: %1;"
            "border-bottom: 1px solid %2;"
        "}"
    ).arg(headerBackground, borderColor));
    
    // Update text colors in header
    QString textColor = isDark ? "#E5E7EB" : "#1F2937";
    projectNameLabel->setStyleSheet(QString("QLabel { color: %1; letter-spacing: -0.5px; }").arg(textColor));
    
    // Update settings button for theme consistency
    QString settingsBtnStyle = QString(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 %1, stop:1 %2);"
        "    border: 1px solid %3;"
        "    color: #A4373A;"  // Burgundy color
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    border-radius: 20px;"
        "    padding: 0;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                "stop:0 %4, stop:1 rgba(164, 55, 58, 0.05));"
        "    border: 1px solid #A4373A;"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 4px 12px rgba(164, 55, 58, 0.15);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(164, 55, 58, 0.1);"
        "    border: 1px solid #A4373A;"
        "    transform: translateY(0px);"
        "    box-shadow: 0 2px 4px rgba(164, 55, 58, 0.2);"
        "}"
    ).arg(
        isDark ? "#2D2D30" : "#FFFFFF",  // Background top
        isDark ? "#1A1A1D" : "#F9FAFB",  // Background bottom
        isDark ? "#3D3D40" : "#E5E7EB",  // Border
        isDark ? "#3D3D40" : "#FEFEFE"   // Hover background top
    );
    
    settingsBtn->setStyleSheet(settingsBtnStyle);
}

void MainWindow::setProjectName(const QString &projectName)
{
    currentProjectName = projectName;
    setWindowTitle("MiniAccess - " + projectName);
    projectNameLabel->setText(projectName);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == sidebarWidget) {
        if (event->type() == QEvent::Enter) {
            onSidebarEnter();
            return true;
        } else if (event->type() == QEvent::Leave) {
            onSidebarLeave();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onSidebarEnter()
{
    if (sidebarExpanded) return;
    
    // Expand sidebar width
    sidebarOverlay->setGeometry(0, 68, 240, 750 - 68);
    sidebarWidget->setFixedWidth(240);
    sidebarShadowEffect->setEnabled(true);
    
    // Show labels
    for (QLabel* label : sidebarLabelsWidgets) {
        label->setVisible(true);
    }
    
    sidebarExpanded = true;
}

void MainWindow::onSidebarLeave()
{
    if (!sidebarExpanded) return;
    
    // Collapse sidebar width
    sidebarOverlay->setGeometry(0, 68, 72, 750 - 68);
    sidebarWidget->setFixedWidth(72);
    sidebarShadowEffect->setEnabled(false);
    
    // Hide labels
    for (QLabel* label : sidebarLabelsWidgets) {
        label->setVisible(false);
    }
    
    sidebarExpanded = false;
}

void MainWindow::onSidebarItemClicked()
{
    // This function is handled by lambda functions in button connections
}

void MainWindow::updateSidebarTheme(bool isDark)
{
    // Sidebar always uses burgundy but with different shades
    QString sidebarBackground = isDark ? "#8B2635" : "#A4373A";  // Darker burgundy for dark theme
    QString borderColor = isDark ? "#6D1D29" : "#8B2635";
    
    sidebarWidget->setStyleSheet(QString(
        "QWidget {"
            "background: %1;"
            "border-right: 1px solid %2;"
        "}"
    ).arg(sidebarBackground, borderColor));
    
    // Update sidebar button colors for both themes
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        QString iconColor = "#FFFFFF";  // Always white icons on burgundy
        QString hoverBg = isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(255, 255, 255, 0.15)";
        
        QString buttonStyle = QString(
            "QPushButton {"
                "background: transparent;"
                "border: none;"
                "border-radius: 8px;"
                "font-size: 20px;"
                "color: %1;"
                "text-align: center;"
                "font-weight: 500;"
                "padding: 2px;"
            "}"
            "QPushButton:hover {"
                "background: %2;"
                "color: #FFFFFF;"
                "border-radius: 8px;"
            "}"
        ).arg(iconColor, hoverBg);
        
        // First button (Inicio) is active
        if (i == 0) {
            QString activeBg = isDark ? "rgba(255, 255, 255, 0.15)" : "rgba(255, 255, 255, 0.2)";
            
            buttonStyle = QString(
                "QPushButton {"
                    "background: %1;"
                    "border: none;"
                    "border-radius: 8px;"
                    "font-size: 20px;"
                    "color: #FFFFFF;"
                    "text-align: center;"
                    "font-weight: 600;"
                    "padding: 2px;"
                "}"
                "QPushButton:hover {"
                    "background: %2;"
                "}"
            ).arg(activeBg, hoverBg);
        }
        
        btn->setStyleSheet(buttonStyle);
    }
    
    // Update sidebar label colors
    for (QLabel* label : sidebarLabelsWidgets) {
        label->setStyleSheet(
            "QLabel {"
                "color: #FFFFFF;"  // Always white text on burgundy
                "font-weight: 500;"
                "padding: 0;"
                "margin: 0;"
            "}"
        );
    }
}

void MainWindow::updateMainContentTheme(bool isDark)
{
    QString mainBackground = isDark ?
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0F0F11, stop:1 #1A1A1D)" :
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FAFAFA, stop:1 #F5F5F5)";
    
    mainContainer->setStyleSheet(QString(
        "QWidget { background: %1; }"
    ).arg(mainBackground));
    
    // Actualizar colores del texto
    QString titleColor = isDark ? "#F9FAFB" : "#111827";
    QString descColor = isDark ? "#9CA3AF" : "#6B7280";
    
    welcomeLabel->setStyleSheet(QString(
        "QLabel { "
            "color: %1; "
            "letter-spacing: -1px; "
            "margin-bottom: 8px;"
        " }"
    ).arg(titleColor));
    
    descriptionLabel->setStyleSheet(QString(
        "QLabel { "
            "color: %1; "
            "line-height: 1.6; "
            "letter-spacing: -0.2px;"
        " }"
    ).arg(descColor));
    
    // Actualizar todas las otras secciones
    QList<QLabel*> labels = mainContainer->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label != welcomeLabel && label != descriptionLabel) {
            if (label->font().pointSize() >= 20) {
                // Es un título
                label->setStyleSheet(QString(
                    "QLabel { "
                        "color: %1; "
                        "margin-top: 24px; "
                        "margin-bottom: 12px; "
                        "letter-spacing: -0.5px;"
                    " }"
                ).arg(titleColor));
            } else {
                // Es una descripción
                label->setStyleSheet(QString(
                    "QLabel { "
                        "color: %1; "
                        "line-height: 1.7; "
                        "letter-spacing: -0.1px;"
                    " }"
                ).arg(descColor));
            }
        }
    }
}

void MainWindow::switchToView(int viewIndex)
{
    currentViewIndex = viewIndex;
    
    switch(viewIndex) {
        case 0:
            // Home view
            stackedWidget->setCurrentIndex(0);
            break;
        case 1:
            // Table Editor view
            stackedWidget->setCurrentIndex(1);
            // Update table editor theme if needed
            tableEditorView->updateTheme(ThemeManager::instance().isDark());
            break;
        default:
            // For now, other views will show home
            stackedWidget->setCurrentIndex(0);
            break;
    }
}

void MainWindow::updateSidebarSelection(int selectedIndex)
{
    // Update button styles to show selection
    for (int i = 0; i < sidebarButtons.size(); ++i) {
        QPushButton *btn = sidebarButtons[i];
        bool isDark = ThemeManager::instance().isDark();
        
        QString iconColor = "#FFFFFF";
        QString hoverBg = isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(255, 255, 255, 0.15)";
        
        QString buttonStyle;
        
        if (i == selectedIndex) {
            // Selected button style
            QString activeBg = isDark ? "rgba(255, 255, 255, 0.15)" : "rgba(255, 255, 255, 0.2)";
            
            buttonStyle = QString(
                "QPushButton {"
                    "background: %1;"
                    "border: none;"
                    "border-radius: 8px;"
                    "font-size: 20px;"
                    "color: #FFFFFF;"
                    "text-align: center;"
                    "font-weight: 600;"
                    "padding: 2px;"
                "}"
                "QPushButton:hover {"
                    "background: %2;"
                "}"
            ).arg(activeBg, hoverBg);
        } else {
            // Normal button style
            buttonStyle = QString(
                "QPushButton {"
                    "background: transparent;"
                    "border: none;"
                    "border-radius: 8px;"
                    "font-size: 20px;"
                    "color: %1;"
                    "text-align: center;"
                    "font-weight: 500;"
                    "padding: 2px;"
                "}"
                "QPushButton:hover {"
                    "background: %2;"
                    "color: #FFFFFF;"
                    "border-radius: 8px;"
                "}"
            ).arg(iconColor, hoverBg);
        }
        
        btn->setStyleSheet(buttonStyle);
    }
}
