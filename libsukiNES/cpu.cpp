#include "cpu.h"

// Local includes
#include "assert.h"
#include "memory.h"

namespace sukiNES
{
	inline int TestZero(byte value)
	{
		return value == 0;
	}

	inline int TestNegative(byte value)
	{
		return (value & SUKINES_BIT(7)) ? true : false;
	}

	enum CpuFlags
	{
		Carry = 0,
		Zero = 1,
		InterruptDisabled = 2,
		Decimal = 3,
		Break = 4,
		Unused = 5,
		Overflow = 6,
		Negative = 7
	};

	template<int FlagBit>
	struct Flag
	{
		static inline byte read(Cpu* cpu)
		{
			return (cpu->_registers.ProcessorStatus.raw & SUKINES_BIT(FlagBit)) ? 1 : 0;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			switch(value)
			{
			case 1:
				cpu->_registers.ProcessorStatus.raw |= SUKINES_BIT(FlagBit);
				break;
			case 0:
				cpu->_registers.ProcessorStatus.raw &= ~SUKINES_BIT(FlagBit);
				break;
			}
		}
	};

	enum RegisterName
	{
		A,
		X,
		Y
	};

	template<int Reg>
	struct Register
	{
		static inline byte read(Cpu* cpu)
		{
			return 0;
		}

		static inline void write(Cpu* cpu, byte value)
		{
		}
	};

	template<>
	struct Register<A>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_registers.A;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.A = value;
		}
	};

	template<>
	struct Register<X>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_registers.X;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.X = value;
		}
	};

	template<>
	struct Register<Y>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_registers.Y;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.Y = value;
		}
	};

	template<class Address>
	struct ToAddress
	{
		static inline byte read(Cpu* cpu)
		{
			cpu->_memory->read(Address::read(cpu));
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_memory->write(Address::read(cpu), value);
		}
	};

	template<class Address>
	struct RelativeAddress
	{
		static inline word read(Cpu* cpu)
		{
			offset relativeByte = static_cast<offset>(Address::read(cpu));

			return static_cast<word>(cpu->_registers.ProgramCounter + relativeByte + 1);
		}
	};

	struct NextByte
	{
		static inline byte read(Cpu* cpu)
		{
			cpu->_registers.ProgramCounter++;
			return cpu->_memory->read(cpu->_registers.ProgramCounter);
		}
	};

	struct NextWord
	{
		static inline word read(Cpu* cpu)
		{
			byte lowByte = cpu->_memory->read(++cpu->_registers.ProgramCounter);
			byte highByte = cpu->_memory->read(++cpu->_registers.ProgramCounter);

			word readWord;
			readWord.setLowByte(lowByte);
			readWord.setHighByte(highByte);

			return readWord;
		}
	};

	template<
		template<class, class>
			class Action,
		class Operand1,
		class Operand2
	>
	struct Instruction
	{
		static inline void execute(Cpu* cpu)
		{
			Action<Operand1, Operand2>::execute(cpu);
		}
	};

	template<class Test, class A, class B>
	struct JumpImplementation
	{
		static inline void execute(Cpu* cpu)
		{
			word jumpAddress = A::read(cpu);
			if (Test::execute(cpu))
			{
				cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
			}
		}
	};

	struct AlwaysTrue
	{
		static inline bool execute(Cpu *cpu)
		{
			return true;
		}
	};

	template<int FlagBit, int ExpectedValue>
	struct FlagTest
	{
		static inline bool execute(Cpu *cpu)
		{
			byte temp = Flag<FlagBit>::read(cpu);
			return temp == ExpectedValue;
		}
	};

	template<class A, class B>
	struct JMP : public JumpImplementation<AlwaysTrue, A, B>
	{
	};

	template<class A, class B>
	struct JSR
	{
		static inline void execute(Cpu* cpu)
		{
			word jumpAddress = A::read(cpu);
			if (AlwaysTrue::execute(cpu))
			{
				cpu->push(cpu->_registers.ProgramCounter);

				cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
			}
		}
	};
	
	template<class A, class B>
	struct BCS : public JumpImplementation<FlagTest<Carry,1>, A, B>
	{
	};

	template<class A, class B>
	struct BCC : public JumpImplementation<FlagTest<Carry,0>, A, B>
	{
	};

	template<class A, class B>
	struct BEQ : public JumpImplementation<FlagTest<Zero,1>, A, B>
	{
	};

	template<class A, class B>
	struct BNE: public JumpImplementation<FlagTest<Zero,0>, A, B>
	{
	};

	template<class Source, class Destination>
	struct Load
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Source::read(cpu);

			Destination::write(cpu, value);

			cpu->_registers.ProcessorStatus.Zero = TestZero(value);
			cpu->_registers.ProcessorStatus.Negative = TestNegative(value);
		}
	};

	template<class Source, class Destination>
	struct Store : public Load<Source, Destination>
	{
	};

	template<class A, class B>
	struct SetFlag
	{
		static inline void execute(Cpu* cpu)
		{
			A::write(cpu, 1);
		}
	};

	template<class A, class B>
	struct ClearFlag
	{
		static inline void execute(Cpu* cpu)
		{
			A::write(cpu, 0);
		}
	};

	template<class A, class B>
	struct NOP
	{
		static inline void execute(Cpu* cpu)
		{
		}
	};

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

		_setupInstructions();
	}

	Cpu::~Cpu()
	{
	}

	void Cpu::executeOpcode()
	{
		sukiAssertWithMessage(_memory, "Please setup a memory for the CPU");

		byte opcode = _memory->read(_registers.ProgramCounter);

		if (_instructions[opcode])
		{
			_instructions[opcode](this);
		}

		_registers.ProgramCounter++;
	}

	void Cpu::push(byte value)
	{
		word stackAdress;
		stackAdress.setHighByte(0x1);
		stackAdress.setLowByte( _registers.StackPointer);

		_memory->write(stackAdress, value);

		_registers.StackPointer--;
	}

	void Cpu::push(word value)
	{
		push(value.lowByte());
		push(value.highByte());
	}

	void Cpu::_setupInstructions()
	{
		registerOpcode< 0x20, Instruction<JSR, NextWord, void> >();
		registerOpcode< 0x4C, Instruction<JMP, NextWord, void> >();

		registerOpcode< 0x90, Instruction<BCC, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xB0, Instruction<BCS, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xF0, Instruction<BEQ, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xD0, Instruction<BNE, RelativeAddress<NextByte>, void> >();

		registerOpcode< 0xA2, Instruction<Load, NextByte, Register<X>> >();
		registerOpcode< 0xA9, Instruction<Load, NextByte, Register<A>> >();

		registerOpcode< 0x86, Instruction<Store, Register<X>, ToAddress<NextByte>> >();

		registerOpcode< 0xEA, Instruction<NOP, void, void> >();

		registerOpcode< 0x38, Instruction<SetFlag, Flag<Carry>, void> >();
		registerOpcode< 0x18, Instruction<ClearFlag, Flag<Carry>, void> >();
	}
}
