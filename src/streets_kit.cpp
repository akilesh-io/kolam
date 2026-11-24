// streets_kit.cpp - the helpers in streets_kit.h: the village street as a
// Tamil comic, speech balloons, children and village folk, lamps, paper
// windmills, the temple tower. Used by Chapters 3 and 6.
#include "streets_kit.h"

#include <algorithm>
#include <cmath>

namespace streets {

Vector2 Lerp2(Vector2 a, Vector2 b, float t) {
  return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

float Approach(float value, float target, float rate) {
  return value + (target - value) * std::fmin(1.0f, rate * GetFrameTime());
}

Line LineAt(const std::vector<Line> &lines, size_t i, const Line &fallback) {
  return i < lines.size() ? lines[i] : fallback;
}

// A number from 0 to 1 that looks random but is always the same for `i`
static float Hash(float i) {
  const float s = std::sin(i * 12.9898f + 78.233f) * 43758.5453f;
  return s - std::floor(s);
}

//------------------------------------------------------------------------------
// Balloons
//------------------------------------------------------------------------------
const float BALLOON_TEXT = 24;
const float BALLOON_WIDTH = 300; // the most the words take across

static std::vector<std::string> Wrap(const std::string &text) {
  std::vector<std::string> rows;
  std::string row, word;
  for (size_t i = 0; i <= text.size(); i++) {
    if (i < text.size() && text[i] != ' ') {
      word += text[i];
      continue;
    }
    const std::string tried = row.empty() ? word : row + " " + word;
    if (!row.empty() && MeasureString(tried, BALLOON_TEXT) > BALLOON_WIDTH) {
      rows.push_back(row);
      row = word;
    } else {
      row = tried;
    }
    word.clear();
  }
  if (!row.empty())
    rows.push_back(row);
  return rows;
}

void DrawBalloon(const Line &line, float typed, Vector2 mouth, float time) {
  const std::vector<std::string> rows = Wrap(line.text);
  float textWidth = 40;
  for (const std::string &row : rows)
    textWidth = std::fmax(textWidth, MeasureString(row, BALLOON_TEXT));
  const float w = textWidth + 40;
  const float h = rows.size() * BALLOON_TEXT * 1.2f + 26;
  const float cx = std::fmax(14 + w / 2, std::fmin(706 - w / 2, mouth.x));
  const float top = std::fmax(12.0f, mouth.y - 50 - h);
  const Rectangle r = {cx - w / 2, top, w, h};
  // the tail, from the balloon's bottom edge to just short of the mouth
  const float tx = std::fmax(r.x + 26, std::fmin(r.x + w - 26, mouth.x));
  const Vector2 tip = {mouth.x + (tx - mouth.x) * 0.2f, mouth.y - 12};
  const bool tail = tip.y > r.y + h + 6;
  DrawRectangleRounded({r.x - 3, r.y - 3, r.width + 6, r.height + 6}, 0.5f, 10,
                       COMIC_INK);
  if (tail)
    FillTriangle({tx - 14, r.y + h - 4}, {tx + 14, r.y + h - 4},
                 {tip.x, tip.y + 3}, COMIC_INK);
  DrawRectangleRounded(r, 0.5f, 10, COMIC_PAPER);
  if (tail)
    FillTriangle({tx - 9, r.y + h - 6}, {tx + 9, r.y + h - 6}, tip,
                 COMIC_PAPER);

  // the words, typed out
  size_t left = (size_t)std::fmax(0, typed);
  float y = r.y + 13;
  for (const std::string &row : rows) {
    const size_t n = std::min(left, row.size());
    DrawString(row.substr(0, n), r.x + 20, y, BALLOON_TEXT, COMIC_INK);
    left = left > row.size() + 1 ? left - row.size() - 1 : 0;
    y += BALLOON_TEXT * 1.2f;
  }
  const bool all = typed >= line.text.size();
  if (line.hold > 0 && !all) { // hold to say it: a ring filling up
    const Vector2 c = {r.x + w - 4, r.y + h - 4};
    const float pulse = 0.5f + 0.4f * std::sin(time * 4);
    DrawCircleV(c, 15, COMIC_INK);
    DrawRing(c, 9, 12, 0, 360, 24, Faded(COMIC_PAPER, pulse));
    DrawRing(c, 9, 12, -90, -90 + 360 * typed / line.text.size(), 24,
             KUMKUM);
  } else if (all) { // more to come
    const float bob = 2 * std::sin(time * 5);
    FillTriangle({r.x + w - 22, r.y + h - 13 + bob},
                 {r.x + w - 12, r.y + h - 13 + bob},
                 {r.x + w - 17, r.y + h - 6 + bob}, Faded(COMIC_INK, 0.6f));
  }
}

void Balloons::Start(const std::vector<Line> &say) {
  lines = say;
  current = 0;
  typed = 0;
  active = !lines.empty();
}

bool Balloons::Update() {
  if (!active)
    return false;
  if (AdvanceDialogue(lines, current, typed)) {
    active = false;
    return true;
  }
  return false;
}

void Balloons::Draw(Vector2 anu, Vector2 other, float time) const {
  if (!active)
    return;
  const Line &line = lines[current];
  const bool isAnu = line.speaker == "Anu" || line.speaker == "Little Anu";
  DrawBalloon(line, typed, isAnu ? anu : other, time);
}

void Quip::Say(const Line &said, float seconds) {
  line = said;
  shown = seconds;
  typed = 0;
}

void Quip::Update() {
  if (shown <= 0)
    return;
  shown -= GetFrameTime();
  typed += TextCharsPerSecond() * GetFrameTime();
}

void Quip::Draw(Vector2 mouth, float time) const {
  if (shown <= 0)
    return;
  Line quiet = line;
  quiet.hold = 0;
  DrawBalloon(quiet, std::fmin(typed, (float)line.text.size()), mouth, time);
}

void DrawHint(Vector2 at, float time, float alpha) {
  const float pulse = 0.45f + 0.4f * std::sin(time * 4);
  const float grow = 2 * std::sin(time * 4);
  DrawRing(at, 21 + grow, 28 + grow, 0, 360, 32,
           Faded(COMIC_INK, alpha * pulse * 0.6f));
  DrawRing(at, 22 + grow, 26 + grow, 0, 360, 32,
           Faded(COMIC_PAPER, alpha * pulse));
}

void DrawGoOn(float time) {
  DrawRing({660, 660}, 9, 14, 0, 360, 24, Faded(COMIC_INK, 0.35f));
  DrawRing({660, 660}, 10, 13, 0, 360, 24,
           Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 3)));
}

