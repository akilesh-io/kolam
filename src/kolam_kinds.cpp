// kolam_kinds.cpp - the other kinds of kolam, each a small game of its own on
// the Mechanics page (the kolam book's pages are sikku kolams):
//   Pulli     drag from dot to dot; straight lines make stars and diamonds
//   Padi      drag along the grid; steps out from a square, kaavi (red) border
//   Poo       paint flower petals of any colour into a flower's shapes
//   Lotus     drag out rings of petals from the middle; tap to colour them
//   Freehand  draw anything; it can be mirrored all the way round
// Every game is drawn in rice flour on the doorstep's floor, with Amma's note
// pinned in the top-right corner: her drawing of that kind of kolam and a
// line in her hand. Her note is only for inspiration: the player presses
// Done whenever they like what they've made.
// Pulli and Padi copy every line to all eight matching places (turned and
// mirrored), the way those kolams are always symmetrical.
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <tuple>

#include "game.h"
#include "rlgl.h"

namespace {

const float PI_F = 3.14159265f;

// The floor and the flour
const Color FLOOR_TOP = {58, 50, 42, 255};
const Color FLOOR_BOTTOM = {44, 38, 32, 255};
const Color SWEPT = {50, 44, 37, 255}; // the patch swept and washed for it
const Color KAAVI = {176, 62, 40, 255}; // red earth, for padi borders
// Amma's note: paper and ink
const Color NOTE_PAPER = {243, 234, 210, 255};
const Color NOTE_INK = {40, 42, 92, 255};
const Color NOTE_FAINT = {120, 100, 80, 255};

// Colours of powder and petals: rice flour first
const Color COLOURS[] = {
    {242, 238, 228, 255}, // rice flour / jasmine
    {240, 150, 30, 255},  // marigold
    {246, 206, 52, 255},  // yellow
    {206, 48, 72, 255},   // rose
    {232, 120, 160, 255}, // pink (lotus)
    {74, 146, 62, 255},   // leaf green
    {128, 72, 168, 255},  // purple
};

// The layout, on the 720x720 stage
const Rectangle AREA = {60, 120, 460, 460}; // where the kolam is drawn
const Vector2 CENTRE = {290, 350};
const Rectangle NOTE = {540, 18, 166, 214}; // Amma's note, top right
const float BUTTON_Y = 616, BUTTON_W = 140, BUTTON_H = 46;
const float SWATCH_X = 623, SWATCH_TOP = 272, SWATCH_GAP = 46;
const float SWATCH_R = 17;
const float POO_RADIUS = 225;   // the flower to fill
const float LOTUS_RADIUS = 228; // the biggest lotus
const int PULLI_DOTS = 7;       // each way
const int FREEHAND_COPIES = 8;  // round the middle, when mirrored

const char *NAMES[KOLAM_KIND_COUNT] = {"Pulli kolam", "Padi kolam",
                                       "Poo kolam", "Lotus kolam",
                                       "Freehand kolam"};
const char *TAGS[KOLAM_KIND_COUNT] = {"join the dots", "steps", "petals",
                                      "lotus", "free"};
const char *HINTS[KOLAM_KIND_COUNT] = {
    "Drag from dot to dot. Erase wipes lines away; Undo takes one back.",
    "Drag along the lines. Pick kaavi for the red border.",
    "Pick a colour, then paint petals into the flower.",
    "Drag out from the middle for petals. Tap a ring to colour it.",
    "Draw anything. Erase wipes lines away; Undo takes one back."};
const char *AMMAS_NOTES[KOLAM_KIND_COUNT] = {
    "Two squares make a star.",
    "Steps out from the square. Kaavi on the edge.",
    "Marigold, jasmine, rose. Every petal full.",
    "Thamarai. One petal at a time, all round.",
    "No dots today. Let your hand go."};

float Distance(Vector2 a, Vector2 b) { return std::hypot(a.x - b.x, a.y - b.y); }

Color Vary(Color c, int amount) { // a little different, like real powder
  auto v = [&](unsigned char x) {
    return (unsigned char)std::max(
        0, std::min(255, x + GetRandomValue(-amount, amount)));
  };
  return {v(c.r), v(c.g), v(c.b), c.a};
}

// A line of flour: a soft glow under a solid line
void FlourLine(Vector2 a, Vector2 b, float thick, Color c) {
  DrawLineEx(a, b, thick * 2.6f, Faded(c, 0.08f));
  DrawLineEx(a, b, thick, c);
  DrawCircleV(a, thick / 2, c);
  DrawCircleV(b, thick / 2, c);
}

// Smooths a hand-drawn line (Chaikin's method, keeping its ends)
std::vector<Vector2> SmoothOpen(const std::vector<Vector2> &points) {
  std::vector<Vector2> path = points;
  for (int round = 0; round < 2 && path.size() > 2; round++) {
    std::vector<Vector2> smoother = {path.front()};
    for (size_t i = 0; i + 1 < path.size(); i++) {
      const Vector2 a = path[i], b = path[i + 1];
      smoother.push_back({0.75f * a.x + 0.25f * b.x, 0.75f * a.y + 0.25f * b.y});
      smoother.push_back({0.25f * a.x + 0.75f * b.x, 0.25f * a.y + 0.75f * b.y});
    }
    smoother.push_back(path.back());
    path = smoother;
  }
  return path;
}

//------------------------------------------------------------------------------
// The eight symmetries of a square grid (turns and mirrors), for Pulli and
// Padi: point (x, y) on a grid of 0..n
//------------------------------------------------------------------------------
std::vector<std::pair<int, int>> Images(int x, int y, int n) {
  std::vector<std::pair<int, int>> all = {
      {x, y},         {n - x, y},     {x, n - y},     {n - x, n - y},
      {y, x},         {n - y, x},     {y, n - x},     {n - y, n - x}};
  return all;
}

//------------------------------------------------------------------------------
// Padi: straight lines along a grid, one grid step each. A step holds flour
// (two thin parallel lines) or kaavi (a red band between two flour lines).
//------------------------------------------------------------------------------
const int PADI_N = 8; // grid points 0..8 each way
enum PadiState { PADI_NONE, PADI_FLOUR, PADI_KAAVI };

struct PadiStep {
  int x, y;      // where it starts
  bool across;   // to (x + 1, y); otherwise down to (x, y + 1)
  bool operator<(const PadiStep &o) const {
    return std::tie(x, y, across) < std::tie(o.x, o.y, o.across);
  }
};

// The same step turned or mirrored
std::vector<PadiStep> PadiImages(PadiStep s) {
  const int x2 = s.across ? s.x + 1 : s.x, y2 = s.across ? s.y : s.y + 1;
  std::vector<PadiStep> out;
  const auto a = Images(s.x, s.y, PADI_N), b = Images(x2, y2, PADI_N);
  for (size_t i = 0; i < a.size(); i++) {
    const int ax = a[i].first, ay = a[i].second;
    const int bx = b[i].first, by = b[i].second;
    PadiStep t;
    t.across = ay == by;
    t.x = std::min(ax, bx);
    t.y = std::min(ay, by);
    out.push_back(t);
  }
  return out;
}

void DrawPadiStep(Vector2 a, Vector2 b, int state, float scale) {
  const bool across = std::fabs(b.x - a.x) > std::fabs(b.y - a.y);
  const float off = 3.6f * scale;
  // reach a little past the ends, so corners close up
  const Vector2 dir = across ? Vector2{1, 0} : Vector2{0, 1};
  const Vector2 p = {a.x - dir.x * off, a.y - dir.y * off};
  const Vector2 q = {b.x + dir.x * off, b.y + dir.y * off};
  const Vector2 side = across ? Vector2{0, off} : Vector2{off, 0};
  if (state == PADI_KAAVI)
    DrawLineEx(p, q, off * 2, KAAVI);
  const Color flour = COLOURS[0];
  DrawLineEx({p.x - side.x, p.y - side.y}, {q.x - side.x, q.y - side.y},
             std::fmax(1.2f, 2.2f * scale), flour);
  DrawLineEx({p.x + side.x, p.y + side.y}, {q.x + side.x, q.y + side.y},
             std::fmax(1.2f, 2.2f * scale), flour);
}

// Amma's padi kolam: a square in a square, a kaavi border and four ways out
std::map<PadiStep, int> AmmasPadi() {
  std::map<PadiStep, int> steps;
  auto ring = [&](int from, int to, int state) {
    for (int i = from; i < to; i++) {
      steps[{i, from, true}] = state;
      steps[{i, to, true}] = state;
      steps[{from, i, false}] = state;
      steps[{to, i, false}] = state;
    }
  };
  ring(3, 5, PADI_FLOUR);
  ring(2, 6, PADI_FLOUR);
  ring(1, 7, PADI_KAAVI);
  for (int i = 0; i < 2; i++) { // the four ways out, through the border
    steps[{4, i, false}] = PADI_FLOUR;
    steps[{4, 6 + i, false}] = PADI_FLOUR;
    steps[{i, 4, true}] = PADI_FLOUR;
    steps[{6 + i, 4, true}] = PADI_FLOUR;
  }
  return steps;
}

//------------------------------------------------------------------------------
// Poo: a flower of four rings, each split into petal-shaped places to fill.
// Sizes are fractions of the flower's radius.
//------------------------------------------------------------------------------
const int POO_RINGS = 4;
const int POO_SECTORS[POO_RINGS] = {1, 8, 16, 8};

// The outside edge of ring k at angle a (radians)
float PooEdge(int ring, float angle) {
  const float sector = 2 * PI_F / 8;
  float f = std::fmod(angle, sector) / sector;
  if (f < 0)
    f += 1;
  switch (ring) {
  case 0:
    return 0.22f;
  case 1: // scalloped, a round petal in each of 8
    return 0.49f + 0.06f * std::sin(PI_F * f);
  case 2:
    return 0.75f;
  default: // pointed, a star of 8
    return 0.75f + 0.25f * (1 - std::fabs(2 * f - 1));
  }
}

// Which place (ring and sector, as one number) a point is in, or -1
int PooPlace(Vector2 p, Vector2 centre, float radius) {
  const float r = Distance(p, centre) / radius;
  float angle = std::atan2(p.y - centre.y, p.x - centre.x);
  if (angle < 0)
    angle += 2 * PI_F;
  int first = 0;
  for (int ring = 0; ring < POO_RINGS; ring++) {
    if (r < PooEdge(ring, angle)) {
      const int sector =
          (int)(angle / (2 * PI_F / POO_SECTORS[ring])) % POO_SECTORS[ring];
      return first + sector;
    }
    first += POO_SECTORS[ring];
  }
  return -1;
}

int PooPlaceCount() {
  int count = 0;
  for (int s : POO_SECTORS)
    count += s;
  return count;
}

void DrawPooOutline(Vector2 centre, float radius, float thick, Color c) {
  const int SAMPLES = 160;
  for (int ring = 0; ring < POO_RINGS; ring++) {
    Vector2 last = {0, 0};
    for (int i = 0; i <= SAMPLES; i++) {
      const float a = 2 * PI_F * i / SAMPLES;
      const float r = PooEdge(ring, a) * radius;
      const Vector2 p = {centre.x + std::cos(a) * r, centre.y + std::sin(a) * r};
      if (i > 0)
        DrawLineEx(last, p, thick, c);
      last = p;
    }
  }
  // the lines between the places in rings 1 and 2
  for (int ring = 1; ring <= 2; ring++) {
    for (int s = 0; s < POO_SECTORS[ring]; s++) {
      const float a = 2 * PI_F * s / POO_SECTORS[ring];
      const float r1 = PooEdge(ring - 1, a) * radius;
      const float r2 = PooEdge(ring, a) * radius;
      DrawLineEx({centre.x + std::cos(a) * r1, centre.y + std::sin(a) * r1},
                 {centre.x + std::cos(a) * r2, centre.y + std::sin(a) * r2},
                 thick, c);
    }
  }
}

// A random point inside place `place` of a flower
Vector2 PointInPooPlace(Vector2 centre, float radius, int place) {
  int ring = 0;
  while (place >= POO_SECTORS[ring]) {
    place -= POO_SECTORS[ring];
    ring++;
  }
  const float span = 2 * PI_F / POO_SECTORS[ring];
  const float a = span * (place + GetRandomValue(30, 970) / 1000.0f);
  const float in = ring == 0 ? 0 : PooEdge(ring - 1, a) * radius + 3;
  const float out = PooEdge(ring, a) * radius - 3;
  // spread evenly over the area, not bunched towards the middle
  const float u = GetRandomValue(0, 1000) / 1000.0f;
  const float r = std::sqrt(in * in + (out * out - in * in) * u);
  return {centre.x + std::cos(a) * r, centre.y + std::sin(a) * r};
}

// Fills place `place` of a flower with one colour (for Amma's drawing, and
// under the petals as they cover a place)
void FillPooPlace(Vector2 centre, float radius, int place, Color c) {
  int ring = 0;
  while (place >= POO_SECTORS[ring]) {
    place -= POO_SECTORS[ring];
    ring++;
  }
  const float span = 2 * PI_F / POO_SECTORS[ring];
  const int STEPS = 10;
  for (int i = 0; i < STEPS; i++) {
    const float a1 = span * (place + (float)i / STEPS);
    const float a2 = span * (place + (float)(i + 1) / STEPS);
    auto at = [&](float a, float r) {
      return Vector2{centre.x + std::cos(a) * r * radius,
                     centre.y + std::sin(a) * r * radius};
    };
    const float in1 = ring == 0 ? 0 : PooEdge(ring - 1, a1);
    const float in2 = ring == 0 ? 0 : PooEdge(ring - 1, a2);
    const Vector2 p1 = at(a1, in1), p2 = at(a2, in2);
    const Vector2 q1 = at(a1, PooEdge(ring, a1)), q2 = at(a2, PooEdge(ring, a2));
    FillTriangle(p1, q1, q2, c);
    FillTriangle(p1, q2, p2, c);
  }
}

//------------------------------------------------------------------------------
// Lotus: rings of 8 pointed petals around a round middle
//------------------------------------------------------------------------------
const int LOTUS_PETALS = 8;
const float LOTUS_MIDDLE = 0.13f; // the middle's radius, of the flower's
const float LOTUS_WIDTH = PI_F / LOTUS_PETALS * 0.95f; // half a petal, widest

struct LotusRing {
  float tip;   // how far out the petals reach, of the flower's radius
  float angle; // where the first petal points
  int colour;  // index into COLOURS; 0 = flour outline only
};

float LotusBase(float tip) { return std::fmax(LOTUS_MIDDLE * 0.8f, tip * 0.22f); }

// How wide (half, in radians) a petal is at s (0 = base, 1 = tip)
float LotusHalfWidth(float s) {
  return LOTUS_WIDTH * std::pow(std::sin(PI_F * std::pow(s, 0.75f)), 0.85f);
}

// `blank`: what an uncoloured petal is filled with (the floor, or paper), so
// the rings behind don't show through
void DrawLotusRing(Vector2 centre, float radius, const LotusRing &ring,
                   float thick, Color outline, Color blank) {
  const float base = LotusBase(ring.tip);
  const int SIDE = 14;
  for (int p = 0; p < LOTUS_PETALS; p++) {
    const float axis = ring.angle + 2 * PI_F * p / LOTUS_PETALS;
    std::vector<Vector2> shape;
    for (int i = 0; i <= SIDE; i++) { // up one side, down the other
      const float s = (float)i / SIDE;
      const float r = (base + (ring.tip - base) * s) * radius;
      const float a = axis - LotusHalfWidth(s);
      shape.push_back({centre.x + std::cos(a) * r, centre.y + std::sin(a) * r});
    }
    for (int i = SIDE - 1; i >= 1; i--) {
      const float s = (float)i / SIDE;
      const float r = (base + (ring.tip - base) * s) * radius;
      const float a = axis + LotusHalfWidth(s);
      shape.push_back({centre.x + std::cos(a) * r, centre.y + std::sin(a) * r});
    }
    const float mid = (base + (ring.tip - base) * 0.45f) * radius;
    const Vector2 middle = {centre.x + std::cos(axis) * mid,
                            centre.y + std::sin(axis) * mid};
    const Color fill = ring.colour > 0 ? COLOURS[ring.colour] : blank;
    for (size_t i = 0; i < shape.size(); i++)
      FillTriangle(middle, shape[i], shape[(i + 1) % shape.size()], fill);
    for (size_t i = 0; i < shape.size(); i++)
      DrawLineEx(shape[i], shape[(i + 1) % shape.size()], thick, outline);
    // a vein down the middle
    DrawLineEx({centre.x + std::cos(axis) * base * radius * 1.3f,
                centre.y + std::sin(axis) * base * radius * 1.3f},
               {centre.x + std::cos(axis) * ring.tip * radius * 0.85f,
                centre.y + std::sin(axis) * ring.tip * radius * 0.85f},
               thick * 0.6f, Faded(outline, 0.6f));
  }
}

void DrawLotusMiddle(Vector2 centre, float radius, int colour, float thick,
                     Color outline) {
  DrawCircleV(centre, LOTUS_MIDDLE * radius, COLOURS[colour]);
  DrawRing(centre, LOTUS_MIDDLE * radius - thick, LOTUS_MIDDLE * radius, 0, 360,
           40, outline);
  for (int i = 0; i < 6; i++) { // seeds
    const float a = 2 * PI_F * i / 6;
    DrawCircleV({centre.x + std::cos(a) * radius * LOTUS_MIDDLE * 0.5f,
                 centre.y + std::sin(a) * radius * LOTUS_MIDDLE * 0.5f},
                std::fmax(1.0f, radius * 0.014f), Faded(outline, 0.8f));
  }
}

// Is `p` on a petal of this ring?
bool OnLotusRing(Vector2 p, Vector2 centre, float radius, const LotusRing &ring) {
  const float r = Distance(p, centre) / radius;
  const float base = LotusBase(ring.tip);
  if (r < base || r > ring.tip)
    return false;
  const float s = (r - base) / (ring.tip - base);
  const float angle = std::atan2(p.y - centre.y, p.x - centre.x);
  const float step = 2 * PI_F / LOTUS_PETALS;
  float d = std::fmod(angle - ring.angle, step);
  if (d < 0)
    d += step;
  d = std::fmin(d, step - d);
  return d <= LotusHalfWidth(s);
}

//------------------------------------------------------------------------------
// Amma's drawings, in a square `area`: the same on her note and on the cards
//------------------------------------------------------------------------------
void AmmasPulli(Rectangle area) {
  const int N = 5;
  const float cell = area.width / N;
  auto dot = [&](int x, int y) {
    return Vector2{area.x + (x + 0.5f) * cell, area.y + (y + 0.5f) * cell};
  };
  const float thick = std::fmax(1.4f, cell * 0.06f);
  const int square[4][2] = {{1, 1}, {3, 1}, {3, 3}, {1, 3}};
  const int diamond[4][2] = {{2, 0}, {4, 2}, {2, 4}, {0, 2}};
  for (int i = 0; i < 4; i++) {
    const int j = (i + 1) % 4;
    DrawLineEx(dot(square[i][0], square[i][1]), dot(square[j][0], square[j][1]),
               thick, NOTE_INK);
    DrawLineEx(dot(diamond[i][0], diamond[i][1]),
               dot(diamond[j][0], diamond[j][1]), thick, NOTE_INK);
  }
  DrawLineEx(dot(0, 0), dot(4, 4), thick * 0.7f, Faded(NOTE_INK, 0.6f));
  DrawLineEx(dot(4, 0), dot(0, 4), thick * 0.7f, Faded(NOTE_INK, 0.6f));
  for (int y = 0; y < N; y++)
    for (int x = 0; x < N; x++)
      DrawCircleV(dot(x, y), std::fmax(1.2f, cell * 0.07f), NOTE_FAINT);
}

void AmmasPadi(Rectangle area) {
  const float step = area.width / PADI_N;
  const float scale = step / 57.5f;
  for (const auto &s : AmmasPadi()) {
    const Vector2 a = {area.x + s.first.x * step, area.y + s.first.y * step};
    const Vector2 b = {a.x + (s.first.across ? step : 0),
                       a.y + (s.first.across ? 0 : step)};
    // in her note, ink stands in for the flour
    const bool across = s.first.across;
    const float off = std::fmax(1.2f, 3.6f * scale);
    const Vector2 side = across ? Vector2{0, off} : Vector2{off, 0};
    if (s.second == PADI_KAAVI)
      DrawLineEx(a, b, off * 2, KAAVI);
    DrawLineEx({a.x - side.x, a.y - side.y}, {b.x - side.x, b.y - side.y}, 1,
               NOTE_INK);
    DrawLineEx({a.x + side.x, a.y + side.y}, {b.x + side.x, b.y + side.y}, 1,
               NOTE_INK);
  }
}

void AmmasPoo(Rectangle area) {
  const Vector2 c = {area.x + area.width / 2, area.y + area.height / 2};
  const float radius = area.width / 2;
  const int count = PooPlaceCount();
  for (int place = 0; place < count; place++) {
    Color colour = COLOURS[2]; // the middle: yellow
    if (place >= 1 && place < 9)
      colour = COLOURS[1]; // marigold
    else if (place >= 9 && place < 25)
      colour = place % 2 ? COLOURS[3] : COLOURS[0]; // rose and jasmine
    else if (place >= 25)
      colour = COLOURS[5]; // leaves
    FillPooPlace(c, radius, place, colour);
  }
  DrawPooOutline(c, radius, 1, NOTE_INK);
}

void AmmasLotus(Rectangle area) {
  const Vector2 c = {area.x + area.width / 2, area.y + area.height / 2};
  const float radius = area.width / 2;
  DrawLotusRing(c, radius, {0.98f, -PI_F / 2, 4}, 1, NOTE_INK, NOTE_PAPER);
  DrawLotusRing(c, radius, {0.66f, -PI_F / 2 + PI_F / 8, 0}, 1, NOTE_INK,
                NOTE_PAPER);
  DrawLotusMiddle(c, radius, 2, 1, NOTE_INK);
}

void AmmasFreehand(Rectangle area) {
  const Vector2 c = {area.x + area.width / 2, area.y + area.height / 2};
  const float radius = area.width / 2 * 0.95f;
  const int SAMPLES = 240;
  Vector2 last = {0, 0};
  for (int i = 0; i <= SAMPLES; i++) { // an eight-petalled rose curve
    const float a = 2 * PI_F * i / SAMPLES;
    const float r = radius * std::fabs(std::cos(4 * a)) * 0.9f + radius * 0.1f;
    const Vector2 p = {c.x + std::cos(a) * r, c.y + std::sin(a) * r};
    if (i > 0)
      DrawLineEx(last, p, 1.4f, NOTE_INK);
    last = p;
  }
  for (int i = 0; i <= SAMPLES; i++) { // and a swirl in the middle
    const float a = 4 * PI_F * i / SAMPLES;
    const float r = radius * 0.32f * i / SAMPLES;
    const Vector2 p = {c.x + std::cos(a) * r, c.y + std::sin(a) * r};
    if (i > 0)
      DrawLineEx(last, p, 1.2f, NOTE_INK);
    last = p;
  }
}

//------------------------------------------------------------------------------
// The scene every kind shares: the floor, Amma's note, the buttons, Done
//------------------------------------------------------------------------------
class KindScene : public Scene {
public:
  explicit KindScene(int kindOf) : kind(kindOf) {
    for (int i = 0; i < 260; i++) // grit and specks on the floor, made once
      specks.push_back({(float)GetRandomValue(-300, 1020),
                        (float)GetRandomValue(0, 720),
                        (float)GetRandomValue(5, 18) / 10.0f});
    PlayEffect("page", 0.5f);
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    if (finished) {
      finishTime += dt;
      if (finishTime > 1.2f && ContinuePressed())
        done = true;
      return;
    }
    // the buttons
    if (Clicked(ButtonRect(0)) || IsKeyPressed(KEY_R)) {
      Clear();
      PlayEffect("page", 0.5f);
      return;
    }
    if (Clicked(ButtonRect(1)) && HasSomething()) {
      finished = true;
      PlayEffect("bell", 0.6f);
      return;
    }
    if (HasEraser() && (Clicked(DrawToolRect()) || Clicked(EraseToolRect()))) {
      erasing = CheckCollisionPointRec(GetMousePosition(), EraseToolRect());
      wiping = false;
      return;
    }
    if (HasUndo() && (Clicked(ButtonRect(2)) || IsKeyPressed(KEY_Z))) {
      Undo();
      return;
    }
    for (int i = 0; i < SwatchCount(); i++) {
      if (Clicked(SwatchRect(i))) {
        colour = SwatchColour(i);
        return;
      }
    }
    if (ExtraButton() && Clicked(ExtraRect())) {
      ExtraPressed();
      return;
    }
    if (erasing) { // swipe over lines to wipe them away
      if (PressedOnFloor())
        wiping = true;
      if (wiping && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        EraseAt(GetMousePosition());
      if (wiping && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        wiping = false;
        EndWipe();
      }
      return;
    }
    UpdateKolam(dt);
  }

  void Draw() override {
    DrawBandGradient(0, 720, FLOOR_TOP, FLOOR_BOTTOM);
    for (const Vector3 &s : specks)
      DrawCircleV({s.x, s.y}, s.z, {70, 62, 52, 255});
    DrawRectangleRounded({AREA.x - 24, AREA.y - 24, AREA.width + 48,
                          AREA.height + 48},
                         0.08f, 8, SWEPT);
    DrawKolam();
    rub.DrawFading();
    if (!finished && erasing)
      DrawEraserRing();

    if (finished) { // admired: a few glints, then back to the kolams
      const std::vector<Vector2> points = Highlights();
      for (size_t i = 0; i < points.size() && !points.empty(); i += 7) {
        const float t = std::fmod(finishTime * 0.9f + i * 0.137f, 1.0f);
        DrawSparkle(points[i], 10 * std::sin(PI_F * t), Faded(WHITE, 0.9f));
      }
      if (finishTime > 1.2f) {
        const float blink = 0.55f + 0.45f * std::sin(time * 3);
        DrawTextCentered(std::string(Tap()) + " to go back to the kolams", 290,
                         640, 18,
                         Faded(FLOUR, blink));
      }
    } else {
      DrawControls();
    }
    DrawString(NAMES[kind], 40, 22, 34, FLOUR);
    if (!finished)
      DrawString(HINTS[kind], 40, 66, 16, MUTED);
    DrawNote();
  }

protected:
  int kind;
  float time = 0;
  int colour = 0; // the colour chosen, an index into COLOURS
  RubOut rub;     // what Undo or the eraser took, fading away
  bool erasing = false; // the eraser is chosen (Pulli, Padi, Freehand)
  bool wiping = false;  // the eraser is pressed down on the floor

  // What each kind does
  virtual void UpdateKolam(float dt) = 0;
  virtual void DrawKolam() const = 0;
  virtual void Clear() = 0;
  virtual bool HasSomething() const = 0;
  virtual std::vector<Vector2> Highlights() const = 0; // where glints go
  virtual bool HasUndo() const { return false; }
  virtual bool HasEraser() const { return false; } // Draw / Erase tools
  virtual void EraseAt(Vector2) {} // the eraser passes over this point
  virtual void EndWipe() {}        // and is lifted
  virtual void Undo() {}
  virtual int SwatchCount() const { return 0; }
  virtual int SwatchColour(int i) const { return i; }
  virtual bool ExtraButton() const { return false; } // one more, right side
  virtual std::string ExtraLabel() const { return ""; }
  virtual void ExtraPressed() {}

  // A press that starts on the floor (not on a button or the note)
  bool PressedOnFloor() const {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      return false;
    const Vector2 m = GetMousePosition();
    if (CheckCollisionPointRec(m, NOTE))
      return false;
    for (int i = 0; i < 3; i++)
      if ((i < 2 || HasUndo()) && CheckCollisionPointRec(m, ButtonRect(i)))
        return false;
    for (int i = 0; i < SwatchCount(); i++)
      if (CheckCollisionPointRec(m, SwatchRect(i)))
        return false;
    if (ExtraButton() && CheckCollisionPointRec(m, ExtraRect()))
      return false;
    if (HasEraser() && (CheckCollisionPointRec(m, DrawToolRect()) ||
                        CheckCollisionPointRec(m, EraseToolRect())))
      return false;
    return true;
  }

  // Buttons along the bottom: 0 Start over, 1 Done, 2 Undo (if any)
  Rectangle ButtonRect(int i) const {
    const int count = HasUndo() ? 3 : 2;
    const float width = count * BUTTON_W + (count - 1) * 12;
    const int place = HasUndo() ? (i == 2 ? 0 : i + 1) : i;
    return {290 - width / 2 + place * (BUTTON_W + 12), BUTTON_Y, BUTTON_W,
            BUTTON_H};
  }
  // where a colour can be tapped: a little bigger than it's drawn
  Rectangle SwatchRect(int i) const {
    const float reach = SWATCH_R + 5;
    return {SWATCH_X - reach, SwatchTop() + i * SWATCH_GAP - reach, 2 * reach,
            2 * reach};
  }
  // Draw / Erase, side by side under Amma's note (the colours go below)
  Rectangle DrawToolRect() const { return {SWATCH_X - 64, 244, 62, 40}; }
  Rectangle EraseToolRect() const { return {SWATCH_X + 2, 244, 62, 40}; }
  float SwatchTop() const { return HasEraser() ? 320 : SWATCH_TOP; }
  Rectangle ExtraRect() const {
    return {SWATCH_X - 62, SwatchTop() + SwatchCount() * SWATCH_GAP + 4, 124,
            40};
  }

private:
  std::vector<Vector3> specks;
  bool finished = false;
  float finishTime = 0;

  void DrawButtonAt(Rectangle r, const std::string &label, bool on) const {
    const bool over = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.3f, 8,
                         on ? (over ? KUMKUM : Faded(KUMKUM, 0.8f))
                            : Faded(FLOUR, over ? 0.14f : 0.07f));
    DrawRectangleRoundedLinesEx(r, 0.3f, 8, 1, Faded(FLOUR, 0.3f));
    DrawTextCentered(label, r.x + r.width / 2, r.y + 12, 20,
                     on ? WHITE : FLOUR);
  }

