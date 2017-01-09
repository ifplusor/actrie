#include <sys/timeb.h>

#include "common.h"


const size_t POOLREGIONSIZE = REGIONSIZE;
const size_t POOLPOSITIONSIZE = POSITIONMASK+1;


long long getSystemTime()
{
	struct timeb t;
	ftime(&t);
	return t.time*1000+t.millitm;
}
