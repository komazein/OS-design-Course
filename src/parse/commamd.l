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
#include "shellmanager.h"

Token return_token(const std::string& text, int type)  { return std::make_pair(text, type); }

char arg_c[64];       // args for commands
shell_manager* shell; // provide for flex 
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

mkdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "mkdir %s", arg_c); shell->command_mkdir(std::string arg(arg_c));}
chdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "chdir %s", arg_c); shell->command_cd(std::string arg(arg_c));}
lsdir                         { command_ls();}
rmdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "rmdir %s", arg_c); shell->command_rm(std::string arg(arg_c));}
lkdir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "lkdir %s", arg_c); shell->command_lkdir(std::string arg(arg_c));}
lndir[ \t]+[a-zA-Z0-9_/.-]+   { sscanf(yytext, "lndir %s", arg_c); shell->command_lndir(std::string arg(arg_c));}
find[ \t]+[a-zA-Z0-9_/.-]+    { sscanf(yytext, "find %s",  arg_c); shell->command_find(std::string arg(arg_c));}
touch[ \t]+[a-zA-Z0-9_-]+\.[a-zA-Z0-9]+  { sscanf(yytext, "touch %s", arg_c); shell->command_touch(std::string arg(arg_c));}
delete[ \t]+[a-zA-Z0-9_/.-]+  { sscanf(yytext, "delete %s", arg_c); shell->command_delete(std::string arg(arg_c));}
cat[ \t]+[a-zA-Z0-9_/.-]+     { sscanf(yytext, "cat %s", arg_c);   shell->command_cat(std::string arg(arg_c));}


exit                          { shell->exit();}
format                        { shell->format();}
help                          { shell->help();} 
\n                            { return 0;}
.*                            { printf("syntax error for undeclare commmand: %s\n", yytext);}



%%