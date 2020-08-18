// parse_tmats.cpp

#include "tmats.h"
#include "binbuff.h"
//#include "ch10_packet_header.h"
// using std::FILE*;
// using std::fopen;
// using std::fclose;

//#include <fstream>

int main()
{
	
	
	
	
	TMATS t(ref);
	uint32_t tmats_eof = UINT32_MAX;
	
	tmats_eof = t.parse(infile);
	if(tmats_eof == UINT32_MAX)
	{
		printf("TMATS EOF not found\n");
		return 0;
	}
	else
		printf("TMATS EOF at %d\n", tmats_eof);
	
	// Create a BinBuff object pointing to the first Ch10 byte.
	uint32_t tmats_end = tmats_eof + t.EOF_SIZE;
	BinBuff bb(infile, tmats_end, read_size);
	total_read_pos = tmats_end + read_size;
	
	// Create a Ch10PacketHeader object with new binary buffer. 
	Ch10PacketHeader pkthdr(bb);
	pkthdr.parse();
	
	fclose(infile);
	return 0;
}
