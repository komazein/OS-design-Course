#include "test.h"
#include <FlexLexer.h>
file_system_manager fileSystem;
int main()
{

yyFlexLexer lexer;
    lexer.yylex();  // ��͵��������� lexer.l �ж���Ĵʷ�����
    
    //testFuncMkdir();  // Test the mkdir command in the file system manager

    // Uncomment to run other tests
    // func();          // Test function for directory tree and block scheduler
    // func2();         // Test function for dentry key search
    //     while (1)
//     {
//         printf("[");
        
//         printf("]$ ");
//         yylex();
//     }
    return 0;
}