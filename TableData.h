#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QComboBox>
#include <QRegExp>

// Delegate para campos de datos - estilo consistente con TableView
class DataFieldDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DataFieldDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
};

class TableData : public QWidget
{
    Q_OBJECT

public:
    explicit TableData(QWidget *parent = nullptr);
    ~TableData();

    // Configurar la vista de datos basada en campos de diseño
    void setupDataView(const QStringList &fieldNames, const QStringList &fieldTypes);
    
    // Configurar nombre de tabla
    void setTableName(const QString &tableName);
    
    // Obtener datos ingresados
    QList<QStringList> getAllPersonData() const;
    
    // Limpiar todos los datos
    void clearAllData();
    QString fieldTypeForColumn(int col) const;
    void markCellInvalid(int row,int col,const QString& msg) const;
    void clearCellError(int row,int col) const;

public slots:
    void onPersonDataChanged(QTableWidgetItem *item);

private slots:
    void addNewPersonRow();
    void removeEmptyRows();
    void onDesignViewClicked();

signals:
    void switchToDesignView();
    void personDataChanged();
    void dataUpdated(const QList<QStringList> &allData);

private:
    void createUI();
    void createHeader();
    void setupTableForPersonData();
    void configureColumnWidths();
    void addPersonRow(const QStringList &personData = QStringList());
    QString getTableStyle();
    void updateExampleData();
    QString generateExampleData(const QString &dataType, int column);
    
    // UI Components
    QVBoxLayout *mainLayout;
    QWidget *headerWidget;
    QLabel *tableNameLabel;
    QPushButton *designViewBtn;
    QTableWidget *dataTable;
    
    // Data storage
    QStringList savedFieldNames;
    QStringList savedFieldTypes;
    QString currentTableName;
    int nextPersonId;
    
    // Delegates para estilo consistente con TableView
    DataFieldDelegate *dataFieldDelegate;
};

#endif // TABLEDATA_H
