#ifndef PARSER_PATHS_H_
#define PARSER_PATHS_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "managed_path.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"

class ParserPaths
{
    private:
        ManagedPath ch10_input_path_;
        ManagedPath output_dir_;

        // Map of packet type to enable state
        std::map<Ch10PacketType, bool> pkt_type_enabled_map_;

        // Map of packet type to parsed output data dir path
        std::map<Ch10PacketType, ManagedPath> pkt_type_output_dir_map_;

        // Raw TMATS as ASCII file
        ManagedPath tmats_output_path_;

        // Time Data packet output file path
        ManagedPath tdp_output_path_;

        // Vector of maps of Ch10PacketType to output file path in 
        // which the vector index is the same as the worker ID.
        std::vector<std::map<Ch10PacketType, ManagedPath>> worker_path_vec_;

    public:
        ParserPaths();
        ~ParserPaths() {}
        virtual const ManagedPath& GetCh10Path() const { return ch10_input_path_; }
        virtual const ManagedPath& GetOutputDir() const { return output_dir_; }
        virtual const std::map<Ch10PacketType, bool>& GetCh10PacketTypeEnabledMap() const
        { return pkt_type_enabled_map_; }
        virtual const ManagedPath& GetTMATSOutputPath() const { return tmats_output_path_; }
        virtual const ManagedPath& GetTDPOutputPath() const { return tdp_output_path_; }
        virtual const std::map<Ch10PacketType, ManagedPath>& GetCh10PacketTypeOutputDirMap() const
        { return pkt_type_output_dir_map_; }
        virtual const std::vector<std::map<Ch10PacketType, ManagedPath>>& GetWorkerPathVec() const
        { return worker_path_vec_; }

        /*
        Create paths relevant to parsed ch10 packets and metadata
        files. Populates:

        - ch10_input_path_;
        - output_dir_;
        - pkt_type_enabled_map_
        - tmats_output_path_
        - tdp_output_path_
        - pkt_type_output_dir_map_

        Args:
            ch10_input_path     --> Full path to input ch10 file
            output_dir          --> Output directory
            packet_enabled_map	--> Map of Ch10PacketType to boolean. True = enabled,
                                    False = disabled
            total_worker_count	--> Count of workers expected to be
                                    created to parse according to
                                    configuration settings

        Return:
            False if an error occurs, true otherwise.
        */
        virtual bool CreateOutputPaths(const ManagedPath& ch10_input_path,
            const ManagedPath& output_dir, const std::map<Ch10PacketType, bool>& packet_enabled_map,
            uint16_t total_worker_count);

        // External call to similar function defined below.
        bool RemoveCh10PacketOutputDirs(const std::set<Ch10PacketType>& parsed_packet_types) const;



        ///////////////////////////////////////////////////////////////////
        //                      Internal functions
        ///////////////////////////////////////////////////////////////////


        /*
        Create and verify output directories for enabled ch10 packet types.

        Args:
            output_dir			--> ManagedPath object giving the output directory into
                                    which packet type-specific dirs will be created
            base_file_name		--> ManagedPath object with the base file name on which
                                    to build the output directory name. May be a complete
                                    path to a file, in which case the parent path will be
                                    stripped and only the file name used, or a file name
                                    only.
            packet_enabled_map	--> Map of Ch10PacketType to boolean. True = enabled,
                                    False = disabled
            pkt_type_output_dir_map --> Map of Ch10PacketType to specific output directory,
                                        the product of this function
            create_dir				--> True if the directory ought to be created, false
                                        otherwise

        Return:
            True if successful and all output directories were created; false otherwise.
        */
        bool CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
                                        const ManagedPath& base_file_name,
                                        const std::map<Ch10PacketType, bool>& packet_enabled_map,
                                        std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, 
                                        bool create_dir);



        /*
        Generate a vector of maps of Ch10PacketType to ManagedPath. The path object
        is a file path to which data for the given Ch10PacketType
        ought to be written by the worker associated with the index of the vector
        from which the map was retrieved.

        Args:
            total_worker_count			--> Count of workers expected to be
                                            created to parse according to
                                            configuration settings
            pkt_type_output_dir_map		--> Map of Ch10PacketType to base output
                                            directory to which files associated
                                            with the packet type ought to be written.
                                            This is the pkt_type_output_dir_map from
                                            CreateCh10PacketOutputDirs.
            output_vec_mapped_paths		--> Vector of maps in which the index in
                                            the vector is the same as the worker which
                                            ought to utilize the mapped output file
                                            paths.
            file_extension				--> String not including the '.'.
                                            Ex: file_extension = 'txt'
        */
        void CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
                                            const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
                                            std::vector<std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
                                            std::string file_extension);



        /*
        Remove output dirs which represent packets types that were not parsed and
        which do not contain any output data.

        Args:
            output_dir_map		--> Map of Ch10PacketType to created output dirs
            parsed_packet_types --> Set of Ch10PacketType to be filled

        Return:
            True if all dirs that exist in output_dir_map which do not associate
            with a type that is in parsed_packet_types are deleted. Only dirs that
            exist will attempt to be deleted and therefore a false return value is 
            only possible for dirs which exist and can't be deleted. 
        */
        bool RemoveCh10PacketOutputDirs(const std::map<Ch10PacketType, ManagedPath>& output_dir_map,
            const std::set<Ch10PacketType>& parsed_packet_types) const;

};

#endif  // PARSER_PATHS_H_