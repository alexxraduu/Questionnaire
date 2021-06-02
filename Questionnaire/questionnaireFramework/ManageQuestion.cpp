#include <QMessageBox>

#include "ManageQuestion.h"
#include "Utils.h"

ManageQuestion::ManageQuestion(std::shared_ptr<Question> question, QWidget* parent) :
	QWidget(parent),
	m_question(question),
	m_selectedDomain(m_question->getChapter().getDomain()),
	m_selectedChapter(m_question->getChapter()),
	m_numberOfAnswers(m_question->getAnswers().size())
{
	ui.setupUi(this);

	connect(ui.singleAnswerRadio, SIGNAL(clicked()), this, SLOT(singleAnswerRadioSelected()));
	connect(ui.multipleAnswerRadio, SIGNAL(clicked()), this, SLOT(multipleAnswerRadioSelected()));
	connect(ui.numberOfAnswersComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(numberOfAnswersChanged(int)));

	connect(ui.domainTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onDomainClicked(int, int)));
	connect(ui.chapterTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onChapterClicked(int, int)));

	connect(ui.saveButton, SIGNAL(clicked()), SLOT(onSaveButtonClicked()));
	connect(ui.deleteButton, SIGNAL(clicked()), SLOT(onDeleteButtonClicked()));

	initializeUI();
}

void ManageQuestion::initializeUI()
{
	if (m_question->getId() == 0) // if new question then hide "delete question" button
	{
		ui.deleteButton->setVisible(false);
	}

	// add items for numberOfAnswers combo box
	ui.numberOfAnswersComboBox->addItem("2 answers", QVariant(2)); // a
	ui.numberOfAnswersComboBox->addItem("3 answers", QVariant(3)); // b
	ui.numberOfAnswersComboBox->addItem("4 answers", QVariant(4)); // c
	ui.numberOfAnswersComboBox->addItem("5 answers", QVariant(5)); // d

	switch (m_question->getType()) {
	case Question::questionType::SingleAnswer:
		ui.singleAnswerRadio->click();
		singleAnswerRadioSelected();
		break;
	case Question::questionType::MultipleAnswer:
		ui.multipleAnswerRadio->click();
		if (m_question->getId() == 0) // if new question
		{
			ui.numberOfAnswersComboBox->setCurrentIndex(ui.numberOfAnswersComboBox->findData(4)); // set default number of answers to 4
		}
		else
		{
			ui.numberOfAnswersComboBox->setCurrentIndex(ui.numberOfAnswersComboBox->findData(m_question->getAnswers().size()));
		}
		multipleAnswerRadioSelected();
		putMultipleAnswerItems();
		break;
	}

	populateItems();
}

void ManageQuestion::getDomainList()
{
	m_listOfDomains.clear();
	QString sql = "SELECT * from domain";
	std::pair<QString, QString> pairUserId(":id", m_question->getTeacherId());
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairUserId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Domain domain(id, name);
		m_listOfDomains.push_back(domain);
	}
}

void ManageQuestion::getChapterList()
{
	m_listOfChapters.clear();
	QString sql = "SELECT * from chapter where domainId = :domainId";
	std::pair<QString, int> pairDomainId(":domainId", m_selectedDomain.getId());
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairDomainId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Chapter chapter(id, name, m_question->getChapter().getDomain());
		m_listOfChapters.push_back(chapter);
	}
}

void ManageQuestion::populateDomainTable()
{
	getDomainList();

	ui.domainTableWidget->setRowCount(m_listOfDomains.size());
	ui.domainTableWidget->setColumnCount(2);
	ui.domainTableWidget->verticalHeader()->hide(); // hide vertical header
	ui.domainTableWidget->hideColumn(0); // hide id column
	QStringList tableHeader;
	tableHeader << "#" << "Domain name";
	ui.domainTableWidget->setHorizontalHeaderLabels(tableHeader);
	ui.domainTableWidget->horizontalHeader()->setStretchLastSection(true);

	int rowCount = 0;
	int markedRow = 0;
	std::for_each(m_listOfDomains.begin(), m_listOfDomains.end(), [&](Domain& domain)
		{
			ui.domainTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(domain.getId()))); // id column(hidden)
			ui.domainTableWidget->setItem(rowCount, 1, new QTableWidgetItem(domain.getName())); // name column
			if (domain.getId() == m_selectedDomain.getId())
			{
				markedRow = rowCount;
			}
			rowCount++;
		});

	ui.domainTableWidget->selectRow(markedRow); // mark(select) the domain of the question
	ui.domainTableWidget->scrollToItem(ui.domainTableWidget->item(markedRow, 1)); // scroll to domain of the question
	if (m_listOfDomains.size() != 0 && m_question->getId() == 0) // if new question
	{
		m_selectedDomain = Utils::findItemById(ui.domainTableWidget->item(markedRow, 0)->text().toInt(), m_listOfDomains); // save the first domain in the list
	}
}

