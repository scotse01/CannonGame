/*
-----------------------------------------------------------------------------
Filename:    CannonGame.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
Tutorial Framework (for Ogre 1.9)
http://www.ogre3d.org/wiki/
-----------------------------------------------------------------------------
 */

#include "CannonGame.h"
#include "DotScene.h"
#include <Shapes/OgreBulletCollisionsStaticPlaneShape.h>
#include <Shapes/OgreBulletCollisionsBoxShape.h>
#include <Shapes/OgreBulletCollisionsSphereShape.h>
//uses tiny xml 1 whcih can be installed on the system
// or files called into project

const Ogre::uint32 CannonGame::INTERSECTABLE = 2;
const Ogre::uint32 CannonGame::BALL = 4;
const Ogre::uint32 CannonGame::TARGET = 8;
const Ogre::uint32 CannonGame::GROUND = 16;
const Ogre::uint32 CannonGame::HIT = 32;

//---------------------------------------------------------------------------
CannonGame::CannonGame(void):
																																				mIntersectionQuery(0),
																																				m_numEntities(0),
																																				m_moveSpeed(500),
																																				m_gravity(0, -9.81, 0),
																																				m_bounds(),
																																				elevation(0),
																																				rotation(0),
																																				score(0),
																																				shots(10),
																																				highScore(0),
																																				fired(false),
																																				created(false),
																																				hitCount(0)
{
}
//---------------------------------------------------------------------------
CannonGame::~CannonGame(void)
{
}

bool CannonGame::frameStarted(const Ogre::FrameEvent & evt)
{
	bool result = BaseApplication::frameStarted(evt);

	m_world->stepSimulation(evt.timeSinceLastFrame);

	return result;
}

bool CannonGame::frameRenderingQueued(const Ogre::FrameEvent& evt){
	bool result = BaseApplication::frameRenderingQueued(evt);

	if(score>highScore)
	{
		highScore = score;
	}

	Ogre::String rotStr = Ogre::StringConverter::toString(rotation);
	Ogre::String elevStr = Ogre::StringConverter::toString(elevation * -1);
	Ogre::String shotsStr = Ogre::StringConverter::toString(shots);
	Ogre::String hitsStr = Ogre::StringConverter::toString(hitCount);
	Ogre::String scoreStr = Ogre::StringConverter::toString(score);
	Ogre::String highScoreStr = Ogre::StringConverter::toString(highScore);

	guiRoot->getChild("Rotation")->setText("Rotation: " + rotStr);
	guiRoot->getChild("Elevation")->setText("Elevation: " + elevStr);
	guiRoot->getChild("Shots")->setText("Shots Remaining: " + shotsStr);
	guiRoot->getChild("Hits")->setText("Hits: " + hitsStr);
	guiRoot->getChild("Score")->setText("Score: " + scoreStr);
	guiRoot->getChild("HighScore")->setText("High Score: " + highScoreStr);


	for(iter = m_bodies.begin(); iter < m_bodies.end();iter++)
	{
		if((*iter)->getQueryFlags() == INTERSECTABLE+BALL && (*iter)->getLinearVelocity()==Ogre::Vector3(0,0,0))
		{

			(*iter)->getSceneNode()->detachAllObjects();
			m_bodies.erase(iter);
			fired = false;
			for(iter1 = targetList.begin(); iter1 < targetList.end();iter1++)
			{
				if((*iter1)->getQueryFlags() == INTERSECTABLE+TARGET+HIT)
				{
					(*iter1)->setQueryFlags(INTERSECTABLE|TARGET);
				}
			}
			score = score + hitCount * hitCount;
			hitCount = 0;
		}
	}
	mIntersectionQuery->execute(this);
	return result;
}

