#include <QShortcut>
#include <QMessageBox>

#include "ManageTest.h"
#include "CreateTest.h"
#include "Utils.h"

ManageTest::ManageTest(std::shared_ptr<User>connectedUser, QWidget *parent):
	m_connectedUser(connectedUser),
	QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.createTestButton, SIGNAL(clicked()), SLOT(onCreateTestButtonClicked()));
	connect(ui.testTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onTestCellClicked(int, int)));
	QShortcut* deleteShortcut = new QShortcut(QKeySequence("Del"), this);
	connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onDelButtonClicked()));
	populateTestTable();

	// Refresh shortcut
	QShortcut* refreshShortcut = new QShortcut(QKeySequence("F5"), this);
	connect(refreshShortcut, SIGNAL(activated()), this, SLOT(onF5()));
}

void ManageTest::onCreateTestButtonClicked()
{
	CreateTest* createTests=new CreateTest(m_connectedUser);
	createTests->show();
	populateTestTable();
}

ManageTest::~ManageTest()
{
}

void ManageTest::populateTestTable()
{
	getTestsList();
	// reset table first
	ui.testTableWidget->reset();
	ui.testTableWidget->setRowCount(0);
	  
	ui.testTableWidget->setRowCount(m_listOfTests.size());
	ui.testTableWidget->setColumnCount(6);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	ui.testTableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
	ui.testTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "Domain" << "Available time" << "Start time" << "End time"<<"Number of questions";
	ui.testTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfTests.begin(), m_listOfTests.end(), [&](Test& test)
		{
			ui.testTableWidget->setSortingEnabled(false);
			ui.testTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(test.getId()))); // id column
			ui.testTableWidget->setItem(rowCount, 1, new QTableWidgetItem(test.getDomain().getName())); // domian column
			ui.testTableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString::number(test.getAvailableTime()))); // available time column
			ui.testTableWidget->setItem(rowCount, 3, new QTableWidgetItem(test.getStartTime().toString("ddd MMMM dd yyyy | hh:mm "))); // start time column
			ui.testTableWidget->setItem(rowCount, 4, new QTableWidgetItem(test.getEndTime().toString("ddd MMMM dd yyyy | hh:mm "))); // end time column
			ui.testTableWidget->setItem(rowCount, 5, new QTableWidgetItem(QString::number(test.getNumberOfQuestions()))); // number of questions column
			rowCount++;
			ui.testTableWidget->setSortingEnabled(true);
		});
}

void ManageTest::getTestsList()
{
	m_listOfTests.clear();
	QString sql = "SELECT * FROM test t, domain d WHERE t.teacherId= :teacherId AND t.domainId=d.id";
	
	std::pair<QString, QString> pairTeacherId(":teacherId", QString::number(m_connectedUser->getId()));
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairTeacherId });

	while (query.next())
	{
		int id = query.value("t.id").toInt();
		int availableTime = query.value("t.availableTime").toInt();
		QDateTime startTime = query.value("t.startTime").toDateTime();
		QDateTime endTime = query.value("t.endTime").toDateTime();
		int numberOfQuestions= query.value("t.numberOfQuestions").toInt();

		int domainId= query.value("d.id").toInt();
		QString domainName = query.value("d.name").toString();

		Domain domain(domainId, domainName);
		Test test(id, m_connectedUser->getId(), availableTime, numberOfQuestions, startTime, endTime, domain);

		m_listOfTests.push_back(test);
	}
}

void ManageTest::onTestCellClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.testTableWidget->item(row, 0); // get id from column 0 of table
	Test selectedTest = Utils::findItemById(tableItem->text().toInt(), m_listOfTests);
	m_selectedTest = selectedTest;
}

void ManageTest::onDelButtonClicked()
{
	m_selectedTest.remove();
	populateTestTable();
	Utils::logActivity(m_connectedUser->getEmail() + " deleted a test!", Logger::Level::Info);
	QMessageBox::information(this, "", "The test was removed succesfully!", QMessageBox::Button::Ok);
}

void ManageTest::onF5()
{
	populateTestTable();
}
