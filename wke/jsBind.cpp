//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSFunction.h>
#include <JavaScriptCore/SourceCode.h>
#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/Completion.h>
#include <JavaScriptCore/OpaqueJSString.h>
#include <WebCore/GCController.h>
#include <WebCore/JSDOMWindowCustom.h>
#include <WebCore/Page.h>
#include <WebCore/Frame.h>
#include <WebCore/Chrome.h>
#include <WebCore/ChromeClient.h>

#include "wkeDebug.h"

//cexer: 必须包含在后面，因为其中的 wke.h -> windows.h 会定义 max、min，导致 WebCore 内部的 max、min 出现错乱。
#include "wkeWebView.h"


//////////////////////////////////////////////////////////////////////////




int wkeJSParamCount(wkeJSState* es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    return (int)exec->argumentCount();
}

wkeJSType wkeJSParamType(wkeJSState* es, int argIdx)
{
    return wkeJSTypeOf(wkeJSParam(es, argIdx));
}

wkeJSValue wkeJSParam(wkeJSState* es, int argIdx)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue value = exec->argument(argIdx);
    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSType wkeJSTypeOf(wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);

    if (value.isUndefined())
        return JSTYPE_UNDEFINED;

    if (value.isBoolean())
        return JSTYPE_BOOLEAN;

    if (value.isNumber())
        return JSTYPE_NUMBER;

    if (value.isString())
        return JSTYPE_STRING;

    if (value.isObject()) {
        // Return "undefined" for objects that should be treated
        // as null when doing comparisons.
        if (JSC::asObject(value)->structure()->typeInfo().masqueradesAsUndefined())
            return JSTYPE_UNDEFINED;

        JSC::CallData callData;
        JSC::JSObject* object = JSC::asObject(value);
        if (object->methodTable()->getCallData(object, callData) != JSC::CallTypeNone)
            return JSTYPE_FUNCTION;
    }

    return JSTYPE_OBJECT;
}

bool wkeJSIsNumber(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_NUMBER ? true : false;
}

bool wkeJSIsString(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_STRING ? true : false;
}

bool wkeJSIsBool(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_BOOLEAN ? true : false;
}

bool wkeJSIsObject(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_OBJECT ? true : false;
}

bool wkeJSIsFunction(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_FUNCTION ? true : false;
}

bool wkeJSIsUndefined(wkeJSValue v)
{
    return wkeJSTypeOf(v) == JSTYPE_UNDEFINED ? true : false;
}

bool wkeJSIsNull(wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.isNull();
}

bool wkeJSIsArray(wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    if (!value.isObject())
        return false;

    return value.inherits(&JSC::JSArray::s_info);
}

bool wkeJSIsTrue(wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.isTrue();
}

bool wkeJSIsFalse(wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.isFalse();
}

int wkeJSToInt(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.toInt32((JSC::ExecState*)es);
}

float wkeJSToFloat(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.toFloat((JSC::ExecState*)es);
}

double wkeJSToDouble(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.toNumber((JSC::ExecState*)es);
}

bool wkeJSToBool(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    return value.toBoolean((JSC::ExecState*)es);
}


template <class T>
class wkeSimpleStringT
{
public:
    wkeSimpleStringT()
        : m_buffer(NULL)
        , m_capacity(0)
        , m_size(0)
    {}

   ~wkeSimpleStringT()
    {
        delete [] m_buffer;
    }

public:
    void assign(const T* ptr, size_t len)
    {
        if (!m_buffer || m_capacity < len)
        {
            delete [] m_buffer;
            m_buffer = new T[len * 2 + 1];
            m_capacity = len * 2;
        }
        memcpy(m_buffer, ptr, len * sizeof(T));
        m_buffer[len] = 0;
    }

    const T* c_str() const
    {
        return m_buffer;
    }

protected:
    T* m_buffer;
    size_t m_size;
    size_t m_capacity;
};

static wkeSimpleStringT<char> s_sharedStringBuffer;
static wkeSimpleStringT<wchar_t> s_sharedStringBufferW;

