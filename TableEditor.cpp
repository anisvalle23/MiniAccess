#include "TableEditor.h"
#include <QDebug>

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
    
    // Schema section
    schemaSection = new QWidget();
    QVBoxLayout *schemaSectionLayout = new QVBoxLayout(schemaSection);
    schemaSectionLayout->setContentsMargins(0, 0, 0, 0);
    schemaSectionLayout->setSpacing(8);
    
    schemaLabel = new QLabel("schema");
    schemaLabel->setFont(QFont("Inter", 12, QFont::Medium));
    schemaLabel->setStyleSheet("QLabel { color: #6B7280; }");
    
    schemaComboBox = new QComboBox();
    schemaComboBox->addItem("public");
    schemaComboBox->setFont(QFont("Inter", 14));
    schemaComboBox->setStyleSheet(
        "QComboBox {"
            "background-color: #FFFFFF;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 8px;"
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
    );
    
    schemaSectionLayout->addWidget(schemaLabel);
    schemaSectionLayout->addWidget(schemaComboBox);
    leftPanelLayout->addWidget(schemaSection);
    
    // New table button
    newTableBtn = new QPushButton("New table");
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
    connect(schemaComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &TableEditor::onSchemaChanged);
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
    
    QLineEdit *searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search tables...");
    searchBox->setFont(QFont("Inter", 13));
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
    
    QPushButton *filterBtn = new QPushButton("⚙");
    filterBtn->setFixedSize(32, 32);
    filterBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #F9FAFB;"
            "border: 1px solid #D1D5DB;"
            "border-radius: 6px;"
            "color: #6B7280;"
            "font-size: 14px;"
        "}"
        "QPushButton:hover {"
            "background-color: #F3F4F6;"
        "}"
    );
    
    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(filterBtn);
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
    QLabel *noTablesLabel = new QLabel("No tables or views");
    noTablesLabel->setFont(QFont("Inter", 13, QFont::Medium));
    noTablesLabel->setStyleSheet("QLabel { color: #6B7280; }");
    noTablesLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *noTablesDesc = new QLabel("Any tables or views you create\nwill be listed here.");
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
    toolbarTitle = new QLabel("Table Editor");
    toolbarTitle->setFont(QFont("Inter", 20, QFont::Bold));
    toolbarTitle->setStyleSheet("QLabel { color: #111827; }");
    
    toolbarLayout->addWidget(toolbarTitle);
    toolbarLayout->addStretch();
    
    // Create table button
    createTableBtn = new QPushButton("Create a table");
    createTableBtn->setFont(QFont("Inter", 14, QFont::Medium));
    createTableBtn->setStyleSheet(
        "QPushButton {"
            "background-color: #A4373A;"
            "color: #FFFFFF;"
            "border: none;"
            "border-radius: 8px;"
            "padding: 10px 20px;"
            "font-weight: 500;"
        "}"
        "QPushButton:hover {"
            "background-color: #8B2635;"
        "}"
        "QPushButton:pressed {"
            "background-color: #6D1D29;"
        "}"
    );
    
    toolbarLayout->addWidget(createTableBtn);
    rightPanelLayout->addWidget(toolbar);
    
    connect(createTableBtn, &QPushButton::clicked, this, &TableEditor::onNewTableClicked);
}

