#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include "Orangutan.h"

#include <sstream>

#pragma warning ( disable : 4244 )


class App : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
  
 public:
  

  Ogre::SceneNode*        mNode;
  Orangutan::Librarian*   mLibrarian;
  Orangutan::Geometry*    mGeometry;
  
  App()
  {
   _makeOgre();
   _makeOIS();

   // Create the Librarian this is the "Root" class of Orangutan, responsible for
   // creating and destroying Orangutan Geometries.
   mLibrarian = new Orangutan::Librarian();

   // A node to attach our about to be created Geometry too.
   mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

   // Create a Geometry, but though the SceneManager as it's a MovableObject.
   // Use "OrangutanGeometry" as the type of MovableObject to create, as long as the Librarian
   // has been created, Ogre knows what you mean.
   mGeometry = static_cast<Orangutan::Geometry*>( mSceneMgr->createMovableObject("OrangutanGeometry") );
   mGeometry->setMaterialName(0, "uv_blend_test");
   mGeometry->setMaterialName(1, "BaseWhiteNoLighting");
   
   // Then attach it. Just treat Geometries like Entities, Lights or ManualObjects.
   mNode->attachObject(mGeometry);
   mGeometry->createPlane(Ogre::Vector3(0,0,0), Ogre::Vector2(10,10));
#if 1
   Orangutan::Displacement* dis = mGeometry->createDisplacement(Ogre::Vector3(0,0,0), Ogre::Vector3(0.25,0.25,0.25));
   dis->begin(10,10);
   for (size_t i=0;i < 10*10;i++)
   {
    dis->sample(i * 0.01f, Ogre::ColourValue(0.5f,0.5f,0.5f,0.5f));
   }
   dis->end();
   mNode->showBoundingBox(true);
#endif
   Orangutan::Block* block = mGeometry->createBlock(Ogre::Vector3(0,0,0), Ogre::Vector3(20,1,10));
   block->quad_index(Orangutan::Block::Quad_Top, 1);
   block->quad_show(Orangutan::Block::Quad_Left);
   mGeometry->saveAsOokFile("test.ook");
  }
  
 ~App()
  {
   std::cout << "\n** Average FPS is " << mWindow->getAverageFPS() << "\n\n";
   
   // Because Librarian is a the factory for OrangutanGeometries, deleting it before cleaning
   // up Ogre will not delete the OrangutanGeometries, as in Ogre's World the factory
   // doesn't exist anymore. So any SceneManagers or at least the Geometeries must be deleted
   // by hand whilst the Librarian still exists, to prevent memory leaks.
   mRoot->destroySceneManager(mSceneMgr);
   delete mLibrarian;
   delete mRoot;
  }
  
  bool frameStarted(const Ogre::FrameEvent& evt)
  {
   
   if (mWindow->isClosed())
    return false;
   
   mKeyboard->capture();
   if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
     return false;
   mMouse->capture();
   
   Ogre::Vector3 trans(0,0,0);
   
   if (mKeyboard->isKeyDown(OIS::KC_W))
    trans.z = -1;
   else if (mKeyboard->isKeyDown(OIS::KC_S))
    trans.z =  1;
   if (mKeyboard->isKeyDown(OIS::KC_A))
    trans.x = -1;
   else if (mKeyboard->isKeyDown(OIS::KC_D))
    trans.x =  1;
   
   if (trans.isZeroLength() == false)
   {
    Ogre::Vector3 pos = mCamera->getPosition();
    pos += mCamera->getOrientation() * (trans * 5.0f) * evt.timeSinceLastFrame;
    mCamera->setPosition(pos);
   }
   
   return true;
  }
  
  bool keyPressed( const OIS::KeyEvent &e )
  {
   return true;
  }
  
  bool keyReleased( const OIS::KeyEvent &e )
  {
   if (e.key == OIS::KC_F1)
   {
    std::cout << "[Orangutan] FPS: " << mWindow->getAverageFPS() << ", Batches: " << mWindow->getBatchCount() << "\n";
   }
   else if (e.key == OIS::KC_F2)
   {
    if (mCamera->getPolygonMode() == Ogre::PM_WIREFRAME)
     mCamera->setPolygonMode(Ogre::PM_SOLID);
    else
     mCamera->setPolygonMode(Ogre::PM_WIREFRAME);
   }
   return true;
  }
  
  bool mouseMoved( const OIS::MouseEvent &arg )
  {
   Ogre::Real pitch = Ogre::Real(arg.state.Y.rel) * -0.005f;
   Ogre::Real yaw = Ogre::Real(arg.state.X.rel) * -0.005f;
   mCamera->pitch(Ogre::Radian(pitch));
   mCamera->yaw(Ogre::Radian(yaw));
   return true;
  }
  
  bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
  {
   return true;
  }
  
  bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
  {
   return true;
  }
  
  void  _makeOgre()
  {
   srand(time(0));
   
   mRoot = new Ogre::Root("","");
   mRoot->addFrameListener(this);
   
#if 1
  #ifdef _DEBUG
   mRoot->loadPlugin("RenderSystem_Direct3D9_d");
  #else
   mRoot->loadPlugin("RenderSystem_Direct3D9");
  #endif
#else
  #ifdef _DEBUG
   mRoot->loadPlugin("RenderSystem_GL_d.dll");
  #else
   mRoot->loadPlugin("RenderSystem_GL.dll");
  #endif
#endif
   
   mRoot->setRenderSystem(mRoot->getAvailableRenderers()[0]);
   
   Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
  	rgm->addResourceLocation(".", "FileSystem");
   
   mRoot->initialise(false);
   
   mWindow = mRoot->createRenderWindow("Orangutan", 1024, 768, false);
   mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
   mCamera = mSceneMgr->createCamera("Camera");
   mViewport = mWindow->addViewport(mCamera);

   Ogre::ColourValue BackgroundColour = Ogre::ColourValue(0.1337f, 0.1337f, 0.1337f, 1.0f);
   Ogre::ColourValue GridColour = Ogre::ColourValue(0.2000f, 0.2000f, 0.2000f, 1.0f);
   Ogre::ColourValue GroundColour = Ogre::ColourValue(0.2337f, 0.2337f, 0.2337f, 1.0f);

   mViewport->setBackgroundColour(BackgroundColour);
   
   rgm->initialiseAllResourceGroups();
   
   mCamera->setPosition(10,8,10);
   mCamera->lookAt(0,1,0);
   mCamera->setNearClipDistance(0.05f);
   mCamera->setFarClipDistance(1000);
   
   Ogre::Light* light = mSceneMgr->createLight();
   light->setPosition(10,10,10);
   
   mReferenceObject = new Ogre::ManualObject("ReferenceGrid");

   mReferenceObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
   
   Ogre::Real step = 1.0f;
   unsigned int count = 200;
   unsigned int halfCount = count / 2;
   Ogre::Real full = (step * count);
   Ogre::Real half = full / 2;
   Ogre::Real y = 0;
   Ogre::ColourValue c;
   for (unsigned i=0;i < count+1;i++)
   {
    
    if (i == halfCount)
     c = Ogre::ColourValue(0.5f,0.3f,0.3f,1.0f);
    else
     c = GridColour;
    
    mReferenceObject->position(-half,y,-half+(step*i));
    mReferenceObject->colour(BackgroundColour);
    mReferenceObject->position(0,y,-half+(step*i));
    mReferenceObject->colour(c);
    mReferenceObject->position(0,y,-half+(step*i));
    mReferenceObject->colour(c);
    mReferenceObject->position(half,y,-half+(step*i));
    mReferenceObject->colour(BackgroundColour);

    if (i == halfCount)
     c = Ogre::ColourValue(0.3f,0.3f,0.5f,1.0f);
    else
     c = GridColour;
    
    mReferenceObject->position(-half+(step*i),y,-half);
    mReferenceObject->colour(BackgroundColour);
    mReferenceObject->position(-half+(step*i),y,0);
    mReferenceObject->colour(c);
    mReferenceObject->position(-half+(step*i),y,0);
    mReferenceObject->colour(c);
    mReferenceObject->position(-half+(step*i),y, half);
    mReferenceObject->colour(BackgroundColour);

   }
   
   mReferenceObject->position(-1,0,10);
   mReferenceObject->colour(Ogre::ColourValue(0.3f,0.3f,0.5f,1.0f));
   mReferenceObject->position(0,0,11);
   mReferenceObject->colour(Ogre::ColourValue(0.3f,0.3f,0.5f,1.0f));
   mReferenceObject->position(0,0,11);
   mReferenceObject->colour(Ogre::ColourValue(0.3f,0.3f,0.5f,1.0f));
   mReferenceObject->position( 1,0,10);
   mReferenceObject->colour(Ogre::ColourValue(0.3f,0.3f,0.5f,1.0f));
   
   
   mReferenceObject->position(10,0,-1);
   mReferenceObject->colour(Ogre::ColourValue(0.6f,0.3f,0.3f,1.0f));
   mReferenceObject->position(11,0,0);
   mReferenceObject->colour(Ogre::ColourValue(0.6f,0.3f,0.3f,1.0f));
   mReferenceObject->position(11,0,0);
   mReferenceObject->colour(Ogre::ColourValue(0.6f,0.3f,0.3f,1.0f));
   mReferenceObject->position(10,0,1);
   mReferenceObject->colour(Ogre::ColourValue(0.6f,0.3f,0.3f,1.0f));
   
   mReferenceObject->end();
   mSceneMgr->getRootSceneNode()->attachObject(mReferenceObject);
    
    
   mTimer = 0.0f;
  }
  
  void  _makeOIS()
  {
   // Initialise OIS
   OIS::ParamList pl;
   size_t windowHnd = 0;
   std::ostringstream windowHndStr;
   mWindow->getCustomAttribute("WINDOW", &windowHnd);
   windowHndStr << windowHnd;
   pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
   mInputManager = OIS::InputManager::createInputSystem( pl );
   mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
   mKeyboard->setEventCallback(this);
   mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
   mMouse->setEventCallback(this);
   mMouse->getMouseState().width = mViewport->getActualWidth();
   mMouse->getMouseState().height = mViewport->getActualHeight();
  }
  
  Ogre::Root*             mRoot;
  Ogre::RenderWindow*     mWindow;
  Ogre::Viewport*         mViewport;
  Ogre::SceneManager*     mSceneMgr;
  Ogre::Camera*           mCamera;
  Ogre::Real              mNextUpdate;
  OIS::InputManager*      mInputManager;
  OIS::Keyboard*          mKeyboard;
  OIS::Mouse*             mMouse;
  Ogre::ManualObject*     mReferenceObject;
  Ogre::Real              mTimer;
};

void main()
{
 App* app  = new App();
 app->mRoot->startRendering();
 delete app;
}


