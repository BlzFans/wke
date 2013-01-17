
#include "WebUIApp.h"
#include "WebPanelOverlayElement.h"

Ogre::Overlay* getWebBrowser()
{
    Ogre::OverlayManager& om = Ogre::OverlayManager::getSingleton();
    return om.getByName("WebBrowser");
}

WebPanelOverlayElement* getAddressBar()
{
    return (WebPanelOverlayElement*)getWebBrowser()->getChild("AddressBar");
}

WebPanelOverlayElement* getWebView()
{
    return (WebPanelOverlayElement*)getWebBrowser()->getChild("WebView");
}

wkeClientHandler clientHandler;
void onTitleChanged(const wkeClientHandler* clientHandler, const wkeString title)
{
    HWND hWnd = 0;
    Ogre::Root::getSingleton().getAutoCreatedWindow()->getCustomAttribute("WINDOW", &hWnd);
    SetWindowTextW(hWnd, wkeToStringW(title));
}

void onURLChanged(const wkeClientHandler* clientHandler, const wkeString url)
{
    char script[1024];
    sprintf(script, "setURL('%s')", wkeToString(url));
    getAddressBar()->runJS(script);
}

//-------------------------------------------------------------------------------------
WebUIApp::WebUIApp(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    mWebBrowser(NULL),
    mAddressBar(NULL),
    mWebView(NULL),
    mWebPanelFactory(NULL),
    mFocusCtrl(NULL),
    m_nKey(OIS::KC_UNASSIGNED)
{
}

//-------------------------------------------------------------------------------------
WebUIApp::~WebUIApp(void)
{
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    OGRE_DELETE mRoot;
    OGRE_DELETE mWebPanelFactory;
}
//-------------------------------------------------------------------------------------
void WebUIApp::go(void)
{
    mResourcesCfg = "resources.cfg";

#ifdef _DEBUG
    mPluginsCfg = "plugins_d.cfg";
#else
    mPluginsCfg = "plugins.cfg";
#endif

    // construct Ogre::Root
    mRoot = OGRE_NEW Ogre::Root(mPluginsCfg);

    mWebPanelFactory = OGRE_NEW WebPanelOverlayElementFactory();
    Ogre::OverlayManager::getSingleton().addOverlayElementFactory(mWebPanelFactory);

    //-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);
 
    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName, true);
        }
    }

    //-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        ///WebKit does not support single-precision float
        Ogre::ConfigOptionMap& options = mRoot->getRenderSystem()->getConfigOptions();
        options["Floating-point mode"].currentValue = "Consistent";

        int w = atoi(options["Video Mode"].currentValue.c_str());
        if (w < 1024)
        {
            if (strchr(options["Video Mode"].currentValue.c_str(), '@'))
                options["Video Mode"].currentValue = "1024 x 768 @ 32-bit colour";
            else
                options["Video Mode"].currentValue = "1024 x 768";
        }
        
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "OgreWebKit");
        if (!mRoot)
            return;
    }
    else
    {
        return;
    }
    //-------------------------------------------------------------------------------------
    // choose scenemanager
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
    //-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    
    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);
    //-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
 
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
    //-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    //-------------------------------------------------------------------------------------
    // load resources
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    mSceneMgr->setSkyBox(true, "Examples/EveningSkyBox");

    // dim orange ambient and two bright orange lights to match the skybox
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.2f, 0));
    Ogre::Light* light = mSceneMgr->createLight();
    light->setPosition(2000, 1000, -1000);
    light->setDiffuseColour(1, 0.5f, 0);
    light = mSceneMgr->createLight();
    light->setPosition(-2000, 1000, 1000);
    light->setDiffuseColour(1, 0.5, 0);

    mPivot = mSceneMgr->getRootSceneNode()->createChildSceneNode();  // create a pivot node

    // create a child node and attach an ogre head and some smoke to it
    Ogre::SceneNode* headNode = mPivot->createChildSceneNode(Ogre::Vector3(100, 0, 0));
    headNode->attachObject(mSceneMgr->createEntity("Head", "ogrehead.mesh"));
    headNode->attachObject(mSceneMgr->createParticleSystem("Smoke", "Examples/Smoke"));

    mCamera->setPosition(0, 30, 350);

    //-------------------------------------------------------------------------------------
    //create FrameListener
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));

    mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mWebBrowser = getWebBrowser();
    mWebBrowser->show();

    mAddressBar = getAddressBar();
    mAddressBar->loadFile("html/addressbar.html");
    
    mWebView = getWebView();
    mWebView->setWebTransparent(true);
    mWebView->loadFile("html/mac-osx-lion.html");

    clientHandler.onTitleChanged = onTitleChanged;
    clientHandler.onURLChanged = onURLChanged;
    mWebView->setClientHandler(&clientHandler);

    //Set initial mouse clipping size
    windowResized(mWindow);

    mRoot->addFrameListener(this);
    mRoot->startRendering();
}
//-------------------------------------------------------------------------------------
bool WebUIApp::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    //Key auto repeat
    if (mFocusCtrl && m_nKey != OIS::KC_UNASSIGNED && (m_fRepeatDelay -= evt.timeSinceLastFrame) < 0)
    {
        m_fRepeatDelay = 0.035f;
        OIS::KeyEvent arg(NULL, m_nKey, m_nChar);
        mFocusCtrl->keyReleased(arg);
        mFocusCtrl->keyPressed(arg);
    }

    // spin the head around and make it float up and down
    mPivot->setPosition(0, Ogre::Math::Sin(mRoot->getTimer()->getMilliseconds() / 150.0f) * 10, 0);
    mPivot->yaw(Ogre::Radian(-evt.timeSinceLastFrame * 1.5f));

    mCameraMan->frameRenderingQueued(evt);

    return true;
}
//-------------------------------------------------------------------------------------
bool WebUIApp::keyPressed( const OIS::KeyEvent &arg )
{
    if (mFocusCtrl)
    {
        m_nKey = arg.key;
        m_nChar = arg.text;
        m_fRepeatDelay = 0.300f;
        mFocusCtrl->keyPressed(arg);
    }

    return true;
}