void DrawCurtain(float amount, Color colour) {
  if (amount > 0)
    DrawRectangleRec(FullScreen(), Faded(colour, std::fmin(1, amount)));
}

//------------------------------------------------------------------------------
// People
//------------------------------------------------------------------------------
const Color IRIS = {66, 34, 28, 255};
const Color MOUTH = {122, 38, 50, 255};
const Color BLUSH = {244, 112, 112, 70};

// A limb: ink, then the colour inside it
static void Limb(Vector2 a, Vector2 b, float width, Color c, float line) {
  DrawLineEx(a, b, width + 2 * line, PERSON_INK);
  DrawCircleV(a, width / 2 + line, PERSON_INK);
  DrawCircleV(b, width / 2 + line, PERSON_INK);
  DrawLineEx(a, b, width, c);
  DrawCircleV(a, width / 2, c);
  DrawCircleV(b, width / 2, c);
}

static void Poly(const Vector2 *p, int n, Color c, float line) {
  InkPoly(p, n, line * 1.4f, PERSON_INK);
  for (int i = 1; i + 1 < n; i++)
    FillTriangle(p[0], p[i], p[i + 1], c);
}

Folk LittleMeena(float height) {
  Folk f;
  f.height = height;
  f.top = {24, 110, 116, 255};
  f.bottom = {236, 128, 48, 255};
  f.border = {24, 128, 130, 255};
  f.jasmine = true;
  return f;
}

Folk MeenasBoy(float height) {
  Folk f;
  f.height = height;
  f.head = 0.17f;
  f.top = {250, 206, 60, 255};
  f.bottom = {60, 110, 190, 255};
  f.hair = Folk::SHORT;
  f.skirt = false;
  return f;
}

