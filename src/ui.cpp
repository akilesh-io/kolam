// ui.cpp - fonts, menus, buttons, text and other small drawing helpers shared
// by all screens, plus finding the files that are built into the game.
#include <algorithm>
#include <cmath>

#include "game.h"

//------------------------------------------------------------------------------
// Files built into the game
//------------------------------------------------------------------------------
const EmbeddedFile *FindAsset(const std::string &path) {
  for (const EmbeddedFile *file = EMBEDDED_FILES; file->path; file++) {
    if (path == file->path)
      return file;
  }
  TraceLog(LOG_WARNING, "Missing asset: %s", path.c_str());
  return nullptr;
}

//------------------------------------------------------------------------------
// Fonts
//------------------------------------------------------------------------------
static Font fonts[2];
static bool fontsLoaded = false;

static bool IsPowerOfTwo(int n) { return n > 0 && (n & (n - 1)) == 0; }

// Loads a font with letters `glyphSize` pixels tall, so they are drawn at
// (about) their real size instead of being stretched, which would blur them
static Font LoadSharpFont(const char *path, int glyphSize) {
  const EmbeddedFile *file = FindAsset(path);
  if (!file)
    return GetFontDefault();
  Font font = LoadFontFromMemory(".ttf", file->data, (int)file->size, glyphSize,
                                 nullptr, 0);
  if (font.texture.id == 0)
    return GetFontDefault();
  // Mipmaps keep text sharp when it's drawn smaller than the letters were made.
  // Older phone graphics can only make them for power-of-two sized textures.
  const Texture2D &t = font.texture;
  if (IsPowerOfTwo(t.width) && IsPowerOfTwo(t.height)) {
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
  } else {
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  }
  return font;
}

void LoadFonts(float pixelScale) {
  UnloadFonts(); // when the window size changes, the fonts are made again
  // Letters 64 px tall at the game's normal size, bigger on big screens
  // (capped, because very big letters take a lot of memory)
  int glyphSize = (int)(64 * pixelScale);
  glyphSize = glyphSize < 64 ? 64 : (glyphSize > 192 ? 192 : glyphSize);
  fonts[UI_FONT] = LoadSharpFont("assets/fonts/BalooThambi2.ttf", glyphSize);
  fonts[HAND_FONT] = LoadSharpFont("assets/fonts/Kalam-Bold.ttf", glyphSize);
  fontsLoaded = true;
}

void UnloadFonts() {
  if (!fontsLoaded)
    return;
  for (Font &font : fonts) {
    if (font.texture.id != GetFontDefault().texture.id)
      UnloadFont(font);
  }
  fontsLoaded = false;
}

// Settings -> Text size: words grow by this much. Small words grow the most
// (they're the hard ones to read on a phone); big titles only a little, so
// they still fit where they are.
static float Scaled(float size) {
  // (players found even "Large" too small on a phone: these are bigger)
  const float GROW[TEXT_SIZE_COUNT] = {0, 0.32f, 0.6f};
  return size + GROW[settings.textSize] * std::fmin(size, 24.0f);
}

void DrawString(const std::string &text, float x, float y, float size,
                Color color, FontStyle font) {
  DrawTextEx(fonts[font], text.c_str(), {x, y}, Scaled(size), 0, color);
}

float MeasureString(const std::string &text, float size, FontStyle font) {
  return MeasureTextEx(fonts[font], text.c_str(), Scaled(size), 0).x;
}

void DrawTextCentered(const std::string &text, float centerX, float y,
                      float size, Color color, FontStyle font) {
  DrawString(text, centerX - MeasureString(text, size, font) / 2, y, size,
             color, font);
}

