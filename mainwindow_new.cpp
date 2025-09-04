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
    
    // Settings button
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
    // Settings button hover effect
}

void MainWindow::onSettingsButtonLeave()
{
    // Settings button leave effect
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