void CannonGame::createFrameListener(void) {
	BaseApplication::createFrameListener();

	mIntersectionQuery = mSceneMgr->createIntersectionQuery(INTERSECTABLE);
	mRoot->addFrameListener(this);
}
//---------------------------------------------------------------------------
void CannonGame::createScene(void)
{


	//perform the magic required to make textures work
	//Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(0);
	//I would like terrain and shadows please
	//mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	parseDotScene("cannon.scene", "General", mSceneMgr); //file name, Ogre's group name, scene manager for added nodes

	//perform the magic required to make textures work
	//Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(0);

	//create the scene stuff

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l1 = mSceneMgr->createLight("MainLight");
	l1->setPosition(20,80,50);




	mSceneMgr->setSkyBox(true, "Examples/TrippySkyBox", 300, true);


	//move camera
	mCamera->move(Ogre::Vector3::UNIT_Y * 10);


	//build a floor
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);

	Ogre::MeshManager::getSingleton().createPlane("ground",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			plane, 200000, 200000, 20, 20, true, 1, 9000, 9000,
			Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);

	entGround->setMaterialName("Examples/Rockwall");
	entGround->setCastShadows(false);

	//Bullet Physics
	m_world = new OgreBulletDynamics::DynamicsWorld(mSceneMgr, m_bounds, m_gravity*10);
	m_debugDrawer = new OgreBulletCollisions::DebugDrawer();
	m_debugDrawer->setDrawWireframe(true);
	m_world->setDebugDrawer(m_debugDrawer);

	//set up the ground
	OgreBulletCollisions::CollisionShape *shape = new OgreBulletCollisions::StaticPlaneCollisionShape(Ogre::Vector3::UNIT_Y,0);

	OgreBulletDynamics::RigidBody *groundBody = new OgreBulletDynamics::RigidBody("GroundBody", m_world);

	groundBody->setStaticShape(shape,
			1, //restitution(bounciness)
			1000000 //friction
	);

	entGround->setQueryFlags(INTERSECTABLE|GROUND);

	//store objects for later
	m_bodies.push_back(groundBody);
	m_shapes.push_back(shape);

	Ogre::SceneNode *parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("CannonNode", Ogre::Vector3(0, 05, 0));

	mSceneMgr->getRootSceneNode()->removeChild(mSceneMgr->getSceneNode("Cylinder.001"));
	mSceneMgr->getRootSceneNode()->removeChild(mSceneMgr->getSceneNode("Cylinder"));
	mSceneMgr->getRootSceneNode()->removeChild(mSceneMgr->getSceneNode("Torus.000"));

	Ogre::SceneNode *gunNode = parentNode->createChildSceneNode("GunNode");
	gunNode->addChild(mSceneMgr->getSceneNode("Cylinder"));
	gunNode->addChild(mSceneMgr->getSceneNode("Cylinder.001"));

	parentNode->addChild(mSceneMgr->getSceneNode("Torus.000"));

	parentNode->yaw(Ogre::Degree(180));
	parentNode->setPosition(Ogre::Vector3(0,8,0));

	mCamera->detachFromParent();
	gunNode->attachObject(mCamera);

	mCamera->setPosition(Ogre::Vector3(-1,8,-17));

	Ogre::Light* l2 = mSceneMgr->createLight("CannonLight");
	l2->setPosition(Ogre::Vector3(-1,20,-17));

	createPenguin(Ogre::Vector3(500,0,-500));
	createPenguin(Ogre::Vector3(400,0,-500));
	createPenguin(Ogre::Vector3(300,0,-500));
	createPenguin(Ogre::Vector3(200,0,-500));
	createPenguin(Ogre::Vector3(100,0,-500));
	createPenguin(Ogre::Vector3(0,0,-500));
	createPenguin(Ogre::Vector3(-500,0,-500));
	createPenguin(Ogre::Vector3(-400,0,-500));
	createPenguin(Ogre::Vector3(-300,0,-500));
	createPenguin(Ogre::Vector3(-200,0,-500));
	createPenguin(Ogre::Vector3(-100,0,-500));

	createPenguin(Ogre::Vector3(400,0,-600));
	createPenguin(Ogre::Vector3(300,0,-600));
	createPenguin(Ogre::Vector3(200,0,-600));
	createPenguin(Ogre::Vector3(100,0,-600));
	createPenguin(Ogre::Vector3(0,0,-600));
	createPenguin(Ogre::Vector3(-100,0,-600));
	createPenguin(Ogre::Vector3(-200,0,-600));
	createPenguin(Ogre::Vector3(-300,0,-600));
	createPenguin(Ogre::Vector3(-400,0,-600));

	createPenguin(Ogre::Vector3(400,0,-600));
	createPenguin(Ogre::Vector3(300,0,-600));
	createPenguin(Ogre::Vector3(200,0,-600));
	createPenguin(Ogre::Vector3(100,0,-600));
	createPenguin(Ogre::Vector3(0,0,-600));
	createPenguin(Ogre::Vector3(-100,0,-600));
	createPenguin(Ogre::Vector3(-200,0,-600));
	createPenguin(Ogre::Vector3(-300,0,-600));
	createPenguin(Ogre::Vector3(-400,0,-600));


	createPenguin(Ogre::Vector3(300,0,-700));
	createPenguin(Ogre::Vector3(200,0,-700));
	createPenguin(Ogre::Vector3(100,0,-700));
	createPenguin(Ogre::Vector3(0,0,-700));
	createPenguin(Ogre::Vector3(-100,0,-700));
	createPenguin(Ogre::Vector3(-200,0,-700));
	createPenguin(Ogre::Vector3(-300,0,-700));

	createPenguin(Ogre::Vector3(300,0,-700));
	createPenguin(Ogre::Vector3(200,0,-700));
	createPenguin(Ogre::Vector3(100,0,-700));
	createPenguin(Ogre::Vector3(0,0,-700));
	createPenguin(Ogre::Vector3(-100,0,-700));
	createPenguin(Ogre::Vector3(-200,0,-700));
	createPenguin(Ogre::Vector3(-300,0,-700));

	createPenguin(Ogre::Vector3(300,0,-700));
	createPenguin(Ogre::Vector3(200,0,-700));
	createPenguin(Ogre::Vector3(100,0,-700));
	createPenguin(Ogre::Vector3(0,0,-700));
	createPenguin(Ogre::Vector3(-100,0,-700));
	createPenguin(Ogre::Vector3(-200,0,-700));
	createPenguin(Ogre::Vector3(-300,0,-700));

	createPenguin(Ogre::Vector3(200,0,-800));
	createPenguin(Ogre::Vector3(100,0,-800));
	createPenguin(Ogre::Vector3(0,0,-800));
	createPenguin(Ogre::Vector3(-100,0,-800));
	createPenguin(Ogre::Vector3(-200,0,-800));

	createPenguin(Ogre::Vector3(200,0,-800));
	createPenguin(Ogre::Vector3(100,0,-800));
	createPenguin(Ogre::Vector3(0,0,-800));
	createPenguin(Ogre::Vector3(-100,0,-800));
	createPenguin(Ogre::Vector3(-200,0,-800));

	createPenguin(Ogre::Vector3(200,0,-800));
	createPenguin(Ogre::Vector3(100,0,-800));
	createPenguin(Ogre::Vector3(0,0,-800));
	createPenguin(Ogre::Vector3(-100,0,-800));
	createPenguin(Ogre::Vector3(-200,0,-800));

	createPenguin(Ogre::Vector3(200,0,-800));
	createPenguin(Ogre::Vector3(100,0,-800));
	createPenguin(Ogre::Vector3(0,0,-800));
	createPenguin(Ogre::Vector3(-100,0,-800));
	createPenguin(Ogre::Vector3(-200,0,-800));

	createPenguin(Ogre::Vector3(100,0,-900));
	createPenguin(Ogre::Vector3(0,0,-900));
	createPenguin(Ogre::Vector3(-100,0,-900));

	createPenguin(Ogre::Vector3(100,0,-900));
	createPenguin(Ogre::Vector3(0,0,-900));
	createPenguin(Ogre::Vector3(-100,0,-900));

	createPenguin(Ogre::Vector3(100,0,-900));
	createPenguin(Ogre::Vector3(0,0,-900));
	createPenguin(Ogre::Vector3(-100,0,-900));

	createPenguin(Ogre::Vector3(100,0,-900));
	createPenguin(Ogre::Vector3(0,0,-900));
	createPenguin(Ogre::Vector3(-100,0,-900));

	createPenguin(Ogre::Vector3(100,0,-900));
	createPenguin(Ogre::Vector3(0,0,-900));
	createPenguin(Ogre::Vector3(-100,0,-900));

	createPenguin(Ogre::Vector3(0,0,-1000));

	createPenguin(Ogre::Vector3(0,0,-1000));

	createPenguin(Ogre::Vector3(0,0,-1000));

	createPenguin(Ogre::Vector3(0,0,-1000));

	createPenguin(Ogre::Vector3(0,0,-1000));

	createPenguin(Ogre::Vector3(0,0,-1000));



	mCamera->getViewport()->setOverlaysEnabled(false);
	if(!created)
	{
		mCamera->yaw(Ogre::Degree(180));
		setupGUI();
		created = true;
	}
}

