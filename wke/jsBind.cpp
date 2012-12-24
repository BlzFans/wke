#include <WebCore/config.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSFunction.h>
#include <JavaScriptCore/SourceCode.h>
#include <JavaScriptCore/Completion.h>
#include <WebCore/GCController.h>
#include <WebCore/JSDOMWindowCustom.h>
#include <WebCore/Page.h>
#include <WebCore/Frame.h>
#include <WebCore/Chrome.h>
#include <WebCore/ChromeClient.h>

#include "wke.h"
#include "wkeDebug.h"
#include "stringTable.h"

int jsArgCount(jsExecState es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    return (int)exec->argumentCount();
}

jsType jsArgType(jsExecState es, int argIdx)
{
    return jsTypeOf(jsArg(es, argIdx));
}

jsValue jsArg(jsExecState es, int argIdx)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue value = exec->argument(argIdx);
    return JSC::JSValue::encode(value);
}

jsType jsTypeOf(jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);

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

bool jsIsNumber(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_NUMBER ? true : false;
}

bool jsIsString(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_STRING ? true : false;
}

bool jsIsBoolean(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_BOOLEAN ? true : false;
}

bool jsIsObject(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_OBJECT ? true : false;
}

bool jsIsFunction(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_FUNCTION ? true : false;
}

bool jsIsUndefined(jsValue v)
{
    return jsTypeOf(v) == JSTYPE_UNDEFINED ? true : false;
}

bool jsIsNull(jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.isNull();
}

bool jsIsArray(jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    if (!value.isObject())
        return false;

    return value.inherits(&JSC::JSArray::s_info);
}

bool jsIsTrue(jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.isTrue();
}

bool jsIsFalse(jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.isFalse();
}

int jsToInt(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.toInt32((JSC::ExecState*)es);
}

float jsToFloat(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.toFloat((JSC::ExecState*)es);
}

double jsToDouble(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.toNumber((JSC::ExecState*)es);
}

bool jsToBoolean(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    return value.toBoolean((JSC::ExecState*)es);
}

const utf8* jsToString(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    JSC::UString str = value.toString((JSC::ExecState*)es);

    return StringTable::addString(str.characters(), str.length());
}

const wchar_t* jsToStringW(jsExecState es, jsValue v)
{
    JSC::JSValue value = JSC::JSValue::decode(v);
    JSC::UString str = value.toString((JSC::ExecState*)es);

    return StringTableW::addString(str.characters(), str.length());
}

jsValue jsInt(int n)
{
    return JSC::JSValue::encode(JSC::jsNumber(n));
}

jsValue jsFloat(float f)
{
    return JSC::JSValue::encode(JSC::jsDoubleNumber(f));
}

jsValue jsDouble(double d)
{
    return JSC::JSValue::encode(JSC::jsDoubleNumber(d));
}

jsValue jsBoolean(bool b)
{
    return JSC::JSValue::encode(JSC::jsBoolean(b));
}

jsValue jsUndefined()
{
    return JSC::JSValue::encode(JSC::jsUndefined());
}

jsValue jsNull()
{
    return JSC::JSValue::encode(JSC::jsNull());
}

jsValue jsTrue()
{
    return JSC::JSValue::encode(JSC::jsBoolean(true));
}

jsValue jsFalse()
{
    return JSC::JSValue::encode(JSC::jsBoolean(false));
}

jsValue jsString(jsExecState es, const utf8* str)
{
    String s = String::fromUTF8(str);
    JSC::JSValue value = JSC::jsString((JSC::ExecState*)es, JSC::UString(s.impl()));
    return JSC::JSValue::encode(value);
}

jsValue jsStringW(jsExecState es, const wchar_t* str)
{
    JSC::JSValue value = JSC::jsString((JSC::ExecState*)es, JSC::UString(str));
    return JSC::JSValue::encode(value);
}

jsValue jsObject(jsExecState es)
{
    JSC::JSValue value(JSC::constructEmptyObject((JSC::ExecState*)es));
    return JSC::JSValue::encode(value);
}

jsValue jsArray(jsExecState es)
{
    JSC::JSValue value(JSC::constructEmptyArray((JSC::ExecState*)es));
    return JSC::JSValue::encode(value);
}

jsValue jsFunction(jsExecState es, jsNativeFunction fn, unsigned int argCount)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSGlobalObject* globalObject = (JSC::JSGlobalObject*)exec->lexicalGlobalObject()->toThisObject(exec);

    JSC::JSValue value(JSC::JSFunction::create(exec, globalObject, argCount, JSC::Identifier(), (JSC::NativeFunction)fn));
    return JSC::JSValue::encode(value);
}


//return the window object
jsValue jsGlobalObject(jsExecState es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue value(exec->lexicalGlobalObject()->toThisObject(exec));

    return JSC::JSValue::encode(value);
}

jsValue jsEval(jsExecState es, const utf8* str)
{
    String s = String::fromUTF8(str);
    return jsEvalW(es, s.charactersWithNullTermination());
}

jsValue jsEvalW(jsExecState es, const wchar_t* str)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;

    // evaluate sets "this" to the global object if it is NULL
    JSC::JSGlobalObject* globalObject = exec->dynamicGlobalObject();
    JSC::SourceCode source = JSC::makeSource(str);

    JSC::JSValue returnValue = JSC::evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), source);
    if (returnValue)
        return JSC::JSValue::encode(returnValue);

    // happens, for example, when the only statement is an empty (';') statement
    return jsUndefined();
}

