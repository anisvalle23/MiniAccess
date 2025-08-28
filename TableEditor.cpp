#include "TableEditor.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

TableEditor::TableEditor(QWidget *parent)
    : QWidget(parent), isDarkTheme(false)
{
    setupUI();
    styleComponents();
}

void TableEditor::setupUI()
{
    // Main layout
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create splitter for resizable panels
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(1);
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
            "background-color: #E5E7EB;"
            "width: 1px;"
        "}"
    );
    
    createLeftPanel();
    createRightPanel();
    createTableCreationPanel();
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    
    // Set initial sizes (left panel smaller)
    mainSplitter->setSizes({300, 900});
    mainSplitter->setCollapsible(0, false);
    mainSplitter->setCollapsible(1, false);
    
    mainLayout->addWidget(mainSplitter);
}

void TableEditor::createLeftPanel()
{
    leftPanel = new QWidget();
    leftPanel->setFixedWidth(300);
    leftPanel->setStyleSheet(
        "QWidget {"
            "background-color: #FAFAFA;"
            "border-right: 1px solid #E5E7EB;"
        "}"
    );
    
    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(16, 16, 16, 16);
    leftPanelLayout->setSpacing(16);
    
    // New table button
    newTableBtn = new QPushButton("Nueva Tabla");
    newTableBtn->setFont(QFont("Inter", 14, QFont::Medium));
    newTableBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #A4373A;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 12px 16px;"
            "font-weight: 500;"
            "text-align: center;"
        "}"
        "QPushButton:hover {"
            "background-color: #8B2635;"
        "}"
        "QPushButton:pressed {"
            "background-color: #6D1D29;"
        "}"
    );
    leftPanelLayout->addWidget(newTableBtn);
    
    // Table list section
    createTableList();
    
    leftPanelLayout->addStretch();
    
    // Connect signals
    connect(newTableBtn, &QPushButton::clicked, this, &TableEditor::onNewTableClicked);
}

void TableEditor::createTableList()
{
    tableListSection = new QWidget();
    QVBoxLayout *tableListLayout = new QVBoxLayout(tableListSection);
    tableListLayout->setContentsMargins(0, 0, 0, 0);
    tableListLayout->setSpacing(8);
    
    // Search and filter header
    QWidget *searchHeader = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchHeader);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(8);
    
    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Buscar tablas");
    searchBox->setFont(QFont("Inter", 12));
    searchBox->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 6px 10px;"
            "font-size: 13px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
    );
    
    searchLayout->addWidget(searchBox);
    tableListLayout->addWidget(searchHeader);
    
    // Tables tree
    tableTree = new QTreeWidget();
    tableTree->setHeaderHidden(true);
    tableTree->setRootIsDecorated(false);
    tableTree->setStyleSheet(
        "QTreeWidget {"
            "background-color: transparent;"
            "border: none;"
            "outline: none;"
            "font-family: 'Inter';"
            "font-size: 13px;"
        "}"
        "QTreeWidget::item {"
            "height: 32px;"
            "padding: 4px 8px;"
            "border-radius: 6px;"
            "color: #374151;"
        "}"
        "QTreeWidget::item:hover {"
            "background-color: #F3F4F6;"
        "}"
        "QTreeWidget::item:selected {"
            "background-color: #EFF6FF;"
            "color: #1D4ED8;"
        "}"
    );
    
    // Add empty state
    QLabel *noTablesLabel = new QLabel("Sin tablas o vistas");
    noTablesLabel->setFont(QFont("Inter", 13, QFont::Medium));
    noTablesLabel->setStyleSheet("QLabel { color: #6B7280; }");
    noTablesLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *noTablesDesc = new QLabel("Cualquier tabla o vista que crees\nse listará aquí.");
    noTablesDesc->setFont(QFont("Inter", 12));
    noTablesDesc->setStyleSheet("QLabel { color: #9CA3AF; line-height: 1.5; }");
    noTablesDesc->setAlignment(Qt::AlignCenter);
    noTablesDesc->setWordWrap(true);
    
    // Create empty state widget
    QWidget *emptyState = new QWidget();
    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyState);
    emptyLayout->setContentsMargins(16, 32, 16, 32);
    emptyLayout->setSpacing(8);
    emptyLayout->addWidget(noTablesLabel);
    emptyLayout->addWidget(noTablesDesc);
    
    tableListLayout->addWidget(emptyState);
    
    leftPanelLayout->addWidget(tableListSection);
}

