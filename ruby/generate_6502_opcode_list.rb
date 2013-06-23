#!/usr/bin/env ruby
#sukiNES opcode generator
#------------------------
#Copyright (c) 2013, Michael Larouche
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#    * Neither the name of the <organization> nor the
#      names of its contributors may be used to endorse or promote products
#      derived from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
#DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

class DisassemblerEntry
	attr_accessor :numBytes
	attr_accessor :opcodeName
	attr_accessor :addressingMode

 	def initialize(numBytes, opcodeName, mode)
		@numBytes = numBytes
		@opcodeName = opcodeName
		@addressingMode = mode
	end
end

# Input data
opcodeListString = <<LIST
Immediate   |ADC|69|2
Zero Page   |ADC|65|2
Zero Page,X |ADC|75|2
Absolute    |ADC|60|3
Absolute,X  |ADC|70|3
Absolute,Y  |ADC|79|3
(Indirect,X)|ADC|61|2
(Indirect),Y|ADC|71|2
Immediate   |AND|29|2
Zero Page   |AND|25|2
Zero Page,X |AND|35|2
Absolute    |AND|2D|3
Absolute,X  |AND|3D|3
Absolute,Y  |AND|39|3
(Indirect,X)|AND|21|2
(Indirect,Y)|AND|31|2
Accumulator |ASL|0A|1
Zero Page   |ASL|06|2
Zero Page,X |ASL|16|2
Absolute    |ASL|0E|3
Absolute,X  |ASL|1E|3
Relative    |BCC|90|2
Relative    |BCS|B0|2
Relative    |BEQ|F0|2
Zero Page   |BIT|24|2
Absolute    |BIT|2C|3
Relative    |BMI|30|2
Relative    |BNE|D0|2
Relative    |BPL|10|2
Implied     |BRK|00|1
Relative    |BVC|50|2
Relative    |BVS|70|2
Implied     |CLC|18|1
Implied     |CLD|D8|1
Implied     |CLI|58|1
Implied     |CLV|B8|1
Immediate   |CMP|C9|2
Zero Page   |CMP|C5|2
Zero Page,X |CMP|D5|2
Absolute    |CMP|CD|3
Absolute,X  |CMP|DD|3
Absolute,Y  |CMP|D9|3
(Indirect,X)|CMP|C1|2
(Indirect),Y|CMP|D1|2
Immediate   |CPX|E0|2
Zero Page   |CPX|E4|2
Absolute    |CPX|EC|3
Immediate   |CPY|C0|2
Zero Page   |CPY|C4|2
Absolute    |CPY|CC|3
Zero Page   |DEC|C6|2
Zero Page,X |DEC|D6|2
Absolute    |DEC|CE|3
Absolute,X  |DEC|DE|3
Implied     |DEX|CA|1
Implied     |DEY|88|1
Immediate   |EOR|49|2
Zero Page   |EOR|45|2
Zero Page,X |EOR|55|2
Absolute    |EOR|4D|3
Absolute,X  |EOR|50|3
Absolute,Y  |EOR|59|3
(Indirect,X)|EOR|41|2
(Indirect),Y|EOR|51|2
Zero Page   |INC|E6|2
Zero Page,X |INC|F6|2
Absolute    |INC|EE|3
Absolute,X  |INC|FE|3
Implied     |INX|E8|1
Implied     |INY|C8|1
Absolute    |JMP|4C|3
Indirect    |JMP|6C|3
Absolute    |JSR|20|3
Immediate   |LDA|A9|2
Zero Page   |LDA|A5|2
Zero Page,X |LDA|B5|2
Absolute    |LDA|AD|3
Absolute,X  |LDA|BD|3
Absolute,Y  |LDA|B9|3
(Indirect,X)|LDA|A1|2
(Indirect),Y|LDA|B1|2
Immediate   |LDX|A2|2
Zero Page   |LDX|A6|2
Zero Page,Y |LDX|B6|2
Absolute    |LDX|AE|3
Absolute,Y  |LDX|BE|3
Immediate   |LDY|A0|2
Zero Page   |LDY|A4|2
Zero Page,X |LDY|B4|2
Absolute    |LDY|AC|3
Absolute,X  |LDY|BC|3
Accumulator |LSR|4A|1
Zero Page   |LSR|46|2
Zero Page,X |LSR|56|2
Absolute    |LSR|4E|3
Absolute,X  |LSR|5E|3
Implied     |NOP|EA|1
Immediate   |ORA|09|2
Zero Page   |ORA|05|2
Zero Page,X |ORA|15|2
Absolute    |ORA|0D|3
Absolute,X  |ORA|1D|3
Absolute,Y  |ORA|19|3
(Indirect,X)|ORA|01|2
(Indirect),Y|ORA|11|2
Implied     |PHA|48|1
Implied     |PHP|08|1
Implied     |PLA|68|1
Implied     |PLP|28|1
Accumulator |ROL|2A|1
Zero Page   |ROL|26|2
Zero Page,X |ROL|36|2
Absolute    |ROL|2E|3
Absolute,X  |ROL|3E|3
Accumulator |ROR|6A|1
Zero Page   |ROR|66|2
Zero Page,X |ROR|76|2
Absolute    |ROR|6E|3
Absolute,X  |ROR|7E|3
Implied     |RTI|40|1
Implied     |RTS|60|1
Immediate   |SBC|E9|2
Zero Page   |SBC|E5|2
Zero Page,X |SBC|F5|2
Absolute    |SBC|ED|3
Absolute,X  |SBC|FD|3
Absolute,Y  |SBC|F9|3
(Indirect,X)|SBC|E1|2
(Indirect),Y|SBC|F1|2
Implied     |SEC|38|1
Implied     |SED|F8|1
Implied     |SEI|78|1
Zero Page   |STA|85|2
Zero Page,X |STA|95|2
Absolute    |STA|8D|3
Absolute,X  |STA|90|3
Absolute,Y  |STA|99|3
(Indirect,X)|STA|81|2
(Indirect),Y|STA|91|2
Zero Page   |STX|86|2
Zero Page,Y |STX|96|2
Absolute    |STX|8E|3
Zero Page   |STY|84|2
Zero Page,X |STY|94|2
Absolute    |STY|8C|3
Implied     |TAX|AA|1
Implied     |TAY|A8|1
Implied     |TSX|BA|1
Implied     |TXA|8A|1
Implied     |TXS|9A|1
Implied     |TYA|98|1
Immediate   |AAC|0B|2
Immediate   |AAC|2B|2
Zero Page   |AAX|87|2
Zero Page,Y |AAX|97|2
(Indirect,X)|AAX|83|2
Absolute    |AAX|8F|3
Immediate   |ARR|6B|2
Immediate   |ASR|4B|2
Immediate   |ATX|AB|2
Absolute,Y  |AXA|9F|3
(Indirect),Y|AXA|93|2
Immediate   |AXS|CB|2
Zero Page   |DCP|C7|2
Zero Page,X |DCP|D7|2
Absolute    |DCP|CF|3
Absolute,X  |DCP|DF|3
Absolute,Y  |DCP|DB|3
(Indirect,X)|DCP|C3|2
(Indirect),Y|DCP|D3|2
Zero Page   |DOP|04|2
Zero Page,X |DOP|14|2
Zero Page,X |DOP|34|2
Zero Page   |DOP|44|2
Zero Page,X |DOP|54|2
Zero Page   |DOP|64|2
Zero Page,X |DOP|74|2
Immediate   |DOP|80|2
Immediate   |DOP|82|2
Immediate   |DOP|89|2
Immediate   |DOP|C2|2
Zero Page,X |DOP|D4|2
Immediate   |DOP|E2|2
Zero Page,X |DOP|F4|2
Zero Page   |ISC|E7|2
Zero Page,X |ISC|F7|2
Absolute    |ISC|EF|3
Absolute,X  |ISC|FF|3
Absolute,Y  |ISC|FB|3
(Indirect,X)|ISC|E3|2
(Indirect),Y|ISC|F3|2
Implied     |KIL|02|1
Implied     |KIL|12|1
Implied     |KIL|22|1
Implied     |KIL|32|1
Implied     |KIL|42|1
Implied     |KIL|52|1
Implied     |KIL|62|1
Implied     |KIL|72|1
Implied     |KIL|92|1
Implied     |KIL|B2|1
Implied     |KIL|D2|1
Implied     |KIL|F2|1
Absolute,Y  |LAR|BB|3
Zero Page   |LAX|A7|2
Zero Page,Y |LAX|B7|2
Absolute    |LAX|AF|3
Absolute,Y  |LAX|BF|3
(Indirect,X)|LAX|A3|2
(Indirect),Y|LAX|B3|2
Implied     |NOP|1A|1
Implied     |NOP|3A|1
Implied     |NOP|5A|1
Implied     |NOP|7A|1
Implied     |NOP|DA|1
Implied     |NOP|FA|1
Zero Page   |RLA|27|2
Zero Page,X |RLA|37|2
Absolute    |RLA|2F|3
Absolute,X  |RLA|3F|3
Absolute,Y  |RLA|3B|3
(Indirect,X)|RLA|23|2
(Indirect),Y|RLA|33|2
Zero Page   |RRA|67|2
Zero Page,X |RRA|77|2
Absolute    |RRA|6F|3
Absolute,X  |RRA|7F|3
Absolute,Y  |RRA|7B|3
(Indirect,X)|RRA|63|2
(Indirect),Y|RRA|73|2
Immediate   |SBC|EB|2
Zero Page   |SLO|07|2
Zero Page,X |SLO|17|2
Absolute    |SLO|0F|3
Absolute,X  |SLO|1F|3
Absolute,Y  |SLO|1B|3
(Indirect,X)|SLO|03|2
(Indirect),Y|SLO|13|2
Zero Page   |SRE|47|2
Zero Page,X |SRE|57|2
Absolute    |SRE|4F|3
Absolute,X  |SRE|5F|3
Absolute,Y  |SRE|5B|3
(Indirect,X)|SRE|43|2
(Indirect),Y|SRE|53|2
Absolute,Y  |SXA|9E|3
Absolute,X  |SYA|9C|3
Absolute    |TOP|0C|3
Absolute,X  |TOP|1C|3
Absolute,X  |TOP|3C|3
Absolute,X  |TOP|5C|3
Absolute,X  |TOP|7C|3
Absolute,X  |TOP|DC|3
Absolute,X  |TOP|FC|3
Immediate   |XAA|8B|2
Absolute,Y  |XAS|9B|3
LIST

