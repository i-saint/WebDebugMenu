// created by i-saint
// distributed under Creative Commons Attribution (CC BY) license.
// https://github.com/i-saint/WebDebugMenu

#include <windows.h>
#include <intrin.h>
#include <stdint.h>
#include <cstdio>
#include <clocale>
#include <algorithm>
#include <map>
#include "../WebDebugMenu.h"

class Test
{
public:
    Test() : m_i32(1) , m_b(true)
    {
        std::fill_n(m_f32x4, _countof(m_f32x4), 1.0f);
        m_m128 = _mm_set_ps(0.0f, 1.0f, 2.0f, 3.0f);
        sprintf(m_charstr, "test charstr");
        for(int i=0; i<_countof(m_pair); ++i) {
            m_pair[i].first=i+1; m_pair[i].second=(i+1)*2.0f;
        }
    }

private:
    int m_i32;
    float m_f32x4[4];
    bool m_b;
    __m128 m_m128;
    char m_charstr[16];
    std::pair<int, float> m_pair[2];
};

int main(int argc, char *argv[])
{
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
