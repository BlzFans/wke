
#include "guiWebCtrl.h"
#include "gfx/gfxDrawUtil.h"
#include "platform/platformInput.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "gui/core/guiCanvas.h"
#include <d3d9.h>

#pragma comment(lib, "../../../source/wke.lib")

static void onTitleChanged(const wkeClientHandler* handler, wkeString title)
{
    ClientHandler* client = (ClientHandler*)handler;
    Con::executef(client->webCtrl, "onTitleChanged", wkeToString(title));
}

static void onURLChanged(const wkeClientHandler* handler, wkeString url)
{
    ClientHandler* client = (ClientHandler*)handler;
    Con::executef(client->webCtrl, "onURLChanged", wkeToString(url));
}


GFX_ImplementTextureProfile(GFXWebTextureProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::NoMipmap | GFXTextureProfile::Dynamic,
                            GFXTextureProfile::None);


IMPLEMENT_CONOBJECT(guiWebCtrl);

guiWebCtrl::guiWebCtrl()
{
    mClientHandler.onTitleChanged = onTitleChanged;
    mClientHandler.onURLChanged = onURLChanged;
    mClientHandler.webCtrl = this;

    mWebView = wkeCreateWebView();
    mWebView->setClientHandler(&mClientHandler);

    mLastUpdateTime = 0;
    mUpdateInterval = 30;
}

guiWebCtrl::~guiWebCtrl()
{
    mWebView->destroy();
    mWebView = NULL;
}

void guiWebCtrl::initPersistFields()
{
    Parent::initPersistFields();

    addField("updateInterval", TypeS32, Offset(mUpdateInterval, guiWebCtrl), "");
}

bool guiWebCtrl::onWake()
{
   if (!Parent::onWake())
      return false;

   mWebView->resize(getWidth(), getHeight());
   mTextureObject.set(getWidth(), getHeight(), GFXFormatR8G8B8A8, &GFXWebTextureProfile, "");

   setActive(true);
   setFirstResponder();

   return true;
}

void guiWebCtrl::onSleep()
{
   Parent::onSleep();
}

void guiWebCtrl::onPreRender()
{
    if (mLastUpdateTime + mUpdateInterval <= Platform::getVirtualMilliseconds())
    {
        mLastUpdateTime = Platform::getVirtualMilliseconds();
        if (mWebView->isDirty())
        {
            GFXLockedRect* rect = mTextureObject.lock(0, NULL, D3DLOCK_DISCARD);
            mWebView->paint(rect->bits, rect->pitch);
            mTextureObject.unlock();
        }
    }
}

void guiWebCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    if (mTextureObject)
    {
        GFX->getDrawUtil()->clearBitmapModulation();
        RectI rect(offset, getExtent());
        GFX->getDrawUtil()->drawBitmapStretch(mTextureObject, rect);
    }
}

bool guiWebCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
    if (Parent::resize(newPosition, newExtent))
    {
        mWebView->resize(getWidth(), getHeight());

        mTextureObject.set(getWidth(), getHeight(), GFXFormatR8G8B8A8, &GFXWebTextureProfile, "");
        return true;
    }

    return false;
}

void guiWebCtrl::onMouseUp(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    mouseUnlock();

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_LBUTTONUP, point.x, point.y, flags);
}

void guiWebCtrl::onMouseDown(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    setFirstResponder();

    //lock the mouse
	mouseLock();

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = WKE_LBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_LBUTTONDOWN, point.x, point.y, flags);
}

void guiWebCtrl::onMouseMove(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MOUSEMOVE, point.x, point.y, flags);
}

void guiWebCtrl::onMouseDragged(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = WKE_LBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MOUSEMOVE, point.x, point.y, flags);
}

bool guiWebCtrl::onMouseWheelUp(const GuiEvent &event)
{
    if (!mAwake || !mVisible)
        return false;

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    return mWebView->mouseWheel(point.x, point.y, (int)event.fval, flags);
}

bool guiWebCtrl::onMouseWheelDown(const GuiEvent &event)
{
    if (!mAwake || !mVisible)
        return false;

    Point2I point = globalToLocalCoord(event.mousePoint);
    
    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    return mWebView->mouseWheel(point.x, point.y, (int)event.fval, flags);
}

