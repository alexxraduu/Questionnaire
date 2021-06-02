#include <QtWidgets/QApplication>
#include <QSqlQuery>
#include <fstream>

#include "LoginPanel.h"
#include "DBOperations.h"
#include "Utils.h"
#include "TestAttempt.h"
int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    LoginPanel w;
    w.show();
    DBOperations::connectToDB();
    Utils::logActivity("Started application!", Logger::Level::Info);
    return a.exec();
}