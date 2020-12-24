/**
 * Simple Command Line Interface (CLI) driver.
 * Features:
 *   - Single- and double-quoting
 *   - Command line history
 *   - Auto-complete
 *   - UTF-8 support
 *
 * URL: <https://github.com/erkia/cli>
 *
 * Copyright (C) 2016 Erki Aring
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLI_H_
#define CLI_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    const char  *name;
    const char  *help;
    int         (*callback)(int argc, char **argv);
} cli_cmd_t;

typedef struct {
    const cli_cmd_t *commands;
    size_t          count;
} cli_cmd_list_t;

void CLI_Init (const cli_cmd_list_t *cmdlist);
void CLI_Utf8 (uint8_t enable);
int CLI_Parse (const char *ptr, size_t len);
int CLI_HandleLine (void);
void CLI_HistoryClear (void);

#endif /* CLI_H_ */
