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

#include <cassert>
#include <cmath>

#include "animatedsprite.h"
#include "being.h"
#include "localplayer.h"

#include "../../resourcemanager.h"

#include "../../image/image.h"
#include "../../image/simpleanimation.h"

#include "../../image/particle/particle.h"

#include "../../../bindings/guichan/graphics.h"
#include "../../../bindings/guichan/gui.h"
#include "../../../bindings/guichan/palette.h"
#include "../../../bindings/guichan/text.h"

#include "../../../bindings/guichan/widgets/speechbubble.h"

#include "../../../bindings/sdl/sound.h"

#include "../../../core/configuration.h"
#include "../../../core/configlistener.h"
#include "../../../core/log.h"

#include "../../../core/map/map.h"

#include "../../../core/utils/dtor.h"
#include "../../../core/utils/gettext.h"
#include "../../../core/utils/stringutils.h"

#include "../../../eathena/db/colordb.h"
#include "../../../eathena/db/emotedb.h"
#include "../../../eathena/db/effectdb.h"
#include "../../../eathena/db/itemdb.h"
#include "../../../eathena/db/iteminfo.h"

#include "../../../eathena/gui/viewport.h"

#include "../../../eathena/net/messageout.h"
#include "../../../eathena/net/protocol.h"

int Being::mNumberOfHairstyles = 1;

static const int X_SPEECH_OFFSET = 18;
static const int Y_SPEECH_OFFSET = 60;

static const int DEFAULT_WIDTH = 32;
static const int DEFAULT_HEIGHT = 32;

class BeingConfigListener : public ConfigListener
{
    public:
        BeingConfigListener(Being *b):
            mBeing(b)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "particleeffects")
            {
                const bool bParticleEffects = config.getValue("particleeffects", 1);
                mBeing->setUseParticleEffects(bParticleEffects);
            }
        }
    private:
        Being *mBeing;
};

Being::Being(const int id, const int job, Map *map):
    mJob(job),
    mX(0), mY(0),
    mAction(STAND),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mSpeechTime(0),
    mAttackSpeed(350),
    mId(id),
    mWalkSpeed(150),
    mDirection(DOWN),
    mMap(NULL),
    mName(""),
    mParticleEffects(config.getValue("particleeffects", 1)),
    mEquippedWeapon(NULL),
    mHairStyle(1), mHairColor(0),
    mGender(GENDER_UNSPECIFIED),
    mPx(0), mPy(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mSpriteIDs(VECTOREND_SPRITE, 0),
    mSpriteColors(VECTOREND_SPRITE, ""),
    mChildParticleEffects(),
    mUsedTargetCursor(NULL)
{
    setMap(map);

    mConfigListener = new BeingConfigListener(this);
    config.addListener("particleeffects", mConfigListener);

    mSpeech = "";
    mOldSpeech = "";
    mNameColor = &guiPalette->getColor(Palette::CHAT);

    mSpeechBubble = NULL;
    mText = NULL;

    mLastUpdate = tick_time;
}

Being::~Being()
{
    mUsedTargetCursor = NULL;
    delete_all(mSprites);
    clearPath();

    config.removeListener("particleeffects", mConfigListener);

    if (player_node && player_node->getTarget() == this)
        player_node->setTarget(NULL);

    setMap(NULL);

    destroy(mSpeechBubble);
    destroy(mText);
}

void Being::setDestination(const Uint16 &destX, const Uint16 &destY)
{
    if (mMap)
        setPath(mMap->findPath(mX, mY, destX, destY));
}

void Being::clearPath()
{
    mPath.clear();
}

void Being::setPath(const Path &path)
{
    mPath = path;

    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
}

void Being::setHairStyle(const int style, const int color)
{
    mHairStyle = style < 0 ? mHairStyle : style % mNumberOfHairstyles;
    mHairColor = color < 0 ? mHairColor : color % ColorDB::size();
}

void Being::setSprite(const int slot, const int id, const std::string &color)
{
    assert(slot >= BASE_SPRITE && slot < VECTOREND_SPRITE);
    mSpriteIDs[slot] = id;
    mSpriteColors[slot] = color;
}

void Being::setSpeech(const std::string &text, const int time)
{
    mSpeech = text;

    // Trim whitespace
    trim(mSpeech);

    // check for links
    std::string::size_type start = mSpeech.find('[');
    std::string::size_type end = mSpeech.find(']', start);

    while (start != std::string::npos && end != std::string::npos)
    {
        // Catch multiple embeds and ignore them so it doesn't crash the client.
        while ((mSpeech.find('[', start + 1) != std::string::npos) &&
               (mSpeech.find('[', start + 1) < end))
        {
            start = mSpeech.find('[', start + 1);
        }

        std::string::size_type position = mSpeech.find('|');
        if (end > 2 && mSpeech[start + 1] == '@' && mSpeech[start + 2] == '@' &&
            mSpeech[end - 1] == '@' && mSpeech[end - 2] == '@' &&
            position != std::string::npos)
        {
            mSpeech.erase(end, 1);
            mSpeech.erase(start, position - start + 1);
        }
        else
        {
            std::string temp = mSpeech.substr(start + 1, end - start - 1);
            trim(temp);

            const ItemInfo itemInfo = ItemDB::get(temp);

            if (itemInfo.getName() != _("Unknown item"))
            {
                mSpeech.erase(start, 1);
                mSpeech.erase(end - 1, 1);
            }
        }

        position = mSpeech.find('@');

        if (position != std::string::npos && position > 0 &&
            mSpeech[position - 1] != '[' && mSpeech[position] == '@' &&
            mSpeech[position + 1] == '@')
            mSpeech.erase(position, 2);

        start = mSpeech.find('[', start + 1);
        end = mSpeech.find(']', start);
    }

    if (!mSpeech.empty())
        mSpeechTime = time <= SPEECH_MAX_TIME ? time : SPEECH_MAX_TIME;
}

void Being::takeDamage(const Being *attacker, const int amount,
                       const AttackType &type)
{
    gcn::Font *font;
    std::string damage = amount ? toString(amount) : type == FLEE ?
            "dodge" : "miss";
    const gcn::Color* color;

    font = gui->getInfoParticleFont();

    // Selecting the right color
    if (type == CRITICAL || type == FLEE)
        color = &guiPalette->getColor(Palette::HIT_CRITICAL);
    else if (!amount)
    {
        // This is intended to be the wrong direction to visually
        // differentiate between hits and misses
        if (attacker == player_node)
            color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);
        else
            color = &guiPalette->getColor(Palette::MISS);
    }
    else if (getType() == MONSTER)
        color = &guiPalette->getColor(Palette::HIT_PLAYER_MONSTER);
    else
        color = &guiPalette->getColor(Palette::HIT_MONSTER_PLAYER);

    if (amount > 0 && type == CRITICAL)
        particleEngine->addTextSplashEffect("crit!", mPx + 16, mPy + 16,
                                            color, font, true);

    // Show damage number
    particleEngine->addTextSplashEffect(damage, mPx + 16, mPy + 16,
                                        color, font, true);
}

