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
			RegBit<6> Overflow;
			RegBit<7> Negative;
		} ProcessorStatus;
	};

	class Cpu
	{
	public:
		Cpu();
		~Cpu();

		void executeOpcode();

		CpuRegisters getRegisters() const
		{
			return _registers;
		}

	private:
		CpuRegisters _registers;
	};
}