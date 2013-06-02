#include "emulatorrunner.h"

// Qt includes
#include <windows.h>

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
}

void EmulatorRunner::powerOn()
{
	if (!_gamePak.hasGamePak())
	{
		return;
	}

	stopEmulation();

	_cpu.powerOn();
	
	resumeEmulation();
}

void EmulatorRunner::reset()
{
	if (!_gamePak.hasGamePak())
	{
		return;
	}

	bool wasEmulationRunning = isEmulationRunning();

	stopEmulation();

	_cpu.reset();

	if (wasEmulationRunning)
	{
		resumeEmulation();
	}
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

void EmulatorRunner::stopEmulation()
{
	QMutexLocker autoLock(&_emulationMutex);
	_isEmulationRunning = false;
}

void EmulatorRunner::resumeEmulation()
{
	QMutexLocker autoLock(&_emulationMutex);
	_isEmulationRunning = true;
}

void EmulatorRunner::quitThread()
{
	_isThreadRunning = false;
}

void EmulatorRunner::run()
{
	while(_isThreadRunning)
	{
		if (isEmulationRunning())
		{
			_cpu.executeOpcode();
		}
	}
}