void ManageQuestion::populateChapterTable()
{
	getChapterList();

	// reset table first
	ui.chapterTableWidget->reset();
	ui.chapterTableWidget->setRowCount(0);

	ui.chapterTableWidget->setRowCount(m_listOfChapters.size());
	ui.chapterTableWidget->setColumnCount(2);
	ui.chapterTableWidget->verticalHeader()->hide(); // hide vertical header
	ui.chapterTableWidget->hideColumn(0); // hide id column
	QStringList tableHeader;
	tableHeader << "#" << "Chapter name";
	ui.chapterTableWidget->setHorizontalHeaderLabels(tableHeader);
	ui.chapterTableWidget->horizontalHeader()->setStretchLastSection(true);

	int rowCount = 0;
	int markedRow = 0;
	std::for_each(m_listOfChapters.begin(), m_listOfChapters.end(), [&](Chapter& chapter)
		{
			ui.chapterTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(chapter.getId()))); // id column(hidden)
			ui.chapterTableWidget->setItem(rowCount, 1, new QTableWidgetItem(chapter.getName())); // name column
			if (chapter.getId() == m_selectedChapter.getId())
			{
				markedRow = rowCount;
			}
			rowCount++;
		});

	ui.chapterTableWidget->selectRow(markedRow); // mark(select) the chapter of the question
	ui.chapterTableWidget->scrollToItem(ui.chapterTableWidget->item(markedRow, 1)); // scroll to the chapter of the question
	if(m_listOfChapters.size() != 0)
	{
		m_selectedChapter = Utils::findItemById(ui.chapterTableWidget->item(markedRow, 0)->text().toInt(), m_listOfChapters);
	}
}

void ManageQuestion::populateItems()
{
	populateDomainTable();
	populateChapterTable();

	this->setWindowTitle(m_question->getTitle()); // window title
	ui.titleInput->setText(m_question->getTitle()); // question title
	ui.questionInput->setPlainText(m_question->getText()); // question text

	switch (m_question->getType())
	{
	case Question::questionType::SingleAnswer:
		ui.answerInput->setText(m_question->getAnswerValue()); // short answer
		break;
	case Question::questionType::MultipleAnswer:
		for (int i = 0; i < m_numberOfAnswers; ++i)
		{
			if (m_question->getId() != 0) {
				m_vectorOfAnswerTextEdit[i]->setPlainText(m_question->getAnswers().at(i).getText()); // # answer
				m_vectorOfAnswerComboBox[i]->setCurrentIndex(m_vectorOfAnswerComboBox[i]->findData(m_question->getAnswers().at(i).isCorrect() ? 1 : 0)); // # true/false
			}
		}
		break;
	}
}

void ManageQuestion::numberOfAnswersChanged(int index)
{
	m_numberOfAnswers = ui.numberOfAnswersComboBox->currentData().toInt();
	putMultipleAnswerItems();
}

void ManageQuestion::onDomainClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.domainTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedDomain = Utils::findItemById(tableItem->text().toInt(), m_listOfDomains);
	m_selectedChapter = Chapter();
	populateChapterTable();
}

void ManageQuestion::onChapterClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.chapterTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedChapter = Utils::findItemById(tableItem->text().toInt(), m_listOfChapters);
}