  void DrawControls() const {
    DrawButtonAt(ButtonRect(0), "Start over", false);
    DrawButtonAt(ButtonRect(1), "Done", HasSomething());
    if (HasUndo())
      DrawButtonAt(ButtonRect(2), "Undo", false);
    for (int i = 0; i < SwatchCount(); i++) {
      const Rectangle r = SwatchRect(i);
      const Vector2 c = {r.x + r.width / 2, r.y + r.height / 2};
      Color swatch = COLOURS[SwatchColour(i)];
      if (kind == KOLAM_PADI && i == 1)
        swatch = KAAVI;
      if (kind == KOLAM_LOTUS && SwatchColour(i) == 0)
        swatch = SWEPT; // no colour: just the outline
      if (SwatchColour(i) == colour)
        DrawRing(c, SWATCH_R + 3, SWATCH_R + 6, 0, 360, 32, FLOUR);
      DrawCircleV(c, SWATCH_R, swatch);
      DrawRing(c, SWATCH_R - 1.5f, SWATCH_R, 0, 360, 32, Faded(BLACK, 0.4f));
    }
    if (ExtraButton())
      DrawButtonAt(ExtraRect(), ExtraLabel(), false);
    if (HasEraser()) {
      DrawButtonAt(DrawToolRect(), "Draw", !erasing);
      DrawButtonAt(EraseToolRect(), "Erase", erasing);
    }
  }

