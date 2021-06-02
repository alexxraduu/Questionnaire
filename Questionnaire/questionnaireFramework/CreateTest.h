#pragma once

#include <QWidget>
#include <vector>

#include "ui_CreateTest.h"
#include "User.h"
#include "Domain.h"

class CreateTest : public QWidget
{
	Q_OBJECT

public:
	CreateTest(std::shared_ptr<User>connectedUser, QWidget* parent = Q_NULLPTR);
	~CreateTest();

private:
	void populateDomainList();
	void getDomainList();

private:
	Ui::CreateTest ui;
	std::shared_ptr<User> m_connectedUser;
	std::vector<Domain> m_listOfDomains;
	Domain m_selectedDomain;

private slots:
	void onDomainClicked(int row, int column);
	void onCreateTestButtonClicked();
};
