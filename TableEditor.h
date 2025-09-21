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

// Forward declarations
class RelationshipsView;
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QRegularExpression>
#include <QToolButton>
#include <QTimer>
#include "TableView.h"
#include "TableData.h"
#include "mainwindow.h"

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

// Custom table item widget with menu button
class TableItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TableItemWidget(const QString &tableName, QWidget *parent = nullptr);
    QString getTableName() const { return tableName; }

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void tableClicked(const QString &tableName);
    void optionsClicked(const QString &tableName, const QPoint &pos);
    void clicked(const QString& tableName);

private:
    QString tableName;
    QPushButton *menuButton;
};

class TableEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditor(QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void setMainWindow(MainWindow* w) { 
        m_mainWindow = w; 
        if (w) {
            this->setProperty("tablesDir", QString::fromStdString(w->tablesDir()));
        }
    }
    MainWindow* mainWindow() const { return m_mainWindow; }
    void updateTableList();
    // Configurar referencia a RelationshipsView
    void setRelationshipsView(RelationshipsView *relationshipsView);
    
    // Methods to get created tables info
    QStringList getCreatedTables() const;
    QStringList getTableFields(const QString &tableName) const;
    QStringList getTableFieldsWithKeys(const QString &tableName) const; // Nuevo método que incluye las llaves
    QStringList readFieldsFromMetaWithKeys(const QString& tablesDir, const QString& tableName) const; // Leer llaves desde archivo
    QStringList getTableForeignKeys(const QString &tableName) const; // Método para obtener solo los Foreign Keys
    QStringList getTablePrimaryKeys(const QString &tableName) const; // Método para obtener solo los Primary Keys
    QStringList getTablePrimaryAndForeignKeys(const QString &tableName) const; // Método para obtener campos que son PK y FK
    QString getFieldType(const QString &tableName, const QString &fieldName) const; // Obtener tipo de dato de un campo específico
    QStringList getTableFieldTypes(const QString &tableName) const; // Obtener todos los tipos de datos de una tabla

    void showStyledMessageBox(const QString &title, const QString &message, QMessageBox::Icon icon = QMessageBox::Warning);
    
    // Método para obtener datos de una tabla específica para validación FK
    QStringList getTableColumnData(const QString &tableName, const QString &fieldName) const;
    void saveDesignForTable(const QString& tableName, bool migrate = true);
    QTimer *designDebounceTimer = nullptr;
    QString pendingTableForSave;

signals:
    void tableCreated(const QString &tableName);
    void tableDeleted(const QString &tableName);
    void tableFieldsChanged(const QString &tableName); // Nueva señal para cambios en campos
    void tableRenamed(const QString &oldName, const QString &newName); // Nueva señal para renombrado
    void foreignKeyRemoved(const QString &tableName, const QString &fieldName); // Nueva señal para FK eliminada
    void foreignKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName); // Nueva señal para FK renombrada
    void primaryKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName); // Nueva señal para PK renombrada

private slots:
    void onCreateTableClicked();
    void onNewTableClicked();
    void onTableSelected();
    void animateCreateTablePanel();
    void onCancelClicked();
    void onSaveClicked();
    void onDeleteColumnClicked();
    void onSidebarItemClicked(QTreeWidgetItem *item, int column);
    void onDeleteTableClicked();
    void showTableContextMenu(const QPoint &pos);
    void showTableOptionsMenu(const QString &tableName, const QPoint &pos);
    void renameTable(const QString &oldName, const QString &newName);
    void performTableSearch();

    void onDesignDebounceTimeout();

private:
    void setupUI();
    void createLeftPanel();
    void createRightPanel();
    void createTableCreationPanel();
    void styleComponents();
    void addTableToSidebar(const QString &tableName);
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
    void deleteTable(const QString &tableName);
    bool isValidTableName(const QString &name);
    void scheduleDesignAutosave(const QString& tableName);
    
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
    QLineEdit   *searchBox = nullptr;
    QToolButton *searchBtn = nullptr;
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
    
    // Relationships view reference
    RelationshipsView *relationshipsView;
    
    // Table instances and data
    TableView *currentTableView;
    TableData *currentTableData;
    QMap<QString, TableDesignData> tableDesigns;
    QMap<QString, TableView*> tableViews;
    QMap<QString, TableData*> tableDatas;
    QString currentTableName;

    MainWindow* m_mainWindow = nullptr;
    bool autoSavingSchema = false;   // evita bucles/duplicados
};

#endif // TABLEEDITOR_H