// Draws text broken into lines no wider than maxWidth
void DrawTextWrapped(const std::string &text, float x, float y, float size,
                     float maxWidth, Color color, FontStyle font) {
  const float lineHeight = Scaled(size) * 1.15f;
  int lines = 0;
  std::string line;
  std::string word;
  for (size_t i = 0; i <= text.size(); i++) {
    if (i < text.size() && text[i] != ' ') {
      word += text[i];
      continue;
    }
    const std::string longer = line.empty() ? word : line + " " + word;
    if (!line.empty() && MeasureString(longer, size, font) > maxWidth) {
      DrawString(line, x, y + lines * lineHeight, size, color, font);
      lines++;
      line = word;
    } else {
      line = longer;
    }
    word.clear();
  }
  DrawString(line, x, y + lines * lineHeight, size, color, font);
}

//------------------------------------------------------------------------------
// Menus and buttons
//------------------------------------------------------------------------------
const float ITEM_HEIGHT = 50;
const float ITEM_GAP = 10;
const float ARROW_SPACE = 34; // above and below the rows of a scrolling menu

// A menu scrolls when it has more items than `rows` (not counting Back)
static bool Scrolls(const Menu &menu) {
  return menu.rows > 0 && (int)menu.items.size() - 1 > menu.rows;
}

static int MaxScroll(const Menu &menu) {
  return std::max(0, (int)menu.items.size() - 1 - menu.rows);
}

// The arrows of a scrolling menu, above its rows and below them. They show
// there is more, and clicking one scrolls.
static Rectangle UpArrow(const Menu &menu) {
  return {menu.centerX - menu.width / 2, menu.top, menu.width,
          ARROW_SPACE - 4};
}

static Rectangle DownArrow(const Menu &menu) {
  const float rowsEnd =
      menu.top + ARROW_SPACE + menu.rows * (ITEM_HEIGHT + ITEM_GAP) - ITEM_GAP;
  return {menu.centerX - menu.width / 2, rowsEnd + 4, menu.width,
          ARROW_SPACE - 4};
}

// Scrolls so the selected item can be seen (Back is always shown)
static void ScrollToSelected(Menu &menu) {
  if (!Scrolls(menu) || menu.selected == (int)menu.items.size() - 1)
    return;
  menu.scroll = std::min(menu.scroll, menu.selected);
  menu.scroll = std::max(menu.scroll, menu.selected - menu.rows + 1);
}

// Scrolls `by` rows, moving the selection along if it went out of sight
static void ScrollBy(Menu &menu, int by) {
  menu.scroll = std::max(0, std::min(MaxScroll(menu), menu.scroll + by));
  if (menu.selected != (int)menu.items.size() - 1)
    menu.selected = std::max(
        menu.scroll, std::min(menu.scroll + menu.rows - 1, menu.selected));
}

void Menu::Set(std::vector<MenuItem> newItems, float newTop, float newWidth,
               int newRows) {
  items = newItems;
  top = newTop;
  width = newWidth;
  centerX = STAGE_SIZE / 2.0f;
  rows = newRows;
  scroll = 0;
  selected = 0;
  while (selected < (int)items.size() - 1 && !items[selected].enabled)
    selected++;
}

void Menu::Select(int i) {
  selected = i;
  ScrollToSelected(*this);
}

bool Menu::Shown(int i) const {
  if (!Scrolls(*this) || i == (int)items.size() - 1)
    return true;
  return i >= scroll && i < scroll + rows;
}

Rectangle Menu::ItemRect(int i) const {
  float y = top + i * (ITEM_HEIGHT + ITEM_GAP);
  if (Scrolls(*this)) { // the rows between the arrows, then Back
    const bool back = i == (int)items.size() - 1;
    const int row = back ? rows : i - scroll;
    y = top + ARROW_SPACE + row * (ITEM_HEIGHT + ITEM_GAP);
    if (back) // below the down arrow
      y += ARROW_SPACE - ITEM_GAP;
  }
  return {centerX - width / 2, y, width, ITEM_HEIGHT};
}

