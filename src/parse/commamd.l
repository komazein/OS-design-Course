/*******************************
Flex comands parsers and rulers are written here
Here comes the following format:
%{
Declaration   -----------head file /  enum  /  define  /  func declar
%}
Definitions   -----------regular expreesion / state
%%
Rules         -----------Define match behavior
%%
User subroutines---------User define program

********************************/

%{
#include <stdio.h>
#include <string.h>
#include "file_system_manager.h"

Token return_token(const std::string& text, int type)  { return std::make_pair(text, type); }

char arg_c[64];       // args for commands
file_system_manager* fileSystem; // provide for flex 
enum TokenType{
    COMMAND = 1,    // KEYWORD
    NAME,           // file name or dir name
    SYMBOL,         // SYMBOL
    NUMBER,         // digital
    CONTANT         // txt contant
    TEMINATE        // enter for teminate
}

%}
%option noyywarp

%%

mkdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "mkdir %s", arg_c); fileSystem->command_mkdir(std::string arg(arg_c));}
chdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "chdir %s", arg_c); fileSystem->command_cd(std::string arg(arg_c));}
lsdir                         { command_ls();}
rmdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "rmdir %s", arg_c); fileSystem->command_rm(std::string arg(arg_c));}
lkdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "lkdir %s", arg_c); fileSystem->command_lkdir(std::string arg(arg_c));}
lndir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "lndir %s", arg_c); fileSystem->command_lndir(std::string arg(arg_c));}
find[ \t]+[a-zA-Z0-9_/.-]+    { sscanf(yytext, "find %s",  arg_c); fileSystem->command_find(std::string arg(arg_c));}
touch[ \t]+[a-zA-Z0-9_-]+\.[a-zA-Z0-9]+  { sscanf(yytext, "touch %s", arg_c); fileSystem->command_touch(std::string arg(arg_c));}
delete[ \t]+[a-zA-Z0-9_/.-]+  { sscanf(yytext, "delete %s", arg_c); fileSystem->command_delete(std::string arg(arg_c));}
cat[ \t]+[a-zA-Z0-9_/.-]+     { sscanf(yytext, "cat %s", arg_c);   fileSystem->command_cat(std::string arg(arg_c));}


exit                          { fileSystem->exit();}
format                        { fileSystem->format();}
help                          { fileSystem->help();} 
\n                            { return 0;}
.*                            { printf("syntax error for undeclare commmand: %s\n", yytext);}



%%