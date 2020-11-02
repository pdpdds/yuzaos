/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BEING_H
#define BEING_H

#include <guichan/color.hpp>

#include <SDL_types.h>

#include <string>
#include <vector>
#include <bitset>

#include "sprite.h"
#include "spritedef.h"

#include "../position.h"

#include "../../image/particle/particlecontainer.h"

#define FIRST_IGNORE_EMOTE 14

#define SPEECH_TIME 500
#define SPEECH_MAX_TIME 1000

class AnimatedSprite;
class BeingConfigListener;
class Image;
class ItemInfo;
class Item;
class Map;
class Graphics;
class Particle;
class Position;
class SimpleAnimation;
class SpeechBubble;
class Text;

typedef std::list<Sprite*> Sprites;
typedef Sprites::iterator SpriteIterator;

enum Gender
{
    GENDER_MALE = 0,
    GENDER_FEMALE = 1,
    GENDER_UNSPECIFIED = 2
};

class Being : public Sprite
{
    public:
        enum Type
        {
            UNKNOWN,
            WARP,
            PLAYER,
            NPC,
            MONSTER,
            INVALID
        };

        /**
         * Action the being is currently performing.
         */
        enum Action
        {
            STAND,
            WALK,
            ATTACK,
            SIT,
            DEAD,
            HURT
        };

        enum Sprite
        {
            BASE_SPRITE = 0,
            SHOE_SPRITE,
            BOTTOMCLOTHES_SPRITE,
            TOPCLOTHES_SPRITE,
            MISC1_SPRITE,
            MISC2_SPRITE,
            HAIR_SPRITE,
            HAT_SPRITE,
            CAPE_SPRITE,
            GLOVES_SPRITE,
            WEAPON_SPRITE,
            SHIELD_SPRITE,
            VECTOREND_SPRITE
        };

        enum TargetCursorSize
        {
            TC_SMALL = 0,
            TC_MEDIUM,
            TC_LARGE,
            NUM_TC
        };

        enum Speech
        {
            NO_SPEECH = 0,
            TEXT_OVERHEAD,
            NO_NAME_IN_BUBBLE,
            NAME_IN_BUBBLE
        };

        enum AttackType
        {
            HIT = 0x00,
            CRITICAL = 0x0a,
            MULTI = 0x08,
            REFLECT = 0x04,
            FLEE = 0x0b
        };

        /**
         * Directions, to be used as bitmask values
         */
        enum { DOWN = 1, LEFT = 2, UP = 4, RIGHT = 8 };

        Uint16 mJob;          /**< Job (player job, npc, monster, ) */
        Uint16 mX, mY;        /**< Tile coordinates */
        Action mAction;       /**< Action the being is performing */
        int mFrame;
        int mWalkTime;
        int mEmotion;         /**< Currently showing emotion */
        int mEmotionTime;     /**< Time until emotion disappears */
        int mSpeechTime;

        int mAttackSpeed;     /**< Attack speed */

        /**
         * Constructor.
         */
        Being(const int id, const int job, Map *map);

        /**
         * Destructor.
         */
        virtual ~Being();

        /**
         * Removes all path nodes from this being.
         */
        void clearPath();

        /**
         * Sets a new destination for this being to walk to.
         */
        virtual void setDestination(const Uint16 &destX, const Uint16 &destY);

        /**
         * Puts a "speech balloon" above this being for the specified amount
         * of time.
         *
         * @param text The text that should appear.
         * @param time The amount of time the text should stay in milliseconds.
         */
        void setSpeech(const std::string &text, const int time = 500);

        /**
         * Puts a damage bubble above this being.
         *
         * @param attacker the attacking being
         * @param damage the amount of damage recieved (0 means miss)
         * @param type the attack type
         */
        virtual void takeDamage(const Being *attacker, const int damage,
                                const AttackType &type);

        /**
         * Handles an attack of another being by this being.
         *
         * @param victim the victim being
         * @param damage the amount of damage dealt (0 means miss)
         * @param type the attack type
         */
        virtual void handleAttack(Being *victim, const int damage,
                                  const AttackType &type);

        /**
         * Returns the name of the being.
         */
        const std::string& getName() const { return mName; }

        /**
         * Sets the name for the being.
         *
         * @param name The name that should appear.
         */
        virtual void setName(const std::string &name) { mName = name; }

        /**
         * Gets the hair color for this being.
         */
        const int getHairColor() const { return mHairColor; }

        /**
         * Gets the hair style for this being.
         */
        const int getHairStyle() const { return mHairStyle; }

        /**
         * Get the number of hairstyles implemented
         */
        static const int getNumOfHairstyles() { return mNumberOfHairstyles; }

        /**
         * Sets the hair style and color for this being.
         */
        virtual void setHairStyle(const int style, const int color);

        /**
         * Sets visible equipments for this being.
         */
        virtual void setSprite(const int slot, const int id,
                               const std::string &color = "");

        /**
         * Sets the gender of this being.
         */
        virtual void setGender(const Gender &gender) { mGender = gender; }

