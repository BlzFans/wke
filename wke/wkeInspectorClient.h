#ifndef WKE_INSPECTOR_CLIENT_H
#define WKE_INSPECTOR_CLIENT_H


//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/InspectorClient.h>


//////////////////////////////////////////////////////////////////////////


namespace wke
{




class InspectorClient : public WebCore::InspectorClient 
{
    virtual void inspectorDestroyed()  override;

    virtual void openInspectorFrontend(WebCore::InspectorController*)  override;

    virtual void highlight()  override;
    virtual void hideHighlight()  override;

    virtual bool sendMessageToFrontend(const String& message) override;
};




};//namespace wke




#endif//#ifndef WKE_INSPECTOR_CLIENT_H