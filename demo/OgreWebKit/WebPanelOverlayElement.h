#ifndef __WEB_OVERLAY_ELEMENT_H__
#define __WEB_OVERLAY_ELEMENT_H__

#include <OgrePanelOverlayElement.h>
#include <OgreOverlayElementFactory.h>
#include <OgreTexture.h>
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include "wke.h"

class WebPanelOverlayElement : public Ogre::PanelOverlayElement
{
public:
    WebPanelOverlayElement(const Ogre::String& name);
    virtual ~WebPanelOverlayElement();

    virtual void initialise();

    virtual const Ogre::String& getTypeName() const;

    virtual void _updateRenderQueue(Ogre::RenderQueue* queue);

    void resize(int w, int h);

    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    void loadURL(const char* url);
    void loadFile(const char* filename);
    void loadHTML(const char* string);

    void goBack();
    void goForward();

    void runJS(const char* script);

    void setClientHandler(wkeClientHandler* handler);

    //webview transparent
    void setWebTransparent(bool transparent);
    void focus();
    void unfocus();
    
    bool isCursorOver(const OIS::MouseEvent &arg);

protected:
    virtual void addBaseParameters();

    wke::IWebView* mWebView;
    Ogre::TexturePtr mTexture;
};

/** Factory for creating PanelOverlayElement instances. */
class WebPanelOverlayElementFactory: public Ogre::OverlayElementFactory
{
public:
    /** See OverlayElementFactory */
    Ogre::OverlayElement* createOverlayElement(const Ogre::String& instanceName)
    {
        return OGRE_NEW WebPanelOverlayElement(instanceName);
    }
    /** See OverlayElementFactory */
    virtual const Ogre::String& getTypeName(void) const;
};

#endif