// settings_screen.cpp - the Settings screen (from the home screen or the
// pause menu). A card of rows, each a name on the left and its control on
// the right: a slider for a volume, or < value > arrows for a choice. Made
// for fingers as much as for a mouse or keys:
//   tap or drag a slider; tap an arrow, or the value itself, to change it
//   Up/Down choose a row, Left/Right change it, Enter, Esc goes back
// Changes show at once (main.cpp applies the settings every frame) and are
// saved when the screen closes.
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "game.h"

namespace {

enum Row { MASTER, MUSIC, SPEED, TEXT, DISPLAY, SHAPE, EDGES };
const char *SHAPE_NAMES[] = {"16:9", "Fill screen", "4:3", "Square"};
const int SHAPE_COUNT = 4;

const float ROW_H = 54, ROW_STEP = 60;
const float PANEL_W = 540;
const float CONTROL_W = 260; // the right part of a row, for its control
const float ARROW_R = 18;

int focus = 0;    // a row; then Reset, then Back
int sliding = -1; // the row whose slider is being dragged

std::vector<Row> Rows() {
#if defined(PLATFORM_PHONE) // a phone is always fullscreen
  return {MASTER, MUSIC, SPEED, TEXT, SHAPE, EDGES};
#else
  return {MASTER, MUSIC, SPEED, TEXT, DISPLAY, SHAPE, EDGES};
#endif
}
int RowCount() { return (int)Rows().size(); }
int ResetFocus() { return RowCount(); }
int BackFocus() { return RowCount() + 1; }

Rectangle Panel() {
  const float h = 104 + RowCount() * ROW_STEP + 100;
  return {360 - PANEL_W / 2, (720 - h) / 2, PANEL_W, h};
}
Rectangle RowRect(int i) {
  const Rectangle p = Panel();
  return {p.x + 22, p.y + 96 + i * ROW_STEP, p.width - 44, ROW_H};
}
Rectangle ResetRect() {
  const Rectangle last = RowRect(RowCount() - 1);
  return {360 - 10 - 200, last.y + ROW_H + 22, 200, 50};
}
Rectangle BackRect() {
  const Rectangle reset = ResetRect();
  return {360 + 10, reset.y, 200, 50};
}

// The parts of a row's control
Rectangle Track(Rectangle r) { // a slider's line (with room to grab it)
  return {r.x + r.width - CONTROL_W + 10, r.y, CONTROL_W - 80, r.height};
}
Vector2 LeftArrow(Rectangle r) {
  return {r.x + r.width - CONTROL_W + ARROW_R + 4, r.y + r.height / 2};
}
Vector2 RightArrow(Rectangle r) {
  return {r.x + r.width - ARROW_R - 6, r.y + r.height / 2};
}
Rectangle ArrowHit(Vector2 c) { // a little bigger than it's drawn
  return {c.x - ARROW_R - 8, c.y - ARROW_R - 8, 2 * ARROW_R + 16,
          2 * ARROW_R + 16};
}

bool IsSlider(Row row) { return row == MASTER || row == MUSIC; }
int &Volume(Row row) {
  return row == MASTER ? settings.masterVolume : settings.musicVolume;
}

const char *Label(Row row) {
  switch (row) {
  case MASTER:
    return "Volume";
  case MUSIC:
    return "Music";
  case SPEED:
    return "Text speed";
  case TEXT:
    return "Text size";
  case DISPLAY:
    return "Display";
  case SHAPE:
    return "Screen shape";
  default:
    return "Screen edges";
  }
}

std::string Value(Row row) {
  switch (row) {
  case MASTER:
  case MUSIC:
    return std::to_string(Volume(row)) + "%";
  case SPEED:
    return TextSpeedName();
  case TEXT:
    return TextSizeName();
  case DISPLAY:
    return settings.fullscreen ? "Fullscreen" : "Window";
  case SHAPE:
    return SHAPE_NAMES[settings.screenShape];
  default:
    return settings.softEdges ? "Soft glow" : "Black bars";
  }
}

void Change(Row row, int direction) {
  auto cycle = [&](int &value, int count) {
    value = (value + direction + count) % count;
  };
  switch (row) {
  case MASTER:
  case MUSIC:
    Volume(row) = std::max(0, std::min(100, Volume(row) + direction * 10));
    break;
  case SPEED:
    cycle(settings.textSpeed, TEXT_SPEED_COUNT);
    break;
  case TEXT:
    cycle(settings.textSize, TEXT_SIZE_COUNT);
    break;
  case DISPLAY:
    settings.fullscreen = !settings.fullscreen;
    break;
  case SHAPE:
    cycle(settings.screenShape, SHAPE_COUNT);
    break;
  default:
    settings.softEdges = !settings.softEdges;
    break;
  }
  PlayEffect("click", 0.4f);
}

void DrawChevron(Vector2 c, bool right, Color color) {
  const float s = right ? 1 : -1;
  DrawLineEx({c.x - 4 * s, c.y - 8}, {c.x + 4 * s, c.y}, 3, color);
  DrawLineEx({c.x + 4 * s, c.y}, {c.x - 4 * s, c.y + 8}, 3, color);
}

void DrawArrowButton(Vector2 c, bool right) {
  const bool over =
      !TouchUi() && CheckCollisionPointRec(GetMousePosition(), ArrowHit(c));
  DrawCircleV(c, ARROW_R, Faded(FLOUR, over ? 0.22f : 0.1f));
  DrawRing(c, ARROW_R - 1.5f, ARROW_R, 0, 360, 32, Faded(FLOUR, 0.35f));
  DrawChevron(c, right, FLOUR);
}

void DrawRowControl(Row row, Rectangle r) {
  const float cy = r.y + r.height / 2;
  if (IsSlider(row)) {
    const Rectangle t = Track(r);
    const float x0 = t.x + 8, x1 = t.x + t.width - 8;
    const float at = x0 + (x1 - x0) * Volume(row) / 100.0f;
    DrawRectangleRounded({x0, cy - 3, x1 - x0, 6}, 1, 6, Faded(FLOUR, 0.18f));
    DrawRectangleRounded({x0, cy - 3, at - x0, 6}, 1, 6, KUMKUM);
    DrawCircleV({at, cy}, 11, FLOUR);
    DrawRing({at, cy}, 8, 11, 0, 360, 24, KUMKUM);
    const std::string v = Value(row);
    DrawString(v, r.x + r.width - 12 - MeasureString(v, 19), cy - 11, 19,
               FLOUR);
    return;
  }
  const Vector2 left = LeftArrow(r), right = RightArrow(r);
  DrawArrowButton(left, false);
  DrawArrowButton(right, true);
  DrawTextCentered(Value(row), (left.x + right.x) / 2, cy - 11, 20, FLOUR);
}

} // namespace

