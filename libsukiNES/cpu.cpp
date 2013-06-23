#include "cpu.h"

// STL includes
#include <algorithm>

// Local includes
#include "assert.h"
#include "inputio.h"
#include "memory.h"
#include "ppu.h"

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
			return (cpu->_registers.ProcessorStatus.raw & SUKINES_BIT(FlagBit)) >> FlagBit;
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->_registers.ProcessorStatus.raw = (cpu->_registers.ProcessorStatus.raw & ~SUKINES_BIT(FlagBit)) |
				((!!value) << FlagBit);
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
		return (value & SUKINES_BIT(Overflow));
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

	enum class AddressBehavior
	{
		AlwaysRead,
		KeepAddress
	};

	template<class AddressSource, AddressBehavior Behavior>
	struct AddressBehaviorImplementation
	{
		static word readAddress(Cpu* cpu)
		{
			return AddressSource::read(cpu);
		}

		static word writeAddress(Cpu* cpu)
		{
			return AddressSource::read(cpu);
		}
	};

	template<class AddressSource>
	struct AddressBehaviorImplementation<AddressSource, AddressBehavior::KeepAddress>
	{
		static word s_lastReadAddress;

		static word readAddress(Cpu* cpu)
		{
			s_lastReadAddress = AddressSource::read(cpu);
			return s_lastReadAddress;
		}

		static word writeAddress(Cpu* cpu)
		{
			return s_lastReadAddress;
		}
	};

	template<class AddressSource>
	word AddressBehaviorImplementation<AddressSource, AddressBehavior::KeepAddress>::s_lastReadAddress = 0;

	template<class AddressSource, AddressBehavior Behavior = AddressBehavior::AlwaysRead>
	struct ToAddress : public AddressBehaviorImplementation<AddressSource, Behavior>
	{
		static inline byte read(Cpu* cpu)
		{
			return cpu->readMemory(readAddress(cpu));
		}

		static inline void write(Cpu* cpu, byte value)
		{
			cpu->writeMemory(writeAddress(cpu), value);
		}
	};

	template<class AddressSource, class Register, AddressBehavior Behavior>
	struct ToAddressPlusRegister : public AddressBehaviorImplementation<AddressSource, Behavior>
	{
		static bool HasCrossedPageBoundary;

		static inline byte read(Cpu* cpu)
		{
			word address = readAddress(cpu);
			byte registerValue = Register::read(cpu);

			HasCrossedPageBoundary = (((address & 0xFF) + registerValue) >= 0x100);

			address += registerValue;

			return cpu->readMemory(address);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			word address = writeAddress(cpu);
			byte registerValue = Register::read(cpu);

			HasCrossedPageBoundary = (((address & 0xFF) + registerValue) >= 0x100);

			address += registerValue;
			cpu->writeMemory(address, value);
		}
	};

	template<class AddressSource, class Register, AddressBehavior Behavior>
	bool ToAddressPlusRegister<AddressSource,Register,Behavior>::HasCrossedPageBoundary = false;
	
	template<class AddressSource, AddressBehavior Behavior = AddressBehavior::AlwaysRead>
	struct ToAddressPlusX : public ToAddressPlusRegister<AddressSource, Register<X>, Behavior>
	{
	};

	template<class AddressSource, AddressBehavior Behavior = AddressBehavior::AlwaysRead>
	struct ToAddressPlusY : public ToAddressPlusRegister<AddressSource, Register<Y>, Behavior>
	{
	};

	template<class AddressSource>
	struct RelativeAddress
	{
		static bool HasCrossedPageBoundary;

		static inline word read(Cpu* cpu)
		{
			offset relativeByte = static_cast<offset>(AddressSource::read(cpu));

			HasCrossedPageBoundary = (((cpu->programCounter()+2) & 0xFF) + relativeByte) > 0x100;

			return static_cast<word>(cpu->programCounter() + relativeByte + 1);
		}
	};

	template<class AddressSource>
	bool RelativeAddress<AddressSource>::HasCrossedPageBoundary = false;

	template<class AddressSource>
	struct IndirectAbsoluteAddress
	{
		static inline word read(Cpu* cpu)
		{
			word absoluteAddress = AddressSource::read(cpu);

			byte lowByte = cpu->readMemory(absoluteAddress);

			absoluteAddress.setLowByte(static_cast<byte>(absoluteAddress.lowByte() + 1));

			byte highByte = cpu->readMemory(absoluteAddress);

			word readWord;
			readWord.setLowByte(lowByte);
			readWord.setHighByte(highByte);

			return readWord;
		}
	};

	template<class AddressSource, AddressBehavior Behavior = AddressBehavior::AlwaysRead>
	struct IndirectXAddress : public AddressBehaviorImplementation<AddressSource, Behavior>
	{
		static inline byte read(Cpu* cpu)
		{
			byte zeroPageIndex = readAddress(cpu);
			zeroPageIndex += Register<X>::read(cpu);

			if (Behavior == AddressBehavior::AlwaysRead)
			{
				cpu->tick();
			}

			byte lowByte = cpu->readMemory(word(zeroPageIndex));
			byte highByte = cpu->readMemory(static_cast<byte>(zeroPageIndex + 1));

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			return cpu->readMemory(resultAddress);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			byte zeroPageIndex = writeAddress(cpu);
			zeroPageIndex += Register<X>::read(cpu);

			if (Behavior == AddressBehavior::AlwaysRead)
			{
				cpu->tick();
			}

			byte lowByte = cpu->readMemory(word(zeroPageIndex));
			byte highByte = cpu->readMemory(static_cast<byte>(zeroPageIndex + 1));

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			cpu->writeMemory(resultAddress, value);
		}
	};

	template<class AddressSource, AddressBehavior Behavior = AddressBehavior::AlwaysRead>
	struct IndirectPlusYAddress : public AddressBehaviorImplementation<AddressSource, Behavior>
	{
		static bool HasCrossedPageBoundary;

		static inline byte read(Cpu* cpu)
		{
			byte zeroPageIndex = readAddress(cpu);
			byte registerY = Register<Y>::read(cpu);

			byte lowByte = cpu->readMemory(word(zeroPageIndex));
			byte highByte = cpu->readMemory(static_cast<byte>(zeroPageIndex + 1));

			HasCrossedPageBoundary = ((lowByte + registerY) >= 0x100);

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			resultAddress += registerY;

			return cpu->readMemory(resultAddress);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			byte zeroPageIndex = writeAddress(cpu);
			byte registerY = Register<Y>::read(cpu);

			HasCrossedPageBoundary = ((static_cast<int>(zeroPageIndex) + registerY + 1) >= 0x100);

			byte lowByte = cpu->readMemory(zeroPageIndex);
			byte highByte = cpu->readMemory(static_cast<byte>(zeroPageIndex + 1));

			HasCrossedPageBoundary = ((lowByte + registerY) >= 0x100);

			word resultAddress;
			resultAddress.setLowByte(lowByte);
			resultAddress.setHighByte(highByte);

			resultAddress += registerY;

			cpu->writeMemory(resultAddress, value);
		}
	};

	template<class AddressSource, AddressBehavior Behavior>
	bool IndirectPlusYAddress<AddressSource, Behavior>::HasCrossedPageBoundary = false;

	struct NextByte
	{
		static inline byte read(Cpu* cpu)
		{
			cpu->_registers.ProgramCounter++;
			return cpu->readMemory(cpu->_registers.ProgramCounter);
		}
	};

	struct NextWord
	{
		static inline word read(Cpu* cpu)
		{
			byte lowByte = cpu->readMemory(++cpu->_registers.ProgramCounter);
			byte highByte = cpu->readMemory(++cpu->_registers.ProgramCounter);

			word readWord;
			readWord.setLowByte(lowByte);
			readWord.setHighByte(highByte);

			return readWord;
		}
	};

	template<class Register, AddressBehavior Behavior>
	struct ToAddressPlusRegister<NextByte, Register, Behavior> : public AddressBehaviorImplementation<NextByte, Behavior>
	{
		static inline byte read(Cpu* cpu)
		{
			word address = readAddress(cpu);
			address = static_cast<byte>(address + Register::read(cpu));
			return cpu->readMemory(address);
		}

		static inline void write(Cpu* cpu, byte value)
		{
			word address = writeAddress(cpu);
			address = static_cast<byte>(address + Register::read(cpu));
			cpu->writeMemory(address, value);
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

	template<class Test, class Addressing, class B>
	struct BranchImplementation
	{
		static inline void execute(Cpu* cpu)
		{
			word jumpAddress = Addressing::read(cpu);
			if (Test::execute(cpu))
			{
				cpu->tick();

				if (Addressing::HasCrossedPageBoundary)
				{
					cpu->tick();
				}

				cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
			}
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

	template<class Addressing, class B>
	struct JMP
	{
		static inline void execute(Cpu* cpu)
		{
			word jumpAddress = Addressing::read(cpu);
			cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
		}
	};

	template<class Addressing, class B>
	struct JSR
	{
		static inline void execute(Cpu* cpu)
		{
			word jumpAddress = Addressing::read(cpu);
			cpu->tick();
			cpu->push(cpu->programCounter());

			cpu->setProgramCounter(static_cast<uint32>(jumpAddress) - 1);
		}
	};

	template<class A, class B>
	struct RTS
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
			cpu->setProgramCounter( cpu->popWord() );
			cpu->tick();
		}
	};

	template<class A, class B>
	struct RTI
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
			byte processorStatus = cpu->popByte();
			word pc = cpu->popWord();

			Register<ProcessorStatus>::write(cpu, processorStatus);
			Flag<Unused>::write(cpu, 1);

			cpu->setProgramCounter(pc -1);
			cpu->_insideIrq = false;
		}
	};
	
	template<class Addressing, class B>
	struct BCS : public BranchImplementation<FlagTest<Carry,1>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BCC : public BranchImplementation<FlagTest<Carry,0>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BEQ : public BranchImplementation<FlagTest<Zero,1>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BNE: public BranchImplementation<FlagTest<Zero,0>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BVS: public BranchImplementation<FlagTest<Overflow,1>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BVC: public BranchImplementation<FlagTest<Overflow,0>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BPL: public BranchImplementation<FlagTest<Negative,0>, Addressing, B>
	{
	};

	template<class Addressing, class B>
	struct BMI: public BranchImplementation<FlagTest<Negative,1>, Addressing, B>
	{
	};

	template<class Addressing>
	struct LoadCycleTick
	{
		static inline void tick(Cpu* cpu)
		{
		}
	};

	template<>
	struct LoadCycleTick<ToAddressPlusX<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct LoadCycleTick<ToAddressPlusX<NextWord>>
	{
		static inline void tick(Cpu* cpu)
		{
			if (ToAddressPlusX<NextWord>::HasCrossedPageBoundary)
			{
				cpu->tick();
			}
		}
	};

	template<>
	struct LoadCycleTick<ToAddressPlusY<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct LoadCycleTick<ToAddressPlusY<NextWord>>
	{
		static inline void tick(Cpu* cpu)
		{
			if (ToAddressPlusY<NextWord>::HasCrossedPageBoundary)
			{
				cpu->tick();
			}
		}
	};

	template<>
	struct LoadCycleTick<IndirectPlusYAddress<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			if (IndirectPlusYAddress<NextByte>::HasCrossedPageBoundary)
			{
				cpu->tick();
			}
		}
	};

	template<class Source, class Destination>
	struct Load
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Source::read(cpu);

			Destination::write(cpu, value);

			LoadCycleTick<Source>::tick(cpu);

			Flag<Zero>::write(cpu, TestZero(value));
			Flag<Negative>::write(cpu, TestNegative(value));
		}
	};

	template<class Addressing>
	struct StoreCycleTick
	{
		static inline void tick(Cpu* cpu)
		{
		}
	};

	template<>
	struct StoreCycleTick<IndirectPlusYAddress<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusX<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusX<NextWord>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusY<NextByte>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusY<NextWord>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusY<NextByte, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct StoreCycleTick<ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<class Source, class Destination>
	struct Store
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Source::read(cpu);

			StoreCycleTick<Destination>::tick(cpu);

			Destination::write(cpu, value);
		}
	};

	template<class Source, class B>
	struct Push
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
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

			cpu->tick();
			cpu->push(readValue);
		}
	};

	template<class Destination, class B>
	struct Pop
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
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
			cpu->tick();
			cpu->tick();
			byte poppedValue = cpu->popByte();

			Register<ProcessorStatus>::write(cpu, poppedValue);

			Flag<Unused>::write(cpu, 1);
			Flag<Break>::write(cpu, 0);
		}
	};

	template<class Flag, class B>
	struct SetFlag
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
			Flag::write(cpu, 1);
		}
	};

	template<class Flag, class B>
	struct ClearFlag
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
			Flag::write(cpu, 0);
		}
	};

	template<class A, class B>
	struct NOP
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<class Addressing>
	struct NOP<Addressing, void>
	{
		static inline void execute(Cpu* cpu)
		{
			Addressing::read(cpu);
			cpu->tick();
		}
	};

	template<>
	struct NOP<void, void>
	{
		static inline void execute(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct NOP<ToAddressPlusX<NextWord>, void>
	{
		static inline void execute(Cpu* cpu)
		{
			ToAddressPlusX<NextWord>::read(cpu);
			if (ToAddressPlusX<NextWord>::HasCrossedPageBoundary)
			{
				cpu->tick();
			}
		}
	};

	template<class Addressing, class B>
	struct NOPImmediate
	{
		static inline void execute(Cpu* cpu)
		{
			Addressing::read(cpu);
		}
	};

	template<class Addressing, class B>
	struct BIT
	{
		static inline void execute(Cpu* cpu)
		{
			byte readValue = Addressing::read(cpu);
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

			LoadCycleTick<B>::tick(cpu);

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

			LoadCycleTick<B>::tick(cpu);

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

			LoadCycleTick<B>::tick(cpu);

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

			LoadCycleTick<B>::tick(cpu);

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

			LoadCycleTick<B>::tick(cpu);

			Flag<Carry>::write(cpu, (temp > 0xFF));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, !((a ^ b) & SUKINES_BIT(Negative)) && ((a ^ temp) & SUKINES_BIT(Negative)));

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

			LoadCycleTick<B>::tick(cpu);

			Flag<Carry>::write(cpu, (temp >= 0 && temp < 0x100));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, ((a ^ b) & SUKINES_BIT(Negative)) && ((a ^ temp) & SUKINES_BIT(Negative)));

			A::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct Increment
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			cpu->tick();
			a++;

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, a);
		}
	};

	template<class Addressing, class B>
	struct Decrement
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			cpu->tick();
			a--;

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, a);
		}
	};

	template<class Source, class Destination>
	struct Transfer
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Source::read(cpu);
			cpu->tick();

			Flag<Zero>::write(cpu, TestZero(a));
			Flag<Negative>::write(cpu, TestNegative(a));

			Destination::write(cpu, a);
		}
	};

	template<>
	struct Transfer<Register<X>, Register<StackPointer>>
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Register<X>::read(cpu);
			cpu->tick();
			Register<StackPointer>::write(cpu, a);
		}
	};

	template<class Addressing, class B>
	struct LSR
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(0);

			a = a >> 1;

			cpu->tick();

			Flag<Negative>::write(cpu, 0);
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, a);
		}
	};

	template<class Addressing, class B>
	struct ASL
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(7);

			a = a << 1;

			cpu->tick();

			Flag<Negative>::write(cpu, TestNegative(a));
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, a);
		}
	};

	template<class Addressing, class B>
	struct ROL
	{
		static inline void execute(Cpu* cpu)
		{
			uint16 temp = static_cast<uint16>(Addressing::read(cpu));
			temp <<= 1;
			if(Flag<Carry>::read(cpu))
			{
				temp |= 0x1;
			}
			Flag<Carry>::write(cpu, temp > 0xFF);

			temp &= 0xFF;

			byte result = static_cast<byte>(temp);

			cpu->tick();

			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Zero>::write(cpu, TestZero(result));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct ROR
	{
		static inline void execute(Cpu* cpu)
		{
			uint16 temp = static_cast<uint16>(Addressing::read(cpu));
			if(Flag<Carry>::read(cpu))
			{
				temp |= 0x100;
			}
			Flag<Carry>::write(cpu, temp & SUKINES_BIT(0));

			temp >>= 1;

			byte result = static_cast<byte>(temp);

			cpu->tick();

			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Zero>::write(cpu, TestZero(result));

			StoreCycleTick<Addressing>::tick(cpu);
			Addressing::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct LAX
	{
		static inline void execute(Cpu* cpu)
		{
			byte value = Addressing::read(cpu);

			Register<A>::write(cpu, value);
			Register<X>::write(cpu, value);

			LoadCycleTick<Addressing>::tick(cpu);

			Flag<Negative>::write(cpu, TestNegative(value));
			Flag<Zero>::write(cpu, TestZero(value));
		}
	};

	template<class Addressing, class B>
	struct AAX
	{
		static inline void execute(Cpu* cpu)
		{
			byte result = Register<X>::read(cpu) & Register<A>::read(cpu);

			Addressing::write(cpu, result);

			StoreCycleTick<Addressing>::tick(cpu);
		}
	};

	template<class Addressing>
	struct IllegalOpcodeTick
	{
		static inline void tick(Cpu* cpu)
		{
		}
	};

	template<>
	struct IllegalOpcodeTick<ToAddress<NextByte, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct IllegalOpcodeTick<ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
		}
	};

	template<>
	struct IllegalOpcodeTick<ToAddress<NextWord, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
		}
	};

	template<>
	struct IllegalOpcodeTick<ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
		}
	};

	template<>
	struct IllegalOpcodeTick<ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>>
	{
		static inline void tick(Cpu* cpu)
		{
			cpu->tick();
			cpu->tick();
		}
	};

	template<class Addressing, class B>
	struct DCP
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			a--;

			int result = static_cast<int>(Register<A>::read(cpu)) - static_cast<int>(a);

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Carry>::write(cpu, (result >= 0) ? 1 : 0);

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Addressing::write(cpu, a);
		}
	};

	template<class Addressing, class B>
	struct ISC
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			a++;
			Addressing::write(cpu, a);

			byte b = Register<A>::read(cpu);
			byte carry = Flag<Carry>::read(cpu) ? 0 : 1;

			int temp = (int)b - (int)a - (int)carry;
			byte result = static_cast<byte>(temp);

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Flag<Carry>::write(cpu, (temp >= 0 && temp < 0x100));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, ((b ^ a) & SUKINES_BIT(Negative)) && ((b ^ temp) & SUKINES_BIT(Negative)));

			Register<A>::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct SLO
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(7);

			a = a << 1;

			Flag<Negative>::write(cpu, TestNegative(a));
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			Addressing::write(cpu, a);

			byte b = Register<A>::read(cpu);

			byte result = a | b;

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			Register<A>::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct RLA
	{
		static inline void execute(Cpu* cpu)
		{
			uint16 temp = static_cast<uint16>(Addressing::read(cpu));
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

			Addressing::write(cpu, result);

			byte a = Register<A>::read(cpu);
			byte b = result;

			result = a & b;

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			Register<A>::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct RRA
	{
		static inline void execute(Cpu* cpu)
		{
			uint16 temp = static_cast<uint16>(Addressing::read(cpu));
			if(Flag<Carry>::read(cpu))
			{
				temp |= 0x100;
			}
			Flag<Carry>::write(cpu, temp & SUKINES_BIT(0));

			temp >>= 1;

			byte result = static_cast<byte>(temp);

			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Zero>::write(cpu, TestZero(result));

			Addressing::write(cpu, result);

			byte a = Register<A>::read(cpu);
			byte b = result;
			byte carry = Flag<Carry>::read(cpu);

			int temp2 = a + b + carry;
			result = static_cast<byte>(temp2);

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Flag<Carry>::write(cpu, (temp2 > 0xFF));
			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));
			Flag<Overflow>::write(cpu, !((a ^ b) & SUKINES_BIT(Negative)) && ((a ^ temp2) & SUKINES_BIT(Negative)));

			Register<A>::write(cpu, result);
		}
	};

	template<class Addressing, class B>
	struct SRE
	{
		static inline void execute(Cpu* cpu)
		{
			byte a = Addressing::read(cpu);
			byte shiffedBit = a & SUKINES_BIT(0);

			a = a >> 1;

			Flag<Negative>::write(cpu, 0);
			Flag<Carry>::write(cpu, shiffedBit);
			Flag<Zero>::write(cpu, TestZero(a));

			Addressing::write(cpu, a);

			byte b = Register<A>::read(cpu);

			byte result = b ^ a;

			IllegalOpcodeTick<Addressing>::tick(cpu);

			Flag<Zero>::write(cpu, TestZero(result));
			Flag<Negative>::write(cpu, TestNegative(result));

			Register<A>::write(cpu, result);
		}
	};

	Cpu::Cpu()
	: _memory(nullptr)
	, _ppu(nullptr)
	, _inputStrobe(0)
	, _inputIO(nullptr)
	, _nmiOccured(false)
	, _insideIrq(false)
	{
		std::fill(std::begin(_buttonStatus), std::end(_buttonStatus), 0);
		std::fill(std::begin(_inputReadCounter), std::end(_inputReadCounter), 0);

		_registers.StackPointer = 0xFF;
		_registers.A = 0;
		_registers.X = 0;
		_registers.Y = 0;
		_registers.ProgramCounter = 0;
		_registers.ProcessorStatus.raw = 0;
		_registers.ProcessorStatus.Unused = true;

		_setupInstructions();

#ifdef SUKINES_DEBUG
		_totalTick = 0;
#endif
	}

	Cpu::~Cpu()
	{
	}

	void Cpu::powerOn()
	{
		_registers.StackPointer = 0xFF;
		_registers.A = 0;
		_registers.X = 0;
		_registers.Y = 0;
		_registers.ProgramCounter = 0;
		_registers.ProcessorStatus.raw = 0;
		_registers.ProcessorStatus.Unused = true;

		reset();
	}

	void Cpu::reset()
	{
		_nmiOccured = false;
		_insideIrq = false;

		word resetVector;
		resetVector.setLowByte( _memory->read(0xFFFC) );
		resetVector.setHighByte( _memory->read(0xFFFD) );

		_registers.ProgramCounter = resetVector;
	}

	void Cpu::executeOpcode()
	{
		#ifdef SUKINES_DEBUG
			_totalTick = 0;
		#endif

		sukiAssertWithMessage(_memory, "Please setup a memory for the CPU");

		byte opcode = readMemory(_registers.ProgramCounter);

		if (_instructions[opcode])
		{
			_instructions[opcode](this);
		}
		else
		{
			sukiAssertWithMessage(false, "Opcode not implemented yet !");
		}

		_registers.ProgramCounter++;

		if (!_insideIrq)
		{
			if (_nmiOccured)
			{
				word nmiVectorAddress;
				nmiVectorAddress.setHighByte(0xFF);
				nmiVectorAddress.setLowByte(0xFA);
				doIrq(nmiVectorAddress);

				_nmiOccured = false;
			}
		}
	}

	void Cpu::push(byte value)
	{
		word stackAdress;
		stackAdress.setHighByte(0x1);
		stackAdress.setLowByte(_registers.StackPointer);

		writeMemory(stackAdress, value);

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

		return readMemory(stackAddress);
	}

	word Cpu::popWord()
	{
		word poppedValue;

		poppedValue.setLowByte( popByte() );
		poppedValue.setHighByte( popByte() );

		return poppedValue;
	}

	byte Cpu::readMemory(word address)
	{
		tick();

		byte readValue = 0;

		if (address >= 0x4016 && address <= 0x4017)
		{
			switch(address.lowByte())
			{
				case 0x16:
				case 0x17:
				{
					byte whichController = address.lowByte() & 0x1;
					if (_inputStrobe == 0)
					{
						if (_inputReadCounter[whichController] < 8)
						{
							readValue = ((_buttonStatus[whichController] >> _inputReadCounter[whichController]) & 0x1) | 0x40;
							++_inputReadCounter[whichController];
						}
						else
						{
							readValue = 0x41;
						}
					}
					else
					{
						readValue = 0x40;
						if (_inputIO)
						{
							readValue |= _inputIO->inputStatus(whichController) & 0x1;
						}
					}
					break;
				}
			}
		}
		else
		{
			readValue = _memory->read(address);
		}

		return readValue;
	}

	void Cpu::writeMemory(word address, byte value)
	{
		tick();

		if (address == 0x4014)
		{
			dmaCopy(value);
		}
		else if (address >= 0x4016 && address <= 0x4017)
		{
			switch(address.lowByte())
			{
			case 0x16:
				if (_inputStrobe == 1 && value == 0)
				{
					for (uint32 whichController = 0; whichController<2; ++whichController)
					{
						_buttonStatus[whichController] = 0;
						_inputReadCounter[whichController] = 0;

						if (_inputIO)
						{
							_buttonStatus[whichController] = _inputIO->inputStatus(whichController);
						}
					}
				}
				_inputStrobe = value;
				break;
			case 0x17:
				break;
			}
		}
		else
		{
			_memory->write(address, value);
		}
	}

	void Cpu::tick()
	{
#ifdef SUKINES_DEBUG
		_totalTick++;
#endif
		sukiAssertWithMessage(_ppu, "Please set the PPU in the CPU");

		// PPU is running 3 times faster than the CPU
		for(int i=0; i<3; i++)
		{
			_ppu->tick();
			if ( _ppu->hasVBlankOccured())
			{
				_nmiOccured = true;
			}
		}

		// TODO: Run APU tick
	}

	void Cpu::dmaCopy(byte memoryPage)
	{
		word ramAddress;
		ramAddress.setHighByte(memoryPage);
		ramAddress.setLowByte(0);

		word oamDataAddress;
		oamDataAddress.setHighByte(0x20);
		oamDataAddress.setLowByte(0x04);

		for(uint32 i=0; i<256; ++i)
		{
			byte readValue = readMemory(ramAddress);
			writeMemory(oamDataAddress, readValue);
			ramAddress++;
		}
	}

	void Cpu::doIrq(word vectorAddress)
	{
#ifdef SUKINES_DEBUG
		_totalTick = 0;
#endif

		tick();
		tick();

		push(_registers.ProgramCounter);
		push(_registers.ProcessorStatus.raw);

		word irqAddress;
		irqAddress.setLowByte( readMemory(vectorAddress) );

		_registers.ProcessorStatus.Break = false;
		_registers.ProcessorStatus.Unused = true;

		++vectorAddress;
		irqAddress.setHighByte( readMemory(vectorAddress) );

		setProgramCounter(irqAddress);

		_insideIrq = true;
	}

	void Cpu::_setupInstructions()
	{
		registerOpcode< 0x20, Instruction<JSR, NextWord, void> >();
		registerOpcode< 0x40, Instruction<RTI, void, void> >();
		registerOpcode< 0x60, Instruction<RTS, void, void> >();

		registerOpcode< 0x4C, Instruction<JMP, NextWord, void> >();
		registerOpcode< 0x6C, Instruction<JMP, IndirectAbsoluteAddress<NextWord>, void> >();

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
		registerOpcode< 0xB1, Instruction<Load, IndirectPlusYAddress<NextByte>, Register<A>> >();
		registerOpcode< 0xB4, Instruction<Load, ToAddressPlusX<NextByte>, Register<Y>> >();
		registerOpcode< 0xB5, Instruction<Load, ToAddressPlusX<NextByte>, Register<A>> >();
		registerOpcode< 0xB6, Instruction<Load, ToAddressPlusY<NextByte>, Register<X>> >();
		registerOpcode< 0xB9, Instruction<Load, ToAddressPlusY<NextWord>, Register<A>> >();
		registerOpcode< 0xBC, Instruction<Load, ToAddressPlusX<NextWord>, Register<Y>> >();
		registerOpcode< 0xBD, Instruction<Load, ToAddressPlusX<NextWord>, Register<A>> >();
		registerOpcode< 0xBE, Instruction<Load, ToAddressPlusY<NextWord>, Register<X>> >();

		registerOpcode< 0x81, Instruction<Store, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x84, Instruction<Store, Register<Y>, ToAddress<NextByte>> >();
		registerOpcode< 0x85, Instruction<Store, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x86, Instruction<Store, Register<X>, ToAddress<NextByte>> >();
		registerOpcode< 0x8C, Instruction<Store, Register<Y>, ToAddress<NextWord>> >();
		registerOpcode< 0x8D, Instruction<Store, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x8E, Instruction<Store, Register<X>, ToAddress<NextWord>> >();
		registerOpcode< 0x91, Instruction<Store, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0x94, Instruction<Store, Register<Y>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x95, Instruction<Store, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x96, Instruction<Store, Register<X>, ToAddressPlusY<NextByte>> >();
		registerOpcode< 0x99, Instruction<Store, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0x9D, Instruction<Store, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0xEA, Instruction<NOP, void, void> >();

		registerOpcode< 0x38, Instruction<SetFlag, Flag<Carry>, void> >();
		registerOpcode< 0x78, Instruction<SetFlag, Flag<InterruptDisabled>, void> >();
		registerOpcode< 0xF8, Instruction<SetFlag, Flag<Decimal>, void> >();

		registerOpcode< 0x18, Instruction<ClearFlag, Flag<Carry>, void> >();
		registerOpcode< 0xB8, Instruction<ClearFlag, Flag<Overflow>, void> >();
		registerOpcode< 0x58, Instruction<ClearFlag, Flag<InterruptDisabled>, void> >();
		registerOpcode< 0xD8, Instruction<ClearFlag, Flag<Decimal>, void> >();

		registerOpcode< 0x24, Instruction<BIT, ToAddress<NextByte>, void> >();
		registerOpcode< 0x2C, Instruction<BIT, ToAddress<NextWord>, void> >();

		registerOpcode< 0x08, Instruction<Push, Register<ProcessorStatus>, void> >();
		registerOpcode< 0x48, Instruction<Push, Register<A>, void> >();

		registerOpcode< 0x28, Instruction<Pop, Register<ProcessorStatus>, void> >();
		registerOpcode< 0x68, Instruction<Pop, Register<A>, void> >();

		registerOpcode< 0x01, Instruction<OR, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x05, Instruction<OR, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x09, Instruction<OR, Register<A>, NextByte> >();
		registerOpcode< 0x0D, Instruction<OR, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x11, Instruction<OR, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0x15, Instruction<OR, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x19, Instruction<OR, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0x1D, Instruction<OR, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0x21, Instruction<AND, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x25, Instruction<AND, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x29, Instruction<AND, Register<A>, NextByte> >();
		registerOpcode< 0x2D, Instruction<AND, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x31, Instruction<AND, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0x35, Instruction<AND, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x39, Instruction<AND, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0x3D, Instruction<AND, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0x41, Instruction<EOR, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x45, Instruction<EOR, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x49, Instruction<EOR, Register<A>, NextByte> >();
		registerOpcode< 0x4D, Instruction<EOR, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x51, Instruction<EOR, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0x55, Instruction<EOR, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x59, Instruction<EOR, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0x5D, Instruction<EOR, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0xC0, Instruction<Compare, Register<Y>, NextByte> >();
		registerOpcode< 0xC1, Instruction<Compare, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0xC4, Instruction<Compare, Register<Y>, ToAddress<NextByte>> >();
		registerOpcode< 0xC5, Instruction<Compare, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0xC9, Instruction<Compare, Register<A>, NextByte> >();
		registerOpcode< 0xCC, Instruction<Compare, Register<Y>, ToAddress<NextWord>> >();
		registerOpcode< 0xCD, Instruction<Compare, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0xD1, Instruction<Compare, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0xD5, Instruction<Compare, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0xD9, Instruction<Compare, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0xDD, Instruction<Compare, Register<A>, ToAddressPlusX<NextWord>> >();
		registerOpcode< 0xE0, Instruction<Compare, Register<X>, NextByte> >();
		registerOpcode< 0xE4, Instruction<Compare, Register<X>, ToAddress<NextByte>> >();
		registerOpcode< 0xEC, Instruction<Compare, Register<X>, ToAddress<NextWord>> >();

		registerOpcode< 0x61, Instruction<Add, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0x65, Instruction<Add, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0x69, Instruction<Add, Register<A>, NextByte> >();
		registerOpcode< 0x6D, Instruction<Add, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0x71, Instruction<Add, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0x75, Instruction<Add, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0x79, Instruction<Add, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0x7D, Instruction<Add, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0xE1, Instruction<Substract, Register<A>, IndirectXAddress<NextByte>> >();
		registerOpcode< 0xE5, Instruction<Substract, Register<A>, ToAddress<NextByte>> >();
		registerOpcode< 0xE9, Instruction<Substract, Register<A>, NextByte> >();
		registerOpcode< 0xED, Instruction<Substract, Register<A>, ToAddress<NextWord>> >();
		registerOpcode< 0xF1, Instruction<Substract, Register<A>, IndirectPlusYAddress<NextByte>> >();
		registerOpcode< 0xF5, Instruction<Substract, Register<A>, ToAddressPlusX<NextByte>> >();
		registerOpcode< 0xF9, Instruction<Substract, Register<A>, ToAddressPlusY<NextWord>> >();
		registerOpcode< 0xFD, Instruction<Substract, Register<A>, ToAddressPlusX<NextWord>> >();

		registerOpcode< 0xE6, Instruction<Increment, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xC8, Instruction<Increment, Register<Y>, void> >();
		registerOpcode< 0xE8, Instruction<Increment, Register<X>, void> >();
		registerOpcode< 0xEE, Instruction<Increment, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xF6, Instruction<Increment, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xFE, Instruction<Increment, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x88, Instruction<Decrement, Register<Y>, void> >();
		registerOpcode< 0xC6, Instruction<Decrement, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xCA, Instruction<Decrement, Register<X>, void> >();
		registerOpcode< 0xCE, Instruction<Decrement, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xD6, Instruction<Decrement, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xDE, Instruction<Decrement, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x8A, Instruction<Transfer, Register<X>, Register<A>> >();
		registerOpcode< 0x98, Instruction<Transfer, Register<Y>, Register<A>> >();
		registerOpcode< 0x9A, Instruction<Transfer, Register<X>, Register<StackPointer>> >();
		registerOpcode< 0xA8, Instruction<Transfer, Register<A>, Register<Y>> >();
		registerOpcode< 0xAA, Instruction<Transfer, Register<A>, Register<X>> >();
		registerOpcode< 0xBA, Instruction<Transfer, Register<StackPointer>, Register<X>> >();

		registerOpcode< 0x46, Instruction<LSR, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x4A, Instruction<LSR, Register<A>, void> >();
		registerOpcode< 0x4E, Instruction<LSR, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x56, Instruction<LSR, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x5E, Instruction<LSR, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x06, Instruction<ASL, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x0A, Instruction<ASL, Register<A>, void> >();
		registerOpcode< 0x0E, Instruction<ASL, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x16, Instruction<ASL, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x1E, Instruction<ASL, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x26, Instruction<ROL, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x2A, Instruction<ROL, Register<A>, void> >();
		registerOpcode< 0x2E, Instruction<ROL, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x36, Instruction<ROL, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x3E, Instruction<ROL, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x66, Instruction<ROR, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x6A, Instruction<ROR, Register<A>, void> >();
		registerOpcode< 0x6E, Instruction<ROR, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x76, Instruction<ROR, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x7E, Instruction<ROR, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		// Illegal opcodes
		registerOpcode< 0x04, Instruction<NOP, NextByte, void> >();
		registerOpcode< 0x0C, Instruction<NOP, NextWord, void> >();
		registerOpcode< 0x14, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0x1A, Instruction<NOP, void, void> >();
		registerOpcode< 0x1C, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();
		registerOpcode< 0x34, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0x3A, Instruction<NOP, void, void> >();
		registerOpcode< 0x3C, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();
		registerOpcode< 0x44, Instruction<NOP, NextByte, void> >();
		registerOpcode< 0x54, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0x5A, Instruction<NOP, void, void> >();
		registerOpcode< 0x5C, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();
		registerOpcode< 0x64, Instruction<NOP, NextByte, void> >();
		registerOpcode< 0x74, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0x7A, Instruction<NOP, void, void> >();
		registerOpcode< 0x7C, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();
		registerOpcode< 0x80, Instruction<NOPImmediate, NextByte, void> >();
		registerOpcode< 0x82, Instruction<NOPImmediate, NextByte, void> >();
		registerOpcode< 0x89, Instruction<NOPImmediate, NextByte, void> >();
		registerOpcode< 0xC2, Instruction<NOPImmediate, NextByte, void> >();
		registerOpcode< 0xD4, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0xDA, Instruction<NOP, void, void> >();
		registerOpcode< 0xDC, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();
		registerOpcode< 0xE2, Instruction<NOPImmediate, NextByte, void> >();
		registerOpcode< 0xF4, Instruction<NOP, ToAddressPlusX<NextByte>, void> >();
		registerOpcode< 0xFA, Instruction<NOP, void, void> >();
		registerOpcode< 0xFC, Instruction<NOP, ToAddressPlusX<NextWord>, void> >();

		registerOpcode< 0xA3, Instruction<LAX, IndirectXAddress<NextByte>, void> >();
		registerOpcode< 0xA7, Instruction<LAX, ToAddress<NextByte>, void> >();
		registerOpcode< 0xAF, Instruction<LAX, ToAddress<NextWord>, void> >();
		registerOpcode< 0xB3, Instruction<LAX, IndirectPlusYAddress<NextByte>, void> >();
		registerOpcode< 0xB7, Instruction<LAX, ToAddressPlusY<NextByte>, void> >();
		registerOpcode< 0xBF, Instruction<LAX, ToAddressPlusY<NextWord>, void> >();

		registerOpcode< 0x83, Instruction<AAX, IndirectXAddress<NextByte>, void> >();
		registerOpcode< 0x87, Instruction<AAX, ToAddress<NextByte>, void> >();
		registerOpcode< 0x8F, Instruction<AAX, ToAddress<NextWord>, void> >();
		registerOpcode< 0x97, Instruction<AAX, ToAddressPlusY<NextByte>, void> >();

		registerOpcode< 0xEB, Instruction<Substract, Register<A>, NextByte> >();

		registerOpcode< 0xC3, Instruction<DCP, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xC7, Instruction<DCP, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xCF, Instruction<DCP, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xD3, Instruction<DCP, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xD7, Instruction<DCP, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xDB, Instruction<DCP, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xDF, Instruction<DCP, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0xE3, Instruction<ISC, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xE7, Instruction<ISC, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xEF, Instruction<ISC, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xF3, Instruction<ISC, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xF7, Instruction<ISC, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xFB, Instruction<ISC, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0xFF, Instruction<ISC, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x03, Instruction<SLO, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x07, Instruction<SLO, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x0F, Instruction<SLO, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x13, Instruction<SLO, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x17, Instruction<SLO, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x1B, Instruction<SLO, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x1F, Instruction<SLO, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x23, Instruction<RLA, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x27, Instruction<RLA, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x2F, Instruction<RLA, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x33, Instruction<RLA, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x37, Instruction<RLA, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x3B, Instruction<RLA, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x3F, Instruction<RLA, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x43, Instruction<SRE, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x47, Instruction<SRE, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x4F, Instruction<SRE, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x53, Instruction<SRE, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x57, Instruction<SRE, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x5B, Instruction<SRE, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x5F, Instruction<SRE, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();

		registerOpcode< 0x63, Instruction<RRA, IndirectXAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x67, Instruction<RRA, ToAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x6F, Instruction<RRA, ToAddress<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x73, Instruction<RRA, IndirectPlusYAddress<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x77, Instruction<RRA, ToAddressPlusX<NextByte, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x7B, Instruction<RRA, ToAddressPlusY<NextWord, AddressBehavior::KeepAddress>, void> >();
		registerOpcode< 0x7F, Instruction<RRA, ToAddressPlusX<NextWord, AddressBehavior::KeepAddress>, void> >();
	}
}
