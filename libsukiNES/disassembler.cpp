#include "disassembler.h"

// STL includes
#include <iomanip>
#include <sstream>
#include <vector>

// Local includes
#include "memory.h"

namespace sukiNES
{
	namespace Disassembler
	{
		struct DisassemblerEntry
		{
			DisassemblerEntry()
			 : opcode(0)
			 , numBytes(1)
			 , prettyOpcodeIndex(0)
			{
			}

			DisassemblerEntry(byte t_opcode, int t_prettyOpcodeIndex, uint32 t_numBytes, int mode)
			 : opcode(t_opcode)
			 , prettyOpcodeIndex(t_prettyOpcodeIndex)
			 , numBytes(t_numBytes)
			 , addressingMode(mode)
			{
			}

			byte opcode;
			int prettyOpcodeIndex;
			uint32 numBytes;
			int addressingMode;
		};

		//BEGIN Disassembler static data
		// Generated by Ruby script generate_6502_opcode_list.rb
		const char prettyOpcodeName[][4] = {
"AAC","AAX","ADC","AND","ARR","ASL","ASR","ATX","AXA","AXS","BCC","BCS","BEQ","BIT","BMI","BNE","BPL","BRK","BVC","BVS","CLC","CLD","CLI","CLV","CMP","CPX","CPY","DCP","DEC","DEX","DEY","DOP","EOR","INC","INX","INY","ISC","JMP","JSR","KIL","LAR","LAX","LDA","LDX","LDY","LSR","NOP","ORA","PHA","PHP","PLA","PLP","RLA","ROL","ROR","RRA","RTI","RTS","SBC","SEC","SED","SEI","SLO","SRE","STA","STX","STY","SXA","SYA","TAX","TAY","TOP","TSX","TXA","TXS","TYA","UNK","XAA","XAS"
};

enum PrettyOpcodeIndex {
AAC_Pretty = 0,
AAX_Pretty = 1,
ADC_Pretty = 2,
AND_Pretty = 3,
ARR_Pretty = 4,
ASL_Pretty = 5,
ASR_Pretty = 6,
ATX_Pretty = 7,
AXA_Pretty = 8,
AXS_Pretty = 9,
BCC_Pretty = 10,
BCS_Pretty = 11,
BEQ_Pretty = 12,
BIT_Pretty = 13,
BMI_Pretty = 14,
BNE_Pretty = 15,
BPL_Pretty = 16,
BRK_Pretty = 17,
BVC_Pretty = 18,
BVS_Pretty = 19,
CLC_Pretty = 20,
CLD_Pretty = 21,
CLI_Pretty = 22,
CLV_Pretty = 23,
CMP_Pretty = 24,
CPX_Pretty = 25,
CPY_Pretty = 26,
DCP_Pretty = 27,
DEC_Pretty = 28,
DEX_Pretty = 29,
DEY_Pretty = 30,
DOP_Pretty = 31,
EOR_Pretty = 32,
INC_Pretty = 33,
INX_Pretty = 34,
INY_Pretty = 35,
ISC_Pretty = 36,
JMP_Pretty = 37,
JSR_Pretty = 38,
KIL_Pretty = 39,
LAR_Pretty = 40,
LAX_Pretty = 41,
LDA_Pretty = 42,
LDX_Pretty = 43,
LDY_Pretty = 44,
LSR_Pretty = 45,
NOP_Pretty = 46,
ORA_Pretty = 47,
PHA_Pretty = 48,
PHP_Pretty = 49,
PLA_Pretty = 50,
PLP_Pretty = 51,
RLA_Pretty = 52,
ROL_Pretty = 53,
ROR_Pretty = 54,
RRA_Pretty = 55,
RTI_Pretty = 56,
RTS_Pretty = 57,
SBC_Pretty = 58,
SEC_Pretty = 59,
SED_Pretty = 60,
SEI_Pretty = 61,
SLO_Pretty = 62,
SRE_Pretty = 63,
STA_Pretty = 64,
STX_Pretty = 65,
STY_Pretty = 66,
SXA_Pretty = 67,
SYA_Pretty = 68,
TAX_Pretty = 69,
TAY_Pretty = 70,
TOP_Pretty = 71,
TSX_Pretty = 72,
TXA_Pretty = 73,
TXS_Pretty = 74,
TYA_Pretty = 75,
UNK_Pretty = 76,
XAA_Pretty = 77,
XAS_Pretty = 78
};

enum AddressingMode {
AddressingMode_Absolute,
AddressingMode_AbsoluteX,
AddressingMode_AbsoluteY,
AddressingMode_Accumulator,
AddressingMode_Immediate,
AddressingMode_Implied,
AddressingMode_Indirect,
AddressingMode_Relative,
AddressingMode_ZeroPage,
AddressingMode_ZeroPageX,
AddressingMode_ZeroPageY,
AddressingMode_IndirectPlusY,
AddressingMode_IndirectX,
AddressingMode_IndirectY
};

DisassemblerEntry disassemblerTable[256] = {
DisassemblerEntry(0x0, BRK_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x1, ORA_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x2, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x3, SLO_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x4, DOP_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x5, ORA_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x6, ASL_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x7, SLO_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x8, PHP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x9, ORA_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xa, ASL_Pretty, 1, AddressingMode_Accumulator),
DisassemblerEntry(0xb, AAC_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xc, TOP_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xd, ORA_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xe, ASL_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xf, SLO_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x10, BPL_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0x11, ORA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x12, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x13, SLO_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x14, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x15, ORA_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x16, ASL_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x17, SLO_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x18, CLC_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x19, ORA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x1a, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x1b, SLO_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x1c, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x1d, ORA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x1e, ASL_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x1f, SLO_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x20, JSR_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x21, AND_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x22, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x23, RLA_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x24, BIT_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x25, AND_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x26, ROL_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x27, RLA_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x28, PLP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x29, AND_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x2a, ROL_Pretty, 1, AddressingMode_Accumulator),
DisassemblerEntry(0x2b, AAC_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x2c, BIT_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x2d, AND_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x2e, ROL_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x2f, RLA_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x30, BMI_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0x31, AND_Pretty, 2, AddressingMode_IndirectY),
DisassemblerEntry(0x32, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x33, RLA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x34, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x35, AND_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x36, ROL_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x37, RLA_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x38, SEC_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x39, AND_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x3a, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x3b, RLA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x3c, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x3d, AND_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x3e, ROL_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x3f, RLA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x40, RTI_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x41, EOR_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x42, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x43, SRE_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x44, DOP_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x45, EOR_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x46, LSR_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x47, SRE_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x48, PHA_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x49, EOR_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x4a, LSR_Pretty, 1, AddressingMode_Accumulator),
DisassemblerEntry(0x4b, ASR_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x4c, JMP_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x4d, EOR_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x4e, LSR_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x4f, SRE_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x50, EOR_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x51, EOR_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x52, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x53, SRE_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x54, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x55, EOR_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x56, LSR_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x57, SRE_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x58, CLI_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x59, EOR_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x5a, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x5b, SRE_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x5c, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x5d, UNK_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x5e, LSR_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x5f, SRE_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x60, RTS_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x61, ADC_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x62, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x63, RRA_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x64, DOP_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x65, ADC_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x66, ROR_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x67, RRA_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x68, PLA_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x69, ADC_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x6a, ROR_Pretty, 1, AddressingMode_Accumulator),
DisassemblerEntry(0x6b, ARR_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x6c, JMP_Pretty, 3, AddressingMode_Indirect),
DisassemblerEntry(0x6d, UNK_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x6e, ROR_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x6f, RRA_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x70, BVS_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0x71, ADC_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x72, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x73, RRA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x74, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x75, ADC_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x76, ROR_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x77, RRA_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x78, SEI_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x79, ADC_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x7a, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x7b, RRA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x7c, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x7d, UNK_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x7e, ROR_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x7f, RRA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x80, DOP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x81, STA_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x82, DOP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x83, AAX_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0x84, STY_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x85, STA_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x86, STX_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x87, AAX_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0x88, DEY_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x89, DOP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x8a, TXA_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x8b, XAA_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0x8c, STY_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x8d, STA_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x8e, STX_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x8f, AAX_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0x90, STA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x91, STA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x92, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x93, AXA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0x94, STY_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x95, STA_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0x96, STX_Pretty, 2, AddressingMode_ZeroPageY),
DisassemblerEntry(0x97, AAX_Pretty, 2, AddressingMode_ZeroPageY),
DisassemblerEntry(0x98, TYA_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x99, STA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x9a, TXS_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x9b, XAS_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x9c, SYA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0x9d, UNK_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0x9e, SXA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0x9f, AXA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xa0, LDY_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xa1, LDA_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xa2, LDX_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xa3, LAX_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xa4, LDY_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xa5, LDA_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xa6, LDX_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xa7, LAX_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xa8, TAY_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xa9, LDA_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xaa, TAX_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xab, ATX_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xac, LDY_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xad, LDA_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xae, LDX_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xaf, LAX_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xb0, BCS_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0xb1, LDA_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xb2, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xb3, LAX_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xb4, LDY_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xb5, LDA_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xb6, LDX_Pretty, 2, AddressingMode_ZeroPageY),
DisassemblerEntry(0xb7, LAX_Pretty, 2, AddressingMode_ZeroPageY),
DisassemblerEntry(0xb8, CLV_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xb9, LDA_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xba, TSX_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xbb, LAR_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xbc, LDY_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xbd, LDA_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xbe, LDX_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xbf, LAX_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xc0, CPY_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xc1, CMP_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xc2, DOP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xc3, DCP_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xc4, CPY_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xc5, CMP_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xc6, DEC_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xc7, DCP_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xc8, INY_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xc9, CMP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xca, DEX_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xcb, AXS_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xcc, CPY_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xcd, CMP_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xce, DEC_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xcf, DCP_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xd0, BNE_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0xd1, CMP_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xd2, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xd3, DCP_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xd4, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xd5, CMP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xd6, DEC_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xd7, DCP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xd8, CLD_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xd9, CMP_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xda, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xdb, DCP_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xdc, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xdd, CMP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xde, DEC_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xdf, DCP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xe0, CPX_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xe1, SBC_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xe2, DOP_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xe3, ISC_Pretty, 2, AddressingMode_IndirectX),
DisassemblerEntry(0xe4, CPX_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xe5, SBC_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xe6, INC_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xe7, ISC_Pretty, 2, AddressingMode_ZeroPage),
DisassemblerEntry(0xe8, INX_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xe9, SBC_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xea, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xeb, SBC_Pretty, 2, AddressingMode_Immediate),
DisassemblerEntry(0xec, CPX_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xed, SBC_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xee, INC_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xef, ISC_Pretty, 3, AddressingMode_Absolute),
DisassemblerEntry(0xf0, BEQ_Pretty, 2, AddressingMode_Relative),
DisassemblerEntry(0xf1, SBC_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xf2, KIL_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xf3, ISC_Pretty, 2, AddressingMode_IndirectPlusY),
DisassemblerEntry(0xf4, DOP_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xf5, SBC_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xf6, INC_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xf7, ISC_Pretty, 2, AddressingMode_ZeroPageX),
DisassemblerEntry(0xf8, SED_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xf9, SBC_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xfa, NOP_Pretty, 1, AddressingMode_Implied),
DisassemblerEntry(0xfb, ISC_Pretty, 3, AddressingMode_AbsoluteY),
DisassemblerEntry(0xfc, TOP_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xfd, SBC_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xfe, INC_Pretty, 3, AddressingMode_AbsoluteX),
DisassemblerEntry(0xff, ISC_Pretty, 3, AddressingMode_AbsoluteX)
};
		//END static data


