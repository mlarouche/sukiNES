#pragma once

namespace sukiNES
{
	class IMemory
	{
	public:
		virtual ~IMemory() {}

		virtual byte read(word address) = 0;
		virtual void write(word address, byte value) = 0;
	};
}