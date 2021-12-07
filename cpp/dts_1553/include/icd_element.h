#ifndef ICD_ELEMENT_H
#define ICD_ELEMENT_H

#include <string>
#include <cstdint>
#include <vector>
#include "parse_text.h"
#include "yaml-cpp/yaml.h"

enum class ICDElementSchema : uint8_t
{
    // Not currently used. This value is here to be
    // backwards-compatible with legacy code.
    MULTIPLE_BIT = 0,

    // MilStd1553B mode code. Relevant to 1553 bus operation.
    MODE_CODE = 1,

    // IEEE standard twos-complement signed 16-bit integer.
    SIGNED16 = 2,

    // IEEE standard two-complement signed 32-bit integer.
    SIGNED32 = 3,

    // IEEE standard unsigned 16-bit integer.
    UNSIGNED16 = 4,

    // IEEE standard unsigned 32-bit integer.
    UNSIGNED32 = 5,

    // MilStd1750 32-bit floating point value.
    FLOAT32_1750 = 6,

    // IEEE 32-bit floating point value.
    FLOAT32_IEEE = 7,

    // IEEE 64-bit floating point value.
    FLOAT64_IEEE = 8,

    // 16-bit floating point value -- standard currently uknown.
    FLOAT16 = 9,

    // 8-bit ASCII
    ASCII = 10,

    // Two or more consecutive bits to be interpreted as a twos-complement signed integer.
    // SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.
    SIGNEDBITS = 11,

    // Single-bit boolean or multiple bits to be interpreted as unsigned integer.
    // SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.
    UNSIGNEDBITS = 12,

    // Default/initial value.
    BAD = 13,

    // Collins Adaptive Processing System (CAPS) 48-bit floating point value.
    CAPS = 14,

    // 32-bit floating point value -- standard currently unknown.
    FLOAT32_GPS = 15,

    // 64-bit floating point value -- standard currently unknown.
    FLOAT64_GPS = 16,

};

class ICDElement
{
   private:
    // tools
    ParseText pt;

    // data members
    const int kICDElementSchemaMaxValue = 16;

   public:
    // Count of input columns required to fill data members.
    static const int kFillElementCount;

    // Channel ID as obtained from the Ch10. This
    // is a value that will have to be matched based on the ICD
    // and a bus mapping. The bus name to channel ID mapping
    // must be done for each Ch10 file, because it can vary.
    uint16_t channel_id_;

    //
    // ICD text file elements
    //

    /*
		-name:			msg_name
		-description:	Name of message
		-format/range:
		-special cases:
		-examples:
			1. MSG01
			2. MSG02		
					
	*/
    std::string msg_name_;

    /*
		-name:			element_name
		-description:	Name of message element
		-format/range:	<message name>_<2 digit word number><2 digit bit number (if any)>
		-special cases:	 
		-examples:
			1. MSG01-01
				(element name of word 1 in message MSG01)
			2. MSG01-0105
				(element name of the bits starting at
				word 1 bit 5 of message MSG01)

	*/
    std::string elem_name_;

    /*
		-name:			xmit_word
		-description:	MilStd1553B 16-bit transmit comand word
		-format/range:	1...RT address (bits 1-5)
						2...
						3...
						4...
						5...
						6...Transmit/Recieve (1 transmit, 0 recieve)
						7...Subaddress (bits 7-11)
						8...
						9...
						10..
						11..
						12..Word Count/Mode Code (bits 12-16)
						13..
						14..
						15..
						16..
		-special cases: 
			1. set xmit_word to 0 if message is Bus Controller to Remote Terminal
			2. word count bits are 0b00000 if word count = 32
			3. subaddress = 0b00000 or 0b11111 for mode codes
			4. if the subaddress is 0b00000 or 0b11111 the word count bits (bits 12-16)
			   no longer define word count. They  define a mode command. Mode 
			   commands of 0b00000 through 0b01111 are used for mode codes which do
               not require transfer of a data word. For these codes, the T/R bit 
			   is set to 1. Codes 0b10000 through 0b11111 are only used for mode codes 
			   that require transfer of a single data word. For these mode codes, 
			   the T/R bit should be set to indicate the direction of data word flow.
			   See section 4.3.3.5.1.7 of below link for more information on mode codes.
			   http://www.milstd1553.com/wp-content/uploads/2012/12/MIL-STD-1553B.pdf
		-examples:
			1.	remote terminal address = 8 (0b01000)
				transmit/recieve = 1 (always 1 for transmit command word)
				subaddress = 2 (0b00010)
				word count = 1 (0b00001)
				xmit_word  -> 17473 (0b0100010001000001)
	*/
    uint16_t xmit_word_;