jsValue jsCall(jsExecState es, jsValue func, jsValue thisValue, jsValue* args, int argCount)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;

    if (!jsIsFunction(func))
        return jsUndefined();

    JSC::JSValue jsThisValue = JSC::JSValue::decode(thisValue);
    if (!jsThisValue.isObject())
        jsThisValue = exec->globalThisValue();

    JSC::MarkedArgumentBuffer argList;
    for (int i = 0; i < argCount; i++)
        argList.append(JSC::JSValue::decode(args[i]));

    JSC::CallData callData;
    JSC::JSObject* object = JSC::asObject(JSC::JSValue::decode(func));
    JSC::CallType callType = object->methodTable()->getCallData(object, callData);

    JSC::JSValue value = JSC::call(exec, object, callType, callData, jsThisValue, argList);
    return JSC::JSValue::encode(value);
}

jsValue jsCallGlobal(jsExecState es, jsValue func, jsValue* args, int argCount)
{
    return jsCall(es, func, jsUndefined(), args, argCount);
}

jsValue jsGet(jsExecState es, jsValue object, const char* prop)
{
    JSC::JSValue o = JSC::JSValue::decode(object);

    JSC::JSValue ret = o.get((JSC::ExecState*)es, JSC::Identifier((JSC::ExecState*)es, prop));
    return JSC::JSValue::encode(ret);
}

void jsSet(jsExecState es, jsValue object, const char* prop, jsValue value)
{
    JSC::JSValue o = JSC::JSValue::decode(object);
    JSC::JSValue v = JSC::JSValue::decode(value);

    JSC::PutPropertySlot slot;
    o.put((JSC::ExecState*)es, JSC::Identifier((JSC::ExecState*)es, prop), v, slot);
}

jsValue jsGetGlobal(jsExecState es, const char* prop)
{
    return jsGet(es, jsGlobalObject(es), prop);
}

void jsSetGlobal(jsExecState es, const char* prop, jsValue v)
{
    jsSet(es, jsGlobalObject(es), prop, v);
}

jsValue jsGetAt(jsExecState es, jsValue object, int index)
{
    JSC::JSValue o = JSC::JSValue::decode(object);

    JSC::JSValue ret = o.get((JSC::ExecState*)es, index);
    return JSC::JSValue::encode(ret);
}

void jsSetAt(jsExecState es, jsValue object, int index, jsValue value)
{
    JSC::JSValue o = JSC::JSValue::decode(object);
    JSC::JSValue v = JSC::JSValue::decode(value);

    o.put((JSC::ExecState*)es, index, v);
}

int jsGetLength(jsExecState es, jsValue object)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue o = JSC::JSValue::decode(object);
    return o.get(exec, JSC::Identifier(exec, "length")).toInt32(exec);
}

void jsSetLength(jsExecState es, jsValue object, int length)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    JSC::JSValue o = JSC::JSValue::decode(object);

    JSC::PutPropertySlot slot;
    o.put(exec, JSC::Identifier(exec, "length"), JSC::jsNumber(length), slot);
}

wkeWebView jsGetWebView(jsExecState es)
{
    JSC::ExecState* exec = (JSC::ExecState*)es;
    WebCore::JSDOMWindow* window = WebCore::asJSDOMWindow(exec->lexicalGlobalObject());
    if (window)
        return (wkeWebView)window->impl()->frame()->page()->chrome()->client()->webView();

    return 0;
}

void jsGC()
{
    WebCore::gcController().garbageCollectNow();
}




static void addFunction(JSC::JSGlobalObject* globalObject, const char* name, jsNativeFunction function, unsigned int argCount)
{
    JSC::ExecState* exec = globalObject->globalExec();

    JSC::Identifier identifier(exec, name);
    JSC::JSFunction* funcObject = JSC::JSFunction::create(exec, globalObject, argCount, identifier, (JSC::NativeFunction)function);

    globalObject->putDirect(globalObject->globalData(), identifier, funcObject);
}

static void addGetter(JSC::JSGlobalObject* globalObject, const char* name, jsNativeFunction function)
{
    JSC::ExecState* exec = globalObject->globalExec();

    JSC::Identifier identifier(exec, name);
    JSC::JSFunction* getterFunc = JSC::JSFunction::create(exec, globalObject, 0, identifier, (JSC::NativeFunction)function);
    globalObject->defineGetter(exec, identifier, getterFunc, 0);
}

static void addSetter(JSC::JSGlobalObject* globalObject, const char* name, jsNativeFunction function)
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
    jsNativeFunction fn;
    unsigned int argCount;
    unsigned int funcType;
};

static Vector<jsFunctionInfo> s_jsFunctions;

void jsBindFunction(const char* name, jsNativeFunction fn, unsigned int argCount)
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

void jsBindGetter(const char* name, jsNativeFunction fn)
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

void jsBindSetter(const char* name, jsNativeFunction fn)
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



jsValue JS_CALL js_outputMsg(jsExecState es)
{
    //ASSERT(jsArgCount(es) == 1);
    //ASSERT(jsArgType(es, 0) == JSTYPE_STRING);

    jsValue value = jsArg(es, 0);
    OutputDebugStringW(jsToStringW(es, value));

    return jsUndefined();
}

jsValue JS_CALL js_getWebViewName(jsExecState es)
{
    wkeWebView webView = jsGetWebView(es);
    return jsString(es, webView->name());
}

jsValue JS_CALL js_setWebViewName(jsExecState es)
{
    const char* name = jsToString(es, jsArg(es, 0));
    wkeWebView webView = jsGetWebView(es);
    webView->setName(name);

    return jsUndefined();
}

void onCreateGlobalObject(JSC::JSGlobalObject* globalObject)
{
    addFunction(globalObject, "outputMsg", js_outputMsg, 1);
    addGetter(globalObject, "webViewName", js_getWebViewName);
    addSetter(globalObject, "webViewName", js_setWebViewName);

    JSC::ExecState* exec = globalObject->globalExec();
    jsSetGlobal(exec, "wke", ::jsString(exec, wkeVersionString()));

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