  // Amma's note, pinned at a slight angle in the top-right corner
  void DrawNote() const {
    rlPushMatrix();
    rlTranslatef(NOTE.x + NOTE.width / 2, NOTE.y + NOTE.height / 2, 0);
    rlRotatef(-3, 0, 0, 1);
    rlTranslatef(-NOTE.width / 2, -NOTE.height / 2, 0);
    DrawRectangle(5, 7, (int)NOTE.width, (int)NOTE.height, {0, 0, 0, 90});
    DrawRectangle(0, 0, (int)NOTE.width, (int)NOTE.height, NOTE_PAPER);
    DrawRectangle((int)NOTE.width / 2 - 26, -8, 52, 18,
                  {236, 226, 190, 200}); // a strip of tape
    DrawAmmasKolam(kind, {14, 14, NOTE.width - 28, NOTE.width - 28});
    DrawTextWrapped(AMMAS_NOTES[kind], 12, NOTE.width - 6, 15, NOTE.width - 20,
                    NOTE_INK, HAND_FONT);
    rlPopMatrix();
  }
};

//------------------------------------------------------------------------------
// Pulli: straight lines from dot to dot, copied to all eight places
//------------------------------------------------------------------------------
class PulliScene : public KindScene {
public:
  PulliScene() : KindScene(KOLAM_PULLI) {}

private:
  static int Dots() { return PULLI_DOTS; }
  std::set<std::pair<int, int>> lines; // pairs of dot numbers, smaller first
  // the lines in the order they were drawn, one drag (and its copies) each,
  // for Undo
  std::vector<std::vector<std::pair<int, int>>> drawnInOrder;
  int from = -1;                       // the dot a drag started on