    /*
		-name:			dest_word
		-description:	MilStd1553B 16-bit recieve comand word
		-format/range:	1...RT address (bits 1-5)
						2...
						3...
						4...
						5...
						6...Transmit/Recieve (1 transmit, 0 recieve)
						7...Subaddress (bits 7-11)
						8...
						9...
						10..
						11..
						12..Word Count/Mode Code (bits 12-16)
						13..
						14..
						15..
						16..
		-special cases: 
			1. set dest_word to 0 if message is Remote Terminal to Bus Controller
			2. word count bits are 0b00000 if word count = 32
			3. subaddress = 0b00000 or 0b11111 for mode codes
			4. if the subaddress is 0b00000 or 0b11111 the word count bits (bits 12-16)
			   no longer define word count. They  define a mode command. Mode 
			   commands of 0b00000 through 0b01111 are used for mode codes which do
               not require transfer of a data word. For these codes, the T/R bit 
			   is set to 1. Codes 0b10000 through 0b11111 are only used for mode codes 
			   that require transfer of a single data word. For these mode codes, 
			   the T/R bit should be set to indicate the direction of data word flow.
			   See section 4.3.3.5.1.7 of below link for more information on mode codes.
			   http://www.milstd1553.com/wp-content/uploads/2012/12/MIL-STD-1553B.pdf
		-examples:
			1.	remote terminal address = 1 (0b00001)
				transmit/recieve = 0 (always 0 for transmit command word)
				subaddress = 3 (0b00011)
				word count = 32 (0b00000)
				xmit_word  -> 2144 (0b0000100001100000)
	*/
    uint16_t dest_word_;

    /*
		-name:			msg_word_count
		-description:	Number of 16-bit words in a 1553 message.
						This is a count of the data words
						transmitted after the command word.
						It is not a direct read out from the
						word count bits (12-16) in the command word. 
						The word count bits in the command word have 
						special cases as described above
		-format/range:	[0 - 32]
		-special cases: 
		-examples:
			1. 32 word message
				msg_word_count = 32
			2. Zero Payload Mode Code
				msg_word_count = 0
	*/
    uint8_t msg_word_count_;

    /*
		-name:			bus_name
		-description:	Name of bus on which message is transmitted
		-format/range:	
		-special cases:
		-examples:
	*/
    std::string bus_name_;

    /*
		-name:			xmit_lru_name
		-description:	Transmit LRU name
		-format/range:	
		-special cases:
		-examples:
	*/
    std::string xmit_lru_name_;

    /*
		-name:			xmit_lru_addr
		-description:	Transmit LRU address
		-format/range:	[0,31]
		-special cases:	0 for bus controller (BC->RT)
		-examples:
	*/
    uint8_t xmit_lru_addr_;

    /*
		-name:			dest_lru_name
		-description:	Recieve LRU name
		-format/range:	
		-special cases: 
		-examples:
	*/
    std::string dest_lru_name_;

    /*
		-name:			dest_lru_name
		-description:	Recieve LRU address
		-format/range:  [0,31]
		-special cases: 0 for bus controller (RT->BC)
		-examples:
	*/
    uint8_t dest_lru_addr_;