const utf8* wkeJSToTempString(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    JSC::UString str = value.toString((JSC::ExecState*)es);

    CString utf8 = str.utf8();
    s_sharedStringBuffer.assign(utf8.data(), utf8.length());
    return s_sharedStringBuffer.c_str();
}

const wchar_t* wkeJSToTempStringW(wkeJSState* es, wkeJSValue v)
{
    JSC::JSValue value = JSC::JSValue::decode((JSC::EncodedJSValue)v);
    JSC::UString str = value.toString((JSC::ExecState*)es);

    s_sharedStringBufferW.assign(str.characters(), str.length());
    return s_sharedStringBufferW.c_str();
}

wkeJSValue wkeJSInt(int n)
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsNumber(n));
}

wkeJSValue wkeJSFloat(float f)
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsDoubleNumber(f));
}

wkeJSValue wkeJSDouble(double d)
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsDoubleNumber(d));
}

wkeJSValue wkeJSBool(bool b)
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsBoolean(b));
}

wkeJSValue wkeJSUndefined()
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsUndefined());
}

wkeJSValue wkeJSNull()
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsNull());
}

wkeJSValue wkeJSTrue()
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsBoolean(true));
}

wkeJSValue wkeJSFalse()
{
    return (wkeJSValue)JSC::JSValue::encode(JSC::jsBoolean(false));
}

wkeJSValue wkeJSString(wkeJSState* es, const utf8* str)
{
    String s = String::fromUTF8(str);
    JSC::JSValue value = JSC::jsString((JSC::ExecState*)es, JSC::UString(s.impl()));
    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSValue wkeJSStringW(wkeJSState* es, const wchar_t* str)
{
    JSC::JSValue value = JSC::jsString((JSC::ExecState*)es, JSC::UString(str));
    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSValue wkeJSEmptyObject(wkeJSState* es)
{
    JSC::JSValue value(JSC::constructEmptyObject((JSC::ExecState*)es));
    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSValue wkeJSEmptyArray(wkeJSState* es)
{
    JSC::JSValue value(JSC::constructEmptyArray((JSC::ExecState*)es));
    return (wkeJSValue)JSC::JSValue::encode(value);
}




//return the window object
wkeJSValue wkeJSGlobalObject(wkeJSState* es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue value(exec->lexicalGlobalObject()->toThisObject(exec));

    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSValue wkeJSEval(wkeJSState* es, const utf8* str)
{
    String s = String::fromUTF8(str);
    return (wkeJSValue)wkeJSEvalW(es, s.charactersWithNullTermination());
}

wkeJSValue wkeJSEvalW(wkeJSState* es, const wchar_t* str)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;

    // evaluate sets "this" to the global object if it is NULL
    JSC::JSGlobalObject* globalObject = exec->dynamicGlobalObject();
    JSC::SourceCode source = JSC::makeSource(str);

    JSC::JSValue returnValue = JSC::evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), source);
    if (returnValue)
        return (wkeJSValue)JSC::JSValue::encode(returnValue);

    // happens, for example, when the only statement is an empty (';') statement
    return wkeJSUndefined();
}

wkeJSValue wkeJSCall(wkeJSState* es, wkeJSValue func, wkeJSValue thisValue, wkeJSValue* args, int argCount)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;

    if (!wkeJSIsFunction(func))
        return wkeJSUndefined();

    JSC::JSValue jsThisValue = JSC::JSValue::decode((JSC::EncodedJSValue)thisValue);
    if (!jsThisValue.isObject())
        jsThisValue = exec->globalThisValue();

    JSC::MarkedArgumentBuffer argList;
    for (int i = 0; i < argCount; i++)
        argList.append(JSC::JSValue::decode((JSC::EncodedJSValue)args[i]));

    JSC::CallData callData;
    JSC::JSObject* object = JSC::asObject(JSC::JSValue::decode((JSC::EncodedJSValue)func));
    JSC::CallType callType = object->methodTable()->getCallData(object, callData);

    JSC::JSValue value = JSC::call(exec, object, callType, callData, jsThisValue, argList);
    return (wkeJSValue)JSC::JSValue::encode(value);
}

