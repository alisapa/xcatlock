#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>

#include "cat.xpm"
#include "lock.xpm"
#include "unicode.h"

// Function declarations
void do_cleanup(int exitCode);
void setup_wm_hints(char *progname, int argc, char **argv,
    int width, int height, Pixmap icon_pxm);
int query_xrm_bool(char *name);
int xim_setup(void);
int max(int a, int b);
void draw_window(Pixmap cat_pxm, Pixmap lock_pxm, int width, int height);

// Global variables
Display *disp;
Window win;
int screen_num;
GC cat_gc = NULL, lock_gc = NULL;
XSizeHints *size_hints;
XWMHints *wm_hints;
XClassHint *class_hints;
XTextProperty progName;
XTextProperty *progNameP;
XrmDatabase xrm_db = {0};
XIM xim = NULL;
XIC xic = NULL;

const char *usage_str =
"Usage: xcatlock <passphrase>\n"
"Options: -test, -tiny, -verbose\n"
;

static XrmOptionDescRec opTable[] = {
  {"-bg",       "*background", XrmoptionSepArg, (caddr_t) NULL},
  {"-fg",       "*foreground", XrmoptionSepArg, (caddr_t) NULL},
  {"-test",     "*istest",     XrmoptionNoArg,  (caddr_t) "1"},
  {"-tiny",     "*tiny",       XrmoptionNoArg,  (caddr_t) "1"},
  {"-verbose",  "*verbose",    XrmoptionNoArg,  (caddr_t) "1"},
};
const int opTableSize = 5;

void do_cleanup(int exitCode) {
  if (xic) XDestroyIC(xic);
  if (xim) XCloseIM(xim);
  if (lock_gc) XFreeGC(disp, lock_gc);
  if (cat_gc) XFreeGC(disp, cat_gc);
  if (progNameP && progName.value) XFree(progName.value);
  if (size_hints) XFree(size_hints);
  if (class_hints) XFree(class_hints);
  if (wm_hints) XFree(wm_hints);

  XUngrabPointer(disp, CurrentTime);
  XUngrabKeyboard(disp, CurrentTime);
  XUnmapWindow(disp, win);
  XCloseDisplay(disp);
  exit(exitCode);
}

void setup_wm_hints(char *progname, int argc, char **argv,
    int width, int height, Pixmap icon_pxm) {

  if (size_hints) {
    size_hints->flags = PPosition | PSize | PMinSize;
    size_hints->min_width = width;
    size_hints->min_height = height;
  } else {
    fprintf(stderr, "%s: failed to set size hints\n", progname);
  }

  if (class_hints) {
    class_hints->res_name = "xcatlock";
    class_hints->res_class = "xcatlock";
  } else {
    fprintf(stderr, "%s: failed to set class hints\n", progname);
  }

  if (wm_hints) {
    wm_hints->initial_state = NormalState; // iconified or normal? normal
    wm_hints->input = True; // Does application need keyboard input? Yes.
    wm_hints->icon_pixmap = icon_pxm;
    wm_hints->flags = StateHint | IconPixmapHint | InputHint;
    wm_hints->flags = StateHint | InputHint;
  } else {
    fprintf(stderr, "%s: failed to set WM hints\n", progname);
  }

  if (XStringListToTextProperty(&progname, 1, &progName)) {
    progNameP = &progName;
  } else {
    fprintf(stderr, "%s: failed to allocate structure for progName.\n",
        progname);
    progNameP = NULL;
  }

  XSetWMProperties(disp, win, progNameP, progNameP, argv, argc, size_hints,
      wm_hints, class_hints);
}

int query_xrm_bool(char *name) {
  char *str_type[20]; // TODO: Is this sufficiently large...?
  XrmValue value;
  if (XrmGetResource(xrm_db, name, name, str_type, &value)) {
    if (value.size > 0)
      return value.addr[0] - '0';
  }
  return 0;
}

