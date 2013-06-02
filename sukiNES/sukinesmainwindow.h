#pragma once

#include <QtWidgets/QMainWindow>

class EmulatorRunner;
class EmulatorWidget;

class QCloseEvent;

class sukiNESMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	sukiNESMainWindow(QWidget *parent = nullptr);
	~sukiNESMainWindow();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private slots:
	void openROM();
	void about();

private:
	void _init();
	void _initMenu();
	void _initCentralWidget();

private:
	EmulatorRunner* _emulatorRunner;
	EmulatorWidget* _emulatorWidget;
};

