#pragma once

#include <QWidget>

#include "ui_TestsResults.h"
#include "UserTestResult.h";
#include "User.h"
#include "Test.h"

class TestsResults : public QWidget
{
	Q_OBJECT

public:
	TestsResults(std::shared_ptr<User> connectedUser, QWidget *parent = Q_NULLPTR);
	~TestsResults();

private:
	Ui::TestsResults ui;
	std::shared_ptr<User> m_connectedUser;
	Test m_selectedTest;
	std::vector<Test> m_listOfTests;
	std::vector<UserTestResult> m_listOfCandidatesResults;

private:
	void getTestsList();
	void getCandidatesResultsList();
	void populateTestTable();
	void populateCandidatesResultsTable();

private slots:
	void onTestCellClicked(int row, int column);
	void onCandidateResultCellClicked(int row, int column);
	void onSearchCandidate();
};
