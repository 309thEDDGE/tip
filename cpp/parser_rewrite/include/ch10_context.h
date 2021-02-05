
#ifndef CH10_CONTEXT_H_
#define CH10_CONTEXT_H_

#include <cstdint>
#include <cstdio>

enum class Ch10PacketType : uint8_t
{
	COMPUTER_GENERATED_DATA_F1 = 0x01,
	TIME_DATA_F1               = 0x11,
	MILSTD1553_F1              = 0x19,
	VIDEO_DATA_F0              = 0x40,
};

class Ch10Context
{
private:
	uint64_t absolute_position_;
	uint64_t tdp_rtc_;
	uint64_t tdp_abs_time_;

	bool searching_for_tdp_;

public:
	const uint64_t& absolute_position;
	const uint64_t& tdp_rtc;
	const uint64_t& tdp_abs_time;
	Ch10Context(const uint64_t& abs_pos) : absolute_position_(abs_pos), 
		absolute_position(absolute_position_),
		tdp_rtc_(0), tdp_rtc(tdp_rtc_), tdp_abs_time_(0), tdp_abs_time(tdp_abs_time_),
		searching_for_tdp_(false) {}

	void SetSearchingForTDP(bool should_search);
	bool ContinueWithPacketType(uint8_t data_type);
	void UpdateAbsolutePosition(uint64_t new_absolute_pos);
};


#endif