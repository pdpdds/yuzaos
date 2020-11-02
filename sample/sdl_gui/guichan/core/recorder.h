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

#ifndef RECORD_H
#define RECORD_H

#include <fstream>
#include <string>

class ChatWindow;

class Recorder
{
    public:
        Recorder(ChatWindow *chat);

        virtual ~Recorder();

        /*
         * Outputs the message to the recorder file
         *
         * @param msg the line to write to the recorded file.
         */
        void record(const std::string &msg);

        /*
         * Outputs the message to the recorder file
         *
         * @param msg The file to write out to. If null, then stop recording.
         */
        void changeRecordingStatus(const std::string &msg);

        /*
         * Whether or not the recorder is in use.
         */
        bool isRecording() { return (bool) mStream.is_open(); }

    private:
        ChatWindow *mChat;

        std::string mFileName;
        std::ofstream mStream;
};

#endif
