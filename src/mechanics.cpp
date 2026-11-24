// mechanics.cpp - the Mechanics page, from the main menu: every game in the
// game, all open from the start, to play outside the story. One page, two
// rows of cards:
// - Amma's kolam book: each page as it starts (a wall page with its separate
//   coloured lines, a drawing page with only the dots and Amma's ink).
// - Other kolams (kolam_kinds.cpp): Amma's drawing of each kind.
// Choosing a card opens it (main.cpp plays it); finishing it gets a tick on
// its card until the game closes. The cards are as big as the screen allows:
// large on a wide screen, smaller on a square one.
// It looks like Amma's pages spread out on a teak table under a lamp: the
// wood grain and the paper's grain and aged edges are made once, in code.
#include <algorithm>
#include <cmath>

#include "game.h"
#include "rlgl.h"

namespace {

// The cards look like the book's own pages (puzzle_scene.cpp)
const Color CARD_PAPER = {243, 234, 210, 255};
const Color CARD_NOTEBOOK = {250, 247, 238, 255};
const Color CARD_INK = {40, 42, 92, 255};
const Color CARD_CRAYON = {150, 60, 160, 255};
const Color CARD_FAINT = {120, 100, 80, 255};
const Color CARD_WALL = {105, 70, 50, 255};
const Color LINE_INKS[] = {{150, 40, 55, 255},  {40, 90, 150, 255},
                           {30, 125, 110, 255}, {190, 125, 20, 255},
                           {85, 120, 40, 255},  {120, 60, 140, 255}};
const Color LINE_CRAYONS[] = {{235, 110, 40, 255}, {225, 70, 130, 255},
                              {60, 160, 80, 255},  {50, 120, 220, 255},
                              {200, 170, 30, 255}, {140, 80, 200, 255}};
const int LINE_COLOURS = sizeof(LINE_INKS) / sizeof(LINE_INKS[0]);

// The layout: two rows of cards, each under its label
const int ROW_CARDS = 7;    // the most cards in a row (the kolam book's)
const float CARD_GAP = 12;  // between two cards
const float MAX_CARD_W = 160;
const float CARD_SHAPE = 1.3f; // height / width
const float ROW1_LABEL_Y = 70, ROW_LABEL_SPACE = 24;
const Rectangle BACK_BUTTON = {280, 598, 160, 50};
const char *ROW_LABELS[2] = {"Amma's kolam book", "Other kolams"};

// A kolam book page as it starts, in "cells" (the distance between two dots,
// from the top-left of its grid), fitted into the card when drawn
struct BookPreview {
  int width = 0, height = 0; // dots across and down
  std::vector<std::vector<Vector2>> lines;
  std::vector<std::pair<Vector2, Vector2>> walls; // Amma's ink (drawing pages)
};

struct Card {
  int page = -1; // a kolam book page, or
  int kind = -1; // another kind of kolam
  BookPreview preview;
};

//------------------------------------------------------------------------------
// Textures, made once: teak planks, and paper (white, tinted when drawn)
//------------------------------------------------------------------------------
Texture2D wood = {}, paper = {};

float Noise(int x, int y) { // the same "random" number for the same x, y
  unsigned int n = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u;
  n = (n ^ (n >> 13)) * 1274126177u;
  return (float)((n ^ (n >> 16)) & 0xffff) / 65535.0f;
}

Texture2D MakeWood() {
  const int W = 512, H = 288, PLANK = 96;
  Image image = GenImageColor(W, H, BLACK);
  Color *pixels = (Color *)image.data;
  const Color dark = {46, 29, 19, 255}, light = {92, 60, 38, 255};
  for (int y = 0; y < H; y++) {
    const int plank = y / PLANK;
    for (int x = 0; x < W; x++) {
      // grain running along the plank, wavy, different in each plank
      const float t = y * 0.55f + 7 * std::sin(x * 0.006f + plank * 2.1f) +
                      2.5f * std::sin(x * 0.021f + y * 0.05f) + plank * 13;
      float g = 0.5f + 0.5f * std::sin(t);
      g = g * g * g;
      const float fine = Noise(x / 3, y) * 0.18f + Noise(x, y) * 0.08f;
      float v = std::fmin(1.0f, 0.35f + 0.5f * g + fine);
      if (y % PLANK < 2 || y % PLANK > PLANK - 2)
        v *= 0.45f; // the gap between two planks
      pixels[y * W + x] = {(unsigned char)(dark.r + (light.r - dark.r) * v),
                           (unsigned char)(dark.g + (light.g - dark.g) * v),
                           (unsigned char)(dark.b + (light.b - dark.b) * v), 255};
    }
  }
  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);
  SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
  return texture;
}

