#include "cpu.h"

// Local includes
#include "assert.h"
#include "memory.h"

namespace sukiNES
{
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

	inline int TestZero(byte value)
	{
		return value == 0;
	}

	inline int TestNegative(byte value)
	{
		return (value & SUKINES_BIT(Negative));
	}

	inline int TestOverflow(int value)
	{
		return (value & 0x40);
	}

	enum RegisterName
	{
		A,
		X,
		Y,
		ProcessorStatus,
		StackPointer
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

	template<>
	struct Register<StackPointer>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->_registers.StackPointer;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.StackPointer = value;
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
	struct ToAddressReadWrite
	{
		static word s_lastReadAddress;

		static inline byte read(Cpu* cpu)
		{
			s_lastReadAddress = Address::read(cpu);
			return cpu->_memory->read(s_lastReadAddress);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_memory->write(s_lastReadAddress, value);
		}
	};

	template<class Address>
	word ToAddressReadWrite<Address>::s_lastReadAddress = 0;

	template<class Address>
	struct RelativeAddress
	{
		static inline word read(Cpu* cpu)
		{
			offset relativeByte = static_cast<offset>(Address::read(cpu));

			return static_cast<word>(cpu->_registers.ProgramCounter + relativeByte + 1);
		}
	};

	template<class Address>
	struct IndirectXAddress
	{
		static inline byte read(Cpu* cpu)
		{
			byte zeroPageIndex = static_cast<offset>(Address::read(cpu));
			zeroPageIndex += Register<X>::read(cpu);

			byte lowByte = cpu->_memory->read(word(zeroPageIndex));
			byte highByte = cpu->_memory->read(static_cast<byte>(zeroPageIndex + 1));

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			return cpu->_memory->read(resultAddress);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			byte zeroPageIndex = static_cast<offset>(Address::read(cpu));
			zeroPageIndex += Register<X>::read(cpu);

			byte lowByte = cpu->_memory->read(word(zeroPageIndex));
			byte highByte = cpu->_memory->read(static_cast<byte>(zeroPageIndex + 1));

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			cpu->_memory->write(resultAddress, value);
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
	struct RTI
	{
		static inline void execute(Cpu* cpu)
		{
			byte processorStatus = cpu->popByte();
			word pc = cpu->popWord();

			Register<ProcessorStatus>::write(cpu, processorStatus);
			Flag<Unused>::write(cpu, 1);

			cpu->setProgramCounter(pc -1);
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
	struct Compare
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

	template<class A, class B>
	struct Add
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);
			byte carry = Flag<Carry>::read(cpu);

			int temp = a + b + carry;
			byte result = static_cast<byte>(temp);

			Flag<Carry>::write(cpu, (temp > 0xFF));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, !((a ^ b) & 0x80) && ((a ^ temp) & 0x80));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct Substract
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte b = B::read(cpu);
			byte carry = Flag<Carry>::read(cpu) ? 0 : 1;

			int temp = (int)a - (int)b - (int)carry;
			byte result = static_cast<byte>(temp);

			Flag<Carry>::write(cpu, (temp >= 0 && temp < 0x100));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, ((a ^ b) & 0x80) && ((a ^ temp) & 0x80));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct Increment
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			a++;

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			A::write(cpu, a);
		}
	};

