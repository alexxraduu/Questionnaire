#pragma once

#include <QDateTime>
#include <QString>

#include "Domain.h"
#include "Question.h"

class Test
{
public:
	Test(uint16_t teacherId, uint16_t availableTime, uint16_t numberOfQuestions, QDateTime startTime, QDateTime endTime, Domain domain);
	Test(uint16_t id, uint16_t teacherId, uint16_t availableTime, uint16_t numberOfQuestions, QDateTime startTime, QDateTime endTime, Domain domain);
	Test();

public:
	uint16_t getId() const;
	uint16_t getAvailableTime() const;
	uint16_t getNumberOfQuestions() const;
	QDateTime getStartTime() const;
	QDateTime getEndTime() const;
	uint16_t getTeacherId() const;
	Domain getDomain() const;
	uint16_t getTestAttemptId() const;
	void saveToDB();
	void remove() const;
	QVector<Question> generateRandomQuestionsForUser(int userId);

private:
	uint16_t m_id;
	uint16_t m_teacherId;
	uint16_t m_availableTime;
	uint16_t m_numberOfQuestions;
	QDateTime m_startTime;
	QDateTime m_endTime;
	Domain m_domain;
	uint16_t m_testAttemptId;
};

