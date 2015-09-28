#ifndef WKE_DRAG_CLIENT_H
#define WKE_DRAG_CLIENT_H

//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/DragClient.h>
#include <WebCore/DragActions.h>


//////////////////////////////////////////////////////////////////////////



namespace wke
{




class DragClient : public WebCore::DragClient
{
protected:
    virtual ~DragClient();;

    virtual void willPerformDragDestinationAction(WebCore::DragDestinationAction, WebCore::DragData*) override;

    virtual void willPerformDragSourceAction(WebCore::DragSourceAction, const WebCore::IntPoint&, WebCore::Clipboard*) override;

    virtual WebCore::DragDestinationAction actionMaskForDrag(WebCore::DragData*) override;

    //We work in window rather than view coordinates here
    virtual WebCore::DragSourceAction dragSourceActionMaskForPoint(const WebCore::IntPoint& windowPoint) override;

    virtual void startDrag(WebCore::DragImageRef dragImage, const WebCore::IntPoint& dragImageOrigin, const WebCore::IntPoint& eventPos, WebCore::Clipboard*, WebCore::Frame*, bool linkDrag = false) override;

    virtual void dragControllerDestroyed() override;
};



}


#endif //#ifndef WKE_DRAG_CLIENT_H