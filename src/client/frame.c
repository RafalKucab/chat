#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "frame.h"

#define MIN_ROWS 5 // top border + 1 chat row + separator + input + bottom
#define MIN_COLS 20

// box-drawing characters (UTF-8: 3 bytes each, but 1 column wide)
#define BOX_H "─"
#define BOX_V "│"
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_ML "├"
#define BOX_MR "┤"

// growable output buffer: a whole frame is built here, then written at once
typedef struct {
  char *data;
  size_t len;
  size_t cap;
} sbuf;

void sb_append(sbuf *sb, const char *s, size_t n) {
  if (sb->len + n > sb->cap) {
    size_t cap = sb->cap ? sb->cap : 4096;
    while (cap < sb->len + n) {
      cap *= 2;
    }
    char *p = realloc(sb->data, cap);
    if (p == NULL) {
      return;
    }
    sb->data = p;
    sb->cap = cap;
  }
  memcpy(sb->data + sb->len, s, n);
  sb->len += n;
}

void sb_puts(sbuf *sb, const char *s) { sb_append(sb, s, strlen(s)); }

// append string s n times (s may be multi-byte, e.g. BOX_H)
void sb_repeat(sbuf *sb, const char *s, int n) {
  size_t len = strlen(s);
  for (int i = 0; i < n; i++) {
    sb_append(sb, s, len);
  }
}

// move cursor to row/col (1-based)
void sb_goto(sbuf *sb, int row, int col) {
  char seq[32];
  int n = snprintf(seq, sizeof(seq), "\x1b[%d;%dH", row, col);
  sb_append(sb, seq, (size_t)n);
}

void write_all(int fd, const char *buf, size_t len) {
  while (len > 0) {
    ssize_t n = write(fd, buf, len);
    if (n < 0) {
      if (errno == EINTR) {
        continue;
      }
      return;
    }
    buf += n;
    len -= (size_t)n;
  }
}

// terminal size in rows/cols; falls back to 24x80 when not a tty
void term_size(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 &&
      ws.ws_col > 0) {
    *rows = ws.ws_row;
    *cols = ws.ws_col;
  } else {
    *rows = 24;
    *cols = 80;
  }
}

// one visual row of the chat area: a slice of a chat line
typedef struct {
  const char *text;
  int len;
} chat_row;

/*
 * Draw the whole chat frame, sized to the terminal.
 *
 *   row 1            ┌── title ────────────┐
 *   rows 2..rows-3   │chat                 │
 *   row rows-2       ├─────────────────────┤
 *   row rows-1       │> input              │
 *   row rows         └─────────────────────┘
 *
 * title:   printable ASCII; cut to fit the top border.
 *
 * history: chat messages, oldest first; each is shown as " user > message".
 *          Newest messages sit at the bottom and long ones wrap. Trailing
 *          '\n'/'\r' are dropped. May be NULL (empty chat).
 * input:   text being typed (not NUL-terminated, input_len bytes); when it
 *          is wider than the box only its tail is shown.
 * Leaves the cursor at the end of the input.
 */
