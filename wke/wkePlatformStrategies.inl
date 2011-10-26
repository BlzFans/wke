#include <WebCore/PlatformStrategies.h>
#include <WebCore/PluginStrategy.h>
#include <WebCore/VisitedLinkStrategy.h>
#include <WebCore/PluginDatabase.h>
#include <WebCore/CookiesStrategy.h>

namespace wke {

class PlatformStrategies : public WebCore::PlatformStrategies, private WebCore::CookiesStrategy, private WebCore::PluginStrategy, private WebCore::VisitedLinkStrategy {
public:
    static void initialize()
    {
        DEFINE_STATIC_LOCAL(PlatformStrategies, platformStrategies, ());
        setPlatformStrategies(&platformStrategies);
    }
    
private:
    PlatformStrategies()
    {
    }

	virtual WebCore::CookiesStrategy* createCookiesStrategy() override
	{
		return this;
	}

    // WebCore::PlatformStrategies
    virtual WebCore::PluginStrategy* createPluginStrategy() override
    {
        return this;
    }

    virtual WebCore::VisitedLinkStrategy* createVisitedLinkStrategy() override
    {
        return this;
    }

	virtual void notifyCookiesChanged() override
	{
	}

    // WebCore::PluginStrategy
    virtual void refreshPlugins() override
    {
        WebCore::PluginDatabase::installedPlugins()->refresh();
    }

    virtual void getPluginInfo(const WebCore::Page*, Vector<WebCore::PluginInfo>& outPlugins) override
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

    virtual bool isLinkVisited(WebCore::Page* page, WebCore::LinkHash hash, const WebCore::KURL &,const WTF::AtomicString &) override
    {
        return page->group().isLinkVisited(hash);
    }

    virtual void addVisitedLink(WebCore::Page* page, WebCore::LinkHash hash) override
    {
        page->group().addVisitedLinkHash(hash);
    }
};

}