// Grainy paper with fibres, darker towards its edges, with rounded corners
Texture2D MakePaper() {
  const int W = 256, H = 332, CORNER = 9;
  Image image = GenImageColor(W, H, WHITE);
  Color *pixels = (Color *)image.data;
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      const float edge = std::fmin(std::fmin(x, W - 1 - x),
                                   std::fmin(y, H - 1 - y)) / 22.0f;
      const float aged = 0.86f + 0.14f * std::fmin(1.0f, edge);
      const float grain = 0.95f + 0.05f * Noise(x, y) +
                          0.025f * Noise(x / 4, y / 4);
      const unsigned char v =
          (unsigned char)(255 * std::fmin(1.0f, aged * grain));
      // a touch warmer where it's aged
      pixels[y * W + x] = {v, (unsigned char)(v * (0.985f + 0.015f * aged)),
                           (unsigned char)(v * (0.95f + 0.05f * aged)), 255};
      // rounded corners
      const int cx = x < CORNER ? CORNER : x >= W - CORNER ? W - 1 - CORNER : x;
      const int cy = y < CORNER ? CORNER : y >= H - CORNER ? H - 1 - CORNER : y;
      if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > CORNER * CORNER)
        pixels[y * W + x].a = 0;
    }
  }
  for (int i = 0; i < 220; i++) { // fibres
    const int x = GetRandomValue(0, W - 1), y = GetRandomValue(0, H - 1);
    const int dx = GetRandomValue(-6, 6), dy = GetRandomValue(-3, 3);
    ImageDrawLine(&image, x, y, x + dx, y + dy, {222, 214, 196, 255});
  }
  for (int i = 0; i < 5; i++) { // a few old spots
    const int x = GetRandomValue(20, W - 20), y = GetRandomValue(20, H - 20);
    ImageDrawCircle(&image, x, y, GetRandomValue(2, 5), {232, 220, 196, 255});
  }
  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);
  SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
  return texture;
}

void DrawStretched(const Texture2D &t, Rectangle where, Color tint) {
  DrawTexturePro(t, {0, 0, (float)t.width, (float)t.height}, where, {0, 0}, 0,
                 tint);
}

// The table: teak planks, a lamp's warm light, darker towards the edges
void DrawTable() {
  if (wood.id == 0)
    wood = MakeWood();
  const Rectangle screen = FullScreen();
  DrawStretched(wood, screen, WHITE);
  DrawCircleGradient({360, 300}, 620, {255, 214, 150, 46}, {255, 214, 150, 0});
  const float edge = 160;
  DrawRectangleGradientV((int)screen.x, 720 - (int)edge, (int)screen.width,
                         (int)edge, {0, 0, 0, 0}, {0, 0, 0, 120});
  DrawRectangleGradientV((int)screen.x, 0, (int)screen.width, 90,
                         {0, 0, 0, 110}, {0, 0, 0, 0});
  DrawRectangleGradientH((int)screen.x, 0, (int)edge, 720, {0, 0, 0, 110},
                         {0, 0, 0, 0});
  DrawRectangleGradientH((int)(screen.x + screen.width - edge), 0, (int)edge,
                         720, {0, 0, 0, 0}, {0, 0, 0, 110});
}

// A line of rice-flour dots, with a small loop round every other one
void DrawDotLine(float x1, float x2, float y, float alpha) {
  for (float x = x1; x <= x2; x += 14)
    DrawCircleV({x, y}, 1.6f, Faded(FLOUR, 0.5f * alpha));
}

std::vector<Card> cards; // the book's pages, then the other kolams
std::vector<bool> done;  // ticks, this time the game is open
int selected = 0;        // a card, or cards.size() for Back

int BookCount() { return (int)Pages().size(); }
int Count() { return (int)cards.size(); }
int RowOf(int i) { return i < BookCount() ? 0 : 1; }
int RowStart(int row) { return row == 0 ? 0 : BookCount(); }
int RowSize(int row) { return row == 0 ? BookCount() : KOLAM_KIND_COUNT; }

float CardWidth() {
  const Rectangle screen = FullScreen();
  const float room = screen.width - 40 - (ROW_CARDS - 1) * CARD_GAP;
  return std::fmin(MAX_CARD_W, room / ROW_CARDS);
}

