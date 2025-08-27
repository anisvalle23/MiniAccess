#include "ThemeManager.h"
#include <QDebug>

const QString ThemeManager::SETTINGS_KEY_THEME = "ui/theme";
const QString ThemeManager::ORGANIZATION_NAME = "MiniAccess";
const QString ThemeManager::APPLICATION_NAME = "MiniAccess";

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentTheme(Theme::Light)
    , m_settings(ORGANIZATION_NAME, APPLICATION_NAME)
    , m_fadeAnimation(nullptr)
    , m_fadeEffect(nullptr)
{
    // Configurar animación de transición
    m_fadeAnimation = new QPropertyAnimation(this);
    m_fadeAnimation->setDuration(200);
    m_fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void ThemeManager::setTheme(Theme theme, bool persist)
{
    if (m_currentTheme == theme) {
        return;
    }
    
    m_currentTheme = theme;
    
    if (persist) {
        saveToSettings();
    }
    
    applyTransition();
    emit themeChanged(theme);
}

void ThemeManager::toggleTheme()
{
    Theme newTheme = (m_currentTheme == Theme::Light) ? Theme::Dark : Theme::Light;
    setTheme(newTheme, true);
}

void ThemeManager::loadFromSettings()
{
    QString themeString = m_settings.value(SETTINGS_KEY_THEME, "light").toString();
    
    Theme theme = Theme::Light;
    if (themeString == "dark") {
        theme = Theme::Dark;
    }
    
    m_currentTheme = theme;
}

void ThemeManager::saveToSettings()
{
    QString themeString = (m_currentTheme == Theme::Dark) ? "dark" : "light";
    m_settings.setValue(SETTINGS_KEY_THEME, themeString);
    m_settings.sync();
}

void ThemeManager::applyTheme()
{
    setupPalette();
    qApp->setStyleSheet(getGlobalStyleSheet());
    qApp->processEvents(); // Asegurar que se aplique inmediatamente
}

void ThemeManager::applyTransition()
{
    // Para una transición más suave, aplicamos el tema inmediatamente
    // pero con una breve animación de opacidad si es posible
    applyTheme();
}

void ThemeManager::setupPalette()
{
    QPalette palette;
    bool dark = isDark();
    
    // Colores base
    QColor background = ThemeTokens::ThemeHelper::getBackgroundColor(dark);
    QColor surface = ThemeTokens::ThemeHelper::getSurfaceColor(dark);
    QColor textPrimary = ThemeTokens::ThemeHelper::getTextPrimaryColor(dark);
    QColor textSecondary = ThemeTokens::ThemeHelper::getTextSecondaryColor(dark);
    QColor accent = ThemeTokens::ThemeHelper::getAccentColor(dark);
    QColor border = ThemeTokens::ThemeHelper::getBorderColor(dark);
    
    // Configurar palette base
    palette.setColor(QPalette::Window, background);
    palette.setColor(QPalette::WindowText, textPrimary);
    palette.setColor(QPalette::Base, surface);
    palette.setColor(QPalette::AlternateBase, surface.lighter(110));
    palette.setColor(QPalette::Text, textPrimary);
    palette.setColor(QPalette::Button, surface);
    palette.setColor(QPalette::ButtonText, textPrimary);
    palette.setColor(QPalette::BrightText, textPrimary);
    palette.setColor(QPalette::Link, accent);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, background);
    
    // Estados disabled
    QColor disabledText = dark ? QColor("#4B5563") : QColor("#9CA3AF");
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);
    
    qApp->setPalette(palette);
}

QString ThemeManager::getGlobalStyleSheet() const
{
    return buildGlobalStyleSheet();
}

QString ThemeManager::getHeroStyleSheet() const
{
    bool dark = isDark();
    QString gradientStart = ThemeTokens::ThemeHelper::getHeroGradientStart(dark);
    QString gradientEnd = ThemeTokens::ThemeHelper::getHeroGradientEnd(dark);
    
    return QString(
        "/* Hero Background Gradient */ "
        ".hero-background { "
        "    background: qradialgradient(cx:0.5, cy:0.4, radius:1.2, "
        "        stop:0 %1, stop:1 %2); "
        "} "
    ).arg(gradientStart, gradientEnd);
}

QString ThemeManager::buildGlobalStyleSheet() const
{
    QString stylesheet;
    stylesheet += buildGeneralStyles();
    stylesheet += buildButtonStyles();
    stylesheet += buildInputStyles();
    stylesheet += buildScrollBarStyles();
    return stylesheet;
}

QString ThemeManager::buildGeneralStyles() const
{
    bool dark = isDark();
    QString background = dark ? ThemeTokens::Dark::BACKGROUND : ThemeTokens::Light::BACKGROUND;
    QString surface = dark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString textPrimary = dark ? ThemeTokens::Dark::TEXT_PRIMARY : ThemeTokens::Light::TEXT_PRIMARY;
    QString textSecondary = dark ? ThemeTokens::Dark::TEXT_SECONDARY : ThemeTokens::Light::TEXT_SECONDARY;
    QString border = dark ? ThemeTokens::Dark::BORDER : ThemeTokens::Light::BORDER;
    QString shadowColor = ThemeTokens::ThemeHelper::getShadowColor(dark);
    
    return QString(
        "/* General Styles */ "
        "QMainWindow { "
        "    background-color: %1; "
        "    color: %2; "
        "} "
        "QWidget { "
        "    background-color: transparent; "
        "    color: %2; "
        "} "
        "QFrame { "
        "    border: 1px solid %3; "
        "    background-color: %4; "
        "} "
        "QLabel { "
        "    background-color: transparent; "
        "    color: %2; "
        "} "
        "/* Cards and Surfaces */ "
        ".surface { "
        "    background-color: %4; "
        "    border: 1px solid %3; "
        "    border-radius: 12px; "
        "} "
        ".card { "
        "    background-color: %4; "
        "    border: 1px solid %3; "
        "    border-radius: 16px; "
        "    /* box-shadow: 0 4px 12px %5; */ "
        "} "
    ).arg(background, textPrimary, border, surface, shadowColor);
}