  // Forgets a line (and its copies), with a fade
  void Remove(const std::pair<int, int> &l) {
    if (lines.erase(l))
      rub.Fade({Dot(l.first), Dot(l.second)}, 4, COLOURS[0]);
    for (auto &g : drawnInOrder)
      g.erase(std::remove(g.begin(), g.end(), l), g.end());
  }

  void EraseAt(Vector2 p) override { // every line the eraser touches
    std::vector<std::pair<int, int>> touched;
    for (const auto &l : lines)
      if (DistanceToSegment(p, Dot(l.first), Dot(l.second)) < ERASER_RADIUS)
        touched.push_back(l);
    for (const auto &l : touched)
      for (const auto &image : LineImages(l.first, l.second))
        Remove(image);
    if (!touched.empty())
      PlayLoop("pencil", 0.3f);
  }

  bool HasUndo() const override { return true; }
  bool HasEraser() const override { return true; }
  void Undo() override { // the last line drawn (and its copies)
    while (!drawnInOrder.empty() && drawnInOrder.back().empty())
      drawnInOrder.pop_back();
    if (drawnInOrder.empty())
      return;
    const std::vector<std::pair<int, int>> last = drawnInOrder.back();
    for (const auto &l : last)
      Remove(l);
    drawnInOrder.pop_back();
    PlayEffect("scribble", 0.6f, 0.8f);
  }

