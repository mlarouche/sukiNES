#include "sukinesmainwindow.h"

// Qt includes
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>

// Local includes
#include "cpuregisterdockwidget.h"
#include "emulatorrunner.h"
#include "emulatorwidget.h"
#include "ppudebuginfodialog.h"
#include "ppuvideodialog.h"

sukiNESMainWindow::sukiNESMainWindow(QWidget *parent)
: QMainWindow(parent)
, _cpuRegisterDockWidget(nullptr)
, _emulatorRunner(nullptr)
, _emulatorWidget(nullptr)
, _ppuDebugInfoDialog(nullptr)
, _ppuVideoDialog(nullptr)
, _actionCpuRegister(nullptr)
, _actionPPUDebugInfo(nullptr)
, _actionPPUVideo(nullptr)
{
	_emulatorRunner = new EmulatorRunner(this);

	_init();
}

sukiNESMainWindow::~sukiNESMainWindow()
{
}

void sukiNESMainWindow::_init()
{
	setWindowTitle(tr("sukiNES"));

	_initMenu();

	_initCentralWidget();
}

void sukiNESMainWindow::_initMenu()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

	QAction *fileLoadAction = new QAction(tr("Open ROM..."), this);
	fileLoadAction->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(fileLoadAction, &QAction::triggered, this, &sukiNESMainWindow::openROM);
	fileMenu->addAction(fileLoadAction);

	fileMenu->addSeparator();

	QAction *quitAction = new QAction(tr("Quit"), this);
	quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
	quitAction->setMenuRole(QAction::QuitRole);
	QObject::connect(quitAction, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
	fileMenu->addAction(quitAction);

	QMenu* emulationMenu = menuBar()->addMenu(tr("Emulation"));

	QAction* powerOnAction = new QAction(tr("Power on"), this);
	powerOnAction->setShortcut(Qt::Key_F5);
	QObject::connect(powerOnAction, &QAction::triggered, [this] () {
		_emulatorRunner->doCommand(EmulatorRunner::Command::PowerOn);
	});

	emulationMenu->addAction(powerOnAction);

	QAction* resetAction = new QAction(tr("Reset"), this);
	resetAction->setShortcut(Qt::Key_F6);
	QObject::connect(resetAction, &QAction::triggered, [this] () {
		_emulatorRunner->doCommand(EmulatorRunner::Command::Reset);
	});
	emulationMenu->addAction(resetAction);

	emulationMenu->addSeparator();

	QAction* pauseAction = new QAction(tr("Pause"), this);
	pauseAction->setShortcut(Qt::Key_F7);
	QObject::connect(pauseAction, &QAction::triggered, [this] () {
		_emulatorRunner->doCommand(EmulatorRunner::Command::StopEmulation);
	});
	emulationMenu->addAction(pauseAction);

	QAction* resumeAction = new QAction(tr("Resume"), this);
	resumeAction->setShortcut(Qt::Key_F8);
	QObject::connect(resumeAction, &QAction::triggered, [this] () {
		_emulatorRunner->doCommand(EmulatorRunner::Command::ResumeEmulation);
	});
	emulationMenu->addAction(resumeAction);

	QMenu* debugMenu = menuBar()->addMenu(tr("Debug"));

	QAction* debugStepAction = new QAction(tr("Step"), this);
	debugStepAction->setShortcut(Qt::Key_F10);
	QObject::connect(debugStepAction, &QAction::triggered, [this] () {
		_emulatorRunner->doCommand(EmulatorRunner::Command::Step);
		_emulatorWidget->callRepaint();
	});
	debugMenu->addAction(debugStepAction);

	debugMenu->addSeparator();

	_actionCpuRegister = new QAction(tr("CPU registers"), this);
	_actionCpuRegister->setCheckable(true);
	QObject::connect(_actionCpuRegister, &QAction::triggered, this, &sukiNESMainWindow::toggleCpuRegisterDockWidget);
	debugMenu->addAction(_actionCpuRegister);

	_actionPPUDebugInfo = new QAction(tr("PPU Debug Info"), this);
	_actionPPUDebugInfo->setCheckable(true);
	QObject::connect(_actionPPUDebugInfo, &QAction::triggered, this, &sukiNESMainWindow::togglePPUDebugInfoDialog);
	debugMenu->addAction(_actionPPUDebugInfo);

	_actionPPUVideo = new QAction(tr("PPU Video"), this);
	_actionPPUVideo->setCheckable(true);
	QObject::connect(_actionPPUVideo, &QAction::triggered, this, &sukiNESMainWindow::togglePPUVideoDialog);
	debugMenu->addAction(_actionPPUVideo);

	QMenu *helpMenu = menuBar()->addMenu( tr("&Help") );

	QAction* aboutAction = new QAction(tr("About sukiNES..."), this);
	QObject::connect(aboutAction, &QAction::triggered, this, &sukiNESMainWindow::about);
	helpMenu->addAction(aboutAction);
}