void TableEditor::createRightPanel()
{
    rightPanel = new QWidget();
    rightPanel->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
        "}"
    );
    
    rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(0);
    
    createToolbar();
    createMainTableArea();
}

void TableEditor::createToolbar()
{
    toolbar = new QWidget();
    toolbar->setFixedHeight(60);
    toolbar->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-bottom: 1px solid #E5E7EB;"
        "}"
    );
    
    toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(24, 0, 24, 0);
    toolbarLayout->setSpacing(16);
    
    // Title
    toolbarTitle = new QLabel("Editor de Tabla");
    toolbarTitle->setFont(QFont("Inter", 20, QFont::Bold));
    toolbarTitle->setStyleSheet("QLabel { color: #111827; }");
    
    toolbarLayout->addWidget(toolbarTitle);
    toolbarLayout->addStretch();
    
    rightPanelLayout->addWidget(toolbar);
}

void TableEditor::createMainTableArea()
{
    mainContentArea = new QWidget();
    mainContentArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    
    mainContentLayout = new QVBoxLayout(mainContentArea);
    mainContentLayout->setContentsMargins(40, 40, 40, 40);
    mainContentLayout->setSpacing(32);
    
    // Create central card similar to the second image
    createTableCard = new ClickableWidget();
    createTableCard->setFixedSize(450, 100);
    createTableCard->setStyleSheet(
        "ClickableWidget {"
            "background-color: #1F2937;"
            "border-radius: 8px;"
            "border: 1px solid #374151;"
        "}"
        "ClickableWidget:hover {"
            "background-color: #374151;"
            "border-color: #4B5563;"
        "}"
    );
    createTableCard->setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout *cardLayout = new QHBoxLayout(createTableCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(12);
    cardLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // Icon (similar to the database icon in the image)
    QLabel *iconLabel = new QLabel("�");
    iconLabel->setFont(QFont("Inter", 32));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("QLabel { color: #60A5FA; }");
    // Text content
    QWidget *textWidget = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textWidget);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(4);
    
    QLabel *cardTitle = new QLabel("Crear una tabla");
    cardTitle->setFont(QFont("Inter", 16, QFont::Bold));
    cardTitle->setStyleSheet("QLabel { color: #FFFFFF; }");
    
    QLabel *cardDesc = new QLabel("Diseña y crea una nueva tabla de base de datos");
    cardDesc->setFont(QFont("Inter", 12));
    cardDesc->setStyleSheet("QLabel { color: #9CA3AF; }");
    cardDesc->setWordWrap(true);
    
    textLayout->addWidget(cardTitle);
    textLayout->addWidget(cardDesc);
    
    cardLayout->addWidget(iconLabel);
    cardLayout->addWidget(textWidget);
    cardLayout->addStretch();
    
    // Center the card horizontally
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(createTableCard);
    centerLayout->addStretch();
    
    mainContentLayout->addLayout(centerLayout);
    
    // Recent items section
    QLabel *recentTitle = new QLabel("Elementos recientes");
    recentTitle->setFont(QFont("Inter", 18, QFont::Medium));
    recentTitle->setStyleSheet("QLabel { color: #111827; margin-top: 32px; }");
    
    QLabel *recentEmpty = new QLabel("Aún no hay elementos recientes");
    recentEmpty->setFont(QFont("Inter", 14));
    recentEmpty->setStyleSheet("QLabel { color: #9CA3AF; }");
    recentEmpty->setAlignment(Qt::AlignCenter);
    
    QLabel *recentDesc = new QLabel("Los elementos aparecerán aquí mientras navegas por tu proyecto");
    recentDesc->setFont(QFont("Inter", 13));
    recentDesc->setStyleSheet("QLabel { color: #9CA3AF; }");
    recentDesc->setAlignment(Qt::AlignCenter);
    
    mainContentLayout->addWidget(recentTitle);
    mainContentLayout->addWidget(recentEmpty);
    mainContentLayout->addWidget(recentDesc);
    mainContentLayout->addStretch();
    
    // Connect card click event
    connect(createTableCard, &ClickableWidget::clicked, this, &TableEditor::onNewTableClicked);
    
    rightPanelLayout->addWidget(mainContentArea);
}

