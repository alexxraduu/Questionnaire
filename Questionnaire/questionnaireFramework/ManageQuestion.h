#pragma once

#include <QWidget>

#include "ui_ManageQuestion.h"
#include "Question.h"
#include "Domain.h"
#include "Chapter.h"
#include "DBOperations.h"

class ManageQuestion : public QWidget
{
	Q_OBJECT

public:
	ManageQuestion(std::shared_ptr<Question> question, QWidget *parent = Q_NULLPTR);
	~ManageQuestion();

private:
	Ui::ManageQuestion ui;
	std::shared_ptr<Question> m_question;
	int m_numberOfAnswers;
	std::vector<Domain> m_listOfDomains;
	std::vector<Chapter> m_listOfChapters;
	Domain m_selectedDomain;
	Chapter m_selectedChapter;
	std::vector<QHBoxLayout*> m_vectorOfAnswerLayouts;
	std::vector<QPlainTextEdit*> m_vectorOfAnswerTextEdit;
	std::vector<QComboBox*> m_vectorOfAnswerComboBox;
	Question::questionType m_questionType;

private:
	void initializeUI();
	void putMultipleAnswerItems();
	void getChapterList();
	void getDomainList();
	void populateDomainTable();
	void populateChapterTable();
	void populateItems();

private slots:
	void singleAnswerRadioSelected();
	void multipleAnswerRadioSelected();
	void numberOfAnswersChanged(int index);
	void onDomainClicked(int row, int column);
	void onChapterClicked(int row, int column);
	void onSaveButtonClicked();
	void onDeleteButtonClicked();
};
