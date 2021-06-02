#include <QShortcut>

#include "UserPanel.h"
#include "LoginPanel.h"
#include "QuestionBank.h"
#include "ManageUsers.h"
#include "ManageTest.h"
#include "Utils.h"
#include "TestAttempt.h"
#include "TestsResults.h"

UserPanel::UserPanel(std::shared_ptr<User> connectedUser, QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->m_connectedUser = connectedUser;
	initializeUI();

	// Logout button
	connect(ui.logoutButton, SIGNAL(clicked()), SLOT(onLogoutButtonClicked()));

	// Teacher panel buttons
	connect(ui.questionBankButton, SIGNAL(clicked()), SLOT(onQuestionBankButtonClicked()));
	connect(ui.createTestButton, SIGNAL(clicked()), SLOT(onCreateTestButtonClicked()));
	connect(ui.seeTestResultsButton, SIGNAL(clicked()), SLOT(onSeeTestResultsButtonClicked()));
	connect(ui.manageCandidatesButton, SIGNAL(clicked()), SLOT(onManageCandidatesButtonClicked()));

	// Administrator panel button
	connect(ui.manageUsersButton, SIGNAL(clicked()), SLOT(onManageUsersButtonClicked()));

	// Candidate panel buttons
	connect(ui.testTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onTestDoubleClicked(int, int)));

	// Refresh shortcut
	QShortcut* refreshShortcut = new QShortcut(QKeySequence("F5"), this);
	connect(refreshShortcut, SIGNAL(activated()), this, SLOT(onF5()));

}

void UserPanel::onLogoutButtonClicked() 
{
	QMessageBox::information(this, "", "You have successfully logged out!", QMessageBox::Button::Ok);
	Utils::logActivity(m_connectedUser->getEmail() + " logged out!", Logger::Level::Info);
	LoginPanel* login = new LoginPanel();
	login->show();
	this->hide();
}

void UserPanel::initializeUI()
{
	QString userFirstName = m_connectedUser->getFirstName();
	QString userLastName = m_connectedUser->getLastName();
	QString userType = m_connectedUser->getTypeAsString();

	QString welcomeMessage = "Welcome " + userFirstName + " " + userLastName + "!";
	QString windowTitle = "QuestionnaireApp - " + userType.toUpper();

	ui.welcomeLabel->setText(welcomeMessage);
	QWidget::setWindowTitle(windowTitle);

	switch (m_connectedUser->getType())
	{
	case User::userType::Administrator:
		ui.userTabWidget->removeTab(1);
		ui.userTabWidget->removeTab(1);
		break;
	case User::userType::Teacher:
		ui.userTabWidget->removeTab(0);
		ui.userTabWidget->removeTab(1);
		break;
	case User::userType::Candidate:
		ui.userTabWidget->removeTab(0);
		ui.userTabWidget->removeTab(0);
		populateTestTable();
		break;
	}
}

void UserPanel::onQuestionBankButtonClicked()
{
	QuestionBank* questionBank = new QuestionBank(this->m_connectedUser);
	questionBank->show();
}

void UserPanel::onCreateTestButtonClicked()
{
	ManageTest* manageTests = new ManageTest(m_connectedUser);
	manageTests->show();
}

void UserPanel::onSeeTestResultsButtonClicked()
{
	Utils::logActivity(m_connectedUser->getEmail() + " checked test results!", Logger::Level::Info);
	TestsResults* testsResults = new TestsResults(m_connectedUser);
	testsResults->show();
}

void UserPanel::onManageCandidatesButtonClicked()
{
	ManageUsers* manageCandidates = new ManageUsers(m_connectedUser);
	manageCandidates->show();

}

void UserPanel::onManageUsersButtonClicked()
{
	ManageUsers* manageUsers = new ManageUsers(m_connectedUser);
	manageUsers->show();
}

double getMark(int candidateId, int testId)
{
	double mark = 0;
	QString sqlQuery = "SELECT u.mark FROM usertestresult u WHERE u.userId = :candidateId AND u.testId = :testId";
	QSqlQuery query(QSqlDatabase::database());
	query.prepare(sqlQuery);
	query.bindValue(":candidateId", candidateId);
	query.bindValue(":testId", testId);
	query.exec();
	if (query.next())
	{
		mark=query.value("u.mark").toDouble();
	}
	return mark;
}