wkeJSValue wkeJSCallGlobal(wkeJSState* es, wkeJSValue func, wkeJSValue* args, int argCount)
{
    return wkeJSCall(es, func, wkeJSUndefined(), args, argCount);
}

wkeJSValue wkeJSGet(wkeJSState* es, wkeJSValue object, const char* prop)
{
    //cexer
    //不能使用JSC::Identifier((JSC::ExecState*)es, prop)构造JSC::Identifier，因JSC::UString内部把const char*地址作为hash值，
    //如果使用相同的内存地址，每次存放不同的属性名称来调用此函数，却都访问的是第一次调用时的属性。
    //JSC::JSValue o = JSC::JSValue::decode(object);
    //JSC::JSValue ret = o.get((JSC::ExecState*)es, JSC::Identifier((JSC::ExecState*)es, prop));
    //return JSC::JSValue::encode(ret);

    wkeJSValue ret = wkeJSUndefined();

    JSC::ExecState* exec = (JSC::ExecState*)es;
    if (JSC::JSGlobalData* data = exec->scopeChain()->globalData)
    {
        JSContextRef ctx = toRef(exec);
        JSObjectRef objectRef = JSValueToObject(ctx, toRef(exec, JSC::JSValue::decode((JSC::EncodedJSValue)object)), NULL);
        JSStringRef propertyName = JSStringCreateWithUTF8CString(prop);
        JSValueRef valueRef = JSObjectGetProperty(ctx, objectRef, propertyName, NULL);
        ret = (wkeJSValue)JSC::JSValue::encode(toJS(exec, valueRef));
    }
    return ret;
}

void wkeJSSet(wkeJSState* es, wkeJSValue object, const char* prop, wkeJSValue value)
{
    //cexer
    //不能使用JSC::Identifier((JSC::ExecState*)es, prop)构造JSC::Identifier，因JSC::UString内部把const char*地址作为hash值，
    //如果使用相同的内存地址，每次存放不同的属性名称来调用此函数，却都访问的是第一次调用时的属性。
    //JSC::JSValue o = JSC::JSValue::decode(object);
    //JSC::JSValue v = JSC::JSValue::decode(value);
    //JSC::PutPropertySlot slot;
    //o.put((JSC::ExecState*)es, JSC::Identifier((JSC::ExecState*)es, prop), v, slot);

    JSC::ExecState* exec = (JSC::ExecState*)es;
    if (JSC::JSGlobalData* data = exec->scopeChain()->globalData)
    {
        JSContextRef ctx = toRef(exec);
        JSObjectRef objectRef = JSValueToObject(ctx, toRef(exec, JSC::JSValue::decode((JSC::EncodedJSValue)object)), NULL);
        JSStringRef propertyName = JSStringCreateWithUTF8CString(prop);
        JSValueRef valueRef = toRef(exec, JSC::JSValue::decode((JSC::EncodedJSValue)value));

        JSObjectSetProperty(ctx, objectRef, propertyName, valueRef, kJSClassAttributeNone, NULL);
    }
}

wkeJSValue wkeJSGetGlobal(wkeJSState* es, const char* prop)
{
    return wkeJSGet(es, wkeJSGlobalObject(es), prop);
}

void wkeJSSetGlobal(wkeJSState* es, const char* prop, wkeJSValue v)
{
    wkeJSSet(es, wkeJSGlobalObject(es), prop, v);
}

wkeJSValue wkeJSGetAt(wkeJSState* es, wkeJSValue object, int index)
{
    JSC::JSValue o = JSC::JSValue::decode((JSC::EncodedJSValue)object);

    JSC::JSValue ret = o.get((JSC::ExecState*)es, index);
    return (wkeJSValue)JSC::JSValue::encode(ret);
}

void wkeJSSetAt(wkeJSState* es, wkeJSValue object, int index, wkeJSValue value)
{
    JSC::JSValue o = JSC::JSValue::decode((JSC::EncodedJSValue)object);
    JSC::JSValue v = JSC::JSValue::decode((JSC::EncodedJSValue)value);

    o.put((JSC::ExecState*)es, index, v);
}

