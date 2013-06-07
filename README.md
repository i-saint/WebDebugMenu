WebDebugMenu
============
debug menu implemented by HTTP Server & HTML.  
you can edit parameters, call functions from your browser.  
it is very easy to embed (because there is no connection between graphics libraries and input libraries), and easy to use.  


##demo movie  
[![WebDebugMenu demo](http://img.youtube.com/vi/469iTc8L6jM/0.jpg)](http://www.youtube.com/watch?v=469iTc8L6jM)


##detailed description
(japanese) http://i-saint.hatenablog.com/entry/2013/05/20/211004


##example
####editing values
```c++
int main(int argc, char *argv[])
{
    wdmInitialize();

    int int_value = 1;
    float float_value = 1.1f;
    bool end_flag = false;
    // ごく普通に表示
    wdmAddNode("Test/int_value", &int_value);
    wdmAddNode("Test/float_value", &float_value);
    // 範囲指定 (スライダで操作できるようになる)
    wdmAddNode("Test/ranged_int_value", &int_value, 0, 100);
    wdmAddNode("Test/ranged_float_value", &float_value, 0.0f, 10.0f);
    // const の値を追加すると readonly ノードになる
    wdmAddNode("Test/const_int_value", (const int*)&int_value);
    wdmAddNode("Test/const_float_value", (const float*)&float_value);

    wdmAddNode("Test/end_flag", &end_flag);
    while(!end_flag) {
        // wdmFlush() でブラウザから来たコマンドの処理が行われる
        // == 変数の更新、関数呼び出し、ブラウザへ送るデータの生成などはこのタイミングで行われる
        wdmFlush();
        ::Sleep(100);
    }
    wdmEraseNode("Test");

    wdmFinalize();
}
```
view in browser:  
<img src="https://raw.github.com/i-saint/WebDebugMenu/master/screenshot/desc2.png" />


####editing class members, calling functions
```c++
class Hoge
{
public:
    Hoge() : m_int_value(0) {}
    int getValue() const { return m_int_value; }
    void setValue(int v) { m_int_value=v; }
    void print() const { printf("Hoge::m_int_value: %d\n", m_int_value); }
    void printx(int v) const { printf("Hoge::m_int_value x %d: %d\n", v, m_int_value*v); }

    // wdmScope は wdmDisable を define しているときは消え失せるスコープ
    // (最終リリースの時は当然デバッグメニューは消える必要があり、
    //  wdmDisable を define するだけで全て消え失せるようになっている)
    wdmScope(
    void addDebugNode(const wdmString &parent)
    {
        wdmString path = parent+wdmFormat("/Hoge:0x%p", this);
        // getter/setter で値を変更するノード (範囲指定付き)
        wdmAddNode(path+"/value", this, &Hoge::getValue, &Hoge::setValue, 0, 100);
        // getter のみの場合 readonly なノードになる
        wdmAddNode(path+"/const_value", this, &Hoge::getValue);

        // 関数を呼ぶノード
        wdmAddNode(path+"/print()", &Hoge::print, this);
        // 引数付き関数を呼ぶノード
        wdmAddNode(path+"/printx()", &Hoge::printx, this);
    }
    )
private:
    int m_int_value;
};

int main(int argc, char *argv[])
{
    wdmInitialize();

    bool end_flag = false;
    Hoge hoge;
    wdmScope( hoge.addDebugNode("Test") );
    wdmAddNode("Test/end_flag", &end_flag);
    while(!end_flag) {
        wdmFlush();
        ::Sleep(100);
    }
    wdmEraseNode("Test");

    wdmFinalize();
}
```
view in browser:  
<img src="https://raw.github.com/i-saint/WebDebugMenu/master/screenshot/desc3.png" />


####editing complex data types (array, SSE __m128, string)
```c++
int main(int argc, char *argv[])
{
    wdmInitialize();

    bool end_flag = false;
    int int_array[8];
    __m128 simd_value;
    char char_array[32] = "hoge-";
    char *char_ptr = char_array;

    std::fill_n(int_array, _countof(int_array), 10);
    simd_value = _mm_set1_ps(1.0f);
    // 配列
    wdmAddNode("Test/int_array", &int_array, 0, 100);
    // SIMD
    wdmAddNode("Test/simd_value", &simd_value, 0.0f, 10.0f);
    // 文字列 (char[])
    wdmAddNode("Test/char_array", &char_array);
    // 文字列 (char* もいける。安全性を考えて readonly ノード)
    wdmAddNode("Test/char_ptr", &char_ptr);

    wdmAddNode("Test/end_flag", &end_flag);
    while(!end_flag) {
        wdmFlush();
        ::Sleep(100);
    }
    wdmEraseNode("Test");

    wdmFinalize();
}
```
view in browser:  
<img src="https://raw.github.com/i-saint/WebDebugMenu/master/screenshot/desc4.png" />

##license
<img src="http://mirrors.creativecommons.org/presskit/buttons/88x31/png/by.png" alt="CC BY" width="200" height="70" />

##thanks
[Poco](http://pocoproject.org/)  
