
#include "WebUIApp.h"
#include "WebPanelOverlayElement.h"

void* fileOpen(const char* path)
{
    Ogre::String name = path;
    std::replace(name.begin(), name.end(), '\\', '/' );
    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(name);
    if (stream.isNull())
        return NULL;

    Ogre::DataStreamPtr* pPtr = new Ogre::DataStreamPtr(stream);
    return pPtr;
}

void fileClose(void* handle)
{
    Ogre::DataStreamPtr* pPtr = (Ogre::DataStreamPtr*)handle;
    delete pPtr;
}

size_t fileSize(void* handle)
{
    Ogre::DataStreamPtr* pPtr = (Ogre::DataStreamPtr*)handle;
    if (pPtr)
        return (*pPtr)->size();

    return 0;
}

int fileRead(void* handle, void* buffer, size_t size)
{
    Ogre::DataStreamPtr* pPtr = (Ogre::DataStreamPtr*)handle;
    if (pPtr)
        return (int)(*pPtr)->read(buffer, size);

    return 0;
}

int fileSeek(void* handle, int offset, int origin)
{
    Ogre::DataStreamPtr* pPtr = (Ogre::DataStreamPtr*)handle;
    if (pPtr)
    {
        int pos = offset;
        if (origin == SEEK_CUR)
        {
            pos = (int)(*pPtr)->tell() + offset;
        }
        else if (origin == SEEK_END)
        {
            pos = (int)(*pPtr)->size() + offset;
        }

        pos = Ogre::Math::Clamp(pos, 0, (int)(*pPtr)->size());
        (*pPtr)->seek(pos);
        return 0;
    }

    return 0;
}

jsValue JS_CALL js_goBack(jsExecState es)
{
    getWebView()->goBack();
    return jsUndefined();
}

jsValue JS_CALL js_goForward(jsExecState es)
{
    getWebView()->goForward();
    return jsUndefined();
}

jsValue JS_CALL js_loadURL(jsExecState es)
{
    const char* url = jsToString(es, jsArg(es, 0));
    getWebView()->loadURL(url);
    return jsUndefined();
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
    wkeInit();
    wkeSetFileSystem(fileOpen, fileClose, fileSize, fileRead, fileSeek);
    jsBindFunction("goBack", js_goBack, 0);
    jsBindFunction("goForward", js_goForward, 0);
    jsBindFunction("loadURL", js_loadURL, 1);

    // Create application object
    WebUIApp app;

    try {
        app.go();
    } catch( Ogre::Exception& e ) {
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
    }

    wkeShutdown();

    return 0;
}