int Menu::Update() {
  const int count = (int)items.size();
  if (count == 0)
    return -1;

  // Keyboard: move up/down, skipping disabled items
  int step = 0;
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_TAB))
    step = 1;
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    step = -1;
  if (step != 0) {
    int next = selected;
    for (int tries = 0; tries < count; tries++) {
      next = (next + step + count) % count;
      if (items[next].enabled)
        break;
    }
    selected = next;
    ScrollToSelected(*this);
  }

  // A long menu scrolls with the mouse wheel (a row per notch; a touchpad's
  // small moves add up) or by clicking its arrows
  static float wheel = 0;
  int scrollBy = 0;
  if (Scrolls(*this)) {
    wheel += GetMouseWheelMove();
    const int notches = (int)wheel; // whole rows; the rest waits
    wheel -= notches;
    scrollBy = -notches;
    if (scroll > 0 && Clicked(UpArrow(*this)))
      scrollBy = -1;
    if (scroll < MaxScroll(*this) && Clicked(DownArrow(*this)))
      scrollBy = 1;
    if (scrollBy != 0)
      ScrollBy(*this, scrollBy);
  }

  // Mouse: hovering selects, clicking chooses
  const Vector2 mouse = GetMousePosition();
  const Vector2 moved = GetMouseDelta();
  for (int i = 0; i < count; i++) {
    if (!Shown(i) || !items[i].enabled ||
        !CheckCollisionPointRec(mouse, ItemRect(i)))
      continue;
    PointAtSomething();
    if (moved.x != 0 || moved.y != 0 || scrollBy != 0)
      selected = i;
    if (Clicked(ItemRect(i))) {
      selected = i;
      return i;
    }
  }

  if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) &&
      items[selected].enabled) {
    PlayEffect("click", 0.5f);
    return selected;
  }
  return -1;
}

// A small triangle in the middle of `r`, pointing up or down; brighter while
// the mouse is over it
static void DrawArrow(Rectangle r, bool up) {
  const float x = r.x + r.width / 2, y = r.y + r.height / 2;
  const bool hover = CheckCollisionPointRec(GetMousePosition(), r);
  const Color color = Faded(FLOUR, hover ? 0.9f : 0.5f);
  if (up)
    DrawTriangle({x, y - 6}, {x - 12, y + 6}, {x + 12, y + 6}, color);
  else
    DrawTriangle({x, y + 6}, {x + 12, y - 6}, {x - 12, y - 6}, color);
}

void Menu::Draw() const {
  if (Scrolls(*this)) { // arrows where there is more
    if (scroll > 0)
      DrawArrow(UpArrow(*this), true);
    if (scroll < MaxScroll(*this))
      DrawArrow(DownArrow(*this), false);
  }
  for (int i = 0; i < (int)items.size(); i++) {
    if (!Shown(i))
      continue;
    const MenuItem &item = items[i];
    const Rectangle r = ItemRect(i);
    const bool highlighted = i == selected && item.enabled;

    if (highlighted) {
      DrawRectangleRounded(r, 0.3f, 8, KUMKUM);
    } else {
      DrawRectangleRounded(r, 0.3f, 8, Faded(FLOUR, 0.06f));
      DrawRectangleRoundedLinesEx(r, 0.3f, 8, 1,
                                  Faded(FLOUR, item.enabled ? 0.3f : 0.12f));
    }

    Color text = highlighted ? WHITE : FLOUR;
    if (!item.enabled)
      text = Faded(MUTED, 0.5f);
    const float centerX = r.x + r.width / 2;
    if (item.detail.empty()) {
      DrawTextCentered(item.label, centerX, r.y + 11, 25, text);
    } else {
      Color detailText = highlighted ? WHITE : MUTED;
      if (!item.enabled)
        detailText = Faded(MUTED, 0.5f);
      DrawTextCentered(item.label, centerX, r.y + 3, 23, text);
      DrawTextCentered(item.detail, centerX, r.y + 28, 15, detailText);
    }
  }
}

void DrawButton(Rectangle rect, const std::string &label, Color fill) {
  DrawRectangleRounded(rect, 0.4f, 6, fill);
  const float size = 20;
  DrawTextCentered(label, rect.x + rect.width / 2,
                   rect.y + (rect.height - size) / 2 - 2, size, WHITE);
}