void Being::handleAttack(Being *victim, const int damage,
                         const AttackType &type)
{
    if (this != player_node)
        setAction(Being::ATTACK);

    mFrame = 0;
    mWalkTime = tick_time;
}

void Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap)
        mMap->removeSprite(mSpriteIterator);

    mMap = map;

    // Add sprite to potential new map
    if (mMap)
        mSpriteIterator = mMap->addSprite(this);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
}

void Being::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void Being::setAction(const Action &action)
{
    SpriteAction currentAction = ACTION_INVALID;

    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case SIT:
            currentAction = ACTION_SIT;
            break;
        case ATTACK:
            if (mEquippedWeapon)
                currentAction = mEquippedWeapon->getAttackType();
            else
                currentAction = ACTION_ATTACK;

            for (int i = 0; i < VECTOREND_SPRITE; i++)
            {
                if (mSprites[i])
                    mSprites[i]->reset();
            }
            break;
        case HURT:
            //currentAction = ACTION_HURT;  // Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
                mSprites[i]->play(currentAction);
        }
        mAction = action;
    }
}

void Being::setDirection(const Uint8 &direction)
{
    if (mDirection == direction)
        return;

    mDirection = direction;
    SpriteDirection dir = getSpriteDirection();

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i])
            mSprites[i]->setDirection(dir);
    }
}

SpriteDirection Being::getSpriteDirection() const
{
    SpriteDirection dir;

    if (mDirection & UP)
        dir = DIRECTION_UP;
    else if (mDirection & DOWN)
        dir = DIRECTION_DOWN;
    else if (mDirection & RIGHT)
        dir = DIRECTION_RIGHT;
    else
        dir = DIRECTION_LEFT;

    return dir;
}

void Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    Position pos = mPath.front();
    mPath.pop_front();

    int dir = 0;
    if (pos.x > mX)
        dir |= RIGHT;
    else if (pos.x < mX)
        dir |= LEFT;
    if (pos.y > mY)
        dir |= DOWN;
    else if (pos.y < mY)
        dir |= UP;

    setDirection(dir);

    if (mMap->tileCollides(pos.x, pos.y))
    {
        setAction(STAND);
        return;
    }

    mX = pos.x;
    mY = pos.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}

