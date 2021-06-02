#include <vector>
#include <QShortcut>
#include <fstream>

#include "QuestionBank.h"
#include "UserPanel.h"
#include "DBOperations.h"
#include "Answer.h"
#include "Utils.h"
#include "../Logging/Logging.h"

QuestionBank::QuestionBank(std::shared_ptr<User> user, QWidget* parent)
	: QWidget(parent),
	m_connectedUser(user),
	m_selectedChapter(),
	m_selectedDomain()
{
	ui.setupUi(this);

	// shortcuts
	QShortcut* returnShortcut = new QShortcut(QKeySequence("Return"), this);
	connect(returnShortcut, SIGNAL(activated()), this, SLOT(onReturnButtonClicked()));
	QShortcut* deleteShortcut = new QShortcut(QKeySequence("Del"), this);
	connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onDelButtonClicked()));
	QShortcut* refreshShortcut = new QShortcut(QKeySequence("F5"), this);
	connect(refreshShortcut, SIGNAL(activated()), this, SLOT(onF5()));

	// domain
	connect(ui.searchDomainInput, SIGNAL(textEdited(const QString&)), SLOT(onSearchDomainInputChanged()));
	connect(ui.domainTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onDomainClicked(int, int)));
	connect(ui.addDomainButton, SIGNAL(clicked()), SLOT(onAddDomainButtonClicked()));

	// chapter
	connect(ui.chapterTableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(onChapterClicked(int, int)));
	connect(ui.addChapterButton, SIGNAL(clicked()), SLOT(onAddChapterButtonClicked()));

	// question
	connect(returnShortcut, SIGNAL(activated()), SLOT(onSearchQuestionInputChanged()));
	connect(ui.questionTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onQuestionDoubleClicked(int, int)));
	connect(ui.addQuestionButton, SIGNAL(clicked()), SLOT(onAddQuestionButtonClicked()));

	// initialize UI
	populateDomainTable();
	populateQuestionTable();
}


void QuestionBank::getDomainList(const QString& searchedValue)
{
	m_listOfDomains.clear();
	QString sql = "SELECT * from domain where name LIKE :name";
	std::pair<QString, QString> pairName(":name", QString("%%1%").arg(searchedValue));
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairName });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Domain domain(id, name);
		m_listOfDomains.push_back(domain);
	}
}

void QuestionBank::getChapterList()
{
	m_listOfChapters.clear();
	QString sql = "SELECT * from chapter where domainId = :domainId";
	std::pair<QString, int> pairDomainId(":domainId", m_selectedDomain.getId());
	QSqlQuery query = DBOperations::selectFromTable(sql, { pairDomainId });

	while (query.next())
	{
		int id = query.value(0).toInt();
		QString name = query.value("name").toString();

		Chapter chapter(id, name, m_selectedDomain);
		m_listOfChapters.push_back(chapter);
	}
}

void QuestionBank::onSearchDomainInputChanged()
{
	populateDomainTable();
}

void QuestionBank::populateDomainTable()
{
	getDomainList(ui.searchDomainInput->text());

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

void QuestionBank::onDomainClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.domainTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedDomain = Utils::findItemById(tableItem->text().toInt(), m_listOfDomains);
	populateChapterTable();
}

void QuestionBank::onChapterClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.chapterTableWidget->item(row, 0); // get id from column 0 of table
	m_selectedChapter = Utils::findItemById(tableItem->text().toInt(), m_listOfChapters);
}

void QuestionBank::onQuestionDoubleClicked(int row, int column)
{
	QTableWidgetItem* tableItem = ui.questionTableWidget->item(row, 0); // get id from column 0 of table
	Question selectedQuestion = Utils::findItemById(tableItem->text().toInt(), m_listOfQuestions);
	ManageQuestion* manageQuestion = new ManageQuestion(std::make_shared<Question>(selectedQuestion));
	manageQuestion->show();
}

void QuestionBank::onAddDomainButtonClicked()
{
	try
	{
		if (ui.addDomainInput->text() == "")
		{
			throw 100;
		}
		Domain newDomain(ui.addDomainInput->text());
		if (newDomain.save())
		{
			QMessageBox::information(this, "", "The domain was added succesfully!", QMessageBox::Button::Ok);
			Utils::logActivity(m_connectedUser->getEmail() + " added " + newDomain.getName() + " domain!", Logger::Level::Info);
			ui.addDomainInput->setText("");
			populateDomainTable();
		}
		else
		{
			throw 200;
		}
	}
	catch (int error)
	{
		switch (error)
		{
		case 100:
			QMessageBox::critical(this, "Error", "You didn't type anything!", QMessageBox::Button::Ok);
			break;
		case 200:
			QMessageBox::critical(this, "Error", "The domain already exists!", QMessageBox::Button::Ok);
			break;
		}
	}
}

void QuestionBank::populateChapterTable()
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
	std::for_each(m_listOfChapters.begin(), m_listOfChapters.end(), [&](Chapter& chapter)
		{
			ui.chapterTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(chapter.getId()))); // id column
			ui.chapterTableWidget->setItem(rowCount, 1, new QTableWidgetItem(chapter.getName())); // name column
			rowCount++;
		});

	ui.chaptersForLabel->setText("Chapters for " + m_selectedDomain.getName());
}