bool Clicked(Rectangle rect) { // every button clicks
  const bool over = CheckCollisionPointRec(GetMousePosition(), rect);
  if (over)
    PointAtSomething();
  const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && over;
  if (clicked)
    PlayEffect("click", 0.5f);
  return clicked;
}

bool ContinuePressed() {
  return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE) ||
         IsKeyPressed(KEY_ENTER);
}

#if defined(PLATFORM_PHONE)
static bool touchUi = true;
#else
static bool touchUi = false;
#endif
bool TouchUi() { return touchUi; }
void SetTouchUi(bool on) { touchUi = on; }
const char *Tap() { return touchUi ? "Tap" : "Click"; }
const char *tap() { return touchUi ? "tap" : "click"; }

// Top-left of the whole screen; it reaches a little further than it's drawn,
// so it's easy to hit with a thumb
static Rectangle NavButtonRect() {
  const Rectangle screen = FullScreen();
  return {screen.x + 12, 12, 60, 60};
}

// (i), just right of the pause button
static Rectangle InfoButtonRect() {
  const Rectangle pause = NavButtonRect();
  return {pause.x + pause.width + 14, pause.y, pause.width, pause.height};
}

bool InfoButton() {
  const Rectangle r = InfoButtonRect();
  return Clicked({r.x - 6, r.y - 12, r.width + 12, r.height + 24});
}

void DrawInfoButton() {
  const Rectangle r = InfoButtonRect();
  const Vector2 c = {r.x + r.width / 2, r.y + r.height / 2};
  DrawCircleV(c, r.width / 2, {20, 16, 30, 150});
  DrawRing(c, r.width / 2 - 2, r.width / 2, 0, 360, 40, Faded(FLOUR, 0.5f));
  DrawCircleV({c.x, c.y - 11}, 3.5f, FLOUR); // an "i"
  DrawRectangleRounded({c.x - 3, c.y - 3, 6, 18}, 0.5f, 4, FLOUR);
}

bool NavButton() {
  const Rectangle r = NavButtonRect();
  return Clicked({r.x - 12, r.y - 12, r.width + 24, r.height + 24});
}

void DrawNavButton(bool back) {
  const Rectangle r = NavButtonRect();
  const Vector2 c = {r.x + r.width / 2, r.y + r.height / 2};
  const bool over = CheckCollisionPointRec(GetMousePosition(), r) && !touchUi;
  DrawCircleV(c, r.width / 2, {20, 16, 30, (unsigned char)(over ? 200 : 150)});
  DrawRing(c, r.width / 2 - 2, r.width / 2, 0, 360, 40, Faded(FLOUR, 0.5f));
  if (back) { // an arrow pointing left
    DrawLineEx({c.x + 12, c.y}, {c.x - 12, c.y}, 4, FLOUR);
    DrawLineEx({c.x - 13, c.y + 1}, {c.x - 2, c.y - 11}, 4, FLOUR);
    DrawLineEx({c.x - 13, c.y - 1}, {c.x - 2, c.y + 11}, 4, FLOUR);
  } else { // pause: two bars
    DrawRectangleRounded({c.x - 10, c.y - 12, 7, 24}, 0.4f, 4, FLOUR);
    DrawRectangleRounded({c.x + 3, c.y - 12, 7, 24}, 0.4f, 4, FLOUR);
  }
}

float DistanceToSegment(Vector2 p, Vector2 a, Vector2 b) {
  const float dx = b.x - a.x, dy = b.y - a.y;
  const float lengthSquared = dx * dx + dy * dy;
  float t = lengthSquared < 0.0001f
                ? 0
                : ((p.x - a.x) * dx + (p.y - a.y) * dy) / lengthSquared;
  t = std::fmax(0.0f, std::fmin(1.0f, t));
  return std::hypot(p.x - (a.x + dx * t), p.y - (a.y + dy * t));
}

