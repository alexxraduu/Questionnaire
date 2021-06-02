#include "Question.h"
#include "DBOperations.h"
#include "Utils.h"

Question::Question(uint16_t id, uint16_t teacherId, Chapter chapter, const QString& title, const QString& text, questionType type, const QString& answer) :
	m_id(id),
	m_teacherId(teacherId),
	m_chapter(chapter),
	m_title(title),
	m_text(text),
	m_type(type),
	m_answerValue(answer)
{
	if (m_type == questionType::MultipleAnswer)
	{
		setAnswerListFromDB();
	}
}

Question::Question(uint16_t id, std::vector<Answer> listOfAnswers) :
	m_id(id),
	m_teacherId(0),
	m_title(""),
	m_text(""),
	m_type(questionType::MultipleAnswer),
	m_chapter(Chapter()),
	m_answerValue(""),
	m_answers(listOfAnswers)
{
}

Question::Question(uint16_t id, const QString& answer) :
	m_id(id),
	m_teacherId(0),
	m_title(""),
	m_text(""),
	m_type(questionType::SingleAnswer),
	m_chapter(Chapter()),
	m_answerValue(answer),
	m_answers()
{
}

Question::Question(uint16_t teacherId) :
	m_id(0),
	m_teacherId(teacherId),
	m_title(""),
	m_text(""),
	m_type(questionType::MultipleAnswer),
	m_chapter(Chapter()),
	m_answerValue("")
{
}

void Question::setAnswerListFromDB()
{
	QString sql = "SELECT * from multiplechoicequestion where questionId = :questionId";
	std::pair<QString, QString> pairId(":questionId", QString::number(m_id));

	QSqlQuery query = DBOperations::selectFromTable(sql, { pairId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString text = query.value("text").toString();
		bool isCorrect = query.value("isCorrect").toBool();

		Answer answer(id, text, isCorrect);
		m_answers.push_back(answer);
	}
}

void Question::setChapter(Chapter chapter)
{
	m_chapter = chapter;
}

void Question::setTitle(const QString& title)
{
	m_title = title;
}

void Question::setText(const QString& text)
{
	m_text = text;
}

void Question::setType(questionType type)
{
	m_type = type;
}

void Question::setAnswerValue(const QString& answerValue)
{
	m_answerValue = answerValue;
}

void Question::setAnswers(std::vector<Answer> answers)
{
	// delete existing answers first
	QString sqlQuery = "DELETE FROM multiplechoicequestion WHERE questionId = :questionId";
	std::pair<QString, QString> pairQuestionId(":questionId", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairQuestionId });

	m_answers = answers;
}

void Question::saveToDB()
{
	if (m_id == 0) // new question
	{
		QString sqlQuery = "INSERT INTO question (teacherId, chapterId, title, text, type, answer) VALUES (:teacherId, :chapterId, :title, :text, :type, :answer);";
		QSqlQuery query(QSqlDatabase::database());
		query.prepare(sqlQuery);
		query.bindValue(":teacherId", QString::number(m_teacherId));
		query.bindValue(":chapterId", QString::number(m_chapter.getId()));
		query.bindValue(":title", m_title);
		query.bindValue(":text", m_text);
		query.bindValue(":type", QString::number((int)m_type));
		query.bindValue(":answer", m_answerValue);
		query.exec();

		// update question id
		m_id = query.lastInsertId().toInt();
	}
	else // existing question
	{
		QString sqlQuery = "UPDATE question SET teacherId = :teacherId, chapterId = :chapterId, title = :title, text = :text, type = :type, answer = :answer where id = :id";
		std::pair<QString, QString> pairQuestionId(":id", QString::number(m_id));
		std::pair<QString, QString> pairTeacher(":teacherId", QString::number(m_teacherId));
		std::pair<QString, QString> pairChapter(":chapterId", QString::number(m_chapter.getId()));
		std::pair<QString, QString> pairTitle(":title", m_title);
		std::pair<QString, QString> pairText(":text", m_text);
		std::pair<QString, QString> pairType(":type", QString::number((int)m_type));
		std::pair<QString, QString> pairAnswer(":answer", m_answerValue);

		DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairQuestionId, pairTeacher, pairChapter, pairTitle, pairText, pairType, pairAnswer });
	}

	for (auto& answer : m_answers) 
	{
		answer.saveToDB(m_id);
	}

}

uint16_t Question::getId() const
{
	return m_id;
}

uint16_t Question::getTeacherId() const
{
	return m_teacherId;
}

Chapter Question::getChapter() const
{
	return m_chapter;
}

QString Question::getTitle() const
{
	return m_title;
}

QString Question::getText() const
{
	return m_text;
}

Question::questionType Question::getType() const
{
	return m_type;
}

QString Question::getTypeAsString() const
{
	switch (m_type)
	{
	case questionType::SingleAnswer:
		return "single answer";
	case questionType::MultipleAnswer:
		return "multiple answers";
	}
}

QString Question::getAnswerValue() const
{
	return m_answerValue;
}

std::vector<Answer> Question::getAnswers() const
{
	return m_answers;
}

void Question::removeFromDB() const
{
	// delete the question
	QString sqlQuery = "DELETE FROM question WHERE id = :id";
	std::pair<QString, QString> pairId(":id", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairId });
	
	// delete question answers
	sqlQuery = "DELETE FROM multiplechoicequestion WHERE questionId = :questionId";
	std::pair<QString, QString> pairQuestionId(":questionId", QString::number(m_id));
	DBOperations::insertDeleteOrUpdateInTable(sqlQuery, { pairQuestionId });

}

bool Question::operator==(const Question& question) 
{
	return (m_id == question.getId());
}
