// autotest.cpp - play the game from a script, for testing: clicks, drags,
// holds and keys at given times, and screenshots of what the player sees.
// Nothing here runs unless one of these options is given:
//
//   --hidden                 no window on the desktop (it still draws)
//   --size WxH               open the window this big, e.g. 1600x720 for a
//                            phone held sideways (20:9)
//   --touch                  play as on a phone: "Tap", no pointer
//   --shot FILE@T            save what's on screen at T seconds, as a PNG.
//                            The game closes after the last shot.
//   --click X,Y@T            click there at T seconds
//   --hold X,Y@T+D           press there at T, let go D seconds later
//   --drag X1,Y1:X2,Y2@T+D   press at the first point at T, move to the
//                            second over D seconds, let go. More points
//                            (X1,Y1:X2,Y2:X3,Y3:...) make a path, each
//                            part taking the same time.
//   --key NAME@T             tap a key: space enter escape up down left
//                            right r n h v z c f5 (or a letter)
//
// X and Y are window pixels: the window opens at 1280x720, where the 720x720
// stage starts at x = 280. Times count from when the game opens.
// For example, with a save to resume (see README "Testing"):
//   ./build/linux/kolam --dev --resume --hidden
//       --click 640,400@4 --shot after.png@5.5
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "game.h"
#include "rlgl.h"

namespace {

// raylib's automation event types (rcore.c, AutomationEventType)
const unsigned int KEY_UP_EVENT = 1, KEY_DOWN_EVENT = 2;
const unsigned int MOUSE_UP_EVENT = 5, MOUSE_DOWN_EVENT = 6;
const unsigned int MOUSE_MOVE_EVENT = 7;

struct Action {
  double at;                  // seconds after the game opened
  unsigned int type;          // one of the events above, or 0 for a shot
  int x = 0, y = 0, key = 0;  // where (window pixels), or which key
  std::string file;           // a shot: where to save it
  bool done = false;
};

std::vector<Action> actions;
bool hidden = false;
int windowWidth = 1280, windowHeight = 720;
double lastShot = -1;
double start = -1;

// "640,400" -> 640, 400
bool ParsePoint(const std::string &text, int &x, int &y) {
  return std::sscanf(text.c_str(), "%d,%d", &x, &y) == 2;
}

int KeyNamed(const std::string &name) {
  const struct {
    const char *name;
    int key;
  } KEYS[] = {{"space", KEY_SPACE}, {"enter", KEY_ENTER},
              {"escape", KEY_ESCAPE}, {"up", KEY_UP},
              {"down", KEY_DOWN},     {"left", KEY_LEFT},
              {"right", KEY_RIGHT},   {"f5", KEY_F5}};
  for (const auto &k : KEYS) {
    if (name == k.name)
      return k.key;
  }
  if (name.size() == 1 && name[0] >= 'a' && name[0] <= 'z')
    return KEY_A + (name[0] - 'a');
  return 0;
}

void Add(double at, unsigned int type, int x, int y, int key = 0) {
  Action a;
  a.at = at;
  a.type = type;
  a.x = x;
  a.y = y;
  a.key = key;
  actions.push_back(a);
}

struct Point {
  int x, y;
};

// A press-and-release, moving along `path` while held
void AddPress(double at, double held, const std::vector<Point> &path) {
  const Point first = path.front(), last = path.back();
  Add(at - 0.05, MOUSE_MOVE_EVENT, first.x, first.y);
  Add(at, MOUSE_DOWN_EVENT, first.x, first.y);
  const size_t parts = path.size() - 1;
  for (size_t p = 0; p < parts; p++) {
    const Point a = path[p], b = path[p + 1];
    const double from = at + held * p / parts, time = held / parts;
    const int STEPS = std::max(1, (int)(time * 30));
    for (int i = 1; i <= STEPS && (a.x != b.x || a.y != b.y); i++)
      Add(from + time * i / STEPS, MOUSE_MOVE_EVENT,
          a.x + (b.x - a.x) * i / STEPS, a.y + (b.y - a.y) * i / STEPS);
  }
  Add(at + held, MOUSE_UP_EVENT, last.x, last.y);
}

bool Fail(const char *option, const std::string &value) {
  TraceLog(LOG_ERROR, "autotest: can't read %s %s", option, value.c_str());
  std::fprintf(stderr, "autotest: can't read %s %s\n", option, value.c_str());
  return false;
}

} // namespace

