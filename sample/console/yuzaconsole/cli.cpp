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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <minwindef.h>
#include <dirent.h>
#include "cli.h"
#include <systemcall_impl.h>
#include <ConsoleManager.h>

#define CLI_LINE_BUFSIZE    32          /**< Maximum length of the command line that can be handled */
#define CLI_ARGV_LEN        11          /**< Maximum number of allowed arguments, including command name */
#define CLI_HISTORY         8           /**< Number of command lines to remember */
#define CLI_HIST_IGNORE_DUP (1 << 0)    /**< Flag to indicate, that duplicate entry should be discarded */
#define CLI_HIST_TEMPORARY  (1 << 1)    /**< Flag to indicate, that entry is temporary */
#define CLI_HIST_DIRTY      (1 << 2)    /**< Flag to indicate, that command line is modified */
#define CLI_HIST_CLEAR      (1 << 3)    /**< Flag to indicate, that history should be cleared */


#if (CLI_HISTORY > 0)
    #define CLI_HAS_HISTORY             /**< If defined, history functionality is included */
#endif

#define CLI_HAS_AUTOCOMPLETE            /**< If defined, auto-complete functionality is included */


/**
 * A structure containing information about the current command line.
 */
typedef struct {
    char    buf[CLI_LINE_BUFSIZE];
    size_t  len;
    uint8_t ready;
} cli_line_t;


/**
 * A structure containing information about the command line history.
 */
typedef struct {
    char        lines[CLI_HISTORY][CLI_LINE_BUFSIZE];
    size_t      pos;
    size_t      len;
    int         flags;
} cli_history_t;


/**
 * A structure containing the state of the driver
 */
typedef struct {
    cli_line_t      line;
#ifdef CLI_HAS_HISTORY
    cli_history_t   history;
#endif
    const cli_cmd_list_t  *cmdlist;
    uint8_t         utf8_enabled;
} cli_control_t;


static cli_control_t _CliCtrl;
static cli_control_t *CliCtrl = &_CliCtrl;


static void CLI_AddChar (char c);
static void CLI_DelChar (void);


/**
 * Write a buffer to the standard output.
 *
 * @param   buf     Buffer
 * @param   len     Length of the buffer
 *
 * @return Number of bytes written
 */
static int CLI_Write (const void *buf, size_t len)
{
    //return write (1, buf, len);

    if (len == 0)
        return 0;

    char temp[MAXPATH] = { 0, };
    memcpy(temp, buf, len);
       
    printf("%s", temp);
    return len;
}


/**
 * Write a string to the standard output.
 *
 * @param   str     Null-terminated string
 *
 * @return Number of bytes written
 */
static int CLI_WriteStr (const char *str)
{
    return CLI_Write (str, strlen (str));
}


/**
 * Write out command prompt and current command.
 */
static void CLI_Prompt (void)
{
    CLI_WriteStr("\r\n");
    char szDir[MAXPATH];
    Syscall_GetCurrentDirectory(MAXPATH, szDir);
    CLI_WriteStr(szDir);
    CLI_Write (CliCtrl->line.buf, CliCtrl->line.len);
}


/**
 * Erase the old command and replace it with a new one.
 *
 * @param   line    Command
 * @param   len     Length of the command
 */
static void CLI_ReplaceCmd (const char *line, size_t len)
{
    if (len < CLI_LINE_BUFSIZE) {

        // Clear the old command
        while (CliCtrl->line.len > 0){
            CLI_DelChar ();
        }

        // Load the command from history
        strncpy (CliCtrl->line.buf, line, len);
        CliCtrl->line.buf[len] = '\0';
        CliCtrl->line.len = len;

        // Write the new command
        CLI_Write (CliCtrl->line.buf, CliCtrl->line.len);

    }
}


#ifdef CLI_HAS_AUTOCOMPLETE

/**
 * Auto-complete a command or print out possible completions in case of ambiguity
 *
 * @param   tab_count   Number of times the tab is pressed consequently (counting from '0')
 */
