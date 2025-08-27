#ifndef THEMETOKENS_H
#define THEMETOKENS_H

#include <QString>
#include <QColor>

namespace ThemeTokens {

// Acento principal (marca)
const QString ACCENT_PRIMARY = "#A4373A";

// Light Theme
namespace Light {
    const QString BACKGROUND = "#FFFFFF";
    const QString SURFACE = "#FFF8F8";
    const QString TEXT_PRIMARY = "#161616";
    const QString TEXT_SECONDARY = "#5A5A5A";
    const QString BORDER = "#EDEDED";
    const QString HERO_GRADIENT_START = "rgba(164, 55, 58, 0.08)";
    const QString HERO_GRADIENT_END = "rgba(164, 55, 58, 0.02)";
    
    // Estados de botones
    const QString BUTTON_HOVER = "#8F2D30";
    const QString BUTTON_PRESSED = "#7A252A";
    const QString BUTTON_DISABLED = "#D1D5DB";
    
    // Estados de superficie
    const QString SURFACE_HOVER = "#F9F9F9";
    const QString SURFACE_PRESSED = "#F3F4F6";
    
    // Sombras
    const QString SHADOW_COLOR = "rgba(164, 55, 58, 0.12)";
    const QString CARD_SHADOW = "rgba(164, 55, 58, 0.08)";
}

// Dark Theme
namespace Dark {
    const QString BACKGROUND = "#0F0F11";
    const QString SURFACE = "#161619";
    const QString TEXT_PRIMARY = "#EDEDED";
    const QString TEXT_SECONDARY = "#9AA0A6";
    const QString BORDER = "#2A2A2E";
    const QString HERO_GRADIENT_START = "rgba(164, 55, 58, 0.04)";
    const QString HERO_GRADIENT_END = "rgba(164, 55, 58, 0.01)";
    
    // Acento ajustado para mejor legibilidad en dark
    const QString ACCENT_ADJUSTED = "#C94A4F";
    
    // Estados de botones
    const QString BUTTON_HOVER = "#D45A5F";
    const QString BUTTON_PRESSED = "#B43F44";
    const QString BUTTON_DISABLED = "#4B5563";
    
    // Estados de superficie
    const QString SURFACE_HOVER = "#1C1C20";
    const QString SURFACE_PRESSED = "#222227";
    
    // Sombras
    const QString SHADOW_COLOR = "rgba(0, 0, 0, 0.25)";
    const QString CARD_SHADOW = "rgba(0, 0, 0, 0.15)";
}

// Helpers para obtener colores según el tema
class ThemeHelper {
public:
    static QColor getAccentColor(bool isDark);
    static QColor getBackgroundColor(bool isDark);
    static QColor getSurfaceColor(bool isDark);
    static QColor getTextPrimaryColor(bool isDark);
    static QColor getTextSecondaryColor(bool isDark);
    static QColor getBorderColor(bool isDark);
    static QString getHeroGradientStart(bool isDark);
    static QString getHeroGradientEnd(bool isDark);
    static QString getButtonHoverColor(bool isDark);
    static QString getButtonPressedColor(bool isDark);
    static QString getShadowColor(bool isDark);
};

} // namespace ThemeTokens

#endif // THEMETOKENS_H