Rectangle CardRect(int i) {
  const float w = CardWidth(), h = w * CARD_SHAPE;
  const int row = RowOf(i);
  const int inRow = RowSize(row);
  const float rowWidth = inRow * w + (inRow - 1) * CARD_GAP;
  // the two rows sit in the middle of the room between the title and Back
  // (they fill it on a wide screen; on a square one the cards are smaller)
  const float rows = 2 * (h + ROW_LABEL_SPACE) + 22;
  const float room = BACK_BUTTON.y - 12 - ROW1_LABEL_Y;
  const float top = ROW1_LABEL_Y + std::fmax(0.0f, (room - rows) / 2);
  const float rowTop = top + ROW_LABEL_SPACE + row * (h + 22 + ROW_LABEL_SPACE);
  return {360 - rowWidth / 2 + (i - RowStart(row)) * (w + CARD_GAP), rowTop,
          w, h};
}

BookPreview MakePreview(int i) {
  BookPreview preview;
  const Page &page = Pages()[i];
  KolamGrid grid;
  if (!grid.Load(page))
    return preview; // a torn page shows blank; CheckPages() reports it
  preview.width = grid.width;
  preview.height = grid.height;
  if (!page.draw)
    preview.lines = LoopPaths(grid, {0, 0}, 1, page.byLittleAnu ? 0.012f : 0);
  for (int y = 1; page.draw && y < 2 * grid.height; y++) {
    for (int x = 1; x < 2 * grid.width; x++) {
      if (!grid.IsBetweenDots(x, y) || !grid.IsWall(x, y) ||
          grid.IsEditable(x, y))
        continue;
      const Vector2 c = {x / 2.0f, y / 2.0f};
      if (x % 2 == 0) // between left and right dots: an upright bar
        preview.walls.push_back({{c.x, c.y - 0.275f}, {c.x, c.y + 0.275f}});
      else
        preview.walls.push_back({{c.x - 0.275f, c.y}, {c.x + 0.275f, c.y}});
    }
  }
  return preview;
}

// Arrow keys: along a row, and between the rows and Back
void MoveSelection() {
  const int back = Count();
  auto nearestIn = [&](int row) { // the card in `row` nearest the chosen one
    const Rectangle from = CardRect(std::min(selected, back - 1));
    int best = RowStart(row);
    for (int i = RowStart(row); i < RowStart(row) + RowSize(row); i++)
      if (std::fabs(CardRect(i).x - from.x) <
          std::fabs(CardRect(best).x - from.x))
        best = i;
    return best;
  };
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    selected = std::min(selected + 1, back);
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    selected = std::max(selected - 1, 0);
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_TAB))
    selected = selected >= back ? back : RowOf(selected) == 0 ? nearestIn(1)
                                                               : back;
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    selected = selected >= back ? nearestIn(1)
               : RowOf(selected) == 1 ? nearestIn(0)
                                      : selected;
}

// The size text fits in `width`, from `size` down
float FitSize(const std::string &text, float size, float width,
              FontStyle font) {
  while (size > 9 && MeasureString(text, size, font) > width)
    size -= 0.5f;
  return size;
}

void DrawBookPreview(const Card &card, Rectangle area) {
  const BookPreview &p = card.preview;
  if (p.width == 0)
    return;
  const Page &page = Pages()[card.page];
  const float cell = std::fmin(area.width / p.width, area.height / p.height);
  const Vector2 at = {area.x + (area.width - cell * p.width) / 2,
                      area.y + (area.height - cell * p.height) / 2};
  auto place = [&](Vector2 v) {
    return Vector2{at.x + v.x * cell, at.y + v.y * cell};
  };
  const float thick = std::fmax(1.3f, cell * 0.05f);
  for (size_t l = 0; l < p.lines.size(); l++) {
    std::vector<Vector2> line;
    for (const Vector2 &v : p.lines[l])
      line.push_back(place(v));
    const int c = (int)l % LINE_COLOURS;
    DrawPath(line, line.size(), true, thick,
             page.byLittleAnu ? LINE_CRAYONS[c] : LINE_INKS[c]);
  }
  for (const auto &wall : p.walls)
    DrawLineEx(place(wall.first), place(wall.second), thick, CARD_WALL);
  for (int y = 0; y < p.height; y++)
    for (int x = 0; x < p.width; x++)
      DrawCircleV(place({x + 0.5f, y + 0.5f}), std::fmax(1.2f, cell * 0.07f),
                  page.byLittleAnu ? CARD_CRAYON : CARD_FAINT);
}