int wkeJSGetLength(wkeJSState* es, wkeJSValue object)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue o = JSC::JSValue::decode((JSC::EncodedJSValue)object);
    return o.get(exec, JSC::Identifier(exec, "length")).toInt32(exec);
}

void wkeJSSetLength(wkeJSState* es, wkeJSValue object, int length)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue o = JSC::JSValue::decode((JSC::EncodedJSValue)object);

    JSC::PutPropertySlot slot;
    o.put(exec, JSC::Identifier(exec, "length"), JSC::jsNumber(length), slot);
}

wkeWebView* wkeJSGetWebView(wkeJSState* es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    WebCore::JSDOMWindow* window = WebCore::asJSDOMWindow(exec->lexicalGlobalObject());
    if (window)
        return (wkeWebView*)window->impl()->frame()->page()->chrome()->client()->webView();

    return 0;
}

void wkeJSCollectGarbge()
{
    WebCore::gcController().garbageCollectNow();
}


void wkeJSAddRef(wkeJSState* es, wkeJSValue val)
{
    JSC::JSValue v = JSC::JSValue::decode((JSC::EncodedJSValue)val);
    JSC::gcProtect(v);
}

void wkeJSReleaseRef(wkeJSState* es, wkeJSValue val)
{
    JSC::JSValue v = JSC::JSValue::decode((JSC::EncodedJSValue)val);
    JSC::gcUnprotect(v);
}



static void addFunction(JSC::JSGlobalObject* globalObject, const char* name, wkeJSNativeFunction function, unsigned int argCount)
{
    JSC::ExecState* exec = globalObject->globalExec();

    JSC::Identifier identifier(exec, name);
    JSC::JSFunction* funcObject = JSC::JSFunction::create(exec, globalObject, argCount, identifier, (JSC::NativeFunction)function);

    globalObject->putDirect(globalObject->globalData(), identifier, funcObject);
}

static void addGetter(JSC::JSGlobalObject* globalObject, const char* name, wkeJSNativeFunction function)
{
    JSC::ExecState* exec = globalObject->globalExec();

    JSC::Identifier identifier(exec, name);
    JSC::JSFunction* getterFunc = JSC::JSFunction::create(exec, globalObject, 0, identifier, (JSC::NativeFunction)function);
    globalObject->defineGetter(exec, identifier, getterFunc, 0);
}

static void addSetter(JSC::JSGlobalObject* globalObject, const char* name, wkeJSNativeFunction function)
{
    JSC::ExecState* exec = globalObject->globalExec();

    JSC::Identifier identifier(exec, name);
    JSC::JSFunction* setterFunc = JSC::JSFunction::create(exec, globalObject, 1, identifier, (JSC::NativeFunction)function);
    globalObject->defineSetter(exec, identifier, setterFunc, 0);
}


#define MAX_NAME_LENGTH 32
#define MAX_FUNCTION_COUNT 1024

#define JS_FUNC   (0)
#define JS_GETTER (1)
#define JS_SETTER (2)
struct jsFunctionInfo
{
    char name[MAX_NAME_LENGTH];
    wkeJSNativeFunction fn;
    unsigned int argCount;
    unsigned int funcType;
};

static Vector<jsFunctionInfo> s_jsFunctions;

void wkeJSBindFunction(const char* name, wkeJSNativeFunction fn, unsigned int argCount)
{
    for (unsigned int i = 0; i < s_jsFunctions.size(); ++i)
    {
        if (s_jsFunctions[i].funcType == JS_FUNC 
          &&strncmp(name, s_jsFunctions[i].name, MAX_NAME_LENGTH) == 0)
        {
            s_jsFunctions[i].fn = fn;
            s_jsFunctions[i].argCount = argCount;
            return;
        }
    }

    jsFunctionInfo funcInfo;
    strncpy(funcInfo.name, name, MAX_NAME_LENGTH - 1);
    funcInfo.name[MAX_NAME_LENGTH - 1] = '\0';
    funcInfo.fn = fn;
    funcInfo.argCount = argCount;
    funcInfo.funcType = JS_FUNC;

    s_jsFunctions.append(funcInfo);
}