void TableEditor::createTableCreationPanel()
{
    createTablePanel = new QWidget(this);
    createTablePanel->setFixedWidth(400);
    createTablePanel->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-left: 1px solid #E5E7EB;"
        "}"
    );
    
    createTablePanelLayout = new QVBoxLayout(createTablePanel);
    createTablePanelLayout->setContentsMargins(24, 24, 24, 24);
    createTablePanelLayout->setSpacing(24);
    
    // Header section
    QWidget *headerWidget = new QWidget();
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    
    QLabel *titleLabel = new QLabel("Columnas");
    titleLabel->setFont(QFont("Inter", 20, QFont::Bold));
    titleLabel->setStyleSheet("QLabel { color: #111827; }");
    
    // Buttons section (top right)
    QWidget *buttonsWidget = new QWidget();
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(12);
    
    QPushButton *aboutBtn = new QPushButton("📝 Acerca de tipos de datos");
    aboutBtn->setFont(QFont("Inter", 12));
    aboutBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F3F4F6;"
            "color: #374151;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
            "background-color: #E5E7EB;"
        "}"
    );
    
    QPushButton *importBtn = new QPushButton("Importar datos desde CSV");
    importBtn->setFont(QFont("Inter", 12));
    importBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F3F4F6;"
            "color: #374151;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
            "background-color: #E5E7EB;"
        "}"
    );
    
    buttonsLayout->addWidget(aboutBtn);
    buttonsLayout->addWidget(importBtn);
    buttonsLayout->addStretch();
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(buttonsWidget);
    createTablePanelLayout->addWidget(headerWidget);
    
    // Table name input
    QWidget *nameWidget = new QWidget();
    QVBoxLayout *nameLayout = new QVBoxLayout(nameWidget);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(8);
    
    QLabel *nameLabel = new QLabel("Nombre de la tabla");
    nameLabel->setFont(QFont("Inter", 14, QFont::Medium));
    nameLabel->setStyleSheet("QLabel { color: #374151; }");
    
    tableNameInput = new QLineEdit();
    tableNameInput->setPlaceholderText("Ingresa el nombre de la tabla");
    tableNameInput->setFont(QFont("Inter", 14));
    tableNameInput->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 12px;"
            "font-size: 14px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
        "QLineEdit:focus {"
            "border-color: #A4373A;"
            "outline: none;"
        "}"
    );
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(tableNameInput);
    createTablePanelLayout->addWidget(nameWidget);
    
    // Columns section
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");
    
    columnsArea = new QWidget();
    columnsLayout = new QVBoxLayout(columnsArea);
    columnsLayout->setContentsMargins(0, 0, 0, 0);
    columnsLayout->setSpacing(12);
    
    // Add default ID column
    QWidget *idColumn = createColumnRow("id", "int", true);
    columnsLayout->addWidget(idColumn);
    
    // Add default created_at column
    QWidget *createdAtColumn = createColumnRow("created_at", "timestamp", false);
    columnsLayout->addWidget(createdAtColumn);
    
    columnsLayout->addStretch();
    scrollArea->setWidget(columnsArea);
    createTablePanelLayout->addWidget(scrollArea);
    
    // Add column button
    addColumnBtn = new QPushButton("Agregar columna");
    addColumnBtn->setFont(QFont("Inter", 14, QFont::Medium));
    addColumnBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F3F4F6;"
            "color: #374151;"
            "border: 1px dashed #D1D5DB;"
            "border-radius: 8px;"
            "padding: 12px;"
            "text-align: center;"
        "}"
        "QPushButton:hover {"
            "background-color: #E5E7EB;"
        "}"
    );
    createTablePanelLayout->addWidget(addColumnBtn);
    
    // Bottom buttons
    QWidget *bottomButtons = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomButtons);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(12);
    
    cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setFont(QFont("Inter", 14, QFont::Medium));
    cancelBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F3F4F6;"
            "color: #374151;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 12px 24px;"
        "}"
        "QPushButton:hover {"
            "background-color: #E5E7EB;"
        "}"
    );
    
    saveBtn = new QPushButton("Guardar");
    saveBtn->setFont(QFont("Inter", 14, QFont::Medium));
    saveBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #059669;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 6px;"
            "padding: 12px 24px;"
        "}"
        "QPushButton:hover {"
            "background-color: #047857;"
        "}"
    );
    
    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(saveBtn);
    createTablePanelLayout->addWidget(bottomButtons);
    
    // Initially hide the panel
    createTablePanel->setGeometry(width(), 0, 400, height());
    createTablePanel->hide();
    
    // Connect signals
    connect(addColumnBtn, &QPushButton::clicked, this, &TableEditor::onAddColumnClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &TableEditor::onCancelClicked);
    connect(saveBtn, &QPushButton::clicked, this, &TableEditor::onSaveClicked);
    
    // Setup animation
    panelAnimation = new QPropertyAnimation(createTablePanel, "geometry");
    panelAnimation->setDuration(300);
    panelAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TableEditor::styleComponents()
{
    updateTheme(isDarkTheme);
}

