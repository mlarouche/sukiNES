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

sukiNESMainWindow::sukiNESMainWindow(QWidget *parent)
: QMainWindow(parent)
, _cpuRegisterDockWidget(nullptr)
, _emulatorRunner(nullptr)
, _emulatorWidget(nullptr)
, _actionCpuRegister(nullptr)
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
	QObject::connect(powerOnAction, &QAction::triggered, _emulatorRunner, &EmulatorRunner::powerOn);
	emulationMenu->addAction(powerOnAction);

	QAction* resetAction = new QAction(tr("Reset"), this);
	resetAction->setShortcut(Qt::Key_F6);
	QObject::connect(resetAction, &QAction::triggered, _emulatorRunner, &EmulatorRunner::reset);
	emulationMenu->addAction(resetAction);

	QMenu* debugMenu = menuBar()->addMenu(tr("Debug"));

	_actionCpuRegister = new QAction(tr("CPU registers"), this);
	_actionCpuRegister->setCheckable(true);
	QObject::connect(_actionCpuRegister, &QAction::triggered, this, &sukiNESMainWindow::toggleCpuRegisterDockWidget);
	debugMenu->addAction(_actionCpuRegister);

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

	_emulatorRunner->stopEmulation();

	QString romFilename = QFileDialog::getOpenFileName(this, tr("Open NES ROM"), QString(), tr("NES file (*.nes)"));
	if(!romFilename.isEmpty())
	{
		if (_emulatorRunner->loadRom(romFilename))
		{
			_emulatorWidget->clearScreen();

			QFileInfo fileInfo(romFilename);

			setWindowTitle(tr("sukiNES - %1").arg(fileInfo.fileName()));

			_emulatorRunner->powerOn();
			if (!_emulatorRunner->isRunning())
			{
				_emulatorRunner->start();
			}
		}
		else
		{
			if (wasEmulationRunning)
			{
				_emulatorRunner->resumeEmulation();
			}
		}
	}
	else
	{
		if (wasEmulationRunning)
		{
			_emulatorRunner->resumeEmulation();
		}
	}
}

void sukiNESMainWindow::about()
{
	QMessageBox::about(this, tr("sukiNES"), tr("sukiNES 0.1\nBy Michael Larouche <michael.larouche@gmail.com>\n\nhttps://github.com/mlarouche/sukiNES"));
}

void sukiNESMainWindow::toggleCpuRegisterDockWidget(bool checked)
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

	if (checked)
	{
		addDockWidget(Qt::RightDockWidgetArea, _cpuRegisterDockWidget);
	}
	else
	{
		_cpuRegisterDockWidget->close();
	}
}