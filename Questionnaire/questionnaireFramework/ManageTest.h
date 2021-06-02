#pragma once

#include <QWidget>

#include "ui_ManageTest.h"
#include "User.h"
#include "Test.h"

class ManageTest : public QWidget
{
	Q_OBJECT

public:
	ManageTest(std::shared_ptr<User>connectedUser, QWidget *parent = Q_NULLPTR);
	~ManageTest();

private:
	Ui::ManageTest ui;
	std::shared_ptr<User> m_connectedUser;
	std::vector<Test> m_listOfTests;
	Test m_selectedTest;

private:
	void populateTestTable();
	void getTestsList();

private slots:
	void onCreateTestButtonClicked();
	void onTestCellClicked(int row, int column);
	void onDelButtonClicked();
	void onF5();
};
