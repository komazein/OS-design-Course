#include "test.h"
void func()
{
    blockScheduler a;
    for(size_t i=0;i<100;i++)
        cout<<i<<","<<a.cal_block_num_dir(i)<<endl;
    
}