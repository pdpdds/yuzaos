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

#include "action.h"
#include "animatedsprite.h"

#include "../../log.h"
#include "../../resourcemanager.h"

#include "../../image/animation.h"
#include "../../image/image.h"

#include "../../utils/xml.h"

#include "../../../bindings/guichan/graphics.h"

AnimatedSprite::AnimatedSprite(SpriteDef *sprite):
    mDirection(DIRECTION_DOWN),
    mLastTime(0),
    mFrameIndex(0),
    mFrameTime(0),
    mSprite(sprite),
    mAction(0),
    mAnimation(0),
    mFrame(0)
{
    assert(mSprite);

    // Take possession of the sprite
    mSprite->incRef();

    // Play the stand animation by default
    play(ACTION_STAND);
}

AnimatedSprite *AnimatedSprite::load(const std::string& filename,
                                     const int variant)
{
    ResourceManager *resman = ResourceManager::getInstance();
    SpriteDef *s = resman->getSprite(filename, variant);

    if (!s)
        return NULL;

    AnimatedSprite *as = new AnimatedSprite(s);
    s->decRef();
    return as;
}

AnimatedSprite::~AnimatedSprite()
{
    mSprite->decRef();
}

void AnimatedSprite::reset()
{
    mFrameIndex = 0;
    mFrameTime = 0;
    mLastTime = 0;
}

void AnimatedSprite::play(const SpriteAction &spriteAction)
{
    Action *action = mSprite->getAction(spriteAction);

    if (!action)
        return;

    mAction = action;

    Animation *animation = mAction->getAnimation(mDirection);

    if (animation && animation != mAnimation && animation->getLength() > 0)
    {
        mAnimation = animation;
        mFrame = mAnimation->getFrame(0);

        reset();
    }
}

void AnimatedSprite::update(const int time)
{
    // Avoid freaking out at first frame or when tick_time overflows
    if (time < mLastTime || mLastTime == 0)
        mLastTime = time;

    // If not enough time has passed yet, do nothing
    if (time <= mLastTime || !mAnimation)
        return;

    unsigned int dt = time - mLastTime;
    mLastTime = time;

    if (!updateCurrentAnimation(dt))
    {
        // Animation finished, reset to default
        play(ACTION_STAND);
    }
}

bool AnimatedSprite::updateCurrentAnimation(const unsigned int time)
{
    if (!mFrame || Animation::isTerminator(*mFrame))
        return false;

    mFrameTime += time;

    while (mFrameTime > mFrame->delay && mFrame->delay > 0)
    {
        mFrameTime -= mFrame->delay;
        mFrameIndex++;

        if (mFrameIndex == mAnimation->getLength())
            mFrameIndex = 0;

        mFrame = mAnimation->getFrame(mFrameIndex);

        if (Animation::isTerminator(*mFrame))
        {
            mAnimation = 0;
            mFrame = 0;
            return false;
        }
    }

    return true;
}

bool AnimatedSprite::draw(Graphics* graphics, const int posX,
                          const int posY) const
{
    if (!mFrame || !mFrame->image)
        return false;

    return graphics->drawImage(mFrame->image,
                               posX + mFrame->offsetX,
                               posY + mFrame->offsetY);
}

void AnimatedSprite::setDirection(const SpriteDirection &direction)
{
    if (mDirection != direction)
    {
        mDirection = direction;

        if (!mAction)
            return;

        Animation *animation = mAction->getAnimation(mDirection);

        if (animation && animation != mAnimation && animation->getLength() > 0)
        {
            mAnimation = animation;
            mFrame = mAnimation->getFrame(0);
            reset();
        }
    }
}

int AnimatedSprite::getWidth() const
{
    return mFrame ? mFrame->image->getWidth() : 0;
}

int AnimatedSprite::getHeight() const
{
    return mFrame ? mFrame->image->getHeight() : 0;
}