void DrawFolk(const Folk &f, float x, float feetY, float time, Vector2 hand,
              float look, bool happy) {
  const float H = f.height;
  auto P = [&](float px, float py) { return Vector2{x + px * H, feetY - py * H}; };
  const float line = std::fmax(1.2f, H * 0.008f);
  const float breathe = 0.004f * std::sin(time * 1.6f + x);
  const float sh = 1 - 2.25f * f.head + breathe; // the shoulders
  const float waist = sh * 0.68f;

  DrawEllipseV(P(0, 0), H * 0.17f, H * 0.025f, {40, 20, 30, 60});
  // legs and feet
  if (f.skirt) {
    DrawEllipseV(P(-0.05f, 0.012f), H * 0.04f, H * 0.018f, SKIN_DARK);
    DrawEllipseV(P(0.05f, 0.012f), H * 0.04f, H * 0.018f, SKIN_DARK);
  } else {
    Limb(P(-0.05f, waist * 0.6f), P(-0.055f, 0.03f), H * 0.055f, SKIN_DARK,
         line);
    Limb(P(0.05f, waist * 0.6f), P(0.055f, 0.03f), H * 0.055f, SKIN, line);
  }
  // pavadai / saree / shorts
  if (f.skirt) {
    const Vector2 skirt[4] = {P(-0.1f, waist), P(0.1f, waist),
                              P(0.19f, 0.03f), P(-0.19f, 0.03f)};
    Poly(skirt, 4, f.bottom, line);
    const Vector2 band[4] = {P(-0.18f, 0.085f), P(0.18f, 0.085f),
                             P(0.19f, 0.03f), P(-0.19f, 0.03f)};
    Poly(band, 4, f.border, line * 0.7f);
    for (int i = -1; i <= 1; i++) // folds
      DrawLineEx(P(i * 0.03f, waist - 0.04f), P(i * 0.08f, 0.1f), line,
                 Faded(PERSON_INK, 0.35f));
  } else {
    const Vector2 shorts[4] = {P(-0.11f, waist), P(0.11f, waist),
                               P(0.12f, waist * 0.58f),
                               P(-0.12f, waist * 0.58f)};
    Poly(shorts, 4, f.bottom, line);
  }
  // blouse or shirt
  const Vector2 top[4] = {P(-0.12f, sh), P(0.12f, sh), P(0.11f, waist - 0.02f),
                          P(-0.11f, waist - 0.02f)};
  Poly(top, 4, f.top, line);
  // arms: the left at her side, the right wherever `hand` is
  const float arm = H * 0.05f;
  Limb(P(-0.115f, sh - 0.03f), P(-0.15f, waist - 0.03f), arm, SKIN_DARK, line);
  const Vector2 shoulder = P(0.115f, sh - 0.03f), palm = P(hand.x, hand.y);
  const Vector2 elbow = {(shoulder.x + palm.x) / 2 + H * 0.02f,
                         (shoulder.y + palm.y) / 2 + H * 0.03f};
  Limb(shoulder, elbow, arm, SKIN, line);
  Limb(elbow, palm, arm, SKIN, line);
  DrawCircleV(P(-0.115f, sh - 0.03f), arm * 0.75f, f.top); // sleeves
  DrawCircleV(shoulder, arm * 0.75f, f.top);

  // the head: skin with a cel shadow down the right
  const float r = H * f.head;
  const Vector2 c = P(0.01f * look, sh + f.head * 0.95f);
  DrawRectangleV({c.x - r * 0.22f, c.y + r * 0.6f}, {r * 0.44f, r * 0.7f},
                 SKIN_DARK); // the neck
  if (f.hair == Folk::PLAITS) { // two plaits, with red ribbons
    for (int side = -1; side <= 1; side += 2) {
      for (int k = 0; k < 4; k++) {
        const Vector2 p = {c.x + side * r * (0.85f + 0.05f * k),
                           c.y + r * (0.3f + 0.42f * k)};
        DrawEllipseV(p, r * 0.24f + line, r * 0.26f + line, PERSON_INK);
        DrawEllipseV(p, r * 0.24f, r * 0.26f, HAIR);
      }
      const Vector2 bow = {c.x + side * r * 1.05f, c.y + r * 1.95f};
      FillTriangle(bow, {bow.x - r * 0.3f, bow.y - r * 0.2f},
                   {bow.x - r * 0.3f, bow.y + r * 0.2f}, KUMKUM);
      FillTriangle(bow, {bow.x + r * 0.3f, bow.y - r * 0.2f},
                   {bow.x + r * 0.3f, bow.y + r * 0.2f}, KUMKUM);
    }
  }
  if (f.hair == Folk::BUN) {
    DrawCircleV({c.x - r * 0.75f, c.y - r * 0.55f}, r * 0.45f + line,
                PERSON_INK);
    DrawCircleV({c.x - r * 0.75f, c.y - r * 0.55f}, r * 0.45f,
                f.grey ? Color{186, 182, 196, 255} : HAIR);
  }
  DrawCircleV(c, r + line, PERSON_INK);
  DrawCircleV(c, r, SKIN_DARK);
  DrawCircleV({c.x - r * 0.07f, c.y - r * 0.05f}, r * 0.9f, SKIN);
  // hair over the top, a fringe
  const Color hair = f.grey ? Color{186, 182, 196, 255} : HAIR;
  if (f.hair == Folk::BALD) {
    DrawCircleSector(c, r + line, 160, 200, 8, hair);
    DrawCircleSector(c, r + line, 340, 380, 8, hair);
  } else {
    DrawCircleSector(c, r + line, 180, 360, 16, PERSON_INK);
    DrawCircleSector({c.x, c.y + line}, r, 185, 355, 16, hair);
    const int locks = f.hair == Folk::SHORT ? 5 : 4;
    for (int k = 0; k < locks; k++) { // the fringe
      const float a = PI + (k + 0.5f) * PI / locks;
      const Vector2 root = {c.x + r * 0.9f * std::cos(a),
                            c.y + r * 0.9f * std::sin(a)};
      FillTriangle({root.x - r * 0.3f, root.y - r * 0.1f},
                   {root.x + r * 0.3f, root.y - r * 0.1f},
                   {root.x + r * 0.05f * (k - 2), c.y - r * 0.18f}, hair);
    }
    DrawCircleSector({c.x - r * 0.2f, c.y - r * 0.55f}, r * 0.3f, 200, 320, 6,
                     {116, 112, 156, 255}); // shine
  }
  if (f.jasmine)
    for (int k = 0; k < 5; k++)
      DrawCircleV({c.x - r * (0.9f - k * 0.09f), c.y - r * (0.2f + k * 0.16f)},
                  r * 0.09f, COMIC_PAPER);
  // the face
  const float eyeY = c.y + r * 0.12f, spread = r * 0.38f;
  const float lookX = look * r * 0.18f;
  for (int side = -1; side <= 1; side += 2) {
    const Vector2 e = {c.x + side * spread + lookX, eyeY};
    if (happy) {
      InkCurve({e.x - r * 0.15f, e.y + r * 0.05f}, {e.x, e.y - r * 0.18f},
               {e.x + r * 0.15f, e.y + r * 0.05f}, std::fmax(1.5f, r * 0.09f),
               PERSON_INK);
    } else {
      const bool blink = std::fmod(time + x * 0.01f, 4.1f) < 0.12f;
      if (blink) {
        DrawLineEx({e.x - r * 0.14f, e.y}, {e.x + r * 0.14f, e.y},
                   std::fmax(1.5f, r * 0.07f), PERSON_INK);
      } else {
        DrawEllipseV(e, r * 0.14f, r * 0.19f, IRIS);
        DrawCircleV({e.x - r * 0.05f, e.y - r * 0.07f},
                    std::fmax(1.0f, r * 0.055f), WHITE);
        DrawCircleV({e.x + r * 0.05f, e.y + r * 0.08f},
                    std::fmax(0.6f, r * 0.025f), WHITE);
        DrawLineEx({e.x - r * 0.17f, e.y - r * 0.17f},
                   {e.x + r * 0.17f, e.y - r * 0.2f},
                   std::fmax(1.5f, r * 0.07f), PERSON_INK); // the lid
      }
    }
    DrawLineEx({e.x - r * 0.12f, e.y - r * 0.38f},
               {e.x + r * 0.12f, e.y - r * 0.4f}, std::fmax(1, r * 0.05f),
               PERSON_INK); // brow
    DrawEllipseV({c.x + side * r * 0.55f + lookX, eyeY + r * 0.32f},
                 r * 0.16f, r * 0.08f, BLUSH);
  }
  const Vector2 m = {c.x + lookX, c.y + r * 0.58f};
  if (happy) {
    DrawCircleSector(m, r * 0.2f, 0, 180, 10, MOUTH);
    DrawCircleSectorLines(m, r * 0.2f, 0, 180, 10, PERSON_INK);
  } else {
    InkCurve({m.x - r * 0.13f, m.y - r * 0.02f}, {m.x, m.y + r * 0.08f},
             {m.x + r * 0.13f, m.y - r * 0.02f}, std::fmax(1, r * 0.05f),
             PERSON_INK);
  }
  if (f.moustache)
    DrawEllipseV({m.x, m.y - r * 0.12f}, r * 0.22f, r * 0.07f, hair);
  if (f.skirt && f.hair != Folk::BALD) // a pottu
    DrawCircleV({c.x + lookX, eyeY - r * 0.42f}, r * 0.05f, KUMKUM);
}