int xim_setup(void) {
  xim = XOpenIM(disp, xrm_db, "xcatlock", "xcatlock");
  if (!xim) {
    fprintf(stderr, "Error: failed to open XIM.\n");
    return -1;
  }

  XIMStyles *im_supported_styles;
  XIMStyle style = 0;
  XGetIMValues(xim, XNQueryInputStyle, &im_supported_styles, NULL);
  for (int i = 0; i < im_supported_styles->count_styles; i++) {
    style = im_supported_styles->supported_styles[i];
    if ((style & (XIMPreeditNone | XIMStatusNone)) == style)
      break;
    else
      style = 0;
  }
  if (!style) {
    fprintf(stderr, "Error: input style XIMPreeditNone | XIMStatusNone is "
        "not supported by the given input method.\n");
    XFree(im_supported_styles);
    return -1;
  }
  xic = XCreateIC(xim, XNInputStyle, style, XNClientWindow, win, NULL);
  XFree(im_supported_styles);
  if (!xic) {
    fprintf(stderr, "Error: Failed to create input context.\n");
    return -1;
  }
  return 0;
}

inline int max(int a, int b) {
  return a > b ? a : b;
}

void draw_window(Pixmap cat_pxm, Pixmap lock_pxm, int width, int height) {
  int x_off = max((width - cat_width) / 2, 0);
  int y_off = max((height - cat_width) / 2, 0);
  XCopyPlane(disp, cat_pxm, win, cat_gc, 0, 0, cat_width, cat_height,
      x_off, y_off, 1);
  XSetClipOrigin(disp, lock_gc, x_off, y_off);
  XCopyPlane(disp, lock_pxm, win, lock_gc, 0, 0, lock_width, lock_height,
      x_off, y_off, 1);
}