void TableEditor::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateLeftPanelTheme(isDark);
    updateRightPanelTheme(isDark);
    updateToolbarTheme(isDark);
    updateTableTheme(isDark);
}

void TableEditor::updateLeftPanelTheme(bool isDark)
{
    QString bgColor = isDark ? "#1F2937" : "#FAFAFA";
    QString borderColor = isDark ? "#374151" : "#E5E7EB";
    
    leftPanel->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
            "border-right: 1px solid %2;"
        "}"
    ).arg(bgColor, borderColor));
    
    // Update New Table button theme
    QString newTableBtnStyle = QString(
        "QPushButton {"
            "background-color: #A4373A;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 12px 16px;"
            "font-weight: 500;"
            "text-align: center;"
        "}"
        "QPushButton:hover {"
            "background-color: #8B2635;"
        "}"
        "QPushButton:pressed {"
            "background-color: #6D1D29;"
        "}"
    );
    newTableBtn->setStyleSheet(newTableBtnStyle);
    
    // Update search box and filter button
    updateSearchComponentsTheme(isDark);
    
    // Update tree widget theme
    updateTreeWidgetTheme(isDark);
    
    // Update empty state labels
    updateEmptyStateTheme(isDark);
}

void TableEditor::updateRightPanelTheme(bool isDark)
{
    QString bgColor = isDark ? "#111827" : "#FFFFFF";
    
    rightPanel->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
        "}"
    ).arg(bgColor));
    
    mainContentArea->setStyleSheet(QString("QWidget { background-color: %1; }").arg(bgColor));
    
    // Update text colors
    QString titleColor = isDark ? "#F9FAFB" : "#111827";
    QString descColor = isDark ? "#9CA3AF" : "#6B7280";
    
    // Update all labels in the main content area
    QList<QLabel*> allLabels = mainContentArea->findChildren<QLabel*>();
    for (QLabel* label : allLabels) {
        if (label->font().pointSize() >= 18) {
            // Title labels
            label->setStyleSheet(QString("QLabel { color: %1; margin-top: 32px; }").arg(titleColor));
        } else {
            // Description labels
            label->setStyleSheet(QString("QLabel { color: %1; }").arg(descColor));
        }
    }
}

void TableEditor::updateToolbarTheme(bool isDark)
{
    QString bgColor = isDark ? "#111827" : "#FFFFFF";
    QString borderColor = isDark ? "#374151" : "#E5E7EB";
    QString titleColor = isDark ? "#F9FAFB" : "#111827";
    
    toolbar->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
            "border-bottom: 1px solid %2;"
        "}"
    ).arg(bgColor, borderColor));
    
    toolbarTitle->setStyleSheet(QString("QLabel { color: %1; }").arg(titleColor));
}

