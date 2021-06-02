#include <QDebug>
#include <QString>
#include <QDebug>
#include <QShortcut>

#include "TestsResults.h"
#include "Utils.h"
#include "TestAttempt.h"

TestsResults::TestsResults(std::shared_ptr<User> connectedUser, QWidget* parent) :
	m_connectedUser(connectedUser),
	QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.testTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onTestCellClicked(int, int)));
	connect(ui.candidateResultTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCandidateResultCellClicked(int, int)));

	// search candidate results
	QShortcut* enterShortcut = new QShortcut(QKeySequence("Return"), this);
	connect(enterShortcut, SIGNAL(activated()), this, SLOT(onSearchCandidate()));

	populateTestTable();
}

void TestsResults::getTestsList()
{
	m_listOfTests.clear();

	QString sqlQuery = "SELECT * FROM test t, domain d WHERE t.teacherId= :teacherId AND t.domainId=d.id";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":teacherId", m_connectedUser->getId());
	query.exec();
	while (query.next())
	{
		// domain
		int domainId = query.value("d.id").toInt();
		QString domainName = query.value("d.name").toString();
		Domain domain(domainId, domainName);

		// test
		int testId = query.value("t.id").toInt();
		int testAvailableTime = query.value("t.availableTime").toInt();
		QDateTime testStartTime = query.value("t.startTime").toDateTime();
		QDateTime testEndTime = query.value("t.endTime").toDateTime();
		int testNumberOfQuestions = query.value("t.numberOfQuestions").toInt();
		Test test(testId, m_connectedUser->getId(), testAvailableTime, testNumberOfQuestions, testStartTime, testEndTime, domain);

		m_listOfTests.push_back(test);
	}
}

void TestsResults::populateTestTable()
{
	getTestsList();

	// remove previous elements
	ui.testTableWidget->reset();
	ui.testTableWidget->setRowCount(0);

	ui.testTableWidget->setRowCount(m_listOfTests.size());
	ui.testTableWidget->setColumnCount(5);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	ui.testTableWidget->setColumnHidden(0, true); // hide id column
	ui.testTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "domain" << "questions" << "start time" << "end time";
	ui.testTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfTests.begin(), m_listOfTests.end(), [&](Test& test)
		{
			ui.testTableWidget->setSortingEnabled(false);
			ui.testTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(test.getId()))); // id column
			ui.testTableWidget->setItem(rowCount, 1, new QTableWidgetItem(test.getDomain().getName())); // domain column
			ui.testTableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::number(test.getNumberOfQuestions()))); // number of questions column
			ui.testTableWidget->setItem(rowCount, 3, new QTableWidgetItem(test.getStartTime().toString("ddd MMMM dd yyyy | hh:mm "))); // start time column
			ui.testTableWidget->setItem(rowCount, 4, new QTableWidgetItem(test.getEndTime().toString("ddd MMMM dd yyyy | hh:mm "))); // end time column
			ui.testTableWidget->setSortingEnabled(true);
			rowCount++;
		});
}

void TestsResults::getCandidatesResultsList()
{
	m_listOfCandidatesResults.clear(); // clear the list first

	QString sqlQuery = "SELECT * FROM usertestresult uts, user u WHERE (uts.userId = u.id AND testId = :testId) AND (u.firstName LIKE :searchedValue OR u.lastName LIKE :searchedValue OR uts.mark LIKE :searchedValue OR u.emailAdress LIKE :searchedValue)";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":testId", m_selectedTest.getId());
	query.bindValue(":searchedValue", QString("%%1%").arg(ui.searchCandidateInput->text()));
	query.exec();
	while (query.next())
	{
		// candidate(user)
		int candidateId = query.value("u.id").toInt();
		QString candidateEmail = query.value("u.emailAdress").toString();
		QString candidateFirstName = query.value("u.firstName").toString();
		QString candidateLastName = query.value("u.lastName").toString();
		User candidate(candidateId, candidateEmail, candidateFirstName, candidateLastName, User::userType::Candidate);

		// user test result
		int resultId = query.value("uts.id").toInt();
		int resultTestId = query.value("uts.testId").toInt();
		QDateTime resultStartTime = query.value("uts.startTime").toDateTime();
		QDateTime resultEndTime = query.value("uts.endTime").toDateTime();
		double resultMark = query.value("uts.mark").toDouble();
		UserTestResult candidateResult(resultId, candidate, resultTestId, resultStartTime, resultEndTime, resultMark);

		m_listOfCandidatesResults.push_back(candidateResult);
	}
}

