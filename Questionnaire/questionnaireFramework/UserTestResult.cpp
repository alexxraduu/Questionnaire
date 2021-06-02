#include "UserTestResult.h"

UserTestResult::UserTestResult(uint16_t id, User candidate,uint16_t testId, QDateTime startTime, QDateTime endTime, double mark):
	m_id(id),
	m_candidate(candidate),
	m_testId(testId),
	m_startTime(startTime),
	m_endTime(endTime),
	m_mark(mark)
{
}

uint16_t UserTestResult::getId() const
{
	return m_id;
}

User UserTestResult::getUser() const
{
	return m_candidate;
}

uint16_t UserTestResult::getTestId() const
{
	return m_testId;
}

QDateTime UserTestResult::getStartTime() const
{
	return m_startTime;
}

QDateTime UserTestResult::getEndTime() const
{
	return m_endTime;
}

double UserTestResult::getMark() const
{
	return m_mark;
}
