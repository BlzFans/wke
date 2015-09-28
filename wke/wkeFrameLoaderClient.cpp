//////////////////////////////////////////////////////////////////////////


#include "wkeFrameLoaderClient.h"


//////////////////////////////////////////////////////////////////////////



namespace wke
{


FrameNetworkingContext::FrameNetworkingContext(WebCore::Frame* frame) 
    : WebCore::FrameNetworkingContext(frame)
{

}

PassRefPtr<FrameNetworkingContext> FrameNetworkingContext::create(WebCore::Frame* frame)
{
    return adoptRef(new FrameNetworkingContext(frame));
}


WebCore::ResourceError FrameNetworkingContext::blockedError(const WebCore::ResourceRequest& request) const 
{
    return frame()->loader()->client()->blockedError(request);
}

WTF::String FrameNetworkingContext::referrer() const 
{
    return frame()->loader()->referrer();
}

WTF::String FrameNetworkingContext::userAgent() const 
{
    return m_userAgent;
}



FrameLoaderClient::FrameLoaderClient(CWebView* webView, WebCore::Page* page) 
    :webView_(webView)
    ,page_(page)
    ,frame_(NULL)
    ,loadFailed_(false)
    ,loaded_(false)
    ,documentReady_(false)
{

}


void FrameLoaderClient::frameLoaderDestroyed()
{
    dbgMsg(L"frameLoaderDestroyed\n");
    delete this;
}

void FrameLoaderClient::setUserAgent(const WTF::String& str)
{
    userAgent_ = str;
}

bool FrameLoaderClient::isDocumentReady() const
{
    return documentReady_;
}

bool FrameLoaderClient::isLoaded() const
{
    return loaded_;
}

bool FrameLoaderClient::isLoadFailed() const
{
    return loadFailed_;
}

WebCore::Frame* FrameLoaderClient::frame() const
{
    return frame_;
}

void FrameLoaderClient::setFrame(WebCore::Frame* frame)
{
    frame_ = frame;
}

PassRefPtr<WebCore::FrameNetworkingContext> FrameLoaderClient::createNetworkingContext()
{
    return FrameNetworkingContext::create(frame_);
}

void FrameLoaderClient::registerForIconNotification(bool listen /*= true*/)
{

}


#if USE(V8)
bool FrameLoaderClient::allowScriptExtension(const WTF::String& extensionName, int extensionGroup)
{

}

void FrameLoaderClient::didCreateIsolatedScriptContext()
{

}

void FrameLoaderClient::didDestroyScriptContextForFrame()
{

}

void FrameLoaderClient::didCreateScriptContextForFrame()
{

}
#endif


void FrameLoaderClient::didPerformFirstNavigation() const 
{

}

void FrameLoaderClient::documentElementAvailable()
{

}

void FrameLoaderClient::dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld*)
{

}

WTF::String FrameLoaderClient::overrideMediaType() const 
{
    return WTF::String();
}

WebCore::ObjectContentType FrameLoaderClient::objectContentType(const WebCore::KURL& url, const WTF::String& mimeType, bool shouldPreferPlugInsForImages)
{
    return WebCore::FrameLoader::defaultObjectContentType(url, mimeType, shouldPreferPlugInsForImages);
}

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
void FrameLoaderClient::showMediaPlayerProxyPlugin(WebCore::Widget*)
{

}

void FrameLoaderClient::hideMediaPlayerProxyPlugin(WebCore::Widget*)
{

}

PassRefPtr<WebCore::Widget> FrameLoaderClient::createMediaPlayerProxyPlugin(const WebCore::IntSize&, WebCore::HTMLMediaElement*, const WebCore::KURL&, const Vector<WTF::String>&, const Vector<WTF::String>&, const WTF::String&)
{

}
#endif


PassRefPtr<WebCore::Widget> FrameLoaderClient::createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const Vector<WTF::String>& paramNames, const Vector<WTF::String>& paramValues)
{
    return 0;
}

void FrameLoaderClient::redirectDataToPlugin(WebCore::Widget* pluginWidget)
{

}

