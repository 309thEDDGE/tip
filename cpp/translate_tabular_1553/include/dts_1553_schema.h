#ifndef DTS_1553_SCHEMA_H_
#define DTS_1553_SCHEMA_H_

#include <string>

// Legend:
// - (O)  = optional map with specific key (key is required, mapped value is not)
// - (R)  = required map with specific key
// - (WR) = wildcard placeholder for required user-defined input
// - (WO) = wildcard placeholder for optional user-defined input

// Wildcards represent user-defined strings and are used to 
// give names to 1553 messages or elements. Within a structure,
// a wildcard may be repeated many times to define all necessary 
// message and/or message elements.

// The inclusion requirement applies at each level of the structure 
// to the current level and down to lower-level structures. For example,
// the optional wildcard which is value mapped to the word_elem key
// may not be relevant for the current message, in which case 
// the word_elem key is mapped to a null value. It may also 
// be the case that the word_elem key maps to multiple 1553 
// word elements, in which case word_elem maps to another
// map with multiple key:value pairs, each of which is mapped
// to another a map which contains the keys off, cnt, schema, etc.

// See additional definitions below and the example DTS1553 yaml file at
const std::string dts_1553_schema = R"(
---
translatable_message_definitions:           # (R)
  _NOT_DEFINED_:                            # (WR), alias: msg_name = 1553 message name
    msg_data:                               # (R) 
      command: [INT]                        # (R)
      lru_addr: [INT]                       # (R)
      lru_subaddr: [INT]                    # (R)
      lru_name: [OPTSTR]                       # (OR)
      bus: STR                              # (R)
      wrdcnt: INT                           # (R)
      rate: FLT                             # (R)
      mode_code: OPTBOOL                       # (OR)
      desc: OPTSTR                             # (OR)
    word_elem:                              # (R)
      _NOT_DEFINED_OPT_:                    # (WO), alias: wrd_elem_name = word/multiple word element
        offset: INT                            # (R)
        cnt: INT                            # (R)
        schema: STR-->{SIGNED16,SIGNED32,UNSIGNED16,UNSIGNED32,FLOAT32_1750,FLOAT32_IEEE,FLOAT64_IEEE,FLOAT16,CAPS,FLOAT32_GPS,FLOAT64_GPS}     # (R)
        msbval: FLT                         # (R)
        desc: OPTSTR                           # (OR)
        uom: OPTSTR                            # (OR)
        multifmt: OPTBOOL                      # (OR)
        class: OPTINT                          # (OR)
    bit_elem:                               # (R)
      _NOT_DEFINED_OPT_:                    # (WO), alias: bit_elem_name = bit/multiple bit element
        offset: INT                            # (R)
        cnt: INT                            # (R)
        schema: STR-->{UNSIGNEDBITS,SIGNEDBITS,ASCII}  # (R)
        msbval: FLT                         # (R)
        desc: OPTSTR                           # (OR)
        uom: OPTSTR                            # (OR)
        multifmt: OPTBOOL                      # (OR)
        class: OPTINT                          # (OR)
        msb: INT                            # (R)
        lsb: INT                            # (R)
        bitcnt: INT                         # (R)
supplemental_bus_map_command_words:         # (R) 
  _NOT_DEFINED_OPT_:                        # (WO), alias: bus_name = name of a bus from which 1553 data were recorded
    - [INT]                                 # (R)
...	
)";

const std::string dts_1553_commword_explanation = R"(
 Command word breakdown:

 1...RT address (bits 1-5)
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

 Boolean values shall be denoted by strings. True shall
 be indicated by True or true and false shall be indicated
 by False or false. 
)";