static void CLI_AutoComplete (int tab_count)
{
    size_t i;
    int found = 0;

    if (CliCtrl->cmdlist != NULL) {

        // Find all matches
        for (i = 0; i < CliCtrl->cmdlist->count; i++) {
            if (!strncmp (CliCtrl->line.buf, CliCtrl->cmdlist->commands[i].name, CliCtrl->line.len)) {
                found++;
            }
        }


        if (found == 1) {
            // One match found - use it for auto-completion
            for (i = 0; i < CliCtrl->cmdlist->count; i++) {
                if (!strncmp (CliCtrl->line.buf, CliCtrl->cmdlist->commands[i].name, CliCtrl->line.len)) {
                    CLI_ReplaceCmd (CliCtrl->cmdlist->commands[i].name, strlen (CliCtrl->cmdlist->commands[i].name));
                    CLI_AddChar (' ');
                }
            }
        } else if (found > 1) {
            // Multiple matches found - print out all possible completions on every second tab-press
            if (tab_count % 2) {
                for (i = 0; i < CliCtrl->cmdlist->count; i++) {
                    if (!strncmp (CliCtrl->line.buf, CliCtrl->cmdlist->commands[i].name, CliCtrl->line.len)) {
                        CLI_WriteStr ("\r\n");
                        CLI_WriteStr (CliCtrl->cmdlist->commands[i].name);
                    }
                }
                CLI_Prompt ();
            }
        }
    }
}

static void CLI_AutoCompleteFileName(int tab_count)
{
    size_t i;
    int found = 0;

    struct dirent* pDirent;
    
    struct dirent* mapDirEntry[256];

    DIR* pDir;
    char szCurrentDir[MAXPATH];
    Syscall_GetCurrentDirectory(MAXPATH, szCurrentDir);

    pDir = opendir(szCurrentDir);
    if (pDir == NULL)
    {    
        return;
    }

    while ((pDirent = readdir(pDir)) != NULL)
    {
        if (!strncmp(CliCtrl->line.buf, pDirent->d_name, CliCtrl->line.len)) 
        {
            mapDirEntry[found] = (dirent*)malloc(sizeof(struct dirent));
            memcpy(mapDirEntry[found], pDirent, sizeof(struct dirent));
            found++;
            
        }
    }

    closedir(pDir);
   
    if (found == 1) 
    {                                   
        CLI_ReplaceCmd(mapDirEntry[0]->d_name, strlen(mapDirEntry[0]->d_name));               
        CLI_AddChar(' ');                    
    }
    else if (found > 1) {
        // Multiple matches found - print out all possible completions on every second tab-press
        if (tab_count % 2) {
            for (i = 0; i < found; i++) {
                if (!strncmp(CliCtrl->line.buf, mapDirEntry[i]->d_name, CliCtrl->line.len)) {
                    CLI_WriteStr("\r\n");
                    CLI_WriteStr(mapDirEntry[i]->d_name);
                }
            }
            CLI_Prompt();
        }
    }

    if (found > 0)
    {
        for (int i = 0; i < found; i++)
            free(mapDirEntry[i]);
    }

}

#endif


#ifdef CLI_HAS_HISTORY

/**
 * Set history related flags.
 *
 * @param   flags   Bitmask of flags to set
 */
static void CLI_HistorySetFlags (int flags)
{
    CliCtrl->history.flags |= flags;
}


/**
 * Clear history related flags.
 *
 * @param   flags   Bitmask of flags to clear
 */
static void CLI_HistoryClearFlags (int flags)
{
    CliCtrl->history.flags &= ~flags;
}


/**
 * Add a command to the history.
 *
 * @param   flags   Bitmask of flags
 */
static void CLI_HistoryAdd (int flags)
{
    size_t i;
    int last_non_temp;

    // Ignore duplicates
    if (flags & CLI_HIST_IGNORE_DUP) {
        last_non_temp = CliCtrl->history.len - 1;
        if (CliCtrl->history.flags & CLI_HIST_TEMPORARY) {
            last_non_temp--;
        }
        if (last_non_temp >= 0) {
            if (strncmp (CliCtrl->history.lines[last_non_temp], CliCtrl->line.buf, CLI_LINE_BUFSIZE) == 0) {
                if (CliCtrl->history.flags & CLI_HIST_TEMPORARY) {
                    CliCtrl->history.len--;
                    CLI_HistoryClearFlags (CLI_HIST_TEMPORARY);
                }
                CliCtrl->history.pos = CliCtrl->history.len;
                return;
            }
        }
    }

    // Find a place in history to add the new line
    if (CliCtrl->history.flags & CLI_HIST_TEMPORARY) {
        // Last entry is temporary, reuse it
        if (CliCtrl->history.len > 0) {
            CliCtrl->history.len--;
        }
    } else {
        // If history is full, shift and make room
        if (CliCtrl->history.len >= CLI_HISTORY) {
            for (i = 0; i < CLI_HISTORY - 1; i++) {
                strncpy (CliCtrl->history.lines[i], CliCtrl->history.lines[i + 1], CLI_LINE_BUFSIZE);
            }
            CliCtrl->history.len = CLI_HISTORY - 1;
        }
    }

    // Add current line to the last position in history
    strncpy (CliCtrl->history.lines[CliCtrl->history.len], CliCtrl->line.buf, CLI_LINE_BUFSIZE);
    CliCtrl->history.len++;
    CliCtrl->history.pos = CliCtrl->history.len;

    // Remember the 'temporary' flag
    if (flags & CLI_HIST_TEMPORARY) {
        CLI_HistorySetFlags (CLI_HIST_TEMPORARY);
    } else {
        CLI_HistoryClearFlags (CLI_HIST_TEMPORARY);
    }
}


