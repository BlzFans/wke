#ifndef WEB_UI_APP_H
#define WEB_UI_APP_H

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreOverlayManager.h>
#include <OgreParticleSystem.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkCameraMan.h>

class WebPanelOverlayElement;
class WebPanelOverlayElementFactory;
class WebUIApp : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
    WebUIApp(void);
    virtual ~WebUIApp(void);

    void go(void);

protected:
    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    //Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    //Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);

    void setFocusCtrl(WebPanelOverlayElement* ctrl);

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    OgreBites::SdkCameraMan* mCameraMan;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

    Ogre::Overlay* mWebBrowser;
    WebPanelOverlayElement* mAddressBar;
    WebPanelOverlayElement* mWebView;
    WebPanelOverlayElementFactory* mWebPanelFactory;

    WebPanelOverlayElement* mFocusCtrl;

    Ogre::SceneNode* mPivot;

    //Key auto repeat
    OIS::KeyCode m_nKey;
    unsigned int m_nChar;
    float m_fRepeatDelay;
};

Ogre::Overlay* getWebBrowser();
WebPanelOverlayElement* getAddressBar();
WebPanelOverlayElement* getWebView();

#endif // #ifndef WEB_UI_APP_H
