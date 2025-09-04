#ifndef THEMEPOPOVER_H
#define THEMEPOPOVER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include "ThemeManager.h"

class ThemePopover : public QWidget
{
    Q_OBJECT

public:
    explicit ThemePopover(QWidget *parent = nullptr);

    void setSettingsButton(QPushButton *button);
    void showPopover(const QPoint &position);
    void hidePopover();
    bool isPopoverVisible() const;
    void closeImmediately();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;
    void hideEvent(QHideEvent* e) override;

private slots:
    void onThemeRadioToggled();
    void onThemeChanged(ThemeManager::Theme theme);

private:
    void setupUI();
    void setupAnimations();
    void updateThemeSelection();
    void positionPopover(const QPoint &anchorPoint);
    void setupCloseButtonStyle();

    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QRadioButton *m_lightRadio;
    QRadioButton *m_darkRadio;
    QButtonGroup *m_themeGroup;
    QLabel *m_descriptionLabel;

    // NEW: header con botón de cierre
    QHBoxLayout *m_headerLayout;     // NEW
    QPushButton *m_closeButton;      // NEW

    // Animaciones
    QPropertyAnimation *m_showAnimation;
    QPropertyAnimation *m_hideAnimation;
    QGraphicsOpacityEffect *m_opacityEffect;
    QGraphicsDropShadowEffect *m_shadowEffect;

    bool m_isVisible;
    QPushButton *m_settingsButton;
};

#endif // THEMEPOPOVER_H