void TestsResults::populateCandidatesResultsTable()
{
	getCandidatesResultsList();

	// delete all the previous elements
	ui.candidateResultTableWidget->reset();
	ui.candidateResultTableWidget->setRowCount(0);

	ui.candidateResultTableWidget->setRowCount(m_listOfCandidatesResults.size());
	ui.candidateResultTableWidget->setColumnCount(4);
	ui.candidateResultTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.candidateResultTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.candidateResultTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.candidateResultTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.candidateResultTableWidget->setColumnHidden(0, true); // hide id column
	ui.candidateResultTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "mark" << "name" << "e-mail address";
	ui.candidateResultTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfCandidatesResults.begin(), m_listOfCandidatesResults.end(), [&](UserTestResult& candidateResult)
		{
			ui.candidateResultTableWidget->setSortingEnabled(false);
			ui.candidateResultTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(candidateResult.getId()))); // id 
			ui.candidateResultTableWidget->setItem(rowCount, 1, new QTableWidgetItem(candidateResult.getMark() == 0?"in progress...":QString::number(candidateResult.getMark()))); // mark
			ui.candidateResultTableWidget->setItem(rowCount, 2, new QTableWidgetItem(candidateResult.getUser().getLastName() + " " + candidateResult.getUser().getFirstName())); // candidate name
			ui.candidateResultTableWidget->setItem(rowCount, 3, new QTableWidgetItem(candidateResult.getUser().getEmail())); // candidate email 

			const double kminMark = 4.5;
			if (candidateResult.getMark() == 0)
			{
				ui.candidateResultTableWidget->item(rowCount, 0)->setBackground(Qt::cyan);
				ui.candidateResultTableWidget->item(rowCount, 1)->setBackground(Qt::cyan);
				ui.candidateResultTableWidget->item(rowCount, 2)->setBackground(Qt::cyan);
				ui.candidateResultTableWidget->item(rowCount, 3)->setBackground(Qt::cyan);
			}
			else if (candidateResult.getMark() < kminMark)
			{
				ui.candidateResultTableWidget->item(rowCount, 0)->setBackground(Qt::red);
				ui.candidateResultTableWidget->item(rowCount, 1)->setBackground(Qt::red);
				ui.candidateResultTableWidget->item(rowCount, 2)->setBackground(Qt::red);
				ui.candidateResultTableWidget->item(rowCount, 3)->setBackground(Qt::red);
			}
			else if (candidateResult.getMark() >= kminMark)
			{
				ui.candidateResultTableWidget->item(rowCount, 0)->setBackground(Qt::green);
				ui.candidateResultTableWidget->item(rowCount, 1)->setBackground(Qt::green);
				ui.candidateResultTableWidget->item(rowCount, 2)->setBackground(Qt::green);
				ui.candidateResultTableWidget->item(rowCount, 3)->setBackground(Qt::green);
			}

			ui.candidateResultTableWidget->setSortingEnabled(true);
			rowCount++;
		});
}

void TestsResults::onTestCellClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.testTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedTest = std::move(Utils::findItemById(tableItem->text().toInt(), m_listOfTests));
	populateCandidatesResultsTable();
}

void getAllQuestions(const UserTestResult& utr, QVector<Question>& allQuestions, QVector<Question>& candidateResultsAnswers)
{
	QString sqlQuery = "SELECT * FROM usertestresultanswer utra, usertestresult utr, question q WHERE (utra.idUserTestResult = utr.id AND utra.questionId = q.id AND utr.id = :userTestResultId)";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":userTestResultId", utr.getId());
	query.exec();
	while (query.next())
	{
		// actual answers
		int questionId = query.value("q.id").toInt();
		int questionTeacherId = query.value("q.teacherId").toInt();
		QString questionTitle = query.value("q.title").toString();
		QString questionText = query.value("q.text").toString();
		int questionTypeValue = query.value("q.type").toInt();
		QString questionAnswer = query.value("q.answer").toString();
		Question::questionType questionType;
		switch (questionTypeValue)
		{
		case 0:
			questionType = Question::questionType::SingleAnswer;
			break;
		case 1:
			questionType = Question::questionType::MultipleAnswer;
			break;
		}

		Question question(questionId, questionTeacherId, Chapter(), questionTitle, questionText, questionType, questionAnswer);
		allQuestions.push_back(question);


		// candidate answers
		QString answerListAsString = query.value("utra.answer").toString();

		switch (questionType)
		{
		case Question::questionType::SingleAnswer:
		{
			Question questionCandidateShort(questionId, answerListAsString);
			candidateResultsAnswers.push_back(questionCandidateShort);
			break;
		}
		case Question::questionType::MultipleAnswer:
		{
			QStringList list = answerListAsString.split(QRegExp("\\s"));
			QVector<int> indexes;
			for (auto& answerIndex : list) {
				if (answerIndex != " " && answerIndex != "")
				{
					indexes.push_back(answerIndex.toInt() - 1);
				}
			}
			std::vector<Answer> listOfAnswers;
			for (int index = 0; index < question.getAnswers().size(); ++index)
			{
				Answer currentAnswer(question.getAnswers().at(index).getId(), question.getAnswers().at(index).getText(), indexes.contains(index) ? 1 : 0);
				listOfAnswers.push_back(currentAnswer);
			}
			Question questionCandidateMultiple(questionId, listOfAnswers);
			candidateResultsAnswers.push_back(questionCandidateMultiple);
			break;
		}
		}
	}
}

void TestsResults::onCandidateResultCellClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.candidateResultTableWidget->item(row, 0); // get id from column 0 of table
	UserTestResult candidateResult = std::move(Utils::findItemById(tableItem->text().toInt(), m_listOfCandidatesResults));
	std::shared_ptr<Test> selectedTest = std::make_shared<Test>(m_selectedTest);

	QVector<Question> allQuestions;
	QVector<Question> candidateResultsAnswers;
	getAllQuestions(candidateResult, allQuestions, candidateResultsAnswers);

	TestAttempt* attempt = new TestAttempt(allQuestions, candidateResultsAnswers, selectedTest);
	attempt->show();

}

void TestsResults::onSearchCandidate()
{
	populateCandidatesResultsTable();
}

TestsResults::~TestsResults()
{
}