void TableEditor::updateTableTheme(bool isDark)
{
    if (!tableView) return;
    
    QString bgColor = isDark ? "#111827" : "#FFFFFF";
    QString borderColor = isDark ? "#374151" : "#E5E7EB";
    QString headerBgColor = isDark ? "#1F2937" : "#F9FAFB";
    QString textColor = isDark ? "#F9FAFB" : "#111827";
    QString headerTextColor = isDark ? "#F3F4F6" : "#374151";
    QString selectionColor = isDark ? "#374151" : "#EBF4FF";
    QString alternateRowColor = isDark ? "#1F2937" : "#F8FAFC";
    
    tableView->setStyleSheet(QString(
        "QTableView {"
            "background-color: %1;"
            "border: 1px solid %2;"
            "border-radius: 8px;"
            "gridline-color: %2;"
            "color: %3;"
            "selection-background-color: %6;"
            "alternate-background-color: %7;"
        "}"
        "QTableView::item {"
            "padding: 8px 12px;"
            "border-bottom: 1px solid %2;"
        "}"
        "QTableView::item:selected {"
            "background-color: %6;"
            "color: %3;"
        "}"
        "QHeaderView::section {"
            "background-color: %4;"
            "color: %5;"
            "padding: 12px 16px;"
            "border: none;"
            "border-bottom: 2px solid %2;"
            "border-right: 1px solid %2;"
            "font-weight: 600;"
        "}"
        "QHeaderView::section:horizontal {"
            "border-top-left-radius: 8px;"
            "border-top-right-radius: 8px;"
        "}"
        "QScrollBar:vertical {"
            "background-color: %1;"
            "width: 8px;"
            "border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
            "background-color: %2;"
            "border-radius: 4px;"
            "min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
            "background-color: %5;"
        "}"
        "QScrollBar:horizontal {"
            "background-color: %1;"
            "height: 8px;"
            "border-radius: 4px;"
        "}"
        "QScrollBar::handle:horizontal {"
            "background-color: %2;"
            "border-radius: 4px;"
            "min-width: 20px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
            "background-color: %5;"
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line {"
            "border: none;"
            "background: none;"
        "}"
    ).arg(bgColor)
     .arg(borderColor)
     .arg(textColor)
     .arg(headerBgColor)
     .arg(headerTextColor)
     .arg(selectionColor)
     .arg(alternateRowColor));
}

void TableEditor::updateSearchComponentsTheme(bool isDark)
{
    QString inputBg = isDark ? "#374151" : "#FFFFFF";
    QString inputBorder = isDark ? "#4B5563" : "#D1D5DB";
    QString inputText = isDark ? "#F9FAFB" : "#111827";
    QString placeholderText = isDark ? "#9CA3AF" : "#6B7280";
    
    if (searchBox) {
        searchBox->setStyleSheet(QString(
            "QLineEdit {"
                "background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 8px;"
                "padding: 8px 12px;"
                "font-size: 14px;"
                "color: %3;"
            "}"
            "QLineEdit::placeholder {"
                "color: %4;"
            "}"
        ).arg(inputBg, inputBorder, inputText, placeholderText));
    }
    
    if (filterBtn) {
        QString filterBtnStyle = QString(
            "QPushButton {"
                "background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 8px;"
                "padding: 8px 12px;"
                "color: %3;"
            "}"
            "QPushButton:hover {"
                "background-color: %4;"
            "}"
        ).arg(inputBg, inputBorder, inputText, isDark ? "#4B5563" : "#F3F4F6");
        filterBtn->setStyleSheet(filterBtnStyle);
    }
}

void TableEditor::updateTreeWidgetTheme(bool isDark)
{
    if (!tableList) return;
    
    QString bgColor = isDark ? "#1F2937" : "#FAFAFA";
    QString textColor = isDark ? "#F9FAFB" : "#111827";
    QString selectionBg = isDark ? "#374151" : "#EFF6FF";
    QString selectionText = isDark ? "#F9FAFB" : "#1D4ED8";
    
    tableList->setStyleSheet(QString(
        "QTreeWidget {"
            "background-color: %1;"
            "border: none;"
            "outline: none;"
            "color: %2;"
        "}"
        "QTreeWidget::item {"
            "height: 32px;"
            "padding: 4px 8px;"
            "border-radius: 6px;"
            "margin: 1px 4px;"
        "}"
        "QTreeWidget::item:hover {"
            "background-color: %3;"
        "}"
        "QTreeWidget::item:selected {"
            "background-color: %3;"
            "color: %4;"
        "}"
    ).arg(bgColor, textColor, selectionBg, selectionText));
}

void TableEditor::updateEmptyStateTheme(bool isDark)
{
    QString textColor = isDark ? "#9CA3AF" : "#6B7280";
    
    // Find and update empty state labels in the left panel
    QList<QLabel*> emptyLabels = leftPanel->findChildren<QLabel*>();
    for (QLabel* label : emptyLabels) {
        if (label->text().contains("Sin tablas") || label->text().contains("se listará aquí")) {
            label->setStyleSheet(QString("QLabel { color: %1; }").arg(textColor));
        }
    }
}

void TableEditor::onNewTableClicked()
{
    showCreateTablePanel();
}

void TableEditor::onAddColumnClicked()
{
    QScrollArea *scrollArea = createTablePanel->findChild<QScrollArea*>("columnsScrollArea");
    if (!scrollArea) return;
    
    QWidget *scrollContent = scrollArea->widget();
    QVBoxLayout *columnsLayout = qobject_cast<QVBoxLayout*>(scrollContent->layout());
    
    if (columnsLayout) {
        // Remove the stretch spacer temporarily
        QLayoutItem *lastItem = columnsLayout->itemAt(columnsLayout->count() - 1);
        if (lastItem && lastItem->spacerItem()) {
            columnsLayout->removeItem(lastItem);
            delete lastItem;
        }
        
        // Create new column row
        QWidget *newColumn = createColumnRow("nueva_columna", "string", false);
        columnsLayout->addWidget(newColumn);
        
        // Add stretch spacer back
        columnsLayout->addStretch();
        
        // Animate the appearance of the new column
        newColumn->setMaximumHeight(0);
        QPropertyAnimation *animation = new QPropertyAnimation(newColumn, "maximumHeight");
        animation->setDuration(300);
        animation->setStartValue(0);
        animation->setEndValue(150);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        
        // Focus on the name input of the new column
        QTimer::singleShot(300, [newColumn]() {
            QLineEdit *nameInput = newColumn->findChild<QLineEdit*>();
            if (nameInput) {
                nameInput->selectAll();
                nameInput->setFocus();
            }
        });
    }
}

void TableEditor::onCancelClicked()
{
    hideCreateTablePanel();
}

void TableEditor::onSaveClicked()
{
    QLineEdit *tableNameInput = createTablePanel->findChild<QLineEdit*>("tableNameInput");
    if (!tableNameInput || tableNameInput->text().trimmed().isEmpty()) {
        // Show error message
        QMessageBox::warning(this, "Error", "Por favor ingresa un nombre para la tabla.");
        return;
    }
    
    QString tableName = tableNameInput->text().trimmed();
    
    // Collect column information
    QScrollArea *scrollArea = createTablePanel->findChild<QScrollArea*>("columnsScrollArea");
    if (!scrollArea) return;
    
    QWidget *scrollContent = scrollArea->widget();
    QVBoxLayout *columnsLayout = qobject_cast<QVBoxLayout*>(scrollContent->layout());
    
    QStringList columns;
    bool hasPrimaryKey = false;
    
    if (columnsLayout) {
        for (int i = 0; i < columnsLayout->count(); i++) {
            QLayoutItem *item = columnsLayout->itemAt(i);
            if (!item || !item->widget()) continue;
            
            QWidget *columnRow = item->widget();
            QLineEdit *nameInput = columnRow->findChild<QLineEdit*>();
            QComboBox *typeCombo = columnRow->findChild<QComboBox*>();
            QLineEdit *defaultInput = columnRow->findChild<QLineEdit*>("defaultInput");
            QPushButton *primaryBtn = columnRow->findChild<QPushButton*>();
            
            if (nameInput && typeCombo) {
                QString columnName = nameInput->text().trimmed();
                if (columnName.isEmpty()) continue;
                
                QString fullType = typeCombo->currentText();
                QString dataType = fullType.split(" - ").first();
                QString defaultValue = defaultInput ? defaultInput->text().trimmed() : "";
                bool isPrimary = primaryBtn ? primaryBtn->isChecked() : false;
                
                if (isPrimary) hasPrimaryKey = true;
                
                QString columnDef = columnName + " " + dataType;
                if (isPrimary) columnDef += " PRIMARY KEY";
                if (!defaultValue.isEmpty()) columnDef += " DEFAULT " + defaultValue;
                
                columns.append(columnDef);
            }
        }
    }
    
    if (columns.isEmpty()) {
        QMessageBox::warning(this, "Error", "La tabla debe tener al menos una columna.");
        return;
    }
    
    if (!hasPrimaryKey) {
        QMessageBox::warning(this, "Error", "La tabla debe tener al menos una clave primaria.");
        return;
    }
    
    // Create SQL statement
    QString sql = QString("CREATE TABLE %1 (\n    %2\n);")
                      .arg(tableName)
                      .arg(columns.join(",\n    "));
    
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirmar creación de tabla");
    msgBox.setText(QString("¿Crear la tabla '%1'?").arg(tableName));
    msgBox.setDetailedText(sql);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    
    if (msgBox.exec() == QMessageBox::Yes) {
        // TODO: Execute SQL and create table in database
        qDebug() << "Creating table with SQL:" << sql;
        
        // Hide panel and show success message
        hideCreateTablePanel();
        
        QMessageBox::information(this, "Éxito", 
                                QString("Tabla '%1' creada exitosamente.").arg(tableName));
        
        // Clear form for next use
        tableNameInput->clear();
    }
}

QWidget* TableEditor::createColumnRow(const QString &name, const QString &type, bool isPrimary)
{
    QWidget *columnRow = new QWidget();
    columnRow->setStyleSheet(
        "QWidget {"
            "background-color: #F9FAFB;"
            "border: 1px solid #E5E7EB;"
            "border-radius: 8px;"
            "padding: 16px;"
        "}"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(columnRow);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);
    
    // Header row with drag handle and delete button
    QWidget *headerWidget = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    
    QLabel *dragHandle = new QLabel("⋮⋮");
    dragHandle->setFont(QFont("Inter", 16));
    dragHandle->setStyleSheet("QLabel { color: #9CA3AF; }");
    dragHandle->setFixedWidth(24);
    
    headerLayout->addWidget(dragHandle);
    headerLayout->addStretch();
    
    // Primary key indicator
    if (isPrimary) {
        QLabel *primaryLabel = new QLabel("1️⃣");
        primaryLabel->setFont(QFont("Inter", 16));
        primaryLabel->setToolTip("Clave primaria");
        headerLayout->addWidget(primaryLabel);
    }
    
    // Settings button
    QPushButton *settingsBtn = new QPushButton("⚙");
    settingsBtn->setFont(QFont("Inter", 14));
    settingsBtn->setFixedSize(24, 24);
    settingsBtn->setStyleSheet(
        "QPushButton {"
            "background-color: transparent;"
            "border: none;"
            "color: #9CA3AF;"
        "}"
        "QPushButton:hover {"
            "color: #6B7280;"
        "}"
    );
    headerLayout->addWidget(settingsBtn);
    
    // Delete button (only for non-primary columns)
    if (!isPrimary) {
        QPushButton *deleteBtn = new QPushButton("✕");
        deleteBtn->setFont(QFont("Inter", 14));
        deleteBtn->setFixedSize(24, 24);
        deleteBtn->setStyleSheet(
            "QPushButton {"
                "background-color: transparent;"
                "border: none;"
                "color: #9CA3AF;"
            "}"
            "QPushButton:hover {"
                "color: #EF4444;"
            "}"
        );
        connect(deleteBtn, &QPushButton::clicked, [columnRow, this]() {
            columnRow->deleteLater();
        });
        headerLayout->addWidget(deleteBtn);
    }
    
    mainLayout->addWidget(headerWidget);
    
    // Column configuration row
    QWidget *configWidget = new QWidget();
    QHBoxLayout *configLayout = new QHBoxLayout(configWidget);
    configLayout->setContentsMargins(0, 0, 0, 0);
    configLayout->setSpacing(12);
    
    // Name input
    QLineEdit *nameInput = new QLineEdit();
    nameInput->setText(name);
    nameInput->setPlaceholderText("Nombre");
    nameInput->setFont(QFont("Inter", 14));
    nameInput->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
    );
    
    // Type selector
    QComboBox *typeCombo = new QComboBox();
    typeCombo->addItems({
        "int - Claves primarias, edades, IDs",
        "float - Promedios, notas, precios", 
        "bool - Estados activos/inactivos",
        "char[N] - Nombres, descripciones cortas",
        "string - Direcciones, observaciones",
        "timestamp - Fechas y horas"
    });
    
    // Set current type
    QString typeText = type + " - ";
    for (int i = 0; i < typeCombo->count(); i++) {
        if (typeCombo->itemText(i).startsWith(typeText)) {
            typeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    typeCombo->setFont(QFont("Inter", 14));
    typeCombo->setStyleSheet(
        "QComboBox {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            "color: #111827;"
        "}"
        "QComboBox::drop-down {"
            "border: none;"
            "width: 20px;"
        "}"
        "QComboBox::down-arrow {"
            "image: none;"
            "border-left: 4px solid transparent;"
            "border-right: 4px solid transparent;"
            "border-top: 4px solid #6B7280;"
            "margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "color: #111827;"
            "selection-background-color: #EFF6FF;"
        "}"
    );
    
    // Default value input
    QLineEdit *defaultInput = new QLineEdit();
    if (name == "created_at") {
        defaultInput->setText("now()");
    }
    defaultInput->setPlaceholderText("Valor por defecto");
    defaultInput->setFont(QFont("Inter", 14));
    defaultInput->setStyleSheet(
        "QLineEdit {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            "color: #111827;"
        "}"
        "QLineEdit::placeholder {"
            "color: #9CA3AF;"
        "}"
    );
    
    // Primary checkbox (checked for ID)
    QPushButton *primaryBtn = new QPushButton();
    primaryBtn->setFixedSize(24, 24);
    primaryBtn->setCheckable(true);
    primaryBtn->setChecked(isPrimary);
    
    QString bgColor = isPrimary ? "#10B981" : "#FFFFFF";
    QString borderColor = isPrimary ? "#10B981" : "#D1D5DB";
    
    primaryBtn->setStyleSheet(
        "QPushButton {"
            "background-color: " + bgColor + ";"
            "border: 2px solid " + borderColor + ";"
            "border-radius: 4px;"
        "}"
        "QPushButton:checked {"
            "background-color: #10B981;"
            "border-color: #10B981;"
        "}"
        "QPushButton:checked::after {"
            "content: '✓';"
            "color: white;"
            "font-weight: bold;"
        "}"
    );
    
    configLayout->addWidget(nameInput, 2);
    configLayout->addWidget(typeCombo, 3);
    configLayout->addWidget(defaultInput, 2);
    configLayout->addWidget(primaryBtn, 0);
    
    mainLayout->addWidget(configWidget);
    
    return columnRow;
}

void TableEditor::showCreateTablePanel()
{
    if (!createTablePanel) return;
    
    createTablePanel->setVisible(true);
    createTablePanel->raise();
    
    // Animation to slide in from right
    QPropertyAnimation *animation = new QPropertyAnimation(createTablePanel, "geometry");
    animation->setDuration(400);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    QRect startRect = createTablePanel->geometry();
    startRect.moveLeft(this->width());
    createTablePanel->setGeometry(startRect);
    
    QRect endRect(this->width() - 450, 0, 450, this->height());
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    
    connect(animation, &QPropertyAnimation::finished, [this]() {
        // Focus on table name input after animation
        QLineEdit *tableNameInput = createTablePanel->findChild<QLineEdit*>("tableNameInput");
        if (tableNameInput) {
            tableNameInput->setFocus();
        }
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TableEditor::hideCreateTablePanel()
{
    if (!createTablePanel) return;
    
    // Animation to slide out to right
    QPropertyAnimation *animation = new QPropertyAnimation(createTablePanel, "geometry");
    animation->setDuration(400);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    
    QRect startRect = createTablePanel->geometry();
    QRect endRect = startRect;
    endRect.moveLeft(this->width());
    
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    
    connect(animation, &QPropertyAnimation::finished, [this]() {
        createTablePanel->setVisible(false);
    });
    
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TableEditor::onDeleteColumnClicked()
{
    // This function is called from the delete button in createColumnRow
    // The actual deletion is handled in the lambda in createColumnRow
    qDebug() << "Column delete requested";
}
