#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QString>

class DBOperations
{
public:
	static void connectToDB();

	template <class T>
	static bool insertDeleteOrUpdateInTable(const QString& sqlQuery, std::initializer_list<T> listOfCols)
	{
		QSqlQuery query(QSqlDatabase::database());
		query.prepare(sqlQuery);
		for (auto& pair : listOfCols)
		{
			query.bindValue(pair.first, pair.second);
		}
		if (query.exec())
		{
			return true;
		}
		return false;
	}

	template <class T>
	static QSqlQuery selectFromTable(const QString& sqlQuery, std::initializer_list<T> listOfCols)
	{
		QSqlQuery query(QSqlDatabase::database());
		query.prepare(sqlQuery);
		for (auto& pair : listOfCols)
		{
			query.bindValue(pair.first, pair.second);
		}
		query.exec();
		return query;
	}
};