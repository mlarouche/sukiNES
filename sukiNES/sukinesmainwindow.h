#pragma once

#include <QtWidgets/QMainWindow>

class CpuRegisterDockWidget;
class EmulatorRunner;
class EmulatorWidget;

class QAction;
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
	void toggleCpuRegisterDockWidget(bool checked);

private:
	void _init();
	void _initMenu();
	void _initCentralWidget();

private:
	CpuRegisterDockWidget* _cpuRegisterDockWidget;
	EmulatorRunner* _emulatorRunner;
	EmulatorWidget* _emulatorWidget;

	QAction* _actionCpuRegister;
};

