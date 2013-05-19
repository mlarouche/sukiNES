#include "blaggtestrombase.h"

// sukiNES includes
#include <inesreader.h>

BlaggTestRomBase::BlaggTestRomBase()
: _result(0)
, _finalProgramCounter(0)
, _resultRamAddress(0xF0)
{
	// Setup MainMemory
	_memory.setGamepakMemory(&_gamePak);
	_memory.setPpuMemory(&_ppu);

	// Setup CPU
	_cpu.setMainMemory(&_memory);
	_cpu.setPPU(&_ppu);

	// Setup PPU
	_ppu.setGamePak(&_gamePak);
}

BlaggTestRomBase::~BlaggTestRomBase()
{
}

bool BlaggTestRomBase::run()
{
	sukiNES::iNESReader nesReader;
	nesReader.setGamePak(&_gamePak);
	nesReader.setPpu(&_ppu);

	if (!nesReader.read(_romFilename))
	{
		_generateFailureMessage("Cannot open NES file %s", _romFilename);
		return false;
	}

	_cpu.powerOn();

	while((int)_cpu.programCounter() != _finalProgramCounter)
	{
		_cpu.executeOpcode();
	}

	_result = _memory.read(_resultRamAddress);

	if (_result != 1)
	{
		failureMessage();
		return false;
	}

	return true;
}
