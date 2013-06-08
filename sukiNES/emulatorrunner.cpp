#include "emulatorrunner.h"

// Qt includes
#include <QtCore/QTimer>

// sukiNES includes
#include <ppuio.h>
#include <inputio.h>
#include <inesreader.h>

EmulatorRunner::EmulatorRunner(QObject* parent)
: QThread(parent)
, _isThreadRunning(true)
, _isEmulationRunning(false)
{
	_mainMemory.setGamepakMemory(&_gamePak);
	_mainMemory.setPpuMemory(&_ppu);

	_ppu.setGamePak(&_gamePak);

	_cpu.setPPU(&_ppu);
	_cpu.setMainMemory(&_mainMemory);

	_tempTimer = new QTimer(this);
	QObject::connect(_tempTimer, &QTimer::timeout, this, &EmulatorRunner::sendCpuUpdated);
	QObject::connect(_tempTimer, &QTimer::timeout, this, &EmulatorRunner::sendPpuUpdated);
}

void EmulatorRunner::setInputIO(sukiNES::InputIO* io)
{
	_cpu.setInputIO(io);
}

void EmulatorRunner::setPPUIO(sukiNES::PPUIO* io)
{
	_ppu.setIO(io);
}

bool EmulatorRunner::loadRom(const QString& romFilename)
{
	sukiNES::iNESReader nesReader;
	nesReader.setGamePak(&_gamePak);
	nesReader.setPpu(&_ppu);

	return nesReader.read(romFilename.toLocal8Bit().constData());
}

void EmulatorRunner::quitThread()
{
	_isThreadRunning = false;
}

void EmulatorRunner::doCommand(EmulatorRunner::Command command)
{
	QMutexLocker locker(&_emulationMutex);
	_commands.enqueue(command);
}

void EmulatorRunner::sendCpuUpdated()
{
	emit cpuUpdated(&_cpu);
}

void EmulatorRunner::sendPpuUpdated()
{
	emit ppuUpdated(&_ppu);
}

void EmulatorRunner::run()
{
	while(_isThreadRunning)
	{
		_emulationMutex.lock();

		while(!_commands.isEmpty())
		{
			auto commandToDo = _commands.dequeue();

			if (!_gamePak.hasGamePak())
			{
				continue;
			}

			switch(commandToDo)
			{
				case Command::PowerOn:
					_cpu.powerOn();
					_isEmulationRunning = true;
					break;
				case Command::Reset:
					_cpu.reset();
					break;
				case Command::ResumeEmulation:
					_isEmulationRunning = true;
					break;
				case Command::StopEmulation:
					_isEmulationRunning = false;
					break;
				case Command::Step:
					_isEmulationRunning = false;
					_cpu.executeOpcode();
					sendCpuUpdated();
					sendPpuUpdated();
					break;
			}
		}

		_emulationMutex.unlock();

		if (isEmulationRunning())
		{
			_cpu.executeOpcode();
		}
	}
}
