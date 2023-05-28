#include <string.h>

int utf8_char_size(char c) {
  // TODO: any more efficient way?
  if ((c & 0x80) == 0x00) return 1;
  else if ((c & 0xe0) == 0xc0) return 2;
  else if ((c & 0xf0) == 0xe0) return 3;
  else if ((c & 0xf8) == 0xf0) return 4;
  else return -1;
}

int utf8_char_equ(char *c1, char *c2, unsigned c2_size) {
  int c1_size = utf8_char_size(*c1);
  if (c1_size == -1 || c1_size != c2_size) return 0;
  return memcmp(c1, c2, c2_size) == 0;
}