bool AutotestParse(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    const std::string option = argv[i];
    if (option == "--hidden") {
      hidden = true;
      continue;
    }
    if (option == "--touch") {
      SetTouchUi(true);
      continue;
    }
    if (option == "--size") {
      if (i + 1 >= argc ||
          std::sscanf(argv[++i], "%dx%d", &windowWidth, &windowHeight) != 2)
        return Fail("--size", i < argc ? argv[i] : "(missing)");
      continue;
    }
    if (option != "--shot" && option != "--click" && option != "--hold" &&
        option != "--drag" && option != "--key")
      continue;
    if (i + 1 >= argc)
      return Fail(option.c_str(), "(missing)");
    const std::string value = argv[++i];
    const size_t atSign = value.rfind('@');
    if (atSign == std::string::npos)
      return Fail(option.c_str(), value);
    const std::string what = value.substr(0, atSign);
    const std::string when = value.substr(atSign + 1);
    double at = 0, held = 0.12;
    if (std::sscanf(when.c_str(), "%lf+%lf", &at, &held) < 1)
      return Fail(option.c_str(), value);

    int x = 0, y = 0;
    if (option == "--shot") {
      Action a;
      a.at = at;
      a.type = 0;
      a.file = what;
      actions.push_back(a);
      lastShot = std::max(lastShot, at);
    } else if (option == "--click" || option == "--hold") {
      if (!ParsePoint(what, x, y))
        return Fail(option.c_str(), value);
      AddPress(at, option == "--click" ? 0.08 : held, {{x, y}, {x, y}});
    } else if (option == "--drag") {
      std::vector<Point> path;
      size_t from = 0;
      while (from <= what.size()) {
        size_t colon = what.find(':', from);
        if (colon == std::string::npos)
          colon = what.size();
        if (!ParsePoint(what.substr(from, colon - from), x, y))
          return Fail(option.c_str(), value);
        path.push_back({x, y});
        from = colon + 1;
      }
      if (path.size() < 2)
        return Fail(option.c_str(), value);
      AddPress(at, held, path);
    } else { // --key
      const int key = KeyNamed(what);
      if (key == 0)
        return Fail(option.c_str(), value);
      Add(at, KEY_DOWN_EVENT, 0, 0, key);
      Add(at + 0.08, KEY_UP_EVENT, 0, 0, key);
    }
  }
  if (hidden)
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
  return true;
}


void AutotestWindowSize(int *width, int *height) {
  *width = windowWidth;
  *height = windowHeight;
}

// At the start of each frame: play the input that's due
void AutotestFrame() {
  if (actions.empty())
    return;
  if (start < 0)
    start = GetTime();
  const double now = GetTime() - start;
  // At most one press or release per frame: on a slow frame both ends of a
  // click can come due together, and played at once they'd cancel out
  bool buttonPlayed = false;
  for (Action &a : actions) {
    if (a.done || a.type == 0 || a.at > now)
      continue;
    const bool button = a.type == MOUSE_DOWN_EVENT || a.type == MOUSE_UP_EVENT;
    if (button && buttonPlayed)
      break; // the rest wait for the next frame, in order
    buttonPlayed = buttonPlayed || button;
    AutomationEvent e = {};
    e.type = a.type;
    if (a.type == KEY_DOWN_EVENT || a.type == KEY_UP_EVENT) {
      e.params[0] = a.key;
    } else if (a.type == MOUSE_MOVE_EVENT) {
      e.params[0] = a.x;
      e.params[1] = a.y;
    } else { // a button: the left one, where the mouse is
      AutomationEvent move = {};
      move.type = MOUSE_MOVE_EVENT;
      move.params[0] = a.x;
      move.params[1] = a.y;
      PlayAutomationEvent(move);
      e.params[0] = MOUSE_BUTTON_LEFT;
    }
    PlayAutomationEvent(e);
    a.done = true;
  }
}

// Just before the frame is shown: take the shots that are due. Returns true
// once the last one is taken, so the game can close.
bool AutotestShots() {
  if (actions.empty() || start < 0)
    return false;
  const double now = GetTime() - start;
  for (Action &a : actions) {
    if (a.done || a.type != 0 || a.at > now)
      continue;
    rlDrawRenderBatchActive(); // what's drawn but still waiting in the batch
    Image image = LoadImageFromScreen();
    ExportImage(image, a.file.c_str());
    UnloadImage(image);
    std::printf("autotest: saved %s at %.1f s\n", a.file.c_str(), now);
    std::fflush(stdout);
    a.done = true;
  }
  return lastShot >= 0 && now > lastShot;
}
