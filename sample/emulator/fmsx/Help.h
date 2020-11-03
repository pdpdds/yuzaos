/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                          Help.h                         **/
/**                                                         **/
/** This file contains help information printed out by the  **/
/** main() routine when started with option "-help".        **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

static const char *HelpText[] =
{
  "Usage: fmsx [-option1 [-option2...]] [filename1] [filename2]",
  "[filename1] = name of file to load as cartridge A",
  "[filename2] = name of file to load as cartridge B",
#ifdef ZLIB
  "  This program will transparently uncompress singular GZIPped",
  "  and PKZIPped files.",
#endif
  "[-option]   =",
  "  -verbose <level>    - Select debugging messages [1]",
  "                        0 - Silent       1 - Startup messages",
  "                        2 - V9938 ops    4 - Disk/Tape",
  "                        8 - Memory      16 - Illegal Z80 ops", 
  "  -hperiod <period>   - Minimal number of CPU cycles per HBlank [228]",
  "  -vperiod <period>   - Minimal number of CPU cycles per VBlank [59736]",
  "  -uperiod <period>   - Number of VBlanks per screen update [2]",
  "  -pal/-ntsc          - Set PAL/NTSC HBlank/VBlank periods [NTSC]",
  "  -help               - Print this help page",
  "  -home <dirname>     - Set directory with system ROM files [off]",
  "  -printer <filename> - Redirect printer output to file [stdout]",
  "  -serial <filename>  - Redirect serial I/O to a file [stdin/stdout]",
  "  -diska <filename>   - Set disk image used for drive A: [DRIVEA.DSK]",
  "                        (multiple -diska options accepted)",
  "  -diskb <filename>   - Set disk image used for drive B: [DRIVEB.DSK]",
  "                        (multiple -diskb options accepted)",
  "  -tape <filename>    - Set tape image file [off]",
  "  -font <filename>    - Set fixed font for text modes [DEFAULT.FNT]",
  "  -logsnd <filename>  - Set soundtrack log file [LOG.MID]",
  "  -state <filename>   - Set emulation state save file [automatic]",
  "  -auto/-noauto       - Use autofire on SPACE [off]",
  "  -ram <pages>        - Number of 16kB RAM pages [4/8/8]",
  "  -vram <pages>       - Number of 16kB VRAM pages [2/8/8]",
  "  -rom <type>         - Select MegaROM mapper types [6,6]",
  "                        (two -rom options accepted)",
  "                        0 - Generic 8kB   1 - Generic 16kB (MSXDOS2)",
  "                        2 - Konami5 8kB   3 - Konami4 8kB",
  "                        4 - ASCII 8kB     5 - ASCII 16kB",
  "                        6 - GameMaster2",
  "                        >6 - Try guessing mapper type",
  "  -msx1/-msx2/-msx2+  - Select MSX model [-msx2]",
  "  -joy <type>         - Select joystick types [0,0]",
  "                        (two -joy options accepted)",
  "                        0 - No joystick",
  "                        1 - Normal joystick",
  "                        2 - Mouse in joystick mode",
  "                        3 - Mouse in real mode",

#ifdef DEBUG
  "  -trap <address>     - Trap execution when PC reaches address [FFFFh]",
  "                        (when keyword 'now' is used in place of the",
  "                        <address>, execution will trap immediately)",
#endif /* DEBUG */

#ifdef SOUND
#ifdef MSDOS
  "  -sound [<quality>]  - Sound emulation quality [1]",
#else
  "  -sound [<quality>]  - Sound emulation quality [44100]",
#endif
  "                        0 - Off                1 - Adlib (MSDOS)",
  "                        Values >8191 are treated as wave synthesis",
  "                        frequencies.",
  "  -nosound            - Same as '-sound 0'",
#endif /* SOUND */

#ifdef UNIX
#ifdef MITSHM
  "  -shm/-noshm         - Use MIT SHM extensions for X [-shm]",
#endif
  "  -sync <frequency>   - Sync screen updates to <frequency> [-nosync]",
  "  -nosync             - Do not sync screen updates",
  "  -static/-nostatic   - Use static color palette [-nostatic]",
  "  -saver/-nosaver     - Save CPU when inactive [-saver]",
  "  -scale <factor>     - Scale window by <factor> [1]",
#endif /* UNIX */

#ifdef MSDOS
  "  -vsync              - Sync screen updates to VBlank [-nosync]",
  "  -sync <frequency>   - Sync screen updates to <frequency> [-nosync]",
  "                        (<frequency> must be in 20Hz..100Hz range)",
  "  -nosync             - Do not sync screen updates",
  "  -static/-nostatic   - Use static color palette [-nostatic]",
  "  -240/-200           - Use non-standard 256x240 mode [-200]",
#endif /* MSDOS */

  "\nKeyboard bindings:",

#ifdef UNIX
  "  [LALT]          - Joystick A button",
  "  [LCONTROL]      - Joystick B button",
  "  [RSHIFT]        - CAPS LOCK",
#endif /* UNIX */
  "  [ALT]           - GRAPH",
  "  [INSERT]        - INSERT",
  "  [DELETE]        - DELETE",
  "  [HOME]          - HOME/CLS",
  "  [END]           - SELECT",
  "  [PGUP]          - STOP/BREAK",
  "  [PGDOWN]        - COUNTRY",
  "  [F6]            - Turn spacebar autofire on/off",
  "  [F7]            - Load emulation from .STA file",
  "  [CONTROL]+[F7]  - Save emulation state to .STA file",
  "  [F8]            - Change disk in drive A:",
  "  [CONTROL]+[F8]  - Change disk in drive B:",
  "  [F9]            - Turn fixed font on/off",
#ifdef SOUND
  "  [F10]           - Turn sound on/off",
#endif
  "  [CONTROL]+[F10] - Turn soundtrack logging on/off",
#ifdef DEBUG
  "  [F11]           - Go into the built-in debugger",
#endif
#ifdef GIFLIB
  "  [CONTROL]+[F11] - Make a screen snapshot (SNAPxxxx.GIF)",
#endif
  "  [F12]           - Quit emulation",
  0
};
