#include "TableEditor.h"
#include "TableView.h"
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
    mainSplitter->setHandleWidth(0); // Sin separador visible
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
            "background-color: transparent;"
            "width: 0px;"
        "}"
    );
    
    createLeftPanel();
    createRightPanel();
    createTableCreationPanel();
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    
    // Set initial sizes (left panel smaller)
    mainSplitter->setSizes({220, 1080}); // Panel izquierdo más pequeño, más espacio para el contenido principal
    mainSplitter->setCollapsible(0, false);
    mainSplitter->setCollapsible(1, false);
    
    mainLayout->addWidget(mainSplitter);
}

void TableEditor::createLeftPanel()
{
    leftPanel = new QWidget();
    leftPanel->setFixedWidth(220);
    leftPanel->setStyleSheet("QWidget { background-color: #FAFAFA; }");

    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(12, 16, 12, 16);
    leftPanelLayout->setSpacing(12);

    // Botón nueva tabla
    newTableBtn = new QPushButton("Nueva Tabla");
    leftPanelLayout->addWidget(newTableBtn);

    // Table list section
    updateTableList();

    // <<< AQUI es donde lo pones >>>
    connect(tableTree, &QTreeWidget::itemClicked,
            this, &TableEditor::onSidebarItemClicked,
            Qt::UniqueConnection);

    leftPanelLayout->addStretch();

    connect(newTableBtn, &QPushButton::clicked, this, &TableEditor::onNewTableClicked);
}


void TableEditor::updateTableList()
{
    if (tableListSection) {
        leftPanelLayout->removeWidget(tableListSection);
        tableListSection->deleteLater();
        tableListSection = nullptr;
    }

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
    
    tableListLayout->addWidget(tableTree);
    
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
    
    // Solo crear el área principal, sin toolbar duplicado
    createMainTableArea();
}

void TableEditor::createMainTableArea()
{
    mainContentArea = new QWidget();
    mainContentArea->setStyleSheet("QWidget { background-color: #FFFFFF; }");
    
    mainContentLayout = new QVBoxLayout(mainContentArea);
    mainContentLayout->setContentsMargins(0, 0, 0, 0); // Sin márgenes para aprovechar todo el espacio
    mainContentLayout->setSpacing(0); // Sin espaciado
    
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
    QLabel *iconLabel = new QLabel("🗄️");
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
    
    // Center the card horizontally and vertically
    QVBoxLayout *centerVerticalLayout = new QVBoxLayout();
    centerVerticalLayout->addStretch();
    
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(createTableCard);
    centerLayout->addStretch();
    
    centerVerticalLayout->addLayout(centerLayout);
    centerVerticalLayout->addStretch();
    
    mainContentLayout->addLayout(centerVerticalLayout);
    
    // Connect card click event
    connect(createTableCard, &ClickableWidget::clicked, this, &TableEditor::onNewTableClicked);
    
    rightPanelLayout->addWidget(mainContentArea);
}

void TableEditor::createTableCreationPanel()
{
    createTablePanel = new QWidget(this);
    createTablePanel->setFixedWidth(240); // Mucho más pequeño
    createTablePanel->setStyleSheet(
        "QWidget {"
            "background-color: #FFFFFF;"
            "border-left: 1px solid #E5E7EB;"
        "}"
    );
    
    createTablePanelLayout = new QVBoxLayout(createTablePanel);
    createTablePanelLayout->setContentsMargins(20, 20, 20, 20); // Márgenes más pequeños
    createTablePanelLayout->setSpacing(20); // Espaciado más compacto
    
    // Header section - Solo título
    QLabel *titleLabel = new QLabel("Nueva Tabla");
    titleLabel->setFont(QFont("Inter", 20, QFont::Bold));
    titleLabel->setStyleSheet("QLabel { color: #111827; }");
    createTablePanelLayout->addWidget(titleLabel);
    
    // Table name input
    QWidget *nameWidget = new QWidget();
    QVBoxLayout *nameLayout = new QVBoxLayout(nameWidget);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(8);
    
    QLabel *nameLabel = new QLabel("Nombre de la tabla");
    nameLabel->setFont(QFont("Inter", 14, QFont::Medium));
    nameLabel->setStyleSheet("QLabel { color: #374151; }");
    
    tableNameInput = new QLineEdit();
    tableNameInput->setObjectName("tableNameInput");
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
    
    // Añadir espacio flexible para empujar los botones hacia abajo
    createTablePanelLayout->addStretch();
    
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
    createTablePanel->setGeometry(width(), 0, 280, height());
    createTablePanel->hide();
    
    // Connect signals
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
    
    leftPanel->setStyleSheet(QString(
        "QWidget {"
            "background-color: %1;"
        "}"
    ).arg(bgColor));
    
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
    // Toolbar eliminado - ya no se necesita actualizar tema
    Q_UNUSED(isDark)
}