void DrawCard(int i, float time) {
  const Card &card = cards[i];
  const Rectangle r = CardRect(i);
  const bool book = card.page >= 0;
  const Page *page = book ? &Pages()[card.page] : nullptr;
  const bool notebook = book && page->byLittleAnu;
  const float k = r.width / MAX_CARD_W; // how big, of the biggest

  if (paper.id == 0)
    paper = MakePaper();
  const bool chosen = i == selected;
  // Each card lies a little crooked; the chosen one lifts off the table
  // (only how it looks: clicking still uses its straight rectangle)
  const float tilt = chosen ? 0 : ((i * 37) % 7 - 3) * 0.45f;
  const float lift = chosen ? 5 * k + 1 : 0;
  rlPushMatrix();
  rlTranslatef(r.x + r.width / 2, r.y + r.height / 2 - lift, 0);
  rlRotatef(tilt, 0, 0, 1);
  rlTranslatef(-(r.x + r.width / 2), -(r.y + r.height / 2), 0);

  // a soft shadow, further away when lifted
  const float drop = 3 + lift * 1.6f;
  DrawRectangleRounded({r.x + 1, r.y + drop + 2, r.width + 2, r.height}, 0.06f,
                       6, {0, 0, 0, 50});
  DrawRectangleRounded({r.x + 2, r.y + drop, r.width, r.height}, 0.06f, 6,
                       {0, 0, 0, 70});
  if (chosen) { // a warm glow around it, gently pulsing
    const float pulse = 0.75f + 0.25f * std::sin(time * 4);
    for (int g = 3; g >= 1; g--)
      DrawRectangleRounded({r.x - 3 * g, r.y - 3 * g, r.width + 6 * g,
                            r.height + 6 * g},
                           0.1f, 6, Faded(KUMKUM, pulse * 0.28f));
    DrawRectangleRounded({r.x - 3, r.y - 3, r.width + 6, r.height + 6}, 0.08f,
                         6, Faded(KUMKUM, pulse));
  }
  DrawStretched(paper, r, notebook ? CARD_NOTEBOOK : CARD_PAPER);
  if (notebook) { // ruled lines and a margin, like her notebook
    for (float y = r.y + 18 * k; y < r.y + r.height - 6; y += 14 * k)
      DrawLineEx({r.x + 3, y}, {r.x + r.width - 3, y}, 1, {170, 200, 230, 120});
    DrawLineEx({r.x + 12 * k, r.y + 3}, {r.x + 12 * k, r.y + r.height - 3}, 1,
               {230, 140, 140, 150});
  }

  // the picture, then a small word and the name under it
  const float margin = 10 * k;
  const Rectangle picture = {r.x + margin, r.y + margin, r.width - 2 * margin,
                             r.width - 2 * margin};
  if (book)
    DrawBookPreview(card, picture);
  else
    DrawAmmasKolam(card.kind, picture);
  const std::string tag =
      book ? std::to_string(card.page + 1) + (page->draw ? " - draw" : " - walls")
           : KolamKindTag(card.kind);
  const std::string name = book ? page->title : KolamKindName(card.kind);
  const float textWidth = r.width - 8;
  const float tagSize = FitSize(tag, 15 * k + 2, textWidth, UI_FONT);
  const float nameSize = FitSize(name, 20 * k + 2, textWidth, HAND_FONT);
  const float textTop = picture.y + picture.height + 4 * k;
  DrawTextCentered(tag, r.x + r.width / 2, textTop, tagSize, CARD_FAINT);
  DrawTextCentered(name, r.x + r.width / 2, textTop + tagSize + 3 * k,
                   nameSize, notebook ? CARD_CRAYON : CARD_INK, HAND_FONT);

  if (!book) { // Amma's notes are taped down
    DrawRectangle((int)(r.x + r.width / 2 - 20 * k), (int)(r.y - 6 * k),
                  (int)(40 * k), (int)(14 * k), {236, 226, 190, 190});
  }
  if (done[i]) { // a tick in the corner
    const float s = std::fmax(7.0f, 12 * k);
    const Vector2 c = {r.x + r.width - s - 2, r.y + s + 2};
    DrawCircleV(c, s, KUMKUM);
    DrawLineEx({c.x - s / 2, c.y}, {c.x - s / 8, c.y + s * 0.42f}, s / 4, FLOUR);
    DrawLineEx({c.x - s / 8, c.y + s * 0.42f}, {c.x + s * 0.55f, c.y - s * 0.42f},
               s / 4, FLOUR);
  }
  rlPopMatrix();
}

