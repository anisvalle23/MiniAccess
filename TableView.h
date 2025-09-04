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
    QStringList getCurrentFieldTypes() const;

signals:
    void switchToDataView();
    void tableDesignChanged(const QStringList &fieldNames, const QStringList &fieldTypes);

private slots:
    void onCellChanged(int row, int column);
    void onCellSelectionChanged();
    void onFieldNameChanged(const QString &text);
    void onDataTypeChanged(const QString &text);
    void onDescriptionChanged();
    void onRequiredChanged(bool checked);
    void onDefaultValueChanged(const QString &text);
    void onDataViewClicked();
    void onDesignViewClicked();
    void onFieldItemChanged(QTableWidgetItem *item);
    
    // Slots para propiedades específicas
    void onTextSizeChanged(const QString &text);
    void onNumberTypeChanged(const QString &text);
    void onCurrencyFormatChanged(const QString &text);
    void onDateFormatChanged(const QString &text);
    
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
    
    // Para tipo moneda
    QWidget *currencyPropertiesWidget;
    QComboBox *currencyFormatCombo; // Lps, Dollar, Euro, Millares
    
    // Para tipo fecha
    QWidget *datePropertiesWidget;
    QComboBox *dateFormatCombo; // DD-MM-YY, DD/MM/YY, DD/MESTEXTO/YYYY
    
    QString currentTableName;
    bool isDarkTheme;
    int currentSelectedRow;
};

#endif // TABLEVIEW_H
