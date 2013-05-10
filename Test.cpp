#include "WebDebugMenu.h"
#include "windows.h"

class Test
{
public:
    Test()
        : m_i32(1)
        , m_b(true)
        , m_f32(10.0f)
    {
        wdmScope( wdmString p = wdmFormat("Test0x%p", this); );
        wdmAddNode(p+"/m_i32", &m_i32);
        wdmAddNode(p+"/m_b", &m_b);
        wdmAddNode(p+"/m_f32", &m_f32);
        wdmAddNode(p+"/print()", &Test::print, this);
    }

    ~Test()
    {
        wdmEraseNode(wdmFormat("Test0x%p", this));
    }

    void print() const
    {
        printf(
            "m_i32:%d\n"
            "m_b:%d\n"
            "m_f32:%f\n",
            m_i32, m_b, m_f32 );
    }

private:
    int m_i32;
    bool m_b;
    float m_f32;
};

int main(int argc, char *argv[])
{
    wdmInitialize();

    Test test;
    for(;;) {
        wdmFlush();
        ::Sleep(1000);
    }

    wdmFinalize();
}