void CannonGame::fireCannon(){
	shots = shots - 1;
	//position of new block
	Ogre::Vector3 position = mCamera->getDerivedPosition() +
			mCamera->getDerivedDirection().normalisedCopy() * 10+Ogre::Vector3(0,-5,0);

	//create an entity
	Ogre::Entity *sphereEntity = mSceneMgr->createEntity("Sphere" +
			Ogre::StringConverter::toString(m_numEntities),
			"penguin.mesh");

	sphereEntity->setCastShadows(true);

	Ogre::AxisAlignedBox bound = sphereEntity->getBoundingBox();
	Ogre::Real size = bound.getSize().y;
	size /= 2.0f;
	//size *= 0.95f;

	//sphereEntity->setMaterialName("Examples/SphereMappedRustySteel");

	//create a node
	Ogre::SceneNode *sphereNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	sphereNode->attachObject(sphereEntity);

	//scale both the node and the size
	sphereNode->scale(0.1f, 0.1f, 0.1f);
	size *= 0.1f;

	//create bullet stuff
	OgreBulletCollisions::SphereCollisionShape *sphereShape =
			new OgreBulletCollisions::SphereCollisionShape(size);

	OgreBulletDynamics::RigidBody *sphereBody =
			new OgreBulletDynamics::RigidBody("sphereRigid" +
					Ogre::StringConverter::toString(m_numEntities),
					m_world);

	sphereBody->setShape(sphereNode,
			sphereShape,
			0.3f, //dynamic restitution
			100.6f, //dynamic friction
			1.0f, //dynamic body mass
			position, //starting position
			Ogre::Quaternion(1, 0, 0, 0)  //orientation
	);
	sphereBody->setDamping(0.2, 0.2);
	//sphereBody->disableDeactivation();
	m_numEntities++;

	sphereBody->setLinearVelocity(mCamera->getDerivedDirection().normalisedCopy()*m_moveSpeed*Ogre::Vector3(1,3,1));

	sphereEntity->setQueryFlags(INTERSECTABLE|BALL);
	sphereBody->setQueryFlags(INTERSECTABLE|BALL);

	m_shapes.push_back(sphereShape);
	m_bodies.push_back(sphereBody);

}