void TableEditor::updateTableTheme(bool isDark)
{
    // No longer need to update table theme here since TableView is a separate window
    // Each TableView window will handle its own theme
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

void TableEditor::onCancelClicked()
{
    hideCreateTablePanel();
}

void TableEditor::onSaveClicked()
{
    if (!tableNameInput || tableNameInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor ingresa un nombre para la tabla.");
        return;
    }

    QString tableName = tableNameInput->text().trimmed();

    hideCreateTablePanel();

    // Si no existe ya en el sidebar, agregarlo
    QList<QTreeWidgetItem*> found = tableTree->findItems(tableName, Qt::MatchExactly);
    if (found.isEmpty())
        addTableToSidebar(tableName);

    showTableView(tableName);
    tableNameInput->clear();
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
    
    QRect endRect(this->width() - 300, 0, 280, this->height()); // Posición más a la izquierda y más pequeño
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    
    connect(animation, &QPropertyAnimation::finished, [this]() {
        // Focus on table name input after animation
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

void TableEditor::showTableView(const QString &tableName)
{
    currentTableName = tableName;

    // 1) Crear o recuperar el TableView de cache
    TableView *view = nullptr;
    if (tableViews.contains(tableName)) {
        view = tableViews.value(tableName);
    } else {
        view = new TableView(this);
        view->setTableName(tableName);
        view->updateTheme(isDarkTheme);
        view->setProperty("tableName", tableName);

        // Conexiones SOLO al crearlo (UniqueConnection por seguridad)
        connect(view, &TableView::switchToDataView, this, [this]() {
            switchToDataView();
        }, Qt::UniqueConnection);

        connect(view, &TableView::tableDesignChanged, this,
                [this, tableName](const QStringList &fieldNames, const QStringList &fieldTypes) {
                    // Guardar diseño en "arreglos" por tabla
                    TableDesignData &d = tableDesigns[tableName];
                    d.fieldNames = fieldNames;
                    d.fieldTypes = fieldTypes;
                    // Si existe su TableData, sincronizar
                    if (tableDatas.contains(tableName) && tableDatas.value(tableName)) {
                        tableDatas.value(tableName)->setupDataView(fieldNames, fieldTypes);
                    }
                }, Qt::UniqueConnection);

        tableViews.insert(tableName, view);
    }

    // 2) Crear o recuperar el TableData de cache (aunque no se muestre todavía)
    TableData *data = nullptr;
    if (tableDatas.contains(tableName)) {
        data = tableDatas.value(tableName);
    } else {
        data = new TableData(this);
        data->setTableName(tableName);
        data->setProperty("tableName", tableName);

        connect(data, &TableData::switchToDesignView, this, [this]() {
            switchToDesignView();
        }, Qt::UniqueConnection);

        // Si ya hay diseño guardado, aplicarlo
        if (tableDesigns.contains(tableName)) {
            const auto &d = tableDesigns.value(tableName);
            data->setupDataView(d.fieldNames, d.fieldTypes);
        }
        tableDatas.insert(tableName, data);
    }

    // 3) Mostrar SOLO el view (diseño) ahora
    //    Limpia layout principal y añade el que corresponde (sin borrar caches)
    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(view);
    view->show();
}


void TableEditor::addTableToSidebar(const QString &tableName)
{
    // Create new tree widget item for the table
    auto *tableItem = new QTreeWidgetItem(tableTree);
    tableItem->setText(0, tableName);
    tableItem->setIcon(0, QIcon("🗄️")); // You can use a proper icon here
    
    // Connect item click to show the table
    connect(tableTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        if (item) {
            QString selectedTableName = item->text(0);
            showTableView(selectedTableName);
        }
    });
}

void TableEditor::showTableDataView(const QString &tableName)
{
    currentTableName = tableName;

    // Asegura que existen en cache
    if (!tableViews.contains(tableName)) {
        tableViews.insert(tableName, new TableView(this));
        tableViews[tableName]->setTableName(tableName);
        tableViews[tableName]->updateTheme(isDarkTheme);
        connect(tableViews[tableName], &TableView::switchToDataView, this, [this]() {
            switchToDataView();
        }, Qt::UniqueConnection);
    }
    if (!tableDatas.contains(tableName)) {
        tableDatas.insert(tableName, new TableData(this));
        tableDatas[tableName]->setTableName(tableName);
        connect(tableDatas[tableName], &TableData::switchToDesignView, this, [this]() {
            switchToDesignView();
        }, Qt::UniqueConnection);

        if (tableDesigns.contains(tableName)) {
            const auto &d = tableDesigns.value(tableName);
            tableDatas[tableName]->setupDataView(d.fieldNames, d.fieldTypes);
        }
    }

    // Mostrar solo Data
    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(tableDatas.value(tableName));
    tableDatas.value(tableName)->show();
}

void TableEditor::onDeleteColumnClicked()
{
    // This function is called from the delete button in createColumnRow
    // The actual deletion is handled in the lambda in createColumnRow
    qDebug() << "Column delete requested";
}

void TableEditor::onTableSelected()
{
    qDebug() << "Table selected";
    // Implementación básica para evitar error de linking
}

void TableEditor::onCreateTableClicked()
{
    qDebug() << "Create table clicked";
    // Implementación básica para evitar error de linking
}

void TableEditor::animateCreateTablePanel()
{
    qDebug() << "Animate create table panel";
    // Implementación básica para evitar error de linking
}

void TableEditor::switchToDataView()
{
    if (!tableDatas.contains(currentTableName)) return;
    TableData *data = tableDatas.value(currentTableName);
    if (!data) return;

    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(data);
    data->show();
}

void TableEditor::switchToDesignView()
{
    if (!tableViews.contains(currentTableName)) return;
    TableView *view = tableViews.value(currentTableName);
    if (!view) return;

    while (QLayoutItem *child = mainContentLayout->takeAt(0)) {
        if (auto *w = child->widget()) {
            w->hide();
            mainContentLayout->removeWidget(w);
        }
        delete child;
    }
    mainContentLayout->addWidget(view);
    view->show();
}

void TableEditor::onSidebarItemClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (!item) return;
    const QString selectedTableName = item->text(0);
    showTableView(selectedTableName);   // o showTableDataView si quieres abrir en datos
}
