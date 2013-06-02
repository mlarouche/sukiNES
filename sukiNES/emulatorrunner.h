#pragma once

// Qt includes
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

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

class EmulatorRunner : public QThread
{
	Q_OBJECT
public:
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

signals:
	void cpuUpdated(sukiNES::Cpu* cpu);

public slots:
	void powerOn();
	void reset();
	void stopEmulation();
	void resumeEmulation();

protected:
	virtual void run() override;

private slots:
	void sendCpuUpdated();

private:
	sukiNES::Cpu _cpu;
	sukiNES::GamePak _gamePak;
	sukiNES::MainMemory _mainMemory;
	sukiNES::PPU _ppu;

	mutable QMutex _emulationMutex;

	bool _isThreadRunning;
	bool _isEmulationRunning;
};