//////////////////////////////////////////////////////////////////////////

#include "wkeContextMenuClient.h"

//////////////////////////////////////////////////////////////////////////

namespace wke
{


ContextMenuClient::~ContextMenuClient()
{

}
void ContextMenuClient::contextMenuDestroyed()
{
    delete this;
}

void ContextMenuClient::stopSpeaking()
{

}

void ContextMenuClient::speak(const String&)
{

}

bool ContextMenuClient::isSpeaking()
{
    return false;
}

void ContextMenuClient::lookUpInDictionary(WebCore::Frame*)
{

}

void ContextMenuClient::searchWithGoogle(const WebCore::Frame*)
{

}

void ContextMenuClient::downloadURL(const WebCore::KURL& url)
{

}

void ContextMenuClient::contextMenuItemSelected(WebCore::ContextMenuItem* item, const WebCore::ContextMenu* parentMenu)
{

}

PassOwnPtr<WebCore::ContextMenu> ContextMenuClient::customizeMenu(PassOwnPtr<WebCore::ContextMenu> menu)
{
    return menu;
}




};//namespace wke