#ifndef WKE_PLATFORM_STRATEGIES_H
#define WKE_PLATFORM_STRATEGIES_H


//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/PlatformStrategies.h>
#include <WebCore/PluginStrategy.h>
#include <WebCore/VisitedLinkStrategy.h>
#include <WebCore/PluginDatabase.h>
#include <WebCore/CookiesStrategy.h>
#include <WebCore/Page.h>
#include <WebCore/PageGroup.h>


//////////////////////////////////////////////////////////////////////////




namespace wke 
{





class PlatformStrategies : public WebCore::PlatformStrategies
                          , private WebCore::CookiesStrategy
                          , private WebCore::PluginStrategy
                          , private WebCore::VisitedLinkStrategy 
{
public:
    static void initialize();

private:
    PlatformStrategies();
    virtual WebCore::CookiesStrategy* createCookiesStrategy() override;

    // WebCore::PlatformStrategies
    virtual WebCore::PluginStrategy* createPluginStrategy() override;
    virtual WebCore::VisitedLinkStrategy* createVisitedLinkStrategy() override;

    virtual void notifyCookiesChanged() override;

    // WebCore::PluginStrategy
    virtual void refreshPlugins() override;
    virtual void getPluginInfo(const WebCore::Page*, Vector<WebCore::PluginInfo>& outPlugins) override;
    virtual bool isLinkVisited(WebCore::Page* page, WebCore::LinkHash hash, const WebCore::KURL &,const WTF::AtomicString &) override;
    virtual void addVisitedLink(WebCore::Page* page, WebCore::LinkHash hash) override;
};





};//namespace wke





#endif //#ifndef WKE_PLATFORM_STRATEGIES_H