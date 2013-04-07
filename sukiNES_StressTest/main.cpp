// STL includes
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

// sukiNES includes
#include <cpu.h>

#ifdef SUKINES_PLATFORM_WINDOWS
#include <windows.h>
#endif

static const char* LogFilename = "nestest.log";
static const char* RomFilename = "nestest.nes";

void showpause()
{
	system("pause");
}

#ifdef SUKINES_PLATFORM_WINDOWS
#define generateFailureMessage(message, actual, expected) \
	if (IsDebuggerPresent()) __debugbreak(); \
	_generateFailureMessage(message, actual, expected)
#else
#define generateFailureMessage(message, actual, expected) _generateFailureMessage(message, actual, expected)
#endif

class NesStressTest
{
public:
	NesStressTest()
	: _failureMessage(nullptr)
	, _currentLine(1)
	{
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
		std::ifstream logFile(LogFilename);
		if(!logFile)
		{
			std::cerr << "Cannot open log file " << LogFilename << std::endl;
			return false;
		}

		while(!logFile.eof())
		{
			std::getline(logFile, _lineBuffer);
			decodeLine();

			// Check program counter before executing the opcode
			auto currentRegisters = cpu.getRegisters();
			if(currentRegisters.ProgramCounter != _expectedRegisters.ProgramCounter)
			{
				generateFailureMessage("Program counter not equal: actual=%#X, expected=%#X", currentRegisters.ProgramCounter, _expectedRegisters.ProgramCounter);
				return false;
			}

			cpu.executeOpcode();

			// Check all the other CPU registers
			currentRegisters = cpu.getRegisters();

			if(currentRegisters.A != _expectedRegisters.A)
			{
				generateFailureMessage("A not equal: actual=%#X, expected=%#X", currentRegisters.A, _expectedRegisters.A);
				return false;
			}

			if(currentRegisters.X != _expectedRegisters.X)
			{
				generateFailureMessage("X not equal: actual=%#X, expected=%#X", currentRegisters.X, _expectedRegisters.X);
				return false;
			}

			if(currentRegisters.Y != _expectedRegisters.Y)
			{
				generateFailureMessage("Y not equal: actual=%#X, expected=%#X", currentRegisters.Y, _expectedRegisters.Y);
				return false;
			}

			if(currentRegisters.ProcessorStatus.raw != _expectedRegisters.ProcessorStatus.raw)
			{
				generateFailureMessage("Processor Status not equal: actual=%#X, expected=%#X", currentRegisters.ProcessorStatus.raw, _expectedRegisters.ProcessorStatus.raw);
				return false;
			}

			if(currentRegisters.StackPointer != _expectedRegisters.StackPointer)
			{
				generateFailureMessage("Stack Pointer not equal: actual=%#X, expected=%#X", currentRegisters.StackPointer, _expectedRegisters.StackPointer);
				return false;
			}

			_currentLine++;
		}

		return true;
	}

	void printFailure()
	{
		if(_failureMessage)
		{
			fprintf(stderr, "FAILURE: %s\nAt line %d: %s\n", _failureMessage, _currentLine, _lineBuffer.c_str());
		}
		else
		{
			fprintf(stderr, "FAILURE !\nAt line %d: %s\n", _currentLine, _lineBuffer.c_str());
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

	template<typename T1, typename T2>
	void _generateFailureMessage(const char* message, T1 actual, T2 expected)
	{
		if(_failureMessage)
		{
			delete[] _failureMessage;
			_failureMessage = nullptr;
		}

		_failureMessage = new char[1024];
		sprintf(_failureMessage, message, actual, expected);
	}

private:
	sukiNES::Cpu cpu;

	char* _failureMessage;
	std::string _lineBuffer;
	uint32 _currentLine;
	sukiNES::CpuRegisters _expectedRegisters;
};

int main(int argc, char** argv)
{
	atexit(showpause);

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
