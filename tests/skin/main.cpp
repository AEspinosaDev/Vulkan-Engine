#include <iostream>
#include "test.h"

int main(int argc, char* argv[])
{
    Logger::init(LogLevel::Info, "skinTest.log");
    Application app;
    try
    {
        app.run(argc,argv);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        Logger::shutdown();
        return EXIT_FAILURE;
    }
    
    Logger::shutdown();
    return EXIT_SUCCESS;
}