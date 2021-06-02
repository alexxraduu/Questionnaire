#include "DBOperations.h"
#include "Utils.h"

void DBOperations::connectToDB()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("questionnaire.go.ro");
	db.setDatabaseName("questionnaire");
	db.setUserName("dev");
	db.setPassword("1q2w3e");
	Utils::logActivity("Successfully connected to database!", Logger::Level::Info);
}