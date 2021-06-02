#pragma once

#include <iostream>
#include <QString>

class Domain
{
public:
	Domain(uint16_t id, const QString& name);
	Domain(const QString& name);
	Domain();
	bool save() const;
	void remove() const;
	uint16_t getId() const;
	QString getName() const;
	int numberOfQuestions();

private:
	uint16_t m_id;
	QString m_name;
};