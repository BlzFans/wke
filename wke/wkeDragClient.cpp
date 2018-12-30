//////////////////////////////////////////////////////////////////////////


#include "wkeDragClient.h"


//////////////////////////////////////////////////////////////////////////


namespace wke
{



DragClient::~DragClient()
{

}


void DragClient::dragControllerDestroyed()
{
    delete this;
}

void DragClient::willPerformDragSourceAction(WebCore::DragSourceAction, const WebCore::IntPoint&, WebCore::Clipboard*)
{

}

void DragClient::willPerformDragDestinationAction(WebCore::DragDestinationAction, WebCore::DragData*)
{

}

void DragClient::startDrag(WebCore::DragImageRef dragImage, const WebCore::IntPoint& dragImageOrigin, const WebCore::IntPoint& eventPos, WebCore::Clipboard*, WebCore::Frame*, bool linkDrag /*= false*/)
{

}

WebCore::DragSourceAction DragClient::dragSourceActionMaskForPoint(const WebCore::IntPoint& windowPoint)
{
    return WebCore::DragSourceActionAny;
}

WebCore::DragDestinationAction DragClient::actionMaskForDrag(WebCore::DragData*)
{
    return WebCore::DragDestinationActionAny;
}





};//namespace wke