# Associate an opcode to a entry
opcodeEntryHash = {}
# List of opcode for the CPU instructions
opcodeList = []

opcodeEntryHashRubyCode = "opcodeEntryHash = { "

# Parse the input date to extract the opcodes and argument information
opcodeListString.split("\n").each do |line|
	lineComponents = line.strip.split("|")
	
	addressingMode = lineComponents[0].strip
	
	opcodeList << lineComponents[1]

	opcodeName = "\"#{lineComponents[1]}\""

	opcodeHex = lineComponents[2]
	numBytes = lineComponents[3]

	opcodeEntryHashRubyCode += "0x#{opcodeHex} => DisassemblerEntry.new(#{numBytes},#{opcodeName},\"#{addressingMode}\"),"
end
opcodeEntryHashRubyCode += "}"

# Add UNK to the list
opcodeList << "UNK"

# Remove duplicate in list
opcodeList = opcodeList.uniq.sort

# Populate the opcodeEntryHash hash.
eval opcodeEntryHashRubyCode

addressingModeHash =
{
	"Absolute" => "AddressingMode_Absolute",
	"Absolute,X" => "AddressingMode_AbsoluteX",
	"Absolute,Y" => "AddressingMode_AbsoluteY",
	"Accumulator" => "AddressingMode_Accumulator",
	"Immediate" => "AddressingMode_Immediate",
	"Implied" => "AddressingMode_Implied",
	"Indirect" => "AddressingMode_Indirect",
	"Relative" => "AddressingMode_Relative",
	"Zero Page" => "AddressingMode_ZeroPage",
	"Zero Page,X" => "AddressingMode_ZeroPageX",
	"Zero Page,Y" => "AddressingMode_ZeroPageY",
	"(Indirect),Y" => "AddressingMode_IndirectPlusY",
	"(Indirect,X)" => "AddressingMode_IndirectX",
	"(Indirect,Y)" => "AddressingMode_IndirectY"
}