//------------------------------------------------------------------------------
// The street
//------------------------------------------------------------------------------
static void DrawPuffCloud(float x, float y, float size) {
  const Vector2 PUFFS[] = {{-0.7f, -0.2f}, {-0.3f, -0.5f}, {0.15f, -0.6f},
                           {0.6f, -0.35f}};
  const float RADII[] = {0.32f, 0.42f, 0.46f, 0.34f};
  for (int pass = 0; pass < 2; pass++) {
    const float grow = pass == 0 ? 2.5f : 0;
    const Color c = pass == 0 ? COMIC_INK : COMIC_PAPER;
    for (int i = 0; i < 4; i++)
      DrawCircleV({x + PUFFS[i].x * size, y + PUFFS[i].y * size},
                  RADII[i] * size + grow, c);
    DrawRectangleRec({x - size - grow, y - 0.22f * size, 1.95f * size + 2 * grow,
                      0.22f * size + grow},
                     c);
  }
}

void DrawStreetSky(float bottom, bool night, float time, float scroll) {
  const Rectangle all = FullScreen();
  if (night) {
    DrawBandGradient(0, bottom, {14, 18, 58, 255}, {72, 44, 96, 255});
    BeginTone(TONE_DOTS, 7);
    DrawBandGradient(0, bottom, Faded({4, 6, 30, 255}, 0.7f),
                     Faded({4, 6, 30, 255}, 0.1f));
    EndTone();
    for (int i = 0; i < 30; i++) {
      const float x = all.x + std::fmod(i * 173.0f - scroll * 0.05f + 4000,
                                        all.width);
      const float y = 12 + std::fmod(i * 67.0f, std::fmax(20, bottom - 120));
      const float twinkle = 0.6f + 0.4f * std::sin(time * 2 + i * 1.7f);
      DrawSparkle({x, y}, (2.5f + (i % 3)) * twinkle, {255, 246, 200, 230});
    }
    return;
  }
  DrawBandGradient(0, bottom, {104, 184, 236, 255}, {226, 240, 226, 255});
  BeginTone(TONE_DOTS, 7);
  DrawBandGradient(0, bottom * 0.75f, Faded({30, 90, 190, 255}, 0.55f),
                   Faded({30, 90, 190, 255}, 0));
  EndTone();
  for (int i = 0; i < 4; i++)
    DrawPuffCloud(all.x + std::fmod(i * 420 + time * 6 - scroll * 0.08f +
                                        8000,
                                    all.width + 300) -
                      150,
                  70.0f + (i % 2) * 40, 34.0f + (i % 3) * 8);
}

void DrawFarVillage(float groundY, bool night, float time, float scroll) {
  const Rectangle all = FullScreen();
  if (night) {
    DrawComicHills(groundY - 110, groundY, 70, 3, {70, 64, 130, 255},
                   {40, 36, 90, 255});
  } else {
    DrawComicHills(groundY - 120, groundY, 80, 1, {156, 164, 226, 255},
                   {86, 96, 176, 255});
    DrawComicHills(groundY - 70, groundY, 40, 4, {112, 128, 200, 255},
                   {60, 72, 150, 255});
  }
  // fields and back gardens between here and the hills
  DrawBandGradient(groundY, 720 - groundY,
                   night ? Color{40, 52, 70, 255} : Color{150, 196, 110, 255},
                   night ? Color{30, 40, 56, 255} : Color{116, 168, 90, 255});
  DrawToneRect({all.x, groundY, all.width, 720 - groundY},
               night ? Color{10, 14, 30, 255} : LEAF_DARK, 0.15f, 0.6f,
               TONE_DOTS, 5);
  DrawBand(groundY - 1, 2.5f, COMIC_INK);
  if (night) // the palms don't show at night, behind the lit houses
    return;
  const Color trunk = {150, 98, 56, 255};
  const Color leaves = LEAF;
  const float spacing = 260, shift = std::fmod(scroll * 0.4f, spacing);
  const int base = StablePlace(scroll * 0.4f, spacing, all.x);
  int j = 0;
  for (float x = all.x - spacing - shift; x < all.x + all.width + spacing;
       x += spacing, j++) {
    const int i = base + j;
    DrawComicPalm(x + 60 * Hash(i), groundY + 4, 120 + 50 * Hash(i + 7), time,
                  trunk, leaves);
  }
}

const Color WALL_COLOURS[] = {{252, 222, 140, 255}, {246, 176, 184, 255},
                              {150, 214, 206, 255}, {204, 194, 240, 255},
                              {250, 244, 226, 255}, {186, 222, 150, 255}};

