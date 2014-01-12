#include "WebDebugMenu.h"

#ifndef wdmDisableEnumMemberVariables
#define _NO_CVCONST_H
#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace {

enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

struct EMVContext
{
    HANDLE hprocess;
    ULONG64 modbase;
    std::string this_type;
    std::string current_class;
    std::string current_type;
    std::string current_value;
    std::string tmp_name;
    wdmMemberInfo mi;

    EMVContext()
        : hprocess(::GetCurrentProcess())
        , modbase((ULONG64)::GetModuleHandleA(nullptr))
    {
        this_type.reserve(MAX_SYM_NAME);
        current_class.reserve(MAX_SYM_NAME);
        current_type.reserve(MAX_SYM_NAME);
        current_value.reserve(MAX_SYM_NAME);
        tmp_name.reserve(MAX_SYM_NAME);
        memset(&mi, 0, sizeof(mi));
    }

    void updateMemberInfo()
    {
        mi.this_type  = this_type.c_str();
        mi.class_name = current_class.c_str();
        mi.type_name  = current_type.c_str();
        mi.value_name = current_value.c_str();
    }
};

bool GetSymbolName(EMVContext &ctx, DWORD t)
{
    WCHAR *wname = NULL;
    if(::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_SYMNAME, &wname )) {
        size_t num = 0;
        char out[MAX_SYM_NAME];
        ::wcstombs_s(&num, out, wname, _countof(out));
        ctx.tmp_name = out;
        ::LocalFree(wname);
        return true;
    }
    return false;
}

bool GetSymbolTypeNameImpl(EMVContext &ctx, DWORD t, std::string &ret)
{
    DWORD tag = 0;
    DWORD basetype = 0;
    if(!::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_SYMTAG, &tag)) {
        return false;
    }

    if(tag==SymTagArrayType) {
        DWORD count = 0;
        ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_COUNT, &count);
        char a[128];
        sprintf(a, "[%d]", count);
        ret += a;

        DWORD tid = 0;
        ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_TYPEID, &tid);
        return GetSymbolTypeNameImpl(ctx, tid, ret);
    }
    else if(tag==SymTagPointerType) {
        ret = "*"+ret;

        DWORD tid = 0;
        ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_TYPEID, &tid);
        return GetSymbolTypeNameImpl(ctx, tid, ret);
    }
    else if(tag==SymTagBaseType) {
        ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_BASETYPE, &basetype);
        ULONG64 length = 0;
        ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_LENGTH, &length);
        std::string type;
        switch(basetype) {
        case btChar:  type="char"; break;
        case btWChar: type="wchar"; break;
        case btBool:  type="bool"; break;
        case btInt:   type="int"; break;
        case btUInt:  type="uint"; break;
        case btFloat: type="float"; break;
        }
        switch(basetype) {
        case btInt:
        case btUInt:
        case btFloat:
            char bits[32];
            sprintf(bits, "%d", length*8);
            type+=bits;
            break;
        }
        ret = type+ret;
    }
    else { // user defined type
        WCHAR *wname = nullptr;
        if(::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_SYMNAME, &wname )) {
            char name[MAX_SYM_NAME];
            size_t num = 0;
            ::wcstombs_s(&num, name, wname, _countof(name));
            ::LocalFree(wname);
            ret+=name;
        }
    }
    return true;
}
bool GetSymbolTypeName(EMVContext &ctx, DWORD t)
{
    ctx.tmp_name.clear();
    return GetSymbolTypeNameImpl(ctx, t, ctx.tmp_name);
}

