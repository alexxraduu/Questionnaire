#include "Chapter.h"
#include "DBOperations.h"

Chapter::Chapter(uint16_t id, const QString& name, Domain domain) :
    m_id(id),
    m_name(name),
    m_domain(domain)
{
}

Chapter::Chapter():
    m_id(0),
    m_name(""),
    m_domain(Domain())
{
}

bool Chapter::save() const
{
    QString sqlQuery = "INSERT INTO chapter (domainId, name) VALUES (:domainId, :name)";
    std::pair<QString, QString> pairDomainId(":domainId", QString::number(m_domain.getId()));
    std::pair<QString, QString> pairName(":name", m_name);
    return DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairName, pairDomainId });
}

void Chapter::remove() const
{
    QString sqlQuery = "DELETE FROM chapter WHERE id = :id";
    std::pair<QString, QString> pairId(":id", QString::number(m_id));
    DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairId });
}

uint16_t Chapter::getId() const
{
    return m_id;
}

QString Chapter::getName() const
{
    return m_name;
}

Domain Chapter::getDomain() const
{
    return m_domain;
}
