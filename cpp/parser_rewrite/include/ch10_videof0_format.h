#pragma once
// Video Data, Format 0, pp. 93 IRIG106 Ch10

#ifndef VIDEODATAF0FMT_H
#define VIDEODATAF0FMT_H

#include <cstdint>

const uint32_t TransportStream_UNIT_SIZE = 188;
const uint32_t MAX_TransportStream_UNITS = 600;
const uint32_t MAX_SIZE = TransportStream_UNIT_SIZE * MAX_TransportStream_UNITS;
typedef uint16_t video_datum; // sizeof this value must be equal to RECORDED_DATA_SIZE
const uint32_t RECORDED_DATA_SIZE = uint32_t(sizeof(video_datum));
const uint32_t MAX_DATA_COUNT = MAX_SIZE / RECORDED_DATA_SIZE;
const uint32_t TransportStream_DATA_COUNT = TransportStream_UNIT_SIZE / RECORDED_DATA_SIZE;

class VideoF0CSDWFmt
{
public:
	uint32_t : 23;  // reserved
	uint32_t BA : 1; // bit alignment: 0 = little-endian, 1 = big-endian
	uint32_t PL : 4;  // payload: 0 = MPEG-2 MP@ML, 1 = H.264 MP@L2.1, 2 = H.264 MP@L2.2, 3 = H.264 MP@L3, 4--15 = reserved
	uint32_t KLV : 1;  // KLV metadata present in MPEG-2 video, 0 = no metadata present
	uint32_t SRS : 1;  // SCR/RTC Sync. Indicates of the MPEG-2 SCR is RTC
	uint32_t IPH : 1;  // Intra-packet header. Indicates if intra-packet time stamps are inserted before each transport packet.
	uint32_t ET : 1;  // Embedded Time. Indicates if embedded time is present in the MPEG-2 video data.
};

#endif
