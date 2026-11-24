// help.cpp - "How to play": every way of playing, each shown by a small
// animated hand (gestures.cpp) and a few words. Opened with the (i) button
// beside the pause button while playing, or from the pause menu.
#include <string>

#include "game.h"

namespace {
const Rectangle HELP_PANEL = {60, 24, 600, 672};
const Rectangle HELP_CLOSE = {260, 626, 200, 54};
const float ROW_TOP = 84, ROW_STEP = 94, ROW_H = 88;

struct HelpRow {
  int move; // a Move, or -1 for a drawn icon
  const char *touchText, *mouseText;
};

const HelpRow ROWS[] = {
    {MOVE_TAP, "Tap things to look at them, and to go on.",
     "Click things to look at them, and to go on."},
    {MOVE_SWIPE, "Swipe to move on when someone is talking.",
     "Click (or swipe) to move on when someone is talking."},
    {MOVE_HOLD, "Hold when something needs time: hard words, a fire.",
     "Hold when something needs time: hard words, a fire."},
    {MOVE_DRAG, "Drag to draw kolam lines, or to carry things.",
     "Drag to draw kolam lines, or to carry things."},
    {-1, "Erase or Undo fixes a line you drew.",
     "Erase or Undo fixes a line you drew."},
};
const int ROW_COUNT = sizeof(ROWS) / sizeof(ROWS[0]);
} // namespace

bool UpdateHelpScreen() {
  return Clicked(HELP_CLOSE) || BackPressed() || IsKeyPressed(KEY_ENTER) ||
         IsKeyPressed(KEY_SPACE);
}

void DrawHelpScreen(float time) {
  DrawRectangleRounded({HELP_PANEL.x + 4, HELP_PANEL.y + 8, HELP_PANEL.width, HELP_PANEL.height},
                       0.05f, 8, {0, 0, 0, 90});
  DrawRectangleRounded(HELP_PANEL, 0.05f, 8, {26, 24, 44, 245});
  DrawRectangleRoundedLinesEx(HELP_PANEL, 0.05f, 8, 1.5f, Faded(FLOUR, 0.22f));
  DrawTextCentered("How to play", 360, HELP_PANEL.y + 14, 44, FLOUR, HAND_FONT);
  for (int i = 0; i < ROW_COUNT; i++) {
    const float y = HELP_PANEL.y + ROW_TOP + i * ROW_STEP;
    const Rectangle row = {HELP_PANEL.x + 20, y, HELP_PANEL.width - 40, ROW_H};
    DrawRectangleRounded(row, 0.2f, 8, Faded(FLOUR, 0.04f));
    const Vector2 icon = {row.x + 70, y + 30};
    if (ROWS[i].move >= 0) {
      DrawGesture((Move)ROWS[i].move, icon, time + i * 0.37f, 1, {-0.6f, 0});
    } else { // Erase and Undo, as their buttons look
      DrawRectangleRounded({icon.x - 52, y + 18, 50, 30}, 0.4f, 6, KUMKUM);
      DrawTextCentered("Erase", icon.x - 27, y + 24, 14, WHITE);
      DrawRectangleRounded({icon.x + 4, y + 18, 50, 30}, 0.4f, 6,
                           Faded(FLOUR, 0.15f));
      DrawTextCentered("Undo", icon.x + 29, y + 24, 14, FLOUR);
    }
    DrawTextWrapped(TouchUi() ? ROWS[i].touchText : ROWS[i].mouseText,
                    row.x + 150, y + 12, 21, row.width - 165, FLOUR);
  }
  DrawTextCentered("Stuck? Wait a moment: what's next will glow.", 360,
                   HELP_PANEL.y + ROW_TOP + ROW_COUNT * ROW_STEP + 2, 17,
                   MUTED);
  const bool over = !TouchUi() && CheckCollisionPointRec(GetMousePosition(), HELP_CLOSE);
  DrawRectangleRounded(HELP_CLOSE, 0.4f, 8, over ? KUMKUM : Faded(KUMKUM, 0.85f));
  DrawTextCentered("Got it", 360, HELP_CLOSE.y + 14, 22, WHITE);
}
