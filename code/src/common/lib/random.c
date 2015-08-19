
#include "stdlib.h"
#include "lib.h"

#include "random.h"

/*create a random number between min and max*/
uint8_t random(uint8_t min, uint8_t max)         //添加stdlib.h 头文件
{
    DBG_ASSERT(max >= min __DBG_LINE);
//    srand(TA0R);
    srand(100);
    return (min + rand() % (max - min + 1));
}