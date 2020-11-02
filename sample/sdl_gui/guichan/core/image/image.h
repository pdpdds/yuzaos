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

#ifndef IMAGE_H
#define IMAGE_H

#include <SDL.h>

#ifdef HAVE_CONFIG_H
#include "../../../config.h"
#endif

#ifdef USE_OPENGL

/* The definition of OpenGL extensions by SDL is giving problems with recent
 * gl.h headers, since they also include these definitions. As we're not using
 * extensions anyway it's safe to just disable the SDL version.
 */
#define NO_SDL_GLEXT

#include <SDL_opengl.h>
#endif

#include "../resource.h"

class Dye;
struct SDL_Rect;
struct SDL_Surface;
class SubImage;

/**
 * Defines a class for loading and storing images.
 */
class Image : public Resource
{
    friend class SDLGraphics;
#ifdef USE_OPENGL
    friend class OpenGLGraphics;
#endif

    friend class SubImage;

    public:
        /**
         * Destructor.
         */
        virtual ~Image();

        /**
         * Loads an image from a buffer in memory.
         *
         * @param buffer     The memory buffer containing the image data.
         * @param bufferSize The size of the memory buffer in bytes.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(void *buffer, unsigned bufferSize);

        /**
         * Loads an image from a buffer in memory and recolors it.
         *
         * @param buffer     The memory buffer containing the image data.
         * @param bufferSize The size of the memory buffer in bytes.
         * @param dye        The dye used to recolor the image.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(void *buffer, unsigned bufferSize,
                              const Dye &dye);

        /**
         * Loads a resized image from another image. Essentially just a wrapper
         * to the resize function so that the ResourceManager can resize images.
         *
         * @param image      The image to resize.
         * @param w          The width of the rescaled image.
         * @param h          The height of the rescaled image.
         *
         * @return <code>NULL</code> if an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *resize(Image *image, const int w, const int h);


        /**
         * Loads an image from an SDL surface.
         */
        static Image *load(SDL_Surface *);

        /**
         * Frees the resources created by SDL.
         */
        virtual void unload();

        /**
         * Returns the width of the image.
         */
        virtual int getWidth() const { return mBounds.w; }

        /**
         * Returns the height of the image.
         */
        virtual int getHeight() const { return mBounds.h; }

        /**
         * Creates a new image with the desired clipping rectangle.
         *
         * @return <code>NULL</code> if creation failed and a valid
         *         object otherwise.
         */
        virtual SubImage *getSubImage(const int x, const int y,
                                      const int width, const int height);

        /**
         * Sets the alpha value of this image.
         */
        virtual void setAlpha(float alpha);

        /**
         * Returns the alpha value of this image.
         */
        float getAlpha() const;

#ifdef USE_OPENGL
        /**
         * Sets the target image format. Use <code>false</code> for SDL and
         * <code>true</code> for OpenGL.
         */
        static void setLoadAsOpenGL(const bool useOpenGL);

        int getTextureWidth() const { return mTexWidth; }
        int getTextureHeight() const { return mTexHeight; }
        static int getTextureType() { return mTextureType; }
#endif

        /**
         * Merges two image SDL_Surfaces together. This is for SDL use only, as
         * reducing the number of surfaces that SDL has to render can cut down
         * on the number of blit operations necessary, which in turn can help
         * improve overall framerates. Don't use unless you are using it to
         * reduce the number of overall layers that need to be drawn through SDL.
         */
        Image* merge(Image* image, const int x, const int y);

        /**
         * Resizes an image to a given width or height.
         *
         * TODO: Implement OpenGL routines to do this as well.
         */
        Image* resize(const int width, const int height);

    protected:
        /**
         * Returns the current alpha value at a given pixel. Used to compute
         * what the alpha value should be for a given pixel in SDL mode.
         */
        Uint8* mStoredAlpha;

        /**
         * Constructor.
         */
#ifdef USE_OPENGL
        Image(const GLuint &glimage, const int width, const int height,
              const int texWidth, const int texHeight);

        /**
         * Returns the first power of two equal or bigger than the input.
         */
        static int powerOfTwo(const int input);
#endif
        Image(SDL_Surface *image, Uint8* alphas = NULL);

        SDL_Rect mBounds;
        bool mLoaded;

#ifdef USE_OPENGL
        GLuint mGLImage;
        int mTexWidth, mTexHeight;

        static bool mUseOpenGL;
        static int mTextureType;
        static int mTextureSize;
#endif
        SDL_Surface *mImage;
        float mAlpha;
};

/**
 * A clipped version of a larger image.
 */
class SubImage : public Image
{
    public:
        /**
         * Constructor.
         */
        SubImage(Image *parent, SDL_Surface *image, const int x, const int y,
                 const int width, const int height);
#ifdef USE_OPENGL
        SubImage(Image *parent, const GLuint &image, const int x, const int y,
                 const int width, const int height, const int texWidth,
                 const int textHeight);
#endif

        /**
         * Destructor.
         */
        virtual ~SubImage();

        /**
         * Creates a new image with the desired clipping rectangle.
         *
         * @return <code>NULL</code> if creation failed and a valid
         *         image otherwise.
         */
        SubImage *getSubImage(const int x, const int y, const int width,
                              const int height);

        /**
         * Sets the alpha value of this image.
         */
        virtual void setAlpha(float alpha);

    private:
        Image *mParent;
};

#endif
