#pragma once

// STL includes
#include <vector>

// Local includes
#include "memory.h"

namespace sukiNES
{
	static const uint32 RomBankSize = SUKINES_KB(16);
	static const uint32 ChrBankSize = SUKINES_KB(8);

	enum class MirroringType
	{
		Horizontal,
		Vertical,
		FourScren
	};

	class GamePak : public IMemory
	{
	public:
		GamePak();
		~GamePak();

		virtual byte read(word address);
		virtual void write(word address, byte value);

		void setRomData(DynamicArray<byte>&& romData);

		void setChrData(DynamicArray<byte>&& chrData)
		{
			_chrData = std::forward<DynamicArray<byte>>(chrData);
			_chrBank = _chrData.get();
		}

		bool hasSaveRam() const
		{
			return _hasSaveRam;
		}

		void setHasSaveRam(bool value)
		{
			_hasSaveRam = value;
		}

		MirroringType mirroring() const
		{
			return _mirroring;
		}

		void setMirroring(MirroringType value)
		{
			_mirroring = value;
		}

		uint32 romPageCount() const
		{
			return _romData.size() / RomBankSize;
		}

		uint32 chrPageCount() const
		{
			return _chrData.size() / ChrBankSize;
		}

		uint32 mapper() const
		{
			return _mapper;
		}

		void setMapper(uint32 mapper)
		{
			_mapper = mapper;
		}

	private:
		DynamicArray<byte> _romData;
		DynamicArray<byte> _chrData;

		byte* _romBank[2];
		byte* _chrBank;

		MirroringType _mirroring;
		bool _hasSaveRam;
		uint32 _mapper;
	};
}