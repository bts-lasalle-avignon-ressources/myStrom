#include "MyStromSwitch.h"
#include <QApplication>

/**
 * @file main.cpp
 * @brief Programme principal
 * @details Crée et affiche la fenêtre principale de l'application
 *
 * @param argc
 * @param argv[]
 * @return int
 */
int main(int argc, char* argv[])
{
    QApplication  a(argc, argv);
    MyStromSwitch myStromSwitch;

    myStromSwitch.show();

    return a.exec();
}
