#pragma once

namespace sukiNES
{
	class GamePak;
	class Mapper;
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
		Mapper* createMapper(uint32 mapperNumber) const;

	private:
		GamePak* _gamePak;
		PPU* _ppu;
	};
}
