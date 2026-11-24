// cursor.cpp - the mouse pointer: a plain white arrow, and a pointing hand
// over anything that can be clicked. The pictures are Phosphor Icons (MIT
// licence, see assets/cursors/LICENSE.txt), made from art/cursors/*.svg.
// They are drawn on top of everything, after the colour shader, so they never
// turn grey. Phones have no pointer, so none is drawn there.
#include "game.h"

namespace {

// Each picture is drawn this many stage units square. The "point" is where
// the arrow's tip and the hand's fingertip are, in the 256-unit picture.
const float CURSOR_SIZE = 30;
const Vector2 ARROW_POINT = {36, 36};
const Vector2 HAND_POINT = {116, 20};

Texture2D arrow = {}, hand = {};
bool pointing = false;    // over something clickable this frame
bool sceneCursor = false; // a scene draws its own pointer this frame

Texture2D LoadCursor(const char *path) {
  const EmbeddedFile *file = FindAsset(path);
  if (!file)
    return Texture2D{};
  Image image = LoadImageFromMemory(".png", file->data, (int)file->size);
  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);
  GenTextureMipmaps(&texture); // stays smooth when drawn small
  SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
  return texture;
}

} // namespace

void InitCursor() {
  // (on a phone too: the hand shows gestures in hints, see gestures.cpp)
  arrow = LoadCursor("assets/cursors/arrow.png");
  hand = LoadCursor("assets/cursors/hand.png");
#if !defined(PLATFORM_PHONE) // phones have no pointer
  if (arrow.id != 0 && hand.id != 0)
    HideCursor(); // ours replaces the system's
#endif
}

// The pointing hand, its fingertip at `tip`, `size` stage units square
void DrawHandIcon(Vector2 tip, float size, float alpha) {
  if (hand.id == 0)
    return;
  const Rectangle where = {tip.x - HAND_POINT.x / 256 * size,
                           tip.y - HAND_POINT.y / 256 * size, size, size};
  DrawTexturePro(hand, {0, 0, (float)hand.width, (float)hand.height}, where,
                 {0, 0}, 0, Faded(WHITE, alpha));
}

void PointAtSomething() { pointing = true; }
void SceneDrawsOwnCursor() { sceneCursor = true; }

// At the start of a frame: the scene says again what's under the mouse
void UpdateCursor() {
  pointing = false;
  sceneCursor = false;
}

void DrawCursor(Vector2 stageOrigin, float scale) {
  const Texture2D &picture = pointing ? hand : arrow;
  if (picture.id == 0 || sceneCursor || TouchUi() ||
      (!IsCursorOnScreen() && !IsWindowHidden()))
    return;
  const Vector2 mouse = GetMousePosition();
  const Vector2 point = pointing ? HAND_POINT : ARROW_POINT;
  const float size = CURSOR_SIZE * scale;
  const Rectangle where = {
      stageOrigin.x + mouse.x * scale - point.x / 256 * size,
      stageOrigin.y + mouse.y * scale - point.y / 256 * size, size, size};
  DrawTexturePro(picture, {0, 0, (float)picture.width, (float)picture.height},
                 where, {0, 0}, 0, WHITE);
}
