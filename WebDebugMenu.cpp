#include <windows.h>
#include "WebDebugMenu.h"

#define wdmEachBuiltinTypes(Template)\
    Template(int8_t)\
    Template(int16_t)\
    Template(int32_t)\
    Template(uint8_t)\
    Template(uint16_t)\
    Template(uint32_t)\
    Template(bool)\
    Template(float)\
    Template(double)

#define Template(T)\
    template wdmDataNode<T>;\
    template wdmPropertyNode<T>;
wdmEachBuiltinTypes(Template)
#undef Template



// std::string が dll を跨ぐと問題が起きる可能性があるため、
// std::string を保持して dll を跨がない wdmEventData と、const char* だけ保持して dll 跨ぐ wdmEvent に分ける
struct wdmEventData
{
    wdmID node;
    wdmString command;

    wdmEvent toEvent() const
    {
        wdmEvent tmp = {node, command.c_str()};
        return tmp;
    }
};

class wdmSystem
{
public:
    typedef std::map<wdmID, wdmNode*> node_cont;
    typedef std::vector<wdmEventData> event_cont;

    static void         createInstance();
    static void         releaseInstance();
    static wdmSystem*   getInstance();

    wdmID       generateID();
    wdmNode*    findNode(const char *path);
    wdmNode*    createNode(const char *path);
    void        registerNode(wdmNode *node);
    void        unregisterNode(wdmNode *node);
    void        addEvent(const wdmEventData &e);
    void        flushEvent();

private:
    node_cont m_nodes;
    event_cont m_events;
    wdmNode *m_root;
};

extern "C" {
    wdmIntermodule void     wdmInitialize() { wdmSystem::createInstance(); }
    wdmIntermodule void     wdmFinalize()   { wdmSystem::releaseInstance(); }
    wdmIntermodule void     wdmFlush()      { wdmSystem::getInstance()->flushEvent(); }

    wdmIntermodule wdmID    _wdmGenerateID()                 { return wdmSystem::getInstance()->generateID(); }
    wdmIntermodule wdmNode* _wdmFindNode(const char *path)   { return wdmSystem::getInstance()->findNode(path); }
    wdmIntermodule wdmNode* _wdmCreateNode(const char *path) { return wdmSystem::getInstance()->createNode(path); }
    wdmIntermodule void     _wdmRegisterNode(wdmNode *node)  { wdmSystem::getInstance()->registerNode(node); }
    wdmIntermodule void     _wdmUnregisterNode(wdmNode *node){ wdmSystem::getInstance()->unregisterNode(node); }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason==DLL_PROCESS_ATTACH) {
        wdmInitialize();
    }
    else if(fdwReason==DLL_PROCESS_DETACH) {
        wdmFinalize();
    }
    return TRUE;
}
