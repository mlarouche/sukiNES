#pragma once

#include <functional>

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

		void push(byte value);
		void push(word value);

		CpuRegisters getRegisters() const
		{
			return _registers;
		}

		template<int>
		friend struct Flag;

		template<int>
		friend struct Register;

		template<class Address>
		friend struct ToAddress;

		template<class Address>
		friend struct RelativeAddress;

		template<class A, class B>
		friend struct JSR;

		friend struct NextByte;
		friend struct NextWord;

	private:
		template<byte Opcode, class Instruction>
		void registerOpcode()
		{
			_instructions[Opcode] = std::function<void(Cpu*)>(&Instruction::execute);
		}

		void _setupInstructions();

	private:
		CpuRegisters _registers;
		IMemory* _memory;

		std::function<void(Cpu*)> _instructions[256];
	};
}