/**
 * Fetch a command from the history and place it on the command line.
 *
 * @param   step    Direction to step in history
 */
static void CLI_HistoryFetch (int step)
{
    char *line;
    int pos;

    // If line buffer is modified, add it as a temporary entry to the history
    if (CliCtrl->history.flags & CLI_HIST_DIRTY) {
        if (CliCtrl->line.len < CLI_LINE_BUFSIZE) {
            CliCtrl->line.buf[CliCtrl->line.len] = '\0';
            CLI_HistoryAdd (CLI_HIST_TEMPORARY);
            CliCtrl->history.pos--;
        }
        CLI_HistoryClearFlags (CLI_HIST_DIRTY);
    }

    // Walk the history
    pos = CliCtrl->history.pos;
    pos += step;
    if (pos < 0) {
        pos = 0;
    } else if (pos > (CliCtrl->history.len - 1)) {
        pos = (CliCtrl->history.len - 1);
    }
    CliCtrl->history.pos = pos;

    // Load the command from history
    line = CliCtrl->history.lines[CliCtrl->history.pos];
    CLI_ReplaceCmd (line, strlen (line));
}


/**
 * Marks history to be cleared in the next call to CLI_Parse().
 */
void CLI_HistoryClear (void)
{
    CLI_HistorySetFlags (CLI_HIST_CLEAR);
}

#endif // CLI_HAS_HISTORY

static int CLI_Exec_Process(int argc, char** argv)
{
    if (argc == 0)
        return 0;

    char szArg[MAX_PATH] = { 0, };

    for (int i = 1; i < argc; i++)
    {
        strcat(szArg, argv[i]);
    }

    int handle = Syscall_CreateProcess(argv[0], szArg, 16);

    bool graphicMode = Syscall_IsGraphicMode();
    if (graphicMode == false)
    {
        if (handle != 0)
            Syscall_WaitForChildProcess(handle);
    }

    return handle > 0;
}

/**
 * Execute a command with given arguments.
 *
 * @param   argc    Number of arguments in argv
 * @param   argv    Arguments, first argument is command name
 *
 * @return Result returned by the command or '127' in case command is not found
 */

static int CLI_Exec (int argc, char **argv)
{
    size_t i;

    CLI_WriteStr ("\r\n");

    if (CliCtrl->cmdlist != NULL) {
        for (i = 0; i < CliCtrl->cmdlist->count; i++) {
            if (!strcmp (argv[0], CliCtrl->cmdlist->commands[i].name)) {
                return CliCtrl->cmdlist->commands[i].callback (argc, argv);
            }
        }
    }

    if (CLI_Exec_Process(argc, argv))
        return 1;

    //CLI_WriteStr ("CLI_Exec: ");
    CLI_WriteStr (argv[0]);
    CLI_WriteStr (": command not found");

    return 127;
}


/**
 * Handle special keys (command line sequences)
 *
 * @param   cmd     Command identifier
 */
static void CLI_HandleCSI (uint8_t cmd)
{
    switch (cmd) {
        case 'H':
            // UP
#ifdef CLI_HAS_HISTORY
            CLI_HistoryFetch (-1);
#endif
            break;
        case 'P':
            // DOWN
#ifdef CLI_HAS_HISTORY
            CLI_HistoryFetch (1);
#endif
            break;
        case 'C':
            // RIGHT
            break;
        case 'D':
            // LEFT
            break;
    }
}


