/*
 *  Aethyra
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code from The Mana World.
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

#include <algorithm>
#include <cstring>
#include <physfs.h>

#include "wallpapermanager.h"

#include "../configuration.h"
#include "../log.h"

#include "../utils/stringutils.h"

namespace
{
    const std::string wallpaperFolder = config.getValue("wallpaperFolder",
                                                        "graphics/images/");
    const std::string wallpaperBase = config.getValue("wallpaperBase",
                                                      "login_wallpaper");
    const int defaultWidth = config.getValue("defaultWidth", 800);
    const int defaultHeight = config.getValue("defaultHeight", 600);
}

struct wallpaper {
    Uint16 width;
    Uint16 height;
};

std::vector<struct wallpaper> wallpapers;
bool haveBackup; // Is the backup (no size given) version availabnle?

bool wallpaperCompare(struct wallpaper x, struct wallpaper y)
{
    const int aX = x.width * x.height;
    const int aY = y.width * y.height;

    return aX > aY || (aX == aY && x.width > y.width);
}

void Wallpaper::loadWallpapers()
{
    char **imgs = PHYSFS_enumerateFiles(wallpaperFolder.c_str());
    char **i;
    size_t baseLen = wallpaperBase.size();
    int width;
    int height;

    wallpapers.clear();

    haveBackup = false;

    for (i = imgs; *i != NULL; i++)
    {
        if (strncmp(*i, wallpaperBase.c_str(), baseLen) == 0)
        {
            const std::string path = wallpaperBase + "_%dx%d.png";

            if (strlen(*i) == baseLen + 4)
            {
                if (haveBackup)
                    logger->log("Duplicate default wallpaper!");
                else
                    haveBackup = true;
            }
            else if (sscanf(*i, path.c_str(), &width, &height) == 2)
            {
                struct wallpaper wp;
                wp.width = width;
                wp.height = height;
                wallpapers.push_back(wp);
            }
        }
    }

    PHYSFS_freeList(imgs);

    std::sort(wallpapers.begin(), wallpapers.end(), wallpaperCompare);
}

std::string Wallpaper::getWallpaper(const int width, const int height)
{
    std::vector<wallpaper>::iterator iter;
    wallpaper wp;

    for (iter = wallpapers.begin(); iter != wallpapers.end(); iter++)
    {
        wp = *iter;
        if (wp.width <= width && wp.height <= height)
            return std::string(strprintf("%s%s_%dx%d.png",
                                         wallpaperFolder.c_str(),
                                         wallpaperBase.c_str(),
                                         wp.width, wp.height));
    }

    if (haveBackup)
        return std::string(wallpaperFolder + wallpaperBase + ".png");

    return std::string("");
}

int Wallpaper::getWidth(const std::string &file)
{
    int width = 0, height = 0;
    const std::string path = wallpaperFolder + wallpaperBase + "_%dx%d.png";

    sscanf(file.c_str(), path.c_str(), &width, &height);
    return width > 0 ? width : defaultWidth;
}

int Wallpaper::getHeight(const std::string &file)
{
    int width = 0, height = 0;

    const std::string path = wallpaperFolder + wallpaperBase + "_%dx%d.png";

    sscanf(file.c_str(), path.c_str(), &width, &height);
    return height > 0 ? height : defaultHeight;
}
