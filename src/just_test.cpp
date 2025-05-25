#include "test.h"
class testii
{
    private:
        int x;
        char b;
    public:
        void init(){return;}
        int f()
        {
            x=1;
            b='x';
            return x;
        }
        void out()
        {
            cout<<x<<" "<<b<<endl;
        }
};
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