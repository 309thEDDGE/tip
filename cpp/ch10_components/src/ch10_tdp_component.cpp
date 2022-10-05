#include "ch10_tdp_component.h"

Ch10Status Ch10TDPComponent::Parse(const uint8_t*& data)
{
    // Parse the TDP CSDW
    ParseElements(tdp_csdw_elem_vec_, data);

    uint64_t abs_time = 0;
    uint8_t doy = 0;

    // Filter if time format is none(0xF) or time source is none(0xF).
    if ((*tdp_csdw_elem_.element)->time_fmt == 0xf || (*tdp_csdw_elem_.element)->src == 0xf)
    {
        // Update context with TDP data.
        ctx_->UpdateWithTDPData(abs_time, doy, false, *(*tdp_csdw_elem_.element));
        return Ch10Status::TDP_NONE;
    }

    // Parse the IRIG time (day format) or non-IRIG time (day, month, year format)
    // then compute absolute time in nanoseconds.

    if ((*tdp_csdw_elem_.element)->date_fmt == 0)
    {
        ParseElements(tdp_irig_elem_vec_, data);
        abs_time = ComputeIRIGTime(*tdp_irig_elem_.element);
        SPDLOG_DEBUG("({:02d}) time data packet absolute time (abs_time_): {:d}",
                     ctx_->thread_id, abs_time);
        doy = 1;
    }
    else
    {
        ParseElements(tdp_nonirig_elem_vec_, data);
        abs_time = ComputeNonIRIGTime(*tdp_nonirig_elem_.element);
        doy = 0;
    }

    // Update context with TDP data.
    ctx_->UpdateWithTDPData(abs_time, doy, true, *(*tdp_csdw_elem_.element));

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
    std::tm temptime = {0};
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

uint64_t Ch10TDPComponent::ComputeNonIRIGTime(const TDF1DataNonIRIGFmt* const nonirig_fmt)
{
    uint16_t year = uint16_t(1000. * nonirig_fmt->OYn + 100. * nonirig_fmt->HYn +
                             10. * nonirig_fmt->TYn + nonirig_fmt->Yn);
    uint16_t mth = uint16_t(10. * nonirig_fmt->TOn + nonirig_fmt->On);
    uint16_t day = uint16_t(10. * nonirig_fmt->TDn + nonirig_fmt->Dn);
    uint16_t hour = uint16_t(10. * nonirig_fmt->THn + nonirig_fmt->Hn);
    uint16_t minute = uint16_t(10. * nonirig_fmt->TMn + nonirig_fmt->Mn);
    uint16_t sec = uint16_t(10. * nonirig_fmt->TSn + nonirig_fmt->Sn);
    uint16_t ms = uint16_t(100. * nonirig_fmt->Hmn + 10. * nonirig_fmt->Tmn);

    // Get seconds since the epoch using mktime.
    std::tm temptime = {0};
    temptime.tm_sec = sec;
    temptime.tm_min = minute;
    temptime.tm_hour = hour;

    // Values from Ch10 may be [0, 30] and struct time takes
    // [1, 31] for tm_mday. This would explain why DRA shows
    // day+1, where day is tm_mday recovered from data I parse.
    // Will add 1 to day below to correct this.
    //
    // Note: the correction mentioned above is causing an off-by-one error
    // in both doy and non-doy date and time computations. My only
    // guess is that this correction and the above explanation is in
    // error. I now change day+1 to day for both.
    temptime.tm_mday = day;
    temptime.tm_mon = mth - 1;
    temptime.tm_year = year - 1900;
    temptime.tm_isdst = 0;
#ifdef __WIN64
    uint64_t rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
    uint64_t rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif

    // Convert to nanoseconds since the epoch and add milliseconds.
    rawtime = rawtime * uint64_t(1e9) + uint64_t(ms) * uint64_t(1e6);

    return rawtime;
}