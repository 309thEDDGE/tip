#ifndef META_MAIN_H_
#define META_MAIN_H_

#include <string>
#include <cstdio>
#include "sysexits.h"
#include "stream_buffering.h"
#include "version_info.h"
#include "argument_validation.h"
#include "ch10_parse_main.h"
#include "meta_cli.h"

int MetaMain(int argc, char** argv);

#endif  // META_MAIN_H_