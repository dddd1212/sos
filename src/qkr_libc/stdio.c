#include "stdio.h"
#include "string.h"
uint32 vgprintf(WriteCharFunc f, void * ctx, int8 * fmt, va_list ap) {
	uint32 write_count = 0;
	char num_arr[21]; // max uint64 base10 size + null.
	int num_arr_idx = 0;
	int base = 0;
	int32 padding = 0;
	int8 * orig;
	while (*fmt != '\x00') { // go over the format
		if (*fmt == '%') { // special character:
			orig = fmt;
			fmt++;
			if (*fmt == '%') { // handle the %% case:
				if (f(ctx, '%') == FALSE) { // write char
					return write_count;
				}
				write_count++;
				fmt++;
				continue;
			}

			if (*fmt == '0') { // handle wanted padding. For now we support just one character of padding.
				fmt++;
				padding = *fmt - '0';
				fmt++;
			}
			else {
				padding = 0;
			}

			if (*fmt == 's') { // handle string
				char * str = va_arg(ap, char*);
				int32 str_size = (int32)strlen(str);
				// pad with spaces:
				for (int32 idx = 0; idx < padding - str_size; idx++) {
					if (f(ctx, ' ') == FALSE) { // write char
						return write_count;
					}
					write_count++;
				}
				// write the string:
				while (*str != '\x00') {
					if (f(ctx, *str) == FALSE) { // write char
						return write_count;
					}
					str++;
					write_count++;
				}
				fmt++;
				continue;
			}
			else if (*fmt == 'd') { // handle integer
				base = 10;
			}
			else if (*fmt == 'X' || *fmt == 'x') { // handle base16
				base = 16;
			}
			if (base) {
				uint64 param = va_arg(ap, uint64);

				num_arr_idx = 20; // start from the end
				num_arr[num_arr_idx] = '\x00';
				num_arr_idx--;
				do {
					int digit = param % base;
					if (digit < 10) {
						num_arr[num_arr_idx] = '0' + digit;
					}
					else {
						if (*fmt == 'X') num_arr[num_arr_idx] = 'A' - 10 + digit;
						else num_arr[num_arr_idx] = 'a' - 10 + digit;
					}
					num_arr_idx--;
					padding--;
					param /= base;
				} while (param != 0);
				base = 0;
				// Write the padding:
				for (int32 p = 0; p < padding; p++) {
					if (f(ctx, '0') == FALSE) { // write char
						return write_count;
					}
					write_count++;
				}

				//  Write the number:
				for (uint32 p = num_arr_idx + 1; num_arr[p] != '\x00'; p++) {
					if (f(ctx, num_arr[p]) == FALSE) { // write char
						return write_count;
					}
					write_count++;
				}
				fmt++;
				continue;
			}
			// If we got to here that means that we can't handle the %:
			fmt = orig;
			// handle it as regular case.
		} // from here its the regular case:

		if (f(ctx, *fmt) == FALSE) { // write char
			return write_count;
		}
		write_count++;
		fmt++;
		continue;
	}
	return write_count;
}

BOOL write_char_to_ptr(void * ctx, int8 c) {
	int8 ** addr = (int8 **)ctx;
	**addr = c;
	(*addr)++;
	return TRUE;
}

int sprintf(int8 * out, int8 * fmt, ...) {
	va_list ap;
	va_start(ap, 0);
	uint32 count = vgprintf(write_char_to_ptr, &out, fmt, ap);
	va_end(ap);
	out[count] = '\x00';
	return count;
}
