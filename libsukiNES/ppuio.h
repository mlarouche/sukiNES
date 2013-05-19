#pragma once

namespace sukiNES
{
	class PPUIO
	{
	public:
		virtual ~PPUIO() {}

		virtual void putPixel(sint32 x, sint32 y, byte paletteIndex) = 0;
		virtual void onVBlank() = 0;
	};
}