PassRefPtr<WebCore::Widget> FrameLoaderClient::createPlugin(const WebCore::IntSize& pluginSize, WebCore::HTMLPlugInElement* element, const WebCore::KURL& url, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually)
{
    Vector<String> newParamNames = paramNames;
    Vector<String> newParamValues = paramValues;

    unsigned int i = 0;
    for (; i < newParamNames.size(); i++) {
        if (equalIgnoringCase(paramNames[i], "wmode"))
            break;
    }

    if (i == newParamNames.size()) {
        newParamNames.append("wmode");
        newParamValues.append("opaque");
    }

    if (!equalIgnoringCase(newParamValues[i], "opaque") &&
        !equalIgnoringCase(newParamValues[i], "transparent")) {
            newParamValues[i] = "opaque";
    }

    RefPtr<WebCore::PluginView> pluginView = WebCore::PluginView::create(frame_, pluginSize, element, url, newParamNames, newParamValues, mimeType, loadManually);
    if (pluginView->status() == WebCore::PluginStatusLoadedSuccessfully)
        return pluginView;

    return 0;
}

void FrameLoaderClient::transferLoadingResourceFromPage(WebCore::ResourceLoader*, const WebCore::ResourceRequest&, WebCore::Page*)
{

}

void FrameLoaderClient::didTransferChildFrameToNewDocument(WebCore::Page*)
{

}

PassRefPtr<WebCore::Frame> FrameLoaderClient::createFrame(const WebCore::KURL& url, const WTF::String& name, WebCore::HTMLFrameOwnerElement* ownerElement, const WTF::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    FrameLoaderClient* loader = new FrameLoaderClient(webView_, page_);
    RefPtr<WebCore::Frame> childFrame = WebCore::Frame::create(page_, ownerElement, loader);
    loader->setFrame(childFrame.get());

    frame_->tree()->appendChild(childFrame);
    childFrame->tree()->setName(name);
    childFrame->init();

    frame_->loader()->loadURLIntoChildFrame(url, referrer, childFrame.get());
    if (!childFrame->tree()->parent())
        return 0;

    return childFrame.release();
}

void FrameLoaderClient::download(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)
{

}

bool FrameLoaderClient::canCachePage() const 
{
    return false;
}

void FrameLoaderClient::dispatchDidBecomeFrameset(bool)
{

}

void FrameLoaderClient::didRestoreFromPageCache()
{

}

void FrameLoaderClient::didSaveToPageCache()
{

}

void FrameLoaderClient::transitionToCommittedForNewPage()
{
    bool transparent = webView_->isTransparent();
    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparent : WebCore::Color::white;

    WebCore::IntSize size(webView_->width(), webView_->height());
    frame_->createView(size, backgroundColor, transparent, WebCore::IntSize(), false);
}

void FrameLoaderClient::transitionToCommittedFromCachedFrame(WebCore::CachedFrame*)
{

}

void FrameLoaderClient::savePlatformDataToCachedFrame(WebCore::CachedFrame*)
{

}

String FrameLoaderClient::userAgent(const WebCore::KURL&)
{
    if (userAgent_.isEmpty())
    {
        return "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.2 (KHTML, like Gecko) Safari/535.2 wke/1.2";
    }
    else
    {
        return userAgent_;
    }
}

void FrameLoaderClient::setTitle(const WebCore::StringWithDirection& title, const WebCore::KURL&)
{

}

PassRefPtr<WebCore::DocumentLoader> FrameLoaderClient::createDocumentLoader(const WebCore::ResourceRequest& request, const WebCore::SubstituteData& data)
{
    return WebCore::DocumentLoader::create(request, data);
}

void FrameLoaderClient::prepareForDataSourceReplacement()
{

}

void FrameLoaderClient::didFinishLoad()
{

}

void FrameLoaderClient::provisionalLoadStarted()
{

}

void FrameLoaderClient::restoreViewState()
{

}

void FrameLoaderClient::saveViewStateToItem(WebCore::HistoryItem*)
{

}

void FrameLoaderClient::frameLoadCompleted()
{

}

WTF::String FrameLoaderClient::generatedMIMETypeForURLScheme(const WTF::String& URLScheme) const 
{
    return String();
}

bool FrameLoaderClient::representationExistsForURLScheme(const WTF::String& URLScheme) const 
{
    return false;
}

bool FrameLoaderClient::canShowMIMETypeAsHTML(const String& MIMEType) const 
{
    return true;
}

bool FrameLoaderClient::canShowMIMEType(const String& MIMEType) const 
{
    return true;
}

bool FrameLoaderClient::canHandleRequest(const WebCore::ResourceRequest&) const 
{
    return true;
}

bool FrameLoaderClient::shouldFallBack(const WebCore::ResourceError&)
{
    return false;
}

WebCore::ResourceError FrameLoaderClient::pluginWillHandleLoadError(const WebCore::ResourceResponse& response)
{
    return WebCore::ResourceError(String(WebKitErrorDomain), WebKitErrorPlugInWillHandleLoad, response.url().string(), String());
}

