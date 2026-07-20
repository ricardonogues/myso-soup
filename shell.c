#include "user.h"

void main(void) {
  while (1) {
  prompt:
    printf("> ");
    char cmdline[128];
    for (int i = 0;; i++) {
      char ch = getchar();
      putchar(ch);
      if (i == sizeof(cmdline)) {
        printf("command line too long\n");
        goto prompt;
      } else if (ch == '\r') {
        printf("\n");
        cmdline[i] = '\0';
        break;
      } else {
        cmdline[i] = ch;
      }
    }

    if (!strcmp(cmdline, "hello")) {
      printf("Hello world from the shell\n");
    } else if (!strcmp(cmdline, "exit")) {
      exit();
    } else {
      printf("unknown command: %s\n", cmdline);
    }
  }
}
