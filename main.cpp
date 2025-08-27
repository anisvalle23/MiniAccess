#include "mainwindow.h"
#include "splashscreen.h"
#include "ThemeManager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Configurar información de la aplicación para QSettings
    QCoreApplication::setOrganizationName("MiniAccess");
    QCoreApplication::setApplicationName("MiniAccess");
    
    // Inicializar sistema de temas
    ThemeManager::instance().loadFromSettings();
    ThemeManager::instance().applyTheme();
    
    // Mostrar pantalla de bienvenida
    SplashScreen *splash = new SplashScreen();
    splash->show();
    
    return a.exec();
}