	template<class A, class B>
	struct Decrement
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			a--;

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			A::write(cpu, a);
		}
	};

	template<class A, class B>
	struct Transfer
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			B::write(cpu, a);
		}
	};

	template<>
	struct Transfer<Register<X>, Register<StackPointer>>
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Register<X>::read(cpu);
			Register<StackPointer>::write(cpu, a);
		}
	};

	template<class A, class B>
	struct LSR
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(0);

			a = a >> 1;

			Flag<Negative>::write(cpu, 0);
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			A::write(cpu, a);
		}
	};

	template<class A, class B>
	struct ASL
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = A::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(7);

			a = a << 1;

			Flag<Negative>::write(cpu, TestNegative(a));
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			A::write(cpu, a);
		}
	};

	template<class A, class B>
	struct ROL
	{
		static inline void execute(Cpu* cpu)
		{
			u16 temp = static_cast<u16>(A::read(cpu));
			temp <<= 1;
			if(Flag<Carry>::read(cpu))
			{
				temp |= 0x1;
			}
			Flag<Carry>::write(cpu, temp > 0xFF);

			temp &= 0xFF;

			byte result = static_cast<byte>(temp);

			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Zero>::write(cpu, TestZero(result));

			A::write(cpu, result);
		}
	};

	template<class A, class B>
	struct ROR
	{
		static inline void execute(Cpu* cpu)
		{
			u16 temp = static_cast<u16>(A::read(cpu));
			if(Flag<Carry>::read(cpu))
			{
				temp |= 0x100;
			}
			Flag<Carry>::write(cpu, temp & SUKINES_BIT(0));

			temp >>= 1;

			byte result = static_cast<byte>(temp);

			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Zero>::write(cpu, TestZero(result));

			A::write(cpu, result);
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
		else
		{
			sukiAssertWithMessage(false, "Opcode not implemented yet !");
		}

		_registers.ProgramCounter++;
	}

	void Cpu::push(byte value)
	{
		word stackAdress;
		stackAdress.setHighByte(0x1);
		stackAdress.setLowByte(_registers.StackPointer);

		_memory->write(stackAdress, value);

		_registers.StackPointer--;
	}

	void Cpu::push(word value)
	{
		push(value.highByte());
		push(value.lowByte());
	}

	byte Cpu::popByte()
	{
		_registers.StackPointer++;

		word stackAddress;
		stackAddress.setHighByte(0x1);
		stackAddress.setLowByte(_registers.StackPointer);

		return _memory->read(stackAddress);
	}

	word Cpu::popWord()
	{
		word poppedValue;

		poppedValue.setLowByte( popByte() );
		poppedValue.setHighByte( popByte() );

		return poppedValue;
	}

	void Cpu::_setupInstructions()
	{
		registerOpcode< 0x20, Instruction<JSR, NextWord, void> >();
		registerOpcode< 0x40, Instruction<RTI, void, void> >();
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

		registerOpcode< 0xA0, Instruction<Load, NextByte, Register<Y>> >();
		registerOpcode< 0xA1, Instruction<Load, IndirectXAddress<NextByte>, Register<A>> >();
		registerOpcode< 0xA2, Instruction<Load, NextByte, Register<X>> >();
		registerOpcode< 0xA4, Instruction<Load, ToAddress<NextByte>, Register<Y>> >();
		registerOpcode< 0xA5, Instruction<Load, ToAddress<NextByte>, Register<A>> >();
		registerOpcode< 0xA6, Instruction<Load, ToAddress<NextByte>, Register<X>> >();
		registerOpcode< 0xA9, Instruction<Load, NextByte, Register<A>> >();
		registerOpcode< 0xAC, Instruction<Load, ToAddress<NextWord>, Register<Y>> >();
		registerOpcode< 0xAD, Instruction<Load, ToAddress<NextWord>, Register<A>> >();
		registerOpcode< 0xAE, Instruction<Load, ToAddress<NextWord>, Register<X>> >();

		registerOpcode< 0x81, Instruction<Store, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x84, Instruction<Store, Register<Y>, ToAddress<NextByte>> >();
		registerOpcode< 0x85, Instruction<Store, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x86, Instruction<Store, Register<X>, ToAddress<NextByte>> >();
		registerOpcode< 0x8C, Instruction<Store, Register<Y>, ToAddress<NextWord>> >();
		registerOpcode< 0x8D, Instruction<Store, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x8E, Instruction<Store, Register<X>, ToAddress<NextWord>> >();

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

		registerOpcode< 0x01, Instruction<OR, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x05, Instruction<OR, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x09, Instruction<OR, Register<A>, NextByte> >();
		registerOpcode< 0x21, Instruction<AND, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x25, Instruction<AND, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x29, Instruction<AND, Register<A>, NextByte> >();
		registerOpcode< 0x41, Instruction<EOR, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x45, Instruction<EOR, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x49, Instruction<EOR, Register<A>, NextByte> >();

		registerOpcode< 0xC0, Instruction<Compare, Register<Y>, NextByte> >();
		registerOpcode< 0xC1, Instruction<Compare, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0xC4, Instruction<Compare, Register<Y>, ToAddress<NextByte>> >();
		registerOpcode< 0xC5, Instruction<Compare, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0xC9, Instruction<Compare, Register<A>, NextByte> >();
		registerOpcode< 0xE0, Instruction<Compare, Register<X>, NextByte> >();
		registerOpcode< 0xE4, Instruction<Compare, Register<X>,ToAddress<NextByte>> >();

		registerOpcode< 0x61, Instruction<Add, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x65, Instruction<Add, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x69, Instruction<Add, Register<A>, NextByte> >();

		registerOpcode< 0xE1, Instruction<Substract, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0xE5, Instruction<Substract, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0xE9, Instruction<Substract, Register<A>, NextByte> >();

		registerOpcode< 0xE6, Instruction<Increment, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0xC8, Instruction<Increment, Register<Y>, void> >();
		registerOpcode< 0xE8, Instruction<Increment, Register<X>, void> >();

		registerOpcode< 0x88, Instruction<Decrement, Register<Y>, void> >();
		registerOpcode< 0xC6, Instruction<Decrement, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0xCA, Instruction<Decrement, Register<X>, void> >();

		registerOpcode< 0x8A, Instruction<Transfer, Register<X>, Register<A>> >();
		registerOpcode< 0x98, Instruction<Transfer, Register<Y>, Register<A>> >();
		registerOpcode< 0x9A, Instruction<Transfer, Register<X>, Register<StackPointer>> >();
		registerOpcode< 0xA8, Instruction<Transfer, Register<A>, Register<Y>> >();
		registerOpcode< 0xAA, Instruction<Transfer, Register<A>, Register<X>> >();
		registerOpcode< 0xBA, Instruction<Transfer, Register<StackPointer>, Register<X>> >();

		registerOpcode< 0x46, Instruction<LSR, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0x4A, Instruction<LSR, Register<A>, void> >();

		registerOpcode< 0x06, Instruction<ASL, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0x0A, Instruction<ASL, Register<A>, void> >();

		registerOpcode< 0x26, Instruction<ROL, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0x2A, Instruction<ROL, Register<A>, void> >();

		registerOpcode< 0x66, Instruction<ROR, ToAddressReadWrite<NextByte>, void> >();
		registerOpcode< 0x6A, Instruction<ROR, Register<A>, void> >();
	}
}
