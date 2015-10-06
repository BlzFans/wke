//////////////////////////////////////////////////////////////////////////


#include "wkeFrameLoaderClient.h"
#include "wkeString.h"


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
    :m_webView(webView)
    ,m_page(page)
    ,m_frame(NULL)
    ,m_loadFailed(false)
    ,m_loaded(false)
    ,m_documentReady(false)
{

}


void FrameLoaderClient::frameLoaderDestroyed()
{
    dbgMsg(L"frameLoaderDestroyed\n");
    delete this;
}

void FrameLoaderClient::setUserAgent(const WTF::String& str)
{
    m_userAgent = str;
}

bool FrameLoaderClient::isDocumentReady() const
{
    return m_documentReady;
}

bool FrameLoaderClient::isLoaded() const
{
    return m_loaded;
}

bool FrameLoaderClient::isLoadFailed() const
{
    return m_loadFailed;
}

WebCore::Frame* FrameLoaderClient::frame() const
{
    return m_frame;
}

void FrameLoaderClient::setFrame(WebCore::Frame* frame)
{
    m_frame = frame;
}

PassRefPtr<WebCore::FrameNetworkingContext> FrameLoaderClient::createNetworkingContext()
{
    return FrameNetworkingContext::create(m_frame);
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

    RefPtr<WebCore::PluginView> pluginView = WebCore::PluginView::create(m_frame, pluginSize, element, url, newParamNames, newParamValues, mimeType, loadManually);
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
    FrameLoaderClient* loader = new FrameLoaderClient(m_webView, m_page);
    RefPtr<WebCore::Frame> childFrame = WebCore::Frame::create(m_page, ownerElement, loader);
    loader->setFrame(childFrame.get());

    m_frame->tree()->appendChild(childFrame);
    childFrame->tree()->setName(name);
    childFrame->init();

    m_frame->loader()->loadURLIntoChildFrame(url, referrer, childFrame.get());
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
    bool transparent = m_webView->isTransparent();
    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparent : WebCore::Color::white;

    WebCore::IntSize size(m_webView->width(), m_webView->height());
    m_frame->createView(size, backgroundColor, transparent, WebCore::IntSize(), false);
}

void FrameLoaderClient::transitionToCommittedFromCachedFrame(WebCore::CachedFrame*)
{

}

void FrameLoaderClient::savePlatformDataToCachedFrame(WebCore::CachedFrame*)
{

}

