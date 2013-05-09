#include "WebDebugMenu.h"
#include "windows.h"

int main(int argc, char *argv[])
{
    wdmInitialize();

    int i = 1;
    bool b = false;
    float f = 10.0f;
    wdmAddDataNode("Test/test_int", &i);
    wdmAddDataNode("Test/test_bool", &b);
    wdmAddDataNode("Test/test_float", &f);
    for(;;) {
        wdmFlush();
        ::Sleep(1000);
    }

    wdmFinalize();
}