		std::string disassemble(word programCounter, IMemory* memory)
		{
			byte opcode = memory->read(programCounter);

			auto entry = disassemblerTable[opcode];

			std::vector<byte> arguments;
			// Fetch the arguments (if any)
			for(uint32 i=0; i<entry.numBytes-1; ++i)
			{
				arguments.push_back(memory->read(++programCounter));
			}

			// Output read bytes
			std::string temp;
			temp.reserve(1024);
			std::stringstream resultBuffer(temp);

			resultBuffer << std::hex << std::uppercase << std::setfill('0');
			resultBuffer << std::setw(2) << (int)opcode;

			for(auto arg : arguments)
			{
				resultBuffer << " "  << std::setw(2) << (int)arg;
			}

			for(uint32 i=0; i<(3-entry.numBytes); ++i)
			{
				resultBuffer << "   ";
			}

			// Output opcode name
			resultBuffer << "  " << prettyOpcodeName[entry.prettyOpcodeIndex] << " ";

			// Output arguments
			switch(entry.addressingMode)
			{
				case AddressingMode_Absolute:
				{
					word value;
					value.setLowByte(arguments[0]);
					value.setHighByte(arguments[1]);

					resultBuffer << "$" << std::setw(4) << (int)value;
					break;
				}
				case AddressingMode_AbsoluteX:
				{
					word value;
					value.setLowByte(arguments[0]);
					value.setHighByte(arguments[1]);

					resultBuffer << "$" << std::setw(4) << (int)value << ",X";
					break;
				}
				case AddressingMode_AbsoluteY:
				{
					word value;
					value.setLowByte(arguments[0]);
					value.setHighByte(arguments[1]);

					resultBuffer << "$" << std::setw(4) << (int)value << ",Y";
					break;
				}
				case AddressingMode_Accumulator:
					resultBuffer << "A";
					break;
				case AddressingMode_Immediate:
					resultBuffer << "#$" << std::setw(2) << (int)arguments[0];
					break;
				case AddressingMode_Implied:
					break;
				case AddressingMode_Indirect:
				{
					word value;
					value.setLowByte(arguments[0]);
					value.setHighByte(arguments[1]);

					resultBuffer << "($" << std::setw(4) << (int)value << ")";
					break;
				}
				case AddressingMode_Relative:
				{
					offset relativeByte = static_cast<offset>(arguments[0]);
					word resultAddress = static_cast<word>(programCounter + relativeByte + 1);
					resultBuffer << "$" << std::setw(4) << (int)resultAddress;
					break;
				}
				case AddressingMode_ZeroPage:
					resultBuffer << "$" << std::setw(2) << (int)arguments[0];
					break;
				case AddressingMode_ZeroPageX:
					resultBuffer << "$" << std::setw(2) << (int)arguments[0] << ",X";
					break;
				case AddressingMode_ZeroPageY:
					resultBuffer << "$" << std::setw(2) << (int)arguments[0] << ",Y";
					break;
				case AddressingMode_IndirectPlusY:
					resultBuffer << "($" << std::setw(2) << (int)arguments[0] << "),Y";
					break;
				case AddressingMode_IndirectX:
					resultBuffer << "($" << std::setw(2) << (int)arguments[0] << ",X)";
					break;
				case AddressingMode_IndirectY:
					resultBuffer << "($" << std::setw(2) << (int)arguments[0] << ",Y)";
					break;
			}

			return resultBuffer.str();
		}
	}
}