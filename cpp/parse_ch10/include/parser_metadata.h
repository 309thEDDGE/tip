#ifndef PARSER_METADATA_H_
#define PARSER_METADATA_H_

// Order of includes matters here. See the comment in 
// ch10_context.h.

#include <string>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include "ch10_context.h"
#include "parquet_tdpf1.h"
#include "parser_paths.h"
#include "iterable_tools.h"
#include "parser_config_params.h"
#include "tmats_data.h"
#include "managed_path.h"
#include "parse_text.h"
#include "provenance_data.h"
#include "tip_md_document.h"
#include "sha256_tools.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"
#include "ch10_packet_type_specific_metadata.h"

class ParserMetadataFunctions
{

    public:

        ParserMetadataFunctions() {}
        ~ParserMetadataFunctions() {}
        

        /*
        Add provenance data to metadata output.

        Args:
            md						--> TIPMDDocument object to which metadata 
                                        will be added
            input_ch10_file_path	--> Path to ch10 file which was parsed
            packet_type_label		--> See ch10_packet_type.h
            prov_data				--> Object of ProvenanceData that contains
                                        pre-filled provenance data
        */
        virtual void RecordProvenanceData(TIPMDDocument* md, const ManagedPath& input_ch10_file_path,
        	const std::string& packet_type_label, const ProvenanceData& prov_data);



        /*
        Add user configuration data to metadata output.

        Args:
            config_category			--> MDCategoryMap to which User configuration
                                        metadata will be added  
            user_config				--> ParserConfigParams object which has
                                        been configured with data from the
                                        user config file
        */
        virtual void RecordUserConfigData(std::shared_ptr<MDCategoryMap> config_category, 
        	const ParserConfigParams& user_config);



        /*
        Collect raw TMATS strings into a single string and write to disk.
        Use TMATSParser to parse and associate TMATS metadata into maps
        in preparation for recording to yaml metadata file.

        Args:
        tmats_vec					--> Vector of strings into which workers will push
                                        TMATS matter
        tmats_file_path				--> Complete path including the output file name
                                        to which TMATS matter is recorded
        tmats_data					--> TMATSData object
        parsed_pkt_types			--> Set of Ch10PacketType that contains only the
                                        present and parsed types

        */
        void ProcessTMATS(const std::vector<std::string>& tmats_vec,
                        const ManagedPath& tmats_file_path,
                        TMATSData& tmats_data, 
                        const std::set<Ch10PacketType>& parsed_pkt_types);



        /*
        Filter TMATS data based on packet type and add filtered
        maps to metadata.

        Args:

            tmats_data				--> TMATSData object
            md						--> TIPMDDocument object to which metadata 
                                        will be added
            pkt_type				--> Ch10PacketType

        Return:
            False if errors occur; true otherwise
        */
        virtual bool ProcessTMATSForType(const TMATSData* tmats_data, TIPMDDocument* md,
            Ch10PacketType pkt_type);

        

        /*
        Write string to file.

        Args:
            outpath		--> Output file path
            outdata		--> String of data to write to file

        Return:
            False if parent directory does not exist; true otherwise.
        */
        virtual bool WriteStringToFile(const ManagedPath& outpath, const std::string& outdata);



        /*
        Record time data ("Time Data, Format 1") which is stored in Ch10Context
        instances to Parquet format.

        Args:
            ctx_vec			--> Vector of Ch10Contexts from which time data
                                will be gathered
            pqtdp			--> Instance of ParquetTDPF1 which will be used
                                to write the time data to Parquet
            file_path		--> Complete output file path

        Return:
            False if any steps fail; true otherwise.
        */
        bool WriteTDPData(const std::vector<const Ch10Context*>& ctx_vec,
            ParquetTDPF1* pqtdp, const ManagedPath& file_path);



