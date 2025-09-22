#ifndef RELATIONSHIPSVIEW_H
#define RELATIONSHIPSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// Forward declarations
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QShowEvent>

// Estructura para almacenar información completa de una relación
struct RelationshipInfo {
    QString sourceTable;
    QString sourceField;
    QString targetTable;
    QString targetField;
    QString type;
    QString description;
    
    RelationshipInfo() = default;
    RelationshipInfo(const QString &st, const QString &sf, const QString &tt, const QString &tf, const QString &t)
        : sourceTable(st), sourceField(sf), targetTable(tt), targetField(tf), type(t) {}
};

class TableGraphicsItem;
class RelationshipLine;
class TableEditor;

// Custom QGraphicsView for drag and drop
class RelationshipDesignerView : public QGraphicsView
{
    Q_OBJECT

public:
    RelationshipDesignerView(QGraphicsScene *scene, class RelationshipsView *parent);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    class RelationshipsView *relationshipsView;
};

class RelationshipsView : public QWidget
{
    Q_OBJECT

public:
    explicit RelationshipsView(QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void refreshTableList();
    void addTableToDesigner(const QString &tableName, const QPointF &position);
    void setTableEditor(TableEditor *tableEditor);
    void onTableRenamed(const QString &oldName, const QString &newName); // Nuevo método para renombrado
    bool hasRelationshipForField(const QString &tableName, const QString &fieldName); // Verificar si existe relación para un campo
    QString getReferencedTableForField(const QString &tableName, const QString &fieldName); // Obtener tabla referenciada por un campo FK
    QString getReferencedFieldForField(const QString &tableName, const QString &fieldName); // Obtener campo referenciado por un campo FK
    void refreshAvailableTablesFromStorage();
    void forceRefreshTables(); // Método para forzar refresh desde exterior
    const QList<RelationshipInfo>& getRelationships() const { return relationships; }

signals:
    void relationshipCreated(const RelationshipInfo& info);
    void relationshipsChanged(const QList<RelationshipInfo>& all);

public slots:
    void onTableFieldsChanged(const QString &tableName); // Slot público para actualización en tiempo real
    void onForeignKeyRemoved(const QString &tableName, const QString &fieldName); // Slot para FK eliminada
    void onForeignKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName); // Slot para FK renombrada
    void onPrimaryKeyRenamed(const QString &tableName, const QString &oldFieldName, const QString &newFieldName); // Slot para PK renombrada
    void applyTableRenameImmediate(const QString& oldName, const QString& newName);
    void onTableDeleted(const QString &tableName); // Slot para manejar tabla eliminada

protected:
    void showEvent(QShowEvent *event) override; // Para refrescar cuando la vista se vuelve visible

private slots:
    void onNewRelationshipClicked();
    void onCreateRelationship();
    void onDeleteRelationship();
    void onTableSelectionChanged();
    void onRelationshipSelectionChanged();
    void onRelationshipDoubleClicked(QListWidgetItem *item);
    void onRelationshipLineDoubleClicked(RelationshipLine* line);
    void showTableDetails(const QString &tableName);
    void addTableToDesigner(const QString &tableName, const QPoint &position);
    void onTableCloseRequested(TableGraphicsItem* table); // New slot for handling table close
    void onInfoButtonClicked(); // New slot for info button

public:
    QString getCleanFieldName(const QString &fieldName); // Método para limpiar nombres de campos de iconos

private:
    RelationshipInfo* findRelationshipForField(const QString &tableName, const QString &fieldName); // Buscar relación específica
    
    // Private methods used in implementation
    void setupUI();
    void styleComponents();
    void loadRelationships();
    void loadTables();
    void loadDesignerState();
    void saveDesignerState();
    void showAllTablesInDesigner();
    void showAllTablesAndRelationships();
    void createRelationshipBetweenTables(const QString &sourceTable, const QString &targetTable, const QString &type);
    QString getProjectRelationshipsPath();
    void createToolbar();
    void createMainArea();
    void createRelationshipsList();
    void createRelationshipDesigner();
    void createPropertiesPanel();
    void updateSourceFields(const QString &tableName);
    void updateTargetFields(const QString &tableName);
    void clearDesignerArea();
    void updatePropertiesPanel(const QString &relationshipName);
    bool validateForeignKeyNaming(const QString &fieldName, const QString &tableName, bool exact);
    bool validateDataTypeCompatibility(const QString &sourceTable, const QString &targetTable, const QString &relationshipType);
    
