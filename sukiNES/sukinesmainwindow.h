#pragma once

#include <QtWidgets/QMainWindow>

class CpuRegisterDockWidget;
class EmulatorRunner;
class EmulatorWidget;
class PPUDebugInfoDialog;

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
	void toggleCpuRegisterDockWidget(bool isChecked);
	void togglePPUDebugInfoDialog(bool isChecked);

private:
	void _init();
	void _initMenu();
	void _initCentralWidget();

private:
	CpuRegisterDockWidget* _cpuRegisterDockWidget;
	EmulatorRunner* _emulatorRunner;
	EmulatorWidget* _emulatorWidget;
	PPUDebugInfoDialog* _ppuDebugInfoDialog;

	QAction* _actionCpuRegister;
	QAction* _actionPPUDebugInfo;
};

