#include "cmd.h"
const cmdmsg_t g_cmdmsg[] = {
{0x00002328L, "MSG_RESPONSE_DATA", "Y N"}
, {0x00002329L, "MSG_BAD_PARM1", "An incorrect parameter was\nentered for the command."}
, {0x0000232aL, "MSG_BAD_SYNTAX", "The syntax of the command is incorrect."}
, {0x0000232bL, "MSG_STRIKE_ANY_KEY", "Press any key to continue . . . %0"}
, {0x0000232cL, "MSG_CMD_DELETE", "%1, Delete (Y/N)? %0"}
, {0x0000232dL, "MSG_COM_SEARCH_DIR_BAD", "The system cannot find the\ncommand processor in the path specified."}
, {0x0000232eL, "MSG_REN_INVAL_PATH_FILENAME", "The system cannot accept the path\nor file name requested."}
, {0x0000232fL, "MSG_INVALID_DATE", "The system cannot accept the date entered."}
, {0x00002330L, "MSG_NO_BAT_LABEL", "No batch label specified to GOTO command."}
, {0x00002331L, "MSG_DIR_BAD_COMMAND_OR_FILE", "The name specified is not recognized as an\ninternal or external command, operable program or batch file."}
, {0x00002332L, "MSG_REN_INVALID_TIME", "The system cannot accept the time entered."}
, {0x00002333L, "MSG_BAD_DOS_VER", "The application program is not compatible\nwith the version of the operating system being used."}
, {0x00002334L, "MSG_COMM_VER", "Microsoft(R) Windows %1(TM)\n(C) Copyright 1985-1996 Microsoft Corp."}
, {0x00002335L, "MSG_C", "^C"}
, {0x00002336L, "MSG_FILES_COPIED", "%1 file(s) copied."}
, {0x00002337L, "MSG_CURRENT_DATE", "The current date is: %0"}
, {0x00002338L, "MSG_CURRENT_TIME", "The current time is: %0"}
, {0x00002339L, "MSG_DIR_OF", " Directory of %1\n"}
, {0x0000233aL, "MSG_OUT_OF_ENVIRON_SPACE", "The system is out of environment space."}
, {0x0000233bL, "MSG_INVALID_DOS_FILENAME", " The file name entered is not valid in DOS mode."}
, {0x0000233cL, "MSG_EXEC_FAILURE", "The system cannot execute the specified program."}
, {0x0000233dL, "MSG_FILES_FREE", "     %1 File(s)   %2 bytes free"}
, {0x0000233eL, "MSG_FILES_ONLY", "     %1 File(s)."}
, {0x0000233fL, "MSG_LINES_TOO_LONG", "The input line is too long."}
, {0x00002340L, "MSG_CONT_LOST_BEF_COPY", "The contents of the target file\nwere lost."}
, {0x00002341L, "MSG_INSRT_DISK_BAT", "Insert the diskette that contains the batch file\nand press any key when ready. %0"}
, {0x00002342L, "MSG_ENTER_NEW_DATE", "Enter the new date: (mm-dd-yy) %0"}
, {0x00002343L, "MSG_ENTER_NEW_TIME", "Enter the new time: %0"}
, {0x00002344L, "MSG_RDR_HNDL_CREATE", "The handle could not be duplicated\nduring redirection of handle %1."}
, {0x00002345L, "MSG_ECHO_OFF", "ECHO is off."}
, {0x00002346L, "MSG_ECHO_ON", "ECHO is on."}
, {0x00002347L, "MSG_VERIFY_OFF", "VERIFY is off."}
, {0x00002348L, "MSG_VERIFY_ON", "VERIFY is on."}
, {0x00002349L, "MSG_CANNOT_COPIED_ONTO_SELF", "The file cannot be copied onto itself."}
, {0x0000234aL, "MSG_SYNERR_GENL", "%1 was unexpected at this time."}
, {0x0000234bL, "MSG_TOP_LEVEL_PROCESS_CAN", "CMD.EXE has halted. %0"}
, {0x0000234cL, "MSG_PID_IS", "The Process Identification Number is %1."}
, {0x0000234dL, "MSG_DUP_FILENAME_OR_NOT_FD", "A duplicate file name exists, or the file\ncannot be found."}
, {0x0000234eL, "MSG_ARE_YOU_SURE", "%1, Are you sure (Y/N)? %0"}
, {0x0000234fL, "MSG_TOKEN_TOO_LONG", "The following character string is too long:\n%1"}
, {0x00002350L, "MSG_MS_DOS_VERSION", "Windows %1 Version %2.%3 %0"}
, {0x00002351L, "MSG_PIPE_FAILURE", "The handle could not be duplicated during\na pipe operation."}
, {0x00002352L, "MSG_MS_MORE", "More? %0"}
, {0x00002353L, "MSG_REAL_MODE_ONLY", "The system cannot complete the process."}
, {0x00002354L, "MSG_TYPE_FILENAME", "\n%1\n\n"}
, {0x00002355L, "MSG_INVALID_INPUT", "Unable to read from input."}
, {0x0000235bL, "MSG_DR_VOL_SERIAL", " Volume Serial Number is %1"}
, {0x0000235cL, "MSG_DIR_EXISTS", "A subdirectory or file %1 already exists."}
, {0x0000235dL, "MSG_ERR_PROC_ARG", "Error occurred while processing: %1."}
, {0x0000235eL, "MSG_HAS_NO_LABEL", " Volume in drive %1 has no label."}
, {0x0000235fL, "MSG_DR_VOL_LABEL", " Volume in drive %1 is %2"}
, {0x00002360L, "MSG_KEYS_ON", "KEYS is on."}
, {0x00002361L, "MSG_KEYS_OFF", "KEYS is off."}
, {0x00002362L, "MSG_START_INVALID_PARAMETER", "The system cannot accept the START command parameter %1."}
, {0x00002363L, "MSG_CMD_FILE_NOT_FOUND", "The system cannot find the file %1."}
, {0x00002364L, "MSG_CMD_INVAL_PIPE", "The process tried to write to a nonexistent pipe."}
, {0x00002365L, "MSG_CMD_CANT_FRGRND", "The system cannot start the session in the foreground."}
, {0x00002366L, "MSG_CMD_SYS_ERR", "\nA non-recoverable error occurred.\nThe process ended."}
, {0x00002367L, "MSG_CMD_DIV_0", "\nThe system detected a divide by zero error."}
, {0x00002368L, "MSG_CMD_SOFT_ERR", "\nThe process has stopped.  The software\ndiagnostic code (trap number) is %1."}
, {0x00002369L, "MSG_CMD_COPROC", "\nThe process ended when it tried to use\na non-existent math processor."}
, {0x0000236aL, "MSG_CMD_KILLED", "The external process was canceled by a Ctrl+Break or another process."}
, {0x0000236bL, "MSG_CMD_NOT_RECOGNIZED", "\"%1\" is not a recognized device."}
, {0x0000236cL, "MSG_CMD_BATCH_FILE_MISSING", "The batch file cannot be found."}
, {0x0000236eL, "MSG_FILES_MOVED", "%1 file(s) moved."}
, {0x0000236fL, "MSG_TRAPC", "A program attempted to reference storage outside the\nlimits of a stack segment.  The program was ended.\n\n%1"}
, {0x00002370L, "MSG_UNEXPEC_ERROR_ENC", "Error %1 encountered an unexpected end of file\non the redirected input. The process has stopped."}
, {0x00002371L, "MSG_LITERAL_TEXT", "%1%0"}
, {0x00002372L, "MSG_NOT_IMPLEMENTED", "Command not implemented."}
, {0x00002373L, "MSG_DIR", "<DIR>       %0"}
, {0x00002374L, "MSG_NO_MEMORY", "Out of memory."}
, {0x00002375L, "MSG_INVALID_SWITCH", "Invalid switch - \"%1\"."}
, {0x00002376L, "MSG_PARAMETER_FORMAT_NOT_CORRECT", "Parameter format not correct - \"%1\"."}
, {0x00002377L, "MSG_ERROR_IN_DIRCMD", "(Error occurred in environment variable)"}
, {0x00002378L, "MSG_FILES_COUNT_FREE", "     %1 File(s) %2 bytes"}
, {0x00002379L, "MSG_FILES_TOTAL_FREE", "                      %1 bytes free"}
, {0x0000237aL, "MSG_FILE_TOTAL", "     Total Files Listed:"}
, {0x0000237bL, "MSG_BATCH_TERM", "Terminate batch job (Y/N)? %0"}
, {0x0000237cL, "MSG_BAD_CURDIR", "The current directory is invalid."}
, {0x0000237dL, "MSG_HELP_CHDIR", "Displays the name of or changes the current directory.\n\nCHDIR [/D] [drive:][path]\nCHDIR [..]\nCD [/D] [drive:][path]\nCD [..]\n\n  ..   Specifies that you want to change to the parent directory.\n\nType CD drive: to display the current directory in the specified drive.\nType CD without parameters to display the current drive and directory.\n\nUse the /D switch to change current drive in addition to changing current\ndirectory for a drive."}
, {0x0000237eL, "MSG_HELP_CLS", "Clears the screen.\n\nCLS"}
, {0x0000237fL, "MSG_HELP_COPY", "Copies one or more files to another location.\n\nCOPY [/A | /B] source [/A | /B] [+ source [/A | /B] [+ ...]] [destination\n  [/A | /B]] [/V] [/N]\n\n  source       Specifies the file or files to be copied.\n  /A           Indicates an ASCII text file.\n  /B           Indicates a binary file.\n  destination  Specifies the directory and/or filename for the new file(s).\n  /V           Verifies that new files are written correctly.\n  /N           Uses short filename, if available, when copying a file with a\n               non-8dot3 name.\n  /Z           Copies networked files in restartable mode.\n\nTo append files, specify a single file for destination, but multiple files\nfor source (using wildcards or file1+file2+file3 format)."}
, {0x00002380L, "MSG_HELP_DATE", "Displays or sets the date.\n\nDATE [date]\n\nType DATE without parameters to display the current date setting and\na prompt for a new one.  Press ENTER to keep the same date."}
, {0x00002381L, "MSG_HELP_DEL_ERASE", "Deletes one or more files.\n\nDEL [/P] [/F] [/S] [/Q] [/A[[:]attributes]] [[drive:][path]filename\nERASE [/P] [/F] [/S] [/Q] [/A[[:]attributes]] [[drive:][path]filename\n\n  [drive:][path]filename\n                Specifies the file(s) to delete.  Specify multiple\n                files by using wildcards.\n  /P            Prompts for confirmation before deleting each file.\n  /F            Force deleting of read-only files.\n  /S            Delete specified files from all subdirectories.\n  /Q            Quiet mode, do not ask if ok to delete on global wildcard\n  /A            Selects files to delete based on attributes\n  attributes    R  Read-only files            S  System files\n                H  Hidden files               A  Files ready for archiving\n                -  Prefix meaning not"}
, {0x00002382L, "MSG_HELP_DIR", "Displays a list of files and subdirectories in a directory.\n\nDIR [drive:][path][filename] [/P] [/W] [/D] [/A[[:]attributes]]\n  [/O[[:]sortorder]] [/T[[:]timefield]] [/S] [/B] [/L] [/N] [/X] [/C]\n\n  [drive:][path][filename]\n              Specifies drive, directory, and/or files to list.\n\n  /P          Pauses after each screenful of information.\n  /W          Uses wide list format.\n  /D          Same as wide but files are list sorted by column.\n  /A          Displays files with specified attributes.\n  attributes   D  Directories                R  Read-only files\n               H  Hidden files               A  Files ready for archiving\n               S  System files               -  Prefix meaning not\n  /O          List by files in sorted order.\n  sortorder    N  By name (alphabetic)       S  By size (smallest first)\n               E  By extension (alphabetic)  D  By date & time (earliest first)\n               G  Group directories first    -  Prefix to reverse order\n  /T          Controls which time field displayed or used for sorting\n  timefield   C  Creation\n              A  Last Access\n              W  Last Written\n  /S          Displays files in specified directory and all subdirectories.\n  /B          Uses bare format (no heading information or summary).\n  /L          Uses lowercase.\n  /N          New long list format where filenames are on the far right.\n  /X          This displays the short names generated for non-8dot3 file\n              names.  The format is that of /N with the short name inserted\n              before the long name. If no short name is present, blanks are\n              displayed in its place.\n  /C          Display the thousand separator in file sizes.  This is the\n              default.  Use /-C to disable display of separator.\n\nSwitches may be preset in the DIRCMD environment variable.  Override\npreset switches by prefixing any switch with - (hyphen)--for example, /-W."}
, {0x00002383L, "MSG_HELP_EXIT", "Quits the CMD.EXE program (command interpreter).\n\nEXIT"}
, {0x00002384L, "MSG_HELP_MKDIR", "Creates a directory.\n\nMKDIR [drive:]path\nMD [drive:]path"}
, {0x00002385L, "MSG_HELP_PATH", "Displays or sets a search path for executable files.\n\nPATH [[drive:]path[;...][;%%PATH%%]\nPATH ;\n\nType PATH ; to clear all search-path settings and direct cmd.exe to search\nonly in the current directory.\nType PATH without parameters to display the current path.\nIncluding %%PATH%% in the new path setting causes the old path to be appended\nto the new setting."}
, {0x00002386L, "MSG_HELP_PROMPT", "Changes the cmd.exe command prompt.\n\nPROMPT [text]\n\n  text    Specifies a new command prompt.\n\nPrompt can be made up of normal characters and the following special codes:\n\n  $A   & (Ampersand)\n  $B   | (pipe)\n  $C   ( (Left parenthesis)\n  $D   Current date\n  $E   Escape code (ASCII code 27)\n  $F   ) (Right parenthesis)\n  $G   > (greater-than sign)\n  $H   Backspace (erases previous character)\n  $L   < (less-than sign)\n  $N   Current drive\n  $P   Current drive and path\n  $Q   = (equal sign)\n  $S     (space)\n  $T   Current time\n  $V   Windows NT version number\n  $_   Carriage return and linefeed\n  $$   $ (dollar sign)"}
, {0x00002387L, "MSG_HELP_RMDIR", "Removes (deletes) a directory.\n\nRMDIR [/S] [/Q] [drive:]path\nRD [/S] [/Q] [drive:]path\n\n    /S      Removes all directories and files in the specified directory\n            in addition to the directory itself.  Used to remove a directory\n            tree.\n\n    /Q      Quiet mode, do not ask if ok to remove a directory tree with /S"}
, {0x00002388L, "MSG_HELP_RENAME", "Renames a file or files.\n\nRENAME [drive:][path]filename1 filename2.\nREN [drive:][path]filename1 filename2.\n\nNote that you cannot specify a new drive or path for your destination file."}
, {0x00002389L, "MSG_HELP_SET", "Displays, sets, or removes cmd.exe environment variables.\n\nSET [variable=[string]]\n\n  variable  Specifies the environment-variable name.\n  string    Specifies a series of characters to assign to the variable.\n\nType SET without parameters to display the current environment variables."}
, {0x0000238aL, "MSG_HELP_TIME", "Displays or sets the system time.\n\nTIME [time]\n\nType TIME with no parameters to display the current time setting and a prompt\nfor a new one.  Press ENTER to keep the same time."}
, {0x0000238bL, "MSG_HELP_TYPE", "Displays the contents of a text file or files.\n\nTYPE [drive:][path]filename"}
, {0x0000238cL, "MSG_HELP_VER", "Displays the Windows NT version.\n\nVER"}
, {0x0000238dL, "MSG_HELP_VERIFY", "Tells cmd.exe whether to verify that your files are written correctly to a\ndisk.\n\nVERIFY [ON | OFF]\n\nType VERIFY without a parameter to display the current VERIFY setting."}
, {0x0000238eL, "MSG_HELP_VOL", "Displays the disk volume label and serial number, if they exist.\n\nVOL [drive:]"}
, {0x0000238fL, "MSG_HELP_CALL", "Calls one batch program from another.\n\nCALL [drive:][path]filename [batch-parameters]\n\n  batch-parameters   Specifies any command-line information required by the\n                     batch program."}
, {0x00002390L, "MSG_HELP_REM", "Records comments (remarks) in a batch file or CONFIG.SYS.\n\nREM [comment]"}
, {0x00002391L, "MSG_HELP_PAUSE", "Suspends processing of a batch program and displays the message\n    Press any key to continue . . . %0\n\nPAUSE"}
, {0x00002392L, "MSG_HELP_ECHO", "Displays messages, or turns command-echoing on or off.\n\n  ECHO [ON | OFF]\n  ECHO [message]\n\nType ECHO without parameters to display the current echo setting."}
, {0x00002393L, "MSG_HELP_GOTO", "Directs cmd.exe to a labeled line in a batch program.\n\nGOTO label\n\n  label   Specifies a text string used in the batch program as a label.\n\nYou type a label on a line by itself, beginning with a colon."}
, {0x00002394L, "MSG_HELP_SHIFT", "Changes the position of replaceable parameters in a batch file.\n\nSHIFT [/n]"}
, {0x00002395L, "MSG_HELP_IF", "Performs conditional processing in batch programs.\n\nIF [NOT] ERRORLEVEL number command\nIF [NOT] string1==string2 command\nIF [NOT] EXIST filename command\n\n  NOT               Specifies that Windows NT should carry out the command only\n                    if the condition is false.\n  ERRORLEVEL number Specifies a true condition if the last program run returned\n                    an exit code equal to or greater than the number specified.\n  command           Specifies the command to carry out if the condition is\n                    met.\n  string1==string2  Specifies a true condition if the specified text strings\n                    match.\n  EXIST filename    Specifies a true condition if the specified filename\n                    exists."}
, {0x00002396L, "MSG_HELP_FOR", "Runs a specified command for each file in a set of files.\n\nFOR %%variable IN (set) DO command [command-parameters]\n\n  %%variable Specifies a replaceable parameter.\n  (set)      Specifies a set of one or more files.  Wildcards may be used.\n  command    Specifies the command to carry out for each file.\n  command-parameters\n             Specifies parameters or switches for the specified command.\n\nTo use the FOR command in a batch program, specify %%%%variable instead\nof %%variable Variable names are case sensitive, so %%i is different\nfrom %%I."}
, {0x00002397L, "MSG_HELP_START", "Starts a separate window to run a specified program or command.\n\nSTART [\"title\"] [/Dpath] [/I] [/MIN] [/MAX] [/SEPARATE | /SHARED]\n      [/LOW | /NORMAL | /HIGH | /REALTIME] [/WAIT] [/B] [command/program] [parameters]\n\n    \"title\"     Title to display in  window title bar.\n    path        Starting directory\n    I           The new environment will be the original environment passed\n                to the cmd.exe and not the current environment.\n    MIN         Start window minimized\n    MAX         Start window maximized\n    SEPARATE    Start 16-bit Windows program in separate memory space\n    SHARED      Start 16-bit Windows program in shared memory space\n    LOW         Start application in the IDLE priority class\n    NORMAL      Start application in the NORMAL priority class\n    HIGH        Start application in the HIGH priority class\n    REALTIME    Start application in the REALTIME priority class\n    WAIT        Start application and wait for it to terminate\n    B           Start application without creating a new window. The\n                application has ^C handling ignored. Unless the application\n                enables ^C processing, ^Break is the only way to interrupt the\n                application\n    command/program\n                If it is an internal cmd command or a batch file then\n                the command processor is run with the /K switch to cmd.exe.\n                This means that the window will remain after the command\n                has been run.\n\n                If it is not an internal cmd command or batch file then\n                it is a program and will run as either a windowed application\n                or a console application.\n\n    parameters  These are the parameters passed to the command/program\n"}
, {0x00002398L, "MSG_HELP_BREAK", "Sets or Clears Extended CTRL+C checking on DOS system\n\nThis is present for Compatibility with DOS systems. It has no effect\nunder Windows NT."}
, {0x00002399L, "MSG_HELP_CMD", "Starts a new instance of the Windows/NT command interpreter\n\nCMD [/X | /Y] [/A | /U] [/Q] [[/C | /K] string]\n\n/C      Carries out the command specified by string and then terminates\n/K      Carries out the command specified by string but remains\n/Q      Turns the echo off\n/A      Causes the output of internal commands to a pipe or file to be ANSI\n/U      Causes the output of internal commands to a pipe or file to be Unicode\n/T:fg   Sets the foreground/background colors (see COLOR /? for more info)\n/X      Enable extensions to the Windows NT version of CMD.EXE\n/Y      Disable extensions to the Windows NT version of CMD.EXE\n\nNote that multiple commands separated by the command separator '&&'\nare accepted for string if surrounded by quotes\n"}
, {0x00002710L, "MSG_TOO_MANY_PARAMETERS", "Too many parameters - %1"}
, {0x00002714L, "MSG_HELP_SETLOCAL", "Begins localization of environment changes in a batch file.\nEnvironment changes made after SETLOCAL has been issued are\nlocal to the batch file.  ENDLOCAL must be issued to\nrestore the previous settings.\n\nSETLOCAL"}
, {0x00002715L, "MSG_HELP_ENDLOCAL", "Ends localization of environment changes in a batch file.\nEnvironment changes made after ENDLOCAL has been issued are\nnot local to the batch file; the previous settings are not\nrestored on termination of the batch file.\n\nENDLOCAL"}
, {0x00002716L, "MSG_HELP_TITLE", "Sets the window title for the command prompt window.\n\nTITLE [string]\n\n  string       Specifies the title for the command prompt window."}
, {0x00002717L, "MSG_HELP_APPEND", "Allows programs to open data files in specified directories as if they were in\nthe current directory.\n\nAPPEND [[drive:]path[;...]] [/X[:ON | :OFF]] [/PATH:ON | /PATH:OFF] [/E]\nAPPEND ;\n\n  [drive:]path Specifies a drive and directory to append.\n  /X:ON        Applies appended directories to file searches and\n               application execution.\n  /X:OFF       Applies appended directories only to requests to open files.\n               /X:OFF is the default setting.\n  /PATH:ON     Applies the appended directories to file requests that already\n               specify a path.  /PATH:ON is the default setting.\n  /PATH:OFF    Turns off the effect of /PATH:ON.\n  /E           Stores a copy of the appended directory list in an environment\n               variable named APPEND.  /E may be used only the first time\n               you use APPEND after starting up your system.\n\nType APPEND ; to clear the appended directory list.\nType APPEND without parameters to display the appended directory list."}
, {0x00002718L, "MSG_HELP_MOVE", "Moves one or more files from one directory to another directory.\n\nMOVE [Source] [Target]\n\n  source       Specifies the path and name of the file(s) to move.\n  target       Specifies the path and name to move file(s) to."}
, {0x00002719L, "MSG_HELP_PUSHDIR", "Stores the current directory for use by the POPD command, then\nchanges to the specified directory.\n\nPUSHD [path | ..]\n\n  path        Specifies the directory to make the current directory."}
, {0x0000271aL, "MSG_HELP_POPDIR", "Changes to the directory stored by the PUSHD command.\n\nPOPD\n"}
, {0x0000271bL, "MSG_FILE_NAME_PRECEEDING_ERROR", "%1 - %0"}
, {0x0000271cL, "MSG_MAX_SETLOCAL", "Maximum setlocal recursion level reached."}
, {0x0000271dL, "MSG_ENTER_JAPAN_DATE", "Enter the new date: (yy-mm-dd) %0"}
, {0x0000271eL, "MSG_ENTER_DEF_DATE", "Enter the new date: (dd-mm-yy) %0"}
, {0x00002721L, "MSG_RDR_HNDL_OPEN", "The handle could not be opened\nduring redirection of handle %1."}
, {0x00002722L, "MSG_PROGRESS", "%r%1%% copied %0"}
, {0x00002723L, "MSG_HELP_COLOR", "Sets the default console foreground and background colors.\n\nCOLOR [attr]\n\n  attr        Specifies color attribute of console output\n\nColor attributes are specified by TWO hex digits -- the first\ncorresponds to the background; the second the foreground.  Each digit\ncan be any of the following values:\n\n    0 = Black       8 = Gray\n    1 = Blue        9 = Light Blue\n    2 = Green       A = Light Green\n    3 = Aqua        B = Light Aqua\n    4 = Red         C = Light Red\n    5 = Purple      D = Light Purple\n    6 = Yellow      E = Light Yellow\n    7 = White       F = Bright White\n\nIf no argument is given, this command restores the color to what it was\nwhen CMD.EXE started.  This value either comes from the current console\nwindow, the /T command line switch or from the DefaultColor registry\nvalue.\n\nThe COLOR command sets ERRORLEVEL to 1 if an attempt is made to execute\nthe COLOR command with a foreground and background color that are the\nsame.\n\nExample: \"COLOR fc\" produces light red on bright white"}
, {0x00002724L, "MSG_MAX_PATH_EXCEEDED", "Maximum path length exceeded - %1"}
, {0x00000000L, "ERROR_SUCCESS", "The operation completed successfully."}
, {0x00000001L, "ERROR_INVALID_FUNCTION                   ;// dderror", "Incorrect function."}
, {0x00000002L, "ERROR_FILE_NOT_FOUND", "The system cannot find the file specified."}
, {0x00000003L, "ERROR_PATH_NOT_FOUND", "The system cannot find the path specified."}
, {0x00000004L, "ERROR_TOO_MANY_OPEN_FILES", "The system cannot open the file."}
, {0x00000005L, "ERROR_ACCESS_DENIED", "Access is denied."}
, {0x00000006L, "ERROR_INVALID_HANDLE", "The handle is invalid."}
, {0x00000007L, "ERROR_ARENA_TRASHED", "The storage control blocks were destroyed."}
, {0x00000008L, "ERROR_NOT_ENOUGH_MEMORY                  ;// dderror", "Not enough storage is available to process this command."}
, {0x00000009L, "ERROR_INVALID_BLOCK", "The storage control block address is invalid."}
, {0x0000000aL, "ERROR_BAD_ENVIRONMENT", "The environment is incorrect."}
, {0x0000000bL, "ERROR_BAD_FORMAT", "An attempt was made to load a program with an\nincorrect format."}
, {0x0000000cL, "ERROR_INVALID_ACCESS", "The access code is invalid."}
, {0x0000000dL, "ERROR_INVALID_DATA", "The data is invalid."}
, {0x0000000eL, "ERROR_OUTOFMEMORY", "Not enough storage is available to complete this operation."}
, {0x0000000fL, "ERROR_INVALID_DRIVE", "The system cannot find the drive specified."}
, {0x00000010L, "ERROR_CURRENT_DIRECTORY", "The directory cannot be removed."}
, {0x00000011L, "ERROR_NOT_SAME_DEVICE", "The system cannot move the file\nto a different disk drive."}
, {0x00000012L, "ERROR_NO_MORE_FILES", "There are no more files."}
, {0x00000013L, "ERROR_WRITE_PROTECT", "The media is write protected."}
, {0x00000014L, "ERROR_BAD_UNIT", "The system cannot find the device specified."}
, {0x00000015L, "ERROR_NOT_READY", "The device is not ready."}
, {0x00000016L, "ERROR_BAD_COMMAND", "The device does not recognize the command."}
, {0x00000017L, "ERROR_CRC", "Data error (cyclic redundancy check)"}
, {0x00000018L, "ERROR_BAD_LENGTH", "The program issued a command but the\ncommand length is incorrect."}
, {0x00000019L, "ERROR_SEEK", "The drive cannot locate a specific\narea or track on the disk."}
, {0x0000001aL, "ERROR_NOT_DOS_DISK", "The specified disk or diskette cannot be accessed."}
, {0x0000001bL, "ERROR_SECTOR_NOT_FOUND", "The drive cannot find the sector requested."}
, {0x0000001cL, "ERROR_OUT_OF_PAPER", "The printer is out of paper."}
, {0x0000001dL, "ERROR_WRITE_FAULT", "The system cannot write to the specified device."}
, {0x0000001eL, "ERROR_READ_FAULT", "The system cannot read from the specified device."}
, {0x0000001fL, "ERROR_GEN_FAILURE", "A device attached to the system is not functioning."}
, {0x00000020L, "ERROR_SHARING_VIOLATION", "The process cannot access the file because\nit is being used by another process."}
, {0x00000021L, "ERROR_LOCK_VIOLATION", "The process cannot access the file because\nanother process has locked a portion of the file."}
, {0x00000022L, "ERROR_WRONG_DISK", "The wrong diskette is in the drive.\nInsert %2 (Volume Serial Number: %3)\ninto drive %1."}
, {0x00000024L, "ERROR_SHARING_BUFFER_EXCEEDED", "Too many files opened for sharing."}
, {0x00000026L, "ERROR_HANDLE_EOF", "Reached end of file."}
, {0x00000027L, "ERROR_HANDLE_DISK_FULL", "The disk is full."}
, {0x00000032L, "ERROR_NOT_SUPPORTED", "The network request is not supported."}
, {0x00000033L, "ERROR_REM_NOT_LIST", "The remote computer is not available."}
, {0x00000034L, "ERROR_DUP_NAME", "A duplicate name exists on the network."}
, {0x00000035L, "ERROR_BAD_NETPATH", "The network path was not found."}
, {0x00000036L, "ERROR_NETWORK_BUSY", "The network is busy."}
, {0x00000037L, "ERROR_DEV_NOT_EXIST  ;// dderror", "The specified network resource or device is no longer\navailable."}
, {0x00000038L, "ERROR_TOO_MANY_CMDS", "The network BIOS command limit has been reached."}
, {0x00000039L, "ERROR_ADAP_HDW_ERR", "A network adapter hardware error occurred."}
, {0x0000003aL, "ERROR_BAD_NET_RESP", "The specified server cannot perform the requested\noperation."}
, {0x0000003bL, "ERROR_UNEXP_NET_ERR", "An unexpected network error occurred."}
, {0x0000003cL, "ERROR_BAD_REM_ADAP", "The remote adapter is not compatible."}
, {0x0000003dL, "ERROR_PRINTQ_FULL", "The printer queue is full."}
, {0x0000003eL, "ERROR_NO_SPOOL_SPACE", "Space to store the file waiting to be printed is\nnot available on the server."}
, {0x0000003fL, "ERROR_PRINT_CANCELLED", "Your file waiting to be printed was deleted."}
, {0x00000040L, "ERROR_NETNAME_DELETED", "The specified network name is no longer available."}
, {0x00000041L, "ERROR_NETWORK_ACCESS_DENIED", "Network access is denied."}
, {0x00000042L, "ERROR_BAD_DEV_TYPE", "The network resource type is not correct."}
, {0x00000043L, "ERROR_BAD_NET_NAME", "The network name cannot be found."}
, {0x00000044L, "ERROR_TOO_MANY_NAMES", "The name limit for the local computer network\nadapter card was exceeded."}
, {0x00000045L, "ERROR_TOO_MANY_SESS", "The network BIOS session limit was exceeded."}
, {0x00000046L, "ERROR_SHARING_PAUSED", "The remote server has been paused or is in the\nprocess of being started."}
, {0x00000047L, "ERROR_REQ_NOT_ACCEP", "No more connections can be made to this remote computer at this time\nbecause there are already as many connections as the computer can accept."}
, {0x00000048L, "ERROR_REDIR_PAUSED", "The specified printer or disk device has been paused."}
, {0x00000050L, "ERROR_FILE_EXISTS", "The file exists."}
, {0x00000052L, "ERROR_CANNOT_MAKE", "The directory or file cannot be created."}
, {0x00000053L, "ERROR_FAIL_I24", "Fail on INT 24"}
, {0x00000054L, "ERROR_OUT_OF_STRUCTURES", "Storage to process this request is not available."}
, {0x00000055L, "ERROR_ALREADY_ASSIGNED", "The local device name is already in use."}
, {0x00000056L, "ERROR_INVALID_PASSWORD", "The specified network password is not correct."}
, {0x00000057L, "ERROR_INVALID_PARAMETER                  ;// dderror", "The parameter is incorrect."}
, {0x00000058L, "ERROR_NET_WRITE_FAULT", "A write fault occurred on the network."}
, {0x00000059L, "ERROR_NO_PROC_SLOTS", "The system cannot start another process at\nthis time."}
, {0x00000064L, "ERROR_TOO_MANY_SEMAPHORES", "Cannot create another system semaphore."}
, {0x00000065L, "ERROR_EXCL_SEM_ALREADY_OWNED", "The exclusive semaphore is owned by another process."}
, {0x00000066L, "ERROR_SEM_IS_SET", "The semaphore is set and cannot be closed."}
, {0x00000067L, "ERROR_TOO_MANY_SEM_REQUESTS", "The semaphore cannot be set again."}
, {0x00000068L, "ERROR_INVALID_AT_INTERRUPT_TIME", "Cannot request exclusive semaphores at interrupt time."}
, {0x00000069L, "ERROR_SEM_OWNER_DIED", "The previous ownership of this semaphore has ended."}
, {0x0000006aL, "ERROR_SEM_USER_LIMIT", "Insert the diskette for drive %1."}
, {0x0000006bL, "ERROR_DISK_CHANGE", "Program stopped because alternate diskette was not inserted."}
, {0x0000006cL, "ERROR_DRIVE_LOCKED", "The disk is in use or locked by\nanother process."}
, {0x0000006dL, "ERROR_BROKEN_PIPE", "The pipe has been ended."}
, {0x0000006eL, "ERROR_OPEN_FAILED", "The system cannot open the\ndevice or file specified."}
, {0x0000006fL, "ERROR_BUFFER_OVERFLOW", "The file name is too long."}
, {0x00000070L, "ERROR_DISK_FULL", "There is not enough space on the disk."}
, {0x00000071L, "ERROR_NO_MORE_SEARCH_HANDLES", "No more internal file identifiers available."}
, {0x00000072L, "ERROR_INVALID_TARGET_HANDLE", "The target internal file identifier is incorrect."}
, {0x00000075L, "ERROR_INVALID_CATEGORY", "The IOCTL call made by the application program is\nnot correct."}
, {0x00000076L, "ERROR_INVALID_VERIFY_SWITCH", "The verify-on-write switch parameter value is not\ncorrect."}
, {0x00000077L, "ERROR_BAD_DRIVER_LEVEL", "The system does not support the command requested."}
, {0x00000078L, "ERROR_CALL_NOT_IMPLEMENTED", "This function is only valid in Windows NT mode."}
, {0x00000079L, "ERROR_SEM_TIMEOUT", "The semaphore timeout period has expired."}
, {0x0000007aL, "ERROR_INSUFFICIENT_BUFFER                ;// dderror", "The data area passed to a system call is too\nsmall."}
, {0x0000007bL, "ERROR_INVALID_NAME", "The filename, directory name, or volume label syntax is incorrect."}
, {0x0000007cL, "ERROR_INVALID_LEVEL", "The system call level is not correct."}
, {0x0000007dL, "ERROR_NO_VOLUME_LABEL", "The disk has no volume label."}
, {0x0000007eL, "ERROR_MOD_NOT_FOUND", "The specified module could not be found."}
, {0x0000007fL, "ERROR_PROC_NOT_FOUND", "The specified procedure could not be found."}
, {0x00000080L, "ERROR_WAIT_NO_CHILDREN", "There are no child processes to wait for."}
, {0x00000081L, "ERROR_CHILD_NOT_COMPLETE", "The %1 application cannot be run in Windows NT mode."}
, {0x00000082L, "ERROR_DIRECT_ACCESS_HANDLE", "Attempt to use a file handle to an open disk partition for an\noperation other than raw disk I/O."}
, {0x00000083L, "ERROR_NEGATIVE_SEEK", "An attempt was made to move the file pointer before the beginning of the file."}
, {0x00000084L, "ERROR_SEEK_ON_DEVICE", "The file pointer cannot be set on the specified device or file."}
, {0x00000085L, "ERROR_IS_JOIN_TARGET", "A JOIN or SUBST command\ncannot be used for a drive that\ncontains previously joined drives."}
, {0x00000086L, "ERROR_IS_JOINED", "An attempt was made to use a\nJOIN or SUBST command on a drive that has\nalready been joined."}
, {0x00000087L, "ERROR_IS_SUBSTED", "An attempt was made to use a\nJOIN or SUBST command on a drive that has\nalready been substituted."}
, {0x00000088L, "ERROR_NOT_JOINED", "The system tried to delete\nthe JOIN of a drive that is not joined."}
, {0x00000089L, "ERROR_NOT_SUBSTED", "The system tried to delete the\nsubstitution of a drive that is not substituted."}
, {0x0000008aL, "ERROR_JOIN_TO_JOIN", "The system tried to join a drive\nto a directory on a joined drive."}
, {0x0000008bL, "ERROR_SUBST_TO_SUBST", "The system tried to substitute a\ndrive to a directory on a substituted drive."}
, {0x0000008cL, "ERROR_JOIN_TO_SUBST", "The system tried to join a drive to\na directory on a substituted drive."}
, {0x0000008dL, "ERROR_SUBST_TO_JOIN", "The system tried to SUBST a drive\nto a directory on a joined drive."}
, {0x0000008eL, "ERROR_BUSY_DRIVE", "The system cannot perform a JOIN or SUBST at this time."}
, {0x0000008fL, "ERROR_SAME_DRIVE", "The system cannot join or substitute a\ndrive to or for a directory on the same drive."}
, {0x00000090L, "ERROR_DIR_NOT_ROOT", "The directory is not a subdirectory of the root directory."}
, {0x00000091L, "ERROR_DIR_NOT_EMPTY", "The directory is not empty."}
, {0x00000092L, "ERROR_IS_SUBST_PATH", "The path specified is being used in\na substitute."}
, {0x00000093L, "ERROR_IS_JOIN_PATH", "Not enough resources are available to\nprocess this command."}
, {0x00000094L, "ERROR_PATH_BUSY", "The path specified cannot be used at this time."}
, {0x00000095L, "ERROR_IS_SUBST_TARGET", "An attempt was made to join\nor substitute a drive for which a directory\non the drive is the target of a previous\nsubstitute."}
, {0x00000096L, "ERROR_SYSTEM_TRACE", "System trace information was not specified in your\nCONFIG.SYS file, or tracing is disallowed."}
, {0x00000097L, "ERROR_INVALID_EVENT_COUNT", "The number of specified semaphore events for\nDosMuxSemWait is not correct."}
, {0x00000098L, "ERROR_TOO_MANY_MUXWAITERS", "DosMuxSemWait did not execute; too many semaphores\nare already set."}
, {0x00000099L, "ERROR_INVALID_LIST_FORMAT", "The DosMuxSemWait list is not correct."}
, {0x0000009aL, "ERROR_LABEL_TOO_LONG", "The volume label you entered exceeds the label character\nlimit of the target file system."}
, {0x0000009bL, "ERROR_TOO_MANY_TCBS", "Cannot create another thread."}
, {0x0000009cL, "ERROR_SIGNAL_REFUSED", "The recipient process has refused the signal."}
, {0x0000009dL, "ERROR_DISCARDED", "The segment is already discarded and cannot be locked."}
, {0x0000009eL, "ERROR_NOT_LOCKED", "The segment is already unlocked."}
, {0x0000009fL, "ERROR_BAD_THREADID_ADDR", "The address for the thread ID is not correct."}
, {0x000000a0L, "ERROR_BAD_ARGUMENTS", "The argument string passed to DosExecPgm is not correct."}
, {0x000000a1L, "ERROR_BAD_PATHNAME", "The specified path is invalid."}
, {0x000000a2L, "ERROR_SIGNAL_PENDING", "A signal is already pending."}
, {0x000000a4L, "ERROR_MAX_THRDS_REACHED", "No more threads can be created in the system."}
, {0x000000a7L, "ERROR_LOCK_FAILED", "Unable to lock a region of a file."}
, {0x000000aaL, "ERROR_BUSY", "The requested resource is in use."}
, {0x000000adL, "ERROR_CANCEL_VIOLATION", "A lock request was not outstanding for the supplied cancel region."}
, {0x000000aeL, "ERROR_ATOMIC_LOCKS_NOT_SUPPORTED", "The file system does not support atomic changes to the lock type."}
, {0x000000b4L, "ERROR_INVALID_SEGMENT_NUMBER", "The system detected a segment number that was not correct."}
, {0x000000b6L, "ERROR_INVALID_ORDINAL", "The operating system cannot run %1."}
, {0x000000b7L, "ERROR_ALREADY_EXISTS", "Cannot create a file when that file already exists."}
, {0x000000baL, "ERROR_INVALID_FLAG_NUMBER", "The flag passed is not correct."}
, {0x000000bbL, "ERROR_SEM_NOT_FOUND", "The specified system semaphore name was not found."}
, {0x000000bcL, "ERROR_INVALID_STARTING_CODESEG", "The operating system cannot run %1."}
, {0x000000bdL, "ERROR_INVALID_STACKSEG", "The operating system cannot run %1."}
, {0x000000beL, "ERROR_INVALID_MODULETYPE", "The operating system cannot run %1."}
, {0x000000bfL, "ERROR_INVALID_EXE_SIGNATURE", "Cannot run %1 in Windows NT mode."}
, {0x000000c0L, "ERROR_EXE_MARKED_INVALID", "The operating system cannot run %1."}
, {0x000000c1L, "ERROR_BAD_EXE_FORMAT", "%1 is not a valid Windows NT application."}
, {0x000000c2L, "ERROR_ITERATED_DATA_EXCEEDS_64k", "The operating system cannot run %1."}
, {0x000000c3L, "ERROR_INVALID_MINALLOCSIZE", "The operating system cannot run %1."}
, {0x000000c4L, "ERROR_DYNLINK_FROM_INVALID_RING", "The operating system cannot run this\napplication program."}
, {0x000000c5L, "ERROR_IOPL_NOT_ENABLED", "The operating system is not presently\nconfigured to run this application."}
, {0x000000c6L, "ERROR_INVALID_SEGDPL", "The operating system cannot run %1."}
, {0x000000c7L, "ERROR_AUTODATASEG_EXCEEDS_64k", "The operating system cannot run this\napplication program."}
, {0x000000c8L, "ERROR_RING2SEG_MUST_BE_MOVABLE", "The code segment cannot be greater than or equal to 64KB."}
, {0x000000c9L, "ERROR_RELOC_CHAIN_XEEDS_SEGLIM", "The operating system cannot run %1."}
, {0x000000caL, "ERROR_INFLOOP_IN_RELOC_CHAIN", "The operating system cannot run %1."}
, {0x000000cbL, "ERROR_ENVVAR_NOT_FOUND", "The system could not find the environment\noption that was entered."}
, {0x000000cdL, "ERROR_NO_SIGNAL_SENT", "No process in the command subtree has a\nsignal handler."}
, {0x000000ceL, "ERROR_FILENAME_EXCED_RANGE", "The filename or extension is too long."}
, {0x000000cfL, "ERROR_RING2_STACK_IN_USE", "The ring 2 stack is in use."}
, {0x000000d0L, "ERROR_META_EXPANSION_TOO_LONG", "The global filename characters, * or ?, are entered\nincorrectly or too many global filename characters are specified."}
, {0x000000d1L, "ERROR_INVALID_SIGNAL_NUMBER", "The signal being posted is not correct."}
, {0x000000d2L, "ERROR_THREAD_1_INACTIVE", "The signal handler cannot be set."}
, {0x000000d4L, "ERROR_LOCKED", "The segment is locked and cannot be reallocated."}
, {0x000000d6L, "ERROR_TOO_MANY_MODULES", "Too many dynamic link modules are attached to this\nprogram or dynamic link module."}
, {0x000000d7L, "ERROR_NESTING_NOT_ALLOWED", "Can't nest calls to LoadModule."}
, {0x000000d8L, "ERROR_EXE_MACHINE_TYPE_MISMATCH", "The image file %1 is valid, but is for a machine type other\nthan the current machine."}
, {0x000000e6L, "ERROR_BAD_PIPE", "The pipe state is invalid."}
, {0x000000e7L, "ERROR_PIPE_BUSY", "All pipe instances are busy."}
, {0x000000e8L, "ERROR_NO_DATA", "The pipe is being closed."}
, {0x000000e9L, "ERROR_PIPE_NOT_CONNECTED", "No process is on the other end of the pipe."}
, {0x000000eaL, "ERROR_MORE_DATA              ;// dderror", "More data is available."}
, {0x000000f0L, "ERROR_VC_DISCONNECTED", "The session was cancelled."}
, {0x000000feL, "ERROR_INVALID_EA_NAME", "The specified extended attribute name was invalid."}
, {0x000000ffL, "ERROR_EA_LIST_INCONSISTENT", "The extended attributes are inconsistent."}
, {0x00000103L, "ERROR_NO_MORE_ITEMS", "No more data is available."}
, {0x0000010aL, "ERROR_CANNOT_COPY", "The Copy API cannot be used."}
, {0x0000010bL, "ERROR_DIRECTORY", "The directory name is invalid."}
, {0x00000113L, "ERROR_EAS_DIDNT_FIT", "The extended attributes did not fit in the buffer."}
, {0x00000114L, "ERROR_EA_FILE_CORRUPT", "The extended attribute file on the mounted file system is corrupt."}
, {0x00000115L, "ERROR_EA_TABLE_FULL", "The extended attribute table file is full."}
, {0x00000116L, "ERROR_INVALID_EA_HANDLE", "The specified extended attribute handle is invalid."}
, {0x0000011aL, "ERROR_EAS_NOT_SUPPORTED", "The mounted file system does not support extended attributes."}
, {0x00000120L, "ERROR_NOT_OWNER", "Attempt to release mutex not owned by caller."}
, {0x0000012aL, "ERROR_TOO_MANY_POSTS", "Too many posts were made to a semaphore."}
, {0x0000012bL, "ERROR_PARTIAL_COPY", "Only part of a Read/WriteProcessMemory request was completed."}
, {0x0000013dL, "ERROR_MR_MID_NOT_FOUND", "The system cannot find message for message number 0x%1\nin message file for %2."}
, {0x000001e7L, "ERROR_INVALID_ADDRESS", "Attempt to access invalid address."}
, {0x00000216L, "ERROR_ARITHMETIC_OVERFLOW", "Arithmetic result exceeded 32 bits."}
, {0x00000217L, "ERROR_PIPE_CONNECTED", "There is a process on other end of the pipe."}
, {0x00000218L, "ERROR_PIPE_LISTENING", "Waiting for a process to open the other end of the pipe."}
, {0x000003e2L, "ERROR_EA_ACCESS_DENIED", "Access to the extended attribute was denied."}
, {0x000003e3L, "ERROR_OPERATION_ABORTED", "The I/O operation has been aborted because of either a thread exit\nor an application request."}
, {0x000003e4L, "ERROR_IO_INCOMPLETE", "Overlapped I/O event is not in a signalled state."}
, {0x000003e5L, "ERROR_IO_PENDING                         ;// dderror", "Overlapped I/O operation is in progress."}
, {0x000003e6L, "ERROR_NOACCESS", "Invalid access to memory location."}
, {0x000003e7L, "ERROR_SWAPERROR", "Error performing inpage operation."}
, {0x000003e9L, "ERROR_STACK_OVERFLOW", "Recursion too deep, stack overflowed."}
, {0x000003eaL, "ERROR_INVALID_MESSAGE", "The window cannot act on the sent message."}
, {0x000003ebL, "ERROR_CAN_NOT_COMPLETE", "Cannot complete this function."}
, {0x000003ecL, "ERROR_INVALID_FLAGS", "Invalid flags."}
, {0x000003edL, "ERROR_UNRECOGNIZED_VOLUME", "The volume does not contain a recognized file system.\nPlease make sure that all required file system drivers are loaded and that the\nvolume is not corrupt."}
, {0x000003eeL, "ERROR_FILE_INVALID", "The volume for a file has been externally altered such that the\nopened file is no longer valid."}
, {0x000003efL, "ERROR_FULLSCREEN_MODE", "The requested operation cannot be performed in full-screen mode."}
, {0x000003f0L, "ERROR_NO_TOKEN", "An attempt was made to reference a token that does not exist."}
, {0x000003f1L, "ERROR_BADDB", "The configuration registry database is corrupt."}
, {0x000003f2L, "ERROR_BADKEY", "The configuration registry key is invalid."}
, {0x000003f3L, "ERROR_CANTOPEN", "The configuration registry key could not be opened."}
, {0x000003f4L, "ERROR_CANTREAD", "The configuration registry key could not be read."}
, {0x000003f5L, "ERROR_CANTWRITE", "The configuration registry key could not be written."}
, {0x000003f6L, "ERROR_REGISTRY_RECOVERED", "One of the files in the Registry database had to be recovered\nby use of a log or alternate copy.  The recovery was successful."}
, {0x000003f7L, "ERROR_REGISTRY_CORRUPT", "The Registry is corrupt. The structure of one of the files that contains\nRegistry data is corrupt, or the system's image of the file in memory\nis corrupt, or the file could not be recovered because the alternate\ncopy or log was absent or corrupt."}
, {0x000003f8L, "ERROR_REGISTRY_IO_FAILED", "An I/O operation initiated by the Registry failed unrecoverably.\nThe Registry could not read in, or write out, or flush, one of the files\nthat contain the system's image of the Registry."}
, {0x000003f9L, "ERROR_NOT_REGISTRY_FILE", "The system has attempted to load or restore a file into the Registry, but the\nspecified file is not in a Registry file format."}
, {0x000003faL, "ERROR_KEY_DELETED", "Illegal operation attempted on a Registry key which has been marked for deletion."}
, {0x000003fbL, "ERROR_NO_LOG_SPACE", "System could not allocate the required space in a Registry log."}
, {0x000003fcL, "ERROR_KEY_HAS_CHILDREN", "Cannot create a symbolic link in a Registry key that already\nhas subkeys or values."}
, {0x000003fdL, "ERROR_CHILD_MUST_BE_VOLATILE", "Cannot create a stable subkey under a volatile parent key."}
, {0x000003feL, "ERROR_NOTIFY_ENUM_DIR", "A notify change request is being completed and the information\nis not being returned in the caller's buffer. The caller now\nneeds to enumerate the files to find the changes."}
, {0x0000041bL, "ERROR_DEPENDENT_SERVICES_RUNNING", "A stop control has been sent to a service which other running services\nare dependent on."}
, {0x0000041cL, "ERROR_INVALID_SERVICE_CONTROL", "The requested control is not valid for this service"}
, {0x0000041dL, "ERROR_SERVICE_REQUEST_TIMEOUT", "The service did not respond to the start or control request in a timely\nfashion."}
, {0x0000041eL, "ERROR_SERVICE_NO_THREAD", "A thread could not be created for the service."}
, {0x0000041fL, "ERROR_SERVICE_DATABASE_LOCKED", "The service database is locked."}
, {0x00000420L, "ERROR_SERVICE_ALREADY_RUNNING", "An instance of the service is already running."}
, {0x00000421L, "ERROR_INVALID_SERVICE_ACCOUNT", "The account name is invalid or does not exist."}
, {0x00000422L, "ERROR_SERVICE_DISABLED", "The specified service is disabled and cannot be started."}
, {0x00000423L, "ERROR_CIRCULAR_DEPENDENCY", "Circular service dependency was specified."}
, {0x00000424L, "ERROR_SERVICE_DOES_NOT_EXIST", "The specified service does not exist as an installed service."}
, {0x00000425L, "ERROR_SERVICE_CANNOT_ACCEPT_CTRL", "The service cannot accept control messages at this time."}
, {0x00000426L, "ERROR_SERVICE_NOT_ACTIVE", "The service has not been started."}
, {0x00000427L, "ERROR_FAILED_SERVICE_CONTROLLER_CONNECT", "The service process could not connect to the service controller."}
, {0x00000428L, "ERROR_EXCEPTION_IN_SERVICE", "An exception occurred in the service when handling the control request."}
, {0x00000429L, "ERROR_DATABASE_DOES_NOT_EXIST", "The database specified does not exist."}
, {0x0000042aL, "ERROR_SERVICE_SPECIFIC_ERROR", "The service has returned a service-specific error code."}
, {0x0000042bL, "ERROR_PROCESS_ABORTED", "The process terminated unexpectedly."}
, {0x0000042cL, "ERROR_SERVICE_DEPENDENCY_FAIL", "The dependency service or group failed to start."}
, {0x0000042dL, "ERROR_SERVICE_LOGON_FAILED", "The service did not start due to a logon failure."}
, {0x0000042eL, "ERROR_SERVICE_START_HANG", "After starting, the service hung in a start-pending state."}
, {0x0000042fL, "ERROR_INVALID_SERVICE_LOCK", "The specified service database lock is invalid."}
, {0x00000430L, "ERROR_SERVICE_MARKED_FOR_DELETE", "The specified service has been marked for deletion."}
, {0x00000431L, "ERROR_SERVICE_EXISTS", "The specified service already exists."}
, {0x00000432L, "ERROR_ALREADY_RUNNING_LKG", "The system is currently running with the last-known-good configuration."}
, {0x00000433L, "ERROR_SERVICE_DEPENDENCY_DELETED", "The dependency service does not exist or has been marked for\ndeletion."}
, {0x00000434L, "ERROR_BOOT_ALREADY_ACCEPTED", "The current boot has already been accepted for use as the\nlast-known-good control set."}
, {0x00000435L, "ERROR_SERVICE_NEVER_STARTED", "No attempts to start the service have been made since the last boot."}
, {0x00000436L, "ERROR_DUPLICATE_SERVICE_NAME", "The name is already in use as either a service name or a service display\nname."}
, {0x00000437L, "ERROR_DIFFERENT_SERVICE_ACCOUNT", "The account specified for this service is different from the account\nspecified for other services running in the same process."}
, {0x0000044cL, "ERROR_END_OF_MEDIA", "The physical end of the tape has been reached."}
, {0x0000044dL, "ERROR_FILEMARK_DETECTED", "A tape access reached a filemark."}
, {0x0000044eL, "ERROR_BEGINNING_OF_MEDIA", "Beginning of tape or partition was encountered."}
, {0x0000044fL, "ERROR_SETMARK_DETECTED", "A tape access reached the end of a set of files."}
, {0x00000450L, "ERROR_NO_DATA_DETECTED", "No more data is on the tape."}
, {0x00000451L, "ERROR_PARTITION_FAILURE", "Tape could not be partitioned."}
, {0x00000452L, "ERROR_INVALID_BLOCK_LENGTH", "When accessing a new tape of a multivolume partition, the current\nblocksize is incorrect."}
, {0x00000453L, "ERROR_DEVICE_NOT_PARTITIONED", "Tape partition information could not be found when loading a tape."}
, {0x00000454L, "ERROR_UNABLE_TO_LOCK_MEDIA", "Unable to lock the media eject mechanism."}
, {0x00000455L, "ERROR_UNABLE_TO_UNLOAD_MEDIA", "Unable to unload the media."}
, {0x00000456L, "ERROR_MEDIA_CHANGED", "Media in drive may have changed."}
, {0x00000457L, "ERROR_BUS_RESET", "The I/O bus was reset."}
, {0x00000458L, "ERROR_NO_MEDIA_IN_DRIVE", "No media in drive."}
, {0x00000459L, "ERROR_NO_UNICODE_TRANSLATION", "No mapping for the Unicode character exists in the target multi-byte code page."}
, {0x0000045aL, "ERROR_DLL_INIT_FAILED", "A dynamic link library (DLL) initialization routine failed."}
, {0x0000045bL, "ERROR_SHUTDOWN_IN_PROGRESS", "A system shutdown is in progress."}
, {0x0000045cL, "ERROR_NO_SHUTDOWN_IN_PROGRESS", "Unable to abort the system shutdown because no shutdown was in progress."}
, {0x0000045dL, "ERROR_IO_DEVICE", "The request could not be performed because of an I/O device error."}
, {0x0000045eL, "ERROR_SERIAL_NO_DEVICE", "No serial device was successfully initialized.  The serial driver will unload."}
, {0x0000045fL, "ERROR_IRQ_BUSY", "Unable to open a device that was sharing an interrupt request (IRQ)\nwith other devices. At least one other device that uses that IRQ\nwas already opened."}
, {0x00000460L, "ERROR_MORE_WRITES", "A serial I/O operation was completed by another write to the serial port.\n(The IOCTL_SERIAL_XOFF_COUNTER reached zero.)"}
, {0x00000461L, "ERROR_COUNTER_TIMEOUT", "A serial I/O operation completed because the time-out period expired.\n(The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.)"}
, {0x00000462L, "ERROR_FLOPPY_ID_MARK_NOT_FOUND", "No ID address mark was found on the floppy disk."}
, {0x00000463L, "ERROR_FLOPPY_WRONG_CYLINDER", "Mismatch between the floppy disk sector ID field and the floppy disk\ncontroller track address."}
, {0x00000464L, "ERROR_FLOPPY_UNKNOWN_ERROR", "The floppy disk controller reported an error that is not recognized\nby the floppy disk driver."}
, {0x00000465L, "ERROR_FLOPPY_BAD_REGISTERS", "The floppy disk controller returned inconsistent results in its registers."}
, {0x00000466L, "ERROR_DISK_RECALIBRATE_FAILED", "While accessing the hard disk, a recalibrate operation failed, even after retries."}
, {0x00000467L, "ERROR_DISK_OPERATION_FAILED", "While accessing the hard disk, a disk operation failed even after retries."}
, {0x00000468L, "ERROR_DISK_RESET_FAILED", "While accessing the hard disk, a disk controller reset was needed, but\neven that failed."}
, {0x00000469L, "ERROR_EOM_OVERFLOW", "Physical end of tape encountered."}
, {0x0000046aL, "ERROR_NOT_ENOUGH_SERVER_MEMORY", "Not enough server storage is available to process this command."}
, {0x0000046bL, "ERROR_POSSIBLE_DEADLOCK", "A potential deadlock condition has been detected."}
, {0x0000046cL, "ERROR_MAPPED_ALIGNMENT", "The base address or the file offset specified does not have the proper\nalignment."}
, {0x00000474L, "ERROR_SET_POWER_STATE_VETOED            ;public_win40", "An attempt to change the system power state was vetoed by another   ;public_win40\napplication or driver.                                              ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1141 SymbolicName=ERROR_SET_POWER_STATE_FAILED            ;public_win40\nLanguage=English                                                    ;public_win40\nThe system BIOS failed an attempt to change the system power state. ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1142 SymbolicName=ERROR_TOO_MANY_LINKS\nLanguage=English\nAn attempt was made to create more links on a file than\nthe file system supports."}
, {0x0000047eL, "ERROR_OLD_WIN_VERSION", "The specified program requires a newer version of Windows."}
, {0x0000047fL, "ERROR_APP_WRONG_OS", "The specified program is not a Windows or MS-DOS program."}
, {0x00000480L, "ERROR_SINGLE_INSTANCE_APP", "Cannot start more than one instance of the specified program."}
, {0x00000481L, "ERROR_RMODE_APP", "The specified program was written for an older version of Windows."}
, {0x00000482L, "ERROR_INVALID_DLL", "One of the library files needed to run this application is damaged."}
, {0x00000483L, "ERROR_NO_ASSOCIATION", "No application is associated with the specified file for this operation."}
, {0x00000484L, "ERROR_DDE_FAIL", "An error occurred in sending the command to the application."}
, {0x00000485L, "ERROR_DLL_NOT_FOUND", "One of the library files needed to run this application cannot be found."}
, {0x0000089aL, "ERROR_BAD_USERNAME", "The specified username is invalid."}
, {0x000008caL, "ERROR_NOT_CONNECTED", "This network connection does not exist."}
, {0x00000961L, "ERROR_OPEN_FILES", "This network connection has files open or requests pending."}
, {0x00000962L, "ERROR_ACTIVE_CONNECTIONS", "Active connections still exist."}
, {0x00000964L, "ERROR_DEVICE_IN_USE", "The device is in use by an active process and cannot be disconnected."}
, {0x000004b0L, "ERROR_BAD_DEVICE", "The specified device name is invalid."}
, {0x000004b1L, "ERROR_CONNECTION_UNAVAIL", "The device is not currently connected but it is a remembered connection."}
, {0x000004b2L, "ERROR_DEVICE_ALREADY_REMEMBERED", "An attempt was made to remember a device that had previously been remembered."}
, {0x000004b3L, "ERROR_NO_NET_OR_BAD_PATH", "No network provider accepted the given network path."}
, {0x000004b4L, "ERROR_BAD_PROVIDER", "The specified network provider name is invalid."}
, {0x000004b5L, "ERROR_CANNOT_OPEN_PROFILE", "Unable to open the network connection profile."}
, {0x000004b6L, "ERROR_BAD_PROFILE", "The network connection profile is corrupt."}
, {0x000004b7L, "ERROR_NOT_CONTAINER", "Cannot enumerate a non-container."}
, {0x000004b8L, "ERROR_EXTENDED_ERROR", "An extended error has occurred."}
, {0x000004b9L, "ERROR_INVALID_GROUPNAME", "The format of the specified group name is invalid."}
, {0x000004baL, "ERROR_INVALID_COMPUTERNAME", "The format of the specified computer name is invalid."}
, {0x000004bbL, "ERROR_INVALID_EVENTNAME", "The format of the specified event name is invalid."}
, {0x000004bcL, "ERROR_INVALID_DOMAINNAME", "The format of the specified domain name is invalid."}
, {0x000004bdL, "ERROR_INVALID_SERVICENAME", "The format of the specified service name is invalid."}
, {0x000004beL, "ERROR_INVALID_NETNAME", "The format of the specified network name is invalid."}
, {0x000004bfL, "ERROR_INVALID_SHARENAME", "The format of the specified share name is invalid."}
, {0x000004c0L, "ERROR_INVALID_PASSWORDNAME", "The format of the specified password is invalid."}
, {0x000004c1L, "ERROR_INVALID_MESSAGENAME", "The format of the specified message name is invalid."}
, {0x000004c2L, "ERROR_INVALID_MESSAGEDEST", "The format of the specified message destination is invalid."}
, {0x000004c3L, "ERROR_SESSION_CREDENTIAL_CONFLICT", "The credentials supplied conflict with an existing set of credentials."}
, {0x000004c4L, "ERROR_REMOTE_SESSION_LIMIT_EXCEEDED", "An attempt was made to establish a session to a network server, but there\nare already too many sessions established to that server."}
, {0x000004c5L, "ERROR_DUP_DOMAINNAME", "The workgroup or domain name is already in use by another computer on the\nnetwork."}
, {0x000004c6L, "ERROR_NO_NETWORK", "The network is not present or not started."}
, {0x000004c7L, "ERROR_CANCELLED", "The operation was cancelled by the user."}
, {0x000004c8L, "ERROR_USER_MAPPED_FILE", "The requested operation cannot be performed on a file with a user mapped section open."}
, {0x000004c9L, "ERROR_CONNECTION_REFUSED", "The remote system refused the network connection."}
, {0x000004caL, "ERROR_GRACEFUL_DISCONNECT", "The network connection was gracefully closed."}
, {0x000004cbL, "ERROR_ADDRESS_ALREADY_ASSOCIATED", "The network transport endpoint already has an address associated with it."}
, {0x000004ccL, "ERROR_ADDRESS_NOT_ASSOCIATED", "An address has not yet been associated with the network endpoint."}
, {0x000004cdL, "ERROR_CONNECTION_INVALID", "An operation was attempted on a non-existent network connection."}
, {0x000004ceL, "ERROR_CONNECTION_ACTIVE", "An invalid operation was attempted on an active network connection."}
, {0x000004cfL, "ERROR_NETWORK_UNREACHABLE", "The remote network is not reachable by the transport."}
, {0x000004d0L, "ERROR_HOST_UNREACHABLE", "The remote system is not reachable by the transport."}
, {0x000004d1L, "ERROR_PROTOCOL_UNREACHABLE", "The remote system does not support the transport protocol."}
, {0x000004d2L, "ERROR_PORT_UNREACHABLE", "No service is operating at the destination network endpoint\non the remote system."}
, {0x000004d3L, "ERROR_REQUEST_ABORTED", "The request was aborted."}
, {0x000004d4L, "ERROR_CONNECTION_ABORTED", "The network connection was aborted by the local system."}
, {0x000004d5L, "ERROR_RETRY", "The operation could not be completed.  A retry should be performed."}
, {0x000004d6L, "ERROR_CONNECTION_COUNT_LIMIT", "A connection to the server could not be made because the limit on the number of\nconcurrent connections for this account has been reached."}
, {0x000004d7L, "ERROR_LOGIN_TIME_RESTRICTION", "Attempting to login during an unauthorized time of day for this account."}
, {0x000004d8L, "ERROR_LOGIN_WKSTA_RESTRICTION", "The account is not authorized to login from this station."}
, {0x000004d9L, "ERROR_INCORRECT_ADDRESS", "The network address could not be used for the operation requested."}
, {0x000004daL, "ERROR_ALREADY_REGISTERED", "The service is already registered."}
, {0x000004dbL, "ERROR_SERVICE_NOT_FOUND", "The specified service does not exist."}
, {0x000004dcL, "ERROR_NOT_AUTHENTICATED                 ;public_win40", "The operation being requested was not performed because the user    ;public_win40\nhas not been authenticated.                                         ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1245 SymbolicName=ERROR_NOT_LOGGED_ON                     ;public_win40\nLanguage=English                                                    ;public_win40\nThe operation being requested was not performed because the user    ;public_win40\nhas not logged on to the network.                                   ;public_win40\nThe specified service does not exist.                               ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1246 SymbolicName=ERROR_CONTINUE                          ;public_win40\nLanguage=English                                                    ;public_win40\nReturn that wants caller to continue with work in progress.         ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1247 SymbolicName=ERROR_ALREADY_INITIALIZED               ;public_win40\nLanguage=English                                                    ;public_win40\nAn attempt was made to perform an initialization operation when     ;public_win40\ninitialization has already been completed.                          ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\nMessageId=1248 SymbolicName=ERROR_NO_MORE_DEVICES                   ;public_win40\nLanguage=English                                                    ;public_win40\nNo more local devices.                                              ;public_win40\n.                                                                   ;public_win40\n                                                                    ;public_win40\n;\n;\n;\n;///////////////////////////\n;//                       //\n;// Security Status Codes //\n;//                       //\n;///////////////////////////\n;\n;\n\nMessageId=1300 SymbolicName=ERROR_NOT_ALL_ASSIGNED\nLanguage=English\nNot all privileges referenced are assigned to the caller."}
, {0x00000515L, "ERROR_SOME_NOT_MAPPED", "Some mapping between account names and security IDs was not done."}
, {0x00000516L, "ERROR_NO_QUOTAS_FOR_ACCOUNT", "No system quota limits are specifically set for this account."}
, {0x00000517L, "ERROR_LOCAL_USER_SESSION_KEY", "No encryption key is available.  A well-known encryption key was returned."}
, {0x00000518L, "ERROR_NULL_LM_PASSWORD", "The NT password is too complex to be converted to a LAN Manager\npassword.  The LAN Manager password returned is a NULL string."}
, {0x00000519L, "ERROR_UNKNOWN_REVISION", "The revision level is unknown."}
, {0x0000051aL, "ERROR_REVISION_MISMATCH", "Indicates two revision levels are incompatible."}
, {0x0000051bL, "ERROR_INVALID_OWNER", "This security ID may not be assigned as the owner of this object."}
, {0x0000051cL, "ERROR_INVALID_PRIMARY_GROUP", "This security ID may not be assigned as the primary group of an object."}
, {0x0000051dL, "ERROR_NO_IMPERSONATION_TOKEN", "An attempt has been made to operate on an impersonation token\nby a thread that is not currently impersonating a client."}
, {0x0000051eL, "ERROR_CANT_DISABLE_MANDATORY", "The group may not be disabled."}
, {0x0000051fL, "ERROR_NO_LOGON_SERVERS", "There are currently no logon servers available to service the logon\nrequest."}
, {0x00000520L, "ERROR_NO_SUCH_LOGON_SESSION", " A specified logon session does not exist.  It may already have\n been terminated."}
, {0x00000521L, "ERROR_NO_SUCH_PRIVILEGE", " A specified privilege does not exist."}
, {0x00000522L, "ERROR_PRIVILEGE_NOT_HELD", " A required privilege is not held by the client."}
, {0x00000523L, "ERROR_INVALID_ACCOUNT_NAME", "The name provided is not a properly formed account name."}
, {0x00000524L, "ERROR_USER_EXISTS", "The specified user already exists."}
, {0x00000525L, "ERROR_NO_SUCH_USER", "The specified user does not exist."}
, {0x00000526L, "ERROR_GROUP_EXISTS", "The specified group already exists."}
, {0x00000527L, "ERROR_NO_SUCH_GROUP", "The specified group does not exist."}
, {0x00000528L, "ERROR_MEMBER_IN_GROUP", "Either the specified user account is already a member of the specified\ngroup, or the specified group cannot be deleted because it contains\na member."}
, {0x00000529L, "ERROR_MEMBER_NOT_IN_GROUP", "The specified user account is not a member of the specified group account."}
, {0x0000052aL, "ERROR_LAST_ADMIN", "The last remaining administration account cannot be disabled\nor deleted."}
, {0x0000052bL, "ERROR_WRONG_PASSWORD", "Unable to update the password.  The value provided as the current\npassword is incorrect."}
, {0x0000052cL, "ERROR_ILL_FORMED_PASSWORD", "Unable to update the password.  The value provided for the new password\ncontains values that are not allowed in passwords."}
, {0x0000052dL, "ERROR_PASSWORD_RESTRICTION", "Unable to update the password because a password update rule has been\nviolated."}
, {0x0000052eL, "ERROR_LOGON_FAILURE", "Logon failure: unknown user name or bad password."}
, {0x0000052fL, "ERROR_ACCOUNT_RESTRICTION", "Logon failure: user account restriction."}
, {0x00000530L, "ERROR_INVALID_LOGON_HOURS", "Logon failure: account logon time restriction violation."}
, {0x00000531L, "ERROR_INVALID_WORKSTATION", "Logon failure: user not allowed to log on to this computer."}
, {0x00000532L, "ERROR_PASSWORD_EXPIRED", "Logon failure: the specified account password has expired."}
, {0x00000533L, "ERROR_ACCOUNT_DISABLED", "Logon failure: account currently disabled."}
, {0x00000534L, "ERROR_NONE_MAPPED", "No mapping between account names and security IDs was done."}
, {0x00000535L, "ERROR_TOO_MANY_LUIDS_REQUESTED", "Too many local user identifiers (LUIDs) were requested at one time."}
, {0x00000536L, "ERROR_LUIDS_EXHAUSTED", "No more local user identifiers (LUIDs) are available."}
, {0x00000537L, "ERROR_INVALID_SUB_AUTHORITY", "The subauthority part of a security ID is invalid for this particular use."}
, {0x00000538L, "ERROR_INVALID_ACL", "The access control list (ACL) structure is invalid."}
, {0x00000539L, "ERROR_INVALID_SID", "The security ID structure is invalid."}
, {0x0000053aL, "ERROR_INVALID_SECURITY_DESCR", "The security descriptor structure is invalid."}
, {0x0000053cL, "ERROR_BAD_INHERITANCE_ACL", "The inherited access control list (ACL) or access control entry (ACE)\ncould not be built."}
, {0x0000053dL, "ERROR_SERVER_DISABLED", "The server is currently disabled."}
, {0x0000053eL, "ERROR_SERVER_NOT_DISABLED", "The server is currently enabled."}
, {0x0000053fL, "ERROR_INVALID_ID_AUTHORITY", "The value provided was an invalid value for an identifier authority."}
, {0x00000540L, "ERROR_ALLOTTED_SPACE_EXCEEDED", "No more memory is available for security information updates."}
, {0x00000541L, "ERROR_INVALID_GROUP_ATTRIBUTES", "The specified attributes are invalid, or incompatible with the\nattributes for the group as a whole."}
, {0x00000542L, "ERROR_BAD_IMPERSONATION_LEVEL", "Either a required impersonation level was not provided, or the\nprovided impersonation level is invalid."}
, {0x00000543L, "ERROR_CANT_OPEN_ANONYMOUS", "Cannot open an anonymous level security token."}
, {0x00000544L, "ERROR_BAD_VALIDATION_CLASS", "The validation information class requested was invalid."}
, {0x00000545L, "ERROR_BAD_TOKEN_TYPE", "The type of the token is inappropriate for its attempted use."}
, {0x00000546L, "ERROR_NO_SECURITY_ON_OBJECT", "Unable to perform a security operation on an object\nwhich has no associated security."}
, {0x00000547L, "ERROR_CANT_ACCESS_DOMAIN_INFO", "Indicates a Windows NT Server could not be contacted or that\nobjects within the domain are protected such that necessary\ninformation could not be retrieved."}
, {0x00000548L, "ERROR_INVALID_SERVER_STATE", "The security account manager (SAM) or local security\nauthority (LSA) server was in the wrong state to perform\nthe security operation."}
, {0x00000549L, "ERROR_INVALID_DOMAIN_STATE", "The domain was in the wrong state to perform the security operation."}
, {0x0000054aL, "ERROR_INVALID_DOMAIN_ROLE", "This operation is only allowed for the Primary Domain Controller of the domain."}
, {0x0000054bL, "ERROR_NO_SUCH_DOMAIN", "The specified domain did not exist."}
, {0x0000054cL, "ERROR_DOMAIN_EXISTS", "The specified domain already exists."}
, {0x0000054dL, "ERROR_DOMAIN_LIMIT_EXCEEDED", "An attempt was made to exceed the limit on the number of domains per server."}
, {0x0000054eL, "ERROR_INTERNAL_DB_CORRUPTION", "Unable to complete the requested operation because of either a\ncatastrophic media failure or a data structure corruption on the disk."}
, {0x0000054fL, "ERROR_INTERNAL_ERROR", "The security account database contains an internal inconsistency."}
, {0x00000550L, "ERROR_GENERIC_NOT_MAPPED", "Generic access types were contained in an access mask which should\nalready be mapped to non-generic types."}
, {0x00000551L, "ERROR_BAD_DESCRIPTOR_FORMAT", "A security descriptor is not in the right format (absolute or self-relative)."}
, {0x00000552L, "ERROR_NOT_LOGON_PROCESS", "The requested action is restricted for use by logon processes\nonly.  The calling process has not registered as a logon process."}
, {0x00000553L, "ERROR_LOGON_SESSION_EXISTS", "Cannot start a new logon session with an ID that is already in use."}
, {0x00000554L, "ERROR_NO_SUCH_PACKAGE", "A specified authentication package is unknown."}
, {0x00000555L, "ERROR_BAD_LOGON_SESSION_STATE", "The logon session is not in a state that is consistent with the\nrequested operation."}
, {0x00000556L, "ERROR_LOGON_SESSION_COLLISION", "The logon session ID is already in use."}
, {0x00000557L, "ERROR_INVALID_LOGON_TYPE", "A logon request contained an invalid logon type value."}
, {0x00000558L, "ERROR_CANNOT_IMPERSONATE", "Unable to impersonate via a named pipe until data has been read\nfrom that pipe."}
, {0x00000559L, "ERROR_RXACT_INVALID_STATE", "The transaction state of a Registry subtree is incompatible with the\nrequested operation."}
, {0x0000055aL, "ERROR_RXACT_COMMIT_FAILURE", "An internal security database corruption has been encountered."}
, {0x0000055bL, "ERROR_SPECIAL_ACCOUNT", "Cannot perform this operation on built-in accounts."}
, {0x0000055cL, "ERROR_SPECIAL_GROUP", "Cannot perform this operation on this built-in special group."}
, {0x0000055dL, "ERROR_SPECIAL_USER", "Cannot perform this operation on this built-in special user."}
, {0x0000055eL, "ERROR_MEMBERS_PRIMARY_GROUP", "The user cannot be removed from a group because the group\nis currently the user's primary group."}
, {0x0000055fL, "ERROR_TOKEN_ALREADY_IN_USE", "The token is already in use as a primary token."}
, {0x00000560L, "ERROR_NO_SUCH_ALIAS", "The specified local group does not exist."}
, {0x00000561L, "ERROR_MEMBER_NOT_IN_ALIAS", "The specified account name is not a member of the local group."}
, {0x00000562L, "ERROR_MEMBER_IN_ALIAS", "The specified account name is already a member of the local group."}
, {0x00000563L, "ERROR_ALIAS_EXISTS", "The specified local group already exists."}
, {0x00000564L, "ERROR_LOGON_NOT_GRANTED", "Logon failure: the user has not been granted the requested\nlogon type at this computer."}
, {0x00000565L, "ERROR_TOO_MANY_SECRETS", "The maximum number of secrets that may be stored in a single system has been\nexceeded."}
, {0x00000566L, "ERROR_SECRET_TOO_LONG", "The length of a secret exceeds the maximum length allowed."}
, {0x00000567L, "ERROR_INTERNAL_DB_ERROR", "The local security authority database contains an internal inconsistency."}
, {0x00000568L, "ERROR_TOO_MANY_CONTEXT_IDS", "During a logon attempt, the user's security context accumulated too many\nsecurity IDs."}
, {0x00000569L, "ERROR_LOGON_TYPE_NOT_GRANTED", "Logon failure: the user has not been granted the requested logon type\nat this computer."}
, {0x0000056aL, "ERROR_NT_CROSS_ENCRYPTION_REQUIRED", "A cross-encrypted password is necessary to change a user password."}
, {0x0000056bL, "ERROR_NO_SUCH_MEMBER", "A new member could not be added to a local group because the member does\nnot exist."}
, {0x0000056cL, "ERROR_INVALID_MEMBER", "A new member could not be added to a local group because the member has the\nwrong account type."}
, {0x0000056dL, "ERROR_TOO_MANY_SIDS", "Too many security IDs have been specified."}
, {0x0000056eL, "ERROR_LM_CROSS_ENCRYPTION_REQUIRED", "A cross-encrypted password is necessary to change this user password."}
, {0x0000056fL, "ERROR_NO_INHERITANCE", "Indicates an ACL contains no inheritable components"}
, {0x00000570L, "ERROR_FILE_CORRUPT", "The file or directory is corrupt and non-readable."}
, {0x00000571L, "ERROR_DISK_CORRUPT", "The disk structure is corrupt and non-readable."}
, {0x00000572L, "ERROR_NO_USER_SESSION_KEY", "There is no user session key for the specified logon session."}
, {0x00000573L, "ERROR_LICENSE_QUOTA_EXCEEDED", "The service being accessed is licensed for a particular number of connections.\nNo more connections can be made to the service at this time\nbecause there are already as many connections as the service can accept."}
, {0x00000578L, "ERROR_INVALID_WINDOW_HANDLE", "Invalid window handle."}
, {0x00000579L, "ERROR_INVALID_MENU_HANDLE", "Invalid menu handle."}
, {0x0000057aL, "ERROR_INVALID_CURSOR_HANDLE", "Invalid cursor handle."}
, {0x0000057bL, "ERROR_INVALID_ACCEL_HANDLE", "Invalid accelerator table handle."}
, {0x0000057cL, "ERROR_INVALID_HOOK_HANDLE", "Invalid hook handle."}
, {0x0000057dL, "ERROR_INVALID_DWP_HANDLE", "Invalid handle to a multiple-window position structure."}
, {0x0000057eL, "ERROR_TLW_WITH_WSCHILD", "Cannot create a top-level child window."}
, {0x0000057fL, "ERROR_CANNOT_FIND_WND_CLASS", "Cannot find window class."}
, {0x00000580L, "ERROR_WINDOW_OF_OTHER_THREAD", "Invalid window, belongs to other thread."}
, {0x00000581L, "ERROR_HOTKEY_ALREADY_REGISTERED", "Hot key is already registered."}
, {0x00000582L, "ERROR_CLASS_ALREADY_EXISTS", "Class already exists."}
, {0x00000583L, "ERROR_CLASS_DOES_NOT_EXIST", "Class does not exist."}
, {0x00000584L, "ERROR_CLASS_HAS_WINDOWS", "Class still has open windows."}
, {0x00000585L, "ERROR_INVALID_INDEX", "Invalid index."}
, {0x00000586L, "ERROR_INVALID_ICON_HANDLE", "Invalid icon handle."}
, {0x00000587L, "ERROR_PRIVATE_DIALOG_INDEX", "Using private DIALOG window words."}
, {0x00000588L, "ERROR_LISTBOX_ID_NOT_FOUND", "The listbox identifier was not found."}
, {0x00000589L, "ERROR_NO_WILDCARD_CHARACTERS", "No wildcards were found."}
, {0x0000058aL, "ERROR_CLIPBOARD_NOT_OPEN", "Thread does not have a clipboard open."}
, {0x0000058bL, "ERROR_HOTKEY_NOT_REGISTERED", "Hot key is not registered."}
, {0x0000058cL, "ERROR_WINDOW_NOT_DIALOG", "The window is not a valid dialog window."}
, {0x0000058dL, "ERROR_CONTROL_ID_NOT_FOUND", "Control ID not found."}
, {0x0000058eL, "ERROR_INVALID_COMBOBOX_MESSAGE", "Invalid message for a combo box because it does not have an edit control."}
, {0x0000058fL, "ERROR_WINDOW_NOT_COMBOBOX", "The window is not a combo box."}
, {0x00000590L, "ERROR_INVALID_EDIT_HEIGHT", "Height must be less than 256."}
, {0x00000591L, "ERROR_DC_NOT_FOUND", "Invalid device context (DC) handle."}
, {0x00000592L, "ERROR_INVALID_HOOK_FILTER", "Invalid hook procedure type."}
, {0x00000593L, "ERROR_INVALID_FILTER_PROC", "Invalid hook procedure."}
, {0x00000594L, "ERROR_HOOK_NEEDS_HMOD", "Cannot set non-local hook without a module handle."}
, {0x00000595L, "ERROR_GLOBAL_ONLY_HOOK", "This hook procedure can only be set globally."}
, {0x00000596L, "ERROR_JOURNAL_HOOK_SET", "The journal hook procedure is already installed."}
, {0x00000597L, "ERROR_HOOK_NOT_INSTALLED", "The hook procedure is not installed."}
, {0x00000598L, "ERROR_INVALID_LB_MESSAGE", "Invalid message for single-selection listbox."}
, {0x00000599L, "ERROR_SETCOUNT_ON_BAD_LB", "LB_SETCOUNT sent to non-lazy listbox."}
, {0x0000059aL, "ERROR_LB_WITHOUT_TABSTOPS", "This list box does not support tab stops."}
, {0x0000059bL, "ERROR_DESTROY_OBJECT_OF_OTHER_THREAD", "Cannot destroy object created by another thread."}
, {0x0000059cL, "ERROR_CHILD_WINDOW_MENU", "Child windows cannot have menus."}
, {0x0000059dL, "ERROR_NO_SYSTEM_MENU", "The window does not have a system menu."}
, {0x0000059eL, "ERROR_INVALID_MSGBOX_STYLE", "Invalid message box style."}
, {0x0000059fL, "ERROR_INVALID_SPI_VALUE", "Invalid system-wide (SPI_*) parameter."}
, {0x000005a0L, "ERROR_SCREEN_ALREADY_LOCKED", "Screen already locked."}
, {0x000005a1L, "ERROR_HWNDS_HAVE_DIFF_PARENT", "All handles to windows in a multiple-window position structure must\nhave the same parent."}
, {0x000005a2L, "ERROR_NOT_CHILD_WINDOW", "The window is not a child window."}
, {0x000005a3L, "ERROR_INVALID_GW_COMMAND", "Invalid GW_* command."}
, {0x000005a4L, "ERROR_INVALID_THREAD_ID", "Invalid thread identifier."}
, {0x000005a5L, "ERROR_NON_MDICHILD_WINDOW", "Cannot process a message from a window that is not a multiple document\ninterface (MDI) window."}
, {0x000005a6L, "ERROR_POPUP_ALREADY_ACTIVE", "Popup menu already active."}
, {0x000005a7L, "ERROR_NO_SCROLLBARS", "The window does not have scroll bars."}
, {0x000005a8L, "ERROR_INVALID_SCROLLBAR_RANGE", "Scroll bar range cannot be greater than 0x7FFF."}
, {0x000005a9L, "ERROR_INVALID_SHOWWIN_COMMAND", "Cannot show or remove the window in the way specified."}
, {0x000005aaL, "ERROR_NO_SYSTEM_RESOURCES", "Insufficient system resources exist to complete the requested service."}
, {0x000005abL, "ERROR_NONPAGED_SYSTEM_RESOURCES", "Insufficient system resources exist to complete the requested service."}
, {0x000005acL, "ERROR_PAGED_SYSTEM_RESOURCES", "Insufficient system resources exist to complete the requested service."}
, {0x000005adL, "ERROR_WORKING_SET_QUOTA", "Insufficient quota to complete the requested service."}
, {0x000005aeL, "ERROR_PAGEFILE_QUOTA", "Insufficient quota to complete the requested service."}
, {0x000005afL, "ERROR_COMMITMENT_LIMIT", "The paging file is too small for this operation to complete."}
, {0x000005b0L, "ERROR_MENU_ITEM_NOT_FOUND", "A menu item was not found."}
, {0x000005b1L, "ERROR_INVALID_KEYBOARD_HANDLE", "Invalid keyboard layout handle."}
, {0x000005b2L, "ERROR_HOOK_TYPE_NOT_ALLOWED", "Hook type not allowed."}
, {0x000005b3L, "ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION", "This operation requires an interactive windowstation."}
, {0x000005b4L, "ERROR_TIMEOUT", "This operation returned because the timeout period expired."}
, {0x000005b5L, "ERROR_INVALID_MONITOR_HANDLE", "Invalid monitor handle."}
, {0x000005dcL, "ERROR_EVENTLOG_FILE_CORRUPT", "The event log file is corrupt."}
, {0x000005ddL, "ERROR_EVENTLOG_CANT_START", "No event log file could be opened, so the event logging service did not start."}
, {0x000005deL, "ERROR_LOG_FILE_FULL", "The event log file is full."}
, {0x000005dfL, "ERROR_EVENTLOG_FILE_CHANGED", "The event log file has changed between reads."}
, {0x000006a4L, "RPC_S_INVALID_STRING_BINDING", "The string binding is invalid."}
, {0x000006a5L, "RPC_S_WRONG_KIND_OF_BINDING", "The binding handle is not the correct type."}
, {0x000006a6L, "RPC_S_INVALID_BINDING", "The binding handle is invalid."}
, {0x000006a7L, "RPC_S_PROTSEQ_NOT_SUPPORTED", "The RPC protocol sequence is not supported."}
, {0x000006a8L, "RPC_S_INVALID_RPC_PROTSEQ", "The RPC protocol sequence is invalid."}
, {0x000006a9L, "RPC_S_INVALID_STRING_UUID", "The string universal unique identifier (UUID) is invalid."}
, {0x000006aaL, "RPC_S_INVALID_ENDPOINT_FORMAT", "The endpoint format is invalid."}
, {0x000006abL, "RPC_S_INVALID_NET_ADDR", "The network address is invalid."}
, {0x000006acL, "RPC_S_NO_ENDPOINT_FOUND", "No endpoint was found."}
, {0x000006adL, "RPC_S_INVALID_TIMEOUT", "The timeout value is invalid."}
, {0x000006aeL, "RPC_S_OBJECT_NOT_FOUND", "The object universal unique identifier (UUID) was not found."}
, {0x000006afL, "RPC_S_ALREADY_REGISTERED", "The object universal unique identifier (UUID) has already been registered."}
, {0x000006b0L, "RPC_S_TYPE_ALREADY_REGISTERED", "The type universal unique identifier (UUID) has already been registered."}
, {0x000006b1L, "RPC_S_ALREADY_LISTENING", "The RPC server is already listening."}
, {0x000006b2L, "RPC_S_NO_PROTSEQS_REGISTERED", "No protocol sequences have been registered."}
, {0x000006b3L, "RPC_S_NOT_LISTENING", "The RPC server is not listening."}
, {0x000006b4L, "RPC_S_UNKNOWN_MGR_TYPE", "The manager type is unknown."}
, {0x000006b5L, "RPC_S_UNKNOWN_IF", "The interface is unknown."}
, {0x000006b6L, "RPC_S_NO_BINDINGS", "There are no bindings."}
, {0x000006b7L, "RPC_S_NO_PROTSEQS", "There are no protocol sequences."}
, {0x000006b8L, "RPC_S_CANT_CREATE_ENDPOINT", "The endpoint cannot be created."}
, {0x000006b9L, "RPC_S_OUT_OF_RESOURCES", "Not enough resources are available to complete this operation."}
, {0x000006baL, "RPC_S_SERVER_UNAVAILABLE", "The RPC server is unavailable."}
, {0x000006bbL, "RPC_S_SERVER_TOO_BUSY", "The RPC server is too busy to complete this operation."}
, {0x000006bcL, "RPC_S_INVALID_NETWORK_OPTIONS", "The network options are invalid."}
, {0x000006bdL, "RPC_S_NO_CALL_ACTIVE", "There is not a remote procedure call active in this thread."}
, {0x000006beL, "RPC_S_CALL_FAILED", "The remote procedure call failed."}
, {0x000006bfL, "RPC_S_CALL_FAILED_DNE", "The remote procedure call failed and did not execute."}
, {0x000006c0L, "RPC_S_PROTOCOL_ERROR", "A remote procedure call (RPC) protocol error occurred."}
, {0x000006c2L, "RPC_S_UNSUPPORTED_TRANS_SYN", "The transfer syntax is not supported by the RPC server."}
, {0x000006c4L, "RPC_S_UNSUPPORTED_TYPE", "The universal unique identifier (UUID) type is not supported."}
, {0x000006c5L, "RPC_S_INVALID_TAG", "The tag is invalid."}
, {0x000006c6L, "RPC_S_INVALID_BOUND", "The array bounds are invalid."}
, {0x000006c7L, "RPC_S_NO_ENTRY_NAME", "The binding does not contain an entry name."}
, {0x000006c8L, "RPC_S_INVALID_NAME_SYNTAX", "The name syntax is invalid."}
, {0x000006c9L, "RPC_S_UNSUPPORTED_NAME_SYNTAX", "The name syntax is not supported."}
, {0x000006cbL, "RPC_S_UUID_NO_ADDRESS", "No network address is available to use to construct a universal\nunique identifier (UUID)."}
, {0x000006ccL, "RPC_S_DUPLICATE_ENDPOINT", "The endpoint is a duplicate."}
, {0x000006cdL, "RPC_S_UNKNOWN_AUTHN_TYPE", "The authentication type is unknown."}
, {0x000006ceL, "RPC_S_MAX_CALLS_TOO_SMALL", "The maximum number of calls is too small."}
, {0x000006cfL, "RPC_S_STRING_TOO_LONG", "The string is too long."}
, {0x000006d0L, "RPC_S_PROTSEQ_NOT_FOUND", "The RPC protocol sequence was not found."}
, {0x000006d1L, "RPC_S_PROCNUM_OUT_OF_RANGE", "The procedure number is out of range."}
, {0x000006d2L, "RPC_S_BINDING_HAS_NO_AUTH", "The binding does not contain any authentication information."}
, {0x000006d3L, "RPC_S_UNKNOWN_AUTHN_SERVICE", "The authentication service is unknown."}
, {0x000006d4L, "RPC_S_UNKNOWN_AUTHN_LEVEL", "The authentication level is unknown."}
, {0x000006d5L, "RPC_S_INVALID_AUTH_IDENTITY", "The security context is invalid."}
, {0x000006d6L, "RPC_S_UNKNOWN_AUTHZ_SERVICE", "The authorization service is unknown."}
, {0x000006d7L, "EPT_S_INVALID_ENTRY", "The entry is invalid."}
, {0x000006d8L, "EPT_S_CANT_PERFORM_OP", "The server endpoint cannot perform the operation."}
, {0x000006d9L, "EPT_S_NOT_REGISTERED", "There are no more endpoints available from the endpoint mapper."}
, {0x000006daL, "RPC_S_NOTHING_TO_EXPORT", "No interfaces have been exported."}
, {0x000006dbL, "RPC_S_INCOMPLETE_NAME", "The entry name is incomplete."}
, {0x000006dcL, "RPC_S_INVALID_VERS_OPTION", "The version option is invalid."}
, {0x000006ddL, "RPC_S_NO_MORE_MEMBERS", "There are no more members."}
, {0x000006deL, "RPC_S_NOT_ALL_OBJS_UNEXPORTED", "There is nothing to unexport."}
, {0x000006dfL, "RPC_S_INTERFACE_NOT_FOUND", "The interface was not found."}
, {0x000006e0L, "RPC_S_ENTRY_ALREADY_EXISTS", "The entry already exists."}
, {0x000006e1L, "RPC_S_ENTRY_NOT_FOUND", "The entry is not found."}
, {0x000006e2L, "RPC_S_NAME_SERVICE_UNAVAILABLE", "The name service is unavailable."}
, {0x000006e3L, "RPC_S_INVALID_NAF_ID", "The network address family is invalid."}
, {0x000006e4L, "RPC_S_CANNOT_SUPPORT", "The requested operation is not supported."}
, {0x000006e5L, "RPC_S_NO_CONTEXT_AVAILABLE", "No security context is available to allow impersonation."}
, {0x000006e6L, "RPC_S_INTERNAL_ERROR", "An internal error occurred in a remote procedure call (RPC)."}
, {0x000006e7L, "RPC_S_ZERO_DIVIDE", "The RPC server attempted an integer division by zero."}
, {0x000006e8L, "RPC_S_ADDRESS_ERROR", "An addressing error occurred in the RPC server."}
, {0x000006e9L, "RPC_S_FP_DIV_ZERO", "A floating-point operation at the RPC server caused a division by zero."}
, {0x000006eaL, "RPC_S_FP_UNDERFLOW", "A floating-point underflow occurred at the RPC server."}
, {0x000006ebL, "RPC_S_FP_OVERFLOW", "A floating-point overflow occurred at the RPC server."}
, {0x000006ecL, "RPC_X_NO_MORE_ENTRIES", "The list of RPC servers available for the binding of auto handles\nhas been exhausted."}
, {0x000006edL, "RPC_X_SS_CHAR_TRANS_OPEN_FAIL", "Unable to open the character translation table file."}
, {0x000006eeL, "RPC_X_SS_CHAR_TRANS_SHORT_FILE", "The file containing the character translation table has fewer than\n512 bytes."}
, {0x000006efL, "RPC_X_SS_IN_NULL_CONTEXT", "A null context handle was passed from the client to the host during\na remote procedure call."}
, {0x000006f1L, "RPC_X_SS_CONTEXT_DAMAGED", "The context handle changed during a remote procedure call."}
, {0x000006f2L, "RPC_X_SS_HANDLES_MISMATCH", "The binding handles passed to a remote procedure call do not match."}
, {0x000006f3L, "RPC_X_SS_CANNOT_GET_CALL_HANDLE", "The stub is unable to get the remote procedure call handle."}
, {0x000006f4L, "RPC_X_NULL_REF_POINTER", "A null reference pointer was passed to the stub."}
, {0x000006f5L, "RPC_X_ENUM_VALUE_OUT_OF_RANGE", "The enumeration value is out of range."}
, {0x000006f6L, "RPC_X_BYTE_COUNT_TOO_SMALL", "The byte count is too small."}
, {0x000006f7L, "RPC_X_BAD_STUB_DATA", "The stub received bad data."}
, {0x000006f8L, "ERROR_INVALID_USER_BUFFER", "The supplied user buffer is not valid for the requested operation."}
, {0x000006f9L, "ERROR_UNRECOGNIZED_MEDIA", "The disk media is not recognized.  It may not be formatted."}
, {0x000006faL, "ERROR_NO_TRUST_LSA_SECRET", "The workstation does not have a trust secret."}
, {0x000006fbL, "ERROR_NO_TRUST_SAM_ACCOUNT", "The SAM database on the Windows NT Server does not have a computer\naccount for this workstation trust relationship."}
, {0x000006fcL, "ERROR_TRUSTED_DOMAIN_FAILURE", "The trust relationship between the primary domain and the trusted\ndomain failed."}
, {0x000006fdL, "ERROR_TRUSTED_RELATIONSHIP_FAILURE", "The trust relationship between this workstation and the primary\ndomain failed."}
, {0x000006feL, "ERROR_TRUST_FAILURE", "The network logon failed."}
, {0x000006ffL, "RPC_S_CALL_IN_PROGRESS", "A remote procedure call is already in progress for this thread."}
, {0x00000700L, "ERROR_NETLOGON_NOT_STARTED", "An attempt was made to logon, but the network logon service was not started."}
, {0x00000701L, "ERROR_ACCOUNT_EXPIRED", "The user's account has expired."}
, {0x00000702L, "ERROR_REDIRECTOR_HAS_OPEN_HANDLES", "The redirector is in use and cannot be unloaded."}
, {0x00000703L, "ERROR_PRINTER_DRIVER_ALREADY_INSTALLED", "The specified printer driver is already installed."}
, {0x00000704L, "ERROR_UNKNOWN_PORT", "The specified port is unknown."}
, {0x00000705L, "ERROR_UNKNOWN_PRINTER_DRIVER", "The printer driver is unknown."}
, {0x00000706L, "ERROR_UNKNOWN_PRINTPROCESSOR", "The print processor is unknown."}
, {0x00000707L, "ERROR_INVALID_SEPARATOR_FILE", "The specified separator file is invalid."}
, {0x00000708L, "ERROR_INVALID_PRIORITY", "The specified priority is invalid."}
, {0x00000709L, "ERROR_INVALID_PRINTER_NAME", "The printer name is invalid."}
, {0x0000070aL, "ERROR_PRINTER_ALREADY_EXISTS", "The printer already exists."}
, {0x0000070bL, "ERROR_INVALID_PRINTER_COMMAND", "The printer command is invalid."}
, {0x0000070cL, "ERROR_INVALID_DATATYPE", "The specified datatype is invalid."}
, {0x0000070dL, "ERROR_INVALID_ENVIRONMENT", "The Environment specified is invalid."}
, {0x0000070eL, "RPC_S_NO_MORE_BINDINGS", "There are no more bindings."}
, {0x0000070fL, "ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT", "The account used is an interdomain trust account.  Use your global user account or local user account to access this server."}
, {0x00000710L, "ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT", "The account used is a Computer Account.  Use your global user account or local user account to access this server."}
, {0x00000711L, "ERROR_NOLOGON_SERVER_TRUST_ACCOUNT", "The account used is an server trust account.  Use your global user account or local user account to access this server."}
, {0x00000712L, "ERROR_DOMAIN_TRUST_INCONSISTENT", "The name or security ID (SID) of the domain specified is inconsistent\nwith the trust information for that domain."}
, {0x00000713L, "ERROR_SERVER_HAS_OPEN_HANDLES", "The server is in use and cannot be unloaded."}
, {0x00000714L, "ERROR_RESOURCE_DATA_NOT_FOUND", "The specified image file did not contain a resource section."}
, {0x00000715L, "ERROR_RESOURCE_TYPE_NOT_FOUND", "The specified resource type can not be found in the image file."}
, {0x00000716L, "ERROR_RESOURCE_NAME_NOT_FOUND", "The specified resource name can not be found in the image file."}
, {0x00000717L, "ERROR_RESOURCE_LANG_NOT_FOUND", "The specified resource language ID cannot be found in the image file."}
, {0x00000718L, "ERROR_NOT_ENOUGH_QUOTA", "Not enough quota is available to process this command."}
, {0x00000719L, "RPC_S_NO_INTERFACES", "No interfaces have been registered."}
, {0x0000071aL, "RPC_S_CALL_CANCELLED", "The server was altered while processing this call."}
, {0x0000071bL, "RPC_S_BINDING_INCOMPLETE", "The binding handle does not contain all required information."}
, {0x0000071cL, "RPC_S_COMM_FAILURE", "Communications failure."}
, {0x0000071dL, "RPC_S_UNSUPPORTED_AUTHN_LEVEL", "The requested authentication level is not supported."}
, {0x0000071eL, "RPC_S_NO_PRINC_NAME", "No principal name registered."}
, {0x0000071fL, "RPC_S_NOT_RPC_ERROR", "The error specified is not a valid Windows NT RPC error code."}
, {0x00000720L, "RPC_S_UUID_LOCAL_ONLY", "A UUID that is valid only on this computer has been allocated."}
, {0x00000721L, "RPC_S_SEC_PKG_ERROR", "A security package specific error occurred."}
, {0x00000722L, "RPC_S_NOT_CANCELLED", "Thread is not cancelled."}
, {0x00000723L, "RPC_X_INVALID_ES_ACTION", "Invalid operation on the encoding/decoding handle."}
, {0x00000724L, "RPC_X_WRONG_ES_VERSION", "Incompatible version of the serializing package."}
, {0x00000725L, "RPC_X_WRONG_STUB_VERSION", "Incompatible version of the RPC stub."}
, {0x00000726L, "RPC_X_INVALID_PIPE_OBJECT", "The idl pipe object is invalid or corrupted."}
, {0x00000727L, "RPC_X_INVALID_PIPE_OPERATION", "The operation is invalid for a given idl pipe object."}
, {0x00000728L, "RPC_X_WRONG_PIPE_VERSION", "The idl pipe version is not supported."}
, {0x0000076aL, "RPC_S_GROUP_MEMBER_NOT_FOUND", "The group member was not found."}
, {0x0000076bL, "EPT_S_CANT_CREATE", "The endpoint mapper database could not be created."}
, {0x0000076cL, "RPC_S_INVALID_OBJECT", "The object universal unique identifier (UUID) is the nil UUID."}
, {0x0000076dL, "ERROR_INVALID_TIME", "The specified time is invalid."}
, {0x0000076eL, "ERROR_INVALID_FORM_NAME", "The specified Form name is invalid."}
, {0x0000076fL, "ERROR_INVALID_FORM_SIZE", "The specified Form size is invalid"}
, {0x00000770L, "ERROR_ALREADY_WAITING", "The specified Printer handle is already being waited on"}
, {0x00000771L, "ERROR_PRINTER_DELETED", "The specified Printer has been deleted"}
, {0x00000772L, "ERROR_INVALID_PRINTER_STATE", "The state of the Printer is invalid"}
, {0x00000773L, "ERROR_PASSWORD_MUST_CHANGE", "The user must change his password before he logs on the first time."}
, {0x00000774L, "ERROR_DOMAIN_CONTROLLER_NOT_FOUND", "Could not find the domain controller for this domain."}
, {0x00000775L, "ERROR_ACCOUNT_LOCKED_OUT", "The referenced account is currently locked out and may not be logged on to."}
, {0x00000776L, "OR_INVALID_OXID", "The object exporter specified was not found."}
, {0x00000777L, "OR_INVALID_OID", "The object specified was not found."}
, {0x00000778L, "OR_INVALID_SET", "The object resolver set specified was not found."}
, {0x00000779L, "RPC_S_SEND_INCOMPLETE", "Some data remains to be sent in the request buffer."}
, {0x000017e6L, "ERROR_NO_BROWSER_SERVERS_FOUND", "The list of servers for this workgroup is not currently available"}
, {0x000007d0L, "ERROR_INVALID_PIXEL_FORMAT", "The pixel format is invalid."}
, {0x000007d1L, "ERROR_BAD_DRIVER", "The specified driver is invalid."}
, {0x000007d2L, "ERROR_INVALID_WINDOW_STYLE", "The window style or class attribute is invalid for this operation."}
, {0x000007d3L, "ERROR_METAFILE_NOT_SUPPORTED", "The requested metafile operation is not supported."}
, {0x000007d4L, "ERROR_TRANSFORM_NOT_SUPPORTED", "The requested transformation operation is not supported."}
, {0x000007d5L, "ERROR_CLIPPING_NOT_SUPPORTED", "The requested clipping operation is not supported."}
, {0x00000bb8L, "ERROR_UNKNOWN_PRINT_MONITOR", "The specified print monitor is unknown."}
, {0x00000bb9L, "ERROR_PRINTER_DRIVER_IN_USE", "The specified printer driver is currently in use."}
, {0x00000bbaL, "ERROR_SPOOL_FILE_NOT_FOUND", "The spool file was not found."}
, {0x00000bbbL, "ERROR_SPL_NO_STARTDOC", "A StartDocPrinter call was not issued."}
, {0x00000bbcL, "ERROR_SPL_NO_ADDJOB", "An AddJob call was not issued."}
, {0x00000bbdL, "ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED", "The specified print processor has already been installed."}
, {0x00000bbeL, "ERROR_PRINT_MONITOR_ALREADY_INSTALLED", "The specified print monitor has already been installed."}
, {0x00000bbfL, "ERROR_INVALID_PRINT_MONITOR", "The specified print monitor does not have the required functions."}
, {0x00000bc0L, "ERROR_PRINT_MONITOR_IN_USE", "The specified print monitor is currently in use."}
, {0x00000bc1L, "ERROR_PRINTER_HAS_JOBS_QUEUED", "The requested operation is not allowed when there are jobs queued to the printer."}
, {0x00000bc2L, "ERROR_SUCCESS_REBOOT_REQUIRED", "The requested operation is successful.  Changes will not be effective until the system is rebooted."}
, {0x00000bc3L, "ERROR_SUCCESS_RESTART_REQUIRED", "The requested operation is successful.  Changes will not be effective until the service is restarted."}
, {0x00000fa0L, "ERROR_WINS_INTERNAL", "WINS encountered an error while processing the command."}
, {0x00000fa1L, "ERROR_CAN_NOT_DEL_LOCAL_WINS", "The local WINS can not be deleted."}
, {0x00000fa2L, "ERROR_STATIC_INIT", "The importation from the file failed."}
, {0x00000fa3L, "ERROR_INC_BACKUP", "The backup Failed.  Was a full backup done before ?"}
, {0x00000fa4L, "ERROR_FULL_BACKUP", "The backup Failed.  Check the directory that you are backing the database to."}
, {0x00000fa5L, "ERROR_REC_NON_EXISTENT", "The name does not exist in the WINS database."}
, {0x00000fa6L, "ERROR_RPL_NOT_ALLOWED", "Replication with a non-configured partner is not allowed."}
, {0x00001004L, "ERROR_DHCP_ADDRESS_CONFLICT", "The DHCP client has obtained an IP address that is already in use on the network.  The local interface will be disabled until the DHCP client can obtain a new address."}
, {0xFFFFFFFF, "_END_", "_END_"}
};
