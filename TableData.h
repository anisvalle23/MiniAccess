#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QWidget>
#include <QVBoxLayout>
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
#include <QTimer>
#include "projectpathsqt.h"

// Forward declaration
class RelationshipsView;
class TableEditor;

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
    void setupDataViewWithUniqueFields(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, const QStringList &numberTypes, const QStringList &dateFormats, const QList<int> &uniqueColumns, int primaryKeyColumn = -1);
    void setupDataViewWithAllFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &textSizes, const QStringList &numberTypes, const QStringList &dateFormats, const QList<int> &uniqueColumns, int primaryKeyColumn = -1);

    // Configurar nombre de tabla
    void setTableName(const QString &tableName);

    // Configurar referencia a RelationshipsView para validaciones FK
    void setRelationshipsView(RelationshipsView *relationshipsView);

    // Configurar referencia a TableEditor para acceso a datos
    void setTableEditor(TableEditor *tableEditor);

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
    QString getNumberTypeForColumn(int column) const; // Obtener tipo de número para columna específica
    QString getDateFormatForColumn(int column) const; // Obtener formato de fecha para columna específica
    QString getMillaresDecimalsForColumn(int column) const; // Obtener decimales para columna específica
    QString getTextSizeForColumn(int column) const; // Obtener tamaño de texto para columna específica

    // Métodos para filtros y búsqueda
    void createFilterControls();
    void applyFilters();
    void clearFilters();
    void sortByColumn(int column, Qt::SortOrder order);
    void sortDataRowsOnly(int column, bool ascending); // Nueva función para ordenar solo filas con datos

    // Actualizar tema
    void updateTheme(bool isDark);

    // Acceso a formatos guardados
    QStringList getSavedDateFormats() const { return savedDateFormats; }
    QString formatDateWithTextMonth(const QDate &date, const QString &format) const; // Convertir fecha a formato con mes en texto

    // Manejo de Foreign Key changes
    void onForeignKeyRemoved(const QString &tableName, const QString &fieldName); // Notificación cuando se quita FK

    void saveAllToMad();
    void flushPendingDataSave();
    void loadDataFromMad();

public slots:
    void onPersonDataChanged(QTableWidgetItem *item);

private slots:
    void addNewPersonRow();
    void removeEmptyRows();
    void onDesignViewClicked();
    void addNewRow();
    void deleteSelectedRow();
    void onDataItemChanged(QTableWidgetItem* it);

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
    void applyNumberFormats(); // Aplicar formatos de números específicos
    void applyDateFormats(); // Aplicar formatos de fecha específicos

    // Métodos para validación de llaves foráneas
    bool validateForeignKeyConstraints(int row);
    bool isFieldForeignKey(const QString &fieldName);
    bool hasEstablishedRelationship(const QString &fieldName);
    QString getReferencedTable(const QString &fieldName);
    QString getReferencedField(const QString &fieldName);
    bool valueExistsInReferencedTable(const QString &tableName, const QString &fieldName, const QString &value);
    bool isOneToOnePrimaryKeyRelation(const QString &currentTable, const QString &currentField, 
                                     const QString &referencedTable, const QString &referencedField);
    bool validateOneToOneConstraint(const QString &currentTable, const QString &currentField,
                                   const QString &referencedTable, const QString &referencedField, 
                                   const QString &value);
    QStringList getTableData(const QString &tableName, const QString &fieldName);
    QStringList getTableDataDirectFromCatalog(const QString &tableName, const QString &fieldName);

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
    QStringList savedNumberTypes; // Tipos de números para cada campo
    QStringList savedDateFormats; // Formatos de fecha para cada campo
    QStringList savedMillaresDecimals; // Decimales de millares para cada campo
    QStringList savedTextSizes; // Tamaños de texto para cada campo
    QList<int> savedUniqueColumns; // Índices de columnas que deben ser únicas
    QString currentTableName;
    int nextPersonId;
    int primaryKeyColumnIndex; // Índice de la columna Primary Key (-1 si no hay)

    // Delegates para estilo consistente con TableView
    DataFieldDelegate *dataFieldDelegate;

    // Referencia a RelationshipsView para validaciones FK
    RelationshipsView *relationshipsView;

    // Referencia a TableEditor para acceso a datos
    TableEditor *tableEditor;

    // Controles de filtro y búsqueda
    QLineEdit *searchField;
    QComboBox *sortColumnCombo;
    QComboBox *sortOrderCombo;
    QComboBox *numberCondition;
    QLineEdit *numberValue1;
    QLineEdit *numberValue2;
    QPushButton *clearFiltersBtn;

    mutable QLabel *m_warnLabel = nullptr;
    mutable QTimer *m_warnTimer = nullptr;

    QTimer* dataDebounceTimer = nullptr;
};

#endif // TABLEDATA_H
