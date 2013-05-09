#ifndef WebDebugMenu_h
#define WebDebugMenu_h

#ifndef wdmDisable
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
    virtual void        eraseChild(wdmNode *child)=0;
    virtual void        setName(const char *name, size_t len=0)=0;
    virtual void        setParent(wdmNode *parent)=0;
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

template<> inline bool wdmParse(const char *text, int8_t  &value)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, int16_t &value)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, int32_t &value)  { int32_t t;  if(sscanf(text, "%i", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint8_t  &value) { uint32_t t; if(sscanf(text, "%u", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint16_t &value) { uint32_t t; if(sscanf(text, "%u", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, uint32_t &value) { uint32_t t; if(sscanf(text, "%u", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, bool &value)     { int32_t t;  if(sscanf(text, "%i", &t)==1){ value=t; return true; } return false; }
template<> inline bool wdmParse(const char *text, float &value)    { return sscanf(text, "%f", &value)==1; }
template<> inline bool wdmParse(const char *text, double &value)   { return sscanf(text, "%lf", &value)==1; }

template<> inline size_t wdmStringnize(char *text, size_t len, int8_t  value)  { return snprintf(text, len, "%hhi", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, int16_t value)  { return snprintf(text, len, "%hi", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, int32_t value)  { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint8_t  value) { return snprintf(text, len, "%hhu", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint16_t value) { return snprintf(text, len, "%hu", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint32_t value) { return snprintf(text, len, "%u", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, bool value)     { return snprintf(text, len, "%hhi", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, float value)    { return snprintf(text, len, "%f", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, double value)   { return snprintf(text, len, "%lf", value); }




class wdmNodeBase : public wdmNode
{
protected:
    virtual ~wdmNodeBase()
    {
        _wdmUnregisterNode(this);
        if(m_parent) { m_parent->eraseChild(this); }
        while(!m_children.empty()) { m_children.front()->release(); }
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
        : m_parent(NULL)
        , m_id(_wdmGenerateID())
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
                n->setParent(this);
                m_children.push_back(n);
            }
            n->addChild(path+s+1, child);
        }
        else {
            // 同名ノードがある場合、古いのは削除
            if(n!=NULL) { n->release(); }
            child->setName(path);
            child->setParent(this);
            m_children.push_back(child);
        }
    }

    virtual void    eraseChild(wdmNode *child)  { m_children.erase(std::find(m_children.begin(), m_children.end(), child)); }
    virtual void    setName(const char *name, size_t len=0)
    {
        m_name = len!=0 ? wdmString(name, len) : name;
    }
    virtual void    setParent(wdmNode *parent)  { m_parent=parent; }

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
        if(strncmp(evt.command, "exec()", 6)==0) {
            if(m_exec) {
                m_exec();
                return true;
            }
        }
        return false;
    }

    void setCommand(exec_t exec) { m_exec=exec; }

private:
    node_cont m_children;
    wdmNode *m_parent;
    wdmID m_id;
    wdmString m_name;
    exec_t   m_exec;
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




inline wdmString wdmFormat(const char *fmt, ...)
{
    char buf[1024];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);
    return buf;
}

template<class T>
inline void wdmAddDataNode(const wdmString &path, T *value)
{
    auto *n = new wdmDataNode<T>(value);
    _wdmGetRootNode()->addChild(path.c_str(), n);
}

template<class T, class C, class Getter, class Setter>
inline void _wdmAddPropertyNode(const wdmString &path, C *_this, Getter getter, Setter setter)
{
    auto *n = new wdmPropertyNode<T>(std::bind(getter, _this), std::bind(setter, _this, std::placeholders::_1));
    _wdmGetRootNode()->addChild(path.c_str(), n);
};
#define wdmAddPropertyNode(Path, This, Getter, Setter, Type) _wdmAddPropertyNode<Type>(Path, This, Getter, Setter)

inline void wdmEraseNode(const wdmString &path)
{
    wdmNode *n = _wdmGetRootNode()->findChild(path.c_str());
    if(n!=NULL) { n->release(); }
};

#else // wdmDisable

#define wdmInitialize(...)
#define wdmFinalize(...)
#define wdmFlush(...)
#define wdmAddDataNode(...)
#define wdmAddPropertyNode(...)
#define wdmEraseNode(...)

#endif // wdmDisable
#endif // WebDebugMenu_h