    /*
		-name:			xmit_lru_subaddr
		-description:	Transmit subaddress
		-format/range:	[0,31]
		-special cases: 0 or 31 for mode code
		-examples:
	*/
    uint8_t xmit_lru_subaddr_;

    /*
		-name:			dest_lru_subaddr
		-description:	Recieve subaddress
		-format/range:	[0,31]
		-special cases: 0 or 31 for mode code
		-examples:
	*/
    uint8_t dest_lru_subaddr_;

    /*
		-name:			rate
		-description:	Message transfer rate
		-format/range:	Hz
		-special cases:
		-examples:
			1. 50.0
			2. 3.125
	*/
    float rate_;

    /*
		-name:			offset
		-description:	Integer offset of first
						16-bit raw data word 1553 msg payload in which 
						the bits that define this element value
						are found.
		-format/range:	[0,31]
		-special cases:
		-examples:
			1. for element MSG01-01 (word one)
				offset = 0
			2. MSG01-0510 (word 5 bit 10)
				offset = 4
	*/
    uint8_t offset_;

    /*
		-name:			elem_word_count
		-description:	Quantity of 16-bit words, starting at offset,
						required to recover the relevant bits necessary
						to define the value.
		-format/range:	[1,4]
		-special cases:
		-examples:
			1. 64 bit element (4 words)
				elem_word_count = 4
			2. bit level element spanning two words
				elem_word_count = 2
			3. single bit element
				elem_word_count = 1
			4. 32 bit element (2 words)
				elem_word_count = 2
	*/
    uint8_t elem_word_count_;

    /*
		-name:			schema
		-description:	Enum representative of comet schema. 
						Refer to ICDElementSchema above for 
						integer values and more detail.
						Ingested from ICD text file as unsigned
						integer value.
		-format/range:	[1,16]
		-special cases:
		-examples:
			1. single bit level
				schema = 12
			2. unsigned bit level element spanning two words
				schema = 12
			3. two word signed IEEE integer (SIGNED32)
				schema = 3
				
	*/
    ICDElementSchema schema_;

    /*
		-name:			is_bitlevel
		-description:	If true, indicates that the element is a 
						bit-level specification, in contrast with 
						word-level spec. If false then the element
						is word-level spec and will be comprised of
						one or more 16-bit words. SIGNEDBITS or 
						UNSIGNEDBITS schema should be used when
						is_bitlevel_ is true.
		-format/range:	[0,1] 1 = bit level, 0 = not bit level
		-special cases:
		-examples:
			1. element defined by word2 bit5 through word3 bit 6
				is_bitlevel = 1
			2. element defined by 32 bit IEEE integer (2 words)
				is_bitlevel = 0
	*/
    bool is_bitlevel_;

    /*
		-name:			is_multiformat
		-description:	Indicates if the element is multi_format.
						See comet flat files description for more
						information.
						Multiformat elements are word- or bit-
						level elements which have the '@' symbol
						in their name (elem_name_). The multiformat
						mechanism provides a way to economize 
						1553 message slots on the bus while offering
						different information as intended by OFP 
						developers. The decimal value that follows the
						'@' symbol in the element name indicates which
						multiformat element version is being defined.
		-format/range:	[0,1] 1 = multi format, 0 = not multi format
		-special cases:
		-examples:
			1. MSG01-01@01 
				is_multiformat = 1
			2. MSG01-02
				is_multiformat = 0
	*/
    bool is_multiformat_;

    /*
		-name:			bitmsb
		-description:	Only applies when is_bitlevel_ = true.
						Most significant bit of the first word
						in the bit-level spec. This follows the 
						comet standard for bit numbering. (1 in 0b1000
						is bit # 1, and 1 in 0b0001 is bit # 4)
		-format/range:	[1,16]
		-special cases: 
			1. only used when is_bitlevel = 1 (set to 0 when is_bitlevel = 0)
			2. bitmsb should be the signed bit position when using the SIGNEDBITS schema
		-examples:
			1. bit level element starting at word 3 bit 2
				bitmsb = 2
			2. single bit element at word 3 bit 4
				bitmsb = 4
	*/
    uint8_t bitmsb_;