const char *TextSizeName() {
  const char *names[TEXT_SIZE_COUNT] = {"Normal", "Large", "Larger"};
  return names[settings.textSize];
}

void OpenSettingsScreen() {
  focus = 0;
  sliding = -1;
}

// Returns true when the player goes back
bool UpdateSettingsScreen() {
  const std::vector<Row> rows = Rows();
  const int count = (int)rows.size();
  const Vector2 mouse = GetMousePosition();

  // Keys: Up/Down choose, Left/Right change, Enter does it
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_TAB))
    focus = std::min(focus + 1, BackFocus());
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    focus = std::max(focus - 1, 0);
  int direction = 0;
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    direction = -1;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    direction = 1;
  if (focus < count && direction != 0)
    Change(rows[focus], direction);
  if (focus >= count && direction != 0)
    focus = direction < 0 ? ResetFocus() : BackFocus();
  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
    if (focus < count)
      Change(rows[focus], 1);
    else if (focus == ResetFocus())
      settings = Settings();
    else
      return true;
  }

  // Mouse and touch
  if (sliding >= 0) { // dragging a slider
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      sliding = -1;
    } else {
      const Rectangle t = Track(RowRect(sliding));
      const float f = (mouse.x - (t.x + 8)) / (t.width - 16);
      const int v = (int)std::lround(std::max(0.0f, std::min(1.0f, f)) * 20) * 5;
      Volume(rows[sliding]) = v;
    }
  }
  for (int i = 0; i < count; i++) {
    const Rectangle r = RowRect(i);
    if (!CheckCollisionPointRec(mouse, r))
      continue;
    if (!TouchUi() && (GetMouseDelta().x != 0 || GetMouseDelta().y != 0))
      focus = i;
    if (IsSlider(rows[i])) {
      const Rectangle t = Track(r);
      if (CheckCollisionPointRec(mouse, {t.x - 10, t.y, t.width + 20, t.height})) {
        PointAtSomething();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          sliding = i;
          focus = i;
        }
      }
      continue;
    }
    if (Clicked(ArrowHit(LeftArrow(r)))) {
      focus = i;
      Change(rows[i], -1);
    } else if (Clicked(ArrowHit(RightArrow(r))) || Clicked(r)) {
      focus = i; // a tap anywhere else on the row: the next value
      Change(rows[i], 1);
    }
  }
  if (Clicked(ResetRect())) {
    focus = ResetFocus();
    settings = Settings();
  }
  if (Clicked(BackRect()))
    return true;
  return false;
}

