#include "inesreader.h"

// STL includes
#include <cstdio>
#include <cstring>

// Local includes
#include "assert.h"
#include "gamepak.h"
#include "mapper.h"
#include "ppu.h"

// Mappers
#include "unrom_mapper.h"

static const char iNESHeader[] = "NES\x1a";

namespace sukiNES
{
	class CFileWrapper
	{
	public:
		CFileWrapper()
		: m_file(nullptr)
		{
		
		}
	
		~CFileWrapper()
		{
			if(m_file)
			{
				fclose(m_file);
			}
		}
	
		bool open(const char* fileName, const char *openMode)
		{
			m_file = fopen(fileName, openMode);
		
			return m_file != NULL;
		}
	
		size_t read(void* ptr, size_t size, size_t count)
		{
			return fread(ptr,size,count, m_file);
		}
	
		int getc()
		{
			return fgetc(m_file);
		}
	
		int seek(long int offset, int origin)
		{
			return fseek(m_file, offset, origin);
		}
	
	private:
		FILE *m_file;
	};

	iNESReader::iNESReader()
	: _gamePak(nullptr)
	, _ppu(nullptr)
	{
	}

	iNESReader::~iNESReader()
	{
	}

	bool iNESReader::read(const char* filename)
	{
		sukiAssertWithMessage(_gamePak, "Please setup the game pak for iNESReader");

		CFileWrapper file;
		if (!file.open(filename, "rb"))
		{
			return false;
		}

		// Read magic number
		byte headerBuffer[4];
		file.read(headerBuffer, sizeof(byte), 4);

		if (memcmp(headerBuffer, iNESHeader, 4) != 0)
		{
			return false;
		}

		// Read header information
		byte romPageCount = static_cast<byte>(file.getc());
		byte chrPageCount = static_cast<byte>(file.getc());
		byte controlByte1 = static_cast<byte>(file.getc());
		byte controlByte2 = static_cast<byte>(file.getc());
		byte wramPageCount = static_cast<byte>(file.getc());

		for(int i=0; i<7; ++i)
		{
			file.getc();
		}

		uint32 mapperNumber = (controlByte2 & 0xF0) | ((controlByte1 & 0xF0) >> 4);

		_gamePak->setMapperNumber(mapperNumber);
		_gamePak->setMapper(createMapper(mapperNumber));

		byte mirroring;

		if (controlByte1 & SUKINES_BIT(3))
		{
			mirroring = static_cast<byte>(PPU::NameTableMirroring::FourScreen);
		}
		else
		{
			mirroring = static_cast<byte>( (controlByte1 & SUKINES_BIT(0)) ?  PPU::NameTableMirroring::Vertical : PPU::NameTableMirroring::Horizontal );
		}

		_gamePak->setMirroring(mirroring);
		_gamePak->setHasSaveRam( (controlByte1 & SUKINES_BIT(1)) ? true : false );

		// Read ROM pages
		uint32 romDataSize = romPageCount * RomBankSize;
		DynamicArray<byte> romData(romDataSize);

		file.read(romData.get(), sizeof(byte), romDataSize);

		_gamePak->setRomData(std::move(romData));

		// Read CHR pages
		if (chrPageCount > 0)
		{
			uint32 chrDataSize = chrPageCount * ChrBankSize;
			DynamicArray<byte> chrData(chrDataSize);

			file.read(chrData.get(), sizeof(byte), chrDataSize);

			_gamePak->setChrData(std::move(chrData));
		}

		if (_ppu)
		{
			_ppu->setNametableMirroring(static_cast<PPU::NameTableMirroring>(mirroring));
		}

		return true;
	}
	
	Mapper* iNESReader::createMapper(uint32 mapperNumber) const
	{
		Mapper* mapper = nullptr;

		switch(mapperNumber)
		{
		case 2:
			mapper = new UnromMapper(_gamePak);
			break;
		default:
			break;
		}

		return mapper;
	}
}
