/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/yuza/yuza-iostream.h"

#include <stat_def.h>

YuzaIoStream *YuzaIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = fopen(path.c_str(), writeMode ? "wb" : "rb");

	if (handle)
		return new YuzaIoStream(handle);

	return nullptr;
}

YuzaIoStream::YuzaIoStream(void *handle) :
		StdioStream(handle) {
}

int32 YuzaIoStream::size() const {
	
	if (_handle == 0)
	{
		return StdioStream::size();
	}

	long curpos = ftell((FILE*)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	long filesize = ftell((FILE *)_handle);
	fseek((FILE *)_handle, curpos, SEEK_SET);

	return filesize;
}
