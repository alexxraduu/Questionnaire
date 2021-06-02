#pragma once

#include <QMainWindow>
#include <QMessageBox>

#include "ui_UserPanel.h"
#include "User.h"
#include "Test.h"

class UserPanel : public QMainWindow
{
    Q_OBJECT

public:
    UserPanel(std::shared_ptr<User> connectedUser, QWidget* parent = Q_NULLPTR);
    ~UserPanel();
    void initializeUI();

private:
    Ui::UserPanel ui;
    std::shared_ptr<User> m_connectedUser;
    std::vector<Test> m_listOfTests;
    std::map<int, QString> m_teacherMap;

private:
    void populateTestTable();
    void getTestsList();

private slots:
    void onLogoutButtonClicked();

    // Teacher panel
    void onQuestionBankButtonClicked();
    void onCreateTestButtonClicked();
    void onSeeTestResultsButtonClicked();
    void onManageCandidatesButtonClicked();

    // Administrator panel
    void onManageUsersButtonClicked();

    // Candidate panel
    void onTestDoubleClicked(int row, int column);
    void onF5();
};