float DistanceToPath(Vector2 p, const std::vector<Vector2> &path,
                     size_t *index) {
  float best = 1e9f;
  for (size_t i = 0; i < path.size(); i++) {
    const float d = i + 1 < path.size()
                        ? DistanceToSegment(p, path[i], path[i + 1])
                        : std::hypot(p.x - path[i].x, p.y - path[i].y);
    if (d < best) {
      best = d;
      if (index)
        *index = i;
    }
  }
  return best;
}

void DrawEraserRing() {
  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    return;
  const Vector2 m = GetMousePosition();
  DrawCircleV(m, ERASER_RADIUS, Faded(FLOUR, 0.12f));
  ::DrawRing(m, ERASER_RADIUS - 1.5f, ERASER_RADIUS, 0, 360, 32,
             Faded(FLOUR, 0.7f));
}

void RubOut::Fade(const std::vector<Vector2> &path, float thick,
                  Color colour) {
  if (path.size() >= 2)
    fading.push_back({path, thick, colour, 0.45f});
}

void RubOut::DrawFading() {
  const float dt = GetFrameTime();
  for (Fading &f : fading)
    f.left -= dt;
  fading.erase(std::remove_if(fading.begin(), fading.end(),
                              [](const Fading &f) { return f.left <= 0; }),
               fading.end());
  for (const Fading &f : fading) {
    const float a = f.left / 0.45f;
    for (size_t i = 0; i + 1 < f.path.size(); i++)
      DrawLineEx(f.path[i], f.path[i + 1], f.thick * (0.6f + 0.4f * a),
                 Faded(f.colour, a * 0.8f));
  }
}

bool BackPressed() {
  return IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) ||
         IsKeyPressed(KEY_BACK);
}

//------------------------------------------------------------------------------
// Dialogue
//------------------------------------------------------------------------------
void DrawDialogueBox(const Line &line, float typed, float top) {
  DrawPanel({20, top, 680, 200});
  if (!line.speaker.empty())
    DrawString(line.speaker, 50, top + 16, 24, KUMKUM);

  const size_t visible =
      typed < line.text.size() ? (size_t)typed : line.text.size();
  // narration is a little softer than people talking
  const Color textColor = line.speaker.empty() ? Faded(FLOUR, 0.8f) : FLOUR;
  DrawTextWrapped(line.text.substr(0, visible), 50, top + 48, 24, 620,
                  textColor);

  if (line.hold > 0 && visible < line.text.size()) { // "hold to say it"
    const float pulse = 0.4f + 0.4f * std::sin((float)GetTime() * 4);
    DrawRing({676, top + 176}, 9, 12, 0, 360, 24, Faded(FLOUR, pulse));
  } else if (visible == line.text.size() && !TouchUi()) { // "more" arrow
    // (on a phone, the swipe hint in the corner says it: DrawDialogueHint)
    const float blink = 0.5f + 0.5f * std::sin((float)GetTime() * 4);
    DrawString("v", 670, top + 170, 20, Faded(FLOUR, blink));
  }
}

// What the line being said is waiting for, this frame (DrawDialogueHint)
static DialogueWait dialogueWait = WAIT_NONE;
static DialogueWait lastWait = WAIT_NONE;
static float waitedFor = 0;      // how long it has waited like that
static bool finishedByPress = false; // this press showed the line whole

DialogueWait DialogueWaiting() { return dialogueWait; }

void UpdateDialogueWait() { // at the start of each frame
  if (dialogueWait != WAIT_NONE && dialogueWait == lastWait)
    waitedFor += GetFrameTime();
  else
    waitedFor = 0;
  lastWait = dialogueWait;
  dialogueWait = WAIT_NONE;
}

