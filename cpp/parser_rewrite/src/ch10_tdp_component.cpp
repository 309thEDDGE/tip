#include "ch10_tdp_component.h"

Ch10Status Ch10TDPComponent::Parse(const uint8_t*& data, uint64_t& loc)
{
	// Parse the TDP CSDW
	ParseElements(tdp_csdw_elem_vec_, data, loc);

	// Filter if time format is none(0xF) or time source is none(0xF).
	if ((*tdp_csdw_elem_.element)->time_fmt == 0xf || (*tdp_csdw_elem_.element)->src == 0xf)
	{
		return Ch10Status::TDP_NONE;
	}

	// Parse the IRIG time (day format) or non-IRIG time (day, month, year format)
	// then compute absolute time in nanoseconds.
	uint64_t abs_time = 0;
	uint8_t doy = 0;
	if ((*tdp_csdw_elem_.element)->date_fmt == 0)
	{
		ParseElements(tdp_irig_elem_vec_, data, loc);
		abs_time = ComputeIRIGTime(*tdp_irig_elem_.element);
		doy = 1;
	}
	else
	{
		ParseElements(tdp_nonirig_elem_vec_, data, loc);
	}

	// Update context with TDP data.
	//ctx_->UpdateWithTDPData();
	return Ch10Status::OK;
}

uint64_t Ch10TDPComponent::ComputeIRIGTime(const TDF1DataIRIGFmt* const irig_fmt)
{

	uint16_t day = uint16_t(100. * irig_fmt->HDn + 10. * irig_fmt->TDn + irig_fmt->Dn);
	uint16_t hour = uint16_t(10. * irig_fmt->THn + irig_fmt->Hn);
	uint16_t minute = uint16_t(10. * irig_fmt->TMn + irig_fmt->Mn);
	uint16_t sec = uint16_t(10. * irig_fmt->TSn + irig_fmt->Sn);
	uint16_t ms = uint16_t(100. * irig_fmt->Hmn + 10. * irig_fmt->Tmn);

	// Check day for 0 error.
	if (day == 0)
		day = 1;

	// Get seconds into the first year of the epoch.
	std::tm temptime = { 0 };
	temptime.tm_sec = sec;
	temptime.tm_min = minute;
	temptime.tm_hour = hour;
	temptime.tm_year = 70;
	temptime.tm_isdst = 0;
	temptime.tm_mday = day;
#ifdef __WIN64
	uint64_t rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
	uint64_t rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif

	// Convert to nanoseconds.
	rawtime = rawtime * uint64_t(1e9) + uint64_t(ms) * uint64_t(1e6);

	return rawtime;
}