void CannonGame::createPenguin(Ogre::Vector3 pos)
{

	//create an entity
	Ogre::Entity *cubeEntity = mSceneMgr->createEntity("Cube" +
			Ogre::StringConverter::toString(m_numEntities),
			"penguin.mesh");

	cubeEntity->setCastShadows(true);

	Ogre::AxisAlignedBox bound = cubeEntity->getBoundingBox();
	Ogre::Vector3 size = bound.getSize();
	size /= 2.0f;
	//size *= 0.95f;

	//create a node
	Ogre::SceneNode *cubeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	cubeNode->attachObject(cubeEntity);
	//scale both the node and the size
	cubeNode->scale(0.25f, 0.25f, 0.25f);
	size *= 0.25f;

	cubeNode->pitch(Ogre::Degree(180));

	//create bullet stuff
	OgreBulletCollisions::BoxCollisionShape *cubeShape =
			new OgreBulletCollisions::BoxCollisionShape(size);

	OgreBulletDynamics::RigidBody *cubeBody =
			new OgreBulletDynamics::RigidBody("CubeRigid" +
					Ogre::StringConverter::toString(m_numEntities),
					m_world);

	cubeBody->setShape(cubeNode,
			cubeShape,
			0.6f, //dynamic restitution
			0.6f, //dynamic friction
			0.1f, //dynamic body mass
			pos, //starting position
			cubeNode->getOrientation()//Ogre::Quaternion(0, 0, 0, 1)  //orientation
	);

	cubeEntity->setQueryFlags(INTERSECTABLE|TARGET);
	cubeBody->setQueryFlags(INTERSECTABLE|TARGET);

	m_numEntities++;

	m_shapes.push_back(cubeShape);
	m_bodies.push_back(cubeBody);
	targetList.push_back(cubeEntity);

}
//-------------------------------------------------------------------------------------
// OIS::KeyListener
bool CannonGame::keyPressed( const OIS::KeyEvent& evt ){

	switch (evt.key)
	{
	case OIS::KC_W:
		if (elevation>Ogre::Degree(-10))
		{
			mSceneMgr->getSceneNode("GunNode")->pitch(Ogre::Degree(-1));
			elevation = elevation - Ogre::Degree(1);
		}
		break;
	case OIS::KC_S:
		if(elevation<Ogre::Degree(0))
		{
			mSceneMgr->getSceneNode("GunNode")->pitch(Ogre::Degree(1));
			elevation = elevation + Ogre::Degree(1);
		}
		break;
	case OIS::KC_A:
		mSceneMgr->getSceneNode("CannonNode")->yaw(Ogre::Degree(1));
		rotation = rotation - Ogre::Degree(1);
		break;
	case OIS::KC_D:
		mSceneMgr->getSceneNode("CannonNode")->yaw(Ogre::Degree(-1));
		rotation = rotation + Ogre::Degree(1);
		break;
	case OIS::KC_SPACE:
	case OIS::KC_R:
	case OIS::KC_UP:
	case OIS::KC_DOWN:
	case OIS::KC_LEFT:
	case OIS::KC_RIGHT:
		break;
	default:
		BaseApplication::keyPressed(evt);
		break;
	}
	return true;
}