// The gesture a line is waiting for, in the bottom-right corner: for the
// first few times, or when the player seems not to know what to do
void DrawDialogueHint(float time) {
  if (lastWait == WAIT_NONE)
    return;
  const Rectangle screen = FullScreen();
  const Vector2 at = {std::fmin(screen.x + screen.width, 720.0f) - 70, 640};
  if (lastWait == WAIT_HOLD) {
    if (!GestureLearned(MOVE_HOLD) || waitedFor > 2.5f)
      DrawGesture(MOVE_HOLD, at, time);
  } else if (lastWait == WAIT_GO_ON && TouchUi()) {
    if (!GestureLearned(MOVE_SWIPE) || waitedFor > 4)
      DrawGesture(MOVE_SWIPE, {at.x + 30, at.y}, time);
    else { // once learned: a small "swipe" mark
      const float p = std::fmod(time, 1.2f) / 1.2f;
      for (int k = 0; k < 2; k++) {
        const float x = at.x + 30 - k * 14 - 10 * p;
        DrawLineEx({x, at.y - 8}, {x - 8, at.y}, 3, Faded(FLOUR, 0.7f));
        DrawLineEx({x - 8, at.y}, {x, at.y + 8}, 3, Faded(FLOUR, 0.7f));
      }
    }
  }
}

bool AdvanceDialogue(const std::vector<Line> &lines, size_t &current,
                     float &typed) {
  if (lines.empty())
    return true;
  const Line &line = lines[current];
  if (line.hold > 0) { // the words come out only while the button is held
    const bool held = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                      IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER);
    if (typed < line.text.size()) {
      dialogueWait = held ? WAIT_TYPING : WAIT_HOLD;
      if (held) {
        typed += line.text.size() / line.hold * GetFrameTime();
        if (typed >= line.text.size())
          GestureDone(MOVE_HOLD);
      }
      finishedByPress = true; // (letting go of the hold isn't a swipe on)
      return false;
    }
  } else {
    typed += TextCharsPerSecond() * GetFrameTime();
  }
  const bool whole = typed >= lines[current].text.size();
  dialogueWait = whole ? WAIT_GO_ON : WAIT_TYPING;
  const bool tapped = ContinuePressed();
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    finishedByPress = false;
  const bool swiped = Swiped() && !finishedByPress;
  if (!tapped && !swiped)
    return false;

  if (!whole) { // first: show the whole line
    typed = (float)lines[current].text.size();
    finishedByPress = true;
    return false;
  }
  // then on to the next line: on a phone only with a swipe, so a stray tap
  // never skips a line before it's read (a mouse, or a key, just clicks)
  if (TouchUi() && !swiped)
    return false;
  if (swiped)
    GestureDone(MOVE_SWIPE);
  if (current + 1 < lines.size()) {
    current++;
    typed = 0;
    return false;
  }
  return true; // that was the last line
}

//------------------------------------------------------------------------------
// Small drawing helpers
//------------------------------------------------------------------------------
float Between(float x, float from, float to) {
  const float t = std::fmax(0, std::fmin(1, (x - from) / (to - from)));
  return t * t * (3 - 2 * t);
}

Color Faded(Color color, float alpha) {
  if (alpha < 0)
    alpha = 0;
  if (alpha > 1)
    alpha = 1;
  color.a = (unsigned char)(color.a * alpha);
  return color;
}

void DrawPanel(Rectangle rect) {
  DrawRectangleRounded(rect, 0.06f, 8, PANEL);
  DrawRectangleRoundedLinesEx(rect, 0.06f, 8, 1.5f, Faded(FLOUR, 0.25f));
}

void DrawBand(float y, float height, Color color) {
  const Rectangle r = FullScreen();
  DrawRectangleRec({r.x, y, r.width, height}, color);
}

void DrawBandGradient(float y, float height, Color top, Color bottom) {
  const Rectangle r = FullScreen();
  // floor and +2: whole units that cover the edges, with no hairline gaps
  DrawRectangleGradientV((int)std::floor(r.x), (int)y, (int)r.width + 2,
                         (int)height, top, bottom);
}

void DrawDim() { DrawRectangleRec(FullScreen(), {0, 0, 0, 150}); }

//------------------------------------------------------------------------------
// Home screen background: the solved pages of the kolam book draw themselves
// one after another in rice-flour white.
//------------------------------------------------------------------------------
bool HomeIsWide() { return FullScreen().width >= 1100; }

float HomeColumnX() {
  const Rectangle screen = FullScreen();
  return HomeIsWide() ? screen.x + screen.width * 0.3f : STAGE_SIZE / 2.0f;
}