# Generate the table for the opcode name
# along with the enum index list
prettyOpcodeName = []
prettyOpcodeEnumList = []
formattedOpcodeNameList = []

index = 0
opcodeList.each do |opcode|
	prettyOpcodeName  << "#{opcode}_Pretty"
	prettyOpcodeEnumList << "#{opcode}_Pretty = #{index}"
	formattedOpcodeNameList << "\"#{opcode}\""
	index += 1
end

puts "const char prettyOpcodeName[][4] = {"
puts formattedOpcodeNameList.join(",")
puts "};"
puts ""

puts "enum PrettyOpcodeIndex {"
puts prettyOpcodeEnumList.join(",\n")
puts "};"
puts ""

# Generate the dissambler table
disassemblerTableList = []

(0..0xFF).each do |opcodeHex|
	entry = opcodeEntryHash[opcodeHex]

	if entry == nil then
		entry = DisassemblerEntry.new(1, "UNK", "Implied")
	end
	
	opcodeIndex = opcodeList.index(entry.opcodeName)
	disassemblerTableList << "DisassemblerEntry(0x#{opcodeHex.to_s(16)}, #{prettyOpcodeName[opcodeIndex]}, #{entry.numBytes}, #{addressingModeHash[entry.addressingMode]})"
end

puts "enum AddressingMode {"
puts addressingModeHash.values.join(",\n")
puts "};"
puts ""

puts "DisassemblerEntry disassemblerTable[256] = {"
puts disassemblerTableList.join(",\n")
puts "};"