        /**
         * Gets the gender of this being.
         */
        const Gender &getGender() const { return mGender; }

        /**
         * Makes this being take the next step of his path.
         */
        virtual void nextStep();

        /**
         * Performs being logic.
         */
        virtual void logic();

        /**
         * Draws the speech text above the being.
         */
        void drawSpeech(const int offsetX, const int offsetY);

        /**
         * Draws the emotion picture above the being.
         */
        void drawEmotion(Graphics *graphics, const int offsetX,
                         const int offsetY);

        /**
         * Returns the type of the being.
         */
        virtual Type getType() const;

        /**
         * Gets the walk speed.
         */
        Uint16 getWalkSpeed() const { return mWalkSpeed; }

        /**
         * Sets the walk speed.
         */
        void setWalkSpeed(const Uint16 &speed);

        /**
         * Gets the sprite id.
         */
        int getId() const { return mId; }

        /**
         * Sets the sprite id.
         */
        void setId(const int id) { mId = id; }

        /**
         * Sets the map the being is on
         */
        virtual void setMap(Map *map);

        /**
         * Sets the current action.
         */
        virtual void setAction(const Action &action);

        /**
         * Returns the current direction.
         */
        Uint8 getDirection() const { return mDirection; }

        /**
         * Sets the current direction.
         */
        virtual void setDirection(const Uint8 &direction);

        /**
         * Gets the current action.
         */
        const int getWalkTime() { return mWalkTime; }

        /**
         * Returns the direction the being is facing.
         */
        SpriteDirection getSpriteDirection() const;

        /**
         * Draws this being to the given graphics context.
         *
         * @see Sprite::draw(Graphics, int, int)
         */
        virtual void draw(Graphics *graphics, const int offsetX,
                          const int offsetY) const;

        /**
         * Returns the pixel X coordinate.
         */
        const int getPixelX() const { return mPx; }

        /**
         * Returns the pixel Y coordinate.
         *
         * @see Sprite::getPixelY()
         */
        const int getPixelY() const { return mPy; }

        /**
         * Get the current X pixel offset.
         */
        const int getXOffset() const { return getOffset(LEFT, RIGHT); }

        /**
         * Get the current Y pixel offset.
         */
        const int getYOffset() const { return getOffset(UP, DOWN); }

        /**
         * Returns the horizontal size of the current base sprite of the being
         */
        virtual const int getWidth() const;

        /**
         * Returns the vertical size of the current base sprite of the being
         */
        virtual const int getHeight() const;

        /**
         * Returns the required size of a target cursor for this being.
         */
        virtual Being::TargetCursorSize getTargetCursorSize() const
        { return TC_MEDIUM; }

        /**
         * Take control of a particle.
         */
        void controlParticle(Particle *particle);

        /**
         * Sets the target animation for this being.
         */
        void setTargetAnimation(SimpleAnimation* animation);

        /**
         * Sets whether or not to show particle effects.
         */
        void setUseParticleEffects(bool particles)
            { mParticleEffects = particles; }

        /**
         * Untargets the being
         */
        void untarget() { mUsedTargetCursor = NULL; }

        void setEmote(const Uint8 &emotion, const Uint8 &emote_time)
        {
            mEmotion = emotion;
            mEmotionTime = emote_time;
        }

        virtual AnimatedSprite* getSprite(const int index) const
            { return mSprites[index]; }

        static void load();

    protected:
        /**
         * Sets the new path for this being.
         */
        void setPath(const Path &path);

        /**
         * Let the sub-classes react to a replacement
         */
        virtual void updateCoords() {}

        int mId;                        /**< Unique sprite id */
        Uint16 mWalkSpeed;              /**< Walking speed */
        Uint8 mDirection;               /**< Facing direction */
        Map *mMap;                      /**< Map on which this being resides */
        std::string mName;              /**< Name of character */
        SpriteIterator mSpriteIterator;
        bool mParticleEffects;          /**< Whether to display particles or not */

        /** Engine-related infos about weapon. */
        const ItemInfo* mEquippedWeapon;

        static int mNumberOfHairstyles; /** Number of hair styles in use */

        Path mPath;
        std::string mSpeech;
        std::string mOldSpeech;
        Text *mText;
        Uint16 mHairStyle, mHairColor;
        Gender mGender;
        int mPx, mPy;                   /**< Pixel coordinates */

        const gcn::Color* mNameColor;

        int mLastUpdate;

        std::vector<AnimatedSprite*> mSprites;
        std::vector<int> mSpriteIDs;
        std::vector<std::string> mSpriteColors;
        ParticleList mChildParticleEffects;

        BeingConfigListener *mConfigListener;

    private:
        /**
         * Calculates the offset in the given directions.
         * If walking in direction 'neg' the value is negated.
         */
        const int getOffset(const char &pos, const char &neg) const;

        // Speech Bubble components
        SpeechBubble *mSpeechBubble;

        // Target cursor being used
        SimpleAnimation* mUsedTargetCursor;
};

#endif
