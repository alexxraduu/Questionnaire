#pragma once

#include <QWidget>
#include <QVector>
#include <QCheckBox>

#include "ui_TestAttempt.h"
#include "Question.h"
#include "User.h"
#include "Test.h"

class TestAttempt : public QWidget
{
	Q_OBJECT

	enum class mode
	{
		view,
		attempt
	};

public:
	TestAttempt(std::shared_ptr<User> connectedCandidate, QVector<Question> listOfQuestions, std::shared_ptr<Test> selectedTest, QWidget *parent = Q_NULLPTR); // attempt
	TestAttempt(QVector<Question> listOfCandidateAnswers, QVector<Question> listOfQuestions, std::shared_ptr<Test> selectedTest, QWidget *parent = Q_NULLPTR); // view
	~TestAttempt();

public:
	const size_t kTotalPoints = 9;
	const size_t kStartPoint = 1;
	const size_t kMaxTimeOutsideAppAllowed = 15; // secs

private:
	Ui::TestAttempt ui;

	// time countdown
	QTimer* timer;
	qint32 counter;
	
	// time outside the app countdown
	QTimer* timerForCheating;
	qint32 counterForCheating;

	mode m_frameMode;
	std::shared_ptr<User> m_connectedCandidate;
	std::shared_ptr<Test> m_selectedTest;
	QVector<Question> m_listOfQuestions;
	QVector<Question> m_listOfCandidateAnswers;
	QVector<QCheckBox*> m_listOfAnswerWidgets;
	uint16_t m_currentQuestionIndex;
	uint16_t m_numberOfCurrentMultipleAnswers;
	uint16_t m_idUserTestAttempt;
	double m_mark;

private:
	void initializeUI();
	void initializeUIForView();
	void saveCandidateAnswers();
	void endTest();
	void cheatingDetected();
	void calculateGrade();
	void saveResultsToDB();
	void saveQuestionAnswersToDB(const Question& question);
	void closeEvent(QCloseEvent* event);

private slots:
	void onPreviousButtonClicked();
	void onNextButtonClicked();
	void showRemainingTime();
	void showTimeOutsideApp();
};
