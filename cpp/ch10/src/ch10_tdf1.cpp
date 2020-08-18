// ch10_tdf1.cpp

#include "ch10_tdf1.h"

Ch10TDF1::~Ch10TDF1()
{
	
}

void Ch10TDF1::Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd)
{
	ch10td_ptr_ = ch10td;
	ch10hd_ptr_ = ch10hd;

	// Set the Ch10TimeData RTC from the packet header for this
	// Time Data Packet.
	ch10td_.timedatapkt_rtc_ = ch10hd_ptr_->header_rtc_;
}

uint8_t Ch10TDF1::Parse()
{
	retcode_ = 0;
	
	// "Parse" by setting the channel specific data class pointer 
	// to the appropriate place in the raw bytes.
	data_fmt_ptr_ = (const TDF1ChanSpecFormat*) bb_ptr_->Data();

	/*
	Logic to filter inaccurate or erroneous time data.

	DRA filters all time data (and corresponding 1553 payloads)
	if the time src is 0 (internal time derived from the recorder)
	and the time format (fmt) is 0 (IRIG-B). Prior to including this
	logic, I did not exclude such packets, which have incorrect date/time.
	My guess for why this is true is that an internal source is inherently
	incorrect when the time format is specifically that of an external one,
	in this case IRIG-B, which originates in the EGI or, most importantly,
	not within the recorder. Therefore I'll filter all time data packets
	which specifcy an internal time source and time format relevant to
	an external source. The only internal time format available seems to 
	be the Real-time clock (data_fmt_ptr_ == 3). 
	
	Upon further investigation, I find
	that many test stand Ch10s have source = 0 and format = 0, so this is
	not a reliable way to filter. I can't find any other way that DRA would
	identify a time packet as invalid other than scanning the time packets
	for anomalous jumps, then discarding initial packets. 

	Also filter if time format is none (0xF) or time source is none (0xF).
	*/
	/*if (data_fmt_ptr_->src == 0 && fmt->time_fmt != 3)
	{
		debug_info();
#ifdef DEBUG
#if DEBUG > 0
		printf("Filtering Time Data Packet: time source is internal (0) and time format is not real-time clock (3)!\n");
#endif
#endif
		stat = TDF1Status::INCOMPATIBLE;
		retcode = 1;
		return retcode;
	}*/
	if (data_fmt_ptr_->time_fmt == 0xf || data_fmt_ptr_->src == 0xF)
	{
		//debug_info();
#ifdef DEBUG
#if DEBUG > 0
		printf("Filtering Time Data Packet: time source or time format is None (invalid)!\n");
#endif
#endif
		status_ = TDF1Status::INVALID;
		retcode_ = 1;
		return retcode_;
	}
	
	#ifdef DEBUG
	if (DEBUG > 2)
		debug_info();
	#endif
	
	// Advance buffer location to beginning of 
	// data.
	bb_ptr_->AdvanceReadPos(data_fmt_size_);
	
	if(data_fmt_ptr_->date_fmt == 0)
	{
		// 3-byte date format.
		proc_3byte_date();
	}
	else if(data_fmt_ptr_->date_fmt == 1)
	{
		// 4-byte date format.
		proc_4byte_date();
	}

	status_ = TDF1Status::PARSE_OK;
	return retcode_;
}

void Ch10TDF1::proc_3byte_date()
{
	ch10td_.doy_ = 1;
	d3fmt = (const TDF1Data3Format*) bb_ptr_->Data();
	
	uint16_t day = uint16_t(100.*d3fmt->HDn + 10.*d3fmt->TDn + d3fmt->Dn);
	uint16_t hour = uint16_t(10.*d3fmt->THn + d3fmt->Hn);
	uint16_t minute = uint16_t(10.*d3fmt->TMn + d3fmt->Mn);
	uint16_t sec = uint16_t(10.*d3fmt->TSn + d3fmt->Sn);
	uint16_t ms = uint16_t(100.*d3fmt->Hmn + 10.*d3fmt->Tmn);

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
	rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
	rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif
	
	// Convert to nanoseconds.
	ch10td_.timedatapkt_abstime_ = rawtime*uint64_t(1e9) + uint64_t(ms)*uint64_t(1e6);

#ifdef DEBUG
#if DEBUG > 3
	{
		char buff[200];
		strftime(buff, 199, "%j:%H:%M:%S\n", &temptime);
		printf("(%03u) 3byte date: %s\n", id_, buff);
		//printf("(%03u) 3byte date: %s\n", id, asctime(&temptime));
	}
#endif
#endif
}

void Ch10TDF1::proc_4byte_date()
{
	ch10td_.doy_ = 0;
	d4fmt = (const TDF1Data4Format*) bb_ptr_->Data();
	
	uint16_t year = uint16_t(1000.*d4fmt->OYn + 100.*d4fmt->HYn + 10.*d4fmt->TYn + d4fmt->Yn);
	uint16_t mth = uint16_t(10.*d4fmt->TOn + d4fmt->On);
	uint16_t day = uint16_t(10.*d4fmt->TDn + d4fmt->Dn);
	uint16_t hour = uint16_t(10.*d4fmt->THn + d4fmt->Hn);
	uint16_t minute = uint16_t(10.*d4fmt->TMn + d4fmt->Mn);
	uint16_t sec = uint16_t(10.*d4fmt->TSn + d4fmt->Sn);
	uint16_t ms = uint16_t(100.*d4fmt->Hmn + 10.*d4fmt->Tmn);
	
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
	temptime.tm_mon = mth-1;
	temptime.tm_year = year - 1900;
	temptime.tm_isdst = 0;
#ifdef __WIN64
	rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
	rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif
	
	// Convert to nanoseconds since the epoch and add milliseconds.
	ch10td_.timedatapkt_abstime_ = rawtime*uint64_t(1e9) + uint64_t(ms)*uint64_t(1e6);

#ifdef DEBUG
#if DEBUG > 3
	char buff[200];
	strftime(buff, 199, "%Y-%m-%d %H:%M:%S\n", &temptime);
	printf("(%03u) 4byte date: %s\n", id_, buff);
#endif
#endif
}

void Ch10TDF1::debug_info()
{
	printf("\n-- (%03u) Time Data Packet, Format 1 -- \n", id_);
	printf("Source 		= %hhu\n", data_fmt_ptr_->src);
	printf("Time format	= %hhu\n", data_fmt_ptr_->time_fmt);
	printf("Leap year 	= %hhu\n", data_fmt_ptr_->leap_year);
	printf("Date format	= %hhu\n", data_fmt_ptr_->date_fmt);
}
