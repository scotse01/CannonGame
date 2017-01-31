/*
-----------------------------------------------------------------------------
Filename:    CannonGame.h
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

#ifndef __CannonGame_h_
#define __CannonGame_h_

#include "BaseApplication.h"

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include <OgreBulletDynamicsRigidBody.h>
#include <deque>

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <vector>

//---------------------------------------------------------------------------

class CannonGame : public BaseApplication, public Ogre::IntersectionSceneQueryListener
{
public:
	static const Ogre::uint32 INTERSECTABLE;
	static const Ogre::uint32 BALL;
	static const Ogre::uint32 TARGET;
	static const Ogre::uint32 GROUND;
	static const Ogre::uint32 HIT;

	CannonGame();
	virtual ~CannonGame();
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	bool frameStarted(const Ogre::FrameEvent& evt);

	// OIS::KeyListener
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );

protected:
	CEGUI::OgreRenderer* mRenderer;
	virtual void createScene(void);
	virtual void createFrameListener(void);
	virtual void fireCannon(void);
	virtual void createPenguin(Ogre::Vector3 pos);
	virtual bool queryResult(Ogre::MovableObject* first,Ogre::MovableObject* second );
	virtual void setupGUI(void);
	virtual void restart(void);

	bool queryResult(Ogre::MovableObject* first, Ogre::SceneQuery::WorldFragment* fragment ){
		return true;
	}

	OgreBulletDynamics::DynamicsWorld *m_world;
	OgreBulletCollisions::DebugDrawer *m_debugDrawer;

	int score;
	int shots;
	int highScore;
	int hitCount;
	int m_numEntities;
	int m_moveSpeed;
	Ogre::Vector3 m_gravity;
	Ogre::AxisAlignedBox m_bounds;

	bool fired;
	bool created;

	Ogre::Degree elevation;
	Ogre::Degree rotation;
	Ogre::AnimationState *mAnimationState;

	std::deque<OgreBulletDynamics::RigidBody *> m_bodies;
	std::deque<OgreBulletDynamics::RigidBody *>::iterator iter;
	std::deque<OgreBulletCollisions::CollisionShape *> m_shapes;

	Ogre::vector<Ogre::Entity*>::type targetList;
	Ogre::vector<Ogre::Entity*>::iterator iter1;

	CEGUI::Window* guiRoot;

	Ogre::IntersectionSceneQuery *mIntersectionQuery;

};

//---------------------------------------------------------------------------

#endif // #ifndef __CannonGame_h_

//---------------------------------------------------------------------------
