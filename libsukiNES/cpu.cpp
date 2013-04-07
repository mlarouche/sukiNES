#include "cpu.h"

// Local includes
#include "assert.h"
#include "memory.h"

namespace sukiNES
{
	Cpu::Cpu()
	: _memory(nullptr)
	{
		_registers.StackPointer = 0xFF;
		_registers.A = 0;
		_registers.X = 0;
		_registers.Y = 0;
		_registers.ProgramCounter = 0;
		_registers.ProcessorStatus.raw = 0;
		_registers.ProcessorStatus.Unused = true;
	}

	Cpu::~Cpu()
	{
	}

	void Cpu::executeOpcode()
	{
		sukiAssertWithMessage(_memory, "Please setup a memory for the CPU");
	}
}
