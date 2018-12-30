//////////////////////////////////////////////////////////////////////////


#include "wkePlatformStrategies.h"


//////////////////////////////////////////////////////////////////////////



namespace wke
{



void PlatformStrategies::initialize()
{
    DEFINE_STATIC_LOCAL(PlatformStrategies, platformStrategies, ());
    setPlatformStrategies(&platformStrategies);
}

PlatformStrategies::PlatformStrategies()
{

}

WebCore::VisitedLinkStrategy* PlatformStrategies::createVisitedLinkStrategy()
{
    return this;
}

WebCore::PluginStrategy* PlatformStrategies::createPluginStrategy()
{
    return this;
}

WebCore::CookiesStrategy* PlatformStrategies::createCookiesStrategy()
{
    return this;
}


void PlatformStrategies::addVisitedLink(WebCore::Page* page, WebCore::LinkHash hash)
{
    page->group().addVisitedLinkHash(hash);
}

bool PlatformStrategies::isLinkVisited(WebCore::Page* page, WebCore::LinkHash hash, const WebCore::KURL &,const WTF::AtomicString &)
{
    return page->group().isLinkVisited(hash);
}

void PlatformStrategies::getPluginInfo(const WebCore::Page*, Vector<WebCore::PluginInfo>& outPlugins)
{
    const Vector<WebCore::PluginPackage*>& plugins = WebCore::PluginDatabase::installedPlugins()->plugins();

    outPlugins.resize(plugins.size());

    for (size_t i = 0; i < plugins.size(); ++i) {
        WebCore::PluginPackage* package = plugins[i];

        WebCore::PluginInfo info;
        info.name = package->name();
        info.file = package->fileName();
        info.desc = package->description();

        const WebCore::MIMEToDescriptionsMap& mimeToDescriptions = package->mimeToDescriptions();

        info.mimes.reserveCapacity(mimeToDescriptions.size());

        WebCore::MIMEToDescriptionsMap::const_iterator end = mimeToDescriptions.end();
        for (WebCore::MIMEToDescriptionsMap::const_iterator it = mimeToDescriptions.begin(); it != end; ++it) {
            WebCore::MimeClassInfo mime;

            mime.type = it->first;
            mime.desc = it->second;
            mime.extensions = package->mimeToExtensions().get(mime.type);

            info.mimes.append(mime);
        }

        outPlugins[i] = info;
    }
}

void PlatformStrategies::refreshPlugins()
{
    WebCore::PluginDatabase::installedPlugins()->refresh();
}

void PlatformStrategies::notifyCookiesChanged()
{

}




};//namespace wke