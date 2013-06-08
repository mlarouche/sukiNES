#pragma once

// Qt includes
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QQueue>

// sukiNES includes
#include <cpu.h>
#include <gamepak.h>
#include <mainmemory.h>
#include <ppu.h>

namespace sukiNES
{
	class PPUIO;
	class InputIO;
}

class QTimer;

class EmulatorRunner : public QThread
{
	Q_OBJECT
public:
	enum class Command
	{
		PowerOn,
		Reset,
		StopEmulation,
		ResumeEmulation,
		Step
	};

	EmulatorRunner(QObject* parent = nullptr);

	void setPPUIO(sukiNES::PPUIO* io);
	void setInputIO(sukiNES::InputIO* io);

	bool loadRom(const QString& romFilename);

	void quitThread();

	bool isEmulationRunning() const
	{
		QMutexLocker autoLock(&_emulationMutex);
		return _isEmulationRunning;
	}

	void doCommand(Command value);

signals:
	void cpuUpdated(sukiNES::Cpu* cpu);
	void ppuUpdated(sukiNES::PPU* ppu);

protected:
	virtual void run() override;

private slots:
	void sendCpuUpdated();
	void sendPpuUpdated();

private:
	sukiNES::Cpu _cpu;
	sukiNES::GamePak _gamePak;
	sukiNES::MainMemory _mainMemory;
	sukiNES::PPU _ppu;

	mutable QMutex _emulationMutex;

	QQueue<Command> _commands;

	bool _isThreadRunning;
	bool _isEmulationRunning;

	QTimer* _tempTimer;
};