/*
 *  A chat recorder
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
 *
 *  This file is part of Aethyra.
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

#include <physfs.h>

#include "configuration.h"
#include "recorder.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "../eathena/gui/chat.h"

Recorder::Recorder(ChatWindow *chat) :
    mChat(chat)
{
}

Recorder::~Recorder()
{
    config.setValue(mChat->getWindowName() + "Record", mFileName);

    if (isRecording())
        changeRecordingStatus("");
}

void Recorder::record(const std::string &msg)
{
    if (isRecording())
        mStream << msg << std::endl;
}

void Recorder::changeRecordingStatus(const std::string &msg)
{
    mFileName = msg;
    trim(mFileName);

    if (mFileName.empty())
    {
        if (mStream.is_open())
        {
            mStream.close();

            /*
             * Message should go after mStream is closed so that it isn't
             * recorded.
             */
            mChat->chatLog(_("Finishing recording."), BY_SERVER);
        }
        else
            mChat->chatLog(_("Not currently recording."), BY_SERVER);
    }
    else if (mStream.is_open())
        mChat->chatLog(_("Already recording."), BY_SERVER);
    else
    {
        /*
         * Message should go before mStream is opened so that it isn't
         * recorded.
         */
        mChat->chatLog(_("Starting to record..."), BY_SERVER);

        std::stringstream file;
        file << PHYSFS_getUserDir();
#if (defined __USE_UNIX98 || defined __FreeBSD__)
        file << ".aethyra/";
#elif defined __APPLE__
        file << "Desktop/";
#endif
        file << mFileName;

        mStream.open(file.str().c_str(), std::ios_base::app);

        if (!mStream.is_open())
            mChat->chatLog(_("Failed to start recording."), BY_SERVER);
    }
}
