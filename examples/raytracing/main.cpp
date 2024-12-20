#include "application.h"
#include <iostream>

int main(int argc, char* argv[]) {

    Application app;
    try
    { app.run(argc, argv); } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}