#pragma once

#include "TL4000.h"
#include "visa.h"


struct ResourceString_t;
void error_exit(ViSession handle, ViStatus err);
void waitKeypress();
int controllaser(ViBoolean mode);
ViStatus find_instruments(ViString findPattern, ViChar** resource);
ViStatus get_device_id(ViSession handle);