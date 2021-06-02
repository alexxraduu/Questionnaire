#include "Domain.h"
#include "DBOperations.h"

Domain::Domain(uint16_t id, const QString& name) :
	m_id(id),
	m_name(name)
{
}

Domain::Domain(const QString& name) :
	m_id(NULL),
	m_name(name)
{
}

Domain::Domain() :
	m_id(0),
	m_name("")
{
}

bool Domain::save() const
{
	QString sqlQuery = "INSERT INTO domain (name) VALUES (:name)";
	std::pair<QString, QString> pairName(":name", m_name);
	return DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairName });
}

void Domain::remove() const
{
	//delete domain
	QString sqlQuery = "DELETE FROM domain WHERE id = :id";
	std::pair<QString, QString> pairId(":id", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairId });

	//delete chapters of domains
	sqlQuery = "DELETE FROM chapter WHERE domainId = :domainId";
	std::pair<QString, QString> pairDomainId(":domainId", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairDomainId });
}

uint16_t Domain::getId() const
{
	return m_id;
}

QString Domain::getName() const
{
	return m_name;
}

int Domain::numberOfQuestions()
{
	QString sqlQuery = "SELECT * FROM chapter c,question q WHERE c.domainId= :domainId AND q.chapterId=c.id";
	std::pair<QString, QString> pairDomainId(":domainId", QString::number(m_id));
	QSqlQuery query = DBOperations::selectFromTable(sqlQuery, {pairDomainId});
	int count = 0;
	while (query.next())
	{
		++count;
	}
	return count;
}
