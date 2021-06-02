#include "Answer.h"
#include "DBOperations.h"
#include "Utils.h"

Answer::Answer(uint16_t id, const QString& text, bool isCorrect) :
	m_id(id),
	m_text(text),
	m_isCorrect(isCorrect)
{
}

Answer::Answer(const QString& text, bool isCorrect) :
	m_id(0),
	m_text(text),
	m_isCorrect(isCorrect)
{
}

uint16_t Answer::getId() const
{
	return m_id;
}

QString Answer::getText() const
{
	return m_text;
}

bool Answer::isCorrect() const
{
	return m_isCorrect;
}

void Answer::saveToDB(int questionId) const
{
	QString sqlQuery = "INSERT INTO multiplechoicequestion (questionId, text, isCorrect) VALUES (:questionId, :text, :isCorrect)";
	std::pair<QString, QString> pairQuestionId(":questionId", QString::number(questionId));
	std::pair<QString, QString> pairText(":text", m_text);
	std::pair<QString, QString> pairIsCorrect(":isCorrect", QString::number((int)m_isCorrect));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairQuestionId, pairText, pairIsCorrect });
}
