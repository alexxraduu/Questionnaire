#include <QMessageBox>
#include <QShortcut>

#include "CreateTest.h"
#include "Utils.h"
#include "Test.h"

CreateTest::CreateTest(std::shared_ptr<User>connectedUser, QWidget *parent):
	m_connectedUser(connectedUser),
	QWidget(parent)
{
	ui.setupUi(this);
	populateDomainList();
	ui.startTimeInput->setDateTime(QDateTime::currentDateTime());
	ui.endTimeInput->setDateTime(QDateTime::currentDateTime());
	//domain
	connect(ui.domainTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onDomainClicked(int, int)));

	connect(ui.createTestButton, SIGNAL(clicked()), SLOT(onCreateTestButtonClicked()));
}

CreateTest::~CreateTest()
{
}

void CreateTest::populateDomainList()
{
	getDomainList();

	// reset table first
	ui.domainTableWidget->reset();
	ui.domainTableWidget->setRowCount(0);

	ui.domainTableWidget->setRowCount(m_listOfDomains.size());
	ui.domainTableWidget->setColumnCount(2);
	ui.domainTableWidget->verticalHeader()->hide(); // hide vertical header
	ui.domainTableWidget->hideColumn(0); // hide id column
	QStringList tableHeader;
	tableHeader << "#" << "Domain name";
	ui.domainTableWidget->setHorizontalHeaderLabels(tableHeader);
	ui.domainTableWidget->horizontalHeader()->setStretchLastSection(true);

	int rowCount = 0;
	std::for_each(m_listOfDomains.begin(), m_listOfDomains.end(), [&](Domain& domain)
		{
			ui.domainTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(domain.getId()))); // id column
			ui.domainTableWidget->setItem(rowCount, 1, new QTableWidgetItem(domain.getName())); // name column
			rowCount++;
		});

}

void CreateTest::getDomainList()
{
	m_listOfDomains.clear();
	QString sql = "SELECT * from domain";
	std::pair<QString, QString> pairTeacherId(":teacherId", QString::number(m_connectedUser->getId()));
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairTeacherId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Domain domain(id, name);
		m_listOfDomains.push_back(domain);
	}
}

int timeDifference(QDateTime date1, QDateTime date2)
{
	int seconds = (date1.toSecsSinceEpoch() -date2.toSecsSinceEpoch());
	int minutes = seconds/60;
	return minutes;
}

void CreateTest::onCreateTestButtonClicked()
{
	try {
		if (m_selectedDomain.getId() == 0)
		{
			throw 400;
		}
		if (ui.timeInput->text() == "")
		{
			throw 100;
		}
		if (ui.numberOfQuestionsInput->text() == "")
		{
			throw 200;
		}
		if (ui.startTimeInput->dateTime() >= ui.endTimeInput->dateTime())
		{
			throw 300;
		}
		if (ui.numberOfQuestionsInput->text().toInt() > m_selectedDomain.numberOfQuestions())
		{
			throw 500;
		}
		if (timeDifference(ui.endTimeInput->dateTime(),ui.startTimeInput->dateTime())<ui.timeInput->text().toInt())
		{
			throw 600;
		}
		if (ui.timeInput->text().toInt() < 1)
		{
			throw 600;
		}
		if (ui.numberOfQuestionsInput->text().toInt() < 1)
		{
			throw 700;
		}

		int time = ui.timeInput->text().toInt();
		QDateTime startTime = ui.startTimeInput->dateTime();
		QDateTime endTime = ui.endTimeInput->dateTime();
		int numberOfQuestions = ui.numberOfQuestionsInput->text().toInt();
		Test test(m_connectedUser->getId(), time, numberOfQuestions, startTime, endTime, m_selectedDomain);
		test.saveToDB();
		Utils::logActivity(m_connectedUser->getEmail() + " created a test!", Logger::Level::Info);
		this->close();
	}
	catch (int error)
	{
		switch (error)
		{
		case 100:
			QMessageBox::warning(this, "", "Time can't be empty!", QMessageBox::Button::Ok);
			break;
		case 200:
			QMessageBox::warning(this, "", "Number of questions can't be empty!", QMessageBox::Button::Ok);
			break;
		case 300:
			QMessageBox::warning(this, "", "Start time must be lower than end time!", QMessageBox::Button::Ok);
			break;
		case 400:
			QMessageBox::warning(this, "", "You have to choose a domain!", QMessageBox::Button::Ok);
			break;
		case 500:
			QMessageBox::warning(this, "", "The domain contains less questions!", QMessageBox::Button::Ok);
			break;
		case 600:
			QMessageBox::warning(this, "", "Invalid time!", QMessageBox::Button::Ok);
			break;
		case 700:
			QMessageBox::warning(this, "", "Minimum 1 question required!", QMessageBox::Button::Ok);
			break;
		}
	}
}

void CreateTest::onDomainClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.domainTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedDomain = Utils::findItemById(tableItem->text().toInt(), m_listOfDomains);
}
