#include "cpu.h"

namespace sukiNES
{
	Cpu::Cpu()
	{
		_registers.StackPointer = 0xFF;
		_registers.A = 0;
		_registers.X = 0;
		_registers.Y = 0;
		_registers.ProgramCounter = 0;
		_registers.ProcessorStatus.raw = 0;
	}

	Cpu::~Cpu()
	{
	}

	void Cpu::executeOpcode()
	{
	}
}
