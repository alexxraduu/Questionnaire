#include "User.h"

User::User(uint16_t id, const QString& email, const QString& firstName, const QString& lastName, userType type) :
    m_id(id),
    m_email(email),
    m_firstName(firstName),
    m_lastName(lastName),
    m_type(type)
{
}

User::User(const QString& email, const QString& firstName, const QString& lastName, userType type) :
    m_email(email),
    m_firstName(firstName),
    m_lastName(lastName),
    m_type(type)
{
}

uint16_t User::getId() const
{
    return m_id;
}

QString User::getEmail() const
{
    return m_email;
}

QString User::getFirstName() const
{
    return m_firstName;
}

QString User::getLastName() const
{
    return m_lastName;
}

User::userType User::getType() const
{
    return m_type;
}

QString User::getTypeAsString() const
{
    switch (m_type) 
    {
    case userType::Administrator:
        return "administrator";
    case userType::Teacher:
        return "teacher";
    case userType::Candidate:
        return "candidate";
    }
}

void User::remove() const
{
    QString sqlQuery = "DELETE FROM user WHERE id = :id";
    std::pair<QString, QString> pairId(":id", QString::number(m_id));
    DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairId });
}

void User::addToDB() const
{
    QString sqlQuery = "INSERT INTO user (emailAdress, password, firstName, lastName, type) VALUES (:email, 'user', :firstName, :lastName, :type)";
    std::pair<QString, QString> pairEmail(":email", m_email);
    std::pair<QString, QString> pairFirstName(":firstName", m_firstName);
    std::pair<QString, QString> pairLastName(":lastName", m_lastName);
    std::pair<QString, QString> pairType(":type", getTypeAsString());
    DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairEmail, pairFirstName, pairLastName, pairType});
}
