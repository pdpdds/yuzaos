/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
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

#include <sstream>

#include <guichan/color.hpp>

#include "dye.h"

#include "../log.h"

#include "../utils/dtor.h"

DyePalette::DyePalette(const std::string &description)
{
    const int size = description.length();

    if (size == 0 || description[0] != '#')
        // TODO: load palette from file.
        return;

    int pos = 1;
    for (;;)
    {
        if (pos + 6 > size)
            break;

        int val = 0;
        const std::string str = description.substr(pos, pos + 6);

        sscanf(str.c_str(), "%06x", &val);
        mColors.push_back(new gcn::Color(val));
        pos += 6;

        if (pos == size)
            return;
        if (description[pos] != ',')
            break;

        ++pos;
    }
}

DyePalette::~DyePalette()
{
    delete_all(mColors);
}

void DyePalette::getColor(const int intensity, gcn::Color* color) const
{
    if (intensity == 0)
    {
        color->r = 0;
        color->g = 0;
        color->b = 0;
        return;
    }

    const int last = mColors.size();

    if (last == 0)
        return;

    const int i = intensity * last / 255;
    const int t = intensity * last % 255;

    const int j = t != 0 ? i : i - 1;

    // Get the exact color if any, the next color otherwise.
    const int r2 = mColors[j]->r,
              g2 = mColors[j]->g,
              b2 = mColors[j]->b;

    if (t == 0)
    {
        // Exact color.
        color->r = r2;
        color->g = g2;
        color->b = b2;
        return;
    }

    // Get the previous color. First color is implicitly black.
    int r1 = 0, g1 = 0, b1 = 0;
    if (i > 0)
    {
        r1 = mColors[i - 1]->r;
        g1 = mColors[i - 1]->g;
        b1 = mColors[i - 1]->b;
    }

    // Perform a linear interpolation.
    color->r = ((255 - t) * r1 + t * r2) / 255;
    color->g = ((255 - t) * g1 + t * g2) / 255;
    color->b = ((255 - t) * b1 + t * b2) / 255;
}

Dye::Dye(const std::string &description)
{
    for (int i = 0; i < 7; ++i)
        mDyePalettes[i] = 0;

    if (description.empty())
        return;

    std::string::size_type next_pos = 0, length = description.length();
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = description.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = length;

        if (next_pos <= pos + 3 || description[pos + 1] != ':')
        {
            logger->log("Error, invalid dye: %s", description.c_str());
            return;
        }

        int i = 0;

        switch (description[pos])
        {
            case 'R': i = 0; break;
            case 'G': i = 1; break;
            case 'Y': i = 2; break;
            case 'B': i = 3; break;
            case 'M': i = 4; break;
            case 'C': i = 5; break;
            case 'W': i = 6; break;
            default:
                logger->log("Error, invalid dye: %s", description.c_str());
                return;
        }
        mDyePalettes[i] = new DyePalette(description.substr(pos + 2,
                                                            next_pos - pos - 2));
        ++next_pos;
    }
    while (next_pos < length);
}

Dye::~Dye()
{
    for (int i = 0; i < 7; ++i)
        destroy(mDyePalettes[i]);
}

void Dye::update(gcn::Color *color) const
{
    const int cmax = std::max(color->r, std::max(color->g, color->b));
    if (cmax == 0)
        return;

    const int cmin = std::min(color->r, std::min(color->g, color->b));
    const int intensity = color->r + color->g + color->b;

    // not pure
    if (cmin != cmax && (cmin != 0 || (intensity != cmax && intensity != 2 * cmax)))
        return;

    const int i = (color->r != 0) | ((color->g != 0) << 1) | ((color->b != 0) << 2);

    if (mDyePalettes[i - 1])
        mDyePalettes[i - 1]->getColor(cmax, color);
}

void Dye::instantiate(std::string &target, const std::string &palettes)
{
    std::string::size_type next_pos = target.find('|');

    if (next_pos == std::string::npos || palettes.empty())
        return;

    ++next_pos;

    std::ostringstream s;
    s << target.substr(0, next_pos);
    std::string::size_type last_pos = target.length(), pal_pos = 0;
    do
    {
        std::string::size_type pos = next_pos;
        next_pos = target.find(';', pos);

        if (next_pos == std::string::npos)
            next_pos = last_pos;

        if (next_pos == pos + 1 && pal_pos != std::string::npos)
        {
            std::string::size_type pal_next_pos = palettes.find(';', pal_pos);
            s << target[pos] << ':';
            if (pal_next_pos == std::string::npos)
            {
                s << palettes.substr(pal_pos);
                s << target.substr(next_pos);
                pal_pos = std::string::npos;
                break;
            }
            s << palettes.substr(pal_pos, pal_next_pos - pal_pos);
            pal_pos = pal_next_pos + 1;
        }
        else if (next_pos > pos + 2)
        {
            s << target.substr(pos, next_pos - pos);
        }
        else
        {
            logger->log("Error, invalid dye placeholder: %s", target.c_str());
            return;
        }
        s << target[next_pos];
        ++next_pos;
    }
    while (next_pos < last_pos);

    target = s.str();
}
