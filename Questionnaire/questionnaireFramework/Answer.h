#pragma once

#include <iostream>
#include <QString>

class Answer
{
public:
	Answer(uint16_t id, const QString& text, bool isCorrect);
	Answer(const QString& text, bool isCorrect);
	uint16_t getId() const;
	QString getText() const;
	bool isCorrect() const;
	void saveToDB(int questionId) const;

private:
	uint16_t m_id;
	QString m_text;
	bool m_isCorrect;
};