int StablePlace(float moved, float spacing, float left) {
  // x + moved = left - spacing + (moved - shift) + j * spacing, where
  // moved - shift is a whole number of spacings
  // (fmod is exact, so moved - shift is a whole number of spacings)
  const float shift = std::fmod(moved, spacing);
  const int whole = (int)std::lround((moved - shift) / spacing);
  return whole + (int)std::floor(left / spacing) - 1;
}

void DrawStreetHouse(float x, float groundY, float W, float H, int style,
                     float time, bool night) {
  const Color wall = WALL_COLOURS[style % 6];
  const float plinth = H * 0.11f;
  const float roofTop = groundY - H, roofBottom = groundY - H + H * 0.24f;
  const float wallBottom = groundY - plinth;
  // the wall, shaded under the eaves
  ComicRect({x, roofBottom, W, wallBottom - roofBottom}, wall, 2.6f);
  DrawToneRect({x, roofBottom, W, H * 0.16f}, {150, 80, 40, 255}, 0.6f, 0,
               TONE_DOTS, 5);
  // the clay-tile roof
  const Vector2 roof[4] = {{x - 14, roofBottom}, {x + 10, roofTop},
                           {x + W - 10, roofTop}, {x + W + 14, roofBottom}};
  ComicPoly(roof, 4, {214, 92, 52, 255}, 2.6f);
  DrawToneRect({x - 4, roofTop + 4, W + 8, roofBottom - roofTop - 6},
               {110, 24, 18, 255}, 0.05f, 0.5f, TONE_DOTS, 4);
  for (int row = 1; row < 4; row++) {
    const float t = row / 4.0f, y = roofTop + t * (roofBottom - roofTop);
    const float inset = 10 - 24 * t;
    DrawLineEx({x + inset, y}, {x + W - inset, y}, 1.6f, COMIC_INK);
    for (float tx = x + inset + (row % 2) * 12; tx < x + W - inset && !night;
         tx += 24) // (cheap lines: software rendering pays for each piece)
      DrawLineEx({tx, y - (roofBottom - roofTop) / 4 + 2}, {tx, y}, 1.2f,
                 Faded(COMIC_INK, 0.5f));
  }
  ComicRect({x - 16, roofBottom - 3, W + 32, 9}, TEAK_DARK, 2);
  // the door, one side; a window on the other
  const bool doorLeft = style % 2 == 0;
  const float doorW = std::fmax(50, std::fmin(78, W * 0.24f));
  const float doorH = (wallBottom - roofBottom) * 0.72f;
  const float doorX = doorLeft ? x + W * 0.18f : x + W * 0.82f - doorW;
  const Rectangle door = {doorX, wallBottom - doorH, doorW, doorH};
  ComicRect({door.x - 6, door.y - 8, door.width + 12, door.height + 8},
            TEAK_DARK, 2.4f);
  if (night) { // ajar, the lamp on inside
    DrawRectangleRec(door, {255, 196, 96, 255});
    ComicRect({door.x, door.y, door.width * 0.45f, door.height}, TEAK, 1.8f);
    ComicRect({door.x + door.width * 0.82f, door.y, door.width * 0.18f,
               door.height},
              TEAK, 1.8f);
  } else {
    ComicRect({door.x, door.y, door.width / 2, door.height}, TEAK, 1.8f);
    ComicRect({door.x + door.width / 2, door.y, door.width / 2, door.height},
              TEAK, 1.8f);
    for (int k = 0; k < 2; k++)
      InkRect({door.x + 5 + k * door.width / 2, door.y + 8,
               door.width / 2 - 10, door.height * 0.4f},
              1.2f);
  }
  // the thoranam of mango leaves over the door
  DrawLineEx({door.x - 6, door.y - 9}, {door.x + door.width + 6, door.y - 9},
             1.6f, COMIC_INK);
  for (int i = 0; i < (int)(doorW / 10); i++) { // (plain triangles: cheap)
    const float lx = door.x - 2 + i * 10 + 4, sway = std::sin(time * 2 + i);
    FillTriangle({lx - 4.5f, door.y - 9}, {lx + 4.5f, door.y - 9},
                 {lx + sway, door.y + 12}, COMIC_INK);
    FillTriangle({lx - 3, door.y - 8}, {lx + 3, door.y - 8},
                 {lx + sway, door.y + 9}, i % 2 ? LEAF : Color{96, 168, 64, 255});
  }
  const float winW = std::fmin(W * 0.24f, 90), winH = H * 0.22f;
  const float winX = doorLeft ? x + W * 0.7f - winW / 2 : x + W * 0.3f - winW / 2;
  const Rectangle win = {winX, roofBottom + H * 0.16f, winW, winH};
  for (int side = 0; side < 2; side++)
    ComicRect({side == 0 ? win.x - winW * 0.3f - 6 : win.x + win.width + 6,
               win.y - 4, winW * 0.3f, winH + 8},
              {48, 152, 110, 255}, 2);
  ComicRect({win.x - 6, win.y - 6, win.width + 12, win.height + 12}, TEAK, 2.2f);
  DrawRectangleRec(win, night ? Color{255, 200, 100, 255}
                              : Color{70, 50, 62, 255});
  if (!night)
    DrawToneRect(win, COMIC_INK, 0.2f, 0.5f, TONE_LINES, 4);
  for (int i = 1; i < 4; i++)
    DrawLineEx({win.x + i * win.width / 4, win.y},
               {win.x + i * win.width / 4, win.y + win.height}, 3, COMIC_INK);
  InkRect(win, 1.8f);
  // the plinth and thinnai seat in red oxide, a step at the door
  ComicRect({x - 4, wallBottom, W + 8, plinth}, RED_OXIDE, 2.4f);
  DrawToneRect({x - 4, wallBottom, W + 8, plinth}, COMIC_INK, 0.05f, 0.45f,
               TONE_DOTS, 4);
  ComicRect({x - 6, wallBottom - 6, W + 12, 8}, {232, 104, 74, 255}, 2);
  ComicRect({door.x - 10, groundY - plinth * 0.5f, door.width + 20,
             plinth * 0.5f},
            {216, 210, 198, 255}, 2);
  // what each house keeps outside
  if (style % 3 == 0)
    DrawComicJasmine({doorLeft ? x + W - 90.0f : x + 10.0f, roofBottom + 10, 80,
                      wallBottom - roofBottom - 30},
                     time);
  if (style % 3 == 1)
    DrawComicPot(doorLeft ? x + W - 40 : x + 40, wallBottom - 10, 16,
                 style % 2 == 0);
  DrawGroundKolam({door.x + door.width / 2, groundY + 34}, 30, style);
}

