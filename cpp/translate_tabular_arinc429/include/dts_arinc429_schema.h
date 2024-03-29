#ifndef DTS_ARINC429_SCHEMA_H_
#define DTS_ARINC429_SCHEMA_H_

#include <string>

// Legend:
// - (O)  = optional map with specific key (key is required, mapped value is not)
// - (R)  = required map with specific key
// - (WR) = wildcard placeholder for required user-defined input
// - (WO) = wildcard placeholder for optional user-defined input

// Wildcards represent user-defined strings and are used to
// give names to 429 messages or elements. Within a structure,
// a wildcard may be repeated many times to define all necessary
// message and/or message elements.

// For more information about wildcards, see tip_dts1553_schema.yaml.

const std::string dts_429_schema = R"(
---
translatable_word_definitions:              # (R)
  _NOT_DEFINED_:                            # (WR), alias: wrd_name = 429 message name
    wrd_data:                               # (R)
      label: INT                            # (R)
      bus: STR                              # (R)
      sdi: INT                              # (R)
      rate: OPTFLT                            # (O)
      desc: OPTSTR                            # (O)
      lru_name: OPTSTR                        # (O)
    elem:                                   # (R)
      _NOT_DEFINED_:                        # (WR), alias: elem_name = bit/multiple bit element
        schema: STR-->{UNSIGNEDBITS,SIGNEDBITS,ASCII,BCD,SIGNMAG}   # (R)
        msbval: FLT                         # (R)
        lsb: INT                            # (R)
        bitcnt: INT                         # (R)
        desc: OPTSTR                           # (O)
        uom: OPTSTR                            # (O)
        class: INT                          # (R)
supplemental_bus_map_labels:                # (R)
  _NOT_DEFINED_OPT_:                        # (WO), alias: bus_name = name of a bus from which 429 data were recorded
    - [INT]                                 # (R)
...
)";

const std::string dts_429_word_explanation = R"(
 #### ARINC429 Word Bit Definition ####

 Transfer Order | ARINC 429 32-bit Word Format
                |
        8       | 1... Label
        7       | 2...
        6       | 3...
        5       | 4...
        4       | 5...
        3       | 6...
        2       | 7...
        1       | 8...
        9       | 9...SDI
       10       | 10..
       11       | 11..DATA
       12       | 12..
       13       | 13..
       14       | 14..
       15       | 15..
       16       | 16..
       17       | 17..
       18       | 18..
       19       | 19..
       20       | 20..
       21       | 21..
       22       | 22..
       23       | 23..
       24       | 24..
       25       | 25..
       26       | 26..
       27       | 27..
       28       | 28..
       29       | 29..
       30       | 30..SSM
       31       | 31..
       32       | 32..PARITY
)";