bool WebUIApp::keyReleased( const OIS::KeyEvent &arg )
{
    if (mFocusCtrl)
    {
        m_nKey = OIS::KC_UNASSIGNED;
        mFocusCtrl->keyReleased(arg);
    }

    return true;
}

bool WebUIApp::mouseMoved( const OIS::MouseEvent &arg )
{
    if (arg.state.X.abs <= 0 || arg.state.Y.abs <= 0)
        return true;

    if (arg.state.X.abs >= arg.state.width || arg.state.Y.abs >= arg.state.height)
        return true;

    bool handled = false;
    if (mAddressBar->isCursorOver(arg))
        handled = mAddressBar->mouseMoved(arg);
    else if (mWebView->isCursorOver(arg))
        handled = mWebView->mouseMoved(arg);

    if (!handled && arg.state.buttonDown(OIS::MB_Right))
    {
        mCameraMan->injectMouseMove(arg);
    }

    return true;
}

bool WebUIApp::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (arg.state.X.abs <= 0 || arg.state.Y.abs <= 0)
        return true;

    if (arg.state.X.abs >= arg.state.width || arg.state.Y.abs >= arg.state.height)
        return true;

    bool handled = false;
    if (mAddressBar->isCursorOver(arg))
    {
        if (mAddressBar->mousePressed(arg, id))
        {
             setFocusCtrl(mAddressBar);
             handled = true;
        }
    }
    else if (mWebView->isCursorOver(arg))
    {
        if (mWebView->mousePressed(arg, id))
        {
            setFocusCtrl(mWebView);
            handled = true;
        }
    }

    if (!handled)
    {
        mCameraMan->injectMouseDown(arg, id);
    }

    return true;
}

bool WebUIApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (arg.state.X.abs <= 0 || arg.state.Y.abs <= 0)
        return true;

    if (arg.state.X.abs >= arg.state.width || arg.state.Y.abs >= arg.state.height)
        return true;

    bool handled = false;
    if (mAddressBar->isCursorOver(arg))
        handled = mAddressBar->mouseReleased(arg, id);
    else if (mWebView->isCursorOver(arg))
        handled = mWebView->mouseReleased(arg, id);

    if (!handled)
    {
        mCameraMan->injectMouseUp(arg, id);
    }

    return true;
}

//Adjust mouse clipping area
void WebUIApp::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;

    const int h = 31;
    mAddressBar->setPosition(0, 0);
    mAddressBar->resize(width, h);

    mWebView->setPosition(0, (Ogre::Real)h);
    mWebView->resize(width, height - h);
}

//Unattach OIS before window shutdown (very important under Linux)
void WebUIApp::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

void WebUIApp::setFocusCtrl(WebPanelOverlayElement* ctrl)
{
    if (mFocusCtrl)
        mFocusCtrl->unfocus();

    mFocusCtrl = ctrl;
    if (mFocusCtrl)
        mFocusCtrl->focus();
}
