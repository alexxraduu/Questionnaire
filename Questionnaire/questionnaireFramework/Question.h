#pragma once

#include <QString>
#include <iostream>
#include <vector>

#include "Chapter.h"
#include "Answer.h"

class Question
{
public:
	enum class questionType
	{
		SingleAnswer = 0,
		MultipleAnswer = 1
	};

public:
	Question(uint16_t, uint16_t, Chapter, const QString&, const QString&, questionType, const QString&);
	Question(uint16_t id, std::vector<Answer> listOfAnswers);   //for Multiple Answers
	Question(uint16_t id, const QString& answer);   //for Short Answer
	Question(uint16_t);

	// getters
	uint16_t getId() const;
	uint16_t getTeacherId() const;
	Chapter getChapter() const;
	QString getTitle() const;
	QString getText() const;
	questionType getType() const;
	QString getTypeAsString() const;
	QString getAnswerValue() const;
	std::vector<Answer> getAnswers() const;

	// setters
	void setAnswerListFromDB();
	void setChapter(Chapter chapter);
	void setTitle(const QString& title);
	void setText(const QString& text);
	void setType(questionType type);
	void setAnswerValue(const QString& answerValue);
	void setAnswers(std::vector<Answer> answers);

	// operations
	void saveToDB();
	void removeFromDB() const;

	//operator
	bool operator==(const Question& question);

private:
	uint16_t m_id;
	uint16_t m_teacherId;
	Chapter m_chapter;
	QString m_title;
	QString m_text;
	questionType m_type;
	QString m_answerValue;
	std::vector<Answer> m_answers;
};