void QuestionBank::onAddChapterButtonClicked()
{
	try
	{
		if (ui.addChapterInput->text() == "")
		{
			throw 100;
		}
		if (m_selectedDomain.getId() == 0)
		{
			throw 200;
		}
		Chapter newChapter(0, ui.addChapterInput->text(), m_selectedDomain);
		if (newChapter.save())
		{
			QMessageBox::information(this, "", "The chapter was added succesfully!", QMessageBox::Button::Ok);
			Utils::logActivity(m_connectedUser->getEmail() + " added " + newChapter.getName() + " chapter for " + m_selectedDomain.getName() + " domain!", Logger::Level::Info);
			ui.addChapterInput->setText("");
			populateChapterTable();
		}
		else
		{
			throw 300;
		}
	}
	catch (int error)
	{
		switch (error)
		{
		case 100:
			QMessageBox::critical(this, "Error", "You didn't type anything!", QMessageBox::Button::Ok);
			break;
		case 200:
			QMessageBox::critical(this, "Error", "You didn't choose anything!", QMessageBox::Button::Ok);
			break;
		case 300:
			QMessageBox::critical(this, "Error", "The chapter already exists!", QMessageBox::Button::Ok);
			break;
		}
	}
}

void QuestionBank::onReturnButtonClicked()
{
	if (ui.addDomainInput->hasFocus())
	{
		onAddDomainButtonClicked();
	}
	else if (ui.addChapterInput->hasFocus())
	{
		onAddChapterButtonClicked();
	}
}

void QuestionBank::onDelButtonClicked()
{
	if (ui.chapterTableWidget->hasFocus())
	{
		QMessageBox::information(this, "", "The chapter " + m_selectedChapter.getName() + " was deleted succesfully!", QMessageBox::Button::Ok);
		m_selectedChapter.remove();
		populateChapterTable();
	}
	else if (ui.domainTableWidget->hasFocus())
	{
		QMessageBox::information(this, "", "The domain " + m_selectedDomain.getName() + " was deleted succesfully!", QMessageBox::Button::Ok);
		m_selectedDomain.remove();
		populateDomainTable();
	}
}

void QuestionBank::onF5()
{
	populateQuestionTable();
}

// QUESTIONS PART START

void QuestionBank::getQuestionsList(const QString& searchedValue)
{
	m_listOfQuestions.clear();

	QString sql = "SELECT * FROM domain d, chapter c, question q WHERE (q.chapterId=c.id AND c.domainId = d.id AND teacherId= :teacherId) AND (q.title LIKE :searchedValue OR q.text LIKE :searchedValue OR c.name LIKE :searchedValue OR d.name LIKE :searchedValue)";
	std::pair<QString, QString> pairTeacherId(":teacherId", QString::number(m_connectedUser->getId()));
	std::pair<QString, QString> pairSearchedValue(":searchedValue", QString("%%1%").arg(searchedValue));

	QSqlQuery query = DBOperations::selectFromTable(sql, { pairTeacherId, pairSearchedValue });

	while (query.next())
	{
		int question_id = query.value("q.id").toInt();
		int question_teacherId = query.value("q.teacherId").toInt();
		int question_chapterId = query.value("c.id").toInt();
		QString question_title = query.value("q.title").toString();
		QString question_text = query.value("q.text").toString();
		int question_typeValue = query.value("q.type").toInt();
		QString question_answer = query.value("q.answer").toString();
		Question::questionType question_type;
		switch (question_typeValue)
		{
		case 0:
			question_type = Question::questionType::SingleAnswer;
			break;
		case 1:
			question_type = Question::questionType::MultipleAnswer;
			break;
		}

		int domain_id = query.value("d.id").toInt();
		QString domain_name = query.value("d.name").toString();
		Domain domain(domain_id, domain_name);

		int chapter_id = query.value("c.id").toInt();
		int chapter_domainId = query.value("c.domainId").toInt();
		QString chapter_name = query.value("c.name").toString();
		Chapter chapter(chapter_id, chapter_name, domain);

		Question question(question_id, question_teacherId, chapter, question_title, question_text, question_type, question_answer);
		m_listOfQuestions.push_back(question);
	}
}

void QuestionBank::populateQuestionTable()
{
	getQuestionsList(ui.searchQuestionInput->text());

	ui.questionTableWidget->clearSelection();
	ui.questionTableWidget->clearContents();
	ui.questionTableWidget->setRowCount(0);

	ui.questionTableWidget->setRowCount(m_listOfQuestions.size());
	ui.questionTableWidget->setColumnCount(5);
	ui.questionTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.questionTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.questionTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.questionTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	ui.questionTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	ui.questionTableWidget->verticalHeader()->hide(); // hide vertical header
	QStringList tableHeader;
	tableHeader << "#" << "Title" << "Domain" << "Chapter" << "Type"; 
	ui.questionTableWidget->setHorizontalHeaderLabels(tableHeader);

	int rowCount = 0;
	std::for_each(m_listOfQuestions.begin(), m_listOfQuestions.end(), [&](Question& question)
		{
			ui.questionTableWidget->setSortingEnabled(false);
			ui.questionTableWidget->setItem(rowCount, 0, new QTableWidgetItem(QString::number(question.getId()))); // id column
			ui.questionTableWidget->setItem(rowCount, 1, new QTableWidgetItem(question.getTitle())); // title column
			ui.questionTableWidget->setItem(rowCount, 2, new QTableWidgetItem(question.getChapter().getDomain().getName())); // domain column
			ui.questionTableWidget->setItem(rowCount, 3, new QTableWidgetItem(question.getChapter().getName())); // chapter column
			ui.questionTableWidget->setItem(rowCount, 4, new QTableWidgetItem(question.getTypeAsString())); // type column
			rowCount++;
			ui.questionTableWidget->setSortingEnabled(true);
		});

}

void QuestionBank::onSearchQuestionInputChanged()
{
	populateQuestionTable();
}

void QuestionBank::onAddQuestionButtonClicked()
{
	Question newQuestion(m_connectedUser->getId());
	ManageQuestion* manageQuestion = new ManageQuestion(std::make_shared<Question>(newQuestion));
	manageQuestion->show();
}

QuestionBank::~QuestionBank()
{
}