void TableEditor::createMainTableArea()
{
    mainContentArea = new QWidget();
    mainContentArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    
    mainContentLayout = new QVBoxLayout(mainContentArea);
    mainContentLayout->setContentsMargins(40, 40, 40, 40);
    mainContentLayout->setSpacing(16);
    
    // Empty state content (like in the image)
    QWidget *emptyStateWidget = new QWidget();
    QVBoxLayout *emptyStateLayout = new QVBoxLayout(emptyStateWidget);
    emptyStateLayout->setAlignment(Qt::AlignCenter);
    emptyStateLayout->setSpacing(16);
    
    // Icon
    QLabel *iconLabel = new QLabel("📊");
    iconLabel->setFont(QFont("Inter", 48));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("QLabel { color: #A4373A; }");
    
    // Title
    emptyStateTitle = new QLabel("Create a table");
    emptyStateTitle->setFont(QFont("Inter", 24, QFont::Bold));
    emptyStateTitle->setStyleSheet("QLabel { color: #111827; }");
    emptyStateTitle->setAlignment(Qt::AlignCenter);
    
    // Description
    emptyStateDesc = new QLabel("Design and create a new database table");
    emptyStateDesc->setFont(QFont("Inter", 16));
    emptyStateDesc->setStyleSheet("QLabel { color: #6B7280; }");
    emptyStateDesc->setAlignment(Qt::AlignCenter);
    
    // Recent items section
    QLabel *recentTitle = new QLabel("Recent items");
    recentTitle->setFont(QFont("Inter", 18, QFont::Medium));
    recentTitle->setStyleSheet("QLabel { color: #111827; margin-top: 32px; }");
    
    QLabel *recentEmpty = new QLabel("No recent items yet");
    recentEmpty->setFont(QFont("Inter", 14));
    recentEmpty->setStyleSheet("QLabel { color: #9CA3AF; }");
    recentEmpty->setAlignment(Qt::AlignCenter);
    
    QLabel *recentDesc = new QLabel("Items will appear here as you browse through your project");
    recentDesc->setFont(QFont("Inter", 13));
    recentDesc->setStyleSheet("QLabel { color: #9CA3AF; }");
    recentDesc->setAlignment(Qt::AlignCenter);
    
    emptyStateLayout->addWidget(iconLabel);
    emptyStateLayout->addWidget(emptyStateTitle);
    emptyStateLayout->addWidget(emptyStateDesc);
    emptyStateLayout->addStretch();
    emptyStateLayout->addWidget(recentTitle);
    emptyStateLayout->addWidget(recentEmpty);
    emptyStateLayout->addWidget(recentDesc);
    emptyStateLayout->addStretch();
    
    mainContentLayout->addWidget(emptyStateWidget);
    rightPanelLayout->addWidget(mainContentArea);
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
    
    // Update schema label color
    QString labelColor = isDark ? "#9CA3AF" : "#6B7280";
    schemaLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(labelColor));
    
    // Update combo box theme
    QString comboBg = isDark ? "#374151" : "#FFFFFF";
    QString comboBorder = isDark ? "#4B5563" : "#D1D5DB";
    QString comboText = isDark ? "#F9FAFB" : "#111827";
    
    schemaComboBox->setStyleSheet(QString(
        "QComboBox {"
            "background-color: %1;"
            "border: 1px solid %2;"
            "border-radius: 8px;"
            "padding: 8px 12px;"
            "font-size: 14px;"
            "color: %3;"
        "}"
        "QComboBox::drop-down {"
            "border: none;"
            "width: 20px;"
        "}"
        "QComboBox::down-arrow {"
            "image: none;"
            "border-left: 4px solid transparent;"
            "border-right: 4px solid transparent;"
            "border-top: 4px solid %4;"
            "margin-right: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
            "background-color: %1;"
            "border: 1px solid %2;"
            "color: %3;"
            "selection-background-color: #A4373A;"
        "}"
    ).arg(comboBg, comboBorder, comboText, labelColor));
    
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
    
    emptyStateTitle->setStyleSheet(QString("QLabel { color: %1; }").arg(titleColor));
    emptyStateDesc->setStyleSheet(QString("QLabel { color: %1; }").arg(descColor));
    
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
        if (label->text().contains("No tables") || label->text().contains("will be listed")) {
            label->setStyleSheet(QString("QLabel { color: %1; }").arg(textColor));
        }
    }
}

void TableEditor::onNewTableClicked()
{
    // TODO: Handle new table creation
    qDebug() << "New table button clicked";
}

void TableEditor::onSchemaChanged(const QString &schema)
{
    // TODO: Handle schema change
    qDebug() << "Schema changed to:" << schema;
}
