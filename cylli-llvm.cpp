#include "./src/CylliLLVM.h"
#include <string>

int main(int argc, char const *argv[])
{
    /*
        Program execution
    */
    std::string cylliCode = R"(
        (printf "I like number %d!! siu \n" 7)
    )";

    /*
        Compiler instation
    */
    CylliLLVM compiler;

    compiler.exec(cylliCode);
    return 0;
}