void DrawStreetGround(float y, float scroll, bool wet) {
  const Rectangle all = FullScreen();
  const Color top = wet ? Color{150, 120, 96, 255} : Color{204, 166, 118, 255};
  const Color bottom = wet ? Color{120, 92, 72, 255} : Color{178, 136, 94, 255};
  DrawBandGradient(y, 720 - y, top, bottom);
  DrawToneRect({all.x, y, all.width, 720 - y}, {110, 70, 40, 255}, 0.05f,
               0.45f, TONE_DOTS, 5);
  DrawBand(y - 1, 3, COMIC_INK);
  const float span = all.width + 200;
  for (int i = 0; i < 26; i++) { // pebbles go by
    const float px =
        all.x - 100 + std::fmod(i * 97.0f - scroll + 100 * span, span);
    const float py = y + 14 + std::fmod(i * 37.0f, 720 - y - 20);
    ComicEllipse({px, py}, 3.5f, 2.4f, {220, 198, 166, 255}, 1.2f);
  }
  for (int k = 0; k < 2; k++) // cart tracks
    InkCurve({all.x, y + 50 + k * 26}, {360, y + 40 + k * 26},
             {all.x + all.width, y + 54 + k * 26}, 1.4f, {130, 92, 62, 255});
}

void DrawGroundKolam(Vector2 c, float size, int style, float alpha) {
  const Color flour = Faded({252, 248, 236, 255}, alpha);
  const float squash = 0.42f; // lying on the ground
  if (style % 2 == 0) { // dots with loops round them
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        const Vector2 d = {c.x + i * size * 0.5f, c.y + j * size * 0.5f * squash};
        DrawCircleV(d, 1.6f, flour);
      }
    }
    for (int k = 0; k < 2; k++)
      DrawEllipseLines((int)c.x, (int)c.y, (int)(size * (0.45f + k * 0.35f)),
                       (int)(size * squash * (0.45f + k * 0.35f)), flour);
  } else { // a flower
    for (int p = 0; p < 6; p++) {
      const float a = p * PI / 3;
      DrawEllipseLines((int)(c.x + std::cos(a) * size * 0.4f),
                       (int)(c.y + std::sin(a) * size * 0.4f * squash),
                       (int)(size * 0.28f), (int)(size * 0.28f * squash),
                       flour);
    }
    DrawCircleV(c, 3, Faded({230, 70, 90, 255}, alpha));
  }
}

void DrawTempleTower(float x, float baseY, float size, float lights,
                     float time) {
  const float s = size;
  const Color stone = {226, 140, 112, 255}, light = {246, 184, 146, 255};
  float top = baseY;
  for (int i = 0; i < 7; i++) {
    const float w = (150 - i * 17) * s, h = 26 * s, y = baseY - (i + 1) * h;
    top = y;
    DrawRectangleRec({x - w / 2, y, w, h}, stone);
    for (int k = 0; k < (int)(w / (15 * s)); k++) // niches with figures
      DrawRectangleRec({x - w / 2 + 5 * s + k * 15 * s, y + 9 * s, 6 * s,
                        12 * s},
                       {120, 56, 52, 255});
    DrawToneRect({x + w * 0.2f, y + 4, w * 0.3f, h - 4}, COMIC_INK, 0.4f, 0.4f,
                 TONE_DOTS, 4);
    InkRect({x - w / 2, y, w, h}, 1.8f);
    ComicRect({x - w / 2 - 3, y, w + 6, 6 * s}, light, 1.4f);
  }
  // the barrel-vaulted top and its kalasams
  ComicRect({x - 34 * s, top - 22 * s, 68 * s, 22 * s}, stone, 1.8f);
  ComicCircle({x - 34 * s, top - 11 * s}, 11 * s, light, 1.6f);
  ComicCircle({x + 34 * s, top - 11 * s}, 11 * s, light, 1.6f);
  for (int k = 0; k < 5; k++) {
    const float kx = x - 24 * s + k * 12 * s;
    ComicCircle({kx, top - 27 * s}, 4.5f * s, {244, 192, 64, 255}, 1.2f);
    FillTriangle({kx - 1.5f * s, top - 30 * s}, {kx + 1.5f * s, top - 30 * s},
                 {kx, top - 38 * s}, {240, 186, 60, 255});
  }
  // festival bulbs along every tier, twinkling
  if (lights <= 0)
    return;
  for (int i = 0; i <= 7; i++) {
    const float w = (150 - std::min(i, 6) * 17) * s, y = baseY - i * 26 * s;
    const int n = (int)(w / (8 * s));
    for (int k = 0; k <= n; k++) {
      if (Hash(i * 31.0f + k) > lights)
        continue;
      const float bx = x - w / 2 + k * w / n;
      const bool on = std::sin(time * 3 + bx * 0.7f + i) > -0.4f;
      DrawCircleV({bx, y}, (on ? 2.2f : 1.4f) * std::fmax(0.8f, s),
                  on ? BULB : Color{220, 150, 80, 255});
    }
  }
}

