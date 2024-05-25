#include <iostream>
#include "application.h"

int main(int argc, char* argv[])
{

    VulkanRenderer app;
    try
    {
        app.run(argc,argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}