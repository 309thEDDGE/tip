
#ifndef CH10_STATUS_H_
#define CH10_STATUS_H_

#include <cstdint>
#include <unordered_map>
#include <string>

enum class Ch10Status : uint8_t
{
    NONE,
    OK,
    PKT_TYPE_YES,
    PKT_TYPE_NO,
    PKT_TYPE_EXIT,
    BAD_SYNC,
    BUFFER_LIMITED,
    INVALID_SECONDARY_HDR_FMT,
    INVALID_INTRAPKT_TS_SRC,
    CHECKSUM_NOT_PRESENT,
    CHECKSUM_NOT_HANDLED,
    CHECKSUM_TRUE,
    CHECKSUM_FALSE,
    TDP_NONE,
    TIME_FORMAT_INCONCLUSIVE,
    MILSTD1553_TS_NOT_HANDLED,
    MILSTD1553_MSG_COUNT,
    MILSTD1553_MSG_LENGTH,
    VIDEOF0_NONINTEGER_SUBPKT_COUNT,
    VIDEOF0_SUBPKT_COUNT_BIG,
    ETHERNETF0_FRAME_COUNT,
    ETHERNETF0_FRAME_LENGTH,
    ETHERNETF0_FRAME_PARSE_ERROR,
    ARINC429F0_PARITY_ERROR,
    ARINC429F0_FORMAT_ERROR,
    ARINC429F0_GAP_TIME_ERROR,
};

const std::unordered_map<Ch10Status, std::string> ch10status_to_string_map = {
    {Ch10Status::NONE, "NONE"},
    {Ch10Status::OK, "OK"},
    {Ch10Status::PKT_TYPE_YES, "PKT_TYPE_YES"},
    {Ch10Status::PKT_TYPE_NO, "PKT_TYPE_NO"},
    {Ch10Status::PKT_TYPE_EXIT, "PKT_TYPE_EXIT"},
    {Ch10Status::BAD_SYNC, "BAD_SYNC"},
    {Ch10Status::BUFFER_LIMITED, "BUFFER_LIMITED"},
    {Ch10Status::INVALID_SECONDARY_HDR_FMT, "INVALID_SECONDARY_HDR_FMT"},
    {Ch10Status::INVALID_INTRAPKT_TS_SRC, "INVALID_INTRAPKT_TS_SRC"},
    {Ch10Status::CHECKSUM_NOT_PRESENT, "CHECKSUM_NOT_PRESENT"},
    {Ch10Status::CHECKSUM_NOT_HANDLED, "CHECKSUM_NOT_HANDLED"},
    {Ch10Status::CHECKSUM_TRUE, "CHECKSUM_TRUE"},
    {Ch10Status::CHECKSUM_FALSE, "CHECKSUM_FALSE"},
    {Ch10Status::TDP_NONE, "TDP_NONE"},
    {Ch10Status::TIME_FORMAT_INCONCLUSIVE, "TIME_FORMAT_INCONCLUSIVE"},
    {Ch10Status::MILSTD1553_TS_NOT_HANDLED, "MILSTD1553_TS_NOT_HANDLED"},
    {Ch10Status::MILSTD1553_MSG_COUNT, "MILSTD1553_MSG_COUNT"},
    {Ch10Status::MILSTD1553_MSG_LENGTH, "MILSTD1553_MSG_LENGTH"},
    {Ch10Status::ARINC429F0_PARITY_ERROR, "ARINC429F0_PARITY_ERROR"},
    {Ch10Status::ARINC429F0_FORMAT_ERROR, "ARINC429F0_FORMAT_ERROR"},
    {Ch10Status::ARINC429F0_GAP_TIME_ERROR, "ARINC429F0_GAP_TIME_ERROR"}};

std::string Ch10StatusString(const Ch10Status& status);

#endif