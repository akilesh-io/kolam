// finale.h - small helpers shared by the last three chapters (ch7_dream.cpp,
// ch8_morning.cpp, ch9_carry.cpp): comic speech bubbles, a little
// conversation that types itself out, crayon strokes, a kolam on the ground
// and a small flower.
#pragma once

#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "game.h"
#include "rlgl.h"

namespace finale {

// A steady pseudo-random number in -1..1 for (seed, i): the same every frame
inline float Noise(float seed, float i) {
  const float s = std::sin(seed * 12.9898f + i * 78.233f) * 43758.547f;
  return (s - std::floor(s)) * 2 - 1;
}

//------------------------------------------------------------------------------
// Speech bubbles, as the comic prints them: a white balloon with an inked
// edge and a tail pointing at whoever is talking. No names, the tail says who.
//------------------------------------------------------------------------------
// How wide text is drawn. (MeasureString leaves out the spaces, which the
// font does draw.)
inline float TextWidth(const std::string &text, float size) {
  float spaces = 0;
  for (char c : text)
    spaces += c == ' ' ? 1 : 0;
  return MeasureString(text, size) + spaces * size * 0.3f;
}

inline std::vector<std::string> WrapWords(const std::string &text, float size,
                                          float maxWidth) {
  std::vector<std::string> rows;
  std::string row, word;
  auto flush = [&]() {
    if (word.empty())
      return;
    const std::string tried = row.empty() ? word : row + " " + word;
    if (!row.empty() && TextWidth(tried, size) > maxWidth) {
      rows.push_back(row);
      row = word;
    } else {
      row = tried;
    }
    word.clear();
  };
  for (char c : text) {
    if (c == ' ')
      flush();
    else
      word += c;
  }
  flush();
  if (!row.empty())
    rows.push_back(row);
  return rows;
}

// `typed`: how many characters show. `tail`: the speaker's mouth (or a point
// off the screen's edge for Anu herself). The bubble sits at `centre`, kept
// on the stage. `hold`: a hard line still being said (a pulsing ring).
// `onStage`: kept inside the 720x720 stage (for anything the player must
// read on every screen shape); otherwise only inside the screen.
inline void DrawBubble(const std::string &text, float typed, Vector2 centre,
                       Vector2 tail, bool hold, float alpha = 1,
                       bool onStage = true) {
  const float size = 23, maxWidth = 300, padX = 20, padY = 12;
  const std::vector<std::string> rows = WrapWords(text, size, maxWidth);
  float width = 40;
  for (const std::string &r : rows)
    width = std::fmax(width, TextWidth(r, size));
  const float height = rows.size() * (size + 4) + padY * 2;
  Rectangle box = {centre.x - width / 2 - padX, centre.y - height / 2,
                   width + padX * 2, height};
  const Rectangle all = FullScreen();
  const float minX = onStage ? 8 : all.x + 8;
  const float maxX = onStage ? 712 : all.x + all.width - 8;
  box.x = std::fmax(minX, std::fmin(maxX - box.width, box.x));
  const Color paper = Faded(COMIC_PAPER, alpha), ink = Faded(COMIC_INK, alpha);
  // the tail leaves the balloon on the side nearest the speaker
  const float bx = std::fmax(box.x + 24, std::fmin(box.x + box.width - 24,
                                                     tail.x));
  const bool below = tail.y > box.y + box.height / 2;
  const float by = below ? box.y + box.height - 4 : box.y + 4;
  const Vector2 b1 = {bx - 11, by}, b2 = {bx + 11, by};
  const Vector2 tip = {tail.x, tail.y};
  DrawRectangleRounded({box.x + 4, box.y + 5, box.width, box.height}, 0.45f,
                       10, Faded({0, 0, 0, 255}, 0.18f * alpha)); // shadow
  DrawRectangleRounded(box, 0.45f, 10, paper);
  DrawRectangleRoundedLinesEx(box, 0.45f, 10, 3, ink);
  FillTriangle(b1, b2, tip, paper);
  DrawLineEx(b1, tip, 3, ink);
  DrawLineEx(b2, tip, 3.6f, ink);
  // the words, typed out
  size_t left = typed < 0 ? 0 : (size_t)typed;
  for (size_t i = 0; i < rows.size(); i++) {
    const std::string shown = rows[i].substr(0, std::min(left, rows[i].size()));
    left = left > rows[i].size() + 1 ? left - rows[i].size() - 1 : 0;
    DrawString(shown, box.x + padX, box.y + padY + i * (size + 4), size, ink);
  }
  const float pulse = 0.45f + 0.4f * std::sin((float)GetTime() * 4);
  const Vector2 corner = {box.x + box.width - 14, box.y + box.height - 12};
  if (hold) // "hold to say it"
    DrawRing(corner, 6, 9, 0, 360, 20, Faded(KUMKUM, pulse * alpha));
}

// Whoever is talking: where the balloon goes and where its tail points
struct BubbleSpot {
  Vector2 centre;
  Vector2 tail;
};

// A few lines said one after another, in bubbles. Click to go on; a line
// with `hold` only comes out while the button is held (AdvanceDialogue).
struct Conversation {
  std::vector<Line> lines;
  size_t current = 0;
  float typed = 0;
  bool active = false;
  bool fresh = false; // started this frame: the click that started it
                      // mustn't also skip its first line

