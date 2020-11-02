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

#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include <memory>

#include "player.h"

class Equipment;
class FloorItem;
class ImageSet;
class Inventory;
class Item;
class Map;

/**
 * The local player character.
 */
class LocalPlayer : public Player
{
    public:
        enum Attribute
        {
            STR = 0xD,
            AGI = 0xE,
            VIT = 0xF,
            INT = 0x10,
            DEX = 0x11,
            LUK = 0x12
        };

        /**
         * Constructor.
         */
        LocalPlayer(const Uint32 &id, const Uint16 &job, Map *map);

        /**
         * Destructor.
         */
        ~LocalPlayer();

        virtual void setName(const std::string &name);
        virtual void logic();

        virtual void setAction(const Action &action);

        /**
         * Sets the current direction.
         */
        virtual void setDirection(const Uint8 &direction);

        /**
         * Adds a new step when walking before calling super. Also, when
         * specified it picks up an item at the end of a path.
         */
        virtual void nextStep();

        /**
         * Returns the player's inventory.
         */
        Inventory* getInventory() const { return mInventory; }

        /**
         * Returns the player's storage
         */
        Inventory* getStorage() const { return mStorage; }

        /**
         * Equips an item.
         */
        void equipItem(Item *item);

        /**
         * Unequips an item.
         */
        void unequipItem(Item *item);

        void useItem(Item *item);
        void dropItem(Item *item, const int quantity);
        void pickUp(FloorItem *item);

        /**
         * Sets the attack range.
         */
        void setAttackRange(const int range) { mAttackRange = range; }

        /**
         * Gets the attack range.
         */
        int getAttackRange() const { return mAttackRange; }

        /**
         * Sents a trade request to the given being.
         */
        void trade(Being *being) const;

        /**
         * Accept or decline a trade offer
         */
        void tradeReply(const bool accept);

        /**
         * Returns true when the player is ready to accept a trade offer.
         * Returns false otherwise.
         */
        bool tradeRequestOk() const;

        /**
         * Sets the trading state of the player, i.e. whether or not he is
         * currently involved into some trade.
         */
        void setTrading(const bool trading) { mTrading = trading; }

        void attack(Being *target = NULL, const bool keep = false);

        /**
         * Triggers whether or not to show the name as a GM name.
         * NOTE: This doesn't mean that just anyone can use this.
         * If the server doesn't acknowlege you, you won't be shown
         * as a GM on other people's clients.
         */
        virtual void setGM();

        void stopAttack();

        /**
         * Overridden to do nothing. The attacks of the local player are
         * displayed as soon as the player attacks, not when the server says
         * the player does.
         *
         * @param victim the victim being
         * @param damage the amount of damage dealt (0 means miss)
         * @param type the attack type
         */
        virtual void handleAttack(Being *victim, const int damage,
                                  const AttackType &type) {}
        /**
         * Returns the current target of the player. Returns 0 if no being is
         * currently targeted.
         */
        Being* getTarget() const;

        /**
         * Sets the target being of the player.
         */
        void setTarget(Being* target);

        /**
         * Sets a new destination for this being to walk to.
         */
        virtual void setDestination(const Uint16 &x, const Uint16 &y);

        /**
         * Sets a new direction to keep walking in.
         */
        void setWalkingDir(const int dir);

        /**
         * Sets going to being to attack
         */
        void setGotoTarget(Being *target);

        /**
         * Returns whether the target is in range to attack
         */
        bool withinAttackRange(Being *target);

        void raiseAttribute(const Attribute &attr);
        void raiseSkill(const Uint16 &skillId);

        void toggleSit();
        void emote(const Uint8 &emotion);

        void revive();

        /**
         * Accessors for mInStorage
         */
        bool getInStorage() { return mInStorage; }
        void setInStorage(const bool inStorage);

        /**
         * Sets the amount of XP. Shows XP gaining effect if the player is on
         * a map.
         */
        void setXp(const int xp);

        /**
         * Shows item pickup effect if the player is on a map.
         */
        void pickedUp(const std::string &item);

        /**
         * Returns the amount of experience points.
         */
        const int getXp() const { return mXp; }

        /**
         * Returns whether the player is still attacking or not.
         */
        const bool isAttacking() const { return mKeepAttacking; }

        /**
         * Overridden to clear attacking on switching maps.
         */
        virtual void setMap(Map *map);

        Uint32 mCharId;     /**< Used only during character selection. */

        Uint32 mJobXp;
        Uint16 mLevel;
        Uint32 mJobLevel;
        Uint32 mXpForNextLevel, mJobXpForNextLevel;
        Uint16 mHp, mMaxHp, mMp, mMaxMp;
        Uint32 mGp;

        Uint16 mAttackRange;

        int mTotalWeight, mMaxWeight;

        Uint8 mAttr[6];
        Uint8 mAttrUp[6];

        int ATK, MATK, DEF, MDEF, HIT, FLEE;
        int ATK_BONUS, MATK_BONUS, DEF_BONUS, MDEF_BONUS, FLEE_BONUS;

        Uint16 mStatPoint, mSkillPoint;
        Uint16 mStatsPointsToAttribute;

        bool mUpdateName;     /** Whether or not the name settings have changed */

        const std::auto_ptr<Equipment> mEquipment;

    protected:
        void walk(const unsigned char &dir);

        int mXp;            /**< Experience points. */

        Being *mTarget;
        FloorItem *mPickUpTarget;

        bool mTrading;
        bool mInStorage;      /**< Whether storage is currently accessible */
        bool mGoingToTarget;
        bool mKeepAttacking;  /** Whether or not to continue to attack */
        int mTargetTime;      /** How long the being has been targeted **/
        int mLastAction;      /**< Time stamp of the last action, -1 if none. */
        int mWalkingDir;      /**< The direction the player is walking in. */
        int mDestX;           /**< X coordinate of destination. */
        int mDestY;           /**< Y coordinate of destination. */

        Inventory *mInventory;
        Inventory *mStorage;

        // Load the target cursors into memory
        void initTargetCursor();

        /**
         * Helper function for loading target cursors
         */
        void loadTargetCursor(const std::string &filename, const int width,
                              const int height, const bool outRange,
                              const Being::TargetCursorSize &size);

        /** Images of the target cursor. */
        ImageSet *mTargetCursorImages[2][NUM_TC];

        /** Animated target cursors. */
        SimpleAnimation *mTargetCursor[2][NUM_TC];
};

extern LocalPlayer *player_node;

#endif
