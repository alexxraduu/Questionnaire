#include <QMessageBox>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QCloseEvent>

#include "TestAttempt.h"
#include "Utils.h"

// attempt test
TestAttempt::TestAttempt(std::shared_ptr<User> connectedCandidate, QVector<Question> listOfQuestions, std::shared_ptr<Test> selectedTest, QWidget* parent) :
	m_connectedCandidate(connectedCandidate),
	m_listOfQuestions(listOfQuestions),
	m_selectedTest(selectedTest),
	m_currentQuestionIndex(0),
	m_numberOfCurrentMultipleAnswers(0),
	m_frameMode(mode::attempt),
	QWidget(parent)
{
	ui.setupUi(this);

	// Cheating timer
	counterForCheating = kMaxTimeOutsideAppAllowed;
	timerForCheating = new QTimer(this);
	connect(timerForCheating, SIGNAL(timeout()), this, SLOT(showTimeOutsideApp()));
	QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, [&](Qt::ApplicationState state)
		{
			if (counterForCheating > 0)
			{
				if (state == Qt::ApplicationInactive)
				{
					timerForCheating->start(1000);
				}
				else if (state == Qt::ApplicationActive)
				{
					timerForCheating->stop();
				}
			}
		});

	// Test timer
	counter = selectedTest->getAvailableTime() * 60;
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showRemainingTime()));
	timer->start(1000);

	// Next/Previous button connectors
	connect(ui.nextButton, SIGNAL(clicked()), SLOT(onNextButtonClicked()));
	connect(ui.previousButton, SIGNAL(clicked()), SLOT(onPreviousButtonClicked()));

	initializeUI();
}

// view question answers
TestAttempt::TestAttempt(QVector<Question> listOfCandidateAnswers, QVector<Question> listOfQuestions, std::shared_ptr<Test> selectedTest, QWidget* parent) :
	m_listOfCandidateAnswers(listOfQuestions),
	m_listOfQuestions(listOfCandidateAnswers),
	m_selectedTest(selectedTest),
	m_currentQuestionIndex(0),
	m_numberOfCurrentMultipleAnswers(0),
	m_frameMode(mode::view),
	QWidget(parent)
{
	ui.setupUi(this);

	// Next/Previous button connectors
	connect(ui.nextButton, SIGNAL(clicked()), SLOT(onNextButtonClicked()));
	connect(ui.previousButton, SIGNAL(clicked()), SLOT(onPreviousButtonClicked()));

	initializeUIForView();
}

void TestAttempt::closeEvent(QCloseEvent* event)
{
	if (m_frameMode == mode::attempt)
	{
		endTest();
	}
	else
	{
		this->close();
	}
}

void TestAttempt::showRemainingTime() {
	counter--;
	if (counter == 0)
	{
		endTest();
		QMessageBox::information(this, "", "Time is over!", QMessageBox::Button::Ok);
	}
	else if (m_selectedTest->getEndTime() < QDateTime::currentDateTime())
	{
		endTest();
	}
	ui.timeLeft->display(QDateTime::fromTime_t(counter).toUTC().toString("hh:mm:ss"));
	qDebug() << QString("Remaining time: %1 seconds").arg(counter);
}

void TestAttempt::showTimeOutsideApp()
{
	counterForCheating--;
	if (counterForCheating == 0)
	{
		cheatingDetected();
	}
	qDebug() << QString("Time outside the app: %1 seconds").arg(counterForCheating);
}

void TestAttempt::endTest()
{
	saveCandidateAnswers();
	calculateGrade();
	saveResultsToDB();

	counterForCheating = 0;
	timerForCheating->stop();
	timer->stop();
	this->close();
}

void TestAttempt::cheatingDetected()
{
	QMessageBox::critical(this, "", "You cheated!", QMessageBox::Button::Ok);
	endTest();
}

int numberOfCorrectAnswers(Question question)
{
	int count = 0;
	for (auto& answer : question.getAnswers())
	{
		if (answer.isCorrect())
		{
			count++;
		}
	}
	return count;
}