  void Start(const std::vector<Line> &say) {
    lines = say;
    current = 0;
    typed = 0;
    active = !lines.empty();
    fresh = true;
  }
  // True on the frame the last line is dismissed
  bool Update() {
    if (!active)
      return false;
    if (fresh) {
      fresh = false;
      return false;
    }
    if (AdvanceDialogue(lines, current, typed)) {
      active = false;
      return true;
    }
    return false;
  }
  const Line *Shown() const { return active ? &lines[current] : nullptr; }
  void Draw(const std::function<BubbleSpot(const std::string &)> &spot) const {
    const Line *line = Shown();
    if (!line)
      return;
    const BubbleSpot s = spot(line->speaker);
    DrawBubble(line->text, typed, s.centre, s.tail,
               line->hold > 0 && typed < line->text.size());
  }
};

// Lines [from, to) of a step's lines (scenes split them between moments)
inline std::vector<Line> Lines(const std::vector<Line> &all, size_t from,
                               size_t to) {
  std::vector<Line> part;
  for (size_t i = from; i < to && i < all.size(); i++)
    part.push_back(all[i]);
  return part;
}

// A pulsing ring: "you can touch this"
inline void DrawHint(Vector2 at, float r, float time, Color c = COMIC_PAPER) {
  const float pulse = 0.5f + 0.5f * std::sin(time * 4);
  DrawRing(at, r + pulse * 4, r + 3 + pulse * 4, 0, 360, 32,
           Faded(c, 0.35f + 0.4f * pulse));
}

// Draws `draw` mirrored left to right around x = `x` (for people who should
// face the other way)
inline void Mirrored(float x, const std::function<void()> &draw) {
  rlDrawRenderBatchActive();
  rlDisableBackfaceCulling();
  rlPushMatrix();
  rlTranslatef(2 * x, 0, 0);
  rlScalef(-1, 1, 1);
  draw();
  rlPopMatrix();
  rlDrawRenderBatchActive();
  rlEnableBackfaceCulling();
}

//------------------------------------------------------------------------------
// Crayon: wax on paper. A line is a few thin, slightly offset strokes, and
// it "boils" (wobbles a little, a few times a second) as if it were alive.
//------------------------------------------------------------------------------
inline void CrayonLine(Vector2 a, Vector2 b, float width, Color c,
                       float seed) {
  const float dx = b.x - a.x, dy = b.y - a.y;
  const float len = std::fmax(0.001f, std::sqrt(dx * dx + dy * dy));
  const Vector2 n = {-dy / len, dx / len};
  for (int k = 0; k < 3; k++) {
    const float off = (k - 1) * width * 0.32f + Noise(seed, k) * 0.6f;
    const float w = k == 1 ? width * 0.7f : width * 0.38f;
    DrawLineEx({a.x + n.x * off, a.y + n.y * off},
               {b.x + n.x * off, b.y + n.y * off}, w,
               Faded(c, k == 1 ? 0.92f : 0.55f));
  }
}

// A wobbly crayon path through `points` (closed: back to the start). `boil`
// changes the wobble; keep it whole numbers (e.g. (int)(time * 5)).
inline void CrayonPath(const std::vector<Vector2> &points, bool closed,
                       float width, Color c, float seed, int boil = 0) {
  const size_t n = points.size();
  if (n < 2)
    return;
  auto at = [&](size_t i) {
    const Vector2 p = points[i % n];
    const float s = seed + boil * 3.17f;
    return Vector2{p.x + Noise(s, i * 2.0f) * 1.6f,
                   p.y + Noise(s, i * 2.0f + 1) * 1.6f};
  };
  const size_t last = closed ? n : n - 1;
  for (size_t i = 0; i < last; i++)
    CrayonLine(at(i), at(i + 1), width, c, seed + i);
}

// Points around an ellipse, a little uneven, the way a child draws a circle
inline std::vector<Vector2> CrayonEllipse(Vector2 c, float rx, float ry,
                                          float seed, int count = 24) {
  std::vector<Vector2> p;
  for (int i = 0; i < count; i++) {
    const float a = i * 2 * PI / count;
    const float wob = 1 + 0.04f * Noise(seed, (float)i);
    p.push_back({c.x + rx * wob * std::cos(a), c.y + ry * wob * std::sin(a)});
  }
  return p;
}

// A small five-petal flower, drawn by hand (in flour, crayon or pencil).
// `grown` 0..1 draws it petal by petal.
inline void DrawLittleFlower(Vector2 c, float r, Color ink, float width,
                             float grown = 1, float turn = 0) {
  for (int p = 0; p < 5; p++) {
    const float show = std::fmin(1, std::fmax(0, grown * 6 - p));
    if (show <= 0)
      break;
    const float a = turn + p * 2 * PI / 5 - PI / 2;
    const Vector2 m = {c.x + r * std::cos(a), c.y + r * std::sin(a)};
    DrawRing(m, r * 0.55f - width / 2, r * 0.55f + width / 2, 0, 360 * show,
             16, ink);
  }
  if (grown > 0.9f)
    DrawCircleV(c, width * 0.9f, ink);
}

//------------------------------------------------------------------------------
// The BIG one (page 6 of the kolam book), solved: the kolam little Anu meant
// to finish when she was big. Its lines, for drawing anywhere.
//------------------------------------------------------------------------------
inline std::vector<std::vector<Vector2>> SolvedPage(int page, Vector2 origin,
                                                    float cell, float wobble,
                                                    std::vector<Vector2> *dots,
                                                    int *across = nullptr,
                                                    int *down = nullptr) {
  KolamGrid grid;
  if (!grid.Load(Pages()[page]) || grid.CountSolutions(true) == 0)
    return {};
  if (dots) {
    for (int y = 0; y < grid.height; y++)
      for (int x = 0; x < grid.width; x++)
        dots->push_back(
            {origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell});
  }
  if (across)
    *across = grid.width;
  if (down)
    *down = grid.height;
  return LoopPaths(grid, origin, cell, wobble);
}

// Squashes a drawing onto the ground at `at`, seen at a low angle
inline Vector2 OnGround(Vector2 p, Vector2 centre, Vector2 at, float scale,
                        float squash = 0.42f) {
  return {at.x + (p.x - centre.x) * scale,
          at.y + (p.y - centre.y) * scale * squash};
}

// Anu's kolam on the doorstep: the BIG one, in rice flour, squashed onto the
// ground at `at` (about `width` across), with a small flower in its corner.
// `rubbed` 0..1: days of feet have worn it away (the flower lasts longest).
inline void DrawGroundKolam(Vector2 at, float width, float rubbed = 0,
                            float flowerGlint = 0) {
  static std::vector<std::vector<Vector2>> loops;
  static std::vector<Vector2> dots;
  static Vector2 centre = {0, 0};
  static Vector2 corner = {0, 0};
  if (loops.empty()) {
    int across = 0, down = 0;
    loops = SolvedPage(5, {0, 0}, 40, 0.8f, &dots, &across, &down);
    centre = {across * 20.0f, down * 20.0f};
    corner = {across * 40.0f + 6, down * 40.0f + 4};
  }
  const float scale = width / (centre.x * 2);
  const float squash = 0.36f;
  const Color flour = {252, 248, 236, 255};
  for (const std::vector<Vector2> &loop : loops) {
    for (size_t i = 0; i + 1 < loop.size(); i++) {
      // rubbed away in patches, where feet went
      if (rubbed > 0 &&
          Noise(std::floor(i / 9.0f), 3) * 0.5f + 0.5f < rubbed * 0.85f)
        continue;
      DrawLineEx(OnGround(loop[i], centre, at, scale, squash),
                 OnGround(loop[i + 1], centre, at, scale, squash),
                 2.6f * (1 - 0.3f * rubbed), Faded(flour, 1 - 0.4f * rubbed));
    }
  }
  for (size_t i = 0; i < dots.size(); i++) {
    if (rubbed > 0 && Noise((float)i, 9) * 0.5f + 0.5f < rubbed * 0.7f)
      continue;
    DrawCircleV(OnGround(dots[i], centre, at, scale, squash), 2, flour);
  }
  // the flower, in the corner: hers
  const Vector2 f = OnGround(corner, centre, at, scale, squash);
  rlPushMatrix();
  rlTranslatef(f.x, f.y, 0);
  rlScalef(1, 0.6f, 1);
  DrawLittleFlower({0, 0}, 8, Faded(flour, 1 - 0.25f * rubbed), 2.6f);
  rlPopMatrix();
  if (flowerGlint > 0)
    DrawSparkle({f.x + 6, f.y - 8}, 9 * flowerGlint, {255, 252, 230, 255});
}

// Where DrawGroundKolam puts the flower, for pointing at it
inline Vector2 GroundKolamFlower(Vector2 at, float width) {
  int across = 7, down = 5; // the BIG one
  const Vector2 centre = {across * 20.0f, down * 20.0f};
  return OnGround({across * 40.0f + 6, down * 40.0f + 4}, centre, at,
                  width / (centre.x * 2), 0.36f);
}

// A soft fade over the whole screen
inline void FadeScreen(Color c, float amount) {
  if (amount > 0)
    DrawRectangleRec(FullScreen(), Faded(c, amount));
}

} // namespace finale
