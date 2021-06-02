#pragma once

#include <QWidget>

#include "ui_QuestionBank.h"
#include "ManageQuestion.h"
#include "User.h"
#include "Chapter.h"
#include "Domain.h"
#include "Question.h"

class QuestionBank: public QWidget
{
	Q_OBJECT

public:
	QuestionBank(std::shared_ptr<User> user, QWidget* parent = Q_NULLPTR);
	~QuestionBank();

private:
	Ui::QuestionBank ui;
	std::shared_ptr<User> m_connectedUser;
	std::vector<Domain> m_listOfDomains;
	std::vector<Chapter> m_listOfChapters;
	std::vector<Question> m_listOfQuestions;
	Domain m_selectedDomain;
	Chapter m_selectedChapter;

private:
	void getChapterList();
	void getDomainList(const QString& searchedValue);
	void getQuestionsList(const QString& searchedValue);
	void populateDomainTable();
	void populateChapterTable();
	void populateQuestionTable();

private slots:

	// table actions
	void onSearchDomainInputChanged();
	void onSearchQuestionInputChanged();
	void onDomainClicked(int row, int column);
	void onChapterClicked(int row, int column);
	void onQuestionDoubleClicked(int row, int column);

	// add stuff
	void onAddDomainButtonClicked();
	void onAddChapterButtonClicked();
	void onAddQuestionButtonClicked();

	// keyboard shortcuts
	void onReturnButtonClicked();
	void onDelButtonClicked();
	void onF5();
};
