#include <QCoreApplication>
#include "Backend.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

   Backend backend;

    return QCoreApplication::exec();
}
