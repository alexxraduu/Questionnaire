#include <QMessageBox>
#include <QShortcut>
#include <regex>

#include "DBOperations.h"
#include "UserPanel.h"
#include "LoginPanel.h"
#include "Utils.h"

LoginPanel::LoginPanel(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QShortcut* returnShortcut = new QShortcut(QKeySequence("Return"), this);
	connect(ui.loginButton, SIGNAL(clicked()), SLOT(onLoginButtonClicked()));
	connect(returnShortcut, SIGNAL(activated()), ui.loginButton, SLOT(click()));
	connect(ui.checkShowPassword, SIGNAL(stateChanged(int)), SLOT(onShowPasswordChecked()));
}

bool isEmailValid(const std::string& email)
{
	const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
	return std::regex_match(email, pattern);
}

std::shared_ptr<User> checkCredentials(const QString& email, const QString& password)
{
	QString sql = "SELECT * from questionnaire.user where emailAdress = :email and binary password = :password";
	std::pair<QString, QString> pairEmail(":email", email);
	std::pair<QString, QString> pairPassword(":password", password);
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairEmail, pairPassword });
	if (query.next())
	{
		int id = query.value(0).toInt();
		QString emailAdress = query.value("emailAdress").toString();
		QString firstName = query.value("firstName").toString();
		QString lastName = query.value("lastName").toString();
		QString userTypeValue = query.value("type").toString();
		User::userType userType;
		if (userTypeValue == "Administrator")
		{
			userType = User::userType::Administrator;
		}
		else if (userTypeValue == "Teacher")
		{
			userType = User::userType::Teacher;
		}
		else if (userTypeValue == "Candidate")
		{
			userType = User::userType::Candidate;
		}
		return std::make_shared<User>(id, emailAdress, firstName, lastName, userType);
	}
	else
	{
		throw 300;
	}
}

void LoginPanel::onLoginButtonClicked()
{
	ui.emailInput->setStyleSheet("color: black;  background-color: white");
	ui.passwordInput->setStyleSheet("color: black;  background-color: white");
	try {
		if (ui.emailInput->text().isEmpty() || ui.passwordInput->text().isEmpty())
		{
			throw 100;
		}
		if (!isEmailValid(ui.emailInput->text().toStdString()))
		{
			throw 200;
		}
		std::shared_ptr<User> connectedUser = checkCredentials(ui.emailInput->text(), ui.passwordInput->text());
		Utils::logActivity(connectedUser->getEmail() + " logged in successfully!", Logger::Level::Info);
		UserPanel* userPanel = new UserPanel(connectedUser);
		userPanel->show();
		this->hide();
	}
	catch (int error)
	{
		if (error == 100)
		{
			QMessageBox::critical(this, "Error", "You didn't type anything!", QMessageBox::Button::Ok);
			if (ui.emailInput->text().isEmpty())
			{
				ui.emailInput->setStyleSheet("color: black;  background-color: pink");
			}
			if (ui.passwordInput->text().isEmpty())
			{
				ui.passwordInput->setStyleSheet("color: black;  background-color: pink");
			}
		}
		else if (error == 200)
		{
			QMessageBox::critical(this, "Error", "E-mail adress is not valid!", QMessageBox::Button::Ok);
			ui.emailInput->setStyleSheet("color: black;  background-color: pink");
		}
		else if (error == 300)
		{
			QMessageBox::critical(this, "Error", "Wrong username or password!", QMessageBox::Button::Ok);
		}
	}
}

void LoginPanel::onShowPasswordChecked()
{
	if (ui.checkShowPassword->isChecked())
	{
		ui.passwordInput->setEchoMode(QLineEdit::Normal);
	}
	else
	{
		ui.passwordInput->setEchoMode(QLineEdit::Password);
	}
}
