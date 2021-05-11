#ifndef PARSEUTIL_H
#define PARSEUTIL_H

#include <ctime>
#include <cstdint>
#include <cmath>
#include <cstdio>

void recover_4byte_date(uint64_t& time_stamp, std::tm*& timeinfo, uint32_t*& sec_fraction);
void recover_3byte_date(uint64_t& time_stamp, std::tm*& timeinfo, uint32_t*& sec_fraction);

#endif PARSEUTIL_H