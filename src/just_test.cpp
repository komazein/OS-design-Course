#include "test.h"
#include <spdlog/spdlog.h>

void func()
{
    blockScheduler a;
    for(size_t i=1;i<100;i++)
        cout<<i<<" "<<a.calSIMblockNUM(i)<<endl;
    return;
}
