/* MollenOS
*
* Copyright 2011 - 2016, Philip Meulengracht
*
* This program is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation ? , either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*
*
* MollenOS C Library - Retrieve timezone information
*/

/* Includes */
#include <orangeos.h>
#include "_time.h"
#include "local.h"

/* Shared timezone information for libc/time functions.  */
__tzinfo_type tzinfo = {1, 0,
	{ {'J', 0, 0, 0, 0, (time_t)0, 0L },
	  {'J', 0, 0, 0, 0, (time_t)0, 0L }
    } 
};

__tzinfo_type *__gettzinfo(void) 
{
  return &tzinfo;
}