  static float Cell() { return AREA.width / Dots(); }
  static Vector2 Dot(int i) {
    return {AREA.x + (i % Dots() + 0.5f) * Cell(), AREA.y + (i / Dots() + 0.5f) * Cell()};
  }
  static int DotNear(Vector2 p) {
    for (int i = 0; i < Dots() * Dots(); i++)
      if (Distance(p, Dot(i)) < Cell() * (TouchUi() ? 0.48f : 0.4f))
        return i;
    return -1;
  }
  static std::vector<std::pair<int, int>> LineImages(int a, int b) {
    const auto ia = Images(a % Dots(), a / Dots(), Dots() - 1);
    const auto ib = Images(b % Dots(), b / Dots(), Dots() - 1);
    std::vector<std::pair<int, int>> out;
    for (size_t i = 0; i < ia.size(); i++) {
      const int p = ia[i].second * Dots() + ia[i].first;
      const int q = ib[i].second * Dots() + ib[i].first;
      out.push_back({std::min(p, q), std::max(p, q)});
    }
    return out;
  }

  void UpdateKolam(float) override {
    const Vector2 m = GetMousePosition();
    if (DotNear(m) >= 0)
      PointAtSomething();
    if (PressedOnFloor())
      from = DotNear(m);
    if (from >= 0 && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      const int to = DotNear(m);
      if (to >= 0 && to != from) {
        const auto images = LineImages(from, to);
        const bool had = lines.count(images[0]) > 0;
        std::vector<std::pair<int, int>> added;
        for (const auto &l : images) {
          if (had) {
            Remove(l);
          } else if (lines.insert(l).second) {
            added.push_back(l);
          }
        }
        if (!added.empty())
          drawnInOrder.push_back(added);
        PlayEffect(had ? "scribble" : "pencil", 0.6f);
      }
      from = -1;
    }
  }

  void DrawKolam() const override {
    for (const auto &l : lines)
      FlourLine(Dot(l.first), Dot(l.second), 4, COLOURS[0]);
    if (from >= 0) { // the line being drawn, and where its copies will go
      const int to = DotNear(GetMousePosition());
      const Vector2 end = to >= 0 ? Dot(to) : GetMousePosition();
      if (to >= 0 && to != from)
        for (const auto &l : LineImages(from, to))
          DrawLineEx(Dot(l.first), Dot(l.second), 3, Faded(FLOUR, 0.3f));
      DrawLineEx(Dot(from), end, 4, Faded(FLOUR, 0.8f));
    }
    for (int i = 0; i < Dots() * Dots(); i++)
      DrawCircleV(Dot(i), 4.5f, COLOURS[0]);
  }

  void Clear() override {
    lines.clear();
    drawnInOrder.clear();
  }
  bool HasSomething() const override { return !lines.empty(); }
  std::vector<Vector2> Highlights() const override {
    std::vector<Vector2> points;
    for (const auto &l : lines) {
      const Vector2 a = Dot(l.first), b = Dot(l.second);
      points.push_back({(a.x + b.x) / 2, (a.y + b.y) / 2});
    }
    return points;
  }
};

//------------------------------------------------------------------------------
// Padi: straight lines along the grid, steps around the square, kaavi too
//------------------------------------------------------------------------------
class PadiScene : public KindScene {
public:
  PadiScene() : KindScene(KOLAM_PADI) { Clear(); }

private:
  std::map<PadiStep, int> steps;
  bool dragging = false;
  int fromX = 0, fromY = 0;
  // Each drag's (or eraser swipe's) changes, in order: a step and what it
  // was before (NONE = empty). Undo puts the last one back.
  std::vector<std::vector<std::pair<PadiStep, int>>> changes;

