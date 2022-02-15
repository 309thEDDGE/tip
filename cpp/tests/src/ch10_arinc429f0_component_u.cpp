#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_arinc429f0_component.h"
#include "spdlog/spdlog.h"
#include <fstream>

class Ch10429F0ComponentTest : public ::testing::Test
{
   protected:
    Ch10429F0Component comp_;
    uint64_t wrd_cnt_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    ARINC429F0MsgFmt fmt_;
    ARINC429F0MsgFmt fmt0_;
    ARINC429F0MsgFmt fmt1_;
    ARINC429F0MsgFmt fmt2_;

    Ch10429F0ComponentTest() : wrd_cnt_(0), data_ptr_(nullptr), status_(Ch10Status::NONE), ctx_(0), comp_(&ctx_)
    {
    }

   public:
   // assign values to associated fields of the ARINC429F0MsgFmt passed in
    void BuildCstmMsg(ARINC429F0MsgFmt &fmt, uint32_t gap, uint32_t bus_spd, uint32_t parity_err, uint32_t fmt_err, uint32_t bus,
                 uint32_t label, uint32_t SDI, uint32_t data, uint32_t SSM, uint32_t parity)
    {
        fmt.gap = gap;
        fmt.BS = bus_spd;
        fmt.PE = parity_err;
        fmt.FE = fmt_err;
        fmt.bus = bus_spd;
        fmt.label = label;
        fmt.SDI = SDI;
        fmt.data = data;
        fmt.SSM = SSM;
        fmt.parity = parity;
    }

    // Build ARINC429F0MsgFmt such that there is a
    // parity error in fmt0_, format error in fmt1_,
    // and no errors or problems in fmt3_
    void BuildMsgs()
    {
        // parity error in fmt0_
        BuildPE(fmt0_);

        // format error  in fmt1_
        BuildFE(fmt1_);

        // no problem  in fmt2_
        BuildNoError(fmt2_);
    }

    // parity error in fmt passed in
    void BuildPE(ARINC429F0MsgFmt &fmt)
    {
        BuildCstmMsg(fmt, 21, 1, 1, 0, 0, 139, 1, 150185, 0, 1);
    }

    // format error in fmt passed in
    void BuildFE(ARINC429F0MsgFmt &fmt)
    {
        BuildCstmMsg(fmt, 21, 1, 0, 1, 0, 139, 1, 150185, 0, 1);
    }

    // no problem in fmt passed in
    void BuildNoError(ARINC429F0MsgFmt &fmt)
    {
        BuildCstmMsg(fmt, 21, 1, 0, 0, 0, 139, 1, 150185, 0, 1);
    }

    // gap time too large in fmt passed in
    void BuildTimeGapError(ARINC429F0MsgFmt &fmt)
    {
        BuildCstmMsg(fmt, 1000011, 1, 0, 0, 0, 139, 1, 150185, 0, 1);
    }
};

/*
 * ParseMessages() Tests
 */

TEST_F(Ch10429F0ComponentTest, ParseMessagesTimeGapTooLarge)
{
    // build too large of a time gap.
    BuildTimeGapError(fmt_);
    data_ptr_ = (const uint8_t*)&fmt_;
    // std::cerr << "[          ] fmt_.gap = " << fmt_.gap << std::endl;

    // Test time gap too large
    status_ = comp_.ParseMessages(1, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ARINC429F0_GAP_TIME_ERROR);
}

TEST_F(Ch10429F0ComponentTest, ParseMessagesParityError)
{
    // build parity error
    BuildPE(fmt_);
    data_ptr_ = (const uint8_t*)&fmt_;

    // Test parity error exists
    status_ = comp_.ParseMessages(1, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ARINC429F0_PARITY_ERROR);
}

TEST_F(Ch10429F0ComponentTest, ParseMessagesFormatError)
{
    // build format error
    BuildFE(fmt_);
    data_ptr_ = (const uint8_t*)&fmt_;

    // Test format error exists
    status_ = comp_.ParseMessages(1, data_ptr_);
    EXPECT_EQ(status_, Ch10Status::ARINC429F0_FORMAT_ERROR);
}

// TEST_F(Ch10429F0ComponentTest, ParseMessagesStatusOK)
// {

//     fs::path temp_path("test_dir");
//     fs::create_directory(temp_path);
//     ManagedPath p("test_dir/test.parquet");
//     std::map<Ch10PacketType, ManagedPath> enabled_paths;
//     enabled_paths[Ch10PacketType::ARINC429_F0] = p;

//     Ch10Context ctx_(0);
//     ctx_.InitializeFileWriters(enabled_paths);  // may need other intitializations to correctly have environment for ParseMessages()
//     Ch10429F0Component comp_(&ctx_);

//     // build format error
//     BuildNoError(fmt_);
//     data_ptr_ = (const uint8_t*)&fmt_;

//     // Test format error exists
//     status_ = comp_.ParseMessages(1, data_ptr_);
//     fs::remove_all(temp_path);
//     EXPECT_EQ(status_, Ch10Status::OK);
// }
