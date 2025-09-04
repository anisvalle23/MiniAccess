#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <QPalette>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "ThemeTokens.h"

class ThemeManager : public QObject
{
    Q_OBJECT
    
public:
    enum class Theme {
        Light,
        Dark
    };
    Q_ENUM(Theme)
    
    // Singleton access
    static ThemeManager& instance();
    
    // API principal
    Theme currentTheme() const { return m_currentTheme; }
    bool isDark() const { return m_currentTheme == Theme::Dark; }
    
    void setTheme(Theme theme, bool persist = true);
    void toggleTheme();
    void loadFromSettings();
    void saveToSettings();
    void applyTheme();
    
    // Helper para obtener stylesheet completo
    QString getGlobalStyleSheet() const;
    QString getHeroStyleSheet() const;
    
signals:
    void themeChanged(Theme theme);
    
private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() = default;
    
    // Prevenir copia
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    // Métodos internos
    void setupPalette();
    void applyTransition();
    QString buildGlobalStyleSheet() const;
    QString buildButtonStyles() const;
    QString buildInputStyles() const;
    QString buildScrollBarStyles() const;
    QString buildGeneralStyles() const;
    
    Theme m_currentTheme;
    QSettings m_settings;
    
    // Para transiciones suaves
    QPropertyAnimation *m_fadeAnimation;
    QGraphicsOpacityEffect *m_fadeEffect;
    
    static const QString SETTINGS_KEY_THEME;
    static const QString ORGANIZATION_NAME;
    static const QString APPLICATION_NAME;
};

#endif // THEMEMANAGER_H