  std::vector<std::pair<PadiStep, int>> wiped; // this eraser swipe's changes

  Vector2 StepEnd(PadiStep s) const {
    return s.across ? Point(s.x + 1, s.y) : Point(s.x, s.y + 1);
  }

  void EraseAt(Vector2 p) override { // every step the eraser touches
    std::vector<PadiStep> touched;
    for (const auto &s : steps)
      if (DistanceToSegment(p, Point(s.first.x, s.first.y), StepEnd(s.first)) <
          ERASER_RADIUS)
        touched.push_back(s.first);
    for (const PadiStep &t : touched)
      for (const PadiStep &image : PadiImages(t)) {
        auto was = steps.find(image);
        if (was == steps.end())
          continue;
        rub.Fade({Point(image.x, image.y), StepEnd(image)}, 7,
                 was->second == PADI_KAAVI ? KAAVI : COLOURS[0]);
        wiped.push_back({image, was->second});
        steps.erase(was);
      }
    if (!touched.empty())
      PlayLoop("pencil", 0.3f);
  }
  void EndWipe() override {
    if (!wiped.empty())
      changes.push_back(wiped);
    wiped.clear();
  }

  bool HasUndo() const override { return true; }
  bool HasEraser() const override { return true; }
  void Undo() override { // puts the last drag (or swipe) back as it was
    if (changes.empty())
      return;
    const auto &last = changes.back();
    for (auto c = last.rbegin(); c != last.rend(); ++c) {
      auto now = steps.find(c->first);
      if (now != steps.end() && c->second == PADI_NONE)
        rub.Fade({Point(c->first.x, c->first.y), StepEnd(c->first)}, 7,
                 now->second == PADI_KAAVI ? KAAVI : COLOURS[0]);
      if (c->second == PADI_NONE)
        steps.erase(c->first);
      else
        steps[c->first] = c->second;
    }
    changes.pop_back();
    PlayEffect("scribble", 0.6f, 0.8f);
  }

  static float Step() { return AREA.width / PADI_N; }
  static Vector2 Point(int x, int y) {
    return {AREA.x + x * Step(), AREA.y + y * Step()};
  }
  static bool PointNear(Vector2 p, int *x, int *y) {
    *x = (int)std::lround((p.x - AREA.x) / Step());
    *y = (int)std::lround((p.y - AREA.y) / Step());
    return *x >= 0 && *y >= 0 && *x <= PADI_N && *y <= PADI_N &&
           Distance(p, Point(*x, *y)) < Step() * (TouchUi() ? 0.5f : 0.45f);
  }
  // The steps from the drag's start towards the mouse, straight across or
  // straight down, whichever the mouse is further along
  std::vector<PadiStep> Dragged() const {
    const Vector2 m = GetMousePosition();
    const Vector2 a = Point(fromX, fromY);
    std::vector<PadiStep> out;
    if (std::fabs(m.x - a.x) >= std::fabs(m.y - a.y)) {
      int to = (int)std::lround((m.x - AREA.x) / Step());
      to = std::max(0, std::min(PADI_N, to));
      for (int x = std::min(fromX, to); x < std::max(fromX, to); x++)
        out.push_back({x, fromY, true});
    } else {
      int to = (int)std::lround((m.y - AREA.y) / Step());
      to = std::max(0, std::min(PADI_N, to));
      for (int y = std::min(fromY, to); y < std::max(fromY, to); y++)
        out.push_back({fromX, y, false});
    }
    return out;
  }
  int Tool() const { return colour == 1 ? PADI_KAAVI : PADI_FLOUR; }

  void UpdateKolam(float) override {
    const Vector2 m = GetMousePosition();
    int x = 0, y = 0;
    if (PointNear(m, &x, &y))
      PointAtSomething();
    if (PressedOnFloor() && PointNear(m, &x, &y)) {
      dragging = true;
      fromX = x;
      fromY = y;
    }
    if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      dragging = false;
      const std::vector<PadiStep> line = Dragged();
      if (line.empty())
        return;
      bool allSet = true; // drawing over the same again rubs it out
      for (const PadiStep &s : line) {
        auto it = steps.find(s);
        allSet = allSet && it != steps.end() && it->second == Tool();
      }
      std::vector<std::pair<PadiStep, int>> changed;
      for (const PadiStep &s : line)
        for (const PadiStep &image : PadiImages(s)) {
          auto was = steps.find(image);
          const int before = was == steps.end() ? PADI_NONE : was->second;
          if (allSet)
            steps.erase(image);
          else
            steps[image] = Tool();
          changed.push_back({image, before});
        }
      changes.push_back(changed);
      PlayEffect(allSet ? "scribble" : "pencil", 0.6f);
    }
  }

  void DrawKolam() const override {
    for (int y = 0; y <= PADI_N; y++)
      for (int x = 0; x <= PADI_N; x++)
        DrawCircleV(Point(x, y), 2, Faded(FLOUR, 0.25f));
    // kaavi first, so flour lines cross over it
    for (int pass = 0; pass < 2; pass++)
      for (const auto &s : steps) {
        if ((s.second == PADI_KAAVI) != (pass == 0))
          continue;
        const Vector2 a = Point(s.first.x, s.first.y);
        const Vector2 b = s.first.across ? Point(s.first.x + 1, s.first.y)
                                         : Point(s.first.x, s.first.y + 1);
        DrawPadiStep(a, b, s.second, 1);
      }
    if (dragging)
      for (const PadiStep &s : Dragged())
        for (const PadiStep &image : PadiImages(s)) {
          const Vector2 a = Point(image.x, image.y);
          const Vector2 b = image.across ? Point(image.x + 1, image.y)
                                         : Point(image.x, image.y + 1);
          DrawLineEx(a, b, 6, Faded(Tool() == PADI_KAAVI ? KAAVI : FLOUR, 0.4f));
        }
  }

  void Clear() override { // back to just the square in the middle
    steps.clear();
    changes.clear();
    wiped.clear();
    for (int i = 3; i < 5; i++) {
      steps[{i, 3, true}] = PADI_FLOUR;
      steps[{i, 5, true}] = PADI_FLOUR;
      steps[{3, i, false}] = PADI_FLOUR;
      steps[{5, i, false}] = PADI_FLOUR;
    }
  }
  bool HasSomething() const override { return steps.size() > 8; }
  int SwatchCount() const override { return 2; } // flour, kaavi
  std::vector<Vector2> Highlights() const override {
    std::vector<Vector2> points;
    for (const auto &s : steps)
      points.push_back(Point(s.first.x, s.first.y));
    return points;
  }
};

//------------------------------------------------------------------------------
// Poo: petals of the chosen colour, painted into the flower's places
//------------------------------------------------------------------------------
class PooScene : public KindScene {
public:
  PooScene() : KindScene(KOLAM_POO) {
    colour = 1;
    Clear();
  }

private:
  struct Petal {
    Vector2 at;
    Color colour;
    float size, turn;
  };
  static float Radius() { return POO_RADIUS; }
  // Holding over a place keeps petals falling into it, all over it, until
  // it's full with no gaps: under the petals a bed of powder (`bed`, in the
  // petals' colour) grows from nothing to solid as they pile up. Painting a
  // full place another colour slowly turns it, the oldest petals first.
  std::vector<std::vector<Petal>> places; // the petals in each place
  std::vector<int> nextPetal;             // when a place is full: replace this
  std::vector<float> bed;   // how solid the powder under each place is, 0..1
  std::vector<Color> bedColour;
  float spawn = 0;          // petals owed (they fall steadily)
  bool painting = false;

