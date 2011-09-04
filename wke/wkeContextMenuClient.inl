#include <WebCore/ContextMenuClient.h>
#include <WebCore/ContextMenu.h>

namespace wke
{
    class ContextMenuClient : public WebCore::ContextMenuClient {
    public:
        virtual ~ContextMenuClient() {  }

        virtual void contextMenuDestroyed() override
        {
            delete this;
        }
        
        virtual PassOwnPtr<WebCore::ContextMenu> customizeMenu(PassOwnPtr<WebCore::ContextMenu> menu) override
        {
            return menu;
        }

        virtual void contextMenuItemSelected(WebCore::ContextMenuItem* item, const WebCore::ContextMenu* parentMenu) override
        {
        }

        virtual void downloadURL(const WebCore::KURL& url) override
        {
        }

        virtual void searchWithGoogle(const WebCore::Frame*) override
        {
        }
        
        virtual void lookUpInDictionary(WebCore::Frame*) override
        {
        }

        virtual bool isSpeaking() override
        {
            return false;
        }
        
        virtual void speak(const String&) override
        {
        }

        virtual void stopSpeaking() override
        {
        }
    };
}
