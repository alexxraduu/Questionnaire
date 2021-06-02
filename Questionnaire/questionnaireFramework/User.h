#pragma once

#include <iostream>
#include <QString>

#include "DBOperations.h"

class User
{
public:
    enum class userType
    {
        Administrator,
        Teacher,
        Candidate
    };

public:
    User(uint16_t id, const QString& email, const QString& firstName, const QString& lastName, userType type);
    User(const QString& email, const QString& firstName, const QString& lastName, userType type);
    User() =default;
    uint16_t getId() const;
    QString getEmail() const;
    QString getFirstName() const;
    QString getLastName() const;
    userType getType() const;
    QString getTypeAsString() const;
    void remove() const;
    void addToDB() const;

private:
    uint16_t m_id;
    QString m_email;
    QString m_firstName;
    QString m_lastName;
    userType m_type;
};