void DrawSettingsScreen() {
  const std::vector<Row> rows = Rows();
  const Rectangle p = Panel();
  DrawRectangleRounded({p.x + 4, p.y + 8, p.width, p.height}, 0.06f, 8,
                       {0, 0, 0, 90});
  DrawRectangleRounded(p, 0.06f, 8, {26, 24, 44, 242});
  DrawRectangleRoundedLinesEx(p, 0.06f, 8, 1.5f, Faded(FLOUR, 0.22f));

  // the title, with a little kolam flourish under it
  DrawTextCentered("Settings", 360, p.y + 14, 44, FLOUR, HAND_FONT);
  for (int j = -3; j <= 3; j++)
    DrawCircleV({360.0f + j * 16, p.y + 72}, 1.8f, Faded(FLOUR, 0.7f));
  for (int j = -3; j < 3; j++)
    DrawRing({368.0f + j * 16, p.y + 72}, 6, 7.4f, j % 2 ? 0 : 180,
             j % 2 ? 180 : 360, 12, Faded(FLOUR, 0.45f));

  for (int i = 0; i < (int)rows.size(); i++) {
    const Rectangle r = RowRect(i);
    const bool chosen = focus == i;
    DrawRectangleRounded(r, 0.3f, 8, Faded(FLOUR, chosen ? 0.09f : 0.035f));
    if (chosen) // a red mark at its left
      DrawRectangleRounded({r.x, r.y + 10, 4, r.height - 20}, 1, 4, KUMKUM);
    DrawString(Label(rows[i]), r.x + 18, r.y + r.height / 2 - 12, 21, FLOUR);
    DrawRowControl(rows[i], r);
  }

  // Reset (quiet) and Back (red)
  const Rectangle reset = ResetRect(), back = BackRect();
  DrawRectangleRounded(reset, 0.4f, 8,
                       Faded(FLOUR, focus == ResetFocus() ? 0.16f : 0.06f));
  DrawRectangleRoundedLinesEx(reset, 0.4f, 8, 1, Faded(FLOUR, 0.35f));
  DrawTextCentered("Reset", reset.x + reset.width / 2, reset.y + 13, 21,
                   FLOUR);
  DrawRectangleRounded(back, 0.4f, 8,
                       focus == BackFocus() ? KUMKUM : Faded(KUMKUM, 0.8f));
  DrawTextCentered("Back", back.x + back.width / 2, back.y + 13, 21, WHITE);
  if (!TouchUi())
    DrawTextCentered("Arrow keys and Enter work too  -  Esc to go back", 360,
                     reset.y + reset.height + 14, 15, Faded(MUTED, 0.8f));
}
