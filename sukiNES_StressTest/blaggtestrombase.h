#pragma once

// sukiNES includes
#include <cpu.h>
#include <gamepak.h>
#include <mainmemory.h>
#include <ppu.h>

// StressTest includes
#include "test.h"

class BlaggTestRomBase : public StressTest::Test
{
public:
	BlaggTestRomBase();
	~BlaggTestRomBase();

	virtual bool run();

protected:
	void setRomFilename(const char* filename)
	{
		_romFilename = filename;
	}
	void setFinalProgramCounter(word pc)
	{
		_finalProgramCounter = pc;
	}
	void setResultRamAddress(word address)
	{
		_resultRamAddress = address;
	}

	virtual void failureMessage() = 0;

protected:
	sukiNES::Cpu _cpu;
	sukiNES::MainMemory _memory;
	sukiNES::GamePak _gamePak;
	sukiNES::PPU _ppu;

	byte _result;

	const char* _romFilename;
	word _finalProgramCounter;
	word _resultRamAddress;
};