  static int Capacity(int place) { // plenty to look heaped up
    if (place == 0)
      return 260;
    return place < 9 ? 150 : 140;
  }

  void Drop(Vector2 finger) {
    const int place = PooPlace(finger, CENTRE, Radius());
    if (place < 0)
      return;
    // half fall round the fingertip, half anywhere in the place it's on
    Vector2 at = PointInPooPlace(CENTRE, Radius(), place);
    if (GetRandomValue(0, 1)) {
      const float a = GetRandomValue(0, 628) / 100.0f;
      const float d = 16 * std::sqrt(GetRandomValue(0, 1000) / 1000.0f);
      const Vector2 near = {finger.x + std::cos(a) * d,
                            finger.y + std::sin(a) * d};
      if (PooPlace(near, CENTRE, Radius()) == place)
        at = near;
    }
    const Color c = COLOURS[colour];
    const Petal petal = {at, Vary(c, 14), 4.2f + GetRandomValue(0, 30) / 10.0f,
                         (float)GetRandomValue(0, 60)};
    std::vector<Petal> &here = places[place];
    if ((int)here.size() < Capacity(place)) {
      here.push_back(petal);
    } else { // full: the oldest petal gives way
      here[nextPetal[place]] = petal;
      nextPetal[place] = (nextPetal[place] + 1) % Capacity(place);
    }
    // the powder bed: solid once the place has had about its fill of petals,
    // drifting towards the colour being poured
    const float step = 1.0f / (Capacity(place) * 0.7f);
    if (bed[place] <= 0)
      bedColour[place] = c;
    bed[place] = std::fmin(1.0f, bed[place] + step);
    const float mix = 2.5f / Capacity(place);
    Color &b = bedColour[place];
    b = {(unsigned char)(b.r + (c.r - b.r) * mix),
         (unsigned char)(b.g + (c.g - b.g) * mix),
         (unsigned char)(b.b + (c.b - b.b) * mix), 255};
  }

  void UpdateKolam(float dt) override {
    const Vector2 m = GetMousePosition();
    if (PooPlace(m, CENTRE, Radius()) >= 0)
      PointAtSomething();
    if (PressedOnFloor()) {
      painting = true;
      for (int i = 0; i < 14; i++) // a handful at once
        Drop(m);
      PlayEffect("pour", 0.35f);
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
      painting = false;
    if (!painting)
      return;
    PlayLoop("pour", 0.25f);
    spawn += dt * 110;
    while (spawn >= 1) {
      Drop(m);
      spawn -= 1;
    }
  }

  void DrawKolam() const override {
    for (int place = 0; place < (int)bed.size(); place++)
      if (bed[place] > 0) // the powder bed, a little darker than the petals
        FillPooPlace(CENTRE, Radius(), place,
                     Faded(ColorBrightness(bedColour[place], -0.12f),
                           std::fmin(1.0f, bed[place] * 1.15f)));
    for (const std::vector<Petal> &here : places)
      for (const Petal &p : here)
        DrawPoly(p.at, 6, p.size, p.turn, p.colour);
    DrawPooOutline(CENTRE, Radius(), 2.6f, COLOURS[0]);
  }

  void Clear() override {
    places.assign(PooPlaceCount(), {});
    nextPetal.assign(PooPlaceCount(), 0);
    bed.assign(PooPlaceCount(), 0);
    bedColour.assign(PooPlaceCount(), COLOURS[0]);
  }
  bool HasSomething() const override {
    for (const auto &here : places)
      if (!here.empty())
        return true;
    return false;
  }
  int SwatchCount() const override { return 6; }
  int SwatchColour(int i) const override {
    const int order[] = {1, 2, 3, 0, 5, 6}; // marigold first
    return order[i];
  }
  std::vector<Vector2> Highlights() const override {
    std::vector<Vector2> points;
    for (const auto &here : places)
      for (size_t i = 0; i < here.size(); i += 9)
        points.push_back(here[i].at);
    return points;
  }
};

//------------------------------------------------------------------------------
// Lotus: drag out rings of petals; tap a ring (or the middle) to colour it
//------------------------------------------------------------------------------
class LotusScene : public KindScene {
public:
  LotusScene() : KindScene(KOLAM_LOTUS) {
    colour = 4;
    Clear();
  }

private:
  static float Radius() { return LOTUS_RADIUS; }
  std::vector<LotusRing> rings; // in the order drawn: biggest first
  int middleColour = 2;
  bool pressing = false, dragging = false;
  Vector2 pressedAt = {0, 0};

  LotusRing RingAtMouse() const {
    const Vector2 m = GetMousePosition();
    const float tip = std::max(
        0.3f, std::min(1.0f, Distance(m, CENTRE) / Radius()));
    return {tip, std::atan2(m.y - CENTRE.y, m.x - CENTRE.x), 0};
  }
  void Sort() {
    std::sort(rings.begin(), rings.end(),
              [](const LotusRing &a, const LotusRing &b) { return a.tip > b.tip; });
  }

  void UpdateKolam(float) override {
    const Vector2 m = GetMousePosition();
    if (Distance(m, CENTRE) < Radius())
      PointAtSomething();
    if (PressedOnFloor()) {
      pressing = true;
      dragging = false;
      pressedAt = m;
    }
    if (pressing && Distance(m, pressedAt) > 10)
      dragging = true;
    if (!pressing || !IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
      return;
    pressing = false;
    if (dragging) { // a new ring of petals (or one moved, if as big)
      const LotusRing ring = RingAtMouse();
      rings.erase(std::remove_if(rings.begin(), rings.end(),
                                 [&](const LotusRing &r) {
                                   return std::fabs(r.tip - ring.tip) < 0.06f;
                                 }),
                  rings.end());
      if (rings.size() >= 6)
        rings.erase(rings.begin());
      LotusRing added = ring;
      added.colour = colour;
      rings.push_back(added);
      Sort();
      PlayEffect("pencil", 0.6f);
      return;
    }
    // a tap: colour the top thing under it
    if (Distance(m, CENTRE) < LOTUS_MIDDLE * Radius()) {
      middleColour = colour == 0 ? 2 : colour;
      PlayEffect("pour", 0.4f);
      return;
    }
    for (int i = (int)rings.size() - 1; i >= 0; i--) { // smallest on top
      if (OnLotusRing(m, CENTRE, Radius(), rings[i])) {
        rings[i].colour = colour;
        PlayEffect("pour", 0.4f);
        return;
      }
    }
  }

  void DrawKolam() const override {
    for (const LotusRing &ring : rings)
      DrawLotusRing(CENTRE, Radius(), ring, 2.4f, COLOURS[0], SWEPT);
    if (dragging) { // where the new ring will go
      LotusRing ghost = RingAtMouse();
      for (int p = 0; p < LOTUS_PETALS; p++) {
        const float axis = ghost.angle + 2 * PI_F * p / LOTUS_PETALS;
        DrawLineEx(CENTRE,
                   {CENTRE.x + std::cos(axis) * ghost.tip * Radius(),
                    CENTRE.y + std::sin(axis) * ghost.tip * Radius()},
                   2, Faded(FLOUR, 0.35f));
      }
    }
    DrawLotusMiddle(CENTRE, Radius(), middleColour, 2.4f, COLOURS[0]);
  }

  void Clear() override {
    rings.clear();
    middleColour = 2;
  }
  bool HasSomething() const override { return !rings.empty(); }
  int SwatchCount() const override { return 7; }
  int SwatchColour(int i) const override {
    const int order[] = {4, 3, 0, 1, 2, 5, 6}; // pink first; 0 = no colour
    return order[i];
  }
  std::vector<Vector2> Highlights() const override {
    std::vector<Vector2> points;
    for (const LotusRing &ring : rings)
      for (int p = 0; p < LOTUS_PETALS; p++) {
        const float axis = ring.angle + 2 * PI_F * p / LOTUS_PETALS;
        points.push_back({CENTRE.x + std::cos(axis) * ring.tip * Radius() * 0.8f,
                          CENTRE.y + std::sin(axis) * ring.tip * Radius() * 0.8f});
      }
    return points;
  }
};

//------------------------------------------------------------------------------
// Freehand: draw anything, mirrored all the way round if you like
//------------------------------------------------------------------------------
class FreehandScene : public KindScene {
public:
  FreehandScene() : KindScene(KOLAM_FREEHAND) {}

private:
  struct Stroke {
    std::vector<Vector2> points; // from the middle of the floor
    int colour;
    bool mirrored;
  };
  std::vector<Stroke> strokes;
  bool drawing = false;
  bool mirror = true;