const std::string dts_1553_parameter_defs = R"(
#### Parameter Definitions ####

 translatable_message_definitions
 
 Required, structure included exactly once in 1553
 data translation specification (DTS1553)
 
 Required top level key which indicates that all 
 structure below it defines 1553 messages to be 
 translated.


 
 _NOT_DEFINED_ 
 alias: msg_name

 Required, structure included at least once in the 
 translatable_message_definitions structure

 Required wildcard to be replaced by the user with a 1553
 message name. The data structure to which this key maps is 
 relevant to this message alone. The lower map with keys msg_data,
 word_elem and bit_elem are immediately below the 
 message name and are required. There may be as few as a single
 msg_name data structure or hundreds, depending on the content
 of the ch10.


 
 msg_data

 Required, structure included at least once in msg_name structure

 Contains data relevant to identifying a 1553 message. This key 
 maps to a lower map with the keys: command, lru_addr, lru_subaddr,
 lru_name, bus, wrdcnt, rate, mode_code, and desc.
 


 command
 
 Required, structure included exactly once in the current map

 Key maps to a sequence of two integers, the 1553 transmit command
 word followed by the receive command word. If the 1553 message is a 
 transmit type message, then the receive command word position is
 filled with a zero as a placeholder. If the message type is a 
 receive message, the transmit position
 shall contain filled a zero. An RT-to-RT type message shall include 
 both non-zero values.


 
 lru_addr
 
 Required, structure included exactly once in the current map

 Key maps to a sequence of two integers, the transmitting lru 
 address and the receiving lru address. These are the addresses 
 that can be determined from a decomposition of the command 
 words. BC-to-RT or RT-to-BC messages only utilize one command
 word, either receive or transmit, respectively, in which case the 
 transmit and receive command word, again respectively, are given
 the placeholder value of zero (see 'command' definition above).
 The same is true for the lru address, which is filled with the 
 placeholder value zero if the transmit or
 recieve command word is also zero. 
 

 
 lru_subaddr 

 Required, structure included exactly once in the current map
 
 Key maps to a sequence of two integers, the transmitting 
 lru subaddress followed by the receiving lru subaddress. This 
 mapping follows the convention that the lru subaddress is zero
 if the transmit or receive command word or lru address is also
 zero.



 lru_name
 
 Required, structure included exactly once in the current map
 
 Key maps to a sequence of two strings, the transmitting
 lru name or label followed by the receiving lru name or label.
 

 
 bus
 
 Required, structure included exactly once in the current map

 Key maps to a string which is the name of the bus on which the 
 relevant message occurs. This string ought to match the name 
 of the bus that is given in TMATS data, or at least be a substring
 of a possible bus name found in TMATS. 


 
 wrdcnt
 
 Required, structure included exactly once in the current map

 Key maps to an integer which is the count of data words transmitted
 in the current message. It does not include command or status words
 transmitted as part of the complete 1553 message transmission. This 
 value can also be determined from the command word(s). One exception
 to this definition is single-word payload mode code messages. It is
 possible that 'mode_code' (see below) is true and the value of the word
 count determined from the command word is not representative of the
 actual count of transmitted data words. Mode code type message word 
 count can only be zero or one and is not determined directly
 from decomposed bits of the command word. In this case, the user 
 must explicitly insert the correct word count in this field, either 
 zero or one. 
 


 rate
 
 Required, structure included exactly once in the current map

 Key maps to a float value which gives the message transmission
 rate with Hz unit.



 mode_code 

 Required, structure included exactly once in the current map
 
 Key maps to a boolean value. True indicates that the message 
 is a mode code type 1553 message and false indicates that the
 message is not a mode code. This boolean can be determined from
 the command word by the value of the subaddress field, which is 0 
 or 31 for mode_code = true and false otherwise.


 
 desc 
 
 Required, structure included exactly once in the current map
 
 Key maps to a string description of the 1553 message.



 word_elem
 
 Required, structure included exactly once in the current map

 Key is one of three keys at the current level of the data structure.
 The other two keys in the map are msg_data (see above) and 
 bit_elem (see below). The key maps to a map which has keys that 
 are the names of the 1553 message elements which are comprised of 
 an integer number of uint16 payload words. Examples include 
 uint16, int16, uint32, int32, uint64, int64, etc. There is another
 data structure to identify and intepret bit-level elements, those
 elements which are comprised of a single bit or multiple bits which
 do not compose an integer number of 1553 payload words. A bit-level
 element may be the first several bits in a word, the last several
 bits in a word, or some number of bits in the middle of a word or
 span an entire word plus the first several bits of the next word.


 
 _NOT_DEFINED_OPT_ 
 alias: wrd_elem_name
 
 Optional, structure may be included if the 1553 message contains
 payload data which ought to be interpreted as an integer number
 of uint16 words.

 Optional wildcard key to be replaced by the user-defined name of the 
 element to be translated from the raw 1553 payload. There may 
 be 0 or many of these data structures, where each structure
 defines exactly one element to be interpreted or translated 
 out of the payload for the given message.


 
 offset (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Key maps to an integer which is the offset within the 1553 
 message data payload of the word(s) to be used to translate
 the current element. This value is zero-indexed such that 
 first word in a 1553 message payload has offset zero.


 
 cnt (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Key maps to an integer which is the count of uint16 1553 message
 payload words to be used to interpret the value of the element.


 
 schema (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Key maps to a string which identifies a specific algorithm 
 for intepreting the word(s) defined as part of the current 
 element. All string values below are to be used only with the 
 'wrd_elem_name' map unless otherwise noted. The optional values are:
 
   // IEEE standard twos-complement signed 16-bit integer.
   SIGNED16

	// IEEE standard twos-complement signed 32-bit integer.
	SIGNED32

	// IEEE standard unsigned 16-bit integer.
	UNSIGNED16

	// IEEE standard unsigned 32-bit integer.
	UNSIGNED32

	// MilStd1750 32-bit floating point value.
	FLOAT32_1750

	// IEEE 32-bit floating point value.
	FLOAT32_IEEE

	// IEEE 64-bit floating point value.
	FLOAT64_IEEE

	// 16-bit floating point value -- standard currently uknown.
	FLOAT16

   // 8-bit ASCII
	ASCII

	// Two or more consecutive bits to be interpreted as a twos-complement signed integer.
	// SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.
   // This schema ought to be used only for bit-level elements defined under the 
   // 'bit_elem_name' map (see below).
	SIGNEDBITS

	// Single-bit boolean or multiple bits to be interpreted as unsigned integer.
	// SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.
   // This schema ought to be used only for bit-level elements defined under the 
   // 'bit_elem_name' map (see below).
	UNSIGNEDBITS

	// Collins Adaptive Processing System (CAPS) 48-bit floating point value.
	CAPS

	// 32-bit floating point value -- standard currently unknown.
	FLOAT32_GPS

	// 64-bit floating point value -- standard currently unknown.
	FLOAT64_GPS



 msbval (relative to wrd_elem_name data structure)

 Required, structure included exactly once in the current map

 Key maps to a floating point value giving the (potentially scaled)
 value of the most significant bit (MSB) in the value to be interpreted
 by the word(s). An example is an interpreted value which provides 
 a high-precision datum with a value between 0 and 1, indicates a 
 single data payload (wrdcnt = 1) and uint16 schema (schema = UNSIGNED16)
 and an msbval of 0.5. An msbval of 0.5 implies a scale factor 
 of 0.5 / 2^15 = 1.52588e-5, following the generic formula:
         msbval / (2 ^ (msb_position - 1)) = scale_factor
 If this word were to have all 16 bits high (word_value = 2^16 - 1)
 then the intended value is word_value * 1.5288e-5 ~= 1.0. The most 
 significant bit of signed values is one bit lower than the highest
 bit which is designated as the sign bit. Using a signed 16 bit word 
 as an example, scale_factor = msbval / 2^14. An unsigned 32-bit scale
 factor is computed by scale_factor = msbval / 2^31. Currently, floating point
 values do not use the msbval and a value of 1 ought to be used as 
 placeholder.



 desc (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Key maps to a string value describing the value to be interpreted
 or extracted from the raw payload. Ex: "height above ground".


 
 uom (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Key maps to a string value indicating the unit of the translated
 element. This string is not currently used at translation time. 
 If unknown or not relevant, a placeholder ought to be used. 


 
 multifmt (relative to wrd_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Key maps to a boolean value indicating that the element is a 
 multi-format type element. This only applies to certain platforms
 and the default value ought to be false.



 class (relative to wrd_elem_name data structure)

 Required, structure included exactly once in the current map

 Key maps to an integer which indicates the security classification of 
 the element. This value is not currently utilized. The default value 
 ought to be zero. 


 
 _NOT_DEFINED_OPT_
 alias: bit_elem_name
 
 Optional, structure may be included if the 1553 message contains
 payload data which ought to be interpreted from a collection of 
 contiguous bits.

 Optional wildcard key to be replaced by the user-defined name of the 
 element to be translated from the raw 1553 payload. There may 
 be 0 or many of these data structures, where each structure
 defines exactly one element to be interpreted or translated 
 out of the payload for the given message.



 offset (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Same meaning as defined in the wrd_elem_name data structure.


 
 cnt (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Same meaning as defined in the wrd_elem_name data structure.


 
 schema (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Same meaning as defined in the wrd_elem_name data structure.



 msbval (relative to bit_elem_name data structure)

 Required, structure included exactly once in the current map

 Same meaning as defined in the wrd_elem_name data structure.



 desc (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map
 
 Same meaning as defined in the wrd_elem_name data structure.


 
 uom (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Same meaning as defined in the wrd_elem_name data structure.


 
 multifmt (relative to bit_elem_name data structure)
 
 Required, structure included exactly once in the current map

 Same meaning as defined in the wrd_elem_name data structure.



 class (relative to bit_elem_name data structure)

 Required, structure included exactly once in the current map

 Same meaning as defined in the wrd_elem_name data structure.



 msb

 Required, structure included exactly once in the current map

 Key maps to an integer which identifies the most significant bit
 within the first word (often the only word) from which bits
 shall be extracted to form the intended value. Bits within 
 each 16-bit word are 1-indexed beginning at the MSB of the 
 word. For example, bit 1 is the MSB and bit 16 is the LSB in 
 the 16-bit word. The msb is the sign bit for signed elements.

 Example 1: bit level element starting at word 3 bit 2 has 
 msb = 2
 Example 2: single bit element at word 3 bit 4 has msb = 4



 lsb

 Required, structure included exactly once in the current map

 Key maps to an integer which identifies the least significant bit 
 within the final word (often the first and only word) from which
 bits shall be extracted to from the intended value, following a 
 1-indexed convention with bit 1 the MSB and bit 16 the LSB
 within the 16-bit word. For example, a four bit sequence which represents,
 via the raw bits, a value in the range 0--15, may be specified by
 bits msb=4 and lsb=7. Bit 4, fourth most signficant, through bit 
 7, 7th most significant, comprise four bits which shall be extracted
 by the translator and scaled according to the msbval.

 If the bits which comprise the value span multiple words, the
 lsb shall represent the least significant bit in the final word
 and thus always have a value in the range [1, 16].
 Example 1: bit level element from word 3 bit 2 -> word 4 bit 5 
 has lsb = 5.
 Example 2: single bit element at word 3 bit 4 has lsb = 4
 Example 3: multiple bit element which spans multiple words 
 has cnt = 3, msb = 15 and lsb = 2. Two bits are extracted
 from the first word [15, 16], all 16 bits from the second word,
 and two bits from the third word [1, 2] for a total of 
 20 bits. Bit 15 from the first word is the most significant bit,
 the bits from the second word are in the middle, and the least 
 signficant bit is bit 2 from the third word. 
 


 bitcnt

 Required, structure included exactly once in the current map

 Key maps to a integer which gives the explicit count of bits  
 to be extracted from the the word(s) at offset 'off'. This 
 value is redudant and ought to be equal to lsb - msb + 1 for 
 bit-level elements which span only a single word. The generic 
 formula which includes the possibility of multi-word bit-level
 elements is bitcnt = (lsb - msb + 1) + (16 * (cnt - 1)), where 
 cnt is the word count defined above.
)";

const std::string dts_1553_suppl_commwords = R"(
 supplemental_bus_map_command_words

 Required, structure included once

 Key maps to another map, to be described below. This 
 optional data structure can assist in automatic bus-mapping 
 which occurs during translation. Lists of transmit/recieve 
 command words may be provided by the user. These command
 words are particularly useful when multiple platform subsystems
 are not turned on during recording of the ch10 and only a small 
 subset of the possible messages are transmitted during the recording.
 In which case, mode code Transmit/Receive command words added to 
 supplemental_bus_map_command_words may be the only data available to 
 the auto bus map utility (aside from TMATS).  
 supplemental_bus_map_command_words are also helpful for mapping 1553 buses 
 recorded in the chapter 10 that are not included in
 translatable_message_definitions.


 
 _NOT_DEFINED_OPT_
 alias: bus_name

 Optional, if the user wishes the supplemental_bus_map_command_words 
 structure to be utilized then at least one bus_name must be given.

 User-defined key, which is the name of a bus given in the 'bus' name
 map of the 'msg_data' mapping. There may be multiple such mappings,
 the set of which represent a unique set of bus names, where each bus
 name that matches a bus name given in the 'bus' map above must represent
 the same actual bus. In other words, use consistent bus names between
 this data structure and the bus names used in 'translatable_message_definitions'.
 The user may also add bus names, and the corresponding data, which do
 not exist in 'translatable_message_definitions'. This is a way to 
 communicate bus to message mapping information to the automatic bus-mapping algorithm
 if data are intentionally not included in 'translatable_message_definitions'.

 Each user-defined bus name shall map to a sequence. Each entry in the
 sequence shall be a sequence of two integers, the transmit command 
 word followed by the receive command word of a message that may
 be transmitted on the bus. If the message contains a transmit or
 receive command word only, the other absent command word shall be 
 represented by a zero. The message is not required to be present
 in the data. A user may provide all transmit/receive command
 word combinations that are possible on the bus. 
 
 In general, the majority of command words relevant to a given bus
 are determined from the 'translatable_message_definitions', but
 some command words, such as non-data mode code type 1553 messages,
 are not included because of the lack of data. It can be very helpful
 to bus mapping to include those command words in this section.

 Additionally, there may be a bus which describes classified data. We do not
 include the message and elements description in the unclass data translation
 specification. If the command words are not classified, those values can
 be included here to assist in bus mappping.

 The best practice for this map is to include all command word pairs that
 can possibly be transmitted on the bus and only include message information
 in 'translatable_message_definitions' for which the user wishes to extract
 translated, i.e., engineering units, data.

)";


#endif  // DTS_1553_SCHEMA_H_ 