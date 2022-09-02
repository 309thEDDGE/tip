#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_1553f1_component.h"

class Ch101553F1ComponentTest : public ::testing::Test
{
   protected:
    Ch101553F1Component comp_;
    uint64_t wrd_cnt_;
    const uint8_t* data_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    MilStd1553F1DataHeaderCommWordFmt fmt_;
    const MilStd1553F1StatusWordFmt* statwrd1_;
    const MilStd1553F1StatusWordFmt* statwrd2_;
    const MilStd1553F1DataHeaderCommWordFmt* fmt_ptr_;

    Ch101553F1ComponentTest() : wrd_cnt_(0), data_ptr_(nullptr), status_(Ch10Status::NONE), 
        ctx_(0), comp_(&ctx_), statwrd1_(nullptr), statwrd2_(nullptr), fmt_ptr_(&fmt_)
    {
    }
};

TEST_F(Ch101553F1ComponentTest, GetWordCountFromDataHeaderRTtoRT)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 12;
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(fmt_.word_count1, wrd_cnt_);

    // word_count1 == 0 => actual word count == 32
    fmt_.word_count1 = 0;
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(32, wrd_cnt_);
}

TEST_F(Ch101553F1ComponentTest, GetWordCountFromDataHeaderNonRTtoRTModeCode)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 12;  // <= 15 ==> no data word
    fmt_.sub_addr1 = 0;
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(0, wrd_cnt_);

    // word_count1 == 15 and sub_addr1 == 0 => actual word count == 1
    // single data payload
    fmt_.word_count1 = 16;
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(1, wrd_cnt_);

    // Repeat two tests with sub_addr1 == 31, also mode code
    fmt_.sub_addr1 = 31;
    fmt_.word_count1 = 12;  // <= 15 ==> no data word
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(0, wrd_cnt_);

    fmt_.word_count1 = 16;
    wrd_cnt_ = comp_.GetWordCountFromDataHeader(&fmt_);
    EXPECT_EQ(1, wrd_cnt_);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadTooManyBytesRTtoRT)
{
    // Set message byte count to > 72, the max possible.
    fmt_.length = 80;
    fmt_.RR = 1;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadTooManyBytesNonRTtoRT)
{
    // Set message byte count to > 72, the max possible.
    fmt_.length = 80;
    fmt_.RR = 0;
    fmt_.tx1 = 0;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);

    // Transmit bit status should not impact this check.
    fmt_.tx1 = 1;
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::MILSTD1553_MSG_LENGTH);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadWordCountRTtoRT)
{
    // length = 25 words, 25-3 = 22 words for RTtoRT > 20
    fmt_.length = 50;
    fmt_.RR = 1;
    fmt_.word_count1 = 20;
    fmt_.sub_addr1 = 10;

    // Set data pointer to address of MilStd1553F1DataHeaderCommWordFmt
    // instance.
    data_ptr_ = (const uint8_t*)&fmt_;

    // Pointer to uint16_t to check value of internal payload_ptr_.
    const uint16_t* payload = (const uint16_t*)data_ptr_;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, fmt_.word_count1);
    EXPECT_EQ(comp_.calc_payload_word_count, 20);
    EXPECT_EQ(comp_.is_payload_incomplete, 0);

    // For RTtoRT, payload pointer ought to be incremented by
    // 3.
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 3);

    // length = 15 words, 15-3 = 12 words for RTtoRT < 32
    fmt_.length = 30;
    fmt_.word_count1 = 0;  // 32
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, 32);
    EXPECT_EQ(comp_.calc_payload_word_count, 12);
    EXPECT_EQ(comp_.is_payload_incomplete, 1);
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 3);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadWordCountNonRTtoRT)
{
    // length = 25 words, 25-2 = 23 words for RT to BC > 20
    fmt_.length = 50;
    fmt_.RR = 0;
    fmt_.tx1 = 1;  // RT to BC
    fmt_.word_count1 = 20;
    fmt_.sub_addr1 = 10;

    // Set data pointer to address of MilStd1553F1DataHeaderCommWordFmt
    // instance.
    data_ptr_ = (const uint8_t*)&fmt_;

    // Pointer to uint16_t to check value of internal payload_ptr_.
    const uint16_t* payload = (const uint16_t*)data_ptr_;

    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, fmt_.word_count1);
    EXPECT_EQ(comp_.calc_payload_word_count, 20);
    EXPECT_EQ(comp_.is_payload_incomplete, 0);

    // For RT to BC, expect payload pointer to be incremented
    // by two.
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 2);

    // length = 15 words, 15-2 = 13 words for RT to BC < 32
    fmt_.length = 30;
    fmt_.word_count1 = 0;  // 32
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, 32);
    EXPECT_EQ(comp_.calc_payload_word_count, 13);
    EXPECT_EQ(comp_.is_payload_incomplete, 1);
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 2);

    // Mode code without payload. Note: If no payload, we
    // don't care if the payload pointer is incremented because
    // it's not used.
    fmt_.sub_addr1 = 0;
    fmt_.word_count1 = 5;
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, 0);
    EXPECT_EQ(comp_.calc_payload_word_count, 0);
    EXPECT_EQ(comp_.is_payload_incomplete, 0);

    // Mode code with payload, RT to BC, skip mode command word followed
    // by status word. Payload ought to be incremented by two.
    fmt_.sub_addr1 = 0;
    fmt_.word_count1 = 20;  // > 15
    fmt_.tx1 = 1;           // RT to BC
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, 1);
    EXPECT_EQ(comp_.calc_payload_word_count, 1);
    EXPECT_EQ(comp_.is_payload_incomplete, 0);
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 2);

    // Mode code with payload, BC to RT, skip mode command word only.
    // Payload ought to be incremented by one.
    fmt_.sub_addr1 = 0;
    fmt_.word_count1 = 20;  // > 15
    fmt_.tx1 = 0;           // BC to RT
    status_ = comp_.ParsePayload(data_ptr_, &fmt_);
    EXPECT_EQ(status_, Ch10Status::OK);
    EXPECT_EQ(comp_.expected_payload_word_count, 1);
    EXPECT_EQ(comp_.calc_payload_word_count, 1);
    EXPECT_EQ(comp_.is_payload_incomplete, 0);
    EXPECT_EQ(*comp_.payload_ptr_ptr, payload + 1);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoRTAllWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 21;
    fmt_.sub_addr1 = 3;

    // + 4 (rxcomm, txcomm, status . . . . status)
    fmt_.length = (fmt_.word_count1 + 4) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 2), statwrd1_);
    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 3 + fmt_.word_count1), statwrd2_);
}
 
TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoRTFewerWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 21;
    fmt_.sub_addr1 = 3;

    // + 4 (rxcomm, txcomm, status . . . . status)
    // subtract 3 to have fewer words than required
    fmt_.length = (fmt_.word_count1 + 4 - 3) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 2), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoRTInsufficientWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 21;
    fmt_.sub_addr1 = 3;

    // + 4 (rxcomm, txcomm, status . . . . status)
    // only two words present, sufficient for only rxcomm, txcomm
    fmt_.length = 2 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsBCtoRTAllWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 11;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 3;

    // + 2 (rxcomm . . . status)
    fmt_.length = (fmt_.word_count1 + 2) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 1 + fmt_.word_count1), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsBCtoRTFewerWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 11;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 3;

    // + 2 (rxcomm . . . status)
    // - 5 to have fewer words
    fmt_.length = (fmt_.word_count1 + 2 - 5) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoBCAllWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 11;
    fmt_.tx1 = 1;
    fmt_.sub_addr1 = 3;

    // + 2 (txcomm, status . . . )
    fmt_.length = (fmt_.word_count1 + 2) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 1), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoBCFewerWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 11;
    fmt_.tx1 = 1;
    fmt_.sub_addr1 = 3;

    // + 2 (txcomm, status . . . )
    // - 3 to have fewer words
    fmt_.length = (fmt_.word_count1 + 2 - 3) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 1), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWOutWordAllWords)
{
    fmt_.RR = 0;

    // word count has different meaning for mode codes. If count > 15, then 
    // a single data word is implied.
    fmt_.word_count1 = 14;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0; 

    // (modecomm, status)
    fmt_.length = 2 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 1), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWOutWordFewerWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 14;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0; 

    // (modecomm, status)
    fmt_.length = 1 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsBroadcastModeCode)
{
    fmt_.RR = 0;

    // word count has different meaning for mode codes. If count > 15, then 
    // a single data word is implied.
    fmt_.word_count1 = 14;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0b11111; 

    // (modecomm)
    fmt_.length = 1 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWWordTxAllWords)
{
    fmt_.RR = 0;

    // word count has different meaning for mode codes. If count > 15, then 
    // a single data word is implied.
    fmt_.word_count1 = 17;
    fmt_.tx1 = 1;
    fmt_.sub_addr1 = 0; 

    // (modecomm, status, data)
    fmt_.length = 3 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 1), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWWordTxFewerWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 17;
    fmt_.tx1 = 1;
    fmt_.sub_addr1 = 0; 

    // (modecomm, status, data)
    fmt_.length = (3 - 2) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWWordRxAllWords)
{
    fmt_.RR = 0;

    // word count has different meaning for mode codes. If count > 15, then 
    // a single data word is implied.
    fmt_.word_count1 = 17;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0; 

    // (modecomm, data, status)
    fmt_.length = 3 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 2), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWWordRxFewerWords)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 17;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0; 

    // (modecomm, data, status)
    fmt_.length = (3 - 1) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsModeCodeWWordBroadcast)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 17;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0b11111; 

    // (modecomm, data)
    fmt_.length = 2 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsBCtoRTBroadcast)
{
    fmt_.RR = 0;
    fmt_.word_count1 = 5;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0b11111; 

    // (rxcomm, data, ....)
    fmt_.length = (fmt_.word_count1 + 1) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoRTBroadcastAllWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 5;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0b11111; 
    fmt_.tx2 = 1;
    // (rxcomm, txcomm, status, data, ....)
    fmt_.length = (fmt_.word_count1 + 3) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 2), statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParseStatusWordsRTtoRTBroadcastFewerWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 5;
    fmt_.tx1 = 0;
    fmt_.sub_addr1 = 0b11111; 

    // (rxcomm, txcomm, status, data, ....)
    fmt_.length = 2 * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();

    comp_.ParseStatusWords(payload, fmt_ptr_, statwrd1_, statwrd2_);

    EXPECT_EQ(nullptr, statwrd1_);
    EXPECT_EQ(nullptr, statwrd2_);
}

TEST_F(Ch101553F1ComponentTest, ParsePayloadCallsParseStatusWords)
{
    fmt_.RR = 1;
    fmt_.word_count1 = 5;
    fmt_.tx1 = 0;

    fmt_.length = (fmt_.word_count1 + 4) * 2;  // * 2 --> bytes
    std::vector<uint16_t> payload_vec(fmt_.length / 2, 0);
    const uint16_t* payload = payload_vec.data();
    const uint8_t* data = reinterpret_cast<const uint8_t*>(payload);

    status_ = comp_.ParsePayload(data, fmt_ptr_);
    EXPECT_EQ(Ch10Status::OK, status_);

    comp_.GetStatusWordPointers(statwrd1_, statwrd2_);

    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 2), statwrd1_);
    EXPECT_EQ(reinterpret_cast<const MilStd1553F1StatusWordFmt*>(payload + 3 + fmt_.word_count1), statwrd2_);
}