void render(const char *title, const chat_list_t *history, const char *input,
            size_t input_len) {
  (void)input; // input line drawing is not implemented yet
  (void)input_len;

  int rows, cols;
  term_size(&rows, &cols);

  sbuf sb = {0};
  sb_puts(&sb, "\x1b[?25l\x1b[2J"); // hide cursor, clear screen

  if (rows < MIN_ROWS || cols < MIN_COLS) {
    sb_goto(&sb, 1, 1);
    sb_puts(&sb, "Terminal too small");
    write_all(STDOUT_FILENO, sb.data, sb.len);
    free(sb.data);
    return;
  }

  int inner = cols - 2;     // width between the side borders
  int chat_rows = rows - 4; // minus top, separator, input, bottom

  // fill chat rows from the bottom up with the newest messages, wrapping
  // long ones across several rows. Each message takes at least one row, so at
  // most chat_rows lines are built (kept in `owned` until drawn).
  chat_row *view = calloc((size_t)chat_rows, sizeof(chat_row));
  char **owned = calloc((size_t)chat_rows, sizeof(char *));
  if (view == NULL || owned == NULL) {
    free(view);
    free(owned);
    free(sb.data);
    return;
  }
  int owned_count = 0;
  int fill = chat_rows;
  size_t msg_count = history != NULL ? history->size : 0;
  for (size_t i = msg_count; i > 0 && fill > 0; i--) {
    const msg_t *msg = history->data[i - 1];
    // fields may lack a NUL terminator: never read past the arrays
    size_t user_len = strnlen(msg->user, sizeof(msg->user));
    size_t msg_len = strnlen(msg->message, sizeof(msg->message));
    while (msg_len > 0 && (msg->message[msg_len - 1] == '\n' ||
                           msg->message[msg_len - 1] == '\r')) {
      msg_len--;
    }

    // " user > message"
    size_t line_len = 1 + user_len + 3 + msg_len;
    char *line = malloc(line_len + 1);
    if (line == NULL) {
      break;
    }
    char *p = line;
    *p++ = ' ';
    memcpy(p, msg->user, user_len);
    p += user_len;
    memcpy(p, " > ", 3);
    p += 3;
    memcpy(p, msg->message, msg_len);
    p[msg_len] = '\0';
    owned[owned_count++] = line;

    int len = (int)line_len;
    int chunks = (len + inner - 1) / inner;
    for (int c = chunks - 1; c >= 0 && fill > 0; c--) {
      int off = c * inner;
      int n = len - off < inner ? len - off : inner;
      view[--fill] = (chat_row){line + off, n};
    }
  }

  // top border with title
  // count columns, not bytes: "┌── " + title + " " + "─"... + "┐" == cols
  sb_goto(&sb, 1, 1);
  int title_room = inner - 4; // minus "──" and the two spaces
  int title_len = (int)strlen(title);
  if (title_len > title_room) {
    title_len = title_room;
  }
  sb_puts(&sb, BOX_TL BOX_H BOX_H " ");
  sb_append(&sb, title, (size_t)title_len);
  sb_puts(&sb, " ");
  sb_repeat(&sb, BOX_H, title_room - title_len);
  sb_puts(&sb, BOX_TR);

  // chat area
  for (int r = 0; r < chat_rows; r++) {
    sb_goto(&sb, 2 + r, 1);
    sb_puts(&sb, BOX_V);
    if (view[r].text != NULL) {
      sb_append(&sb, view[r].text, (size_t)view[r].len);
    }
    sb_repeat(&sb, " ", inner - view[r].len);
    sb_puts(&sb, BOX_V);
  }
  free(view);
  for (int i = 0; i < owned_count; i++) {
    free(owned[i]);
  }
  free(owned);

  // separator
  sb_goto(&sb, rows - 2, 1);
  sb_puts(&sb, BOX_ML);
  sb_repeat(&sb, BOX_H, inner);
  sb_puts(&sb, BOX_MR);

  // input line
  const char *prompt = "> ";
  int prompt_len = (int)strlen(prompt);
  // int visible = inner - prompt_len - 1; // keep one cell for the cursor
  // int shown = (int)input_len < visible ? (int)input_len : visible;
  sb_goto(&sb, rows - 1, 1);
  sb_puts(&sb, BOX_V);
  sb_puts(&sb, prompt);
  // sb_append(&sb, input + input_len - (size_t)shown, (size_t)shown);
  sb_repeat(&sb, " ", inner - prompt_len);
  sb_puts(&sb, BOX_V);

  // bottom border
  sb_goto(&sb, rows, 1);
  sb_puts(&sb, BOX_BL);
  sb_repeat(&sb, BOX_H, inner);
  sb_puts(&sb, BOX_BR);

  // park the cursor after the typed text and show it
  sb_goto(&sb, rows - 1, 2 + prompt_len);
  sb_puts(&sb, "\x1b[?25h");

  write_all(STDOUT_FILENO, sb.data, sb.len);
  free(sb.data);
}
