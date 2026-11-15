// ui.cpp - fonts, menus, buttons, text and other small drawing helpers shared
// by all screens, plus finding the files that are built into the game.
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

void DrawString(const std::string &text, float x, float y, float size,
                Color color, FontStyle font) {
  DrawTextEx(fonts[font], text.c_str(), {x, y}, size, 0, color);
}

float MeasureString(const std::string &text, float size, FontStyle font) {
  return MeasureTextEx(fonts[font], text.c_str(), size, 0).x;
}

void DrawTextCentered(const std::string &text, float centerX, float y,
                      float size, Color color, FontStyle font) {
  DrawString(text, centerX - MeasureString(text, size, font) / 2, y, size,
             color, font);
}

// Draws text broken into lines no wider than maxWidth
void DrawTextWrapped(const std::string &text, float x, float y, float size,
                     float maxWidth, Color color, FontStyle font) {
  const float lineHeight = size * 1.15f;
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

void Menu::Set(std::vector<MenuItem> newItems, float newTop, float newWidth) {
  items = newItems;
  top = newTop;
  width = newWidth;
  selected = 0;
  while (selected < (int)items.size() - 1 && !items[selected].enabled)
    selected++;
}

Rectangle Menu::ItemRect(int i) const {
  return {STAGE_SIZE / 2.0f - width / 2, top + i * (ITEM_HEIGHT + ITEM_GAP),
          width, ITEM_HEIGHT};
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
  }

  // Mouse: hovering selects, clicking chooses
  const Vector2 mouse = GetMousePosition();
  const Vector2 moved = GetMouseDelta();
  for (int i = 0; i < count; i++) {
    if (!items[i].enabled || !CheckCollisionPointRec(mouse, ItemRect(i)))
      continue;
    if (moved.x != 0 || moved.y != 0)
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

void Menu::Draw() const {
  for (int i = 0; i < (int)items.size(); i++) {
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
  const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                       CheckCollisionPointRec(GetMousePosition(), rect);
  if (clicked)
    PlayEffect("click", 0.5f);
  return clicked;
}

bool ContinuePressed() {
  return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE) ||
         IsKeyPressed(KEY_ENTER);
}

bool BackPressed() {
  return IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) ||
         IsKeyPressed(KEY_BACK);
}

//------------------------------------------------------------------------------
// Dialogue
//------------------------------------------------------------------------------
void DrawDialogueBox(const Line &line, float typed) {
  DrawPanel({20, 430, 680, 200});
  if (!line.speaker.empty())
    DrawString(line.speaker, 50, 446, 24, KUMKUM);

  const size_t visible =
      typed < line.text.size() ? (size_t)typed : line.text.size();
  // narration is a little softer than people talking
  const Color textColor = line.speaker.empty() ? Faded(FLOUR, 0.8f) : FLOUR;
  DrawTextWrapped(line.text.substr(0, visible), 50, 478, 24, 620, textColor);

  if (line.hold > 0 && visible < line.text.size()) { // "hold to say it"
    const float pulse = 0.4f + 0.4f * std::sin((float)GetTime() * 4);
    DrawRing({676, 606}, 9, 12, 0, 360, 24, Faded(FLOUR, pulse));
  } else if (visible == line.text.size()) { // blinking "more" arrow
    const float blink = 0.5f + 0.5f * std::sin((float)GetTime() * 4);
    DrawString("v", 670, 600, 20, Faded(FLOUR, blink));
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
      if (held)
        typed += line.text.size() / line.hold * GetFrameTime();
      return false;
    }
  } else {
    typed += TextCharsPerSecond() * GetFrameTime();
  }
  if (!ContinuePressed())
    return false;

  if (typed < lines[current].text.size()) {
    typed = (float)lines[current].text.size(); // first: show the whole line
    return false;
  }
  if (current + 1 < lines.size()) { // then: the next line
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
void DrawHomeBackground(float time) {
  const float KOLAM_DRAW_SECONDS = 7.0f; // drawing the line
  const float KOLAM_HOLD_SECONDS = 2.5f; // admiring it
  const float KOLAM_FADE_SECONDS = 1.2f; // fading away before the next one
  const float CYCLE =
      KOLAM_DRAW_SECONDS + KOLAM_HOLD_SECONDS + KOLAM_FADE_SECONDS;
  const int FIRST_PAGE = 3; // start with page 4, the big festival kolam

  struct Kolam {
    std::vector<std::vector<Vector2>> loops;
    std::vector<Vector2> dots;
  };
  static std::vector<Kolam> kolams; // built once, the first time
  static int first = 0;             // the entry for FIRST_PAGE
  if (kolams.empty()) {
    for (size_t i = 0; i < Pages().size(); i++) {
      KolamGrid grid;
      if (!grid.Load(Pages()[i]) || grid.CountSolutions(true) == 0)
        continue;
      const float cell = std::fmin(
          std::fmin(340.0f / grid.width, 200.0f / grid.height), 60.0f);
      const Vector2 origin = {360 - cell * grid.width / 2,
                              272 - cell * grid.height / 2};
      Kolam kolam;
      kolam.loops = LoopPaths(grid, origin, cell);
      for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++)
          kolam.dots.push_back(
              {origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell});
      }
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

  for (const Vector2 &dot : kolam.dots)
    DrawCircleV(dot, 3, Faded(FLOUR, 0.75f * alpha));
  for (const std::vector<Vector2> &loop : kolam.loops) {
    const size_t shown = (size_t)(eased * loop.size());
    DrawPath(loop, shown, drawn >= 1, 9, Faded(FLOUR, 0.06f * alpha)); // glow
    DrawPath(loop, shown, drawn >= 1, 3, Faded(FLOUR, 0.9f * alpha));
    if (drawn < 1 && shown < loop.size())
      DrawCircleV(loop[shown], 4, FLOUR); // the "hand" drawing it
  }
}
