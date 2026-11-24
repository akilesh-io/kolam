// gestures.cpp - teaching the ways to play. A small animated hand shows a
// gesture where it's wanted (tap, hold, drag or swipe), with its one word.
// Each gesture is remembered once the player has done it a few times (in the
// settings file): then its first-time hints rest, and only come back as a
// nudge when the player seems stuck. Also: telling a swipe from a tap.
#include <cmath>

#include "game.h"

namespace {
Vector2 pressAt = {0, 0};
double pressTime = 0;
bool swiped = false; // a swipe ended this frame
const int LEARNED_AFTER = 3;

const char *Word(Move g) {
  switch (g) {
  case MOVE_TAP:
    return TouchUi() ? "Tap" : "Click";
  case MOVE_HOLD:
    return "Hold";
  case MOVE_DRAG:
    return "Drag";
  default:
    return "Swipe";
  }
}
} // namespace

// Each frame: was there a swipe? (pressed, moved a good way, let go quickly)
void UpdateGestures() {
  swiped = false;
  const Vector2 m = GetMousePosition();
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    pressAt = m;
    pressTime = GetTime();
  }
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) &&
      std::hypot(m.x - pressAt.x, m.y - pressAt.y) > 45 &&
      GetTime() - pressTime < 1.0)
    swiped = true;
}

bool Swiped() { return swiped; }

bool GestureLearned(Move g) {
  return settings.gestureUses[g] >= LEARNED_AFTER;
}

void GestureDone(Move g) {
  if (settings.gestureUses[g] >= LEARNED_AFTER)
    return;
  settings.gestureUses[g]++;
  if (settings.gestureUses[g] == LEARNED_AFTER)
    SaveSettings(); // remembered for next time
}

void DrawGesture(Move g, Vector2 at, float time, float alpha, Vector2 way,
                 Color ink) {
  const float size = 46;
  Vector2 tip = at;
  float press = 0;   // 0 up .. 1 pressed down
  float fill = -1;   // the ring filling while held (0..1), or -1
  float fade = 1;
  switch (g) {
  case MOVE_TAP: {
    const float p = std::fmod(time, 1.3f) / 1.3f;
    press = Between(p, 0.25f, 0.35f) * (1 - Between(p, 0.45f, 0.55f));
    if (p > 0.4f) { // a ripple where it touched
      const float r = (p - 0.4f) / 0.6f;
      DrawRing(at, 6 + 22 * r, 9 + 22 * r, 0, 360, 32,
               Faded(ink, alpha * (1 - r) * 0.8f));
    }
    break;
  }
  case MOVE_HOLD: {
    const float p = std::fmod(time, 2.2f) / 2.2f;
    press = Between(p, 0.05f, 0.15f) * (1 - Between(p, 0.85f, 0.95f));
    fill = Between(p, 0.15f, 0.8f);
    break;
  }
  case MOVE_DRAG: {
    const float p = std::fmod(time, 1.8f) / 1.8f;
    const float t = Between(p, 0.15f, 0.75f);
    press = Between(p, 0.05f, 0.15f) * (1 - Between(p, 0.8f, 0.9f));
    tip = {at.x + way.x * 120 * t, at.y + way.y * 120 * t};
    if (t > 0) // the line it leaves
      DrawLineEx(at, tip, 5, Faded(ink, alpha * 0.6f * press));
    fade = 1 - Between(p, 0.85f, 1);
    break;
  }
  default: { // SWIPE: quick, and gone
    const float p = std::fmod(time, 1.4f) / 1.4f;
    const float t = Between(p, 0.2f, 0.5f);
    press = Between(p, 0.1f, 0.2f) * (1 - Between(p, 0.5f, 0.6f));
    tip = {at.x + way.x * 80 * t, at.y + way.y * 80 * t};
    for (int k = 1; k <= 4 && t > 0 && t < 1; k++) { // a streak behind it
      const float b = std::fmax(0.0f, t - k * 0.08f);
      DrawCircleV({at.x + way.x * 80 * b, at.y + way.y * 80 * b}, 5 - k,
                  Faded(ink, alpha * 0.5f * (1 - k / 5.0f)));
    }
    fade = 1 - Between(p, 0.6f, 0.75f);
    break;
  }
  }
  const float a = alpha * fade;
  if (fill >= 0 && press > 0.5f) { // holding: a ring fills round the finger
    DrawRing(tip, 16, 21, 0, 360, 32, Faded(ink, a * 0.25f));
    DrawRing(tip, 16, 21, -90, -90 + 360 * fill, 32, Faded(KUMKUM, a * 0.95f));
  }
  DrawHandIcon(tip, size * (1 - 0.1f * press), a);
  // its word, under the starting point, with a soft shadow to read on anything
  const char *word = Word(g);
  const bool lightInk = ink.r + ink.g + ink.b > 3 * 128;
  DrawTextCentered(word, at.x + 1, at.y + 36, 18,
                   Faded(lightInk ? BLACK : WHITE, a * 0.6f));
  DrawTextCentered(word, at.x, at.y + 35, 18, Faded(ink, a));
}
