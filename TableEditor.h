#ifndef TABLEEDITOR_H
#define TABLEEDITOR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QHeaderView>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMap>
#include "TableView.h"
#include "TableData.h"

// Structure to store table design data
struct TableDesignData {
    QStringList fieldNames;
    QStringList fieldTypes;
    QStringList fieldDescriptions;
};

// Clickable widget class
class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickableWidget(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
        }
        QWidget::mousePressEvent(event);
    }

signals:
    void clicked();
};

class TableEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditor(QWidget *parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void onCreateTableClicked();
    void onNewTableClicked();
    void onTableSelected();
    void animateCreateTablePanel();
    void onCancelClicked();
    void onSaveClicked();
    void onDeleteColumnClicked();

private:
    void setupUI();
    void createLeftPanel();
    void createRightPanel();
    void createTableCreationPanel();
    void styleComponents();
    void addTableToSidebar(const QString &tableName);
    void updateTableList();
    void createMainTableArea();
    void showWelcomeContent();
    void showCreateTablePanel();
    void hideCreateTablePanel();
    void showTableView(const QString &tableName);
    void showTableDataView(const QString &tableName);
    void switchToDataView();
    void switchToDesignView();
    void syncTableDesignData();
    void applyTableDesignData();
    void updateLeftPanelTheme(bool isDark);
    void updateRightPanelTheme(bool isDark);
    void updateToolbarTheme(bool isDark);
    void updateTableTheme(bool isDark);
    void updateSearchComponentsTheme(bool isDark);
    void updateTreeWidgetTheme(bool isDark);
    void updateEmptyStateTheme(bool isDark);
    
    // UI Components
    QHBoxLayout *mainLayout;
    QSplitter *mainSplitter;
    
    // Left Panel
    QWidget *leftPanel;
    QVBoxLayout *leftPanelLayout;
    QPushButton *newTableBtn;
    QWidget *tableListSection;
    QLabel *tableListLabel;
    QTreeWidget *tableTree;
    QTreeWidget *tableList;
    QLineEdit *searchBox;
    QPushButton *filterBtn;
    
    // Right Panel  
    QWidget *rightPanel;
    QVBoxLayout *rightPanelLayout;
    
    // Toolbar
    QWidget *toolbar;
    QHBoxLayout *toolbarLayout;
    QLabel *toolbarTitle;
    
    // Main content area
    QWidget *mainContentArea;
    QVBoxLayout *mainContentLayout;
    ClickableWidget *createTableCard;
    
    // Create table panel
    QWidget *createTablePanel;
    QVBoxLayout *createTablePanelLayout;
    QLineEdit *tableNameInput;
    QPushButton *cancelBtn;
    QPushButton *saveBtn;
    QPropertyAnimation *panelAnimation;
    
    // Theme variables
    bool isDarkTheme;
    
    // Table instances and data
    TableView *currentTableView;
    TableData *currentTableData;
    QMap<QString, TableDesignData> tableDesigns;
    QString currentTableName;
};

#endif // TABLEEDITOR_H
