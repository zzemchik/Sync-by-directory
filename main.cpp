#include "sinxronDir.hpp"

int main(int argc, char* argv[])
{

    if (argc < 5 || argc > 6)
    {
        std::cout << "Error argument" << std::endl;
        return 0;
    }


    std::string     firstDir = argv[1];
    std::string     secondDir = argv[2];
    std::string     logFile = argv[3];
    size_t          time = atoi(argv[4]);
    size_t count = 0;
    if (argc == 6)
        count = atoi(argv[5]);
    try
    {
        SDir a(firstDir, secondDir, logFile, time, count);
    }
    catch (const char *ex)
    {
        std::cout << ex << std::endl;
    }
    return 0;
}