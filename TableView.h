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
#include <QMessageBox>
#include <QMessageBox>
#include <QTimer>
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

// Custom delegate for field name column
class FieldNameDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FieldNameDelegate(QObject *parent = nullptr);
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
    
    // Obtener campos actuales del diseño
    QStringList getCurrentFieldNames() const;
    QStringList getAllFieldNames() const; // Método que incluye las llaves 🔑
    QStringList getCurrentFieldTypes() const;
    QStringList getCurrentCurrencyFormats() const;
    QStringList getCurrentNumberTypes() const; // Nuevo método para obtener tipos de números
    QStringList getCurrentDateFormats() const; // Nuevo método para obtener formatos de fecha
    QStringList getCurrentMillaresDecimals() const; // Nuevo método para obtener decimales de millares
    QStringList getCurrentTextSizes() const; // Nuevo método para obtener tamaños de texto
    int getPrimaryKeyColumnIndex() const; // Nuevo método para obtener índice de Primary Key
    QList<int> getUniqueKeyColumnIndexes() const; // Nuevo método para obtener índices de campos únicos
    
    // Métodos para obtener nombres de campos con llaves
    QStringList getPrimaryKeyFieldNames() const; // Obtener nombres de campos que son Primary Key
    QStringList getForeignKeyFieldNames() const; // Obtener nombres de campos que son Foreign Key
    
    // Método para recibir resultado de validación de duplicados
    void setUniqueValidationResult(const QString &fieldName, bool hasDuplicates);

signals:
    void switchToDataView();
    void tableDesignChanged(const QStringList &fieldNames, const QStringList &fieldTypes);
    void tableDesignChangedWithFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats);
    void tableDesignChangedWithFormatsAndDecimals(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals);
    void tableDesignChangedWithAllFormats(const QStringList &fieldNames, const QStringList &fieldTypes, const QStringList &currencyFormats, const QStringList &millaresDecimals, const QStringList &numberTypes, const QStringList &dateFormats);
    void foreignKeyRemoved(const QString &tableName, const QString &fieldName); // Nueva señal para FK eliminada
    void checkUniqueFieldDuplicates(const QString &fieldName, int fieldIndex); // Nueva señal para validar duplicados

private slots:
    void onCellChanged(int row, int column);
    void onCellSelectionChanged();
    void onFieldNameChanged(const QString &text);
    void onDataTypeChanged(const QString &text);
    void onDescriptionChanged();
    void onRequiredChanged(bool checked);
    void onForeignKeyChanged(bool checked);
    void onUniqueChanged(bool checked);
    void onDefaultValueChanged(const QString &text);
    void onDataViewClicked();
    void onDesignViewClicked();
    void onFieldItemChanged(QTableWidgetItem *item);
    void onAddRowClicked();
    void onDeleteRowClicked();
    void onNumberDecimalsChanged(const QString &dec);

    // Slots para propiedades específicas
    void onTextSizeChanged(const QString &text);
    void onNumberTypeChanged(const QString &text);
    // void onDecimalPlacesUpdated(const QString &text); // REMOVIDO - usar lambda en connect
    void onCurrencyFormatChanged(const QString &text);
    void onDateFormatChanged(const QString &text);
    void onMillaresDecimalsChanged(const QString &text); // Nuevo slot para decimales de millares
    
    // Función para actualizar ejemplos de datos
    void updateExampleData();
    
    // Función helper para generar ejemplo según tipo de dato
    QString generateExampleData(const QString &dataType, int column);

private:
    void createInterface();
    void createHeader();
    void createTableArea();
    void createPropertiesArea();
    void setupDesignTable();
    void updatePropertiesForRow(int row);
    void updateTableFromProperties();
    void updateSpecificProperties(const QString &dataType);
    void createSpecificPropertiesWidgets();
    void ensureEmptyRowExists();
    void addNewRow();
    QString getTableStyle();
    QString getInputStyle();
    QString getComboStyle();
    QString getTextEditStyle();
    
    // Método para validar integridad de llave primaria
    void validatePrimaryKeyIntegrity();
    bool checkForDuplicates(const QString &fieldName);
    
    // UI Components
    QVBoxLayout *mainLayout;
    QWidget *headerWidget;
    QLabel *tableNameLabel;
    QPushButton *dataViewBtn;
    QSplitter *mainSplitter;
    
    // Table area
    QWidget *tableArea;
    QTableWidget *tableWidget;
    FieldNameDelegate *fieldNameDelegate;
    DataTypeDelegate *dataTypeDelegate;
    DescriptionDelegate *descriptionDelegate;
    
    // Properties area  
    QWidget *propertiesArea;
    QLineEdit *fieldNameEdit;
    QComboBox *dataTypeCombo;
    QTextEdit *descriptionEdit;
    QCheckBox *requiredCheck;
    QCheckBox *foreignKeyCheck;
    QCheckBox *uniqueCheck;
    QLineEdit *defaultValueEdit;
    
    // Propiedades específicas por tipo de dato
    QWidget *specificPropertiesWidget;
    QVBoxLayout *specificPropertiesLayout;
    
    // Para tipo texto (char[N] y string)
    QWidget *textPropertiesWidget;
    QLineEdit *textSizeEdit; // Tamaño máximo del campo texto
    QLabel *textValidationLabel; // Label para mostrar validación de texto
    
    // Para tipo número (int y float)
    QWidget *numberPropertiesWidget;
    QComboBox *numberTypeCombo; // Entero, Decimal, Doble, Byte
    QLabel *numberSizeLabel; // Label para mostrar información de tamaño
    QComboBox *decimalPlacesCombo; // Combo para decimales cuando es Decimal o Doble
    QLabel *decimalPlacesLabel; // Label para decimales
    
    // Para tipo moneda
    QWidget *currencyPropertiesWidget;
    QComboBox *currencyFormatCombo; // Lps, Dollar, Euro, Millares
    QComboBox *millaresDecimalsCombo; // Combo para decimales cuando es Millares
    QLabel *millaresDecimalsLabel; // Label explicativo
    
    // Para tipo fecha
    QWidget *datePropertiesWidget;
    QComboBox *dateFormatCombo; // DD-MM-YY, DD/MM/YY, DD/MESTEXTO/YYYY
    
    QString currentTableName;
    bool isDarkTheme;
    int currentSelectedRow;
    int primaryKeyRow; // Fila que contiene la llave primaria (-1 si no hay)
    QList<int> foreignKeyRows; // Lista de filas que son Foreign Keys
    QList<int> uniqueKeyRows; // Lista de filas que son campos únicos
    
    // Almacenar formatos de moneda por campo
    QStringList fieldCurrencyFormats;
    QStringList fieldMillaresDecimals; // Almacenar decimales para millares por campo
    QStringList fieldTextSizes; // Almacenar tamaños de campo de texto por campo
    QStringList fieldNumberTypes; // Almacenar tipos específicos de números por campo
    QStringList fieldDateFormats; // Almacenar formatos de fecha por campo
    QStringList fieldDecimalPlaces; // Almacenar número de decimales para campos Decimal/Doble

    QLabel *numberDecimalsLabel = nullptr;
    QComboBox *numberDecimalsCombo = nullptr;


};

#endif // TABLEVIEW_H