/**
 * Add a character to the command line.
 *
 * @param   c       Character to add
 */
static void CLI_AddChar (char c)
{
    uint8_t bits;

    if (CliCtrl->line.len < CLI_LINE_BUFSIZE) {

        // Add the byte to the command buffer
        CliCtrl->line.buf[CliCtrl->line.len] = c;
        CliCtrl->line.len++;

#ifdef CLI_HAS_HISTORY
        CLI_HistorySetFlags (CLI_HIST_DIRTY);
#endif

    } else {

        // If it doesn't fit into buffer, then only count code points
        bits = (c & 0xC0);
        if (!CliCtrl->utf8_enabled || (bits != 0x80)) {
            CliCtrl->line.len++;
        }

    }

    CLI_Write (&c, 1);
}


/**
 * Delete a character from the command line.
 */
static void CLI_DelChar (void)
{
    uint8_t bits;

    if (CliCtrl->line.len > 0) {

        CLI_WriteStr ("\x08 \x08");

        // Remove the whole code point from the buffer -
        // no code point validation is done
        while (CliCtrl->line.len > 0) {
            CliCtrl->line.len--;
            if (CliCtrl->line.len < CLI_LINE_BUFSIZE) {
                bits = (CliCtrl->line.buf[CliCtrl->line.len] & 0xC0);
                if (!CliCtrl->utf8_enabled || (bits != 0x80)) {
                    break;
                }
            } else {
                break;
            }
        }

    }
}


/**
 * Parse incoming data and control the command line interface.
 *
 * @param   ptr     Buffer of incoming data
 * @param   len     Length of the buffer
 *
 * @return Number of bytes parsed before the end of the buffer or the end
 *         of the current command was reached
 */
int CLI_Parse (const char *ptr, size_t len)
{
    static uint8_t ignore_next_newline = 0;
    static uint8_t esc = 0;
    static uint8_t tab = 0;

    size_t i;

    // If CLI_Parse() is called before previous line is handled then drop it
    if (CliCtrl->line.ready) {
        CliCtrl->line.len = 0;
        CliCtrl->line.ready = 0;
    }

#ifdef CLI_HAS_HISTORY
    // If CLI_HIST_CLEAR is set, then clear the history now
    if (CliCtrl->history.flags & CLI_HIST_CLEAR) {
        memset (&(CliCtrl->history), 0, sizeof (cli_history_t));
        CLI_HistoryClearFlags (CLI_HIST_CLEAR);
    }
#endif

    for (i = 0; i < len; i++) {

        // ANSI escape code
        if (esc) {
            if (esc == 1) {
                //if (ptr[i] >= 64 && ptr[i] <= 95) 
                {
                    if (ptr[i] == '[') {
                        // Control Sequence Introducer (CSI)
                        esc = 2;
                    } else {
                        // Sequence complete
                        CLI_HandleCSI (ptr[i]);
                        esc = 0;
                    }
                    continue;
                }
            } else {
                if (ptr[i] >= 64 && ptr[i] <= 126) {
                    // Sequence complete
                    CLI_HandleCSI (ptr[i]);
                    esc = 0;
                    continue;
                }
            }
            esc = 0;
        }

        if (ptr[i] == '\r' || ptr[i] == '\n') {

            if (!ignore_next_newline || ptr[i] != '\n') {

                // Handle the received command
                if (CliCtrl->line.len) {
                    CliCtrl->line.ready = 1;
                    return (i + 1);
                } else {
                    CLI_Prompt ();
                }

            }

        } else if (ptr[i] == '\b' || ptr[i] == 127) {

            CLI_DelChar ();
#ifdef CLI_HAS_HISTORY
            CLI_HistorySetFlags (CLI_HIST_DIRTY);
#endif


        } else if (ptr[i] == '\t') {

#ifdef CLI_HAS_AUTOCOMPLETE
            //CLI_AutoComplete (tab);
            CLI_AutoCompleteFileName(tab);
#endif
            tab++;

        } //else if (ptr[i] == 27) (ptr[i] == 0xe0) 
        else if ((unsigned char)ptr[i] == (unsigned char)0xe0)
        {

            // ANSI ESC
            esc = 1;

        } else if (ptr[i] >= 32) {

            CLI_AddChar (ptr[i]);

        } else {

            // Unrecognizable character
            // printf ("%d\r\n", ptr[i]);

        }

        // Handle CRLF-s
        if (ptr[i] == '\r') {
            ignore_next_newline = 1;
        } else {
            ignore_next_newline = 0;
        }

        // Reset the TAB counter
        if (ptr[i] != '\t') {
            tab = 0;
        }
    }

    return len;
}