void sukiNESMainWindow::_initCentralWidget()
{
	_emulatorWidget = new EmulatorWidget(this);

	_emulatorRunner->setPPUIO(_emulatorWidget);
	_emulatorRunner->setInputIO(_emulatorWidget);

	setCentralWidget(_emulatorWidget);
}

void sukiNESMainWindow::closeEvent(QCloseEvent* event)
{
	if (_emulatorRunner->isRunning())
	{
		_emulatorRunner->quitThread();
	}
}

void sukiNESMainWindow::openROM()
{
	bool wasEmulationRunning = _emulatorRunner->isEmulationRunning();

	if (wasEmulationRunning)
	{
		_emulatorRunner->doCommand(EmulatorRunner::Command::StopEmulation);
	}

	QString romFilename = QFileDialog::getOpenFileName(this, tr("Open NES ROM"), QString(), tr("NES file (*.nes)"));
	if(!romFilename.isEmpty())
	{
		if (_emulatorRunner->loadRom(romFilename))
		{
			_emulatorWidget->clearScreen();

			QFileInfo fileInfo(romFilename);

			setWindowTitle(tr("sukiNES - %1").arg(fileInfo.fileName()));

			_emulatorRunner->doCommand(EmulatorRunner::Command::PowerOn);
			if (!_emulatorRunner->isRunning())
			{
				_emulatorRunner->start();
			}
		}
		else
		{
			if (wasEmulationRunning)
			{
				_emulatorRunner->doCommand(EmulatorRunner::Command::ResumeEmulation);
			}
		}
	}
	else
	{
		if (wasEmulationRunning)
		{
			_emulatorRunner->doCommand(EmulatorRunner::Command::ResumeEmulation);
		}
	}
}

void sukiNESMainWindow::about()
{
	QMessageBox::about(this, tr("sukiNES"), tr("sukiNES 0.1\nBy Michael Larouche <michael.larouche@gmail.com>\n\nhttps://github.com/mlarouche/sukiNES"));
}

void sukiNESMainWindow::toggleCpuRegisterDockWidget(bool isChecked)
{
	if (!_cpuRegisterDockWidget)
	{
		_cpuRegisterDockWidget = new CpuRegisterDockWidget(this);
		QObject::connect(_emulatorRunner, &EmulatorRunner::cpuUpdated, _cpuRegisterDockWidget, &CpuRegisterDockWidget::cpuUpdated);
		QObject::connect(_cpuRegisterDockWidget, &CpuRegisterDockWidget::onClosed, [this] () {
			_actionCpuRegister->setChecked(false);
			_cpuRegisterDockWidget->deleteLater();
			_cpuRegisterDockWidget = nullptr;
		} );
	}

	if (isChecked)
	{
		addDockWidget(Qt::RightDockWidgetArea, _cpuRegisterDockWidget);
	}
	else
	{
		_cpuRegisterDockWidget->close();
	}
}

void sukiNESMainWindow::togglePPUDebugInfoDialog(bool isChecked)
{
	if (!_ppuDebugInfoDialog)
	{
		_ppuDebugInfoDialog = new PPUDebugInfoDialog(this);

		QObject::connect(_emulatorRunner, &EmulatorRunner::ppuUpdated, _ppuDebugInfoDialog, &PPUDebugInfoDialog::updatePPUInfo);
		QObject::connect(_ppuDebugInfoDialog, &PPUDebugInfoDialog::finished, [this] (int) {
			_actionPPUDebugInfo->setChecked(false);
			_ppuDebugInfoDialog->deleteLater();
			_ppuDebugInfoDialog = nullptr;
		} );
	}

	if (isChecked)
	{
		_ppuDebugInfoDialog->show();
	}
	else
	{
		_ppuDebugInfoDialog->close();
	}
}

void sukiNESMainWindow::togglePPUVideoDialog(bool isChecked)
{
	if (!_ppuVideoDialog)
	{
		_ppuVideoDialog = new PPUVideoDialog(this);

		QObject::connect(_emulatorRunner, &EmulatorRunner::ppuUpdated, _ppuVideoDialog, &PPUVideoDialog::updatePPUInfo);
		QObject::connect(_ppuVideoDialog, &PPUVideoDialog::finished, [this] (int) {
			_actionPPUVideo->setChecked(false);
			_ppuVideoDialog->deleteLater();
			_ppuVideoDialog = nullptr;
		} );
	}

	if (isChecked)
	{
		_ppuVideoDialog->show();
	}
	else
	{
		_ppuVideoDialog->close();
	}
}
