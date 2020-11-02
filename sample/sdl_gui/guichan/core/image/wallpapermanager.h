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

#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <string>
#include <vector>

#include <SDL_types.h>

/**
 * Handles organizing and choosing of wallpapers.
 */
class Wallpaper
{
    public:
        /**
         * Reads the folder that contains wallpapers and organizes the
         * wallpapers found by area, width, and height.
         */
        static void loadWallpapers();

        /**
         * Returns the largest wallpaper for the given resolution, or the
         * default wallpaper if none are found.
         *
         * @param width the desired width
         * @param height the desired height
         * @return the file to use, or empty if no wallpapers are useable
         */
        static std::string getWallpaper(const int width, const int height);

        /**
         * Returns the width of the image for given file path.
         *
         * Provided for convenience, and will not be accurate for all files if
         * the sizes don't follow the wallpaper naming rules.
         */
        static int getWidth(const std::string &file);

        /**
         * Returns the height of the image for given file path.
         *
         * Provided for convenience, and will not be accurate for all files if
         * the sizes don't follow the wallpaper naming rules.
         */
        static int getHeight(const std::string &file);
};

#endif // WALLPAPER_H