  // A stroke point turned to copy k (0 = as drawn), on the screen
  static Vector2 Turned(Vector2 p, int k) {
    const float a = 2 * PI_F * k / FREEHAND_COPIES;
    return {CENTRE.x + p.x * std::cos(a) - p.y * std::sin(a),
            CENTRE.y + p.x * std::sin(a) + p.y * std::cos(a)};
  }

  // The eraser: wipes away the bits of every stroke (in any of its copies)
  // within reach of `p`, splitting a stroke where it's wiped through
  void EraseAt(Vector2 p) override {
    std::vector<Stroke> kept;
    bool erased = false;
    for (const Stroke &s : strokes) {
      const int copies = s.mirrored ? FREEHAND_COPIES : 1;
      Stroke piece = {{}, s.colour, s.mirrored};
      for (size_t i = 0; i < s.points.size(); i++) {
        bool wiped = false;
        for (int k = 0; k < copies && !wiped; k++)
          wiped = Distance(Turned(s.points[i], k), p) < ERASER_RADIUS;
        if (!wiped) {
          piece.points.push_back(s.points[i]);
          continue;
        }
        erased = true;
        if (i > 0)
          for (int k = 0; k < copies; k++)
            rub.Fade({Turned(s.points[i - 1], k), Turned(s.points[i], k)}, 4,
                     COLOURS[s.colour]);
        if (piece.points.size() >= 2)
          kept.push_back(piece);
        piece.points.clear();
      }
      if (piece.points.size() >= 2)
        kept.push_back(piece);
    }
    if (erased) {
      strokes = kept;
      PlayLoop("pencil", 0.3f);
    }
  }

  void UpdateKolam(float) override {
    const Vector2 m = GetMousePosition();
    const bool onFloor = CheckCollisionPointRec(m, AREA);
    if (PressedOnFloor() && onFloor) {
      drawing = true;
      strokes.push_back({{{m.x - CENTRE.x, m.y - CENTRE.y}}, colour, mirror});
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
      drawing = false;
    if (!drawing)
      return;
    const Vector2 p = {std::max(AREA.x, std::min(AREA.x + AREA.width, m.x)) -
                           CENTRE.x,
                       std::max(AREA.y, std::min(AREA.y + AREA.height, m.y)) -
                           CENTRE.y};
    std::vector<Vector2> &points = strokes.back().points;
    if (Distance(p, points.back()) > 4) {
      points.push_back(p);
      PlayLoop("pencil", 0.4f);
    }
  }

  void DrawStroke(const Stroke &s) const {
    const std::vector<Vector2> path = SmoothOpen(s.points);
    const int copies = s.mirrored ? FREEHAND_COPIES : 1;
    for (int k = 0; k < copies; k++) {
      const float a = 2 * PI_F * k / FREEHAND_COPIES;
      const float c = std::cos(a), sn = std::sin(a);
      std::vector<Vector2> turned;
      for (const Vector2 &p : path)
        turned.push_back({CENTRE.x + p.x * c - p.y * sn,
                          CENTRE.y + p.x * sn + p.y * c});
      if (turned.size() == 1)
        DrawCircleV(turned[0], 2.5f, COLOURS[s.colour]);
      for (size_t i = 0; i + 1 < turned.size(); i++) {
        DrawLineEx(turned[i], turned[i + 1], 4, COLOURS[s.colour]);
        DrawCircleV(turned[i + 1], 2, COLOURS[s.colour]);
      }
    }
  }

  void DrawKolam() const override {
    for (const Stroke &s : strokes)
      DrawStroke(s);
    if (mirror && !strokes.empty() && drawing)
      DrawCircleV(CENTRE, 3, Faded(FLOUR, 0.4f));
  }

  void Clear() override { strokes.clear(); }
  bool HasSomething() const override { return !strokes.empty(); }
  bool HasUndo() const override { return true; }
  bool HasEraser() const override { return true; }
  void Undo() override { // the last line, fading away
    if (strokes.empty())
      return;
    const Stroke &last = strokes.back();
    for (int k = 0; k < (last.mirrored ? FREEHAND_COPIES : 1); k++) {
      std::vector<Vector2> turned;
      for (const Vector2 &p : last.points)
        turned.push_back(Turned(p, k));
      rub.Fade(turned, 4, COLOURS[last.colour]);
    }
    strokes.pop_back();
    PlayEffect("scribble", 0.6f, 0.8f);
  }
  int SwatchCount() const override { return 6; }
  int SwatchColour(int i) const override {
    const int order[] = {0, 1, 2, 3, 5, 6};
    return order[i];
  }
  bool ExtraButton() const override { return true; }
  std::string ExtraLabel() const override {
    return mirror ? "Mirror: on" : "Mirror: off";
  }
  void ExtraPressed() override { mirror = !mirror; }
  std::vector<Vector2> Highlights() const override {
    std::vector<Vector2> points;
    for (const Stroke &s : strokes)
      for (size_t i = 0; i < s.points.size(); i += 6)
        points.push_back({CENTRE.x + s.points[i].x, CENTRE.y + s.points[i].y});
    return points;
  }
};

} // namespace

const char *KolamKindName(int kind) { return NAMES[kind]; }
const char *KolamKindTag(int kind) { return TAGS[kind]; }

void DrawAmmasKolam(int kind, Rectangle area) {
  const float side = std::fmin(area.width, area.height);
  const Rectangle square = {area.x + (area.width - side) / 2,
                            area.y + (area.height - side) / 2, side, side};
  switch (kind) {
  case KOLAM_PULLI:
    AmmasPulli(square);
    break;
  case KOLAM_PADI:
    AmmasPadi(square);
    break;
  case KOLAM_POO:
    AmmasPoo(square);
    break;
  case KOLAM_LOTUS:
    AmmasLotus(square);
    break;
  default:
    AmmasFreehand(square);
    break;
  }
}

std::unique_ptr<Scene> MakeKolamKindScene(int kind) {
  switch (kind) {
  case KOLAM_PULLI:
    return std::unique_ptr<Scene>(new PulliScene());
  case KOLAM_PADI:
    return std::unique_ptr<Scene>(new PadiScene());
  case KOLAM_POO:
    return std::unique_ptr<Scene>(new PooScene());
  case KOLAM_LOTUS:
    return std::unique_ptr<Scene>(new LotusScene());
  default:
    return std::unique_ptr<Scene>(new FreehandScene());
  }
}
