#include <stdio.h>
#include "string.h"
#include <memory.h>
#include "argtable3.h"

void mymain(int n, int e, int E, const char** strings, int nstrings)
{
	printf("option -n = %s\n", ((n) ? "YES" : "NO"));
	printf("option -e = %s\n", ((e) ? "YES" : "NO"));
	printf("option -E = %s\n", ((E) ? "YES" : "NO"));
	for (int i=  0; i < nstrings; i++)
		printf("%s\n", strings[i]);
	printf("\n");
}

int main(int argc, char** argv)
{
	/* Define the allowable command line options, collecting them in argtable[] */
	struct arg_lit* n = arg_lit0("n", NULL, "do not output the trailing newline");
	struct arg_lit* e = arg_lit0("e", NULL, "enable interpretation of the backslash-escaped characters listed below");
	struct arg_lit* E = arg_lit0("E", NULL, "disable interpretation of those sequences in <string>s");
	struct arg_lit* help = arg_lit0(NULL, "help", "print this help and exit");
	struct arg_lit* vers = arg_lit0(NULL, "version", "print version information and exit");
	struct arg_str* strs = arg_strn(NULL, NULL, "STRING", 0, argc + 2, NULL);
	struct arg_end* end = arg_end(20);
	void* argtable[] = { n, e, E, help, vers, strs, end };
	const char* progname = "echo";
	int exitcode = 0;
	int nerrors;

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) != 0)
	{
		/* NULL entries were detected, some allocations must have failed */
		printf("%s: insufficient memory\n", progname);
		exitcode = 1;
		goto exit;
	}

	/* Parse the command line as defined by argtable[] */
	nerrors = arg_parse(argc, argv, argtable);

	/* special case: '--help' takes precedence over error reporting */
	if (help->count > 0)
	{
		printf("Usage: %s", progname);
		arg_print_syntax(stdout, argtable, "\n");
		printf("Echo the STRINGs to standard output.\n\n");
		arg_print_glossary(stdout, argtable, "  %-10s %s\n");
		printf("\nWithout -E, the following sequences are recognized and interpolated:\n\n"
			"  \\NNN   the character whose ASCII code is NNN (octal)\n"
			"  \\\\     backslash\n"
			"  \\a     alert (BEL)\n"
			"  \\b     backspace\n"
			"  \\c     suppress trailing newline\n"
			"  \\f     form feed\n"
			"  \\n     new line\n"
			"  \\r     carriage return\n"
			"  \\t     horizontal tab\n"
			"  \\v     vertical tab\n\n"
			"Report bugs to <foo@bar>.\n");
		exitcode = 0;
		goto exit;
	}

	/* special case: '--version' takes precedence error reporting */
	if (vers->count > 0)
	{
		printf("'%s' example program for the \"argtable\" command line argument parser.\n", progname);
		printf("September 2003, Stewart Heitmann\n");
		exitcode = 0;
		goto exit;
	}

	/* If the parser returned any errors then display them and exit */
	if (nerrors > 0)
	{
		/* Display the error details contained in the arg_end struct.*/
		arg_print_errors(stdout, end, progname);
		printf("Try '%s --help' for more information.\n", progname);
		exitcode = 1;
		goto exit;
	}

	/* Command line parsing is complete, do the main processing */
	mymain(n->count, e->count, E->count, strs->sval, strs->count);

exit:
	/* deallocate each non-null entry in argtable[] */
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	return exitcode;
}
