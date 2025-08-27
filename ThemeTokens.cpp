#include "ThemeTokens.h"

namespace ThemeTokens {

QColor ThemeHelper::getAccentColor(bool isDark) {
    return QColor(isDark ? Dark::ACCENT_ADJUSTED : ACCENT_PRIMARY);
}

QColor ThemeHelper::getBackgroundColor(bool isDark) {
    return QColor(isDark ? Dark::BACKGROUND : Light::BACKGROUND);
}

QColor ThemeHelper::getSurfaceColor(bool isDark) {
    return QColor(isDark ? Dark::SURFACE : Light::SURFACE);
}

QColor ThemeHelper::getTextPrimaryColor(bool isDark) {
    return QColor(isDark ? Dark::TEXT_PRIMARY : Light::TEXT_PRIMARY);
}

QColor ThemeHelper::getTextSecondaryColor(bool isDark) {
    return QColor(isDark ? Dark::TEXT_SECONDARY : Light::TEXT_SECONDARY);
}

QColor ThemeHelper::getBorderColor(bool isDark) {
    return QColor(isDark ? Dark::BORDER : Light::BORDER);
}

QString ThemeHelper::getHeroGradientStart(bool isDark) {
    return isDark ? Dark::HERO_GRADIENT_START : Light::HERO_GRADIENT_START;
}

QString ThemeHelper::getHeroGradientEnd(bool isDark) {
    return isDark ? Dark::HERO_GRADIENT_END : Light::HERO_GRADIENT_END;
}

QString ThemeHelper::getButtonHoverColor(bool isDark) {
    return isDark ? Dark::BUTTON_HOVER : Light::BUTTON_HOVER;
}

QString ThemeHelper::getButtonPressedColor(bool isDark) {
    return isDark ? Dark::BUTTON_PRESSED : Light::BUTTON_PRESSED;
}

QString ThemeHelper::getShadowColor(bool isDark) {
    return isDark ? Dark::SHADOW_COLOR : Light::SHADOW_COLOR;
}

} // namespace ThemeTokens
