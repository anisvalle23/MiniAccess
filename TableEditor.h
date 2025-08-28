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

class TableEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditor(QWidget *parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void onNewTableClicked();
    void onSchemaChanged(const QString &schema);

private:
    void setupUI();
    void createLeftPanel();
    void createRightPanel();
    void createToolbar();
    void createTableList();
    void createMainTableArea();
    void styleComponents();
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
    QWidget *schemaSection;
    QLabel *schemaLabel;
    QComboBox *schemaComboBox;
    QPushButton *newTableBtn;
    QWidget *tableListSection;
    QLabel *tableListLabel;
    QTreeWidget *tableTree;
    QTreeWidget *tableList;
    QLineEdit *searchBox;
    QPushButton *filterBtn;
    QTableWidget *tableView;
    
    // Right Panel  
    QWidget *rightPanel;
    QVBoxLayout *rightPanelLayout;
    
    // Toolbar
    QWidget *toolbar;
    QHBoxLayout *toolbarLayout;
    QLabel *toolbarTitle;
    QPushButton *createTableBtn;
    
    // Main content area
    QWidget *mainContentArea;
    QVBoxLayout *mainContentLayout;
    QLabel *emptyStateTitle;
    QLabel *emptyStateDesc;
    
    // Theme variables
    bool isDarkTheme;
};

#endif // TABLEEDITOR_H
