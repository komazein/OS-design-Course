#include "test.h"
#include <spdlog/spdlog.h>

void func()
{
   
    testii a;
    a.f();
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,0,SEEK_SET);//写回第IN快磁盘
    fwrite(&a, sizeof(testii),1,fp);
    fclose(fp);
    testii b;
    b.out();
    fp=fopen("../disk.img","r+");
    fseek(fp,0,SEEK_SET);//写回第IN快磁盘
    fread(&b, sizeof(testii),1,fp);
    fclose(fp);
    b.out();
}
