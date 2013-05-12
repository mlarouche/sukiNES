#pragma once

namespace sukiNES
{
	class GamePak;
	class PPU;

	class iNESReader
	{
	public:
		iNESReader();
		~iNESReader();

		bool read(const char* filename);

		void setGamePak(GamePak* gamepak)
		{
			_gamePak = gamepak;
		}

		void setPpu(PPU* ppu)
		{
			_ppu = ppu;
		}

	private:
		GamePak* _gamePak;
		PPU* _ppu;
	};
}
