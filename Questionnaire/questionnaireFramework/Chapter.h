#pragma once

#include <iostream>
#include <QString>

#include "Domain.h"

class Chapter
{
public:
    Chapter(uint16_t id, const QString& name, Domain domain);
    Chapter();
    bool save() const;
    void remove() const;
    uint16_t getId() const;
    QString getName() const;
    Domain getDomain() const;

private:
    uint16_t m_id;
    QString m_name;
    Domain m_domain;
};