void wkeJSBindGetter(const char* name, wkeJSNativeFunction fn)
{
    for (unsigned int i = 0; i < s_jsFunctions.size(); ++i)
    {
        if (s_jsFunctions[i].funcType == JS_GETTER 
          &&strncmp(name, s_jsFunctions[i].name, MAX_NAME_LENGTH) == 0)
        {
            s_jsFunctions[i].fn = fn;
            return;
        }
    }

    jsFunctionInfo funcInfo;
    strncpy(funcInfo.name, name, MAX_NAME_LENGTH - 1);
    funcInfo.name[MAX_NAME_LENGTH - 1] = '\0';
    funcInfo.fn = fn;
    funcInfo.argCount = 0;
    funcInfo.funcType = JS_GETTER;

    s_jsFunctions.append(funcInfo);
}

void wkeJSBindSetter(const char* name, wkeJSNativeFunction fn)
{
    for (unsigned int i = 0; i < s_jsFunctions.size(); ++i)
    {
        if (s_jsFunctions[i].funcType == JS_SETTER 
          &&strncmp(name, s_jsFunctions[i].name, MAX_NAME_LENGTH) == 0)
        {
            s_jsFunctions[i].fn = fn;
            return;
        }
    }

    jsFunctionInfo funcInfo;
    strncpy(funcInfo.name, name, MAX_NAME_LENGTH - 1);
    funcInfo.name[MAX_NAME_LENGTH - 1] = '\0';
    funcInfo.fn = fn;
    funcInfo.argCount = 1;
    funcInfo.funcType = JS_SETTER;

    s_jsFunctions.append(funcInfo);
}



wkeJSValue JS_CALL js_outputMsg(wkeJSState* es)
{
    //ASSERT(wkeJSParamCount(es) == 1);
    //ASSERT(wkeJSParamType(es, 0) == JSTYPE_STRING);

    wkeJSValue value = wkeJSParam(es, 0);
    OutputDebugStringW(wkeJSToTempStringW(es, value));

    return wkeJSUndefined();
}

wkeJSValue JS_CALL js_getWebViewName(wkeJSState* es)
{
    wkeWebView* webView = wkeJSGetWebView(es);
    return wkeJSString(es, webView->name());
}

wkeJSValue JS_CALL js_setWebViewName(wkeJSState* es)
{
    const char* name = wkeJSToTempString(es, wkeJSParam(es, 0));
    wkeWebView* webView = wkeJSGetWebView(es);
    webView->setName(name);

    return wkeJSUndefined();
}

JSValueRef objectGetPropertyCallback(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    JSC::ExecState* exec = toJS(ctx);
    JSC::JSObject* obj = toJS(object);

    wkeJSData* p = (wkeJSData*)JSObjectGetPrivate(object);
    if (!p || !p->propertyGet)
        return false;

    WTF::CString str = propertyName->ustring().latin1();
    const char* name = str.data();
    wkeJSValue ret = p->propertyGet((wkeJSState*)exec, (wkeJSValue)JSC::JSValue::encode(obj), name);

    return toRef(exec, JSC::JSValue::decode((JSC::EncodedJSValue)ret));
}

bool objectSetPropertyCallback(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    JSC::ExecState* exec = toJS(ctx);
    JSC::JSObject* obj = toJS(object);

    wkeJSData* p = (wkeJSData*)JSObjectGetPrivate(object);
    if (!p || !p->propertySet)
        return false;

    WTF::CString str = propertyName->ustring().latin1();
    const char* name = str.data();
    return p->propertySet((wkeJSState*)exec, (wkeJSValue)JSC::JSValue::encode(obj), name, (wkeJSValue)JSC::JSValue::encode(toJS(exec,value)));
}

