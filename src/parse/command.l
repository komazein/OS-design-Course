/*******************************
Flex commands, parsers and rules are written here.
Format:
%{
    C/C++ declarations (headers, enums, defines, globals)
%}
Definitions (regex aliases)
%%
Rules         ¡ú Define matching behavior
%%
User subroutines
********************************/

%{
#include <stdio.h>
#include <string.h>
#include <string>
#include "file_system_manager.h"
#include "log_global.h"
char arg_c[64];  // args for commands
extern file_system_manager* fileSystem;  // to be set from outside
enum TokenType {
    COMMAND = 1,   // keyword
    NAME,          // file or dir name
    SYMBOL,        // symbol
    NUMBER,        // number
    CONTANT,       // text content
    TEMINATE       // newline or EOF
};
%}

%option c++
%option noyywrap

%%



mkdir[ \t]+[a-zA-Z0-9_/\-]+ {
    sscanf(yytext, "mkdir %s", arg_c);
    std::string arg(arg_c);
    fileSystem->command_mkdir(arg);
}

chdir[ \t]+([a-zA-Z0-9_\./\-]+) {
    sscanf(yytext, "chdir %s", arg_c);
    std::string arg(arg_c);
    fileSystem->command_cd(arg);
}

lsdir {
    fileSystem->command_ls();
}

rmdir[ \t]+[a-zA-Z0-9_/.\-]+ {
    sscanf(yytext, "rmdir %s", arg_c);
    std::string arg(arg_c);
    fileSystem->command_rm(arg);
}

lkdir[ \t]+([a-zA-Z0-9_/\-]+)[ \t]+([a-zA-Z0-9_/\-]+) {
    char src[256], dest[256];
    sscanf(yytext, "lkdir %255s %255s", src, dest);
    std::string source(src);
    std::string linkname(dest);
    fileSystem->command_lkdir(source, linkname);
}


lndir[ \t]+([a-zA-Z0-9_/.\-]+)[ \t]+([a-zA-Z0-9_/.\-]+) {
    char src[256], dest[256];
    sscanf(yytext, "lndir %255s %255s", src, dest);
    std::string arg1(src);
    std::string arg2(dest);
    fileSystem->command_lndir(arg1, arg2);
}


find[ \t]+([a-zA-Z0-9_/.\-]+)[ \t]*(-b)? {
    char path[100];
    int isFuzzy = 0;
    sscanf(yytext, "find %s", path);

    if (strstr(yytext, "-b") != NULL) {
        isFuzzy = 1;
    }

    std::string filepath(path);
    fileSystem->command_find(filepath, isFuzzy);
}


touch[ \t]+[a-zA-Z0-9_-]+\.[a-zA-Z0-9]+ {
    sscanf(yytext, "touch %s", arg_c);
    std::string arg(arg_c);
    fileSystem->command_touch(arg);
}



cat[ \t]+[a-zA-Z0-9_/.\-]+ {
    sscanf(yytext, "cat %s", arg_c);
    std::string arg(arg_c);
    fileSystem->command_cat(arg);
}

edit[ \t]+([a-zA-Z0-9_\./\-]+)[ \t]+(.+) {
    std::string full(yytext);
    size_t first_space = full.find_first_of(" \t");
    size_t second_space = full.find_first_of(" \t", first_space + 1);

    std::string filename = full.substr(first_space + 1, second_space - first_space - 1);
    std::string content = full.substr(second_space + 1);

    fileSystem->command_edit(filename, content);  
}

chmod[ \t]+[a-zA-Z0-9_]+\.[a-zA-Z0-9_]+[ \t]+user[0-9]+ {
    char filename_c[100];
    int userid;
    sscanf(yytext, "chmod %s user%d", filename_c, &userid);
    std::string filename(filename_c);
    fileSystem->command_athrz(filename, userid);
}

rcmod[ \t]+[a-zA-Z0-9_]+\.[a-zA-Z0-9_]+[ \t]+user[0-9]+ {
    char filename_c[100];
    int userid;
    sscanf(yytext, "rcmod %s user%d", filename_c, &userid);
    std::string filename(filename_c);
    fileSystem->command_rathrz(filename, userid);
}

execute[ \t]+[a-zA-Z0-9_/.-]+\.exe {
    sscanf(yytext, "execute %s", arg_c);
    std::string filename(arg_c);
    fileSystem->command_exe(filename);
}


Exit {
    fileSystem->Exit();
}

format {
    fileSystem->format();
}

help {
    fileSystem->help();
}



\n      { return 0; }

.*      {
     printf("syntax error for undeclared command: %s\n", yytext);
     std::string command(yytext);
     g_logger->warn("syntax error for undeclared command: '{}' ", command);

}

%%
// Optional user-defined functions go here