// What UpdateMechanicsPage() returns for card i
int CodeOf(int i) {
  return cards[i].page >= 0 ? cards[i].page : MECHANICS_KINDS + cards[i].kind;
}

} // namespace

void OpenMechanicsPage(int selectPage) {
  if (cards.empty()) {
    for (int i = 0; i < BookCount(); i++) {
      Card card;
      card.page = i;
      card.preview = MakePreview(i);
      cards.push_back(card);
    }
    for (int k = 0; k < KOLAM_KIND_COUNT; k++) {
      Card card;
      card.kind = k;
      cards.push_back(card);
    }
    done.assign(cards.size(), false);
  }
  // `selectPage`: a page, or MECHANICS_KINDS + a kind (as Update returns);
  // the page after the last one is the first other kolam
  const int i = selectPage >= MECHANICS_KINDS
                    ? BookCount() + selectPage - MECHANICS_KINDS
                    : selectPage;
  selected = std::max(0, std::min(i, Count() - 1));
}

void MarkPageSolved(int page) {
  for (int i = 0; i < Count(); i++)
    if (CodeOf(i) == page)
      done[i] = true;
}

int UpdateMechanicsPage() {
  MoveSelection();
  const Vector2 mouse = GetMousePosition();
  const Vector2 moved = GetMouseDelta();
  for (int i = 0; i < Count(); i++) {
    const Rectangle r = CardRect(i);
    if (!CheckCollisionPointRec(mouse, r))
      continue;
    if (moved.x != 0 || moved.y != 0)
      selected = i;
    if (Clicked(r))
      return CodeOf(i);
  }
  if (CheckCollisionPointRec(mouse, BACK_BUTTON) &&
      (moved.x != 0 || moved.y != 0))
    selected = Count();
  if (Clicked(BACK_BUTTON))
    return MECHANICS_BACK;
  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
    PlayEffect("click", 0.5f);
    return selected >= Count() ? MECHANICS_BACK : CodeOf(selected);
  }
  return -1;
}

void DrawMechanicsPage(float time) {
  DrawTable();
  DrawTextCentered("Mechanics", 360, 10, 46, FLOUR, HAND_FONT);
  // under the title, a little sikku-style flourish of flour
  for (int j = -3; j <= 3; j++)
    DrawCircleV({360.0f + j * 16, 64}, 1.8f, Faded(FLOUR, 0.7f));
  for (int j = -3; j < 3; j++)
    DrawRing({368.0f + j * 16, 64}, 6, 7.4f, j % 2 ? 0 : 180,
             j % 2 ? 180 : 360, 12, Faded(FLOUR, 0.45f));
  for (int row = 0; row < 2; row++) { // each row's label, above its first card
    const Rectangle first = CardRect(RowStart(row));
    const Rectangle last = CardRect(RowStart(row) + RowSize(row) - 1);
    const float y = first.y - ROW_LABEL_SPACE;
    DrawString(ROW_LABELS[row], first.x, y - 3, 21, Faded(FLOUR, 0.9f), HAND_FONT);
    const float after = first.x + MeasureString(ROW_LABELS[row], 21, HAND_FONT);
    DrawDotLine(after + 14, last.x + last.width, y + 11, 1);
  }
  for (int i = 0; i < Count(); i++)
    DrawCard(i, time);
  // Back looks like the menus' buttons
  if (selected >= Count()) {
    DrawRectangleRounded(BACK_BUTTON, 0.3f, 8, KUMKUM);
  } else {
    DrawRectangleRounded(BACK_BUTTON, 0.3f, 8, {30, 20, 14, 150});
    DrawRectangleRoundedLinesEx(BACK_BUTTON, 0.3f, 8, 1, Faded(FLOUR, 0.35f));
  }
  DrawTextCentered("Back", 360, BACK_BUTTON.y + 11, 25,
                   selected >= Count() ? WHITE : FLOUR);
  DrawTextCentered(TouchUi() ? "Back button to go back" : "Esc to go back",
                   360, 660, 15, Faded(FLOUR, 0.45f));
}
