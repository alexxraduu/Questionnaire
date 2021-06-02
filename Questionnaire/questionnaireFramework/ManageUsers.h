#pragma once

#include <QWidget>

#include "ui_ManageUsers.h"
#include "User.h"

class ManageUsers : public QWidget
{
	Q_OBJECT

public:
	ManageUsers(std::shared_ptr<User> user, QWidget *parent = Q_NULLPTR);
	~ManageUsers();
	
public:
	void getUsersList(const QString& searchedValue);

private:
	Ui::ManageUsers ui;
	std::vector<User> m_listOfUsers;
	std::shared_ptr<User> m_connectedUser;
	User m_selectedUser;

private:
	void populateUsersTable();
	std::vector<User> readUsersFromFile(const std::string& fileName);

private slots:
	void onSearchUserInputChanged();
	void onDelButtonClicked();
	void onUserClicked(int row, int column);
	void onAddUsersFromTxtClicked();
};
