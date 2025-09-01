#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableWidget>
#include <QFont>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QSplitter>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>
#include <QStyledItemDelegate>
#include <QPainter>

// Custom delegate for data type column
class DataTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DataTypeDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const override;
};

// Custom delegate for description column (multiline text)
class DescriptionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DescriptionDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

class TableView : public QWidget
{
    Q_OBJECT

public:
    explicit TableView(QWidget *parent = nullptr);
    void setTableName(const QString &tableName);
    void updateTheme(bool isDark);

private slots:
    void onCellSelectionChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void onCellDoubleClicked(int row, int column);
    void onDesignViewClicked();
    void onDataViewClicked();
    void onTableItemChanged(QTableWidgetItem *item);
    void onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    void setupUI();
    void createHeader();
    void createTableArea();
    void createPropertiesArea();
    void addToolbarButton(QHBoxLayout *layout, const QString &icon, const QString &tooltip);
    void styleTable();
    void setupDesignView();
    void setupDataView();
    void updateFieldProperties(int row, const QString &fieldName);
    void updatePropertyLabels(const QString &dataType);
    QLabel* createPropertyLabel(const QString &text);
    QString getInputStyle();
    QString getComboStyle();
    QString getCompactInputStyle();
    QString getCompactComboStyle();
    QString getEnhancedInputStyle();
    QString getEnhancedComboStyle();
    QString getEnhancedTextEditStyle();
    QString getModernInputStyle();
    QString getModernComboStyle();
    
    // UI Components
    QVBoxLayout *mainLayout;
    QWidget *headerWidget;
    QLabel *tableNameLabel;
    QSplitter *mainSplitter;
    
    // Table area
    QWidget *tableArea;
    QTableWidget *tableWidget;
    QPushButton *designViewBtn;
    QPushButton *dataViewBtn;
    
    // Properties area
    QWidget *propertiesArea;
    QLineEdit *fieldNameEdit;
    QComboBox *dataTypeCombo;
    QLabel *dataTypeLabel; // Replace dropdown with simple label
    QLineEdit *fieldSizeEdit;
    QComboBox *fieldSizeCombo; // Para opciones específicas de formato
    QCheckBox *requiredCheck;
    QLineEdit *defaultValueEdit;
    QLineEdit *descriptionEdit;
    
    QString currentTableName;
    bool isDarkTheme;
    
    // Variables para preservar datos entre cambios de vista
    QStringList savedFieldNames;
    QStringList savedFieldTypes;
    QStringList savedFieldDescriptions;
};

#endif // TABLEVIEW_H