    /*
		-name:			bitlsb
		-description:	Only applies when is_bitlevel_ = true.
						Least significant bit of the last word
						in the bit-level spec. This follows the 
						comet standard for bit numbering. (1 in 0b1000
						is bit # 1, and 1 in 0b0001 is bit # 4)
		-format/range:	[1,16]
		-special cases: 
			1. only used when is_bitlevel = 1 (set to 0 when is_bitlevel = 0)
		-examples:
			1. bit level element from word 3 bit 2 -> word 4 bit 5 
				bitlsb = 5
			2. single bit element at word 3 bit 4
				bitlsb = 4
	*/
    uint8_t bitlsb_;

    /*
		-name:			bit_count
		-description:	Only applies when is_bitlevel_ = true.
						Count of all bits that define the
						element value. If only one 16-bit word
						is needed then: 
						bitcount = bitlsb - bitmsb + 1,
						otherwise bitcount is
						bitcount = [1st word](16 - bitmsb + 1) +
						(word_count - 2) * 16 + [last word](bitlsb)
		-format/range:	[1,64]
		-special cases:
			1. only used when is_bitlevel = 1 (set to 0 when is_bitlevel = 0)
		-examples:
			1. bit level element from word 3 bit 2 -> word 5 bit 8
				bit_count = 15(word3) + 16(word4) + 8(word5) = 39
	*/
    uint8_t bit_count_;

    /*
		-name:			classification
		-description:	Classification value as found 
						in comet wrd tables.

						Classification is the classification level of the word. 
						Classification options are:
						0 - UNCLASSIFIED
						1 - CONFIDENTIAL
						2 - SECRET
						9 - VENDOR_PROPRIETARY

		-format/range:	0,1,2,9
		-special cases:
		-examples:
	*/
    uint8_t classification_;

    /*
		-name:			description
		-description:	element description
		-format/range:	string in quotes
		-special cases:
			1. not used for translation, if unsure set to ""
		-examples:
			1. "LATITUDE"
			2. "LONGITUDE"
	*/
    std::string description_;

    /*
		-name:			msb_val
		-description:	Value of the most significant bit. Used to 
						scale the translated value. As an example,
						for an 8 bit unsigned integer the scale factor
						would follow the formula 0b10000000 * scale = msb_val.
						The scaling would then be applied to each value coming 
						off the bus after it is read into the datatype specified
						by schema. Scaling is only done for IEEE standard integer, 
						and bit level schema types.
		-format/range: 
		-special cases: 
			1. note that the msb value for SIGNEDBIT types 
			   is associated with the bit at position 
			   bitmsb + 1 
			   (since bitmsb for SIGNEDBIT is defined to be
				in the sign bit location)
			2. sometimes comet flat files place a 0 in msb_val
			   for boolean. For that reason, msb_val is ignored
			   for boolean types.
		-examples:
			1. no scaling wanted for an unsigned IEEE 16 bit integer
				msb_val = 2^15
			2. no scaling wanted for an signed IEEE 32 bit integer
				msb_val = 2^30				
	*/
    double msb_val_;

    /*
		-name:			uom
		-description:	Unit of measure of the Engineering Unit.
		-format/range:	Generally all caps
		-special cases: 
			1. not used for translation, if unsure set to NONE
		-examples:
			1. FT-SEC
			2. PIRAD
	*/
    std::string uom_;

    // methods/c'tors
    ICDElement();
    ~ICDElement();
    ICDElement(const ICDElement& C);
    ICDElement& operator=(const ICDElement& C);
    bool Fill(const std::string& icdelem_str);
    bool FillElements(const std::vector<std::string>& input_str_vec);
};

#endif