void TestAttempt::calculateGrade()
{
	double pointsPerQuestion = double(kTotalPoints) / double(m_listOfQuestions.size());
	double finalGrade = double(kStartPoint);

	for (int index = 0; index < m_listOfCandidateAnswers.size(); ++index)
	{
		auto questionPoints = [&]()->double
		{
			if (m_listOfQuestions[index].getType() == Question::questionType::MultipleAnswer)
			{
				double pointsPerAnswer = pointsPerQuestion / double(numberOfCorrectAnswers(m_listOfQuestions[index]));
				double partialGrade = 0;
				for (int index2 = 0; index2 < m_listOfCandidateAnswers[index].getAnswers().size(); ++index2)
				{
					if (m_listOfCandidateAnswers[index].getAnswers().at(index2).isCorrect() == true && m_listOfQuestions[index].getAnswers().at(index2).isCorrect() == true)
					{
						partialGrade += pointsPerAnswer;
					}
					else if (m_listOfCandidateAnswers[index].getAnswers().at(index2).isCorrect() == true && m_listOfQuestions[index].getAnswers().at(index2).isCorrect() == false)
					{
						return 0;
					}
				}
				return partialGrade;
			}
			else
			{
				QString answerText = m_listOfQuestions[index].getAnswerValue();
				QRegularExpression correct(answerText, QRegularExpression::CaseInsensitiveOption);
				QRegularExpressionMatch match = correct.match(m_listOfCandidateAnswers[index].getAnswerValue());

				if (match.hasMatch())
				{
					return pointsPerQuestion;
				}
				else
				{
					return 0;
				}
			}
		}();
		finalGrade += questionPoints;
	}
	m_mark = finalGrade;
}

void TestAttempt::saveResultsToDB()
{
	qDebug() << "Mark: " << m_mark;
	QString sqlQuery = "UPDATE usertestresult SET endTime= :endTime , mark = :mark WHERE testId = :testId AND userId= :candidateId";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":endTime", QDateTime::currentDateTime());
	query.bindValue(":mark", QString::number(m_mark, 'f', 2));
	query.bindValue(":testId", m_selectedTest->getId());
	query.bindValue(":candidateId", m_connectedCandidate->getId());
	query.exec();
}

void TestAttempt::saveQuestionAnswersToDB(const Question& question)
{

	QString answer;
	if (question.getType() == Question::questionType::SingleAnswer)
	{
		answer.clear();
		answer = question.getAnswerValue();
	}
	else
	{
		answer.clear();
		for (int index = 0; index < question.getAnswers().size(); ++index)
		{
			if (question.getAnswers().at(index).isCorrect())
			{
				answer += QString::number(index + 1);
				answer += " ";
			}
		}
	}
	QString sqlQuery = "UPDATE usertestresultanswer SET answer = :answer  WHERE idUserTestResult = :testAttemptId AND questionId= :questionId";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":answer", answer);
	query.bindValue(":testAttemptId", m_selectedTest->getTestAttemptId());
	query.bindValue(":questionId", question.getId());
	query.exec();
}