void ManageQuestion::onSaveButtonClicked()
{

	try 
	{
		if (ui.titleInput->text()=="")
		{
			throw 100;
		}
		if (ui.questionInput->toPlainText() == "")
		{
			throw 200;
		}
		if (m_selectedDomain.getId() == 0)
		{
			throw 500;
		}
		if (m_selectedChapter.getId() == 0)
		{
			throw 400;
		}
		m_question->setChapter(m_selectedChapter);
		m_question->setTitle(ui.titleInput->text());
		m_question->setText(ui.questionInput->toPlainText());
		m_question->setType(m_questionType);
		if (m_questionType == Question::questionType::MultipleAnswer)
		{

			std::vector<Answer> listOfAnswers;
			for (int i = 0; i < m_vectorOfAnswerTextEdit.size(); ++i)
			{
				if (m_vectorOfAnswerTextEdit.at(i)->toPlainText() == "")
				{
					throw 300;
				}
				Answer answer(m_vectorOfAnswerTextEdit.at(i)->toPlainText(), m_vectorOfAnswerComboBox.at(i)->currentData().toInt());
				listOfAnswers.push_back(answer);
			}
			m_question->setAnswers(listOfAnswers);
		}
		else
		{	
			if (ui.answerInput->text() == "")
			{
				throw 300;
			}
			m_question->setAnswerValue(ui.answerInput->text());
		}

		m_question->saveToDB();
		QMessageBox::information(this, "", "Question saved!", QMessageBox::Button::Ok);
		this->close();
	}
	catch (int error)
	{
		switch (error)
		{
		case 100:
			QMessageBox::warning(this, "", "Title can't be empty!", QMessageBox::Button::Ok);
			break;
		case 200:
			QMessageBox::warning(this, "", "Question body can't be empty!", QMessageBox::Button::Ok);
			break;
		case 300:
			QMessageBox::warning(this, "", "Answer/Answers can't be empty!", QMessageBox::Button::Ok);
			break;
		case 400:
			QMessageBox::warning(this, "", "You have to choose a chapter!", QMessageBox::Button::Ok);
			break;
		case 500:
			QMessageBox::warning(this, "", "You have to choose a domain!", QMessageBox::Button::Ok);
			break;
		}
	}
}

void ManageQuestion::onDeleteButtonClicked()
{
	m_question->removeFromDB();
	QMessageBox::information(this, "", "Question was deleted!", QMessageBox::Button::Ok);
	this->close();
}

void ManageQuestion::putMultipleAnswerItems()
{
	// delete existing answers items
	for (int i = 0; i < m_vectorOfAnswerLayouts.size(); ++i)
	{
		m_vectorOfAnswerTextEdit[i]->setVisible(false);
		m_vectorOfAnswerComboBox[i]->setVisible(false);
		m_vectorOfAnswerLayouts[i]->removeWidget(m_vectorOfAnswerTextEdit[i]);
		m_vectorOfAnswerLayouts[i]->removeWidget(m_vectorOfAnswerComboBox[i]);
		ui.leftVerticalLayout->removeItem(m_vectorOfAnswerLayouts[i]);
	}
	m_vectorOfAnswerComboBox.clear();
	m_vectorOfAnswerLayouts.clear();
	m_vectorOfAnswerTextEdit.clear();

	// add new answer items
	for (int i = 0; i < m_numberOfAnswers; ++i)
	{
		m_vectorOfAnswerLayouts.push_back(new QHBoxLayout); // add layout
		m_vectorOfAnswerTextEdit.push_back(new QPlainTextEdit); // add textbox
		m_vectorOfAnswerComboBox.push_back(new QComboBox); // add combobox
		m_vectorOfAnswerComboBox[i]->addItem("True", QVariant(1));
		m_vectorOfAnswerComboBox[i]->addItem("False", QVariant(0));

		m_vectorOfAnswerLayouts[i]->addWidget(m_vectorOfAnswerTextEdit[i]); // add textbox to layout
		m_vectorOfAnswerLayouts[i]->addWidget(m_vectorOfAnswerComboBox[i]); // add combobox to layout
		ui.leftVerticalLayout->addLayout(m_vectorOfAnswerLayouts[i]); // add layout to left layout in page
	}
}

void ManageQuestion::singleAnswerRadioSelected()
{
	m_questionType = Question::questionType::SingleAnswer;
	ui.answerLabel->setText("Short answer");
	ui.answerInput->setVisible(true);
	ui.numberOfAnswersLabel->setVisible(false);
	ui.numberOfAnswersComboBox->setVisible(false);
	for (int i = 0; i < m_numberOfAnswers; ++i)
	{
		m_vectorOfAnswerTextEdit[i]->setVisible(false);
		m_vectorOfAnswerComboBox[i]->setVisible(false);
	}
}

void ManageQuestion::multipleAnswerRadioSelected()
{
	m_questionType = Question::questionType::MultipleAnswer;
	ui.answerLabel->setText("Answers");
	ui.answerInput->setVisible(false); 
	ui.numberOfAnswersLabel->setVisible(true);
	ui.numberOfAnswersComboBox->setVisible(true);
	for (int i = 0; i < m_numberOfAnswers; ++i)
	{
		m_vectorOfAnswerTextEdit[i]->setVisible(true);
		m_vectorOfAnswerComboBox[i]->setVisible(true);
	}
}

ManageQuestion::~ManageQuestion()
{
}