static const Color BULB_COLOURS[] = {{255, 226, 110, 255}, {255, 110, 90, 255},
                                     {120, 230, 140, 255}, {120, 190, 255, 255},
                                     {255, 150, 230, 255}};

void DrawBulbString(Vector2 a, Vector2 b, float sag, float time, int seed) {
  const Vector2 mid = {(a.x + b.x) / 2, (a.y + b.y) / 2 + sag * 2};
  DrawSplineSegmentBezierQuadratic(a, mid, b, 1.6f, COMIC_INK);
  const float len = std::fabs(b.x - a.x) + std::fabs(b.y - a.y);
  const int n = std::max(2, (int)(len / 26));
  for (int i = 1; i < n; i++) {
    const float t = (float)i / n;
    const Vector2 p = {(1 - t) * (1 - t) * a.x + 2 * (1 - t) * t * mid.x +
                           t * t * b.x,
                       (1 - t) * (1 - t) * a.y + 2 * (1 - t) * t * mid.y +
                           t * t * b.y + 4};
    const Color c = BULB_COLOURS[(i + seed) % 5];
    const float glow = 0.6f + 0.4f * std::sin(time * 4 - i * 0.9f + seed);
    DrawPoly(p, 8, 9, 0, Faded(c, 0.22f * glow)); // (few sides: cheap)
    DrawPoly(p, 6, 4.4f, 0, COMIC_INK);
    DrawPoly(p, 6, 3.1f, 0, ColorLerp(Faded(c, 1), WHITE, 0.4f * glow));
  }
}

void DrawBunting(Vector2 a, Vector2 b, float sag, float time) {
  const Vector2 mid = {(a.x + b.x) / 2, (a.y + b.y) / 2 + sag * 2};
  DrawSplineSegmentBezierQuadratic(a, mid, b, 1.4f, COMIC_INK);
  const int n = std::max(2, (int)(std::fabs(b.x - a.x) / 24));
  for (int i = 0; i < n; i++) {
    const float t = (i + 0.5f) / n, t2 = (i + 0.95f) / n;
    auto at = [&](float u) {
      return Vector2{(1 - u) * (1 - u) * a.x + 2 * (1 - u) * u * mid.x +
                         u * u * b.x,
                     (1 - u) * (1 - u) * a.y + 2 * (1 - u) * u * mid.y +
                         u * u * b.y};
    };
    const Vector2 p = at(t - 0.4f / n), q = at(t2 - 0.4f / n);
    const Vector2 tip = {(p.x + q.x) / 2 + 2 * std::sin(time * 3 + i),
                         (p.y + q.y) / 2 + 18};
    FillTriangle({p.x - 1.5f, p.y - 1}, {q.x + 1.5f, q.y - 1},
                 {tip.x, tip.y + 2.5f}, COMIC_INK);
    FillTriangle(p, q, tip, BULB_COLOURS[i % 5]);
  }
}

void DrawCrowd(float y, float scroll, float time, bool night) {
  const Rectangle all = FullScreen();
  const Color CLOTHES[] = {{200, 70, 110, 255}, {240, 170, 60, 255},
                           {70, 140, 170, 255}, {250, 240, 220, 255},
                           {120, 80, 160, 255}, {80, 150, 90, 255}};
  for (int row = 0; row < 2; row++) {
    const float spacing = 54 - row * 6, ry = y + row * 22;
    const float moved = scroll * (0.7f + row * 0.3f);
    const float shift = std::fmod(moved, spacing);
    // (each person keeps a whole-number place: working it out from x, and
    // from a different scroll than they move by, changed who they were)
    const int base = StablePlace(moved, spacing, all.x);
    int j = 0;
    for (float x = all.x - spacing - shift; x < all.x + all.width + spacing;
         x += spacing, j++) {
      const int i = base + j + row * 100;
      const float px = x + 14 * Hash(i), bob = 2 * std::sin(time * 3 + i);
      const float s = 1 + row * 0.15f;
      Color body = CLOTHES[(int)(Hash(i + 3) * 6) % 6];
      if (night)
        body = ColorLerp(body, {30, 30, 70, 255}, 0.45f);
      // plain shapes with few sides, many of them: kept cheap to draw
      DrawPoly({px, ry + 20 * s + bob}, 9, 20.5f * s, 0, COMIC_INK);
      DrawPoly({px, ry + 20 * s + bob}, 9, 19 * s, 0, body);
      DrawPoly({px, ry - 6 * s + bob}, 8, 11.5f * s, 0, COMIC_INK);
      DrawPoly({px, ry - 6 * s + bob}, 8, 10 * s, 0,
               night ? Color{110, 76, 70, 255} : SKIN_DARK);
      DrawPoly({px, ry - 10 * s + bob}, 8, 7.5f * s, 0, HAIR);
    }
  }
}