QString ThemeManager::buildButtonStyles() const
{
    bool dark = isDark();
    QString accent = dark ? ThemeTokens::Dark::ACCENT_ADJUSTED : ThemeTokens::ACCENT_PRIMARY;
    QString buttonHover = ThemeTokens::ThemeHelper::getButtonHoverColor(dark);
    QString buttonPressed = ThemeTokens::ThemeHelper::getButtonPressedColor(dark);
    QString surface = dark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString surfaceHover = dark ? ThemeTokens::Dark::SURFACE_HOVER : ThemeTokens::Light::SURFACE_HOVER;
    QString border = dark ? ThemeTokens::Dark::BORDER : ThemeTokens::Light::BORDER;
    QString textPrimary = dark ? ThemeTokens::Dark::TEXT_PRIMARY : ThemeTokens::Light::TEXT_PRIMARY;
    
    return QString(
        "/* Primary Buttons */ "
        "QPushButton { "
        "    background-color: %1; "
        "    color: white; "
        "    border: none; "
        "    border-radius: 8px; "
        "    padding: 8px 16px; "
        "    font-weight: 500; "
        "} "
        "QPushButton:hover { "
        "    background-color: %2; "
        "} "
        "QPushButton:pressed { "
        "    background-color: %3; "
        "} "
        "QPushButton:disabled { "
        "    background-color: %4; "
        "    color: #9CA3AF; "
        "} "
        "/* Secondary Buttons */ "
        "QPushButton.secondary { "
        "    background-color: %5; "
        "    color: %6; "
        "    border: 1px solid %7; "
        "} "
        "QPushButton.secondary:hover { "
        "    background-color: %8; "
        "} "
        "/* Icon Buttons */ "
        "QPushButton.icon { "
        "    background-color: transparent; "
        "    border: none; "
        "    border-radius: 6px; "
        "    padding: 6px; "
        "} "
        "QPushButton.icon:hover { "
        "    background-color: %8; "
        "} "
    ).arg(accent, buttonHover, buttonPressed,
          dark ? ThemeTokens::Dark::BUTTON_DISABLED : ThemeTokens::Light::BUTTON_DISABLED,
          surface, textPrimary, border, surfaceHover);
}

QString ThemeManager::buildInputStyles() const
{
    bool dark = isDark();
    QString surface = dark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString border = dark ? ThemeTokens::Dark::BORDER : ThemeTokens::Light::BORDER;
    QString textPrimary = dark ? ThemeTokens::Dark::TEXT_PRIMARY : ThemeTokens::Light::TEXT_PRIMARY;
    QString accent = dark ? ThemeTokens::Dark::ACCENT_ADJUSTED : ThemeTokens::ACCENT_PRIMARY;
    
    return QString(
        "/* Input Fields */ "
        "QLineEdit, QTextEdit, QPlainTextEdit { "
        "    background-color: %1; "
        "    border: 1px solid %2; "
        "    border-radius: 6px; "
        "    padding: 8px 12px; "
        "    color: %3; "
        "} "
        "QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus { "
        "    border-color: %4; "
        "} "
        "/* Combo Boxes */ "
        "QComboBox { "
        "    background-color: %1; "
        "    border: 1px solid %2; "
        "    border-radius: 6px; "
        "    padding: 6px 12px; "
        "    color: %3; "
        "} "
        "QComboBox:hover { "
        "    border-color: %4; "
        "} "
        "QComboBox::drop-down { "
        "    border: none; "
        "} "
        "QComboBox::down-arrow { "
        "    image: none; "
        "    border: none; "
        "} "
    ).arg(surface, border, textPrimary, accent);
}

QString ThemeManager::buildScrollBarStyles() const
{
    bool dark = isDark();
    QString surface = dark ? ThemeTokens::Dark::SURFACE : ThemeTokens::Light::SURFACE;
    QString border = dark ? ThemeTokens::Dark::BORDER : ThemeTokens::Light::BORDER;
    QString surfaceHover = dark ? ThemeTokens::Dark::SURFACE_HOVER : ThemeTokens::Light::SURFACE_HOVER;
    
    return QString(
        "/* Scroll Bars */ "
        "QScrollBar:vertical { "
        "    background-color: %1; "
        "    width: 12px; "
        "    border-radius: 6px; "
        "} "
        "QScrollBar::handle:vertical { "
        "    background-color: %2; "
        "    border-radius: 6px; "
        "    min-height: 20px; "
        "} "
        "QScrollBar::handle:vertical:hover { "
        "    background-color: %3; "
        "} "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "    border: none; "
        "    background: none; "
        "} "
        "QScrollBar:horizontal { "
        "    background-color: %1; "
        "    height: 12px; "
        "    border-radius: 6px; "
        "} "
        "QScrollBar::handle:horizontal { "
        "    background-color: %2; "
        "    border-radius: 6px; "
        "    min-width: 20px; "
        "} "
        "QScrollBar::handle:horizontal:hover { "
        "    background-color: %3; "
        "} "
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { "
        "    border: none; "
        "    background: none; "
        "} "
    ).arg(surface, border, surfaceHover);
}