void TestAttempt::initializeUI()
{
	// Remove all the previous answer widgets
	ui.shortAnswerInput->setText("");
	for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
	{
		m_listOfAnswerWidgets[index]->setVisible(false);
		ui.questionBodyLayout->removeWidget(m_listOfAnswerWidgets[index]);
	}
	m_listOfAnswerWidgets.clear();

	// Next/Previous/Finish attempt buttons
	if (m_currentQuestionIndex == 0)
	{
		ui.previousButton->setVisible(false);
		ui.nextButton->setText("Next question");
	}
	else if (m_currentQuestionIndex == m_listOfQuestions.size() - 1)
	{
		ui.previousButton->setVisible(true);
		ui.nextButton->setText("Finish attempt!");
	}
	else
	{
		ui.previousButton->setVisible(true);
		ui.nextButton->setText("Next question");
	}

	ui.countLabel->setText(QString::number(m_currentQuestionIndex + 1) + "/" + QString::number(m_listOfQuestions.size())); // Show question #
	ui.textLabel->setText(m_listOfQuestions[m_currentQuestionIndex].getText()); // Show question text

	// Set answers widgets
	switch (m_listOfQuestions[m_currentQuestionIndex].getType()) {
	case Question::questionType::SingleAnswer:
		m_numberOfCurrentMultipleAnswers = 0;
		ui.shortAnswerInput->setVisible(true);
		ui.shortAnswerLabel->setVisible(true);
		break;
	case Question::questionType::MultipleAnswer:
		ui.shortAnswerInput->setVisible(false);
		ui.shortAnswerLabel->setVisible(false);
		m_numberOfCurrentMultipleAnswers = m_listOfQuestions[m_currentQuestionIndex].getAnswers().size();
		for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
		{
			m_listOfAnswerWidgets.push_back(new QCheckBox);
			ui.answersLayout->addWidget(m_listOfAnswerWidgets[index]);
			m_listOfAnswerWidgets[index]->setText(m_listOfQuestions[m_currentQuestionIndex].getAnswers().at(index).getText());
		}
		break;
	}

	if (m_listOfCandidateAnswers.indexOf(m_listOfQuestions[m_currentQuestionIndex]) != -1)
	{
		switch (m_listOfQuestions[m_currentQuestionIndex].getType()) {
		case Question::questionType::SingleAnswer:
			ui.shortAnswerInput->setText(m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswerValue());
			break;
		case Question::questionType::MultipleAnswer:
			for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
			{
				m_listOfAnswerWidgets[index]->setChecked(m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswers().at(index).isCorrect());
			}
			break;
		}
	}
}

void TestAttempt::initializeUIForView()
{
	ui.timeLeft->setVisible(false); // hide the timer

	// Remove all the previous answer widgets
	ui.shortAnswerInput->setText("");
	for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
	{
		m_listOfAnswerWidgets[index]->setVisible(false);
		ui.questionBodyLayout->removeWidget(m_listOfAnswerWidgets[index]);
	}
	m_listOfAnswerWidgets.clear();


	// Next/Previous/Close buttons
	if (m_currentQuestionIndex == 0)
	{
		ui.previousButton->setVisible(false);
		ui.nextButton->setText("Next question");
	}
	else if (m_currentQuestionIndex == m_listOfQuestions.size() - 1)
	{
		ui.previousButton->setVisible(true);
		ui.nextButton->setText("Close window");
	}
	else
	{
		ui.previousButton->setVisible(true);
		ui.nextButton->setText("Next question");
	}

	ui.countLabel->setText(QString::number(m_currentQuestionIndex + 1) + "/" + QString::number(m_listOfQuestions.size())); // Show question #
	ui.textLabel->setText(m_listOfQuestions[m_currentQuestionIndex].getText());

	// Set answers widgets
	switch (m_listOfQuestions[m_currentQuestionIndex].getType())
	{
	case Question::questionType::SingleAnswer:
		m_numberOfCurrentMultipleAnswers = 0;
		ui.shortAnswerInput->setVisible(true);
		ui.shortAnswerLabel->setVisible(true);
		ui.shortAnswerInput->setEnabled(false);
		break;
	case Question::questionType::MultipleAnswer:
		ui.shortAnswerInput->setVisible(false);
		ui.shortAnswerLabel->setVisible(false);
		m_numberOfCurrentMultipleAnswers = m_listOfQuestions[m_currentQuestionIndex].getAnswers().size();
		for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
		{
			m_listOfAnswerWidgets.push_back(new QCheckBox);
			ui.answersLayout->addWidget(m_listOfAnswerWidgets[index]);
			m_listOfAnswerWidgets[index]->setText(m_listOfQuestions[m_currentQuestionIndex].getAnswers().at(index).getText());
			m_listOfAnswerWidgets[index]->setEnabled(false);
		}
		break;
	}

	if (m_listOfCandidateAnswers.indexOf(m_listOfQuestions[m_currentQuestionIndex]) != -1)
	{
		switch (m_listOfQuestions[m_currentQuestionIndex].getType()) {
		case Question::questionType::SingleAnswer:
			ui.shortAnswerInput->setText(m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswerValue());
			if (m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswerValue() == m_listOfQuestions[m_currentQuestionIndex].getAnswerValue())
			{
				ui.shortAnswerInput->setStyleSheet("color: green");
			}
			else
			{
				ui.shortAnswerInput->setStyleSheet("color: red");
			}
			break;
		case Question::questionType::MultipleAnswer:
			for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
			{
				m_listOfAnswerWidgets[index]->setChecked(m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswers().at(index).isCorrect());
				if (m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswers().at(index).isCorrect() && m_listOfQuestions[m_currentQuestionIndex].getAnswers().at(index).isCorrect())
				{
					m_listOfAnswerWidgets[index]->setStyleSheet("color: green");
				}
				
				if (m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswers().at(index).isCorrect() && !m_listOfQuestions[m_currentQuestionIndex].getAnswers().at(index).isCorrect())
				{
					m_listOfAnswerWidgets[index]->setStyleSheet("color: red");
				}

				if (!m_listOfCandidateAnswers[m_currentQuestionIndex].getAnswers().at(index).isCorrect() && m_listOfQuestions[m_currentQuestionIndex].getAnswers().at(index).isCorrect())
				{
					m_listOfAnswerWidgets[index]->setStyleSheet("color: green");
				}
			}
			break;
		}
	}
}