String FrameLoaderClient::userAgent(const WebCore::KURL&)
{
    if (m_userAgent.isEmpty())
    {
        return "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.2 (KHTML, like Gecko) Safari/535.2 wke/1.2";
    }
    else
    {
        return m_userAgent;
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
    if (m_frame->document()->isMediaDocument())
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
    m_loaded = false;
    m_loadFailed = false;
    m_documentReady = false;
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
    (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
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

void FrameLoaderClient::dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction& action, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>)
{
    // cexer 实现URL访问控制
    wke::CWebViewHandler& handler = m_webView->m_handler;

    // 默认允许加载
    if (!handler.navigationCallback)
    {
        (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
        return;
    }

    wkeNavigationType type = (wkeNavigationType)action.type();
    wke::CString url = action.url().string();
    if (handler.navigationCallback(m_webView, handler.navigationCallbackParam, type, &url))
        (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
    else
        (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyIgnore);
}

void FrameLoaderClient::dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction&, const WebCore::ResourceRequest&, PassRefPtr<WebCore::FormState>, const WTF::String& frameName)
{
    (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchDecidePolicyForResponse(WebCore::FramePolicyFunction function, const WebCore::ResourceResponse&, const WebCore::ResourceRequest&)
{
    (m_frame->loader()->policyChecker()->*function)(WebCore::PolicyUse);
}

void FrameLoaderClient::dispatchShow()
{

}

WebCore::Frame* FrameLoaderClient::dispatchCreatePage(const WebCore::NavigationAction& action)
{
    // 实现新窗口控制
    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (!handler.createViewCallback)
        return m_page->mainFrame();

    wkeNavigationType type = (wkeNavigationType)action.type();
    wke::CString url = action.url().string();
    wkeWindowFeatures windowFeatures;
    windowFeatures.x = CW_USEDEFAULT;
    windowFeatures.y = CW_USEDEFAULT;
    windowFeatures.width = CW_USEDEFAULT;
    windowFeatures.height = CW_USEDEFAULT;
    windowFeatures.locationBarVisible = true;
    windowFeatures.menuBarVisible = true;
    windowFeatures.resizable = true;
    windowFeatures.statusBarVisible = true;
    windowFeatures.toolBarVisible = true;
    windowFeatures.fullscreen = false;
    
    wke::CWebView* createdWebView = handler.createViewCallback(m_webView, handler.createViewCallbackParam, type, &url, &windowFeatures);
    if (!createdWebView)
        return m_page->mainFrame();
    
    return createdWebView->mainFrame();
}

void FrameLoaderClient::dispatchDidFirstVisuallyNonEmptyLayout()
{

}

void FrameLoaderClient::dispatchDidFirstLayout()
{

}

void FrameLoaderClient::dispatchDidFinishLoad()
{
    m_loaded = true;

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (handler.loadingFinishCallback)
    {
        wkeLoadingResult result = WKE_LOADING_SUCCEEDED;
        wke::CString url = m_frame->document()->url().string();
        handler.loadingFinishCallback(m_webView, handler.loadingFinishCallbackParam, &url, result, NULL);
    }
}

void FrameLoaderClient::dispatchDidFinishDocumentLoad()
{
    m_documentReady = true;

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (handler.documentReadyCallback)
        handler.documentReadyCallback(m_webView, handler.documentReadyCallbackParam);
}

void FrameLoaderClient::dispatchDidFailLoad(const WebCore::ResourceError& error)
{
    m_loadFailed = true;

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (handler.loadingFinishCallback)
    {
        wkeLoadingResult result = WKE_LOADING_FAILED;
        wke::CString failedReason = error.localizedDescription();
        wke::CString url = error.failingURL();

        if (error.isCancellation())
            result = WKE_LOADING_CANCELED;

        handler.loadingFinishCallback(m_webView, handler.loadingFinishCallbackParam, &url, result, &failedReason);
    }
}

void FrameLoaderClient::dispatchDidFailProvisionalLoad(const WebCore::ResourceError& error)
{
    m_loadFailed = true;

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (handler.loadingFinishCallback)
    {
        wkeLoadingResult result = WKE_LOADING_FAILED;
        wke::CString failedReason = error.localizedDescription();
        wke::CString url = error.failingURL();

        if (error.isCancellation())
            result = WKE_LOADING_CANCELED;

        handler.loadingFinishCallback(m_webView, handler.loadingFinishCallbackParam, &url, result, &failedReason);
    }
}

void FrameLoaderClient::dispatchDidCommitLoad()
{
    if (m_frame == NULL || m_frame != m_page->mainFrame())
        return;

    wke::CWebViewHandler& handler = m_webView->m_handler;
    if (handler.urlChangedCallback == NULL)
        return;

    WebCore::DocumentLoader* loader = m_frame->loader()->documentLoader();
    if (loader == NULL)
        return;

    const WebCore::ResourceRequest& request = loader->request();
    const WebCore::KURL& url = request.firstPartyForCookies();

    wke::CString string(url.string());
    handler.urlChangedCallback(m_webView, handler.urlChangedCallbackParam, &string);
}

void FrameLoaderClient::dispatchDidChangeIcons(WebCore::IconType type)
{

}

void FrameLoaderClient::dispatchDidReceiveTitle(const WebCore::StringWithDirection& title)
{
    if (m_frame == m_page->mainFrame())
    {
        wke::CWebViewHandler& handler = m_webView->m_handler;
        if (handler.titleChangedCallback)
        {
            wke::CString string(title.string());
            handler.titleChangedCallback(m_webView, handler.titleChangedCallbackParam, &string);
        }
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
    request.addHTTPHeaderField("Accept-Charset", m_page->settings()->defaultTextEncodingName() + ",utf-8;q=0.7,*;q=0.3");
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