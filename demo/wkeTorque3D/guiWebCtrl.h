#ifndef WEB_CTRL_H
#define WEB_CTRL_H

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif

#include "wke.h"

class guiWebCtrl;
struct ClientHandler : public wkeClientHandler
{
    guiWebCtrl* webCtrl;
};

class guiWebCtrl : public GuiControl
{
private:
    typedef GuiControl Parent;

public:
    DECLARE_CONOBJECT(guiWebCtrl);

    guiWebCtrl();
    virtual ~guiWebCtrl();
    static void initPersistFields();

    virtual bool onWake();
    virtual void onSleep();
    virtual void onPreRender();
    virtual void onRender(Point2I offset, const RectI &updateRect);
    virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);

    virtual void onMouseUp(const GuiEvent &event);
    virtual void onMouseDown(const GuiEvent &event);
    virtual void onMouseMove(const GuiEvent &event);
    virtual void onMouseDragged(const GuiEvent &event);

    virtual bool onMouseWheelUp(const GuiEvent &event);
    virtual bool onMouseWheelDown(const GuiEvent &event);

    virtual void onRightMouseDown(const GuiEvent &event);
    virtual void onRightMouseUp(const GuiEvent &event);
    virtual void onRightMouseDragged(const GuiEvent &event);

    virtual void onMiddleMouseDown(const GuiEvent &event);
    virtual void onMiddleMouseUp(const GuiEvent &event);
    virtual void onMiddleMouseDragged(const GuiEvent &event);

    virtual bool onKeyDown(const GuiEvent &event);
    virtual bool onKeyUp(const GuiEvent &event);

    virtual void setFirstResponder();
    virtual void onLoseFirstResponder();

    virtual bool getCaretPos(Point2I& pos);

    virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);

    void loadURL(const char* url);
    void loadFile(const char* filename);
    void loadHTML(const char* string);

protected:
    wke::IWebView* mWebView;
    GFXTexHandle mTextureObject;
    U32 mLastUpdateTime;
    U32 mUpdateInterval;
    ClientHandler mClientHandler;
};

#endif