void TestAttempt::saveCandidateAnswers()
{
	Question currentQuestion = m_listOfQuestions[m_currentQuestionIndex];

	if (m_listOfQuestions[m_currentQuestionIndex].getType() == Question::questionType::SingleAnswer) {
		QString candidateShortAnswer;

		candidateShortAnswer = ui.shortAnswerInput->text();
		Question shortQuestion(currentQuestion.getId(), candidateShortAnswer);
		int index = m_listOfCandidateAnswers.indexOf(shortQuestion);
		if (index != -1)
		{
			m_listOfCandidateAnswers.replace(index, shortQuestion);
		}
		else
		{
			m_listOfCandidateAnswers.push_back(shortQuestion);
		}
		saveQuestionAnswersToDB(shortQuestion);
	}
	else
	{
		std::vector<Answer> candidateAnswers;
		for (int index = 0; index < m_numberOfCurrentMultipleAnswers; ++index)
		{
			Answer currentAnswer(currentQuestion.getAnswers().at(index).getId(), currentQuestion.getAnswers().at(index).getText(), m_listOfAnswerWidgets[index]->isChecked() ? 1 : 0);
			candidateAnswers.push_back(currentAnswer);
		}

		Question multipleQuestion(currentQuestion.getId(), candidateAnswers);
		int index = m_listOfCandidateAnswers.indexOf(multipleQuestion);
		if (index != -1)
		{
			m_listOfCandidateAnswers.replace(index, multipleQuestion);
		}
		else
		{
			m_listOfCandidateAnswers.push_back(multipleQuestion);
		}
		saveQuestionAnswersToDB(multipleQuestion);
	}

}

void TestAttempt::onNextButtonClicked()
{
	if (m_currentQuestionIndex < m_listOfQuestions.size() - 1)
	{
		if (m_frameMode == mode::attempt)
		{
			saveCandidateAnswers();
		}
		m_currentQuestionIndex++;
		if (m_frameMode == mode::attempt)
		{
			initializeUI();
		}
		else
		{
			initializeUIForView();
		}
	}
	else
	{
		if (m_frameMode == mode::attempt)
		{
			endTest();
		}
		else
		{
			this->close();
		}
	}
}

void TestAttempt::onPreviousButtonClicked()
{
	if (m_frameMode == mode::attempt)
	{
		saveCandidateAnswers();
	}
	if (m_currentQuestionIndex > 0)
	{
		m_currentQuestionIndex--;
		if (m_frameMode == mode::attempt)
		{
			initializeUI();
		}
		else
		{
			initializeUIForView();
		}
	}
}

TestAttempt::~TestAttempt()
{
}