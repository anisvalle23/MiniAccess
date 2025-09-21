#ifndef FORMULARIOSVIEW_H
#define FORMULARIOSVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QFormLayout>
#include <QTableWidget>
#include <QSplitter>
#include <QFont>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "ThemeManager.h"
#include "catalogbplustree.h"

class MainWindow;

class FormulariosView : public QWidget
{
    Q_OBJECT

public:
    explicit FormulariosView(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~FormulariosView();

    void updateTheme();

public slots:
    void refreshView();
    void onTableCreated(const QString& tableName);  // Nuevo slot para actualizar cuando se crea una tabla

private slots:
    void onCreateFormClicked();
    void onTableSelected();
    void onSaveRecordClicked();
    void onClearFormClicked();
    void onRefreshDataClicked();
    void onRecordSelected();
    void onNewRecordClicked();
    void onEditRecordClicked();
    void onDeleteRecordClicked();
    void onFirstRecordClicked();
    void onPreviousRecordClicked();
    void onNextRecordClicked();
    void onLastRecordClicked();
    void onRecordNavigationChanged();

private:
    void setupUI();
    void createHeaderSection();
    void loadAvailableTables();
    void generateFormForTable(const QString& tableName);
    void loadTableData(const QString& tableName);
    void updateDataTable();
    void clearForm();
    void clearFormInputs();
    void showEmptyState();
    QWidget* createFieldWidget(const QJsonObject& fieldMeta);
    QJsonObject getFormData();
    void populateFormWithRecord(const QJsonObject& record);
    void updateButtonStates();
    void setFormEnabled(bool enabled);
    void saveTableData();
    bool validateFormData();
    
    // Funciones de manejo de datos (similar a TableData)
    void loadDataFromJson();
    void saveDataToJson();
    void populateFormWithRecord(int recordIndex);
    QJsonObject getCurrentRecordData();
    void updateCurrentRecord();
    void addNewRecord();
    void deleteCurrentRecord();
    
    // Funciones de navegación entre registros
    void goToRecord(int index);
    void updateNavigationState();
    QString cleanFieldNameUI(const QString& name);
    
    // Funciones auxiliares de validación
    QString getWidgetValue(QWidget* widget);
    void setFieldError(QWidget* widget, bool hasError);
    void clearValidationErrors();
    bool isValidEmail(const QString& email);
    bool isValidPhone(const QString& phone);
    bool isValidDate(const QString& dateStr);
    void setupRealTimeValidation(QWidget* widget, const QJsonObject& fieldMeta);
    void showFieldError(QWidget* widget, const QString& errorMessage);
    void hideFieldError(QWidget* widget);

    // UI Components
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QComboBox *tableComboBox;
    
    // Buttons
    QPushButton *newRecordBtn;
    QPushButton *editRecordBtn;
    QPushButton *deleteRecordBtn;
    QPushButton *saveRecordBtn;
    
    // Content Area
    QSplitter *mainSplitter;
    
    // Form Area (lado izquierdo)
    QWidget *formAreaWidget;
    QVBoxLayout *formAreaLayout;
    QScrollArea *formScrollArea;
    QWidget *formContentWidget;
    QFormLayout *formLayout;
    
    // Data Viewer (lado derecho)
    QWidget *dataViewerWidget;
    QVBoxLayout *dataViewerLayout;
    QTableWidget *dataTable;
    
    // Empty state
    QWidget *emptyStateWidget;
    QVBoxLayout *emptyStateLayout;
    QLabel *emptyStateLabel;
    
    // Reference to main window
    MainWindow *m_mainWindow;
    
    // Current state
    QString currentTableName;
    QJsonArray currentTableFields;
    QJsonArray tableData;
    int currentRecordIndex;
    bool isEditMode;
    QList<QWidget*> formWidgets;
    QStringList fieldNames;
    
    // Datos y navegación (similar a TableData)
    QList<QJsonObject> allRecords;  // Todos los registros de la tabla actual
    bool hasUnsavedChanges;
    QStringList savedFieldNames;   // Nombres de campos como están en el JSON
    QStringList savedFieldTypes;   // Tipos de campos
    QTimer *dataDebounceTimer;     // Para guardar cambios automáticamente
};

#endif // FORMULARIOSVIEW_H