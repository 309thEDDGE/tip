#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>
#include <parquet/arrow/schema.h>

#include <unordered_map>
#include <fstream>
#include <memory>
#include <vector>
#include "managed_path.h"

class ParquetVideoExtraction
{
   private:
    // Arrow variables.
    arrow::Status st_;
    arrow::MemoryPool* pool_ = arrow::default_memory_pool();
    std::shared_ptr<arrow::io::ReadableFile> arrow_file_;
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader_;
    std::shared_ptr<arrow::Schema> schema_;

    int row_group_count_;
    int channel_id_index_;
    int data_column_index_;
    ManagedPath parquet_path_;
    ManagedPath output_path_;

    std::unordered_map<int32_t, std::ofstream*> video_streams_;
    std::unordered_map<int32_t, std::vector<uint16_t>> write_buffer_;
    std::unordered_map<int32_t, int> buffer_lengths_;

    bool OpenParquetFile(ManagedPath file_path);
    bool ExtractFileTS();
    void WriteRowGroup(const arrow::NumericArray<arrow::Int32Type>& data_arr,
                       const arrow::NumericArray<arrow::Int32Type>& channel_ids);

   public:
    ParquetVideoExtraction() {}

    ~ParquetVideoExtraction()
    {
        for (std::unordered_map<int32_t, std::ofstream*>::iterator it = video_streams_.begin();
             it != video_streams_.end();
             ++it)
        {
            // Close the file
            it->second->close();

            // Delete fstream memory allocation
            delete it->second;
        }
    };

    /*
		Pass the path to the folder with parquet video files

		Returns: nonzero -> If video_path does not exist OR 
							the output TS path could not be created
				 0  -> Other wise	
	*/
    int Initialize(ManagedPath video_path, ManagedPath output_dir);

    /*
		Extract transport stream data from 
		all parquet files in the video_path

		Returns: nonzero -> if success
			     zero    -> if failure
	*/
    int ExtractTS();
};