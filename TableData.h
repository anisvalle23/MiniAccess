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
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

class TableData : public QWidget
{
    Q_OBJECT

public:
    explicit TableData(QWidget *parent = nullptr);
    ~TableData();

    // Configurar la vista de datos basada en campos de diseño
    void setupDataView(const QStringList &fieldNames, const QStringList &fieldTypes, int primaryKeyColumn = -1);
    void setupDataViewWithFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, int primaryKeyColumn = -1);
    void setupDataViewWithFormatsAndDecimals(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, int primaryKeyColumn = -1);
    void setupDataViewWithTextSizes(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, int primaryKeyColumn = -1);
    
    // Configurar nombre de tabla
    void setTableName(const QString &tableName);
    
    // Obtener datos ingresados
    QList<QStringList> getAllPersonData() const;
    
    // Verificar duplicados en una columna específica
    bool hasColumnDuplicates(int columnIndex) const;
    
    // Limpiar todos los datos
    void clearAllData();
    QString fieldTypeForColumn(int col) const;
    void markCellInvalid(int row,int col,const QString& msg) const;
    void clearCellError(int row,int col) const;
    bool isValueValidForType(const QString& type, const QString& value) const;
    void showSoftWarning(int row, int col, const QString& msg) const;
    QString formatCurrency(const QString& raw) const;
    QString formatCurrencyWithFormat(const QString& raw, const QString& format) const; // Formatear con formato específico
    QString formatCurrencyWithFormatAndDecimals(const QString& raw, const QString& format, const QString& decimals) const; // Formatear con formato y decimales específicos
    QString getCurrencyFormatForColumn(int column) const; // Obtener formato para columna específica
    QString getMillaresDecimalsForColumn(int column) const; // Obtener decimales para columna específica
    QString getTextSizeForColumn(int column) const; // Obtener tamaño de texto para columna específica
    
    // Actualizar tema
    void updateTheme(bool isDark);


public slots:
    void onPersonDataChanged(QTableWidgetItem *item);

private slots:
    void addNewPersonRow();
    void removeEmptyRows();
    void onDesignViewClicked();
    void addNewRow();
    void deleteSelectedRow();

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
    void applyCurrencyFormats(); // Aplicar formatos de moneda específicos
    
    // UI Components
    QVBoxLayout *mainLayout;
    QWidget *headerWidget;
    QLabel *tableNameLabel;
    QPushButton *designViewBtn;
    QTableWidget *dataTable;
    
    // Data storage
    QStringList savedFieldNames;
    QStringList savedFieldTypes;
    QStringList savedCurrencyFormats; // Formatos de moneda para cada campo
    QStringList savedMillaresDecimals; // Decimales de millares para cada campo
    QStringList savedTextSizes; // Tamaños de texto para cada campo
    QString currentTableName;
    int nextPersonId;
    int primaryKeyColumnIndex; // Índice de la columna Primary Key (-1 si no hay)
    
    // Delegates para estilo consistente con TableView
    DataFieldDelegate *dataFieldDelegate;

    mutable QLabel *m_warnLabel = nullptr;
    mutable QTimer *m_warnTimer = nullptr;
};

#endif // TABLEDATA_H
