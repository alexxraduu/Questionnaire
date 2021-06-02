#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_LoginPanel.h"

class LoginPanel : public QMainWindow
{
	Q_OBJECT

public:
	LoginPanel(QWidget* parent = Q_NULLPTR);

private:
	Ui::LoginPanelClass ui;

private slots:
	void onLoginButtonClicked();
	void onShowPasswordChecked();
};