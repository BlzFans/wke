#ifndef WKE_FRAME_LOADER_CLIENT_H
#define WKE_FRAME_LOADER_CLIENT_H


//////////////////////////////////////////////////////////////////////////


#include <WebCore/config.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/PluginData.h>
#include <WebCore/HTMLFrameOwnerElement.h>
#include <WebCore/PluginView.h>
#include <WebCore/HTMLPlugInElement.h>
#include <WebCore/FrameNetworkingContext.h>
#include <WebCore/Language.h>
#include <WebCore/FrameLoaderClient.h>
#include <WebCore/Frame.h>
#include <WebCore/Page.h>
#include <WebCore/Settings.h>
#include <WebCore/FrameLoaderStateMachine.h>
#include <WebCore/FormState.h>
#include <WebCore/HTMLFormElement.h>


#include "wkeWebView.h"
#include "wkeDebug.h"


//////////////////////////////////////////////////////////////////////////



namespace wke
{



class FrameNetworkingContext : public WebCore::FrameNetworkingContext 
{
public:
    static PassRefPtr<FrameNetworkingContext> create(WebCore::Frame* frame);

private:
    FrameNetworkingContext(WebCore::Frame* frame);

    virtual WTF::String userAgent() const override;
    virtual WTF::String referrer() const override;

    virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest& request) const override;

    WTF::String m_userAgent;
};




class FrameLoaderClient : public WebCore::FrameLoaderClient
{
public:
    FrameLoaderClient(CWebView* webView, WebCore::Page* page);
    virtual void frameLoaderDestroyed() override;

    virtual bool hasWebView() const override;

    virtual void makeRepresentation(WebCore::DocumentLoader*) override;

    virtual void forceLayout() override;
    virtual void forceLayoutForNonHTML() override;

    virtual void setCopiesOnScroll() override;

    virtual void detachedFromParent2() override;
    virtual void detachedFromParent3() override;

    virtual void assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&) override;
    virtual void dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long identifier, WebCore::ResourceRequest& request, const WebCore::ResourceResponse& redirectResponse) override;

    virtual bool shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier) override;

    virtual void dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&) override;
    virtual void dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&) override;

#if USE(PROTECTION_SPACE_AUTH_CALLBACK)
    virtual bool canAuthenticateAgainstProtectionSpace(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ProtectionSpace&) override;
#endif

    virtual void dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceResponse& response) override;
    virtual void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived) override;
    virtual void dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long identifier) override;
    virtual void dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceError&) override;
    virtual bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length) override;
    virtual void dispatchDidHandleOnloadEvents() override;
    virtual void dispatchDidReceiveServerRedirectForProvisionalLoad() override;
    virtual void dispatchDidCancelClientRedirect() override;
    virtual void dispatchWillPerformClientRedirect(const WebCore::KURL&, double interval, double fireDate) override;

    virtual void dispatchDidChangeLocationWithinPage() override;
    virtual void dispatchDidPushStateWithinPage() override;
    virtual void dispatchDidReplaceStateWithinPage() override;
    virtual void dispatchDidPopStateWithinPage() override;

    virtual void dispatchWillClose() override;
    virtual void dispatchDidReceiveIcon() override;
    virtual void dispatchDidStartProvisionalLoad() override;
    virtual void dispatchDidReceiveTitle(const WebCore::StringWithDirection& title) override;
    virtual void dispatchDidChangeIcons(WebCore::IconType type) override;
    virtual void dispatchDidCommitLoad() override;
    virtual void dispatchDidFailProvisionalLoad(const WebCore::ResourceError&) override;
    virtual void dispatchDidFailLoad(const WebCore::ResourceError&) override;
    virtual void dispatchDidFinishDocumentLoad(WebCore::FrameLoader* loader) override;
    virtual void dispatchDidFinishLoad() override;
    virtual void dispatchDidFirstLayout() override;
    virtual void dispatchDidFirstVisuallyNonEmptyLayout() override;

    virtual WebCore::Frame* dispatchCreatePage(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, PassRefPtr<WebCore::FormState> formState, const WTF::String& frameName) override;

    virtual void dispatchShow() override;
    virtual void dispatchDecidePolicyForResponse(WebCore::FramePolicyFunction function, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&) override;
    virtual void dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>, const WTF::String& frameName) override;
    virtual void dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>) override;

    virtual void cancelPolicyCheck() override;
    virtual void dispatchUnableToImplementPolicy(const WebCore::ResourceError&) override;

    virtual void dispatchWillSendSubmitEvent(WebCore::HTMLFormElement*) override;
    virtual void dispatchWillSubmitForm(WebCore::FramePolicyFunction function, PassRefPtr<WebCore::FormState> formState) override;

    virtual void dispatchDidLoadMainResource(WebCore::DocumentLoader*) override;

    virtual void revertToProvisionalState(WebCore::DocumentLoader*) override;

    virtual void setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&) override;

    virtual void postProgressStartedNotification() override;
    virtual void postProgressEstimateChangedNotification() override;
    virtual void postProgressFinishedNotification() override;

    virtual void setMainFrameDocumentReady(bool) override;

    virtual void startDownload(const WebCore::ResourceRequest&, const String& suggestedName = String()) override;

    virtual void willChangeTitle(WebCore::DocumentLoader*) override;
    virtual void didChangeTitle(WebCore::DocumentLoader*) override;

    virtual void committedLoad(WebCore::DocumentLoader* loader, const char* data, int length) override;
    virtual void finishedLoading(WebCore::DocumentLoader* loader) override;

    virtual void updateGlobalHistory() override;
    virtual void updateGlobalHistoryRedirectLinks() override;

    virtual bool shouldGoToHistoryItem(WebCore::HistoryItem*) const override;
    virtual bool shouldStopLoadingForHistoryItem(WebCore::HistoryItem*) const override;

    virtual void didDisplayInsecureContent() override;

    // The indicated security origin has run active content (such as a
    // script) from an insecure source.  Note that the insecure content can
    // spread to other frames in the same origin.
    virtual void didRunInsecureContent(WebCore::SecurityOrigin*, const WebCore::KURL&) override;

