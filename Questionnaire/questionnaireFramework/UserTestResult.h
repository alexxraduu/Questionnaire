#pragma once

#include <iostream>
#include <QDateTime>

#include "User.h"
#include "Test.h"

class UserTestResult
{
public:
	UserTestResult(uint16_t id, User candidate, uint16_t testId, QDateTime startTime, QDateTime endTime, double mark);
	uint16_t getId() const;
	User getUser() const;
	uint16_t getTestId() const;
	QDateTime getStartTime() const;
	QDateTime getEndTime() const;
	double getMark() const;

private:
	uint16_t m_id;
	User m_candidate;
	uint16_t m_testId;
	QDateTime m_startTime;
	QDateTime m_endTime;
	double m_mark;
};

