/*
 *  Retrieve string pasted depending on OS mechanisms.
 *  Copyright (C) 2001-2009 Wormux Team.
 *  Copyright (C) 2009 Aethyra Development Team.
 *
 *  This file is part of Aethyra based on original code from Wormux.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef _MSC_VER
#include "msvc/config.h"
#elif defined(HAVE_CONFIG_H)
#include "../../../config.h"
#endif

#include <SDL_syswm.h>
#include "clipboard.h"
#include "stringutils.h"

#include "../log.h"

#ifdef WIN32
bool getClipboardContents(std::string& text)
{
    bool ret = false;

    if (!OpenClipboard(NULL))
        return false;

    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (h)
    {
        LPCWSTR data = (LPCWSTR) GlobalLock(h);

        if (data)
        {
            const int len = WideCharToMultiByte(CP_UTF8, 0, data, -1, NULL, 0, NULL, NULL);
            if (len > 0)
            {
                // Convert from UTF-16 to UTF-8
                void *temp = malloc(len);

                if (WideCharToMultiByte(CP_UTF8, 0, data, -1, (LPSTR) temp, len, NULL, NULL))
                    text = strprintf("%s", (char*) temp);

                free(temp);
                ret = true;
            }
        }
        GlobalUnlock(h);
    }
    else
    {
        h = GetClipboardData(CF_TEXT);

        if (h)
        {
            const char *data = (char*) GlobalLock(h);

            if (data)
            {
                text = strprintf("%s", data);
                ret = true;
            }
            GlobalUnlock(h);
        }
    }

    CloseClipboard();
    return ret;
}
#elif defined(__APPLE__)

#ifdef Status
#undef Status
#endif

#include <Carbon/Carbon.h>

bool getClipboardContents(std::string& text)
{
    ScrapRef scrap;

    if (::GetCurrentScrap(&scrap) != noErr)
        return false;

    SInt32 byteCount = 0;
    OSStatus status = ::GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &byteCount);

    if (status != noErr)
        return false;

    char *buffer = new char[byteCount];
    bool ret = ::GetScrapFlavorData(scrap, kScrapFlavorTypeText, &byteCount, buffer) == noErr;

    if (ret)
        text = strprintf("%s", buffer);

    delete[] buffer;
    return ret;
}

#elif USE_X11
static char* getSelection(Display *dpy, Window us, Atom selection)
{
    int max_events = 50;
    Window owner = XGetSelectionOwner (dpy, selection);
    int ret;

    //logger->log("Clipboard: XConvertSelection on %s", XGetAtomName(dpy, selection));
    if (owner == None)
    {
        //logger->log("Clipboard: No owner");
        return NULL;
    }
    XConvertSelection(dpy, selection, XA_STRING, XA_PRIMARY, us, CurrentTime);
    XFlush(dpy);

    while (max_events--)
    {
        XEvent e;

        XNextEvent(dpy, &e);
        if (e.type == SelectionNotify)
        {
            //logger->log("Clipboard: Received %s", XGetAtomName(dpy, e.xselection.selection));
            if (e.xselection.property == None)
            {
                //logger->log("Clipboard: Couldn't convert");
                return NULL;
            }

            long unsigned len, left, dummy;
            int format;
            Atom type;
            unsigned char *data = NULL;

            XGetWindowProperty(dpy, us, e.xselection.property, 0, 0, False,
                               AnyPropertyType, &type, &format, &len, &left, &data);
            if (left < 1)
                return NULL;

            ret = XGetWindowProperty(dpy, us, e.xselection.property, 0, left, False,
                                     AnyPropertyType, &type, &format, &len, &dummy, &data);
            if (ret != Success)
            {
                //logger->log("Clipboard: Failed to get property: %p on %lu", data, len);
                return NULL;
            }

            //logger->log("Clipboard: Got %s: len=%lu left=%lu (event %i)", data, len, left, 50-max_events);
            return (char*) data;
        }
    }
    logger->log("Clipboard: Timeout");
    return NULL;
}

bool getClipboardContents(std::string& text)
{
    SDL_SysWMinfo info;

    //logger->log("Clipboard: Retrieving buffer...");
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) )
    {
        Display *dpy = info.info.x11.display;
        Window us = info.info.x11.window;
        char *data = NULL;

        if (!data)
            data = getSelection(dpy, us, XA_PRIMARY);

        if (!data)
            data = getSelection(dpy, us, XA_SECONDARY);

        if (!data)
        {
            Atom XA_CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", 0);
            data = getSelection(dpy, us, XA_CLIPBOARD);
        }

        if (data)
        {
            text = strprintf("%s", data);
            XFree(data);
        }
    }
    return false;
}
#else
bool getClipboardContents(std::string&)
{
    logger->log("Clipboard: No known clipboard!");
    return false;
}
#endif