void Being::logic()
{
    const int ticks = get_elapsed_time(mLastUpdate) / 10;

    mLastUpdate = tick_time;

    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0 && viewport)
    {
        mSpeechTime = mSpeechTime - ticks;

        if (mSpeechTime <= 0)
        {
            // Remove text and speechbubbles if speech boxes aren't being used
            if (mText)
                destroy(mText);
            else if (mSpeechBubble)
                mSpeechBubble->setVisible(false);
        }
    }

    int oldPx = mPx;
    int oldPy = mPy;

    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();

    if (mPx != oldPx || mPy != oldPy)
        updateCoords();

    if (mEmotion != 0)
    {
        mEmotionTime = mEmotionTime - ticks;
        if (mEmotionTime <= 0)
            mEmotion = 0;
    }

    // Update sprite animations
    if (mUsedTargetCursor != NULL)
        mUsedTargetCursor->update(tick_time * 10);

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i])
            mSprites[i]->update(tick_time * 10);
    }

    // Update particle effects
    mChildParticleEffects.moveTo((float) mPx + 16.0f, (float) mPy + 32.0f);
}

void Being::draw(Graphics *graphics, const int offsetX,
                 const int offsetY) const
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    if (mUsedTargetCursor != NULL)
        mUsedTargetCursor->draw(graphics, px, py);

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i])
            mSprites[i]->draw(graphics, px, py);
    }
}

void Being::drawEmotion(Graphics *graphics, const int offsetX,
                        const int offsetY)
{
    if (!mEmotion)
        return;

    const int px = mPx - offsetX;
    const int py = mPy - offsetY - 64;
    const int emotionIndex = mEmotion - 1;
    const AnimatedSprite *sprite = EmoteDB::getAnimation(emotionIndex);

    if (sprite)
        sprite->draw(graphics, px, py);
}

void Being::drawSpeech(const int offsetX, const int offsetY)
{
    const int px = mPx - offsetX;
    const int py = mPy - offsetY;
    const int speech = config.getValue("speech", NAME_IN_BUBBLE);

    // Draw speech above this being
    if (mSpeechTime > 0 && (speech == NAME_IN_BUBBLE ||
        speech == NO_NAME_IN_BUBBLE))
    {
        if (!mSpeechBubble)
            mSpeechBubble = new SpeechBubble(viewport);

        const bool hasCaption = (mSpeechBubble->getCaption() != "");
        const bool showName = (speech == NAME_IN_BUBBLE);
        const int width = 16;
        const int height = getHeight() - 32;

        if (mOldSpeech != mSpeech || mText)
        {
            mSpeechBubble->setText(mSpeech);
            mSpeechBubble->adjustSize();
        }

        if (mText)
            destroy(mText);

        mOldSpeech = mSpeech;
        mSpeechBubble->setCaption(showName ? mName : "", mNameColor);

        if (hasCaption != showName)
            mSpeechBubble->adjustSize();

        mSpeechBubble->setPosition(px + width - (mSpeechBubble->getWidth() / 2), 
                                   py - height - (mSpeechBubble->getHeight()));
        mSpeechBubble->setVisible(true);
    }
    else if (mSpeechTime > 0 && speech == TEXT_OVERHEAD)
    {
        if (mSpeech == mOldSpeech && mText)
            mText->adviseXY(mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET);
        else
        {
            // don't introduce a memory leak
            if (mSpeechBubble)
                destroy(mSpeechBubble);
            if (mText)
                destroy(mText);

            mOldSpeech = mSpeech;

            mText = new Text(mSpeech, mPx + X_SPEECH_OFFSET, mPy - Y_SPEECH_OFFSET,
                             gcn::Graphics::CENTER,
                             &guiPalette->getColor(Palette::PARTICLE));
        }
    }
    else if (speech == NO_SPEECH)
    {
        if (mSpeechBubble)
            destroy(mSpeechBubble);
        if (mText)
            destroy(mText);
        mOldSpeech = "";
    }
}

Being::Type Being::getType() const
{
    if (mId >= 110000000)
        return WARP;
    else
        return UNKNOWN;
}

void Being::setWalkSpeed(const Uint16 &speed)
{
    mWalkSpeed = speed > 0 ? speed <= 1000 ? speed : 1000 : 1;
}

const int Being::getOffset(const char &pos, const char &neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK || !(mDirection & (pos | neg)))
        return 0;

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0)
        offset = 0;

    // Going into negative direction? Invert the offset.
    if (mDirection & pos)
        offset = -offset;

    return offset;
}

const int Being::getWidth() const
{
    if (AnimatedSprite *base = mSprites[BASE_SPRITE])
        return std::max(base->getWidth(), DEFAULT_WIDTH);
    else
        return DEFAULT_WIDTH;
}

const int Being::getHeight() const
{
    if (AnimatedSprite *base = mSprites[BASE_SPRITE])
        return std::max(base->getHeight(), DEFAULT_HEIGHT);
    else
        return DEFAULT_HEIGHT;
}

void Being::setTargetAnimation(SimpleAnimation* animation)
{
    mUsedTargetCursor = animation;
    mUsedTargetCursor->reset();
}

void Being::load()
{
    // Hairstyles are encoded as negative numbers. Count how far negative
    // we can go.
    int hairstyles = 1;

    while (ItemDB::get(-hairstyles).getSprite(GENDER_MALE) != "error.xml")
        hairstyles++;

    mNumberOfHairstyles = hairstyles;
}

