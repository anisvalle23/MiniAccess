#include "ThemePopover.h"
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>

ThemePopover::ThemePopover(QWidget *parent)
    : QWidget(parent)
    , m_isVisible(false)
    , m_settingsButton(nullptr)
{
    setupUI();
    setupAnimations();
    
    // Configurar ventana
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(200, 120);
    
    // Conectar al ThemeManager
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ThemePopover::onThemeChanged);
    
    // Instalar filtro de eventos para cerrar al hacer click fuera
    qApp->installEventFilter(this);
    
    updateThemeSelection();
}

void ThemePopover::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 12, 16, 12);
    m_mainLayout->setSpacing(8);
    
    // Título
    m_titleLabel = new QLabel("Tema");
    m_titleLabel->setStyleSheet(
        "font-weight: 600; "
        "font-size: 14px; "
        "margin-bottom: 4px;"
    );
    
    // Radio buttons para temas
    m_lightRadio = new QRadioButton("Claro");
    m_darkRadio = new QRadioButton("Oscuro");
    
    m_themeGroup = new QButtonGroup(this);
    m_themeGroup->addButton(m_lightRadio, 0);
    m_themeGroup->addButton(m_darkRadio, 1);
    
    // Descripción
    m_descriptionLabel = new QLabel("Se aplica a toda la aplicación.");
    m_descriptionLabel->setStyleSheet(
        "font-size: 11px; "
        "color: #9AA0A6; "
        "margin-top: 4px;"
    );
    m_descriptionLabel->setWordWrap(true);
    
    // Layout
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_lightRadio);
    m_mainLayout->addWidget(m_darkRadio);
    m_mainLayout->addWidget(m_descriptionLabel);
    m_mainLayout->addStretch();
    
    // Conectar señales
    connect(m_lightRadio, &QRadioButton::toggled, this, &ThemePopover::onThemeRadioToggled);
    connect(m_darkRadio, &QRadioButton::toggled, this, &ThemePopover::onThemeRadioToggled);
}

void ThemePopover::setupAnimations()
{
    // Efecto de opacidad
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);
    
    // Efecto de sombra
    m_shadowEffect = new QGraphicsDropShadowEffect();
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setColor(QColor(0, 0, 0, 60));
    m_shadowEffect->setOffset(0, 8);
    
    // Animación de aparición
    m_showAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_showAnimation->setDuration(180);
    m_showAnimation->setStartValue(0.0);
    m_showAnimation->setEndValue(1.0);
    m_showAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // Animación de desaparición
    m_hideAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_hideAnimation->setDuration(150);
    m_hideAnimation->setStartValue(1.0);
    m_hideAnimation->setEndValue(0.0);
    m_hideAnimation->setEasingCurve(QEasingCurve::InQuad);
    
    connect(m_hideAnimation, &QPropertyAnimation::finished, this, &QWidget::hide);
}

void ThemePopover::showPopover(const QPoint &position)
{
    if (m_isVisible) {
        return;
    }
    
    m_isVisible = true;
    positionPopover(position);
    show();
    raise();
    activateWindow();
    
    // Aplicar sombra
    setGraphicsEffect(m_shadowEffect);
    
    m_showAnimation->start();
}

void ThemePopover::hidePopover()
{
    if (!m_isVisible) {
        return;
    }
    
    m_isVisible = false;
    m_hideAnimation->start();
}

void ThemePopover::positionPopover(const QPoint &anchorPoint)
{
    // Posicionar el popover debajo y ligeramente a la izquierda del ícono
    QPoint popoverPos = anchorPoint;
    popoverPos.setX(popoverPos.x() - width() + 32); // Alinear con el borde derecho del ícono
    popoverPos.setY(popoverPos.y() + 8); // Pequeño espaciado debajo del ícono
    
    // Asegurar que esté dentro de la pantalla
    QRect screenGeometry = qApp->primaryScreen()->availableGeometry();
    if (popoverPos.x() + width() > screenGeometry.right()) {
        popoverPos.setX(screenGeometry.right() - width() - 10);
    }
    if (popoverPos.x() < screenGeometry.left()) {
        popoverPos.setX(screenGeometry.left() + 10);
    }
    if (popoverPos.y() + height() > screenGeometry.bottom()) {
        popoverPos.setY(anchorPoint.y() - height() - 8); // Mostrar arriba del ícono
    }
    
    move(popoverPos);
}

void ThemePopover::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fondo del popover
    QPainterPath path;
    path.addRoundedRect(rect().adjusted(4, 4, -4, -4), 12, 12);
    
    // Color de fondo según el tema
    bool isDark = ThemeManager::instance().isDark();
    QColor backgroundColor = isDark ? QColor("#1C1C20") : QColor("#FFFFFF");
    QColor borderColor = isDark ? QColor("#2A2A2E") : QColor("#E5E7EB");
    
    painter.fillPath(path, backgroundColor);
    painter.setPen(QPen(borderColor, 1));
    painter.drawPath(path);
}

void ThemePopover::onThemeRadioToggled()
{
    QRadioButton *sender = qobject_cast<QRadioButton*>(this->sender());
    if (!sender || !sender->isChecked()) {
        return;
    }
    
    ThemeManager::Theme newTheme = (sender == m_lightRadio) ? 
        ThemeManager::Theme::Light : ThemeManager::Theme::Dark;
    
    ThemeManager::instance().setTheme(newTheme);
    
    // Ocultar popover después de seleccionar
    QTimer::singleShot(200, this, &ThemePopover::hidePopover);
}

void ThemePopover::onThemeChanged(ThemeManager::Theme theme)
{
    updateThemeSelection();
    update(); // Repintar con el nuevo tema
}

void ThemePopover::updateThemeSelection()
{
    bool isDark = ThemeManager::instance().isDark();
    
    // Bloquear señales temporalmente para evitar loops
    m_lightRadio->blockSignals(true);
    m_darkRadio->blockSignals(true);
    
    m_lightRadio->setChecked(!isDark);
    m_darkRadio->setChecked(isDark);
    
    m_lightRadio->blockSignals(false);
    m_darkRadio->blockSignals(false);
}

bool ThemePopover::eventFilter(QObject *object, QEvent *event)
{
    if (m_isVisible && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPos();
        
        // Si se hace clic fuera del popover, ocultarlo
        if (!geometry().contains(globalPos)) {
            hidePopover();
        }
    }
    
    return QWidget::eventFilter(object, event);
}

void ThemePopover::setSettingsButton(QPushButton* button) {
    m_settingsButton = button;
}

bool ThemePopover::isPopoverVisible() const {
    return m_isVisible && isVisible();
}