int main (int argc, char **argv) {
  unsigned int width, height;
  unsigned int display_width, display_height;
  char *progname = argv[0];
  Pixmap cat_pixmap, lock_pixmap;

  int xpos = 0, ypos = 0;
  unsigned int border_width = 4;

  char *display_name = NULL;
  Screen *screen_ptr;

  int res;
  const int errbuf_size = 512;
  char errbuf[errbuf_size];

  // Get command-line or Xresources options
  XrmInitialize();
  XrmParseCommand(&xrm_db, opTable, opTableSize, "xcatlock", &argc, argv);
  if (argc > 2) {
    fprintf(stderr, "Expected 1 argument, got %d\n", argc-1);
    return 1;
  }
  int is_test = query_xrm_bool("xcatlock.istest");
  int is_large = !query_xrm_bool("xcatlock.tiny");
  int is_verbose = query_xrm_bool("xcatlock.verbose");

  if (argc < 2) {
    fprintf(stderr, "%s", usage_str);
    XrmDestroyDatabase(xrm_db);
    return 1;
  }

  // Open display and get default screen
  if (!(disp=XOpenDisplay(display_name))) {
    fprintf(stderr, "%s: cannot connect to X server %s\n",
        progname, XDisplayName(display_name));
    return -1;
  }
  screen_num = DefaultScreen(disp);
  screen_ptr = DefaultScreenOfDisplay(disp);

  // Create the window
  width = height = 48;
  if (is_large) {
    win = XCreateSimpleWindow(disp, RootWindow(disp, screen_num), xpos, ypos,
        width, height, border_width, BlackPixel(disp, screen_num),
        WhitePixel(disp, screen_num));
  } else {
    // Create the smallest possible window, and tell the window manager
    // not to add decorations.
    // Apparently, an InputOnly window can't grab the pointer, so this is the
    // smallest we can get.
    XSetWindowAttributes attrib;
    attrib.override_redirect = True;
    win = XCreateWindow(disp, RootWindow(disp, screen_num), xpos, ypos,
        1, 1, 0, CopyFromParent, InputOutput, CopyFromParent,
        CWOverrideRedirect, &attrib);
  }
  if (!win) do_cleanup(1);

  cat_pixmap = XCreateBitmapFromData(disp, win, cat_bits,
      cat_width, cat_height);
  lock_pixmap = XCreateBitmapFromData(disp, win, lock_bits,
      lock_width, lock_height);

  // Set up window manager hints
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  setup_wm_hints("xcatlock", argc, argv, width, height, cat_pixmap);

  // Input method setup
  int has_xim = (xim_setup() == 0);

  // Select event types
  unsigned long xim_filter_events = 0;
  if (has_xim) XGetIMValues(xim, XNFilterEvents, &xim_filter_events, NULL);
  XSelectInput(disp, win, ExposureMask | KeyPressMask | ButtonPressMask |
      StructureNotifyMask | xim_filter_events);

  // Graphics Contexts setup
  if (is_large) {
    XGCValues gcv = { .background = WhitePixel(disp, screen_num),
      .foreground = BlackPixel(disp, screen_num), .line_width = 1};
    cat_gc = XCreateGC(disp, win, GCForeground | GCBackground | GCLineWidth,
        &gcv);
    XColor exact, actual;
    res = XAllocNamedColor(disp, DefaultColormap(disp, screen_num),
        is_test ? "grey" : "green", &exact, &actual);
    if (res) gcv.foreground = actual.pixel;
    else gcv.foreground = BlackPixel(disp, screen_num);
    gcv.background = WhitePixel(disp, screen_num);
    gcv.clip_mask = lock_pixmap;
    lock_gc = XCreateGC(disp, win, GCForeground | GCBackground | GCLineWidth
        | GCClipMask, &gcv);
  }
  
  XMapWindow(disp, win);

  char buf[20];
  char *pass_start = argv[1];
  char *pass_next = pass_start + utf8_char_size(*pass_start);
  char *pass = argv[1];
  XEvent report;
  while (1) {
    XNextEvent(disp, &report);
    if (XFilterEvent(&report, None)) continue;
    switch (report.type) {
    case Expose:
      // Unless this is the last contiguous expose, don't draw the window
      if (report.xexpose.count != 0)
        break;
      if (is_verbose) printf("Got Expose event\n");

      // Grab the mouse and keyboard!
      if (res = XGrabKeyboard(disp, win, False, GrabModeAsync, GrabModeAsync,
            CurrentTime)) {
        XGetErrorText(disp, res, errbuf, errbuf_size);
        fprintf(stderr, "Failed to grab keyboard: %s\n", errbuf);
        do_cleanup(1);
      }
      if (res = XGrabPointer(disp, win, False, ButtonPressMask, GrabModeAsync,
            GrabModeAsync, None, None, CurrentTime)) {
        XGetErrorText(disp, res, errbuf, errbuf_size);
        fprintf(stderr, "Failed to grab mouse pointer: %s\n", errbuf);
        do_cleanup(1);
      }

      if (is_large) draw_window(cat_pixmap, lock_pixmap, width, height);
      break;
    case ConfigureNotify:
      width = report.xconfigure.width;
      height = report.xconfigure.height;
      if (is_verbose)
        printf("Got ConfigureNotify event: %d %d\n", width, height);
      break;
    case ButtonPress:
      if (is_test) do_cleanup(0);
      break;
    case KeyPress:
      if (is_verbose) printf("Got KeyPress event: %u\n", report.xkey.keycode);
      if (has_xim) Xutf8LookupString(xic, &report.xkey, buf, 20, NULL, NULL);
      else XLookupString(&report.xkey, buf, 20, NULL, NULL);
      if (!buf[0]) continue;
      unsigned siz = utf8_char_size(*buf);
      if (siz == -1) continue;
      if (is_verbose) printf("STR: %s (%d)\n", buf, siz);
      if (utf8_char_equ(pass, buf, siz)) {
        pass += siz;
        if (!*pass) {
success:
          printf("Successfully unlocked!\n");
          do_cleanup(0);
        }
      } else {
        if (utf8_char_equ(pass_start, buf, siz)) {
          pass = pass_next;
        } else {
          pass = pass_start;
        }
      }
      if (is_test) printf("Left to read: %s\n", pass);
      break;
    default:
      break;
    }
  }
}
