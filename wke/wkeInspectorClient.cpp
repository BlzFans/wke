//////////////////////////////////////////////////////////////////////////


#include "wkeInspectorClient.h"


//////////////////////////////////////////////////////////////////////////



namespace wke
{





void InspectorClient::inspectorDestroyed()
{
    delete this;
}


bool InspectorClient::sendMessageToFrontend(const String& message)
{
    return false;
}

void InspectorClient::hideHighlight()
{

}

void InspectorClient::highlight()
{

}

void InspectorClient::openInspectorFrontend(WebCore::InspectorController*)
{

}





};//namespace wke