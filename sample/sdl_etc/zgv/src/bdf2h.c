/* Zgv v5.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * bdf2h.c - convert BDF font file to header for inclusion in zgv.
 *
 * note that this hardly does any error-checking or anything,
 * and may not necessary work for all fonts (though in practice,
 * it should work for all sane ones which define all the ASCII
 * chars).
 *
 * You're welcome to nick this and font.c for other programs etc.
 * (A load-font-at-runtime version should be possible too, BTW.)
 * Note that (IIRC) BDF fonts aren't distributed in binary releases of
 * XFree86, and you'll have to get a copy of the source to get them.
 * Getting a copy on CD is probably the best idea - the source is fairly
 * massive, even compressed.
 *
 * FWIW, the fonts used in zgv are from xc/fonts/bdf/100dpi.
 * (Yes, 75dpi would make more sense, but these ones just fitted better.)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int main(int argc, char* argv[])
{
	FILE* in = stdin;
	static char buf[128];
	int f, g;
	int c = 0, w = 0, i, mask;
	int fw, fh, fox, foy, width, height, x, y, wcount;
	int maxy = -999;
	int ofs = 0;
	int ofstbl[128];
	char* fontdesc = "unnamed";

	if (argc == 2) fontdesc = argv[1];

	/* we explicitly specify `signed char' where it needs to be.
	 * I guess it doesn't matter 'cos it'll always be compiled on
	 * a compiler where signed is the default, but what the hell.
	 */
	printf("#ifndef DEFINED_STRUCT_FONTINFO_TAG\n");
	printf("#define DEFINED_STRUCT_FONTINFO_TAG 1\n");
	printf(
		"struct fontinfo_tag\n"
		"  {\n"
		"  signed char *data;\n"
		"  int table[96];\n"
		"  int yofs,fh,oy;\n"
		"  };\n"
	);
	printf("#endif\n\n");

	/* generate font data */
	printf("static signed char font%sdat[]={\n\n", fontdesc);

	printf("/* data for each char is ox oy w h dwidth, then data */\n\n");

	while (fgets(buf, sizeof(buf), in) != NULL)
	{
		if (strncmp(buf, "FONTBOUNDINGBOX ", 16) == 0)
			sscanf(buf + 16, "%d %d %d %d", &fw, &fh, &fox, &foy);
		else if (strncmp(buf, "ENCODING ", 9) == 0)
			c = atoi(buf + 9);
		else if (strncmp(buf, "DWIDTH ", 7) == 0)
			w = atoi(buf + 7);
		else if (strncmp(buf, "BBX ", 4) == 0)
		{
			sscanf(buf + 4, "%d %d %d %d", &width, &height, &x, &y);
		}
		else if (strcmp(buf, "BITMAP\n") == 0)
		{
			if (c < 32 || c>127) continue;
			ofstbl[c] = ofs;
			printf("/* `%c' */\n", c);
			printf("%d,%d,%d,%d,%d,\n", x, y, width, height, w);
			ofs += 5;
			if (y + height - foy > maxy) maxy = y + height - foy;
			while (fgets(buf, sizeof(buf), in) != NULL && strcmp(buf, "ENDCHAR\n") != 0)
			{
				i = 0;
				wcount = width;
				for (f = 0; f < strlen(buf) - 1; f++)
				{
					c = toupper(buf[f]) - 48; if (c > 9) c -= 7;
					if (c < 0 || c>15)
						printf("error in font - bad hex!\n"), exit(1);
					else
					{
						i = i * 16 + c;
						if (f & 1)
						{

							for (g = 0, mask = 128; g < 8 && wcount--; g++, ofs++, mask >>= 1)
								printf("%c", (i & mask) ? '1' : '0'), printf(",");
							i = 0;
						}
					}
				}
				if (f & 1)
				{
					for (g = 0, mask = 128; g < 8 && wcount--; g++, ofs++, mask >>= 1)
						printf("%c", (i & mask) ? '1' : '0'), printf(",");
				}
				printf("\n");
			}
			printf("\n");
		}
	}

	printf("};\n\n");

	printf("static struct fontinfo_tag font%s={\n", fontdesc);
	printf("font%sdat,\n{", fontdesc);
	/* lookup table for each char (32..127) */
	ofstbl[127] = 0;		/* well, ok, no char 127 :-) */
	for (f = 32; f < 128; f++)
	{
		if ((f & 7) == 0) printf("\n");
		printf("%d,", ofstbl[f]);
	}
	printf("},\n");

	/* yofs, fh, oy */
	printf("%d, %d, %d\n};\n\n\n", fh - maxy, fh, foy);

	exit(0);
}
