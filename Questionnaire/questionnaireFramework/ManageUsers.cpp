#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QShortcut>
#include <fstream>
#include <sstream>

#include "DBOperations.h"
#include "ManageUsers.h"
#include "Utils.h"

ManageUsers::ManageUsers(std::shared_ptr<User> user, QWidget* parent)
	: QWidget(parent),
	m_connectedUser(user)
{
	ui.setupUi(this);
	onSearchUserInputChanged();
	connect(ui.searchUserInput, SIGNAL(textEdited(const QString&)), SLOT(onSearchUserInputChanged()));
	connect(ui.userTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onUserClicked(int, int)));
	connect(ui.addUsersFromTxtButton, SIGNAL(clicked()), SLOT(onAddUsersFromTxtClicked()));

	// shortcuts
	QShortcut* deleteShortcut = new QShortcut(QKeySequence("Del"), this);
	connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onDelButtonClicked()));
}

void ManageUsers::getUsersList(const QString& searchedValue)
{
	m_listOfUsers.clear();
	QString sql;
	if (m_connectedUser->getType() == User::userType::Administrator)
	{
		sql = "SELECT * FROM user WHERE id=:searchedId OR emailAdress LIKE :searchedValue OR firstName LIKE :searchedValue OR lastName LIKE :searchedValue OR type LIKE :searchedValue";
	}
	else
	{
		sql = "SELECT * FROM user WHERE (id=:searchedId OR emailAdress LIKE :searchedValue OR firstName LIKE :searchedValue OR lastName LIKE :searchedValue) AND type='Candidate'";
	}
	std::pair<QString, QString> pairSearchedValue(":searchedValue", QString("%%1%").arg(searchedValue));
	std::pair<QString, QString> pairSearchedId(":searchedId", searchedValue);
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairSearchedValue, pairSearchedId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString email = query.value("emailAdress").toString();
		QString firstName = query.value("firstName").toString();
		QString lastName = query.value("lastName").toString();
		QString typeString = query.value("type").toString();

		User::userType type;
		if (typeString == "Candidate")
		{
			type = User::userType::Candidate;
		}
		else if (typeString == "Administrator")
		{
			type = User::userType::Administrator;
		}
		else if (typeString == "Teacher")
		{
			type = User::userType::Teacher;
		}

		User user(id, email, firstName, lastName, type);
		m_listOfUsers.push_back(user);
	}
}

void ManageUsers::populateUsersTable()
{
	getUsersList(ui.searchUserInput->text());
	// reset table first
	ui.userTableWidget->reset();
	ui.userTableWidget->setRowCount(0);

	ui.userTableWidget->setRowCount(m_listOfUsers.size());
	ui.userTableWidget->setColumnCount(5);
	ui.userTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.userTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.userTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.userTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.userTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	ui.userTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "E-mail address" << "First name" << "Last name" << "Type";
	ui.userTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfUsers.begin(), m_listOfUsers.end(), [&](auto& user)
		{	
			ui.userTableWidget->setSortingEnabled(false);
			ui.userTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(user.getId()))); // id column
			ui.userTableWidget->setItem(rowCount, 1, new QTableWidgetItem(user.getEmail())); // email column
			ui.userTableWidget->setItem(rowCount, 2, new QTableWidgetItem(user.getFirstName())); // first name column
			ui.userTableWidget->setItem(rowCount, 3, new QTableWidgetItem(user.getLastName())); // last name column
			ui.userTableWidget->setItem(rowCount, 4, new QTableWidgetItem(user.getTypeAsString())); // user type column
			rowCount++;
			ui.userTableWidget->setSortingEnabled(true);
		});
}

void ManageUsers::onDelButtonClicked()
{
	m_selectedUser.remove();
	populateUsersTable();
	QMessageBox::information(this, "", "The user " + m_selectedUser.getEmail() + " was removed succesfully!", QMessageBox::Button::Ok);
}

void ManageUsers::onUserClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.userTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedUser = Utils::findItemById(tableItem->text().toInt(), m_listOfUsers);
}

std::vector<User> ManageUsers::readUsersFromFile(const std::string& fileName)
{
	std::vector<User> listOfUsers;
	std::ifstream file(fileName);
	std::string currentLine;
	while (getline(file, currentLine))
	{
		std::istringstream lineParser(currentLine);
		std::string email;
		std::string lastName;
		std::string firstName;
		std::string userType;
		try
		{ 
			lineParser
				>> email
				>> lastName
				>> firstName
				>> userType;
			if (!Utils::isEmailValid(email))
			{
				throw 100;
			}
			if (userType != "candidate" && userType != "teacher")
			{
				throw 200;
			}
			if (m_connectedUser->getType() == User::userType::Teacher)
			{
				if (userType != "candidate")
				{
					throw 300;
				}
			}
			if (userType == "candidate")
			{
				User user(QString::fromStdString(email), QString::fromStdString(firstName), QString::fromStdString(lastName),User::userType::Candidate);
				listOfUsers.push_back(user);
			}
			else if (userType == "teacher")
			{
				User user(QString::fromStdString(email), QString::fromStdString(firstName), QString::fromStdString(lastName), User::userType::Teacher);
				listOfUsers.push_back(user);
			}
		}
		catch (int error)
		{
			switch (error)
			{
			case 100:
				QMessageBox::warning(this, "", "Email " + QString::fromStdString(email) + " not valid!", QMessageBox::Button::Ok);
				break;
			case 200:
				QMessageBox::warning(this, "", "User type not valid (teacher/candidate)!", QMessageBox::Button::Ok);
				break;
			case 300:
				QMessageBox::warning(this, "", "You can only add candidates!", QMessageBox::Button::Ok);
				break;
			}
		}
	}
	return listOfUsers;
}

void ManageUsers::onAddUsersFromTxtClicked()
{
	if(m_connectedUser->getType()==User::userType::Administrator)
	{
		QMessageBox::information(this, "", "Txt file format:\nE-mail Lastname Firstname Type(candidate/teacher)\n\nExample of file:\npauldenino@student.test.ro Denino Paul candidate\nchancemorris@student.test.ro Morris Chance teacher", QMessageBox::Button::Ok);
	} else
	{ 
		QMessageBox::information(this, "", "Txt file format:\nE-mail Lastname Firstname Type(candidate)\n\nExample of file:\npauldenino@student.test.ro Denino Paul candidate\nchancemorris@student.test.ro Morris Chance candidate", QMessageBox::Button::Ok);
	}

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		"/home",
		tr("Text files (*.txt)"));
	if (fileName != NULL)
	{
		for (auto& user : readUsersFromFile(fileName.toStdString()))
		{
			user.addToDB();
		}
			populateUsersTable();
	}
	Utils::logActivity(m_connectedUser->getEmail() + " added users to database!", Logger::Level::Info);
}

void ManageUsers::onSearchUserInputChanged()
{
	populateUsersTable();
}

ManageUsers::~ManageUsers()
{
}