        /*
        Log the ch10_packet_type_map_. This is a convenience function to clean up
        the clutter that this code introduces.

        Args:
            pkt_type_config_map	--> Input map of Ch10PacketType to bool that
                                    represents the enable state of ch10 packet types
        */
        void LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map);



        /*
        Assemble the set of all parsed packet types from the workers and
        log the information.

        Args:
            ctx_vec			        --> Vector of Ch10Contexts from which data
                                        will be gathered
            parsed_packet_types 	--> Set of Ch10PacketType to be filled
        */
        void AssembleParsedPacketTypesSet(const std::vector<const Ch10Context*>& ctx_vec,
            std::set<Ch10PacketType>& parsed_packet_types);


        /*
        Concatenate TMATS matter into single string. In the majority of cases,
        tmats data will be extracted by one worker in one thread unless the 
        tmats matter exceeds the maximum tmats packet size, which is ~134 MB.
        It is unlikely that this will ever be the case. However, if multiple
        workers do observe and parse tmats packets, we loop over all of the 
        Ch10Context instances, one for each worker, and concatenate the result.

        Note this is specific to ch10 Computer Generated Data, Format 1 data.

        Args:
            ctx_vec         --> Vector of Ch10Contexts from which data
                                will be gathered
            tmats_vec       --> Output vector of all tmats matter
        */
        void GatherTMATSData(const std::vector<const Ch10Context*>& ctx_vec,
            std::vector<std::string>& tmats_vec);



        virtual bool RecordCh10PktTypeSpecificMetadata(Ch10PacketType pkt_type, 
            const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
            const TMATSData* tmats, Ch10PacketTypeSpecificMetadata* spec_md);

        virtual bool RecordCh10PktTypeSpecificMetadata(Ch10PacketType pkt_type, 
            const std::vector<const Ch10Context*>& context_vec, MDCategoryMap* runtime_metadata, 
            const TMATSData* tmats);

};

class ParserMetadata
{
    private:

        // Count of first n bytes of the ch10 to hash as part of provenance
        // data collection. The I/O burden is too high to hash the entire
        // ch10, especially when parsing over the network.
        const size_t ch10_hash_byte_count_; 

        ProvenanceData prov_data_;

        ParserConfigParams config_;

        ParserPaths parser_paths_;

    public:
        ParserMetadata();


        /*
        Obtain basic provenance data about the input ch10 file. It is important
        to run this function early in case failures occur so that problems
        can be remedied prior to parsing, otherwise the lengthy task of 
        parsing may be wasted if provenance data can't be collected and
        stored with the output after the fact.

        Args:
            ch10_path       --> Ch10 input file path
            config          --> Instance of populated ParserConfigParams
            parser_paths    --> Instance of configured ParserPaths object,
                                i.e., one for which ::CreateOutputPaths has
                                been called

        Return:
            False if GetProvenanceData returns false; true otherwise.
        */
        virtual bool Initialize(const ManagedPath& ch10_path, const ParserConfigParams& config,
            const ParserPaths& parser_paths);


        /*
        Function called by ParseManager to record metadata.

        Args:
            md_filename         --> File name only of all output metadata
                                    files for each parsed ch10 packet type
            context_vec         --> Vector of Ch10Context instances associatd with
                                    each parser worker thread. Input here to obtain
                                    additional metadata from the contexts. 
            
        Return:
            True if no errors occur; false otherwise.
        */
        virtual bool RecordMetadata(ManagedPath md_filename, 
            const std::vector<const Ch10Context*>& context_vec);

        

        /*
        Record all metadata for a given packet type.

         Args:
            md_filename         --> File name only of all output metadata
                                    files for each parsed ch10 packet type
            pkt_type            --> Ch10PacketType of current metadata to be 
                                    generated
            parser_paths        --> Instance of configured ParserPaths object,
                                    i.e., one for which ::CreateOutputPaths has
                                    been called
            config              --> Instance of populated ParserConfigParams
            prov_data           --> Instance of populated ProvenanceData
            tmats_data          --> Configured instance of TMATSData object, see
                                    ParserMetadataFunctions::ProcessTMATS
            context_vec         --> Vector of Ch10Context instances associatd with
                                    each parser worker thread. Input here to obtain
                                    additional metadata from the contexts. 
            tip_md              --> Instance of TIPMDDocument
            md_funcs            --> Instance of ParserMetadataFunctions
            
        Return:
            True if no errors occur; false otherwise.
        */
        bool RecordMetadataForPktType(const ManagedPath& md_filename, Ch10PacketType pkt_type,
            const ParserPaths* parser_paths, const ParserConfigParams& config, 
            const ProvenanceData& prov_data, const TMATSData* tmats_data, 
            const std::vector<const Ch10Context*>& context_vec,
            TIPMDDocument* tip_md, ParserMetadataFunctions* md_funcs);
};

#endif  // PARSER_METADATA_H_