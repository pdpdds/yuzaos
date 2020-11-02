/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "particle.h"
#include "particlecontainer.h"

#include "../../utils/dtor.h"

ParticleContainer::ParticleContainer(ParticleContainer *parent,
                                     const bool delParent) :
    mDelParent(delParent),
    mNext(parent)
{};

ParticleContainer::~ParticleContainer()
{
    clearLocally();
    if (mNext && mDelParent)
        destroy(mNext);
}

void ParticleContainer::clear()
{
    clearLocally();

    if (mNext)
        mNext->clear();
}

void ParticleContainer::moveTo(const float x, const float y)
{
    if (mNext)
        mNext->moveTo(x, y);
}

// -- particle list ----------------------------------------

ParticleList::ParticleList(ParticleContainer *parent, const bool delParent) :
    ParticleContainer(parent, delParent) {};

ParticleList::~ParticleList() {}

void ParticleList::addLocally(Particle *particle)
{
    if (particle)
    {
        // The effect may not die without the beings permission or we segfault
        particle->disableAutoDelete();
        mElements.push_back(particle);
    }
}

void ParticleList::removeLocally(Particle *particle)
{
    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end(); it++)
    {
        if (*it == particle)
        {
            (*it)->kill();
            // FIXME: cppcheck reports that this is a dangerous delete.
            mElements.erase(it);
        }
    }
}

void ParticleList::clearLocally()
{
    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end(); it++)
        (*it)->kill();
    
    mElements.clear();
}

void ParticleList::moveTo(const float x, const float y)
{
    ParticleContainer::moveTo(x, y);

    for (std::list<Particle *>::iterator it = mElements.begin();
         it != mElements.end();)
    {
        (*it)->moveTo(x, y);
        if ((*it)->isExtinct())
        {
            (*it)->kill();
            it = mElements.erase(it);
        }
        else
            it++;
    }
}

// -- particle vector ----------------------------------------

ParticleVector::ParticleVector(ParticleContainer *parent, const bool delParent) :
    ParticleContainer(parent, delParent) {};

ParticleVector::~ParticleVector() {};

void ParticleVector::setLocally(const int index, Particle *particle)
{
    assert(index >= 0);

    delLocally(index);

    if (mIndexedElements.size() <= (unsigned) index)
        mIndexedElements.resize(index + 1, NULL);

    mIndexedElements[index] = particle;
}

void ParticleVector::delLocally(const int index)
{
    assert(index >= 0);

    if (mIndexedElements.size() <= (unsigned) index)
        return;

    Particle *p = mIndexedElements[index];
    if (p)
    {
        mIndexedElements[index] = NULL;
        p->kill();
    }
}

void ParticleVector::clearLocally()
{
    for (unsigned int i = 0; i < mIndexedElements.size(); i++)
        delLocally(i);
}

void ParticleVector::moveTo(const float x, const float y)
{
    ParticleContainer::moveTo(x, y);

    for (std::vector<Particle *>::iterator it = mIndexedElements.begin();
         it != mIndexedElements.end(); it++)
        if (*it)
        {
            (*it)->moveTo(x, y);

            if ((*it)->isExtinct())
            {
                (*it)->kill();
                *it = NULL;
            }
        }
}

