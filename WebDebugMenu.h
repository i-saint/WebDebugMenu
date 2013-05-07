#ifndef WebDebugMenu_h
#define WebDebugMenu_h

#ifndef wdmDisable
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <intrin.h>
#include <stdint.h>

#if   defined(wdmDLL_Impl)
#   define wdmIntermodule __declspec(dllexport)
#elif defined(wdmDLL)
#   define wdmIntermodule __declspec(dllimport)
#   pragma comment(lib, "WebDebugMenu.lib")
#else
#   define wdmIntermodule
#endif


typedef uint32_t wdmID;
struct wdmIntermodule wdmEvent
{
    wdmID node;
    const char *command;
};

class wdmIntermodule wdmNode
{
protected:
    virtual             ~wdmNode() {}
public:
    virtual void        release()=0;
    virtual wdmID       getID() const=0;
    virtual const char* getName() const=0;
    virtual size_t      getNumChildren()=0;
    virtual wdmNode*    getChild(size_t i)=0;
    virtual void        addChild(wdmNode *child)=0;
    virtual void        eraseChild(wdmNode *child)=0;
    virtual void        setParent(wdmNode *parent)=0;
    virtual size_t      stringnizeValue(char *out, size_t len) const=0;
    virtual bool        handleEvent(const wdmEvent &evt)=0;
};

extern "C" {
    wdmIntermodule void     wdmInitialize();
    wdmIntermodule void     wdmFinalize();
    wdmIntermodule void     wdmFlush();

    // 以下は内部実装用
    wdmIntermodule wdmID    _wdmGenerateID();
    wdmIntermodule wdmNode* _wdmFindNode(const char *path);
    wdmIntermodule wdmNode* _wdmCreateNode(const char *path);
    wdmIntermodule void     _wdmRegisterNode(wdmNode *node);
    wdmIntermodule void     _wdmUnregisterNode(wdmNode *node);
};



#ifdef _WIN32
#define snprintf    _snprintf
#define vsnprintf   _vsnprintf
#endif // _WIN32

typedef std::string wdmString;
template<class T> inline bool wdmParse(const char *text, T &value);
template<class T> inline size_t wdmStringnize(char *text, size_t len, T value);

template<> inline bool wdmParse(const char *text, int8_t  &value)  { return sscanf(text, "%hhi", &value)==1; }
template<> inline bool wdmParse(const char *text, int16_t &value)  { return sscanf(text, "%hi", &value)==1; }
template<> inline bool wdmParse(const char *text, int32_t &value)  { return sscanf(text, "%i", &value)==1; }
template<> inline bool wdmParse(const char *text, uint8_t  &value) { return sscanf(text, "%hhu", &value)==1; }
template<> inline bool wdmParse(const char *text, uint16_t &value) { return sscanf(text, "%hu", &value)==1; }
template<> inline bool wdmParse(const char *text, uint32_t &value) { return sscanf(text, "%u", &value)==1; }
template<> inline bool wdmParse(const char *text, bool &value)     { return sscanf(text, "%hhi", &value)==1; }
template<> inline bool wdmParse(const char *text, float &value)    { return sscanf(text, "%f", &value)==1; }
template<> inline bool wdmParse(const char *text, double &value)   { return sscanf(text, "%lf", &value)==1; }

template<> inline size_t wdmStringnize(char *text, size_t len, int8_t  value)  { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, int16_t value)  { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, int32_t value)  { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint8_t  value) { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint16_t value) { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, uint32_t value) { return snprintf(text, len, "%i", value); }
template<> inline size_t wdmStringnize(char *text, size_t len, bool value)     { return snprintf(text, len, "%i", value); }
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
    typedef std::vector<wdmNode*> children_cont;

    wdmNodeBase()
        : m_parent(NULL)
        , m_id(_wdmGenerateID())
    {
        _wdmRegisterNode(this);
    }

    virtual void release() { delete this; }

    virtual wdmID       getID() const               { return m_id; }
    virtual const char* getName() const             { return m_name.c_str(); }
    virtual size_t      getNumChildren()            { return m_children.size(); }
    virtual wdmNode*    getChild(size_t i)          { return m_children[i]; }
    virtual void        addChild(wdmNode *child)    { m_children.push_back(child); child->setParent(this); }
    virtual void        eraseChild(wdmNode *child)  { m_children.erase(std::find(m_children.begin(), m_children.end(), child)); }
    virtual void        setParent(wdmNode *parent)  { m_parent=parent; }

    virtual size_t      stringnizeValue(char *out, size_t len) const{ return 0; }
    virtual bool        handleEvent(const wdmEvent &evt)            { return false; }

private:
    children_cont m_children;
    wdmNode *m_parent;
    wdmID m_id;
    wdmString m_name;
};


template<class T>
class wdmDataNode : public wdmNodeBase
{
public:
    typedef std::function<void ()>  command_t;

    wdmDataNode() : m_value(NULL) {}

    virtual size_t stringnizeValue(char *out, size_t len) const
    {
        return wdmStringnize(out, len, *m_value);
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        return false;
    }

private:
    T *m_value;
};


template<class T>
class wdmPropertyNode : public wdmNodeBase
{
public:
    typedef std::function<void ()>  command_t;
    typedef std::function<T ()>     getter_t;
    typedef std::function<void (T)> setter_t;

    wdmPropertyNode() {}

    virtual size_t stringnizeValue(char *out, size_t len) const
    {
        return wdmStringnize(out, len, m_getter());
    }

    virtual bool handleEvent(const wdmEvent &evt)
    {
        return false;
    }

private:
    getter_t m_getter;
    setter_t m_setter;
};




inline wdmString wdmFormat(const char *fmt, ...);

template<class T>
inline wdmNode* wdmAddNode(const wdmString &path, T *value);

template<class T>
inline wdmNode* wdmAddNode(const wdmString &path, std::function<T ()> getter, std::function<void (T)> setter);

inline void wdmEraseNode(const wdmString &path);

#else // wdmDisable

#define wdmInitialize(...)
#define wdmFinalize(...)
#define wdmFlush(...)
#define wdmAddNode(...)
#define wdmEraseNode(...)

#endif // wdmDisable
#endif // WebDebugMenu_h
