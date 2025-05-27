#include "tool.h"
#include <iostream>
#include <spdlog/spdlog.h>

void printLsResult(const std::string &name, TYPE type, int resultNum) {
    if (type == DIR) {
        printColor(name, Color::Blue, true);
    } else if (type == SIM_FILE) {
        printColor(name, Color::Green, false);
    } else {
        printColor(name, Color::Default, false);
    }
    if (!(resultNum % 10)) std::cout << std::endl;
}

void printPath(std::vector<std::string> &path) {
    if (path.empty()) {
        spdlog::warn("Path is empty, no path to print.");
        return;
    }

    std::cout << "The Paths of the found file:" << std::endl;
    for (auto &p : path) {
        std::cout << p << std::endl;
    }
}

void showHelpMessage() {
    std::cout << "Available commands:\n";
    std::cout << "mkdir <dirname>      - Create a new directory\n";
    std::cout << "cd <dirname>         - Change current directory\n";
    std::cout << "ls                   - List contents of the current directory\n";
    std::cout << "rm <dirname>         - Remove a directory\n";
    std::cout << "lkdir <dirname>      - Create a symbolic link to a directory (not implemented)\n";
    std::cout << "lndir <dirname>      - Create a hard link to a directory (not implemented)\n";
    std::cout << "find <filename>      - Find a file in the current directory and its subdirectories\n";
    std::cout << "touch <filename>     - Create a new file (not implemented)\n";
    std::cout << "delete <filename>    - Delete a file (not implemented)\n";
    std::cout << "cat <filename>       - Read the contents of a file (not implemented)\n";
    std::cout << "exit                 - Exit the shell\n";
}
