#ifndef WebDebugMenu_h
#define WebDebugMenu_h

#ifndef wdmDisable
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <cstdarg>
#include <intrin.h>
#include <stdint.h>

#if   defined(wdmDLL_Impl)
#   define wdmIntermodule __declspec(dllexport)
#elif defined(wdmStatic)
#   define wdmIntermodule
#else // wdmDynamic
#   define wdmIntermodule __declspec(dllimport)
#   pragma comment(lib, "WebDebugMenu.lib")
#endif


typedef uint32_t wdmID;
struct wdmEvent
{
    wdmID node;
    const char *command;
};

class wdmNode
{
protected:
    virtual             ~wdmNode() {}
public:
    virtual void        release()=0;
    virtual wdmID       getID() const=0;
    virtual const char* getName() const=0;
    virtual size_t      getNumChildren() const=0;
    virtual wdmNode*    findChild(const char *name) const=0;
    virtual wdmNode*    getChild(size_t i) const=0;
    virtual void        addChild(const char *path, wdmNode *child)=0;
    virtual void        setName(const char *name, size_t len=0)=0;
    virtual size_t      jsonize(char *out, size_t len) const=0;
    virtual bool        handleEvent(const wdmEvent &evt)=0;
};

extern "C" {
    wdmIntermodule void     wdmInitialize();
    wdmIntermodule void     wdmFinalize();
    wdmIntermodule void     wdmFlush();

    // 以下は内部実装用
    wdmIntermodule wdmID    _wdmGenerateID();
    wdmIntermodule wdmNode* _wdmGetRootNode();
    wdmIntermodule void     _wdmRegisterNode(wdmNode *node);
    wdmIntermodule void     _wdmUnregisterNode(wdmNode *node);
};



#ifdef _WIN32
#define snprintf    _snprintf
#define vsnprintf   _vsnprintf
#endif // _WIN32

typedef std::string wdmString;
template<class T> inline const char* wdmTypename();
template<class T> inline bool wdmParse(const char *text, T &value);
template<class T> inline size_t wdmStringnize(char *text, size_t len, T value);

template<> inline const char* wdmTypename<int8_t >() { return "int8"; }
template<> inline const char* wdmTypename<int16_t>() { return "int16"; }
template<> inline const char* wdmTypename<int32_t>() { return "int32"; }
template<> inline const char* wdmTypename<uint8_t >(){ return "uint8"; }
template<> inline const char* wdmTypename<uint16_t>(){ return "uint16"; }
template<> inline const char* wdmTypename<uint32_t>(){ return "uint32"; }
template<> inline const char* wdmTypename<bool>()    { return "bool"; }
template<> inline const char* wdmTypename<float>()   { return "float32"; }
template<> inline const char* wdmTypename<double>()  { return "float64"; }

