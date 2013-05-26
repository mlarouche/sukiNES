#pragma once

namespace sukiNES
{
	class InputIO
	{
	public:
		virtual ~InputIO() { }

		virtual byte inputStatus(byte controller) const = 0;
	};
}