#define WebURLErrorDomain TEXT("WebURLErrorDomain")
#define WebKitErrorDomain TEXT("WebKitErrorDomain")
    enum {
        WebURLErrorCancelled = -999,
        WebKitErrorCannotShowURL = 101,
        WebKitErrorFrameLoadInterruptedByPolicyChange = 102,
        WebKitErrorCannotUseRestrictedPort = 103,
        WebKitErrorPlugInWillHandleLoad = 204,
    };

    virtual WebCore::ResourceError cancelledError(const WebCore::ResourceRequest& request) override;
    virtual WebCore::ResourceError blockedError(const WebCore::ResourceRequest& request) override;
    virtual WebCore::ResourceError cannotShowURLError(const WebCore::ResourceRequest& request) override;
    virtual WebCore::ResourceError interruptedForPolicyChangeError(const WebCore::ResourceRequest& request) override;
    virtual WebCore::ResourceError cannotShowMIMETypeError(const WebCore::ResourceResponse& response) override;
    virtual WebCore::ResourceError fileDoesNotExistError(const WebCore::ResourceResponse& response) override;
    virtual WebCore::ResourceError pluginWillHandleLoadError(const WebCore::ResourceResponse& response) override;

    virtual bool shouldFallBack(const WebCore::ResourceError&) override;
    virtual bool canHandleRequest(const WebCore::ResourceRequest&) const override;

    virtual bool canShowMIMEType(const String& MIMEType) const override;
    virtual bool canShowMIMETypeAsHTML(const String& MIMEType) const override;
    virtual bool representationExistsForURLScheme(const WTF::String& URLScheme) const override;
    virtual WTF::String generatedMIMETypeForURLScheme(const WTF::String& URLScheme) const override;

    virtual void frameLoadCompleted() override;

    virtual void saveViewStateToItem(WebCore::HistoryItem*) override;
    virtual void restoreViewState() override;
    virtual void provisionalLoadStarted() override;

    virtual void didFinishLoad() override;

    virtual void prepareForDataSourceReplacement() override;

    virtual PassRefPtr<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest& request, const WebCore::SubstituteData& data) override;

    virtual void setTitle(const WebCore::StringWithDirection& title, const WebCore::KURL&) override;
    virtual String userAgent(const WebCore::KURL&) override;

    virtual void savePlatformDataToCachedFrame(WebCore::CachedFrame*) override;
    virtual void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*) override;
    virtual void transitionToCommittedForNewPage() override;

    virtual void didSaveToPageCache() override;
    virtual void didRestoreFromPageCache() override;
    virtual void dispatchDidBecomeFrameset(bool) override;
    virtual bool canCachePage() const override;
    virtual void download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&) override;

    virtual PassRefPtr<WebCore::Frame> createFrame(const WebCore::KURL& url, const WTF::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
        const WTF::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight) override;

    virtual void didTransferChildFrameToNewDocument(WebCore::Page*) override;
    virtual void transferLoadingResourceFromPage(WebCore::ResourceLoader*, const WebCore::ResourceRequest&, WebCore::Page*) override;

    virtual PassRefPtr<WebCore::Widget> createPlugin(const WebCore::IntSize& pluginSize, WebCore::HTMLPlugInElement* element, const WebCore::KURL& url, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually) override;
    virtual void redirectDataToPlugin(WebCore::Widget* pluginWidget) override;

    virtual PassRefPtr<WebCore::Widget> createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const Vector<WTF::String>& paramNames, const Vector<WTF::String>& paramValues) override;

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    virtual PassRefPtr<WebCore::Widget> createMediaPlayerProxyPlugin(const WebCore::IntSize&, WebCore::HTMLMediaElement*, const WebCore::KURL&, const Vector<WTF::String>&, const Vector<WTF::String>&, const WTF::String&) override;
    virtual void hideMediaPlayerProxyPlugin(WebCore::Widget*) override;
    virtual void showMediaPlayerProxyPlugin(WebCore::Widget*) override;
#endif

    virtual WebCore::ObjectContentType objectContentType(const WebCore::KURL& url, const WTF::String& mimeType, bool shouldPreferPlugInsForImages) override;
    virtual WTF::String overrideMediaType() const override;

    virtual void dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld*) override;
    virtual void documentElementAvailable() override;

    virtual void didPerformFirstNavigation() const override;

#if USE(V8)
    virtual void didCreateScriptContextForFrame() override;
    virtual void didDestroyScriptContextForFrame() override;
    virtual void didCreateIsolatedScriptContext() override;
    virtual bool allowScriptExtension(const WTF::String& extensionName, int extensionGroup) override;
#endif

    virtual void registerForIconNotification(bool listen = true) override;
    virtual PassRefPtr<WebCore::FrameNetworkingContext> createNetworkingContext() override;

    void setFrame(WebCore::Frame* frame);
    WebCore::Frame* frame() const;

    bool isLoadFailed() const;
    bool isLoaded() const;
    bool isDocumentReady() const;
    void setUserAgent(const WTF::String& str);

protected:
    CWebView* m_webView;
    WTF::String m_userAgent;

    WebCore::Page* m_page;
    WebCore::Frame* m_frame;

    bool m_loadFailed;
    bool m_loaded;
    bool m_documentReady;
};





};//namespace wke;



#endif//#ifndef WKE_FRAME_LOADER_CLIENT_H