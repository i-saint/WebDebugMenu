#include <Poco/Mutex.h>
#include <Poco/AtomicCounter.h>
#include <Poco/File.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "WebDebugMenu.h"

#pragma comment(lib, "psapi.lib")
#include <psapi.h>
#include <regex>


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

struct wdmServerConfig
{
    uint16_t port;
    uint16_t max_queue;
    uint16_t max_threads;

    wdmServerConfig()
        : port(10002)
        , max_queue(100)
        , max_threads(4)
    {
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

    wdmSystem();
    ~wdmSystem();
    wdmID       generateID();
    wdmNode*    getRootNode() const;
    void        registerNode(wdmNode *node);
    void        unregisterNode(wdmNode *node);
    void        addEvent(const wdmEventData &e);
    void        flushEvent();

    wdmString&  createJSONData(const wdmID *nodes, size_t num_nodes);

private:
    static wdmSystem *s_inst;
    node_cont m_nodes;
    event_cont m_events;
    wdmNode *m_root;
    bool m_end_flag;
    Poco::AtomicCounter m_idgen;
    Poco::Mutex m_mutex;
    wdmString m_json;

    wdmServerConfig m_conf;
    Poco::Net::HTTPServer *m_server;
};


const char s_root_dir[] = "wdmroot";

struct MIME { const char *ext; const char *type; };
static const MIME s_mime_types[] = {
    {".txt",  "text/plain"},
    {".html", "text/html"},
    {".css",  "text/css"},
    {".js",   "text/javascript"},
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
};

inline size_t GetModulePath(char *out_path, size_t len)
{
    HMODULE mod = 0;
    ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&GetModulePath, &mod);
    DWORD size = ::GetModuleFileNameA(mod, out_path, len);
    return size;
}

inline bool GetModuleDirectory(char *out_path, size_t len)
{
    size_t size = GetModulePath(out_path, len);
    while(size>0) {
        if(out_path[size]=='\\') {
            out_path[size+1] = '\0';
            return true;
        }
        --size;
    }
    return false;
}

static const char* GetCurrentModuleDirectory()
{
    static char s_path[MAX_PATH] = {0};
    if(s_path[0]=='\0') {
        GetModuleDirectory(s_path, MAX_PATH);
    }
    return s_path;
}


class wdmFileRequestHandler: public Poco::Net::HTTPRequestHandler
{
public:
    wdmFileRequestHandler(const std::string &path)
        : m_path(path)
    {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
    {
        const char *ext = s_mime_types[0].ext;
        const char *mime = s_mime_types[0].type;
        size_t epos = m_path.find_last_of(".");
        if(epos!=std::string::npos) {
            ext = &m_path[epos];
            for(size_t i=0; i<_countof(s_mime_types); ++i) {
                if(strcmp(ext, s_mime_types[i].ext)==0) {
                    mime = s_mime_types[i].type;
                }
            }
        }
        response.sendFile(m_path, mime);
    }

private:
    std::string m_path;
};


template<class F>
void EachInputValue(Poco::Net::HTTPServerRequest &request, const F &f)
{
    if(!request.hasContentLength() || request.getContentLength()>1024*4) {
        return;
    }
    size_t size = (size_t)request.getContentLength();
    std::istream& stream = request.stream();
    std::string encoded_content;
    std::string content;
    encoded_content.resize(size);
    stream.read(&encoded_content[0], size);
    Poco::URI::decode(encoded_content, content);

    std::regex reg("(\\d+)->([^;]+)");
    std::cmatch m;
    size_t pos = 0;
    for(;;) {
        if(std::regex_search(content.c_str()+pos, m, reg)) {
            f(m[1].str().c_str(), m[2].str().c_str());
            pos += m.position()+m.length();
        }
        else {
            break;
        }
    }
}

template<class F>
void EachNodeValue(Poco::Net::HTTPServerRequest &request, const F &f)
{
    if(!request.hasContentLength() || request.getContentLength()>1024*4) {
        return;
    }
    size_t size = (size_t)request.getContentLength();
    std::istream& stream = request.stream();
    std::string encoded_content;
    std::string content;
    encoded_content.resize(size);
    stream.read(&encoded_content[0], size);
    Poco::URI::decode(encoded_content, content);

    std::regex reg("(\\d+)");
    std::cmatch m;
    size_t pos = 0;
    for(;;) {
        if(std::regex_search(content.c_str()+pos, m, reg)) {
            f(m[1].str().c_str());
            pos += m.position()+m.length();
        }
        else {
            break;
        }
    }
}

class wdmCommandHandler : public Poco::Net::HTTPRequestHandler
{
public:
    wdmCommandHandler()
    {
    }

    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
    {
        if(request.getURI()=="/command") {
            EachInputValue(request, [&](const char *id, const char *command){
                wdmEventData tmp = {std::atoi(id), command};
                wdmSystem::getInstance()->addEvent(tmp);
            });
            response.setContentType("text/plain");
            response.setContentLength(2);
            std::ostream &ostr = response.send();
            ostr.write("ok", 3);
        }
        else if(request.getURI()=="/data") {
            std::vector<wdmID> nodes;
            EachNodeValue(request, [&](const char *id){
                nodes.push_back(std::atoi(id));
            });

            const wdmString &json = wdmSystem::getInstance()->createJSONData(nodes.empty() ? NULL : &nodes[0], nodes.size());
            response.setContentType("application/json");
            response.setContentLength(json.size());
            std::ostream &ostr = response.send();
            ostr.write(&json[0], json.size());
        }

    }
};

class wdmRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    virtual Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request)
    {
        if(request.getURI() == "/") {
            return new wdmFileRequestHandler(std::string(GetCurrentModuleDirectory())+std::string(s_root_dir)+"/index.html");
        }
        else if(request.getURI()=="/command" || request.getURI()=="/data") {
            return new wdmCommandHandler();
        }
        else {
            std::string path = std::string(GetCurrentModuleDirectory())+std::string(s_root_dir)+request.getURI();
            Poco::File file(path);
            if(file.exists()) {
                return new wdmFileRequestHandler(path);
            }
            else {
                return 0;
            }
        }
    }
};