/**
 * Handle the last received command.
 *
 * @return The result of the executed command
 */
int CLI_HandleLine (void)
{
    size_t i;
    char c;
    int res = -1;

    if (CliCtrl->line.ready) {

        // Leave room for terminating '\0'
        if (CliCtrl->line.len < CLI_LINE_BUFSIZE) {

            size_t argstart = 0;
            size_t bufpos = 0;
            char *argv[CLI_ARGV_LEN];
            int argc = 0;
            uint8_t is_squotes = 0;
            uint8_t is_dquotes = 0;
            uint8_t is_escape = 0;

            CliCtrl->line.buf[CliCtrl->line.len] = '\0';

#ifdef CLI_HAS_HISTORY
            CLI_HistoryAdd (CLI_HIST_IGNORE_DUP);
            CLI_HistorySetFlags (CLI_HIST_DIRTY);
#endif

            for (i = 0; i < CliCtrl->line.len; i++) {

                c = CliCtrl->line.buf[i];

                if (is_escape) {
                    CliCtrl->line.buf[bufpos++] = c;
                    is_escape = 0;
                } else {
                    if (is_squotes) {
                        if (c == '\'') {
                            is_squotes = 0;
                        } else {
                            CliCtrl->line.buf[bufpos++] = c;
                        }
                    } else if (is_dquotes) {
                        if (c == '\"') {
                            is_dquotes = 0;
                        } else if (c == '\\') {
                            is_escape = 1;
                        } else {
                            CliCtrl->line.buf[bufpos++] = c;
                        }
                    } else {
                        if (c == '\\') {
                            is_escape = 1;
                        } else if (c == '\'') {
                            is_squotes = 1;
                        } else if (c == '\"') {
                            is_dquotes = 1;
                        } else if (c == ' ') {
                            if (bufpos - argstart > 0) {
                                CliCtrl->line.buf[bufpos++] = '\0';
                                if (argc < CLI_ARGV_LEN) {
                                    argv[argc] = &(CliCtrl->line.buf[argstart]);
                                }
                                argc++;
                            }
                            argstart = bufpos;
                        } else {
                            CliCtrl->line.buf[bufpos++] = c;
                        }
                    }
                }
            }

            if (is_escape || is_squotes || is_dquotes) {

                CLI_WriteStr ("\r\nCLI_HandleLine: command parse error");

            } else if ((argc > CLI_ARGV_LEN) || ((argc == CLI_ARGV_LEN) && (bufpos - argstart > 0))) {

                CLI_WriteStr ("\r\nCLI_HandleLine: too many arguments");

            } else {

                // Handle last argument
                if (bufpos - argstart > 0) {
                    CliCtrl->line.buf[bufpos++] = '\0';
                    argv[argc++] = &(CliCtrl->line.buf[argstart]);
                }

                if (argc > 0) {

                    // Set all unused arguments refer to empty string, to avoid accidental invalid dereferences
                    // by command handlers (in case they do not check the argument count)
                    char *empty_string = "";
                    for (i = argc; i < (sizeof (argv) / sizeof (char *)); i++) {
                        argv[i] = empty_string;
                    }

                    res = CLI_Exec (argc, argv);
                }
            }

        } else {

            CLI_WriteStr ("\r\nCLI_HandleLine: command too long");

        }

        CliCtrl->line.len = 0;
        CliCtrl->line.ready = 0;
        CLI_Prompt ();

    }

    return res;
}


/**
 * Enable/disable UTF-8 support (only affects backspace).
 *
 * @param   enable  0 - disable, 1 - enable
 */
void CLI_Utf8 (uint8_t enable)
{
    CliCtrl->utf8_enabled = enable;
}


/**
 * Initialize the driver.
 *
 * @param   cmdlist List of available commands and their handlers
 */
void CLI_Init (const cli_cmd_list_t *cmdlist)
{
    memset (CliCtrl, 0, sizeof (cli_control_t));
    CliCtrl->cmdlist = cmdlist;
    CLI_Utf8 (1);
    CLI_Prompt ();
}
