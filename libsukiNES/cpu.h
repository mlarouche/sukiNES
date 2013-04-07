#pragma once

namespace sukiNES
{
	struct CpuRegisters
	{
		byte A;
		byte X;
		byte Y;
		byte StackPointer;
		word ProgramCounter;

		union
		{
			byte raw;
			RegBit<0> Carry;
			RegBit<1> Zero;
			RegBit<2> InterruptDisabled;
			RegBit<3> Decimal;
			RegBit<4> Break;
			RegBit<5> Unused;
			RegBit<6> Overflow;
			RegBit<7> Negative;
		} ProcessorStatus;
	};

	class IMemory;

	class Cpu
	{
	public:
		Cpu();
		~Cpu();

		void setMainMemory(IMemory* memory)
		{
			_memory = memory;
		}

		void setProgramCounter(word address)
		{
			_registers.ProgramCounter = address;
		}

		void disableInterrupt()
		{
			_registers.ProcessorStatus.InterruptDisabled = true;
		}

		void enableInterrupt()
		{
			_registers.ProcessorStatus.InterruptDisabled = false;
		}

		void executeOpcode();

		CpuRegisters getRegisters() const
		{
			return _registers;
		}

	private:
		CpuRegisters _registers;
		IMemory* _memory;
	};
}