void guiWebCtrl::onRightMouseDown(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = WKE_RBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_RBUTTONDOWN, point.x, point.y, flags);
}

void guiWebCtrl::onRightMouseUp(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_RBUTTONUP, point.x, point.y, flags);
}

void guiWebCtrl::onRightMouseDragged(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = WKE_RBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MOUSEMOVE, point.x, point.y, flags);
}

void guiWebCtrl::onMiddleMouseDown(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = WKE_MBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MBUTTONDOWN, point.x, point.y, flags);
}

void guiWebCtrl::onMiddleMouseUp(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = 0;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MBUTTONUP, point.x, point.y, flags);
}

void guiWebCtrl::onMiddleMouseDragged(const GuiEvent &event)
{
    if (!mActive || !mAwake || !mVisible)
        return;

    Point2I point = globalToLocalCoord(event.mousePoint);

    unsigned int flags = WKE_MBUTTON;
    if (event.modifier & SI_CTRL)
        flags |= WKE_CONTROL;
    if (event.modifier & SI_SHIFT)
        flags |= WKE_SHIFT;

    mWebView->mouseEvent(WKE_MSG_MOUSEMOVE, point.x, point.y, flags);
}

bool guiWebCtrl::onKeyDown(const GuiEvent &event)
{
    if (event.keyCode)
    {
        U8 code = TranslateKeyCodeToOS(event.keyCode);
        if (mWebView->keyDown(code, 0, false))
            return true;
    }

    if (event.ascii >= 32)
        return mWebView->keyPress(event.ascii, 0, false);

    return false;
}

bool guiWebCtrl::onKeyUp(const GuiEvent &event)
{
    return mWebView->keyUp(event.ascii, 0, false);
}

void guiWebCtrl::setFirstResponder()
{
    Parent::setFirstResponder();

   GuiCanvas *root = getRoot();
   if (root != NULL)
   {
		root->enableKeyboardTranslation();

        // If the native OS accelerator keys are not disabled
        // then some key events like Delete, ctrl+V, etc may
        // not make it down to us.
		root->setNativeAcceleratorsEnabled( false );
   }
    mWebView->focus();
}

void guiWebCtrl::onLoseFirstResponder()
{
    Parent::onLoseFirstResponder();
    GuiCanvas *root = getRoot();
    if (root != NULL)
    {
        root->setNativeAcceleratorsEnabled( true );
        root->disableKeyboardTranslation();
    }

    mWebView->unfocus();
}

bool guiWebCtrl::getCaretPos(Point2I& pos)
{
    wkeRect rect = mWebView->getCaret();
    if (rect.x == 0 && rect.y == 0)
        return false;

    pos.x = rect.x;
    pos.y = rect.y + rect.h;

    pos = localToGlobalCoord(pos);
    return true;
}

void guiWebCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
    GuiCanvas *pRoot = getRoot();
    if( !pRoot )
        return;

    const int invalid_cur = 999;
    if (pRoot->mCursorChanged == invalid_cur)
        return;

    PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
    PlatformCursorController *pController = pWindow->getCursorController();

    // Now change the cursor shape
    pController->popCursor();
    pController->pushCursor(invalid_cur);
    pRoot->mCursorChanged = invalid_cur;
}


void guiWebCtrl::loadURL(const char* url)
{
    mWebView->loadURL(url);
}

void guiWebCtrl::loadFile(const char* filename)
{
    mWebView->loadFile(filename);
}

void guiWebCtrl::loadHTML(const char* string)
{
    mWebView->loadHTML(string);
}

ConsoleMethod(guiWebCtrl, loadURL, void, 3, 3, "")
{
   object->loadURL(argv[2]);
}

ConsoleMethod(guiWebCtrl, loadFile, void, 3, 3, "")
{
   object->loadFile(argv[2]);
}

ConsoleMethod(guiWebCtrl, loadHTML, void, 3, 3, "")
{
   object->loadHTML(argv[2]);
}