const std::string dts_429_parameter_defs = R"(
#### Parameter Definitions ####

 Note: Boolean values shall be denoted by strings. True shall
 be indicated by True or true and false shall be indicated
 by False or false.


 translatable_word_definitions

 Required, structure included exactly once in 429
 data translation specification (DTS429)

 Required top level key which indicates that all
 structure below it defines an ARINC429 word to be
 translated.



 _NOT_DEFINED_
 alias: wrd_name

 Required, structure included at least once in the
 translatable_word_definitions structure

 Required wildcard to be replaced by the user with a 429
 word name. The data structure to which this key maps is
 relevant to this 429 word alone. The lower map with keys wrd_data
 and elem are immediately below the
 message name and are required. There may be as few as a single
 msg_name data structure or hundreds, depending on the content
 of the ch10.



 wrd_data

 Required, structure included at least once in wrd_name structure

 Contains data relevant to identifying an ARINC 429. This key
 maps to a lower map with the keys: label, bus, rate, sdi, ssm,
 and desc.



 label

 Required, structure included exactly once in the current map

 Key maps to a single octal value that is eqeal to the represents
 the label of the ARINC 429 word.



 bus

 Required, structure included exactly once in the current map

 String which provides the name of the bus. This string will provide
 a name in the format as would be used by TMATS' ARINC 429 subchannel
 name (R-x\ANM-n-m). All alphabetical characters should be upper-case.

 Example: EEC1 EEC2B



 sdi

 Required, structure included exactly once in the current map

 Only used when sdiusd is set to true. The value of 'sdi' is a
 decimal integer representing the value of the two sdi bits in the
 429 message.

 Example: To define that the bits [10,9] are of the value [1,0]
 you would set 'sdi' = 2. To define bits [10,9] as having the
 value [1,1] you would set 'sdi' = 3. There are cases where the
 sdi bits should be ignored. If the SDI bits are irrelevant and
 should be ignored, use the value -1 for sdi.



 rate

 Optional, structure included exactly once in the current map

 Key maps to a float value which gives the message transmission
 rate. The rate matches the defintion of the IPDH's bus speed
 field, as found in IRIG106 Chapter 11.
 Low-speed ARINC-429 bus (12.5 kHz)
 High-speed ARINC-429 bus (100 kHz)



 desc

 Required, structure included exactly once in the current map

 Key maps to a string description of the 429 word.



 lru_name

 Optional, structure included exactly once in the current map

 Key maps to a string identifying the name of the transmitting
 lru.



 elem

 Required, structure included exactly once in the current map

 Key is one of two keys at the current level of the data structure.
 The other keys in the map is wrd_data (see above). The key maps to
 a map which has keys for the bit-level features of a parameter in
  a 429 payload. The bit-level elements are used to define position
 and basic features of a parameter.



 _NOT_DEFINED_
 alias: elem_name

 Required wildcard key to be replaced by the user-defined name of the
 element to be translated from the raw ARINC 429 word. There may
 be 1 or many of these data structures, where each structure
 defines exactly one element to be interpreted or translated
 out of the payload for the given message.



 schema

 Required, structure included exactly once in the current map

 Key maps to a string which identifies a specific algorithm
 for intepreting the bits defined as part of the current
 element. All string values below are to be used only with the
 'elem_name' map unless otherwise noted. The optional values are:

   // 8-bit ASCII
	ASCII

	// Two or more consecutive bits to be interpreted as a twos-complement signed integer.
	// SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.

	SIGNEDBITS

	// Single-bit boolean or multiple bits to be interpreted as unsigned integer.
	// SIGNEDBITS or UNSIGNEDBITS schema should be used when is_bitlevel_ = true.

	UNSIGNEDBITS




 msbval

 Required, structure included exactly once in the current map

 Key maps to a floating point value giving the (potentially scaled)
 value of the most significant bit (MSB) in the value to be interpreted
 by the word(s). An example is an interpreted value which provides
 a high-precision datum with a value between 0 and 1, indicates a
 16 bit unsigned integer with an msbval of 0.5. An msbval of 0.5
 implies a scale factor of 0.5 / 2^15 = 1.52588e-5, following the
 generic formula:
         msbval / (2 ^ (msb_position - 1)) = scale_factor
 If this word were to have all 16 bits high (word_value = 2^16 - 1)
 then the intended value is word_value * 1.5288e-5 ~= 1.0. The most
 significant bit of signed values is one bit lower than the highest
 bit which is designated as the sign bit. Using a signed 16 bit word
 as an example, scale_factor = msbval / 2^14.



 lsb

 Required, structure included exactly once in the current map

 Key maps to an integer which identifies the least significant bit
 within the 429 word from which the number of bits (defined with
 bitcnt) that make up the parameter field shall be extracted. This
 follows a 1-indexed convention with bit 1 being the start of the
 label as defined in the ARINC 429 32-bit Word Format.



 bitcnt

 Required, structure included exactly once in the current map

 Key maps to a integer which gives the explicit count of bits
 to be extracted from the the word at offset 'lsb'. For example
 a parameter occupying the entier data field of the 429 word
 would be defined with 'lsb' = 11 and 'bitcnt' = 19.



 desc

 Optional, structure included exactly once in the current map

 Key maps to a string value describing the value to be interpreted
 or extracted from the raw payload. Ex: "height above ground".
 If unknown or not relevant, a placeholder ought to be used.



 uom

 Optional, structure included exactly once in the current map

 Key maps to a string value indicating the unit of the translated
 element. This string is not currently used at translation time.
 If unknown or not relevant, a placeholder ought to be used.



 class

 Required, structure included exactly once in the current map

 Key maps to an integer which indicates the security classification of
 the element. This value is not currently utilized. The default value
 ought to be zero.

)";


const std::string dts_429_suppl_labels = R"(
 supplemental_bus_map_labels

 Required, structure included once

 Key maps to another map, to be described below. This
 optional data structure can assist in automatic bus-mapping
 which occurs during translation. Lists of ARINC 429 labels
 may be provided by the user. These ARINC 429 labels
 are particularly useful when multiple platform subsystems
 are not turned on during recording of the ch10 and only a small
 subset of the possible messages are transmitted during the recording.
 In which case, additional ARINC 429 labels added to
 supplemental_bus_map_labels may be the only data available to
 the auto bus map utility (aside from TMATS).
 supplemental_bus_map_labels are also helpful for mapping 429 buses
 recorded in the chapter 10 that are not included in
 translatable_word_definitions.



 _NOT_DEFINED_OPT_
 alias: bus_name

 Optional, if the user wishes the supplemental_bus_map_labels
 structure to be utilized then at least one bus_name must be given.

 User-defined key, which is the name of a bus given in the 'bus' name
 map of the 'wrd_data' mapping. There may be multiple such mappings,
 the set of which represent a unique set of bus names, where each bus
 name that matches a bus name given in the 'bus' map above must represent
 the same actual bus. In other words, use consistent bus names between
 this data structure and the bus names used in 'translatable_word_definitions'.
 The user may also add bus names, and the corresponding data, which do
 not exist in 'translatable_word_definitions'. This is a way to
 communicate bus to message mapping information to the automatic bus-mapping
 algorithm if data are intentionally not included in 'translatable_word_definitions'.

 Each user-defined bus name shall map to a sequence. Each entry in the
 sequence shall be a single integer which is the decimal representation
 of the label of an ARINC 429 word that may
 be transmitted on the bus. The 429 label is not required to be present
 in the data. A user may provide all ARINC 429 labels that are possible
 on the bus.

 In general, the majority of 429 labels relevant to a given bus
 are determined from the 'translatable_word_definitions', but
 some 429 labels are not included because of the lack of data.
 It can be very helpful to bus mapping to include those labels
  in this section.

 Additionally, there may be a bus which describes classified data. We do not
 include the word and elements description in the unclass data translation
 specification. If the 429 labels are not classified, those values can
 be included here to assist in bus mappping.

 The best practice for this map is to include all ARINC 429 labels that
 can possibly be transmitted on the bus and only include word information
 in 'translatable_word_definitions' for which the user wishes to extract
 translated, i.e., engineering units, data.
)";
#endif  // DTS_ARINC429_SCHEMA_H_