wdmSystem* wdmSystem::s_inst;

void wdmSystem::createInstance()
{
    if(s_inst==NULL) {
        new wdmSystem();
    }
}

void wdmSystem::releaseInstance()
{
    delete s_inst;
    s_inst = NULL;
}

wdmSystem* wdmSystem::getInstance()
{
    return s_inst;
}


wdmSystem::wdmSystem()
    : m_end_flag(false)
    , m_root(NULL)
    , m_server(NULL)
{
    s_inst = this;
    m_root = new wdmNodeBase();
    m_json.resize(1024*1024*16);

    if(!m_server) {
        Poco::Net::HTTPServerParams* params = new Poco::Net::HTTPServerParams;
        params->setMaxQueued(m_conf.max_queue);
        params->setMaxThreads(m_conf.max_threads);
        params->setThreadIdleTime(Poco::Timespan(3, 0));

        try {
            Poco::Net::ServerSocket svs(m_conf.port);
            m_server = new Poco::Net::HTTPServer(new wdmRequestHandlerFactory(), svs, params);
            m_server->start();
        }
        catch(Poco::IOException &) {
        }
    }
}

wdmSystem::~wdmSystem()
{
    if(m_server) {
        m_end_flag = true;
        m_server->stopAll(false);
        while(m_server->currentConnections()>0 || m_server->currentThreads()>0) {
            ::Sleep(1);
        }
        delete m_server;
        m_server = NULL;
    }

    m_root->release();
    m_root = NULL;
    m_nodes.clear();
}

wdmID wdmSystem::generateID()
{
    return ++m_idgen;
}

wdmNode* wdmSystem::getRootNode() const
{
    return m_root;
}

void wdmSystem::registerNode( wdmNode *node )
{
    if(node!=NULL) {
        Poco::Mutex::ScopedLock lock(m_mutex);
        m_nodes[node->getID()] = node;
    }
}

void wdmSystem::unregisterNode( wdmNode *node )
{
    if(node!=NULL) {
        Poco::Mutex::ScopedLock lock(m_mutex);
        m_nodes.erase(node->getID());
    }
}

void wdmSystem::addEvent( const wdmEventData &e )
{
    Poco::Mutex::ScopedLock lock(m_mutex);
    m_events.push_back(e);
}

void wdmSystem::flushEvent()
{
    Poco::Mutex::ScopedLock lock(m_mutex);
    for(event_cont::iterator ei=m_events.begin(); ei!=m_events.end(); ++ei) {
        const wdmEventData e = *ei;
        node_cont::iterator ni = m_nodes.find(e.node);
        if(ni!=m_nodes.end()) {
            ni->second->handleEvent(e.toEvent());
        }
    }
    m_events.clear();
}

wdmString& wdmSystem::createJSONData(const wdmID *nodes, size_t num_nodes)
{
    Poco::Mutex::ScopedLock lock(m_mutex);
    m_json.resize(m_json.capacity());
    size_t s = 0;
    for(;;) {
        s += snprintf(&m_json[0]+s, m_json.size()-s, "[");
        if(num_nodes==0) {
            s += m_root->jsonize(&m_json[0]+s, m_json.size()-s, 1);
        }
        else {
            bool  first = true;
            for(size_t i=0; i<num_nodes; ++i) {
                node_cont::iterator p = m_nodes.find(nodes[i]);
                if(p!=m_nodes.end()) {
                    if(!first) { s += snprintf(&m_json[0]+s, m_json.size()-s, ", "); }
                    s += p->second->jsonize(&m_json[0]+s, m_json.size()-s, 1);
                }
                first = false;
            }
        }
        s += snprintf(&m_json[0]+s, m_json.size()-s, "]");

        if(s==m_json.size()) {
            m_json.resize(m_json.size()*2);
        }
        else {
            break;
        }
    }
    m_json.resize(s);
    return m_json;
}


extern "C" {
    wdmIntermodule void     wdmInitialize() { wdmSystem::createInstance(); }
    wdmIntermodule void     wdmFinalize()   { wdmSystem::releaseInstance(); }
    wdmIntermodule void     wdmFlush()      { wdmSystem::getInstance()->flushEvent(); }

    wdmIntermodule wdmID    _wdmGenerateID()                            { return wdmSystem::getInstance()->generateID(); }
    wdmIntermodule wdmNode* _wdmGetRootNode()                           { return wdmSystem::getInstance()->getRootNode(); }
    wdmIntermodule void     _wdmRegisterNode(wdmNode *node)             { wdmSystem::getInstance()->registerNode(node); }
    wdmIntermodule void     _wdmUnregisterNode(wdmNode *node)           { wdmSystem::getInstance()->unregisterNode(node); }
};



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason==DLL_PROCESS_ATTACH) {
    }
    else if(fdwReason==DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
