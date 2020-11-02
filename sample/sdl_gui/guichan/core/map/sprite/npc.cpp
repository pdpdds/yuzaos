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

#include "animatedsprite.h"
#include "npc.h"

#include "../../image/particle/particle.h"

#include "../../../bindings/guichan/palette.h"
#include "../../../bindings/guichan/text.h"

#include "../../../core/utils/dtor.h"

#include "../../../eathena/beingmanager.h"

#include "../../../eathena/db/npcdb.h"

#include "../../../eathena/gui/npctext.h"

#include "../../../eathena/net/messageout.h"
#include "../../../eathena/net/protocol.h"

bool NPC::mTalking = false;
int current_npc = 0;

static const int NAME_X_OFFSET = 15;
static const int NAME_Y_OFFSET = 30;

NPC::NPC(const int id, const int job, Map *map):
    Player(id, job, map)
{
    this->job = job;
    const NPCInfo info = NPCDB::get(job);

    // Setup NPC sprites
    int c = BASE_SPRITE;
    for (std::list<NPCsprite*>::const_iterator i = info.sprites.begin();
         i != info.sprites.end(); i++)
    {
        if (c == VECTOREND_SPRITE)
            break;

        std::string file = "graphics/sprites/" + (*i)->sprite;
        int variant = (*i)->variant;
        mSprites[c] = AnimatedSprite::load(file, variant);
        c++;
    }

    loadInitialParticleEffects();

    mName = NULL;

    mNameColor = &guiPalette->getColor(Palette::NPC);
}

NPC::~NPC()
{
    destroy(mName);
}

void NPC::loadInitialParticleEffects()
{
    mChildParticleEffects.clear();

    if (mParticleEffects)
    {
        const NPCInfo info = NPCDB::get(job);

        //setup particle effects
        for (std::list<std::string>::const_iterator i = info.particles.begin();
             i != info.particles.end(); i++)
        {
            controlParticle(particleEngine->addEffect(*i, 0, 0));
        }
    }
}

void NPC::setName(const std::string &name)
{
    const std::string displayName = name.substr(0, name.find('#', 0));

    destroy(mName);
    mName = new Text(displayName, mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET,
                     gcn::Graphics::CENTER, &guiPalette->getColor(Palette::NPC));
    Being::setName(displayName + " (NPC)");
}

void NPC::setGender(const Gender &gender)
{
    Being::setGender(gender);
}

void NPC::setSprite(const int slot, const int id, const std::string &color)
{
    // Fix this later should it not be adequate enough.
    Being::setSprite(slot, id, color);
}

Being::Type NPC::getType() const
{
    return Being::NPC;
}

void NPC::talk()
{
    if (mTalking)
        return;

    mTalking = true;

    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(mId);
    outMsg.writeInt8(0);
}

void NPC::updateCoords()
{
    if (mName)
        mName->adviseXY(mPx + NAME_X_OFFSET, mPy + NAME_Y_OFFSET);
}
