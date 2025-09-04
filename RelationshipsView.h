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

class TableGraphicsItem;
class RelationshipLine;

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

private slots:
    void onCreateRelationship();
    void onDeleteRelationship();
    void onTableSelectionChanged();
    void onRelationshipSelectionChanged();
    void showTableDetails(const QString &tableName);
    void addTableToDesigner(const QString &tableName, const QPoint &position);

private:
    void setupUI();
    void createToolbar();
    void createMainArea();
    void createRelationshipDesigner();
    void createPropertiesPanel();
    void createRelationshipsList();
    void styleComponents();
    void loadTables();
    void loadRelationships();
    void createRelationshipBetweenTables(const QString &table1, const QString &table2, 
                                       const QString &relationship_type);
    void updatePropertiesPanel(const QString &selectedItem);
    
    // UI Components
    QVBoxLayout *mainLayout;
    
    // Toolbar
    QWidget *toolbarWidget;
    QHBoxLayout *toolbarLayout;
    QPushButton *createRelationshipBtn;
    QPushButton *deleteRelationshipBtn;
    QPushButton *refreshBtn;
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
    
    // Theme
    bool isDarkTheme;
};

// Custom graphics items for the visual designer
class TableGraphicsItem : public QGraphicsRectItem
{
public:
    TableGraphicsItem(const QString &tableName, const QRectF &rect, QGraphicsItem *parent = nullptr);
    TableGraphicsItem(const QString &tableName, QGraphicsItem *parent = nullptr);
    void setTableName(const QString &name);
    QString getTableName() const;
    void setFields(const QStringList &fields);
    void updateTheme(bool isDark);
    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    
private:
    QString tableName;
    QStringList fields;
    QGraphicsTextItem *nameText;
    QList<QGraphicsTextItem*> fieldTexts;
    bool isDarkTheme;
};

class RelationshipLine : public QGraphicsLineItem
{
public:
    RelationshipLine(TableGraphicsItem *source, TableGraphicsItem *target, 
                     const QString &relationshipType, QGraphicsItem *parent = nullptr);
    void updatePosition();
    void setRelationshipType(const QString &type);
    QString getRelationshipType() const;
    TableGraphicsItem* getSourceTable() const;
    TableGraphicsItem* getTargetTable() const;
    void updateTheme(bool isDark);
    
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
private:
    TableGraphicsItem *sourceTable;
    TableGraphicsItem *targetTable;
    QString relationshipType;
    QGraphicsTextItem *typeText;
    bool isDarkTheme;
};

#endif // RELATIONSHIPSVIEW_H
