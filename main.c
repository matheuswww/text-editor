#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios orig_termios;
 
void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  // Input flags configuration
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  /*
  * BRKINT: Prevents a SIGINT signal from being sent when a break condition is detected.
  * ICRNL: Disables the conversion of '\r' (carriage return) into '\n' (newline).
  * INPCK: Disables input parity checking.
  * ISTRIP: Prevents stripping the 8th bit of input, allowing full byte data.
  * IXON: Disables software flow control (XON/XOFF).
  */

  // Output flags configuration
  raw.c_oflag &= ~(OPOST);
  /*
  * OPOST: Disables output processing, such as converting '\n' into '\r\n'.
  */

  // Control flags configuration
  raw.c_cflag &= ~(CS8);
  /*
  * CS8: Disables the 8-bit per character setting.
  * However, this might be an issue, as CS8 is usually enabled to work with 8-bit data.
  */

  // Local flags configuration
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  /*
  * ECHO: Disables echoing of typed characters to the terminal.
  * ICANON: Disables canonical mode, allowing input to be read byte by byte.
  * IEXTEN: Disables extended input features, such as Ctrl-V (literal next).
  * ISIG: Disables the generation of signals like SIGINT (Ctrl-C) and SIGTSTP (Ctrl-Z).
  */

  // Control characters configuration
  raw.c_cc[VMIN] = 0;
  /*
  * VMIN: Sets the minimum number of bytes needed for a read operation to return.
  * Here, 0 means the read will return immediately, even if no data is available.
  */

  raw.c_cc[VTIME] = 10 * 4;
  /*
  * VTIME: Sets the maximum wait time (in tenths of a second) for input data.
  * Here, it is set to 40 tenths of a second (4 seconds).
  */

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
  enableRawMode();

  char c;
  while(1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    read(STDIN_FILENO, &c, 1);
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n",c ,c);
    }
    if (c == 'q') break;
  };
  return 0;
}