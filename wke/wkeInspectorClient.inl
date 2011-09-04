#include <WebCore/InspectorClient.h>

namespace wke
{
    class InspectorClient : public WebCore::InspectorClient {

        virtual void inspectorDestroyed()  override
        {
            delete this;
        }

        virtual void openInspectorFrontend(WebCore::InspectorController*)  override
        {
        }

        virtual void highlight()  override
        {
        }

        virtual void hideHighlight()  override
        {
        }

        virtual bool sendMessageToFrontend(const String& message) override
        {
            return false;
        }
    };

}