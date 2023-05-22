/**
 * @mainpage Projekt ICP
 * @link
 * main.cpp
 * @endlink
 *
 * @file main.c
 * @brief Projekt ICP - Simulacia liniek verejnej dopravy
 * @author Pavol Dubovec, xdubov02
 * @author Juraj Lazur, xlazur00
 * @version 1.0
 */

#include "mainwindow.h"
#include <QApplication>

// Funkcia Main pre cely program
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
