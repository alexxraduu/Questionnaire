#include <QSqlQuery>

#include "Test.h"
#include "DBOperations.h"
#include "Chapter.h"
#include "Question.h"
#include "Utils.h"

Test::Test(uint16_t teacherId, uint16_t availableTime, uint16_t numberOfQuestions, QDateTime startTime, QDateTime endTime, Domain domain) :
	m_id(0),
	m_teacherId(teacherId),
	m_availableTime(availableTime),
	m_numberOfQuestions(numberOfQuestions),
	m_startTime(startTime),
	m_endTime(endTime),
	m_domain(domain)
{
}

Test::Test(uint16_t id, uint16_t teacherId, uint16_t availableTime, uint16_t numberOfQuestions, QDateTime startTime, QDateTime endTime, Domain domain) :
	m_id(id),
	m_teacherId(teacherId),
	m_availableTime(availableTime),
	m_numberOfQuestions(numberOfQuestions),
	m_startTime(startTime),
	m_endTime(endTime),
	m_domain(domain)
{
}

Test::Test():
	m_id(0),
	m_teacherId(0),
	m_availableTime(0),
	m_numberOfQuestions(0),
	m_startTime(),
	m_endTime(),
	m_domain("")
{
}

uint16_t Test::getId() const
{
	return m_id;
}

uint16_t Test::getAvailableTime() const
{
	return m_availableTime;
}

uint16_t Test::getNumberOfQuestions() const
{
	return m_numberOfQuestions;
}

QDateTime Test::getStartTime() const
{
	return m_startTime;
}

QDateTime Test::getEndTime() const
{
	return m_endTime;
}

uint16_t Test::getTeacherId() const
{
	return m_teacherId;
}

Domain Test::getDomain() const
{
	return m_domain;
}

uint16_t Test::getTestAttemptId() const
{
	return m_testAttemptId;
}

void Test::saveToDB()
{
	QString sqlQuery = "INSERT INTO test (teacherId, availableTime, startTime, endTime, numberOfQuestions, domainId) VALUES (:teacherId, :availableTime, :startTime, :endTime, :numberOfQuestions, :domainId);";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":teacherId", QString::number(m_teacherId));
	query.bindValue(":availableTime", QString::number(m_availableTime));
	query.bindValue(":startTime", m_startTime);
	query.bindValue(":endTime", m_endTime);
	query.bindValue(":endTime", m_endTime);
	query.bindValue(":numberOfQuestions", QString::number(m_numberOfQuestions));
	query.bindValue(":domainId", QString::number(m_domain.getId()));
	query.exec();
}

void Test::remove() const
{
	QString sqlQuery = "DELETE FROM test WHERE id = :testId";
	std::pair<QString, QString> pairId(":testId", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairId });
}

QVector<Question> Test::generateRandomQuestionsForUser(int candidateId)
{
	QString sqlQuery = "INSERT INTO usertestresult (userId, testId, startTime) VALUES (:candidateId, :testId, :startTime);";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":candidateId", QString::number(candidateId));
	query.bindValue(":testId", QString::number(m_id));
	query.bindValue(":startTime", QDateTime::currentDateTime());
	query.exec();
	m_testAttemptId= query.lastInsertId().toInt();


	QVector<Chapter> listOfRandomChapters;
	QVector<Question> listOfRandomQuestions;

	QString sql = "SELECT * from chapter where domainId = :domainId ORDER BY RAND()";
	std::pair<QString, QString> pairDomainId(":domainId", QString::number(m_domain.getId()));
	query = DBOperations::selectFromTable(sql, { pairDomainId });
	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Chapter chapter(id, name, m_domain);
		listOfRandomChapters.push_back(chapter);
	}

	for (auto& chapter : listOfRandomChapters)
	{
		QString sql = "SELECT * from question where chapterId = :chapterId ORDER BY RAND()";
		std::pair<QString, QString> pairChapterId(":chapterId", QString::number(chapter.getId()));
		QSqlQuery query = DBOperations::selectFromTable(sql, { pairChapterId });
		while (query.next())
		{
			int question_id = query.value("id").toInt();
			QString question_title = query.value("title").toString();
			QString question_text = query.value("text").toString();
			int question_typeValue = query.value("type").toInt();
			QString question_answer = query.value("answer").toString();
			Question::questionType question_type;
			switch (question_typeValue)
			{
			case 0:
				question_type = Question::questionType::SingleAnswer;
				break;
			case 1:
				question_type = Question::questionType::MultipleAnswer;
				break;
			}

			Question question(question_id, m_teacherId, chapter, question_title, question_text, question_type, question_answer);
			listOfRandomQuestions.push_back(question);
		}
	}

	QVector<Question> listOfSelectedQuestions;

	int index = 0;
	while (listOfSelectedQuestions.size() < m_numberOfQuestions)
	{
		if (index == listOfRandomChapters.size())
		{
			index = 0;
		}

		int currentChapterId = listOfRandomChapters[index].getId();

		for (auto& question : listOfRandomQuestions)
		{
			if (question.getChapter().getId() == currentChapterId && listOfSelectedQuestions.indexOf(question) == -1)
			{
				listOfSelectedQuestions.push_back(question);
				break;
			}
		}

		index++;
	}

	for (auto& question : listOfSelectedQuestions)
	{
		QString sqlQuery = "INSERT INTO usertestresultanswer (idUserTestResult, questionId) VALUES (:testId, :questionId)";
		QSqlQuery query(QSqlDatabase::database());
		query.prepare(sqlQuery);
		query.bindValue(":testId", m_testAttemptId);
		query.bindValue(":questionId", question.getId());
		query.exec();
		
	}

	return listOfSelectedQuestions;
}
