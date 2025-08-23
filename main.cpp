#include "mainwindow.h"
#include "splashscreen.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Mostrar pantalla de bienvenida
    SplashScreen *splash = new SplashScreen();
    splash->show();
    
    return a.exec();
}
