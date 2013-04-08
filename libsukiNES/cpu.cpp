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
		return (value & SUKINES_BIT(7));
	}

	inline int TestOverflow(int value)
	{
		return (value & 0x40);
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
			case 0:
				cpu->_registers.ProcessorStatus.raw &= ~SUKINES_BIT(FlagBit);
				break;
			default:
				cpu->_registers.ProcessorStatus.raw |= SUKINES_BIT(FlagBit);
				break;
			}
		}
	};

	enum RegisterName
	{
		A,
		X,
		Y,
		ProcessorStatus
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

	template<>
	struct Register<ProcessorStatus>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_registers.ProcessorStatus.raw;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.ProcessorStatus.raw = value;
		}
	};

	template<class Address>
	struct ToAddress
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_memory->read(Address::read(cpu));
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
			cpu->push(cpu->_registers.ProgramCounter);

			cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
		}
	};

	template<class A, class B>
	struct RTS
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->setProgramCounter( cpu->popWord() );
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

	template<class A, class B>
	struct BVS: public JumpImplementation<FlagTest<Overflow,1>, A, B>
	{
	};

	template<class A, class B>
	struct BVC: public JumpImplementation<FlagTest<Overflow,0>, A, B>
	{
	};

	template<class A, class B>
	struct BPL: public JumpImplementation<FlagTest<Negative,0>, A, B>
	{
	};

	template<class A, class B>
	struct BMI: public JumpImplementation<FlagTest<Negative,1>, A, B>
	{
	};

	template<class Source, class Destination>
	struct Load
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Source::read(cpu);

			Destination::write(cpu, value);

			Flag<Zero>::write(cpu, TestZero(value));
			Flag<Negative>::write(cpu, TestNegative(value));
		}
	};

	template<class Source, class Destination>
	struct Store
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Source::read(cpu);

			Destination::write(cpu, value);
		}
	};

	template<class Source, class B>
	struct Push
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->push(Source::read(cpu));
		}
	};

	template<>
	struct Push<Register<ProcessorStatus>, void>
	{
		static inline void execute(Cpu* cpu)
		{
			byte readValue = Register<ProcessorStatus>::read(cpu);
			readValue |= SUKINES_BIT(Break);

			cpu->push(readValue);
		}
	};

	template<class Destination, class B>
	struct Pop
	{
		static inline void execute(Cpu* cpu)
		{
			byte poppedValue = cpu->popByte();

			Destination::write(cpu, poppedValue);

			Flag<Zero>::write(cpu, TestZero(poppedValue));
			Flag<Negative>::write(cpu, TestNegative(poppedValue));
		}
	};

	template<>
	struct Pop<Register<ProcessorStatus>, void>
	{
		static inline void execute(Cpu* cpu)
		{
			byte poppedValue = cpu->popByte();

			Register<ProcessorStatus>::write(cpu, poppedValue);

			Flag<Unused>::write(cpu, 1);
			Flag<Break>::write(cpu, 0);
		}
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

	template<class MemorySource, class B>
	struct BIT
	{
		static inline void execute(Cpu* cpu)
		{
			byte readValue = MemorySource::read(cpu);
			int test = Register<A>::read(cpu) & readValue;

			Flag<Zero>::write(cpu, TestZero(test));
			Flag<Negative>::write(cpu, TestNegative(readValue));
			Flag<Overflow>::write(cpu, TestOverflow(readValue));
		}
	};

	template<class A, class B>
	struct AND
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);

			byte result = a & b;

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct OR
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);

			byte result = a | b;

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct EOR
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);

			byte result = a ^ b;

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct CMP
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);

			int result = static_cast<int>(a) - static_cast<int>(b);

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Carry>::write(cpu, (result >= 0) ? 1 : 0);
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

	byte Cpu::popByte()
	{
		_registers.StackPointer++;

		word stackAddress;
		stackAddress.setHighByte(0x1);
		stackAddress.setLowByte( _registers.StackPointer);

		return _memory->read(stackAddress);
	}

	word Cpu::popWord()
	{
		word poppedValue;

		poppedValue.setHighByte( popByte() );
		poppedValue.setLowByte( popByte() );

		return poppedValue;
	}

	void Cpu::_setupInstructions()
	{
		registerOpcode< 0x20, Instruction<JSR, NextWord, void> >();
		registerOpcode< 0x60, Instruction<RTS, void, void> >();

		registerOpcode< 0x4C, Instruction<JMP, NextWord, void> >();

		registerOpcode< 0x10, Instruction<BPL, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0x30, Instruction<BMI, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0x50, Instruction<BVC, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0x70, Instruction<BVS, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0x90, Instruction<BCC, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xB0, Instruction<BCS, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xD0, Instruction<BNE, RelativeAddress<NextByte>, void> >();
		registerOpcode< 0xF0, Instruction<BEQ, RelativeAddress<NextByte>, void> >();

		registerOpcode< 0xA2, Instruction<Load, NextByte, Register<X>> >();
		registerOpcode< 0xA9, Instruction<Load, NextByte, Register<A>> >();

		registerOpcode< 0x85, Instruction<Store, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x86, Instruction<Store, Register<X>, ToAddress<NextByte>> >();

		registerOpcode< 0xEA, Instruction<NOP, void, void> >();

		registerOpcode< 0x38, Instruction<SetFlag, Flag<Carry>, void> >();
		registerOpcode< 0x78, Instruction<SetFlag, Flag<InterruptDisabled>, void> >();
		registerOpcode< 0xF8, Instruction<SetFlag, Flag<Decimal>, void> >();

		registerOpcode< 0x18, Instruction<ClearFlag, Flag<Carry>, void> >();
		registerOpcode< 0xB8, Instruction<ClearFlag, Flag<Overflow>, void> >();
		registerOpcode< 0xD8, Instruction<ClearFlag, Flag<Decimal>, void> >();

		registerOpcode< 0x24, Instruction<BIT, ToAddress<NextByte>, void> >();

		registerOpcode< 0x08, Instruction<Push, Register<ProcessorStatus>, void> >();
		registerOpcode< 0x48, Instruction<Push, Register<A>, void> >();

		registerOpcode< 0x28, Instruction<Pop, Register<ProcessorStatus>, void> >();
		registerOpcode< 0x68, Instruction<Pop, Register<A>, void> >();

		registerOpcode< 0x09, Instruction<OR, Register<A>, NextByte> >();
		registerOpcode< 0x29, Instruction<AND, Register<A>, NextByte> >();
		registerOpcode< 0x49, Instruction<EOR, Register<A>, NextByte> >();

		registerOpcode< 0xC9, Instruction<CMP, Register<A>, NextByte> >();
	}
}
