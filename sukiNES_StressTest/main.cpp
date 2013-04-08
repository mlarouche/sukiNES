// STL includes
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

// sukiNES includes
#include <cpu.h>
#include <gamepak.h>
#include <mainmemory.h>
#include <inesreader.h>

#ifdef SUKINES_PLATFORM_WINDOWS
#include <windows.h>
#endif

static const char* LogFilename = "nestest.log";
static const char* RomFilename = "nestest.nes";

#ifdef SUKINES_PLATFORM_WINDOWS
void showpause()
{
	system("pause");
}
#endif

#ifdef SUKINES_PLATFORM_WINDOWS
#define assertIsEqual(actual, expected, message) \
	do { \
		if ( actual != expected ) { \
			if (IsDebuggerPresent()) ForceBreakpoint(); \
			_generateFailureMessage(message, actual, expected); \
			return false; \
		} \
	} while(0)
#else
#define assertIsEqual(actual, expected, message) \
	do { \
		if ( actual != expected ) { \
			_generateFailureMessage(message, actual, expected); \
			return false; \
		}
	} while(0)
#endif

class NesStressTest
{
public:
	NesStressTest()
	: _failureMessage(nullptr)
	, _currentLine(1)
	{
		// Setup MainMemory
		_memory.setGamepakMemory(&_gamePak);

		// Setup memory in CPU
		_cpu.setMainMemory(&_memory);

		// Set initial state of the CPU
		_cpu.setProgramCounter(0xC000);
		_cpu.disableInterrupt();
		_cpu.push(0x00);
		_cpu.push(0x00);
	}

	~NesStressTest()
	{
		if (_failureMessage)
		{
			delete[] _failureMessage;
		}
	}

	bool run()
	{
		// Read ROM
		sukiNES::iNESReader nesReader;
		nesReader.setGamePak(&_gamePak);

		if (!nesReader.read(RomFilename))
		{
			_generateFailureMessage("Cannot open NES file %s", RomFilename);
			return false;
		}

		assertIsEqual(_gamePak.mapper(), 0, "Mapper not equal");
		assertIsEqual(_gamePak.romPageCount(), 1, "ROM page count not equal");
		assertIsEqual(_gamePak.chrPageCount(), 1, "CHR page count not equal");

		// Read log file
		std::ifstream logFile(LogFilename);
		if(!logFile)
		{
			_generateFailureMessage("Cannot open log file %s", LogFilename);
			return false;
		}

		while(!logFile.eof())
		{
			std::getline(logFile, _lineBuffer);
			decodeLine();

			// Check all CPU registers
			auto currentRegisters = _cpu.getRegisters();

			assertIsEqual(currentRegisters.ProgramCounter, _expectedRegisters.ProgramCounter, "Program counter not equal");
			assertIsEqual(currentRegisters.A, _expectedRegisters.A, "A not equal");
			assertIsEqual(currentRegisters.X, _expectedRegisters.X, "X not equal");
			assertIsEqual(currentRegisters.Y, _expectedRegisters.Y, "Y not equal");
			assertIsEqual(currentRegisters.ProcessorStatus.raw, _expectedRegisters.ProcessorStatus.raw, "Processor Status not equal");
			assertIsEqual(currentRegisters.StackPointer, _expectedRegisters.StackPointer, "Stack pointer not equal");

			_cpu.executeOpcode();

			_currentLine++;
		}

		return true;
	}

	void printFailure()
	{
		if(_failureMessage)
		{
			fprintf(stderr, "FAILURE: %s\n", _failureMessage);
			if (!_lineBuffer.empty())
			{
				fprintf(stderr, "At line %d: %s\n", _currentLine, _lineBuffer.c_str());
			}
		}
		else
		{
			fprintf(stderr, "FAILURE !\n");
			if (!_lineBuffer.empty())
			{
				fprintf(stderr, "At line %d: %s\n", _currentLine, _lineBuffer.c_str());
			}
		}
	}

private:
	void decodeLine()
	{
		enum
		{
			Index_PC = 0,
			Index_A = 50,
			Index_X = 55,
			Index_Y = 60,
			Index_P = 65,
			Index_SP = 71,
			Index_Cycle = 78,
			Index_Scanline = 85
		};

		// Decode program counter
		std::string tempSubstring = _lineBuffer.substr(Index_PC, 4);
		_expectedRegisters.ProgramCounter = word(stringToNumber(tempSubstring));

		// Decode A
		tempSubstring = _lineBuffer.substr(Index_A, 2);
		_expectedRegisters.A = static_cast<byte>(stringToNumber(tempSubstring));

		// Decode X
		tempSubstring = _lineBuffer.substr(Index_X, 2);
		_expectedRegisters.X = static_cast<byte>(stringToNumber(tempSubstring));
		
		// Decode Y
		tempSubstring = _lineBuffer.substr(Index_Y, 2);
		_expectedRegisters.Y = static_cast<byte>(stringToNumber(tempSubstring));

		// Decode P(Processor Status)
		tempSubstring = _lineBuffer.substr(Index_P, 2);
		_expectedRegisters.ProcessorStatus.raw = static_cast<byte>(stringToNumber(tempSubstring));

		// Decode SP
		tempSubstring = _lineBuffer.substr(Index_SP, 2);
		_expectedRegisters.StackPointer = static_cast<byte>(stringToNumber(tempSubstring));
	}

	uint32 stringToNumber(const std::string &text)
	{
		static const int HexBase = 16;
		uint32 result = 0;

		for(int index=text.size()-1; index >= 0; --index)
		{
			int digit = 0;
			char charDigit = text[index];

			if(charDigit >= '0' && charDigit <= '9')
			{
				digit = static_cast<int>(charDigit - '0');
			}
			else if(charDigit >= 'A' && charDigit <= 'F')
			{
				digit = static_cast<int>((charDigit - 'A') + 10);
			}

			result += static_cast<int>(digit * std::pow(static_cast<double>(HexBase),text.size()-index-1)); 
		}

		return result;
	}

	template<typename T1>
	void _generateFailureMessage(const char* message, T1 filename)
	{
		_allocateFailureMessage();

		sprintf(_failureMessage, message, filename);
	}

	template<typename T1, typename T2>
	void _generateFailureMessage(const char* message, T1 actual, T2 expected)
	{
		_allocateFailureMessage();

		sprintf(_failureMessage, "%s: actual=%#X, expected=%#X", message, actual, expected);
	}

	void _allocateFailureMessage()
	{
		if(_failureMessage)
		{
			delete[] _failureMessage;
			_failureMessage = nullptr;
		}

		_failureMessage = new char[SUKINES_KB(2)];
	}

private:
	sukiNES::Cpu _cpu;
	sukiNES::MainMemory _memory;
	sukiNES::GamePak _gamePak;

	char* _failureMessage;
	std::string _lineBuffer;
	uint32 _currentLine;
	sukiNES::CpuRegisters _expectedRegisters;
};

int main(int argc, char** argv)
{
#ifdef SUKINES_PLATFORM_WINDOWS
	atexit(showpause);
#endif

	NesStressTest stressTest;

	if (stressTest.run())
	{
		fprintf(stderr, "OK !");
		return 0;
	}
	else
	{
		stressTest.printFailure();
		return 1;
	}
}
