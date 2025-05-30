#include "test.h"
#include <FlexLexer.h>
file_system_manager* fileSystem = new file_system_manager;
int main()
{
  fileSystem->command_ls();
  string name = "user";
  //fileSystem->command_find(name, 1);
  yyFlexLexer lexer;
  while(1){
  // {
  // string name = "build";
  // fileSystem.command_mkdir(name);
    lexer.yylex();

      // Test the mkdir command in the file system manager

    // Uncomment to run other tests
  }

//  TESTWRITEBACK();
    return 0;
}