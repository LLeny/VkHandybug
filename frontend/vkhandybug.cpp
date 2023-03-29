#include <stdlib.h>
#include "app.h"

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
int main(int argc, char **argv)
{
    auto app = std::make_shared<App>();

    app->initialize();
    app->main_loop();
    app->close();

    exit(0);
}