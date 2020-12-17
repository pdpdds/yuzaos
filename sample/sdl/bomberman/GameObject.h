//
//  GameObject.h
//  SDL Game Programming Book
//
//  Created by shaun mitchell on 06/01/2013.
//  Copyright (c) 2013 shaun mitchell. All rights reserved.
//

#ifndef SDL_Game_Programming_Book_GameObject_h
#define SDL_Game_Programming_Book_GameObject_h

#include "LoaderParams.h"
#include "Vector2D.h"
#include <string>
#include <memory>
#include <vector>
#include "IDGenerator.h"
#include "RTTI.h"

class TileLayer;
struct Telegram;
class Level;

class GameObject : public Common::RTTI
{
public:
    
    // base class needs virtual destructor
    virtual ~GameObject() {}
    
    // load from file - int x, int y, int width, int height, std::string textureID, int numFrames, int callbackID = 0, int animSpeed = 0
	virtual void Load(LoaderParams& params) = 0;
    
    // draw the object
    virtual void Draw()=0;
    
    // do update stuff
	virtual void Update() = 0;
    
    // remove anything that needs to be deleted
    virtual void Clean()=0;
    
    // object has collided, handle accordingly
    virtual void Collision() = 0;
    
    // get the type of the object
    virtual std::string type() = 0;

	virtual bool CheckCollision(GameObject* pGameObject) { return false; }
	virtual void ProcessDeadAction(){ setdead(); }
    
    // getters for common variables
    Vector2D& getPosition() { return m_position; }
    Vector2D& getVelocity() { return m_velocity; }

	Vector2D Center()
	{		
		m_center.m_x = m_position.m_x + (m_width / 2);
		m_center.m_y = m_position.m_y + (m_height / 2);

		return m_center;
	}

	Vector2D  Side()const{ return m_vSide; }

	double    MaxForce()const{ return m_dMaxForce; }
	void      SetMaxForce(double mf){ m_dMaxForce = mf; }

	double    MaxSpeed()const{ return m_dMaxSpeed; }
	void      SetMaxSpeed(double new_speed){ m_dMaxSpeed = new_speed; }

	bool         IsTagged()const{ return m_bTag; }
	void         Tag(){ m_bTag = true; }
	void         UnTag(){ m_bTag = false; }

	Vector2D  Heading()const{ return m_vHeading; }
	void      SetHeading(Vector2D new_heading);
	bool      RotateHeadingToFacePosition(Vector2D target);
    
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    
    // scroll along with tile map
    void scroll(float scrollSpeed)
    {
//        if(type() != std::string("Player")) // player is never scrolled
//        {
//            m_position.setX(m_position.getX() - scrollSpeed);
//        }
    }
    
    // is the object currently being updated?
    bool updating() { return m_bUpdating; }
    
    // is the object dead?
    bool dead() { return m_bDead; }
	void setdead() { m_bDead = true; }
    
    // is the object doing a death animation?
    bool dying() { return m_bDying; }
    
    // set whether to update the object or not
    void setUpdating(bool updating) { m_bUpdating = updating; }
    
    void setCollisionLayers(std::vector<TileLayer*>* layers) { m_pCollisionLayers = layers; }

	int ID(){ return m_Id; }

	bool IsCollidable(){ return m_bCollidable; }
	void SetCollidable(bool bCollidable){ m_bCollidable = bCollidable; }

	void SetOwner(Level* pLevel){ m_pLevel = pLevel; }
	Level* GetOwner(){ return m_pLevel; }

	virtual bool HandleMessage(const Telegram& msg){ return false; }
        
protected:
    
    // constructor with default initialisation list
    GameObject() :  m_position(0,0),
                    m_velocity(0,0),
                    m_acceleration(0,0),
                    m_width(0),
                    m_height(0),
                    m_currentRow(0),
                    m_currentFrame(0),
                    m_bUpdating(false),
                    m_bDead(false),
                    m_bDying(false),
                    m_angle(0),
                    m_alpha(255),
					m_bTag(false),
					m_dMass(1.0f),
					m_dMaxForce(3.0f),
					m_bCollidable(true)
    {
		m_Id = IDGenerator::Instance()->GetNextId();
    }

    // movement variables
    Vector2D m_position;
	Vector2D m_center;
    Vector2D m_velocity;
    Vector2D m_acceleration;
    
    // size variables
    int m_width;
    int m_height;
    
    // animation variables
    int m_currentRow;
    int m_currentFrame;
    int m_numFrames;
    std::string m_textureID;
    
    // common boolean variables
    bool m_bUpdating;
    bool m_bDead;
    bool m_bDying;

	//충돌하는 오브젝트인가
	bool m_bCollidable;
    
    // rotation
    double m_angle;
    
    // blending
    int m_alpha;
    
    std::vector<TileLayer*>* m_pCollisionLayers;

	int m_Id;

	//this is a generic flag. 
	bool        m_bTag;

	//a normalized vector pointing in the direction the entity is heading. 
	Vector2D    m_vHeading;

	Vector2D    m_vSide;

	double      m_dMass;

	//the maximum speed this entity may travel at.
	double      m_dMaxSpeed;

	double      m_dMaxForce;

	//게임 레벨
	Level* m_pLevel;
};

#endif
