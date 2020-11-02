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

#include "animation.h"
#include "image.h"
#include "imageset.h"
#include "simpleanimation.h"

#include "../log.h"
#include "../resourcemanager.h"

#include "../utils/dtor.h"

#include "../../bindings/guichan/graphics.h"

SimpleAnimation::SimpleAnimation(Animation *animation):
    mAnimation(animation),
    mAnimationTime(0),
    mAnimationPhase(0),
    mCurrentFrame(mAnimation->getFrame(0))
{
};

SimpleAnimation::SimpleAnimation(xmlNodePtr animationNode):
    mAnimationTime(0),
    mAnimationPhase(0)
{
    mAnimation = new Animation();

    ImageSet *imageset = ResourceManager::getInstance()->getImageSet(
        XML::getProperty(animationNode, "imageset", ""),
        XML::getProperty(animationNode, "width", 0),
        XML::getProperty(animationNode, "height", 0)
    );

    // Get animation frames
    for (xmlNodePtr frameNode = animationNode->xmlChildrenNode; frameNode;
         frameNode = frameNode->next)
    {
        const int delay = XML::getProperty(frameNode, "delay", 0);
        const int offsetX = XML::getProperty(frameNode, "offsetX", 0) - 
                            imageset->getHeight() - 32;
        const int offsetY = XML::getProperty(frameNode, "offsetY", 0) -
                            imageset->getWidth() / 2 - 16;

        if (xmlStrEqual(frameNode->name, BAD_CAST "frame"))
        {
            const int index = XML::getProperty(frameNode, "index", -1);

            if (index < 0)
            {
                logger->log("No valid value for 'index'");
                continue;
            }

            Image *img = imageset->get(index);

            if (!img)
            {
                logger->log("No image at index %d", index);
                continue;
            }

            mAnimation->addFrame(img, delay, offsetX, offsetY);
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "sequence"))
        {
            int start = XML::getProperty(frameNode, "start", -1);
            const int end = XML::getProperty(frameNode, "end", -1);

            if (start < 0 || end < 0)
            {
                logger->log("No valid value for 'start' or 'end'");
                continue;
            }

            while (end >= start)
            {
                Image *img = imageset->get(start);

                if (!img)
                {
                    logger->log("No image at index %d", start);
                    continue;
                }

                mAnimation->addFrame(img, delay, offsetX, offsetY);
                start++;
            }
        }
        else if (xmlStrEqual(frameNode->name, BAD_CAST "end"))
            mAnimation->addTerminator();
    }

    mCurrentFrame = mAnimation->getFrame(0);
}

bool SimpleAnimation::draw(Graphics* graphics, const int posX, const int posY) const
{
    if (!mCurrentFrame || !mCurrentFrame->image)
        return false;

    return graphics->drawImage(mCurrentFrame->image,
                               posX + mCurrentFrame->offsetX,
                               posY + mCurrentFrame->offsetY);
}

void SimpleAnimation::reset()
{
    mAnimationTime = 0;
    mAnimationPhase = 0;
}

void SimpleAnimation::setFrame(unsigned int frame)
{
    if (frame >= mAnimation->getLength())
        frame = mAnimation->getLength() - 1;

    mAnimationPhase = frame;
    mCurrentFrame = mAnimation->getFrame(mAnimationPhase);
}

void SimpleAnimation::update(const unsigned int timePassed)
{
    mAnimationTime += timePassed;

    while (mAnimationTime > mCurrentFrame->delay && mCurrentFrame->delay > 0)
    {
        mAnimationTime -= mCurrentFrame->delay;
        mAnimationPhase++;

        if (mAnimationPhase >= mAnimation->getLength())
            mAnimationPhase = 0;

        mCurrentFrame = mAnimation->getFrame(mAnimationPhase);
    }
}

int SimpleAnimation::getLength()
{
    return mAnimation->getLength();
}

Image *SimpleAnimation::getCurrentImage() const
{
    return mCurrentFrame->image;
}

SimpleAnimation::~SimpleAnimation()
{
    destroy(mAnimation);
}
