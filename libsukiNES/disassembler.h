#pragma once

// STL incldues
#include <string>

namespace sukiNES
{
	class IMemory;

	namespace Disassembler
	{
		std::string disassemble(word programCounter, IMemory* memory);
	}
}