void objectFinalize(JSObjectRef object)
{
    wkeJSData* p = (wkeJSData*)JSObjectGetPrivate(object);
    if (p && p->finalize)
        p->finalize(p);
}

JSValueRef objectCallAsFunctionCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    JSC::ExecState* exec = toJS(ctx);
    JSC::JSObject* obj = toJS(function);

    wkeJSData* p = (wkeJSData*)JSObjectGetPrivate(function);
    if (!p || !p->callAsFunction)
        return false;

    wkeJSValue args[10] = { 0 };
    for (int i = 0; i < argumentCount; ++i)
        args[i] = (wkeJSValue)JSC::JSValue::encode(toJS(exec, arguments[i]));

    wkeJSValue ret = p->callAsFunction((wkeJSState*)exec, (wkeJSValue)JSC::JSValue::encode(toJS(function)), args, argumentCount);
    return toRef(exec, JSC::JSValue::decode((JSC::EncodedJSValue)ret));
}


WKE_API wkeJSValue wkeJSObject(wkeJSState* es, wkeJSData* data)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSGlobalObject* globalObject = (JSC::JSGlobalObject*)exec->lexicalGlobalObject();
    JSContextRef ctx = toRef(exec);

    JSClassDefinition classDef = kJSClassDefinitionEmpty;
    classDef.getProperty = objectGetPropertyCallback;
    classDef.setProperty = objectSetPropertyCallback;
    classDef.finalize = objectFinalize;

    JSClassRef globalClass = JSClassCreate(&classDef);  
    JSObjectRef obj = JSObjectMake(ctx, globalClass, NULL);
    JSObjectSetPrivate(obj ,data);

    JSC::JSValue value = toJS(obj);
    return (wkeJSValue)JSC::JSValue::encode(value);
}

WKE_API wkeJSValue wkeJSFunction(wkeJSState* es, wkeJSData* data)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSGlobalObject* globalObject = (JSC::JSGlobalObject*)exec->lexicalGlobalObject();
    JSContextRef ctx = toRef(exec);

    JSClassDefinition classDef = kJSClassDefinitionEmpty;
    classDef.finalize = objectFinalize;
    classDef.callAsFunction = objectCallAsFunctionCallback;

    JSClassRef globalClass = JSClassCreate(&classDef);  
    JSObjectRef obj = JSObjectMake(ctx, globalClass, NULL);
    JSObjectSetPrivate(obj ,data);

    JSC::JSValue value = toJS(obj);
    return (wkeJSValue)JSC::JSValue::encode(value);
}


WKE_API wkeJSData* wkeJSGetData(wkeJSState* es, wkeJSValue object)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue val = JSC::JSValue::decode((JSC::EncodedJSValue)object);
    JSValueRef valRef = toRef(exec, val);
    JSContextRef ctxRef = toRef(exec);
    return (wkeJSData*)JSObjectGetPrivate(JSValueToObject(ctxRef, valRef, NULL));
}


void onCreateGlobalObject(JSC::JSGlobalObject* globalObject)
{
    addFunction(globalObject, "outputMsg", js_outputMsg, 1);
    addGetter(globalObject, "webViewName", js_getWebViewName);
    addSetter(globalObject, "webViewName", js_setWebViewName);

    JSC::ExecState* exec = globalObject->globalExec();
    wkeJSSetGlobal((wkeJSState*)exec, "wke", ::wkeJSString((wkeJSState*)exec, wkeGetVersionString()));

    for (size_t i = 0; i < s_jsFunctions.size(); ++i)
    {
        if (s_jsFunctions[i].funcType == JS_FUNC)
            addFunction(globalObject, s_jsFunctions[i].name, s_jsFunctions[i].fn, s_jsFunctions[i].argCount);
        else if (s_jsFunctions[i].funcType == JS_GETTER)
            addGetter(globalObject, s_jsFunctions[i].name, s_jsFunctions[i].fn);
        else if (s_jsFunctions[i].funcType == JS_SETTER)
            addSetter(globalObject, s_jsFunctions[i].name, s_jsFunctions[i].fn);
    }
}