void UserPanel::onTestDoubleClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.testTableWidget->item(row, 0); // get id from column 0 of table
	auto selectedTest = std::make_shared<Test>(Utils::findItemById(tableItem->text().toInt(), m_listOfTests));
	try
	{
		if (getMark(m_connectedUser->getId(), selectedTest->getId()) != 0)
		{
			throw 100;
		}
		if (selectedTest->getEndTime() < QDateTime::currentDateTime())
		{
			throw 200;
		}
		Utils::logActivity(m_connectedUser->getEmail() + " started a test!", Logger::Level::Info);
		TestAttempt* attempt = new TestAttempt(m_connectedUser, selectedTest->generateRandomQuestionsForUser(m_connectedUser->getId()), selectedTest);
		attempt->show();
	}
	catch (int error)
	{
		switch (error)
		{
		case 100:
			QMessageBox::warning(this, "", "One attempt allowed!", QMessageBox::Button::Ok);
			break;
		case 200:
			QMessageBox::warning(this, "", "Test closed!", QMessageBox::Button::Ok);
		}
	}
	
}

void UserPanel::onF5()
{
	populateTestTable();
}

void UserPanel::populateTestTable()
{
	getTestsList();

	// reset table first
	ui.testTableWidget->reset();
	ui.testTableWidget->setRowCount(0);

	ui.testTableWidget->setRowCount(m_listOfTests.size());
	ui.testTableWidget->setColumnCount(7);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
	ui.testTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "Domain" << "Available time" << "Questions" << "Start time" << "End time" << "Mark";
	ui.testTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfTests.begin(), m_listOfTests.end(), [&](Test& test)
		{
			ui.testTableWidget->setSortingEnabled(false);
			ui.testTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(test.getId()))); // id column
			ui.testTableWidget->setItem(rowCount, 1, new QTableWidgetItem(test.getDomain().getName() + " (" + m_teacherMap[test.getTeacherId()] + ")")); // domain column
			ui.testTableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::number(test.getAvailableTime()))); // available time column
			ui.testTableWidget->setItem(rowCount, 3, new QTableWidgetItem(QString::number(test.getNumberOfQuestions()))); // number of questions column
			ui.testTableWidget->setItem(rowCount, 4, new QTableWidgetItem(test.getStartTime().toString("ddd MMMM dd yyyy | hh:mm "))); // start time column
			ui.testTableWidget->setItem(rowCount, 5, new QTableWidgetItem(test.getEndTime().toString("ddd MMMM dd yyyy | hh:mm "))); // end time column
			double mark = getMark(m_connectedUser->getId(), test.getId());
			ui.testTableWidget->setItem(rowCount, 6, new QTableWidgetItem(mark == 0 ? "-" : QString::number(mark))); // mark column
			rowCount++;
			ui.testTableWidget->setSortingEnabled(true);
		});
}

void UserPanel::getTestsList()
{
	m_listOfTests.clear();
	QString sql = "SELECT * FROM test t, domain d, user u WHERE t.domainId=d.id AND t.teacherId=u.id";

	std::pair<QString, QString> pair("","");
	QSqlQuery query = DBOperations::selectFromTable(sql, { pair });

	while (query.next())
	{
		int id = query.value("t.id").toInt();
		int availableTime = query.value("t.availableTime").toInt();

		int teacherId = query.value("t.teacherId").toInt();
		QString teacherFirstName = query.value("u.firstName").toString();
		QString teacherLastName = query.value("u.lastName").toString();
		QString teacherName = teacherFirstName + " " + teacherLastName;
		m_teacherMap[teacherId] = teacherName;

		QDateTime startTime = query.value("t.startTime").toDateTime();
		QDateTime endTime = query.value("t.endTime").toDateTime();
		int numberOfQuestions = query.value("t.numberOfQuestions").toInt();

		int domainId = query.value("d.id").toInt();
		QString domainName = query.value("d.name").toString();

		Domain domain(domainId, domainName);
		Test test(id, teacherId, availableTime, numberOfQuestions, startTime, endTime, domain);

		m_listOfTests.push_back(test);
	}
}

UserPanel::~UserPanel()
{
}
