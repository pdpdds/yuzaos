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

#ifndef NPC_H
#define NPC_H

#include "player.h"

class Graphics;
class Text;

class NPC : public Player
{
    public:
        NPC(const int id, const int job, Map *map);

        ~NPC();

        /**
         * Loads initial particle effects.
         */
        void loadInitialParticleEffects();

        void setName(const std::string &name);
        void setGender(const Gender &gender);
        void setSprite(const int slot, const int id, const std::string &color);

        virtual Type getType() const;

        void talk();

        static bool mTalking;

        void updateCoords();

    private:
        Text *mName;
        int job;
};

extern int current_npc;

#endif
