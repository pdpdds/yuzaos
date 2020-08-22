#define MAX_FLOAT_SIZE 320
#include <stdio.h>
#include <ctype.h>

int vfscanf(FILE* stream, const char* format, va_list args)
{
	int suppress = 0;		/* TRUE means scan value but don't actual
							* modify an element of args. */
	int storeShort = 0;		/* TRUE means store a short value. */
	int storeLong = 0;		/* TRUE means store a long value. */
	int width = 0;			/* Field width. */
	register char formatChar = 0; 	/* Current character from format string.
									* Eventually it ends up holding the format
									* type (e.g. 'd' for decimal). */
	register int streamChar = 0;	/* Next character from stream. */
	int assignedFields = 0;		/* Counts number of successfully-assigned
								* fields. */
	int base = 0;			/* Gives base for numbers:  0 means float,
							* -1 means not a number.
							*/
	int sign = 0;			/* TRUE means negative sign. */
	char buf[MAX_FLOAT_SIZE + 1];
	/* Place to accumulate floating-point
	* number for processing. */
	register char *ptr = (char *)0;
	char *savedPtr = 0, *end = 0, *firstPtr = 0;

	assignedFields = 0;
	streamChar = fgetc(stream);
	if (streamChar == EOF) {
		return(EOF);
	}

	/*
	* The main loop is to scan through the characters in format.
	* Anything but a '%' must match the next character from stream.
	* A '%' signals the start of a format field;  the formatting
	* information is parsed, the next value is scanned from the stream
	* and placed in memory, and the loop goes on.
	*/

	for (formatChar = *format; (formatChar != 0) && (streamChar != EOF);
		format++, formatChar = *format) {

		/*
		* A white-space format character matches any number of
		* white-space characters from the stream.
		*/

		if (isspace(formatChar)) {
			while (isspace(streamChar)) {
				streamChar = fgetc(stream);
			}
			continue;
		}

		/*
		* Any character but % must be matched exactly by the stream.
		*/

		if (formatChar != '%') {
			if (streamChar != formatChar) {
				break;
			}
			streamChar = fgetc(stream);
			continue;
		}

		/*
		* Parse off the format control fields.
		*/

		suppress = false;
		storeLong = false;
		storeShort = false;
		width = -1;
		format++;
		formatChar = *format;
		if (formatChar == '*') {
			suppress = true;
			format++;
			formatChar = *format;
		}
		if (isdigit(formatChar)) {
			width = strtoul(format, &end, 10);
			format = end;
			formatChar = *format;
		}
		if (formatChar == 'l') {
			storeLong = true;
			format++;
			formatChar = *format;
		}
		if (formatChar == 'h') {
			storeShort = true;
			format++;
			formatChar = *format;
		}

		/*
		* Skip any leading blanks in the input (except for 'c' format).
		* Also, default the width to infinity, except for 'c' format.
		*/

		if ((formatChar != 'c') && (formatChar != '[')) {
			while (isspace(streamChar)) {
				streamChar = fgetc(stream);
			}
		}
		if ((width <= 0) && (formatChar != 'c')) {
			width = 1000000;
		}

		/*
		* Check for EOF again after parsing away the white space.
		*/
		if (streamChar == EOF) {
			break;
		}

		/*
		* Process the conversion character.  For numbers, this just means
		* turning it into a "base" number that indicates how to read in
		* a number.
		*/

		base = -1;
		switch (formatChar) {

		case '%':
			if (streamChar != '%') {
				goto done;
			}
			streamChar = fgetc(stream);
			break;

		case 'D':
			storeShort = false;
		case 'd':
			base = 10;
			break;

		case 'O':
			storeShort = false;
		case 'o':
			base = 8;
			break;

		case 'X':
			storeShort = false;
		case 'x':
			base = 16;
			break;

		case 'E':
		case 'F':
			storeLong = true;
		case 'e':
		case 'f':
			base = 0;
			break;

			/*
			* Characters and strings are handled in exactly the same way,
			* except that for characters the default width is 1 and spaces
			* are not considered terminators.
			*/

		case 'c':
			if (width <= 0) {
				width = 1;
			}
		case 's':
			if (suppress) {
				while ((width > 0) && (streamChar != EOF)) {
					if (isspace(streamChar) && (formatChar == 's')) {
						break;
					}
					streamChar = fgetc(stream);
					width--;
				}
			}
			else {
				ptr = va_arg(args, char *);
				while ((width > 0) && (streamChar != EOF)) {
					if (isspace(streamChar) && (formatChar == 's')) {
						break;
					}
					*ptr = streamChar;
					ptr++;
					streamChar = fgetc(stream);
					width--;
				}
				if (formatChar == 's') {
					*ptr = 0;
				}
				assignedFields++;
			}
			break;

		case '[':
			format++; formatChar = *format;
			if (formatChar == '^') {
				format++;
			}
			if (!suppress) {
				firstPtr = ptr = va_arg(args, char *);
			}
			savedPtr = (char*)format;
			while ((width > 0) && (streamChar != EOF)) {
				format = savedPtr;
				while (true) {
					if (*format == streamChar) {
						if (formatChar == '^') {
							goto stringEnd;
						}
						else {
							break;
						}
					}
					if ((*format == ']') || (*format == 0)) {
						if (formatChar == '^') {
							break;
						}
						else {
							goto stringEnd;
						}
					}
					format++;
				}
				if (!suppress) {
					*ptr = streamChar;
					ptr++;
				}
				streamChar = fgetc(stream);
				width--;
			}
		stringEnd:
			if (ptr == firstPtr) {
				goto done;
			}
			while ((*format != ']') && (*format != 0)) {
				format++;
			}
			formatChar = *format;
			if (!suppress) {
				*ptr = 0;
				assignedFields++;
			}
			break;

			/*
			* Don't ask why, but for compatibility with UNIX, a null
			* conversion character must always return EOF, and any
			* other conversion character must be treated as decimal.
			*/

		case 0:
			ungetc(streamChar, stream);
			return(EOF);

		default:
			base = 10;
			break;
		}

		/*
		* If the field wasn't a number, then everything was handled
		* in the switch statement above.  Otherwise, we still have
		* to read in a number.  This gets handled differently for
		* integers and floating-point numbers.
		*/

		if (base < 0) {
			continue;
		}

		if (streamChar == '-') {
			sign = true;
			width -= 1;
			streamChar = fgetc(stream);
		}
		else {
			sign = false;
			if (streamChar == '+') {
				width -= 1;
				streamChar = fgetc(stream);
			}
		}

		/*
		* If we're supposed to be parsing a floating-point number, read
		* the digits into a temporary buffer and use the conversion library
		* routine to convert them.
		*/

#define COPYCHAR \
    *ptr = streamChar; ptr++; width--; streamChar = fgetc(stream);

		if (base == 0) {
			if (width > MAX_FLOAT_SIZE) {
				width = MAX_FLOAT_SIZE;
			}
			ptr = buf;
			while ((width > 0) && isdigit(streamChar)) {
				COPYCHAR;
			}
			if ((width > 0) && (streamChar == '.')) {
				COPYCHAR;
			}
			while ((width > 0) && isdigit(streamChar)) {
				COPYCHAR;
			}
			if ((width > 0) && ((streamChar == 'e') || (streamChar == 'E'))) {
				COPYCHAR;
				if ((width > 0) &&
					((streamChar == '+') || (streamChar == '-'))) {
					COPYCHAR;
				}
				while ((width > 0) && isdigit(streamChar)) {
					COPYCHAR;
				}
			}
			*ptr = 0;

			if (ptr == buf) {		/* Not a valid number. */
				goto done;
			}

			if (!suppress) {
				double d;
				d = atof(buf);
				if (sign) {
					d = -d;
				}
				if (storeLong) {
					*(va_arg(args, double *)) = d;
				}
				else {
					*(va_arg(args, float *)) = (float)d;
				}
				assignedFields++;
			}
		}
		else {
			/*
			* This is an integer.  Use special-purpose code for the
			* three supported bases in order to make it run fast.
			*/

			int i;
			int anyDigits;

			i = 0;
			anyDigits = false;
			if (base == 10) {
				while ((width > 0) && isdigit(streamChar)) {
					i = (i * 10) + (streamChar - '0');
					streamChar = fgetc(stream);
					anyDigits = true;
					width -= 1;
				}
			}
			else if (base == 8) {
				while ((width > 0) && (streamChar >= '0')
					&& (streamChar <= '7')) {
					i = (i << 3) + (streamChar - '0');
					streamChar = fgetc(stream);
					anyDigits = true;
					width -= 1;
				}
			}
			else {
				while (width > 0) {
					if (isdigit(streamChar)) {
						i = (i << 4) + (streamChar - '0');
					}
					else if ((streamChar >= 'a') && (streamChar <= 'f')) {
						i = (i << 4) + (streamChar + 10 - 'a');
					}
					else if ((streamChar >= 'A') && (streamChar <= 'F')) {
						i = (i << 4) + (streamChar + 10 - 'A');
					}
					else {
						break;
					}
					streamChar = fgetc(stream);
					anyDigits = true;
					width--;
				}
			}
			if (!anyDigits) {
				goto done;
			}
			if (sign) {
				i = -i;
			}
			if (!suppress) {
				if (storeShort) {
					*(va_arg(args, short *)) = i;
				}
				else {
					*(va_arg(args, int *)) = i;
				}
				assignedFields++;
			}
		}
	}

done:
	ungetc(streamChar, stream);
	if ((streamChar == EOF) && (assignedFields == 0)) {
		return(EOF);
	}
	return(assignedFields);
}