//------------------------------------------------------------------------------
// Things
//------------------------------------------------------------------------------
void DrawAgal(Vector2 c, float scale, float lit, float time) {
  const float s = scale;
  if (lit > 0) {
    const float flicker = 1.5f * std::sin(time * 9 + c.x) + std::sin(time * 23);
    const float f = s * lit;
    FillTriangle({c.x - 3.5f * f, c.y - 4 * s}, {c.x + 3.5f * f, c.y - 4 * s},
                 {c.x + 0.6f * flicker * s, c.y - (17 + flicker) * f},
                 {255, 176, 40, 255});
    DrawCircleV({c.x, c.y - 5 * s}, 3.6f * f, {255, 176, 40, 255});
    DrawCircleV({c.x, c.y - 6 * s}, 1.8f * f, {255, 246, 200, 255});
  } else {
    DrawLineEx({c.x, c.y - 2 * s}, {c.x + 1 * s, c.y - 7 * s}, 1.6f * s,
               COMIC_INK); // the wick
  }
  DrawCircleSector(c, 9 * s, 0, 180, 12, {196, 92, 52, 255});
  DrawRing(c, 8.5f * s, 8.5f * s + 2, 0, 180, 12, COMIC_INK);
  DrawLineEx({c.x - 10 * s, c.y}, {c.x + 10 * s, c.y}, 1.6f, COMIC_INK);
}

void DrawAgalGlow(Vector2 c, float scale, float lit, float time) {
  if (lit <= 0)
    return;
  const float flicker = 1 + 0.06f * std::sin(time * 11 + c.x);
  BeginBlendMode(BLEND_ADDITIVE);
  DrawCircleGradient({c.x, c.y - 8 * scale}, 46 * scale * flicker,
                     {255, 170, 70, (unsigned char)(90 * lit)},
                     {255, 170, 70, 0});
  EndBlendMode();
  BeginTone(TONE_DOTS, 4);
  DrawCircleGradient({c.x, c.y - 10 * scale}, 26 * scale,
                     {255, 214, 90, (unsigned char)(200 * lit)},
                     {255, 214, 90, 0});
  EndTone();
}

static const Color WINDMILL_PAPER[][2] = {
    {{236, 72, 130, 255}, {252, 214, 70, 255}},
    {{60, 170, 90, 255}, {250, 150, 40, 255}},
    {{70, 130, 230, 255}, {236, 60, 60, 255}},
    {{150, 80, 200, 255}, {120, 220, 220, 255}},
    {{250, 120, 40, 255}, {60, 200, 170, 255}}};

void DrawWindmill(Vector2 c, float r, float angle, int colours) {
  const Color *paper = WINDMILL_PAPER[((colours % 5) + 5) % 5];
  for (int i = 0; i < 4; i++) {
    const float a = angle + i * PI / 2;
    const Vector2 tip = {c.x + r * std::cos(a), c.y + r * std::sin(a)};
    const Vector2 fold = {c.x + r * 0.62f * std::cos(a + 1.25f),
                          c.y + r * 0.62f * std::sin(a + 1.25f)};
    const Vector2 back = {c.x + r * 0.5f * std::cos(a + 0.55f),
                          c.y + r * 0.5f * std::sin(a + 0.55f)};
    // an ink triangle a little bigger underneath, for the outline
    const float grow = 1.12f, ox = c.x * (1 - grow), oy = c.y * (1 - grow);
    FillTriangle({c.x, c.y}, {tip.x * grow + ox, tip.y * grow + oy},
                 {fold.x * grow + ox, fold.y * grow + oy}, COMIC_INK);
    FillTriangle(c, tip, fold, paper[i % 2]);
    FillTriangle(c, back, fold, ColorLerp(paper[(i + 1) % 2], BLACK, 0.12f));
  }
  ComicCircle(c, std::fmax(2.0f, r * 0.1f), {244, 192, 64, 255}, 1.2f);
}

void DrawWindmillStick(Vector2 c, float r, Vector2 hand) {
  (void)r;
  DrawLineEx(c, hand, 5, COMIC_INK);
  DrawLineEx(c, hand, 2.6f, {226, 196, 130, 255});
}

void DrawPebble(Vector2 c, float r, int seed) {
  const Color STONES[] = {{176, 168, 156, 255}, {200, 176, 140, 255},
                          {150, 150, 160, 255}, {214, 196, 170, 255},
                          {168, 140, 120, 255}};
  const Color stone = STONES[seed % 5];
  ComicEllipse(c, r, r * 0.78f, stone, std::fmax(1.4f, r * 0.12f));
  DrawEllipseV({c.x + r * 0.2f, c.y + r * 0.25f}, r * 0.6f, r * 0.4f,
               ColorLerp(stone, BLACK, 0.15f));
  DrawEllipseV({c.x - r * 0.35f, c.y - r * 0.3f}, r * 0.28f, r * 0.16f,
               Faded(WHITE, 0.7f));
}

void DrawRain(float time, float amount) {
  const Rectangle all = FullScreen();
  const int n = (int)(110 * amount);
  for (int i = 0; i < n; i++) {
    const float speed = 900 + 300 * Hash(i);
    const float x = all.x + std::fmod(i * 61.7f + time * 160, all.width + 80);
    const float y = std::fmod(Hash(i + 5) * 760 + time * speed, 800.0f) - 60;
    DrawLineEx({x, y}, {x - 7, y + 26}, 1.6f, {226, 238, 255, 150});
  }
}

void DrawConfetti(Vector2 c, float age, int seed) {
  if (age > 1.6f)
    return;
  for (int i = 0; i < 18; i++) {
    const float a = Hash(i + seed * 19.0f) * 2 * PI;
    const float v = 120 + 160 * Hash(i + seed * 7.0f + 3);
    const Vector2 p = {c.x + std::cos(a) * v * age,
                       c.y + std::sin(a) * v * age + 160 * age * age};
    const float alpha = 1 - Between(age, 1.0f, 1.6f);
    DrawPoly(p, 4, 4.5f, age * 400 + i * 40,
             Faded(BULB_COLOURS[(i + seed) % 5], alpha));
  }
}

} // namespace streets