template<> inline bool wdmParse(const char *text, int8_t  &v)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, int16_t &v)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, int32_t &v)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint8_t  &v) { uint32_t t; if(sscanf(text, "%u", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint16_t &v) { uint32_t t; if(sscanf(text, "%u", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint32_t &v) { uint32_t t; if(sscanf(text, "%u", &t)==1){ v=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, bool &v)     { int32_t t;  if(sscanf(text, "%i", &t)==1){ v=t!=0; return true; } return false; }
template<> inline bool wdmParse(const char *text, float &v)    { return sscanf(text, "%f", &v)==1; }
template<> inline bool wdmParse(const char *text, double &v)   { return sscanf(text, "%lf", &v)==1; }

template<> inline size_t wdmStringnize(char *text, size_t len, int8_t  v)  { return snprintf(text, len, "%i", (int32_t)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, int16_t v)  { return snprintf(text, len, "%i", (int32_t)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, int32_t v)  { return snprintf(text, len, "%i", v); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint8_t  v) { return snprintf(text, len, "%u", (uint32_t)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint16_t v) { return snprintf(text, len, "%u", (uint32_t)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint32_t v) { return snprintf(text, len, "%u", v); }
template<> inline size_t wdmStringnize(char *text, size_t len, bool v)     { return snprintf(text, len, "%i", (int32_t)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, float v)    { return snprintf(text, len, "%f", v); }
template<> inline size_t wdmStringnize(char *text, size_t len, double v)   { return snprintf(text, len, "%lf", v); }


struct wdmInt32x2 { int v[2]; int& operator[](size_t i){return v[i];} };
struct wdmInt32x3 { int v[3]; int& operator[](size_t i){return v[i];} };
struct wdmInt32x4 { int v[4]; int& operator[](size_t i){return v[i];} };
struct wdmFloat32x2 { float v[2]; float& operator[](size_t i){return v[i];} };
struct wdmFloat32x3 { float v[3]; float& operator[](size_t i){return v[i];} };
struct wdmFloat32x4 { float v[4]; float& operator[](size_t i){return v[i];} };

template<> inline const char* wdmTypename<wdmInt32x2  >() { return "int32x2"; }
template<> inline const char* wdmTypename<wdmInt32x3  >() { return "int32x3"; }
template<> inline const char* wdmTypename<wdmInt32x4  >() { return "int32x4"; }
template<> inline const char* wdmTypename<wdmFloat32x2>() { return "float32x2"; }
template<> inline const char* wdmTypename<wdmFloat32x3>() { return "float32x3"; }
template<> inline const char* wdmTypename<wdmFloat32x4>() { return "float32x4"; }
template<> inline bool wdmParse(const char *text, wdmInt32x2   &v) { return sscanf(text, "[%d,%d]", &v[0],&v[1])==2; }
template<> inline bool wdmParse(const char *text, wdmInt32x3   &v) { return sscanf(text, "[%d,%d,%d]", &v[0],&v[1],&v[2])==3; }
template<> inline bool wdmParse(const char *text, wdmInt32x4   &v) { return sscanf(text, "[%d,%d,%d,%d]", &v[0],&v[1],&v[2],&v[3])==4; }
template<> inline bool wdmParse(const char *text, wdmFloat32x2 &v) { return sscanf(text, "[%f,%f]", &v[0],&v[1])==2; }
template<> inline bool wdmParse(const char *text, wdmFloat32x3 &v) { return sscanf(text, "[%f,%f,%f]", &v[0],&v[1],&v[2])==3; }
template<> inline bool wdmParse(const char *text, wdmFloat32x4 &v) { return sscanf(text, "[%f,%f,%f,%f]", &v[0],&v[1],&v[2],&v[3])==4; }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmInt32x2   v) { return snprintf(text, len, "[%d,%d]", v[0],v[1]); }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmInt32x3   v) { return snprintf(text, len, "[%d,%d,%d]", v[0],v[1],v[2]); }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmInt32x4   v) { return snprintf(text, len, "[%d,%d,%d,%d]", v[0],v[1],v[2],v[3]); }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmFloat32x2 v) { return snprintf(text, len, "[%f,%f]", v[0],v[1]); }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmFloat32x3 v) { return snprintf(text, len, "[%f,%f,%f]", v[0],v[1],v[2]); }
template<> inline size_t wdmStringnize(char *text, size_t len, wdmFloat32x4 v) { return snprintf(text, len, "[%f,%f,%f,%f]", v[0],v[1],v[2],v[3]); }

// SSE
#ifdef _INCLUDED_MM2
template<> inline const char* wdmTypename<__m128i>() { return wdmTypename<wdmInt32x4  >(); }
template<> inline const char* wdmTypename<__m128>()  { return wdmTypename<wdmFloat32x4>(); }
template<> inline bool wdmParse(const char *text, __m128i &v) { return wdmParse<wdmInt32x4  >(text, (wdmInt32x4&)v); }
template<> inline bool wdmParse(const char *text, __m128 &v)  { return wdmParse<wdmFloat32x4>(text, (wdmFloat32x4&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, __m128i v) { return wdmStringnize<wdmInt32x4  >(text, len, (const wdmInt32x4&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, __m128 v)  { return wdmStringnize<wdmFloat32x4>(text, len, (const wdmFloat32x4&)v); }
#endif // _INCLUDED_MM2

// glm
#ifdef glm_glm
template<> inline const char* wdmTypename<glm::ivec2>() { return wdmTypename<wdmInt32x2>(); }
template<> inline const char* wdmTypename<glm::ivec3>() { return wdmTypename<wdmInt32x3>(); }
template<> inline const char* wdmTypename<glm::ivec4>() { return wdmTypename<wdmInt32x4>(); }
template<> inline const char* wdmTypename<glm::vec2 >() { return wdmTypename<wdmFloat32x2>(); }
template<> inline const char* wdmTypename<glm::vec3 >() { return wdmTypename<wdmFloat32x3>(); }
template<> inline const char* wdmTypename<glm::vec4 >() { return wdmTypename<wdmFloat32x4>(); }
template<> inline bool wdmParse(const char *text, glm::ivec2 &v) { return wdmParse<wdmInt32x2  >(text, (wdmInt32x2&)v); }
template<> inline bool wdmParse(const char *text, glm::ivec3 &v) { return wdmParse<wdmInt32x3  >(text, (wdmInt32x3&)v); }
template<> inline bool wdmParse(const char *text, glm::ivec4 &v) { return wdmParse<wdmInt32x4  >(text, (wdmInt32x4&)v); }
template<> inline bool wdmParse(const char *text, glm::vec2  &v) { return wdmParse<wdmFloat32x2>(text, (wdmFloat32x2&)v); }
template<> inline bool wdmParse(const char *text, glm::vec3  &v) { return wdmParse<wdmFloat32x3>(text, (wdmFloat32x3&)v); }
template<> inline bool wdmParse(const char *text, glm::vec4  &v) { return wdmParse<wdmFloat32x4>(text, (wdmFloat32x4&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::ivec2 v) { return wdmStringnize<wdmInt32x2  >(text, len, (const wdmInt32x2&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::ivec3 v) { return wdmStringnize<wdmInt32x3  >(text, len, (const wdmInt32x3&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::ivec4 v) { return wdmStringnize<wdmInt32x4  >(text, len, (const wdmInt32x4&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::vec2  v) { return wdmStringnize<wdmFloat32x2>(text, len, (const wdmFloat32x2&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::vec3  v) { return wdmStringnize<wdmFloat32x3>(text, len, (const wdmFloat32x3&)v); }
template<> inline size_t wdmStringnize(char *text, size_t len, glm::vec4  v) { return wdmStringnize<wdmFloat32x4>(text, len, (const wdmFloat32x4&)v); }
#endif // glm_glm



class wdmNodeBase : public wdmNode
{
protected:
    virtual ~wdmNodeBase()
    {
        _wdmUnregisterNode(this);
        for(size_t i=0; i<m_children.size(); ++i) { m_children[i]->release(); }
    }

public:
    typedef std::function<void ()>  exec_t;

    struct wdmEqualName
    {
        const char *m_name;
        size_t m_len;
        wdmEqualName(const char *name, size_t len) : m_name(name), m_len(len) {}
        bool operator()(const wdmNode *a) const
        {
            const char *name = a->getName();
            size_t len = strlen(name);
            return len==m_len && strncmp(name, m_name, len)==0;
        }
    };
    static inline size_t wdmFindSeparator(const char *s)
    {
        size_t i=0;
        for(;; ++i) {
            if(s[i]=='/' || s[i]=='\0') { break; }
        }
        return i;
    }
    typedef std::vector<wdmNode*> node_cont;

    wdmNodeBase()
        : m_id(_wdmGenerateID())
    {
        _wdmRegisterNode(this);
    }

    virtual void release() { delete this; }

    virtual wdmID       getID() const               { return m_id; }
    virtual const char* getName() const             { return m_name.c_str(); }
    virtual size_t      getNumChildren() const      { return m_children.size(); }
    virtual wdmNode*    getChild(size_t i) const    { return m_children[i]; }

    virtual wdmNode* findChild(const char *path) const
    {
        size_t s = wdmFindSeparator(path);
        node_cont::const_iterator i=std::find_if(m_children.begin(), m_children.end(), wdmEqualName(path, s));
        wdmNode *c = i==m_children.end() ? NULL : *i;
        return path[s]=='/' ? c->findChild(path+s+1) : c;
    }

    virtual void addChild(const char *path, wdmNode *child)
    {
        size_t s = wdmFindSeparator(path);
        node_cont::const_iterator i=std::find_if(m_children.begin(), m_children.end(), wdmEqualName(path, s));
        wdmNode *n = i==m_children.end() ? NULL : *i;
        if(path[s]=='/') {
            if(n==NULL) {
                n = new wdmNodeBase();
                n->setName(path, s);
                m_children.push_back(n);
            }
            n->addChild(path+s+1, child);
        }
        else {
            // 同名ノードがある場合、古いのは削除
            if(n!=NULL) {
                n->release();
                m_children.erase(i);
            }
            child->setName(path);
            m_children.push_back(child);
        }
    }

    virtual void    eraseChild(wdmNode *child)  { m_children.erase(std::find(m_children.begin(), m_children.end(), child)); }
    virtual void    setName(const char *name, size_t len=0)
    {
        m_name = len!=0 ? wdmString(name, len) : name;
    }

    size_t jsonizeChildren(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "\"children\": [");
        for(size_t i=0; i<getNumChildren(); ++i) {
            s += getChild(i)->jsonize(out+s, len-s);
            if(i+1!=getNumChildren()) { s += snprintf(out+s, len-s, ", "); }
        }
        s += snprintf(out+s, len-s, "]");
        return s;
    }

    virtual size_t  jsonize(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "{\"id\":%d, \"name\":\"%s\", ", getID(), getName());
        s += jsonizeChildren(out+s, len-s);
        s += snprintf(out+s, len-s, "}");
        return s;
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        return false;
    }

private:
    node_cont m_children;
    wdmID m_id;
    wdmString m_name;
};


template<class T>
class wdmDataNode : public wdmNodeBase
{
typedef wdmNodeBase super;
public:
    typedef std::function<void ()>  command_t;

    wdmDataNode(T *value) : m_value(value) {}

    virtual size_t jsonize(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "{\"id\":%d, \"name\":\"%s\", \"type\":\"%s\", \"value\":", getID(), getName(), wdmTypename<T>());
        s += wdmStringnize(out+s, len-s, *m_value);
        s += snprintf(out+s, len-s, ", ");
        s += jsonizeChildren(out+s, len-s);
        s += snprintf(out+s, len-s, "}");
        return s;
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        if(strncmp(evt.command, "set(", 4)==0) {
            T tmp;
            if(wdmParse(evt.command+4, tmp)) {
                *m_value = tmp;
                return true;
            }
        }
        return super::handleEvent(evt);
    }

private:
    T *m_value;
};


template<class T>
class wdmPropertyNode : public wdmNodeBase
{
typedef wdmNodeBase super;
public:
    typedef std::function<T ()>     getter_t;
    typedef std::function<void (T)> setter_t;

    wdmPropertyNode(getter_t getter, setter_t setter)
        : m_getter(getter)
        , m_setter(setter)
    {}

    virtual size_t jsonize(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "{\"id\":%d, \"name\":\"%s\", \"type\":\"%s\", \"value\":", getID(), getName(), wdmTypename<T>());
        s += wdmStringnize(out+s, len-s, m_getter());
        s += snprintf(out+s, len-s, ", ");
        s += jsonizeChildren(out+s, len-s);
        s += snprintf(out+s, len-s, "}");
        return s;
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        if(strncmp(evt.command, "set(", 4)==0) {
            T tmp;
            if(wdmParse(evt.command+4, tmp)) {
                m_setter(tmp);
                return true;
            }
        }
        return super::handleEvent(evt);
    }

private:
    getter_t m_getter;
    setter_t m_setter;
};

template<class R>
class wdmFunctionNode0 : public wdmNodeBase
{
typedef wdmNodeBase super;
public:
    typedef std::function<R ()>  func_t;

    wdmFunctionNode0(func_t func)
        : m_func(func)
    {}

    virtual size_t jsonize(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "{\"id\":%d, \"name\":\"%s\", \"callable\":true, \"arg_types\":[],", getID(), getName() );
        s += jsonizeChildren(out+s, len-s);
        s += snprintf(out+s, len-s, "}");
        return s;
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        if(strncmp(evt.command, "call(", 5)==0) {
            if(m_func) {
                m_func();
                return true;
            }
        }
        return super::handleEvent(evt);
    }

private:
    func_t m_func;
};

template<class R, class A0>
class wdmFunctionNode1 : public wdmNodeBase
{
    typedef wdmNodeBase super;
public:
    typedef std::function<R (A0)>  func_t;

    wdmFunctionNode1(func_t func)
        : m_func(func)
    {}

    virtual size_t jsonize(char *out, size_t len) const
    {
        size_t s = 0;
        s += snprintf(out+s, len-s, "{\"id\":%d, \"name\":\"%s\", \"callable\":true, \"arg_types\":[\"%s\"],", getID(), getName(), wdmTypename<A0>() );
        s += jsonizeChildren(out+s, len-s);
        s += snprintf(out+s, len-s, "}");
        return s;
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        if(strncmp(evt.command, "call(", 5)==0) {
            A0 a0;
            if(m_func && wdmParse(evt.command+5, a0)) {
                m_func(a0);
                return true;
            }
        }
        return super::handleEvent(evt);
    }

private:
    func_t m_func;
};



inline wdmString wdmFormat(const char *fmt, ...)
{
    char buf[1024];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    return buf;
}

#define wdmScope(...) __VA_ARGS__

template<class T>
inline void wdmAddNode(const wdmString &path, T *value)
{
    auto *n = new wdmDataNode<T>(value);
    _wdmGetRootNode()->addChild(path.c_str(), n);
}

template<class Class, class Getter, class Setter>
inline void wdmAddNode(const wdmString &path, Class *_this, Getter getter, Setter setter)
{
    typedef decltype( (_this->*getter)() ) T;
    auto *n = new wdmPropertyNode<T>(std::bind(getter, _this), std::bind(setter, _this, std::placeholders::_1));
    _wdmGetRootNode()->addChild(path.c_str(), n);
}

template<class R>
inline void wdmAddNode(const wdmString &path, R (*f)())
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode0<R>(std::function<R ()>(f)) );
}
template<class R, class C>
inline void wdmAddNode(const wdmString &path, R (C::*mf)(), C *_this)
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode0<R>(std::bind(mf, _this)) );
}
template<class R, class C>
inline void wdmAddNode(const wdmString &path, R (C::*cmf)() const, const C *_this)
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode0<R>(std::bind(cmf, _this)) );
}
template<class R, class A0>
inline void wdmAddNode(const wdmString &path, R (*f)(A0))
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode1<R,A0>(std::bind(f, std::placeholders::_1)) );
}
template<class R, class C, class A0>
inline void wdmAddNode(const wdmString &path, R (C::*mf)(A0), C *_this)
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode1<R,A0>(std::bind(mf, _this, std::placeholders::_1)) );
}
template<class R, class C, class A0>
inline void wdmAddNode(const wdmString &path, R (C::*cmf)(A0) const, const C *_this)
{
    _wdmGetRootNode()->addChild( path.c_str(), new wdmFunctionNode1<R,A0>(std::bind(cmf, _this, std::placeholders::_1)) );
}

inline void wdmEraseNode(const wdmString &path)
{
    wdmNode *n = _wdmGetRootNode()->findChild(path.c_str());
    if(n!=NULL) { n->release(); }
}

#else // wdmDisable

#define wdmInitialize(...)
#define wdmFinalize(...)
#define wdmFlush(...)
#define wdmScope(...)
#define wdmAddNode(...)
#define wdmAddFunctionNode(...)
#define wdmEraseNode(...)

#endif // wdmDisable
#endif // WebDebugMenu_h
