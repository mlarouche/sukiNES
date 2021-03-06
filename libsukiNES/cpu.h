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
	class InputIO;
	class PPU;

	class Cpu
	{
	public:
		Cpu();
		~Cpu();

		void powerOn();
		void reset();

		void executeOpcode();

		void setMainMemory(IMemory* memory)
		{
			_memory = memory;
		}

		void setPPU(PPU* ppu)
		{
			_ppu = ppu;
		}

		void setInputIO(InputIO* io)
		{
			_inputIO = io;
		}

		word programCounter() const
		{
			return _registers.ProgramCounter;
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

		byte readMemory(word address);
		void writeMemory(word address, byte value);

		void push(byte value);
		void push(word value);

		byte popByte();
		word popWord();

		CpuRegisters getRegisters() const
		{
			return _registers;
		}

		void tick();

		template<int>
		friend struct Flag;

		template<int>
		friend struct Register;

		friend struct NextByte;
		friend struct NextWord;

		template<class A, class B>
		friend struct RTI;

#ifdef SUKINES_DEBUG
		int _totalTick;
#endif

	private:
		void dmaCopy(byte memoryPage);
		void doIrq(word vectorAddress);

		template<byte Opcode, class Instruction>
		void registerOpcode()
		{
			_instructions[Opcode] = std::function<void(Cpu*)>(&Instruction::execute);
		}

		void _setupInstructions();

	private:
		CpuRegisters _registers;
		IMemory* _memory;
		PPU* _ppu;

		byte _inputStrobe;
		byte _buttonStatus[2];
		byte _inputReadCounter[2];
		InputIO* _inputIO;

		bool _nmiOccured;
		bool _insideIrq;

		std::function<void(Cpu*)> _instructions[256];
	};
}