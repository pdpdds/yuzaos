#include <windef.h>
#include <math.h>
#include <stringdef.h>
#include <stdint.h>

#define FLAG_IS_SET(x)	\
	((flags & (1 << (strchr(kFlagCharacters, x) - kFlagCharacters))) != 0)

#define SET_FLAG(x)	\
	flags |= 1 << (strchr(kFlagCharacters, x) - kFlagCharacters)

#define PREFIX_IS_SET(x)	\
	((prefixes & (1 << (strchr(kPrefixCharacters, x) - kPrefixCharacters))) != 0)

#define SET_PREFIX(x) \
	prefixes |= 1 << (strchr(kPrefixCharacters, x) - kPrefixCharacters)

const char *kHexDigits = "0123456789abcdef";
const char *kFlagCharacters = "-+ 0";
const char *kPrefixCharacters = "FNhlL";

/*
 *	 % flags width .precision prefix format
 */
int vsnprintf(char *out, size_t size, const char *format, va_list args)
{
	int flags = 0;
	int prefixes = 0;
	int width = 0;
	int precision = 0;
	
	enum {
		kScanText,
		kScanFlags,
		kScanWidth,
		kScanPrecision,
		kScanPrefix,
		kScanFormat
	} state = kScanText;

	while (*format && size > 0) {
		switch (state) {
			case kScanText:
				if (*format == '%') {
					format++;
					state = kScanFlags;
					flags = 0;				/* reset attributes */
					prefixes = 0;
					width = 0;
					precision = 0;
				} else {
					size--;
					*out++ = *format++;
				}
				
				break;
				
			case kScanFlags: {
				const char *c;
				
				if (*format == '%') {
					*out++ = *format++;
					state = kScanText;
					break;
				}
			
				c = strchr(kFlagCharacters, *format);
				if (c) {
					SET_FLAG(*format);
					format++;
				} else
					state = kScanWidth;
				
				break;
			}

			case kScanWidth:
				if (isdigit(*format))
					width = width * 10 + *format++ - '0';
				else if (*format == '.') {
					state = kScanPrecision;
					format++;
				} else
					state = kScanPrefix;
					
				break;
				
			case kScanPrecision:
				if (isdigit(*format))
					precision = precision * 10 + *format++ - '0';
				else
					state = kScanPrefix;
					
				break;
				
			case kScanPrefix: {
				char *c = strchr(kPrefixCharacters, *format);
				if (c) {
					SET_PREFIX(*format);
					format++;
				} else
					state = kScanFormat;

				break;
			}

			case kScanFormat: {
				char temp_string[64];
				
				char pad_char;
				int pad_count;
				int radix = 10;
			
				switch (*format) {
					case 'p':	/* pointer */
						width = 8;
						SET_FLAG('0');
						 
						/* falls through */

					case 'x':
					case 'X':	/* unsigned hex */
					case 'o':	/* octal */
					case 'u':	/* Unsigned decimal */
					case 'd':
					case 'i':	{ /* Signed decimal */
						int is_really_long = PREFIX_IS_SET('L');
						uint64 value;
						if (is_really_long)
							value = va_arg(args, uint64); /* extra long */
						else
							value = va_arg(args, unsigned);		/* long */

						/* figure out base */
						if (*format == 'o')
							radix = 8;
						else if (*format == 'x' || *format == 'X' || *format == 'p')
							radix = 16;
						else
							radix = 10;

						/* handle sign */
						if ((*format == 'd' || *format == 'i')) {
							if (!is_really_long && (long) value < 0) {
								value = (unsigned) (- (long) value);
								if (size > 0) {
									size--;
									*out++ = '-';
								}
							} else if (is_really_long && (int64) value < 0) {
								value = - (int64) value;
								if (size > 0) {
									size--;
									*out++ = '-';
								}
							}
						}

						/* write out the string backwards */
						int index = 63;
						for (;;) {
							temp_string[index] = kHexDigits[value % radix];
							value /= radix;
							if (value == 0)
								break;
						
							if (index == 0)
								break;
								
							index--;
						}

						/* figure out pad char */						
						if (FLAG_IS_SET('0'))
							pad_char = '0';
						else
							pad_char = ' ';

						/* write padding */						
						for (pad_count = width - (64 - index); pad_count > 0 && size > 0;
							pad_count--) {
							*out++ = pad_char;
							size--;
						}

						/* write the string */
						while (index < 64 && size > 0) {
							size--;
							*out++ = temp_string[index++];
						}
				
						break;
					}


					case 'c':	/* Single character */
						*out++ = va_arg(args, char);
						size--;
						break;
				
					case 's': {	/* string */
						int index = 0;
						int max_width;
						char *c = va_arg(args, char*);
						
						if (precision == 0)
							max_width = size;
						else
							max_width = MIN(precision, size);

						for (index = 0; index < max_width && *c; index++)
							*out++ = *c++;

						while (index < MIN(width, max_width)) {
							*out++ = ' ';
							index++;
						}

						size -= index;
						break;
					}
						
					case 'n':	/* character count */
						break;

					case 'f':	/*	fixed point float */
					{
						int index = 0;
						double double_temp;
						double_temp = va_arg(args, double);
						char buffer[512];
						ftoa_fixed(buffer, double_temp);

						/* write the string */
						while (index < strlen(buffer)) {
							size--;
							*out++ = buffer[index++];
						}
	
						break;
					}

					case 'Q':
					{
						int index = 0;
						__int64 int64_temp;
						int64_temp = va_arg(args, __int64);
						char buffer[20];
						_i64toa(int64_temp, buffer, 10);
						/* write the string */
						while (index < strlen(buffer)) {
							size--;
							*out++ = buffer[index++];
						}

						break;
					}

					case 'q':
					{
						int index = 0;
						uint64_t int64_temp;
						int64_temp = va_arg(args, uint64_t);
						char buffer[20];
						_i64toa(int64_temp, buffer, 16);
						/* write the string */
						while (index < strlen(buffer)) {
							size--;
							*out++ = buffer[index++];
						}

						break;
					}
					
					case 'e':
					case 'E':	/* scientific notation */
					case 'g':
					case 'G':	/* e or f, whichever is shorter */
						break;
				}
				
				format++;
				state = kScanText;
				break;				
			}
		}
	}
	
	*out = 0;

	return strlen(out);
}

int snprintf(char *out, int size, const char *fmt, ...)
{
	va_list arglist;

	va_start(arglist, fmt);
	vsnprintf(out, size, fmt, arglist);
	va_end(arglist);
	int len = strlen(out);
	return len;
}