WebCore::ResourceError FrameLoaderClient::fileDoesNotExistError(const WebCore::ResourceResponse& response)
{
    return WebCore::ResourceError();
}

WebCore::ResourceError FrameLoaderClient::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
{
    return WebCore::ResourceError();
}

WebCore::ResourceError FrameLoaderClient::interruptedForPolicyChangeError(const WebCore::ResourceRequest& request)
{
    return WebCore::ResourceError(String(WebKitErrorDomain), WebKitErrorFrameLoadInterruptedByPolicyChange, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClient::cannotShowURLError(const WebCore::ResourceRequest& request)
{
    return WebCore::ResourceError(String(WebKitErrorDomain), WebKitErrorCannotShowURL, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClient::blockedError(const WebCore::ResourceRequest& request)
{
    return WebCore::ResourceError(String(WebKitErrorDomain), WebKitErrorCannotUseRestrictedPort, request.url().string(), String());
}

WebCore::ResourceError FrameLoaderClient::cancelledError(const WebCore::ResourceRequest& request)
{
    return WebCore::ResourceError(String(WebURLErrorDomain), WebURLErrorCancelled, request.url().string(), String());
}

void FrameLoaderClient::didRunInsecureContent(WebCore::SecurityOrigin*, const WebCore::KURL&)
{

}

void FrameLoaderClient::didDisplayInsecureContent()
{

}

bool FrameLoaderClient::shouldStopLoadingForHistoryItem(WebCore::HistoryItem*) const 
{
    return true;
}

bool FrameLoaderClient::shouldGoToHistoryItem(WebCore::HistoryItem*) const 
{
    return true;
}

void FrameLoaderClient::updateGlobalHistoryRedirectLinks()
{

}

void FrameLoaderClient::updateGlobalHistory()
{

}

void FrameLoaderClient::finishedLoading(WebCore::DocumentLoader* loader)
{
    // Telling the frame we received some data and passing 0 as the data is our
    // way to get work done that is normally done when the first bit of data is
    // received, even for the case of a document with no data (like about:blank)
    committedLoad(loader, 0, 0);
}

void FrameLoaderClient::committedLoad(WebCore::DocumentLoader* loader, const char* data, int length)
{
    loader->commitData(data, length);
    if (frame_->document()->isMediaDocument())
        loader->cancelMainResourceLoad(pluginWillHandleLoadError(loader->response()));
}

void FrameLoaderClient::didChangeTitle(WebCore::DocumentLoader*)
{

}

void FrameLoaderClient::willChangeTitle(WebCore::DocumentLoader*)
{

}

void FrameLoaderClient::startDownload(const WebCore::ResourceRequest&, const String& suggestedName /*= String()*/)
{
    dbgMsg(L"startDownload\n");
}

void FrameLoaderClient::setMainFrameDocumentReady(bool)
{

}

void FrameLoaderClient::postProgressFinishedNotification()
{

}

void FrameLoaderClient::postProgressEstimateChangedNotification()
{

}

void FrameLoaderClient::postProgressStartedNotification()
{
    loaded_ = false;
    loadFailed_ = false;
    documentReady_ = false;
}

void FrameLoaderClient::setMainDocumentError(WebCore::DocumentLoader*, const WebCore::ResourceError&)
{

}

void FrameLoaderClient::revertToProvisionalState(WebCore::DocumentLoader*)
{

}

void FrameLoaderClient::dispatchDidLoadMainResource(WebCore::DocumentLoader*)
{

}

void FrameLoaderClient::dispatchWillSubmitForm(WebCore::FramePolicyFunction function, PassRefPtr<WebCore::FormState> formState)
{
    (frame_->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchWillSendSubmitEvent(WebCore::HTMLFormElement*)
{

}

void FrameLoaderClient::dispatchUnableToImplementPolicy(const WebCore::ResourceError&)
{

}

void FrameLoaderClient::cancelPolicyCheck()
{

}

void FrameLoaderClient::dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>)
{
    (frame_->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>, const WTF::String& frameName)
{
    (frame_->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchDecidePolicyForResponse(WebCore::FramePolicyFunction function, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&)
{
    (frame_->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchShow()
{

}

WebCore::Frame* FrameLoaderClient::dispatchCreatePage(const WebCore::NavigationAction&)
{
    return page_->mainFrame();
}

void FrameLoaderClient::dispatchDidFirstVisuallyNonEmptyLayout()
{

}

void FrameLoaderClient::dispatchDidFirstLayout()
{

}

void FrameLoaderClient::dispatchDidFinishLoad()
{
    loaded_ = true;
}

void FrameLoaderClient::dispatchDidFinishDocumentLoad()
{
    documentReady_ = true;
}

void FrameLoaderClient::dispatchDidFailLoad(const WebCore::ResourceError&)
{
    loadFailed_ = true;
}

void FrameLoaderClient::dispatchDidFailProvisionalLoad(const WebCore::ResourceError&)
{
    loadFailed_ = true;
}

void FrameLoaderClient::dispatchDidCommitLoad()
{
    if (frame_ == NULL || frame_ != page_->mainFrame())
        return;

    wkeViewHandler* handler = webView_->handler();
    if (handler == NULL || handler->onURLChanged == NULL)
        return;

    WebCore::DocumentLoader* loader = frame_->loader()->documentLoader();
    if (loader == NULL)
        return;

    const WebCore::ResourceRequest& request = loader->request();
    const WebCore::KURL& url = request.firstPartyForCookies();

    handler->onURLChanged(handler, webView_, (const wkeString)&url.string());
}

void FrameLoaderClient::dispatchDidChangeIcons(WebCore::IconType type)
{

}

void FrameLoaderClient::dispatchDidReceiveTitle(const WebCore::StringWithDirection& title)
{
    if (frame_ == page_->mainFrame())
    {
        wkeViewHandler* handler = webView_->handler();
        if (handler && handler->onTitleChanged)
            handler->onTitleChanged(handler, webView_, (const wkeString)&title.string());
    }
}

void FrameLoaderClient::dispatchDidStartProvisionalLoad()
{

}

void FrameLoaderClient::dispatchDidReceiveIcon()
{

}

void FrameLoaderClient::dispatchWillClose()
{

}

void FrameLoaderClient::dispatchDidPopStateWithinPage()
{

}

void FrameLoaderClient::dispatchDidReplaceStateWithinPage()
{

}

void FrameLoaderClient::dispatchDidPushStateWithinPage()
{

}

void FrameLoaderClient::dispatchDidChangeLocationWithinPage()
{

}

void FrameLoaderClient::dispatchWillPerformClientRedirect(const WebCore::KURL&, double interval, double fireDate)
{

}

void FrameLoaderClient::dispatchDidCancelClientRedirect()
{

}

void FrameLoaderClient::dispatchDidReceiveServerRedirectForProvisionalLoad()
{

}

void FrameLoaderClient::dispatchDidHandleOnloadEvents()
{

}

bool FrameLoaderClient::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int length)
{
    return true;
}

void FrameLoaderClient::dispatchDidFailLoading(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceError&)
{

}

void FrameLoaderClient::dispatchDidFinishLoading(WebCore::DocumentLoader*, unsigned long identifier)
{

}

void FrameLoaderClient::dispatchDidReceiveContentLength(WebCore::DocumentLoader*, unsigned long identifier, int lengthReceived)
{

}

void FrameLoaderClient::dispatchDidReceiveResponse(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ResourceResponse& response)
{

}

#if USE(PROTECTION_SPACE_AUTH_CALLBACK)
bool FrameLoaderClient::canAuthenticateAgainstProtectionSpace(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::ProtectionSpace&)
{

}
#endif

void FrameLoaderClient::dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&)
{

}

void FrameLoaderClient::dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader*, unsigned long identifier, const WebCore::AuthenticationChallenge&)
{

}

bool FrameLoaderClient::shouldUseCredentialStorage(WebCore::DocumentLoader*, unsigned long identifier)
{
    return true;
}

void FrameLoaderClient::dispatchWillSendRequest(WebCore::DocumentLoader*, unsigned long identifier, WebCore::ResourceRequest& request, const WebCore::ResourceResponse& redirectResponse)
{
    request.addHTTPHeaderField("Accept-Language", WebCore::defaultLanguage() + ",en,*");
    request.addHTTPHeaderField("Accept-Charset", page_->settings()->defaultTextEncodingName() + ",utf-8;q=0.7,*;q=0.3");
}

void FrameLoaderClient::assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&)
{

}

void FrameLoaderClient::detachedFromParent3()
{

}

void FrameLoaderClient::detachedFromParent2()
{

}

void FrameLoaderClient::setCopiesOnScroll()
{

}

void FrameLoaderClient::forceLayoutForNonHTML()
{

}

void FrameLoaderClient::forceLayout()
{

}

void FrameLoaderClient::makeRepresentation(WebCore::DocumentLoader*)
{

}

bool FrameLoaderClient::hasWebView() const 
{
    return true;
}


};//namespace wke