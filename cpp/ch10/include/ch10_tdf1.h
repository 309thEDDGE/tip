#ifndef CH10TDF1_H
#define CH10TDF1_H

#include "parse_context.h"
#include "TDF1Format.h"
#include <ctime>

class Ch10TDF1 : public ParseContext<TDF1ChanSpecFormat, TDF1Status>
{
	private:
	const TDF1Data3Format* d3fmt;
	const TDF1Data4Format* d4fmt;

	uint64_t rawtime;
	
	public:
	Ch10TDF1(BinBuff& buff, uint16_t ID) : ParseContext(buff, ID),
		d3fmt(nullptr), d4fmt(nullptr), rawtime(0) {}
	~Ch10TDF1();
	void Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd) override;
	//void operator = (const Ch10TDF1& c);
	//void set_up(BinBuff buff, uint16_t ID);
	//void initialize(uint64_t&);
	uint8_t Parse() override;
	void debug_info();
	void proc_3byte_date();
	void proc_4byte_date();
};

#endif