Rectangle HomeKolamArea() {
  if (!HomeIsWide()) // between the subtitle (ends near y = 140) and the first
    return {190, 161, 340, 150}; // menu button (y = 326)
  const Rectangle screen = FullScreen();
  const float size = std::fmin(screen.width * 0.36f, 500.0f);
  const float x = screen.x + screen.width * 0.68f;
  return {x - size / 2, 360 - size / 2, size, size};
}

void DrawHomeBackground(float time) {
  const float KOLAM_DRAW_SECONDS = 7.0f; // drawing the line
  const float KOLAM_HOLD_SECONDS = 2.5f; // admiring it
  const float KOLAM_FADE_SECONDS = 1.2f; // fading away before the next one
  const float CYCLE =
      KOLAM_DRAW_SECONDS + KOLAM_HOLD_SECONDS + KOLAM_FADE_SECONDS;
  const int FIRST_PAGE = 3; // start with page 4, the big festival kolam

  // Each kolam, solved, in "cells" (the distance between two dots); it's
  // fitted into HomeKolamArea() when drawn, so it follows the screen's shape
  struct Kolam {
    int width = 0, height = 0; // dots across and down
    std::vector<std::vector<Vector2>> loops;
  };
  static std::vector<Kolam> kolams; // built once, the first time
  static int first = 0;             // the entry for FIRST_PAGE
  if (kolams.empty()) {
    for (size_t i = 0; i < Pages().size(); i++) {
      KolamGrid grid;
      if (!grid.Load(Pages()[i]) || grid.CountSolutions(true) == 0)
        continue;
      Kolam kolam;
      kolam.width = grid.width;
      kolam.height = grid.height;
      kolam.loops = LoopPaths(grid, {0, 0}, 1);
      if ((int)i == FIRST_PAGE)
        first = (int)kolams.size();
      kolams.push_back(kolam);
    }
  }

  DrawBandGradient(0, STAGE_SIZE, NIGHT_TOP, NIGHT_BOTTOM);
  if (kolams.empty())
    return;

  const int cycle = (int)(time / CYCLE);
  const Kolam &kolam = kolams[(first + cycle) % kolams.size()];
  const float t = time - cycle * CYCLE;
  const float drawn = std::fmin(t / KOLAM_DRAW_SECONDS, 1.0f);
  const float eased = Between(drawn, 0, 1);
  const float fadeStart = KOLAM_DRAW_SECONDS + KOLAM_HOLD_SECONDS;
  const float alpha =
      t < fadeStart ? 1.0f : 1.0f - (t - fadeStart) / KOLAM_FADE_SECONDS;

  const Rectangle area = HomeKolamArea();
  const float cell =
      std::fmin(std::fmin(area.width / kolam.width, area.height / kolam.height),
                HomeIsWide() ? 90.0f : 50.0f);
  const Vector2 origin = {area.x + (area.width - cell * kolam.width) / 2,
                          area.y + (area.height - cell * kolam.height) / 2};
  const float thick = HomeIsWide() ? 3.5f : 3.0f;
  for (int y = 0; y < kolam.height; y++)
    for (int x = 0; x < kolam.width; x++)
      DrawCircleV({origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell},
                  thick, Faded(FLOUR, 0.75f * alpha));
  for (const std::vector<Vector2> &unit : kolam.loops) {
    std::vector<Vector2> loop;
    for (const Vector2 &p : unit)
      loop.push_back({origin.x + p.x * cell, origin.y + p.y * cell});
    const size_t shown = (size_t)(eased * loop.size());
    DrawPath(loop, shown, drawn >= 1, thick * 3, Faded(FLOUR, 0.06f * alpha));
    DrawPath(loop, shown, drawn >= 1, thick, Faded(FLOUR, 0.9f * alpha));
    if (drawn < 1 && shown < loop.size())
      DrawCircleV(loop[shown], thick + 1, FLOUR); // the "hand" drawing it
  }
}
