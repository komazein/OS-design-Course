#include "test.h"
#include <FlexLexer.h>
file_system_manager fileSystem;

int main()
{
    yyFlexLexer lexer;
    lexer.yylex();  // ��͵��������� lexer.l �ж���Ĵʷ�����
    return 0;
}