    // UI Components
    QVBoxLayout *mainLayout;
    
    // Toolbar
    QWidget *toolbarWidget;
    QHBoxLayout *toolbarLayout;
    QPushButton *createRelationshipBtn;
    QPushButton *deleteRelationshipBtn;
    QPushButton *showAllTablesBtn;
    QLabel *titleLabel;
    
    // Main content area
    QSplitter *mainSplitter;
    QSplitter *leftSplitter;
    
    // Left panel - Tables and relationships list
    QWidget *leftPanel;
    QVBoxLayout *leftPanelLayout;
    
    // Tables section
    QGroupBox *tablesGroup;
    QVBoxLayout *tablesLayout;
    QListWidget *tablesListWidget;
    
    // Relationships section
    QGroupBox *relationshipsGroup;
    QVBoxLayout *relationshipsLayout;
    QListWidget *relationshipsListWidget;
    
    // Center - Relationship Designer (Visual)
    QWidget *designerPanel;
    QVBoxLayout *designerLayout;
    QGraphicsView *designerView;
    QGraphicsScene *designerScene;
    
    // Right panel - Properties
    QWidget *propertiesPanel;
    QVBoxLayout *propertiesLayout;
    QGroupBox *propertiesGroup;
    
    // Properties content
    QScrollArea *propertiesScrollArea;
    QWidget *propertiesContent;
    QVBoxLayout *propertiesContentLayout;
    
    // Relationship creation controls
    QLabel *relationshipTypeLabel;
    QComboBox *relationshipTypeCombo;
    QLabel *sourceTableLabel;
    QComboBox *sourceTableCombo;
    QLabel *targetTableLabel;
    QComboBox *targetTableCombo;
    QLabel *sourceFieldLabel;
    QComboBox *sourceFieldCombo;
    QLabel *targetFieldLabel;
    QComboBox *targetFieldCombo;
    
    // Relationship details
    QLabel *relationshipNameLabel;
    QLineEdit *relationshipNameEdit;
    QLabel *descriptionLabel;
    QTextEdit *descriptionEdit;
    QCheckBox *enforceIntegrityCheck;
    QCheckBox *cascadeDeleteCheck;
    
    QPushButton *applyChangesBtn;
    QPushButton *cancelChangesBtn;
    
    // Data
    QStringList availableTables;
    QMap<QString, QStringList> tableFields;
    QList<TableGraphicsItem*> tableItems;
    QList<RelationshipLine*> relationshipLines;
    QList<RelationshipInfo> relationships; // Nueva lista para almacenar información completa de relaciones
    TableEditor *tableEditor; // Reference to table editor
    
    // Theme
    bool isDarkTheme;
};

// Custom graphics items for the visual designer
class TableGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    
public:
    TableGraphicsItem(const QString &tableName, const QRectF &rect, QGraphicsItem *parent = nullptr);
    TableGraphicsItem(const QString &tableName, QGraphicsItem *parent = nullptr);
    void setTableName(const QString &name);
    QString getTableName() const;
    void setFields(const QStringList &fields);
    void setFieldsWithKeys(const QStringList &fields, const QStringList &primaryKeys, const QStringList &foreignKeys);
    void updateTheme(bool isDark);
    
signals:
    void closeRequested(TableGraphicsItem* table);
    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    
private:
    QString tableName;
    QStringList fields;
    QStringList primaryKeys;
    QStringList foreignKeys;
    QGraphicsTextItem *nameText;
    QList<QGraphicsTextItem*> fieldTexts;
    bool isDarkTheme;
    bool isHovered;
    QRectF getCloseButtonRect() const;
};

class RelationshipLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
    
public:
    RelationshipLine(TableGraphicsItem *source, TableGraphicsItem *target, 
                     const QString &relationshipType, QGraphicsItem *parent = nullptr);
    void updatePosition();
    void setRelationshipType(const QString &type);
    QString getRelationshipType() const;
    TableGraphicsItem* getSourceTable() const;
    TableGraphicsItem* getTargetTable() const;
    void updateTheme(bool isDark);
    
signals:
    void doubleClicked(RelationshipLine* line);
    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    
private:
    TableGraphicsItem *sourceTable;
    TableGraphicsItem *targetTable;
    QString relationshipType;
    QGraphicsTextItem *typeText;
    bool isDarkTheme;
};

#endif // RELATIONSHIPSVIEW_H