void EnumMemberVariables(EMVContext &ctx, DWORD t, const wdmMemberInfoCallback &f)
{
    DWORD tag = 0;
    if(!::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_SYMTAG, &tag)) {
        return;
    }

    if(tag==SymTagData) {
        DWORD offset = 0;
        DWORD tid = 0;
        if( ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_OFFSET, &offset) &&
            ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_TYPEID, &tid) )
        {
            ULONG64 length = 0;
            ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, tid, TI_GET_LENGTH, &length);
            ctx.mi.value = (void*)((size_t)ctx.mi.base_pointer+offset);
            ctx.mi.size = (size_t)length;
            GetSymbolTypeName(ctx, tid);
            ctx.current_type = ctx.tmp_name;
            GetSymbolName(ctx, t);
            ctx.current_value = ctx.tmp_name;
            ctx.updateMemberInfo();
            f(ctx.mi);
        }
    }
    else if(tag==SymTagBaseClass) {
        void *base_prev = ctx.mi.base_pointer;
        DWORD offset = 0;
        DWORD type = 0;
        if( ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_OFFSET, &offset) &&
            ::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_TYPE, &type) )
        {
            ctx.mi.base_pointer = (void*)((size_t)base_prev+offset);
            EnumMemberVariables(ctx, type, f);
            ctx.mi.base_pointer = base_prev;
        }
    }
    else if(tag==SymTagUDT) {
        std::string prev = ctx.current_class;
        GetSymbolName(ctx, t);
        ctx.current_class = ctx.tmp_name;
        DWORD num_members = 0;
        if(::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_GET_CHILDRENCOUNT, &num_members)) {
            TI_FINDCHILDREN_PARAMS *params = (TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS ) + (sizeof(ULONG)*num_members));
            params->Count = num_members;
            params->Start = 0;
            if(::SymGetTypeInfo(ctx.hprocess, ctx.modbase, t, TI_FINDCHILDREN, params )) {
                for(DWORD i=0; i<num_members; ++i) {
                    EnumMemberVariables(ctx, params->ChildId[i], f);
                }
            }
            free(params);
        }
        ctx.current_class = prev;
    }
}

bool EnumMemberVariablesImpl(EMVContext &ctx, const wdmMemberInfoCallback &f)
{
    ULONG tindex = 0;
    bool ok = false;
    {
        char buf[sizeof(SYMBOL_INFO)+MAX_SYM_NAME];
        PSYMBOL_INFO sinfo = (PSYMBOL_INFO)buf;
        sinfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        sinfo->MaxNameLen = MAX_SYM_NAME;

        if(::SymGetTypeFromName(ctx.hprocess, ctx.modbase, ctx.this_type.c_str(), sinfo)) {
            ok = true;
            tindex = sinfo->TypeIndex;
        }
    }
    if(ok) {
        EnumMemberVariables(ctx, tindex, f);
    }
    return ok;
}

} // namespace


// _this: virtual 関数を持つオブジェクト
wdmAPI bool wdmGetClassName(void *_this, char *out, size_t len)
{
    char buf[sizeof(SYMBOL_INFO)+MAX_SYM_NAME];
    PSYMBOL_INFO sinfo = (PSYMBOL_INFO)buf;
    sinfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    sinfo->MaxNameLen = MAX_SYM_NAME;

    // vftable のシンボル名が "class名::`vftable'" になっているので、そこから class 名を取得
    if(::SymFromAddr(::GetCurrentProcess(), (DWORD64)((void***)_this)[0], nullptr, sinfo)) {
        char vftable[MAX_SYM_NAME];
        ::UnDecorateSymbolName(sinfo->Name, vftable, MAX_SYM_NAME, UNDNAME_NAME_ONLY);
        if(char *colon=strstr(vftable, "::`vftable'")) {
            *colon = '\0';
            strncpy(out, vftable, len);
            return true;
        }
    }
    return false;
}

wdmAPI bool wdmEnumMemberVariablesByTypeName(const char *classname, const wdmMemberInfoCallback &f)
{
    EMVContext ctx;
    ctx.this_type = classname;
    return EnumMemberVariablesImpl(ctx, f);
}

wdmAPI bool wdmEnumMemberVariablesByTypeName(const char *classname, void *_this, const wdmMemberInfoCallback &f)
{
    EMVContext ctx;
    ctx.mi.this_pointer = ctx.mi.base_pointer = _this;
    ctx.this_type = classname;
    return EnumMemberVariablesImpl(ctx, f);
}

wdmAPI bool wdmEnumMemberVariablesByPointer(void *_this, const wdmMemberInfoCallback &f)
{
    EMVContext ctx;
    {
        char class_name[MAX_SYM_NAME];
        if(wdmGetClassName(_this, class_name, MAX_SYM_NAME)) {
            ctx.mi.this_pointer = ctx.mi.base_pointer = _this;
            ctx.this_type = class_name;
        }
    }
    if(!ctx.this_type.empty()) {
        return EnumMemberVariablesImpl(ctx, f);
    }
    return false;
}

#endif // wdmDisableEnumMemberVariables
