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

#include "flooritem.h"

#include "../map.h"

#include "../../image/image.h"

#include "../../utils/dtor.h"

#include "../../../bindings/guichan/graphics.h"

#include "../../../eathena/structs/item.h"

FloorItem::FloorItem(const int id, const int itemId, const int x,
                     const int y, Map *map):
    mId(id),
    mX(x),
    mY(y),
    mMap(map)
{
    // Create a corresponding item instance
    mItem = new Item(itemId);

    // Add ourselves to the map
    mSpriteIterator = mMap->addSprite(this);
}

FloorItem::~FloorItem()
{
    // Remove ourselves from the map
    mMap->removeSprite(mSpriteIterator);

    destroy(mItem);
}

const int FloorItem::getItemId() const
{
    return mItem->getId();
}

Item* FloorItem::getItem() const
{
    return mItem;
}

void FloorItem::draw(Graphics *graphics, const int offsetX,
                     const int offsetY) const
{
    graphics->drawImage(mItem->getImage(), mX * 32 + offsetX, mY * 32 + offsetY);
}
