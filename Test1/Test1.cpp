// created by i-saint
// distributed under Creative Commons Attribution (CC BY) license.
// https://github.com/i-saint/WebDebugMenu

#include <windows.h>
#include <intrin.h>
#include <stdint.h>
#include <cstdio>
#include <clocale>
#include <algorithm>

//#define wdmDisable
#include "../WebDebugMenu.h"

struct teststruct {
    int int_value;
    float float_value;

    teststruct() : int_value(10), float_value(10.0f) {}
};

class Test
{
public:
    Test()
        : m_i32(1)
        , m_ci32(64)
        , m_b(true)
        , m_f32(10.0f)
    {
        std::fill_n(m_i32a, _countof(m_i32a), 0);
        _snprintf(m_charstr, _countof(m_charstr), "test charstr");
        _snwprintf(m_wcharstr, _countof(m_wcharstr), L"test wcharstr");

        m_m128 = _mm_set_ps(0.0f, 1.0f, 2.0f, 3.0f);
        wdmScope( wdmString p = wdmFormat("Test0x%p", this); );
        wdmAddNode(p + "/m_i32", &m_i32, wdmMakeRange(100, 500));
        wdmAddNode(p + "/m_i32a", &m_i32a, wdmMakeRange(0, 100));
        wdmAddNode(p+"/property_i32", this, &Test::getI32, &Test::setI32, wdmMakeRange(0, 500));
        wdmAddNode(p+"/property_ro_i32", this, &Test::getI32);
        wdmAddNode(p+"/m_ci32", &m_ci32);
        wdmAddNode(p+"/m_b", &m_b);
        wdmAddNode(p + "/m_f32", &m_f32, wdmMakeRange(- 1.0f, 1.0f));
        wdmAddNode(p + "/m_m128", &m_m128, wdmMakeRange(0.0f, 10.0f));
        wdmAddNode(p+"/dir2/m_charstr", &m_charstr);
        wdmAddNode(p+"/dir2/m_wcharstr", &m_wcharstr);
        wdmAddNode(p+"/dir2/dir3/property_charstr", this, &Test::getCharStr);
        wdmAddNode(p+"/dir2/print()", &Test::print, this);
        wdmAddNode(p+"/func1()", &Test::func1, this);
        wdmAddNode(p+"/func2()", &Test::func2, this);
    }

    ~Test()
    {
        wdmEraseNode( wdmFormat("Test0x%p", this) );
    }

    void setI32(const int &v) { m_i32=v; }
    const int& getI32() const { return m_i32; }
    const char* getCharStr() const { return m_charstr; }

    void func1(int a) { printf("func1: %d\n", a); }
    void func2(int a, int b) { printf("func2: %d %d\n", a, b); }

    void print() const
    {
        printf(
            "m_i32:%d\n"
            "m_ci32:%d\n"
            "m_b:%d\n"
            "m_f32:%f\n",
            m_i32, m_ci32, m_b, m_f32 );
    }

private:
    int m_i32;
    int64_t m_i32a[4];
    const int m_ci32;
    bool m_b;
    float m_f32;
    __m128 m_m128;
    char m_charstr[16];
    wchar_t m_wcharstr[16];
    teststruct m_struct[2];
    std::string m_str;
};

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "utf-8");
    wdmInitialize();
    wdmOpenBrowser();

    {
        Test test;
        bool end_flag = false;
        wdmAddMemberNodes("test", &test, "Test");
        wdmAddNode("end_flag", &end_flag);
        while(!end_flag) {
            wdmFlush();
            ::Sleep(100);
        }
    }
    wdmFinalize();
}
