#include "test.h"
#include <FlexLexer.h>
file_system_manager fileSystem;

int main()
{
    yyFlexLexer lexer;
    lexer.yylex();  // 这就调用了你在 lexer.l 中定义的词法规则
    return 0;
}