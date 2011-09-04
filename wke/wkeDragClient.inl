
#include <WebCore/DragClient.h>
#include <WebCore/DragActions.h>

namespace wke
{
    class DragClient : public WebCore::DragClient
    {
        virtual ~DragClient() {};

        virtual void willPerformDragDestinationAction(WebCore::DragDestinationAction, WebCore::DragData*) override
        {
        }

        virtual void willPerformDragSourceAction(WebCore::DragSourceAction, const WebCore::IntPoint&, WebCore::Clipboard*) override
        {
        }

        virtual WebCore::DragDestinationAction actionMaskForDrag(WebCore::DragData*) override
        {
            return WebCore::DragDestinationActionAny;
        }

        //We work in window rather than view coordinates here
        virtual WebCore::DragSourceAction dragSourceActionMaskForPoint(const WebCore::IntPoint& windowPoint) override
        {
            return WebCore::DragSourceActionAny;
        }
        
        virtual void startDrag(WebCore::DragImageRef dragImage, const WebCore::IntPoint& dragImageOrigin, const WebCore::IntPoint& eventPos, WebCore::Clipboard*, WebCore::Frame*, bool linkDrag = false) override
        {
        }
        
        virtual void dragControllerDestroyed() override
        {
            delete this;
        }
    };
}