bool CannonGame::keyReleased( const OIS::KeyEvent& evt ){
	switch (evt.key)
	{
	case OIS::KC_SPACE:
		if(shots > 0 && !fired)
		{
			fired = true;
			fireCannon();
		}
		break;
	case OIS::KC_R:
		restart();
		break;
	default:
		BaseApplication::keyReleased(evt);
		break;
	}
	return true;
}

bool CannonGame::queryResult(Ogre::MovableObject* first,Ogre::MovableObject* second ){
	if(first->getQueryFlags() == INTERSECTABLE+GROUND && second->getQueryFlags() == INTERSECTABLE+BALL)
	{
		for(iter = m_bodies.begin(); iter < m_bodies.end();iter++)
		{
			if(second->getParentSceneNode() == (*iter)->getSceneNode())
			{
				(*iter)->setDamping(0.8,0.8);
			}
		}
	}
	if(second->getQueryFlags() == INTERSECTABLE+GROUND && first->getQueryFlags() == INTERSECTABLE+BALL)
	{
		for(iter = m_bodies.begin(); iter < m_bodies.end();iter++)
		{
			if(first->getParentSceneNode() == (*iter)->getSceneNode())
			{
				(*iter)->setDamping(0.8,0.8);
			}
		}
	}
	if(first->getQueryFlags() == INTERSECTABLE+TARGET && second->getQueryFlags() == INTERSECTABLE+BALL)
	{
		hitCount = hitCount +1;
		first->setQueryFlags(INTERSECTABLE|TARGET|HIT);
	}
	if(second->getQueryFlags() == INTERSECTABLE+TARGET && first->getQueryFlags() == INTERSECTABLE+BALL)
	{
		hitCount = hitCount +1;
		first->setQueryFlags(INTERSECTABLE|TARGET|HIT);
	}
	return true;
}

void CannonGame::setupGUI(){

	mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

	CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
	CEGUI::Font::setDefaultResourceGroup("Fonts");
	CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	CEGUI::WindowManager::setDefaultResourceGroup("Layouts");
	//loading scheme
	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");



	CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
	//load our file

	guiRoot = wmgr.loadLayoutFromFile("CannonGame.layout");
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
}

void CannonGame::restart()
{
	targetList.clear();
	score = 0;
	shots = 10;
	hitCount = 0;
	fired = false;
	m_bodies.clear();
	m_shapes.clear();
	mSceneMgr->clearScene();
	createScene();
}
//---------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char *argv[])
#endif
{
	// Create application object
	CannonGame app;

	try {
		app.go();
	} catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		std::cerr << "An exception has occurred: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
