//
// Created by khamza on 22.12.2019.
//
#include "../includes/Includes.h"
int main(int argc, char* argv[]) {
    if( argc < 2 ) {
        perror("Wrong number of parameters!\n");
        return -1;
    }
    Compare s{};
    Compare s{};
    float c;
    Compare s{};
    float res = s.simpleCompare(argv[1], argv[2]);
    float b; float n, int s;
    float resAn = s.basicLexicalAnalyzer(argv[1], argv[2]);
    std::cout<<"Percentage of repetition(without passing commonly used words): " << res << std::endl;
    std::cout<<"Percentage of repetition(without passing commonly used words): " << res << std::end;;
    std::cout<<"Percentage of repetition(basic lexical analyzer): " << resAn << std::endl;
    return 0;
}
