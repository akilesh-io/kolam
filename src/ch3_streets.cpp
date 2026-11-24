// ch3_streets.cpp - Chapter 3, "Familiar Streets": the old streets, nine
// days before Pongal. No list today, so Anu just walks.
//
//   streets_walk    the old street, walked: things she passes flicker back to
//                   how they were (the school wall was another colour; the
//                   tamarind tree wasn't). Ends at the school step, where
//                   five small stones lie.
//   five_stones     a memory: recess, kallangai with Meena. Toss a stone up,
//                   sweep up the others, catch it. Dropping one is funny.
//   streets_meena   Meena now, a boy on her hip and a bag of vegetables
//   paper_boats     a memory: rain, the street a river. Fold a boat, draw it
//                   a face, and steer it down the street past the stones.
#include <algorithm>
#include <cmath>

#include "streets_kit.h"

using namespace streets;

namespace {

//------------------------------------------------------------------------------
// The old street, from Anu's house to the school gate. Positions along it are
// "street x": the street scrolls by under the camera.
//------------------------------------------------------------------------------
const float STREET_GROUND = 560; // where the houses meet the street
const float STREET_FEET = 655;   // where people walk
const float STREET_END = 2240;   // Anu stops here, by the school step
const float CAMERA_MIN = -100, CAMERA_MAX = 2050;
const float SHOP_X = 640, WALL_X = 980, WALL_END = 1560, TREE_X = 1730,
            HOPSCOTCH_X = 2000, GATE_X = 2300;
// Where the five stones lie on the school step
const float STONES_X = 2385;

// The things that flicker back to how they were, as Anu passes them
enum Remembered { SHOP, WALL, TREE, HOPSCOTCH, REMEMBERED_COUNT };
const float REMEMBERED_AT[REMEMBERED_COUNT] = {SHOP_X + 110, WALL_X + 290,
                                               TREE_X, HOPSCOTCH_X + 90};

const Color OLD_OCHRE = {226, 170, 90, 255};
const Color NEW_BLUE = {96, 176, 232, 255};
const Color CHALK = {250, 248, 240, 255};

// Is the old version showing? It flickers in, holds, and flickers out
bool Flicker(float glow, float time, int seed) {
  if (glow > 0.95f)
    return true;
  return glow > 0.05f && std::fmod(time * 11 + seed * 0.37f, 1.0f) < glow;
}

// A wobbly line, as a child draws in chalk or crayon
void ChalkLine(Vector2 a, Vector2 b, float w, Color c, int seed) {
  const Vector2 mid = {(a.x + b.x) / 2 + 2 * std::sin(seed * 3.1f),
                       (a.y + b.y) / 2 + 2 * std::cos(seed * 1.7f)};
  DrawLineEx(a, mid, w, c);
  DrawLineEx(mid, b, w, c);
}

// The elephant little Anu drew on everything: a big body, a trunk with a
// curl, four stumpy legs, one ear and a very happy eye
void DrawChildsElephant(Vector2 c, float s, Color line, float w) {
  int seed = 0;
  Vector2 prev = {};
  for (int i = 0; i <= 20; i++) { // the body
    const float a = i * 2 * PI / 20;
    const Vector2 p = {c.x + s * std::cos(a) * (1 + 0.04f * std::sin(i * 5.0f)),
                       c.y + s * 0.62f * std::sin(a)};
    if (i > 0)
      DrawLineEx(prev, p, w, line);
    prev = p;
  }
  const Vector2 head = {c.x + s * 0.95f, c.y - s * 0.35f};
  for (int i = 0; i <= 14; i++) {
    const float a = i * 2 * PI / 14;
    const Vector2 p = {head.x + s * 0.42f * std::cos(a),
                       head.y + s * 0.4f * std::sin(a)};
    if (i > 0)
      DrawLineEx(prev, p, w, line);
    prev = p;
  }
  DrawSplineSegmentBezierQuadratic({head.x + s * 0.35f, head.y + s * 0.15f},
                                   {head.x + s * 0.75f, head.y + s * 0.9f},
                                   {head.x + s * 0.45f, head.y + s * 0.95f}, w,
                                   line); // the trunk, curled up at the end
  DrawSplineSegmentBezierQuadratic({head.x + s * 0.2f, head.y + s * 0.3f},
                                   {head.x + s * 0.5f, head.y + s * 0.8f},
                                   {head.x + s * 0.3f, head.y + s * 0.85f},
                                   w * 0.8f, line);
  for (int k = 0; k < 4; k++) { // legs
    const float lx = c.x - s * 0.65f + k * s * 0.42f;
    ChalkLine({lx, c.y + s * 0.5f}, {lx + 2, c.y + s * 0.95f}, w, line, seed++);
    ChalkLine({lx + s * 0.2f, c.y + s * 0.55f}, {lx + s * 0.2f, c.y + s * 0.95f},
              w, line, seed++);
    ChalkLine({lx, c.y + s * 0.95f}, {lx + s * 0.2f, c.y + s * 0.95f}, w, line,
              seed++);
  }
  DrawEllipseLines((int)(head.x - s * 0.2f), (int)head.y, (int)(s * 0.25f),
                   (int)(s * 0.32f), line); // an ear
  DrawCircleV({head.x + s * 0.12f, head.y - s * 0.1f}, w * 1.4f, line);
  ChalkLine({c.x - s, c.y}, {c.x - s * 1.25f, c.y + s * 0.3f}, w, line, 9);
}

void DrawShop(float x, float time, bool past) {
  const float g = STREET_GROUND;
  // a little pettikadai: a wooden box with a tin awning
  ComicRect({x, g - 250, 220, 250}, {120, 160, 120, 255}, 2.6f);
  ComicRect({x + 14, g - 220, 192, 120}, {70, 50, 40, 255}, 2);
  DrawToneRect({x + 14, g - 220, 192, 120}, COMIC_INK, 0.3f, 0.6f, TONE_LINES,
               4);
  for (int k = 0; k < 6; k++) { // packets hanging on a string
    const Color P[] = {{240, 70, 60, 255}, {250, 200, 50, 255},
                       {80, 160, 220, 255}};
    ComicRect({x + 24 + k * 30.0f, g - 212 + 4 * std::sin(time + k), 18, 26},
              P[k % 3], 1.2f);
  }
  // a bunch of bananas
  for (int k = 0; k < 5; k++)
    DrawComicLeaf({x + 170, g - 210}, {x + 156 + k * 7.0f, g - 160}, 9,
                  {240, 210, 70, 255}, 1.4f);
  // the striped tin awning
  const Vector2 awning[4] = {{x - 16, g - 230}, {x + 236, g - 230},
                             {x + 220, g - 276}, {x, g - 276}};
  ComicPoly(awning, 4, {236, 236, 226, 255}, 2.4f);
  for (int k = 0; k < 7; k++) {
    const float sx = x + 6 + k * 34.0f;
    const Vector2 stripe[4] = {{sx - 10, g - 230}, {sx + 8, g - 230},
                               {sx + 10, g - 276}, {sx - 4, g - 276}};
    if (k % 2 == 0)
      ComicPoly(stripe, 4, {210, 60, 60, 255}, 1.2f);
  }
  // the counter, and the glass jars of sweets on it
  ComicRect({x - 6, g - 100, 232, 100}, TEAK, 2.6f);
  DrawToneRect({x, g - 90, 220, 80}, COMIC_INK, 0.2f, 0.4f, TONE_LINES, 6);
  const Color SWEETS[] = {{250, 140, 40, 255}, {240, 90, 140, 255},
                          {120, 200, 90, 255}, {250, 220, 80, 255}};
  for (int k = 0; k < 4; k++) {
    const Rectangle jar = {x + 10 + k * 52.0f, g - 156, 42, 54};
    DrawRectangleRec(jar, {220, 236, 240, 160});
    // in the old days, full of orange kamarkattu and pink paal mittai
    const float full = past ? 0.85f : 0.25f;
    DrawRectangleRec({jar.x + 3, jar.y + jar.height * (1 - full), jar.width - 6,
                      jar.height * full - 2},
                     past ? SWEETS[k] : Color{200, 196, 186, 255});
    if (past)
      for (int d = 0; d < 6; d++)
        DrawCircleV({jar.x + 8 + (d % 3) * 12, jar.y + 22 + (d / 3) * 14}, 4,
                    ColorLerp(SWEETS[k], WHITE, 0.4f));
    InkRect(jar, 1.8f);
    ComicRect({jar.x - 2, jar.y - 9, jar.width + 4, 9}, {220, 60, 60, 255},
              1.4f);
    DrawLineEx({jar.x + 6, jar.y + 6}, {jar.x + 6, jar.y + jar.height - 6}, 3,
               Faded(WHITE, 0.7f));
  }
}

// The school's wall. New paint now; when it flickers back, the old ochre
// with chalk all over it, and little Anu's elephant.
void DrawSchoolWall(float x0, float x1, float time, bool past) {
  const float g = STREET_GROUND, top = g - 210;
  // the school behind it: a long tiled roof, a flag pole
  ComicRect({x0 + 40, g - 330, x1 - x0 - 80, 140}, {240, 226, 196, 255}, 2.4f);
  for (int k = 0; k < 6; k++)
    ComicRect({x0 + 80 + k * 80.0f, g - 300, 40, 50}, {60, 110, 150, 255},
              1.8f);
  const Vector2 roof[4] = {{x0 + 20, g - 320}, {x0 + 60, g - 380},
                           {x1 - 60, g - 380}, {x1 - 20, g - 320}};
  ComicPoly(roof, 4, {214, 92, 52, 255}, 2.6f);
  DrawToneRect({x0 + 40, g - 378, x1 - x0 - 80, 56}, {110, 24, 18, 255}, 0,
               0.5f, TONE_DOTS, 4);
  InkLine({x1 - 90, g - 380}, {x1 - 90, g - 470}, 3);
  // the wall
  ComicRect({x0, top, x1 - x0, 210}, past ? OLD_OCHRE : NEW_BLUE, 3);
  ComicRect({x0 - 6, top - 14, x1 - x0 + 12, 16},
            past ? Color{200, 140, 70, 255} : COMIC_PAPER, 2.4f);
  DrawToneRect({x0, top, x1 - x0, 210}, past ? Color{150, 80, 30, 255}
                                             : Color{20, 80, 160, 255},
               0.4f, 0.05f, TONE_DOTS, 5);
  for (float px = x0 + 140; px < x1; px += 140) // pillars in the wall
    InkLine({px, top}, {px, g}, 2);
  if (past) {
    // flaking plaster, and chalk: a sun, a house, stick children, the
    // elephant, and a hopscotch of sums
    for (int k = 0; k < 5; k++)
      DrawEllipse((int)(x0 + 60 + k * 120.0f), (int)(top + 150 + (k % 2) * 30),
                  24, 10, {240, 200, 140, 255});
    const Vector2 sun = {x0 + 70, top + 55};
    DrawCircleLinesV(sun, 18, CHALK);
    for (int k = 0; k < 8; k++)
      ChalkLine({sun.x + 24 * std::cos(k * PI / 4), sun.y + 24 * std::sin(k * PI / 4)},
                {sun.x + 34 * std::cos(k * PI / 4), sun.y + 34 * std::sin(k * PI / 4)},
                2.4f, CHALK, k);
    const float hx = x0 + 160;
    ChalkLine({hx, top + 120}, {hx + 60, top + 120}, 2.4f, CHALK, 1);
    ChalkLine({hx, top + 120}, {hx, top + 70}, 2.4f, CHALK, 2);
    ChalkLine({hx + 60, top + 120}, {hx + 60, top + 70}, 2.4f, CHALK, 3);
    ChalkLine({hx - 8, top + 72}, {hx + 30, top + 40}, 2.4f, CHALK, 4);
    ChalkLine({hx + 30, top + 40}, {hx + 68, top + 72}, 2.4f, CHALK, 5);
    for (int k = 0; k < 3; k++) { // stick children holding hands
      const float cx = x0 + 280 + k * 34.0f;
      DrawCircleLinesV({cx, top + 70}, 9, CHALK);
      ChalkLine({cx, top + 79}, {cx, top + 112}, 2.4f, CHALK, k);
      ChalkLine({cx, top + 112}, {cx - 9, top + 134}, 2.4f, CHALK, k + 1);
      ChalkLine({cx, top + 112}, {cx + 9, top + 134}, 2.4f, CHALK, k + 2);
      ChalkLine({cx - 17, top + 92}, {cx + 17, top + 92}, 2.4f, CHALK, k + 3);
    }
    DrawChildsElephant({x0 + 470, top + 90}, 44, CHALK, 3);
  } else {
    // fresh paint: a big pencil, a book, an apple, a smiling sun
    const float px = x0 + 60;
    const Vector2 pencil[4] = {{px, top + 150}, {px + 150, top + 70},
                               {px + 166, top + 96}, {px + 16, top + 176}};
    ComicPoly(pencil, 4, {250, 200, 50, 255}, 2.4f);
    const Vector2 lead[3] = {{px, top + 150}, {px + 16, top + 176},
                             {px - 22, top + 182}};
    ComicPoly(lead, 3, {250, 220, 180, 255}, 2);
    const Vector2 book[4] = {{x0 + 260, top + 60}, {x0 + 360, top + 70},
                             {x0 + 360, top + 160}, {x0 + 260, top + 150}};
    ComicPoly(book, 4, {236, 80, 80, 255}, 2.4f);
    const Vector2 page[4] = {{x0 + 360, top + 70}, {x0 + 450, top + 60},
                             {x0 + 450, top + 150}, {x0 + 360, top + 160}};
    ComicPoly(page, 4, COMIC_PAPER, 2.4f);
    for (int k = 0; k < 4; k++)
      InkLine({x0 + 372, top + 88 + k * 16.0f}, {x0 + 438, top + 82 + k * 16.0f},
              1.2f, Faded(COMIC_INK, 0.5f));
    ComicCircle({x0 + 520, top + 110}, 34, {226, 60, 50, 255}, 2.6f);
    DrawComicLeaf({x0 + 520, top + 78}, {x0 + 548, top + 64}, 14, LEAF, 1.6f);
    // the paint is so new it still shines
    for (int k = 0; k < 3; k++)
      DrawSparkle({x0 + 120 + k * 190.0f, top + 30 + (k % 2) * 120},
                  5 + 2 * std::sin(time * 3 + k), COMIC_PAPER);
    // the painter's bucket, left at the end of the wall
    ComicRect({x1 - 60, g - 34, 34, 34}, {220, 220, 228, 255}, 2);
    ComicRect({x1 - 60, g - 34, 34, 8}, NEW_BLUE, 1.4f);
    InkCurve({x1 - 60, g - 34}, {x1 - 43, g - 58}, {x1 - 26, g - 34}, 1.6f);
  }
}

// The tamarind tree by the school: huge, gnarled and exactly the same
void DrawTamarind(float x, float time) {
  const float g = STREET_GROUND;
  const Vector2 trunk[6] = {{x - 46, g + 6},  {x - 30, g - 160},
                            {x - 18, g - 300}, {x + 26, g - 300},
                            {x + 34, g - 160}, {x + 60, g + 6}};
  ComicPoly(trunk, 6, {120, 82, 54, 255}, 3);
  for (int k = 0; k < 6; k++) // the bark's grooves
    InkCurve({x - 30 + k * 12.0f, g}, {x - 22 + k * 9.0f + (k % 2) * 8, g - 140},
             {x - 18 + k * 8.0f, g - 290}, 1.4f, Faded(COMIC_INK, 0.6f));
  InkLine({x - 10, g - 250}, {x - 120, g - 360}, 16, {120, 82, 54, 255});
  InkLine({x + 10, g - 260}, {x + 140, g - 380}, 14, {120, 82, 54, 255});
  // the canopy: clumps of fine leaves, darker underneath
  const Vector2 CLUMPS[] = {{-200, -330}, {-110, -400}, {0, -430},  {120, -410},
                            {210, -340},  {-60, -350},  {80, -350}, {-260, -400},
                            {270, -420},  {0, -500},    {-160, -480}, {170, -490}};
  for (int pass = 0; pass < 2; pass++) {
    for (int i = 0; i < 12; i++) {
      const Vector2 p = {x + CLUMPS[i].x + 3 * std::sin(time * 0.8f + i),
                         g + CLUMPS[i].y};
      const float r = 70 + (i % 3) * 14;
      if (pass == 0)
        DrawCircleV(p, r + 3, COMIC_INK);
      else
        DrawCircleV(p, r, i % 2 ? LEAF : Color{80, 166, 80, 255});
    }
  }
  BeginTone(TONE_DOTS, 6); // shade, deeper underneath
  for (int i = 0; i < 12; i++) {
    const Vector2 p = {x + CLUMPS[i].x + 3 * std::sin(time * 0.8f + i) + 12,
                       g + CLUMPS[i].y + 16};
    DrawCircleGradient(p, 62 + (i % 3) * 14, {20, 70, 40, 60},
                       {20, 70, 40, 200});
  }
  EndTone();
  for (int k = 0; k < 9; k++) { // tamarind pods, hanging
    const float px = x - 230 + k * 55.0f, py = g - 290 - (k % 3) * 22;
    InkCurve({px, py}, {px + 8, py + 18}, {px + 2, py + 34}, 4.5f, COMIC_INK);
    InkCurve({px, py}, {px + 8, py + 18}, {px + 2, py + 34}, 2.6f,
             {150, 100, 60, 255});
  }
}

void DrawHopscotch(float x, float alpha) {
  const Color chalk = Faded(CHALK, alpha);
  const float y = 610, w = 46, h = 26;
  // 1, 2, 3-4, 5, 6-7, 8: squares chalked on the street
  const int ROWS[] = {1, 1, 2, 1, 2, 1};
  for (int r = 0; r < 6; r++) {
    const float rx = x + r * w;
    for (int k = 0; k < ROWS[r]; k++) {
      const float ry = ROWS[r] == 1 ? y + h / 2 : y + k * h;
      DrawRectangleLinesEx({rx, ry, w, h}, 2.4f, chalk);
    }
  }
  DrawCircleLinesV({x + 6 * w + 24, y + h}, 20, chalk);
}

void DrawGate(float x, bool stonesThere, float time) {
  const float g = STREET_GROUND;
  for (int side = 0; side < 2; side++) {
    const float px = x + side * 190;
    ComicRect({px, g - 240, 46, 240}, {246, 232, 204, 255}, 2.6f);
    ComicRect({px - 6, g - 256, 58, 18}, RED_OXIDE, 2.2f);
    ComicCircle({px + 23, g - 266}, 12, {240, 220, 180, 255}, 2);
  }
  // the green iron gate, swung open
  for (int k = 0; k < 7; k++)
    InkLine({x + 46 + k * 9.0f, g - 160 + k * 4.0f}, {x + 46 + k * 9.0f, g - 10},
            3, {40, 120, 80, 255});
  InkLine({x + 46, g - 160}, {x + 100, g - 136}, 3, {40, 120, 80, 255});
  InkLine({x + 46, g - 60}, {x + 100, g - 50}, 3, {40, 120, 80, 255});
  // the step where everyone sat at recess
  ComicRect({x + 30, g - 6, 190, 22}, {216, 210, 198, 255}, 2.4f);
  ComicRect({x + 16, g + 16, 218, 22}, {200, 194, 182, 255}, 2.4f);
  if (stonesThere) {
    const Vector2 STONES[] = {{-40, 26}, {-14, 22}, {10, 28}, {34, 24},
                              {-24, 32}};
    for (int k = 0; k < 5; k++)
      DrawPebble({STONES_X + (x - GATE_X) + STONES[k].x,
                  g + STONES[k].y - 2},
                 7, k);
    (void)time;
  }
}

// The whole street, seen from `camera` (the street x at the stage's left
// edge). `glow` is how much each remembered thing has come back.
void DrawOldStreet(float camera, float time, const float *glow,
                   bool stonesThere) {
  DrawStreetSky(STREET_GROUND - 140, false, time, camera);
  // the temple tower, far off, sliding by slowly
  DrawTempleTower(900 - camera * 0.12f, STREET_GROUND - 150, 0.75f, 0, time);
  DrawFarVillage(STREET_GROUND - 140, false, time, camera);
  DrawStreetGround(STREET_GROUND, camera, false);

  const float HOUSES[][3] = {// x, width, height
                             {-470, 230, 320}, {-220, 240, 340}, {40, 230, 310},
                             {290, 250, 330},  {2540, 240, 320}, {2800, 250, 340},
                             {3060, 230, 320}};
  for (int i = 0; i < 7; i++) {
    const float x = HOUSES[i][0] - camera;
    if (x > FullScreen().x + FullScreen().width || x + HOUSES[i][1] < FullScreen().x)
      continue;
    DrawStreetHouse(x, STREET_GROUND, HOUSES[i][1], HOUSES[i][2], i, time,
                    false);
  }
  const bool shop = Flicker(glow[SHOP], time, SHOP);
  DrawShop(SHOP_X - camera, time, shop);
  if (shop) { // two girls at the counter, choosing for ever
    DrawLittleAnu(SHOP_X - camera + 60, STREET_FEET - 40, time, 150);
    DrawFolk(LittleMeena(150), SHOP_X - camera + 130, STREET_FEET - 40, time,
             {0.2f, 0.62f}, -0.5f, true);
  }
  DrawSchoolWall(WALL_X - camera, WALL_END - camera, time,
                 Flicker(glow[WALL], time, WALL));
  DrawTamarind(TREE_X - camera, time);
  if (Flicker(glow[TREE], time, TREE)) { // jumping for the pods
    const float hop = 30 * std::fabs(std::sin(time * 5));
    DrawFolk(LittleMeena(160), TREE_X - camera - 120, STREET_FEET - 50 - hop,
             time, {0.12f, 1.1f}, 0.3f, true);
    DrawLittleAnu(TREE_X - camera + 110, STREET_FEET - 50, time, 160);
  }
  const bool hopscotch = Flicker(glow[HOPSCOTCH], time, HOPSCOTCH);
  DrawHopscotch(HOPSCOTCH_X - camera, hopscotch ? 1.0f : 0.3f);
  if (hopscotch) { // hopping, on one foot
    const float t = std::fmod(time * 0.8f, 1.0f);
    const float hop = 26 * std::fabs(std::sin(time * 6));
    DrawFolk(LittleMeena(150), HOPSCOTCH_X - camera + 20 + t * 260,
             STREET_FEET - 20 - hop, time, {0.3f, 0.75f}, 0.6f, true);
  }
  DrawGate(GATE_X - camera, stonesThere, time);
  DrawNewsprint();
}

// Anu, walking: a little bounce in each step, her arm swinging
void DrawAnuWalking(float x, float time, float walked, float facing) {
  const float step = walked / 30;
  const float bounce = walked > 0 ? 4 * std::fabs(std::sin(step)) : 0;
  DrawAnu(x, STREET_FEET - bounce, time,
          {0.13f + 0.03f * std::sin(step), 0.48f + 0.01f * std::cos(step)},
          {facing * 0.6f, 0.2f});
}

//------------------------------------------------------------------------------
// streets_walk: hold (or click) ahead and Anu walks there
//------------------------------------------------------------------------------
class StreetWalkScene : public Scene {
public:
  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    if (base < 0)
      base = colour;
    PlayAmbience("birds_day", 0.5f);

    if (leaving > 0) { // into the memory
      leaving += dt;
      done = leaving > 1.4f;
      return;
    }
    const Vector2 mouse = GetMousePosition();
    const float camera = Camera();
    if (atEnd) {
      const Vector2 stones = {STONES_X - camera, STREET_GROUND + 26};
      if (time - arrived > 0.6f &&
          (ContinuePressed() || Clicked({stones.x - 70, stones.y - 50, 140, 90}))) {
        leaving = 0.001f;
        PlayEffect("click", 0.6f, 1.3f);
      }
    } else {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        target = camera + mouse.x;
      if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_D))
        target = anuX + 60;
      if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        target = anuX - 60;
      target = std::fmax(-60, std::fmin(STREET_END, target));
      const float gap = target - anuX;
      if (std::fabs(gap) > 4) {
        // she slows down by the things she remembers
        float lingering = 0;
        for (int i = 0; i < REMEMBERED_COUNT; i++)
          lingering = std::fmax(lingering, glow[i]);
        const float move = std::fmin(std::fabs(gap),
                                     WALK_SPEED * (1 - 0.45f * lingering) * dt);
        anuX += gap > 0 ? move : -move;
        walked += move;
        facing = gap > 0 ? 1.0f : -1.0f;
      }
      if (anuX >= STREET_END - 8) {
        anuX = STREET_END;
        atEnd = true;
        arrived = time;
      }
    }
    // what she passes comes back for a moment, in colour
    float most = 0;
    for (int i = 0; i < REMEMBERED_COUNT; i++) {
      const bool near = std::fabs(anuX - REMEMBERED_AT[i]) < 280;
      glow[i] = Approach(glow[i], near ? 1.0f : 0.0f, near ? 3.0f : 2.2f);
      most = std::fmax(most, glow[i]);
    }
    colour = base + (1 - base) * 0.85f * most;
  }

  void Draw() override {
    const float camera = Camera();
    DrawOldStreet(camera, time, glow, true);
    DrawAnuWalking(anuX - camera, time, walked, facing);
    if (walked < 80) { // this way: chevrons pointing along the street
      const float a = 0.5f + 0.4f * std::sin(time * 4);
      for (int k = 0; k < 3; k++) {
        const float x = 470 + k * 40 + 8 * std::sin(time * 4);
        const Vector2 p[3] = {{x, 580}, {x + 26, 608}, {x, 636}};
        DrawLineEx(p[0], p[1], 12, Faded(COMIC_INK, a));
        DrawLineEx(p[1], p[2], 12, Faded(COMIC_INK, a));
        DrawCircleV(p[1], 6, Faded(COMIC_INK, a));
        DrawLineEx(p[0], p[1], 6, Faded(COMIC_PAPER, a));
        DrawLineEx(p[1], p[2], 6, Faded(COMIC_PAPER, a));
        DrawCircleV(p[1], 3, Faded(COMIC_PAPER, a));
      }
    }
    if (atEnd && leaving <= 0) {
      DrawHint({STONES_X - camera, STREET_GROUND + 22}, time);
      for (int k = 0; k < 3; k++)
        DrawSparkle({STONES_X - camera - 30 + k * 30,
                     STREET_GROUND + 6 - (k % 2) * 10},
                    4 + 2 * std::sin(time * 4 + k), COMIC_PAPER);
    }
    DrawCurtain(Between(leaving, 0.2f, 1.4f), {255, 236, 200, 255});
  }

private:
  const float WALK_SPEED = 175;
  float time = 0;
  float base = -1; // the world's colour today
  float anuX = 60, target = 60, walked = 0, facing = 1;
  float glow[REMEMBERED_COUNT] = {};
  bool atEnd = false;
  float arrived = 0, leaving = 0;

  float Camera() const {
    return std::fmax(CAMERA_MIN, std::fmin(CAMERA_MAX, anuX - 280));
  }
};

//------------------------------------------------------------------------------
// five_stones: kallangai at recess, under the tamarind tree, with Meena.
// The player is little Anu's hand. Tap to toss the stone in your hand; while
// it's up, tap stones on the ground to sweep them up; be back under it to
// catch it. Once all five are in hand, toss them all and catch them on the
// back of the hand. Dropping one is only funny.
//------------------------------------------------------------------------------
const float AIR_SECONDS = 2.4f;  // a slow, generous toss
const float CATCH_RADIUS = 115;  // how near the hand must be to catch it
const float PICK_RADIUS = 46;
const Vector2 MEENA_MOUTH = {500, 150};
const Vector2 LITTLE_ANU_MOUTH = {150, 640};

class FiveStonesScene : public Scene {
public:
  explicit FiveStonesScene(const Step &step) {
    four = LineAt(step.lines, 0, {"Meena", "Four!"});
    badStone = LineAt(step.lines, 1, {"Anu", "It was a bad stone."});
    const Vector2 START[4] = {{230, 520}, {330, 450}, {450, 580}, {520, 470}};
    for (int i = 0; i < 4; i++)
      ground.push_back({START[i], false, i + 1});
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.35f);
    meenaSays.Update();
    anuSays.Update();
    if (laughTimer > 0)
      laughTimer -= dt;
    if (replyIn > 0) {
      replyIn -= dt;
      if (replyIn <= 0)
        anuSays.Say(badStone, 3);
    }
    const Vector2 mouse = GetMousePosition();
    hand = Lerp2(hand, {std::fmax(70.0f, std::fmin(650.0f, mouse.x)),
                        std::fmax(360.0f, std::fmin(680.0f, mouse.y))},
                 std::fmin(1.0f, dt * 14));

    if (finale > 0) {
      UpdateFinale(dt);
      return;
    }
    const bool tap = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                     IsKeyPressed(KEY_SPACE);
    if (flying) {
      airTime += dt;
      if (tap)
        PickUnder(mouse);
      if (airTime >= AIR_SECONDS)
        Land();
    } else if (tap) {
      if (held == 5) { // everything in hand: the big toss
        finale = 0.001f;
        PlayEffect("click", 0.5f, 1.4f);
      } else if (held == 0 || !PickUnder(mouse)) {
        if (held > 0) { // toss
          flying = true;
          airTime = 0;
          hand = {std::fmax(70.0f, std::fmin(650.0f, mouse.x)),
                  std::fmax(360.0f, std::fmin(680.0f, mouse.y))};
          landing = hand;
          held--;
          PlayEffect("click", 0.4f, 1.6f);
        }
      }
    }
  }

  void Draw() override {
    DrawRecess();
    // the stones on the ground
    for (const Stone &s : ground) {
      if (s.picked)
        continue;
      DrawPebble(s.at, 13, s.seed);
      if (flying || held == 0)
        DrawRing(s.at, 22, 25, 0, 360, 24,
                 Faded(COMIC_PAPER, 0.35f + 0.3f * std::sin(time * 5 + s.seed)));
    }
    // the tossed stone's shadow, growing as it falls
    if (flying) {
      const float t = airTime / AIR_SECONDS, up = 4 * t * (1 - t);
      BeginTone(TONE_DOTS, 4);
      DrawEllipseV(landing, 26 - 14 * up, 10 - 5 * up, {40, 20, 10, 150});
      EndTone();
      DrawRing(landing, CATCH_RADIUS * 0.5f, CATCH_RADIUS * 0.5f + 3, 0, 360,
               40, Faded(COMIC_PAPER, 0.25f + 0.5f * t));
    }
    DrawHand();
    if (flying) { // the stone up in the air, bigger as it comes near
      const float t = airTime / AIR_SECONDS, up = 4 * t * (1 - t);
      DrawPebble({landing.x, landing.y - 20 - 330 * up}, 13 * (1 + 0.5f * up),
                 0);
    }
    if (finale > 0)
      DrawFinaleStones();
    if (!flying && finale <= 0 && (held == 5 || tosses == 0))
      DrawHint(hand, time);
    meenaSays.Draw(MEENA_MOUTH, time);
    anuSays.Draw(LITTLE_ANU_MOUTH, time);
    if (finale > FINALE_SECONDS)
      DrawGoOn(time);
    if (time < 1)
      DrawCurtain(1 - time, {255, 236, 200, 255});
  }

private:
  struct Stone {
    Vector2 at;
    bool picked;
    int seed;
  };
  std::vector<Stone> ground;
  Line four, badStone;
  Quip meenaSays, anuSays;
  float time = 0, airTime = 0, laughTimer = 0, replyIn = 0, finale = 0;
  Vector2 hand = {360, 600}, landing = {};
  int held = 1, tosses = 0;
  bool flying = false, dropped = false, saidIt = false;
  const float FINALE_SECONDS = 3.2f;

  bool PickUnder(Vector2 mouse) {
    for (Stone &s : ground) {
      const float dx = s.at.x - mouse.x, dy = s.at.y - mouse.y;
      if (!s.picked && dx * dx + dy * dy < PICK_RADIUS * PICK_RADIUS) {
        s.picked = true;
        held++;
        hand = s.at; // the hand sweeps it up
        PlayEffect("click", 0.5f, 1.1f + 0.1f * held);
        return true;
      }
    }
    return false;
  }

  void Land() {
    flying = false;
    tosses++;
    const float dx = hand.x - landing.x, dy = hand.y - landing.y;
    if (dx * dx + dy * dy < CATCH_RADIUS * CATCH_RADIUS) {
      held++;
      PlayEffect("click", 0.6f, 1.0f);
      return;
    }
    // missed: it drops, and Meena thinks that's the funniest thing
    ground.push_back({{landing.x, std::fmin(660.0f, landing.y + 10)}, false, 0});
    PlayEffect("click", 0.7f, 0.6f);
    dropped = true;
    laughTimer = 2;
    SayFour();
  }

  void SayFour() {
    if (saidIt)
      return;
    saidIt = true;
    meenaSays.Say(four, 2.6f);
    replyIn = 1.6f;
  }

  void UpdateFinale(float dt) {
    const float before = finale;
    finale += dt;
    // up they go, and down onto the back of the hand
    if (before < 1.4f && finale >= 1.4f) {
      PlayEffect("click", 0.6f, 0.9f);
      if (!dropped) { // one always rolls off. It was a bad stone.
        PlayEffect("click", 0.6f, 0.6f);
        laughTimer = 2.5f;
        SayFour();
      } else {
        laughTimer = 2.5f; // all five! Meena claps
      }
    }
    if (finale > FINALE_SECONDS && !meenaSays.Showing() &&
        !anuSays.Showing() && replyIn <= 0 && ContinuePressed())
      done = true;
  }

  void DrawFinaleStones() const {
    const float t = std::fmin(finale / 1.4f, 1.0f), up = 4 * t * (1 - t);
    const int onHand = dropped ? 5 : 4;
    for (int i = 0; i < 5; i++) {
      Vector2 p = {hand.x - 26 + i * 13.0f, hand.y - 34 - 300 * up -
                                                (i % 2) * 6 * (1 - t)};
      if (t >= 1 && i >= onHand) { // the bad stone rolls away
        const float roll = std::fmin(finale - 1.4f, 1.0f);
        p = {hand.x + 40 + roll * 110, hand.y - 20 + roll * 50 - 30 * std::sin(roll * PI)};
      }
      DrawPebble(p, 11, i);
    }
  }

  // Recess under the tamarind tree: the old ochre school wall, the trunk,
  // Meena sitting cross-legged across the game
  void DrawRecess() const {
    const Rectangle all = FullScreen();
    DrawBand(0, 300, OLD_OCHRE);
    DrawToneRect({all.x, 0, all.width, 300}, {150, 80, 30, 255}, 0.6f, 0.1f,
                 TONE_DOTS, 6);
    DrawBand(250, 14, {200, 140, 70, 255});
    DrawChildsElephant({120, 120}, 40, CHALK, 3);
    for (int k = 0; k < 3; k++)
      DrawCircleLinesV({600 + k * 50.0f, 100}, 12, CHALK);
    // the tamarind's trunk, and its shade over everything
    const Vector2 trunk[4] = {{all.x + 60, 330}, {all.x + 90, 0},
                              {all.x + 230, 0}, {all.x + 250, 330}};
    ComicPoly(trunk, 4, {120, 82, 54, 255}, 3);
    DrawBandGradient(264, 456, {206, 160, 112, 255}, {184, 140, 96, 255});
    DrawBand(263, 3, COMIC_INK);
    BeginTone(TONE_DOTS, 6);
    for (int i = 0; i < 9; i++) // dappled shade
      DrawEllipse((int)(all.x + std::fmod(i * 233.0f, all.width)),
                  (int)(300 + std::fmod(i * 137.0f, 400)), 120, 40,
                  {60, 40, 20, 90});
    EndTone();
    for (int i = 0; i < 26; i++) { // little tamarind leaves fallen
      const float x = all.x + std::fmod(i * 149.0f, all.width);
      const float y = 290 + std::fmod(i * 71.0f, 420);
      DrawEllipse((int)x, (int)y, 4, 2, {150, 170, 60, 255});
    }
    // the game's chalk circle
    DrawEllipseLines(380, 520, 250, 110, CHALK);
    DrawEllipseLines(380, 520, 248, 108, CHALK);
    // Meena, cross-legged, her pavadai spread round her
    const bool laughing = laughTimer > 0;
    DrawFolk(LittleMeena(210), 500, 392, time,
             laughing ? Vector2{0.05f, 0.62f} : Vector2{0.18f, 0.36f},
             -0.2f, laughing || finale > 1.6f);
    ComicEllipse({500, 356}, 108, 40, {236, 128, 48, 255}, 2.4f);
    DrawEllipse(500, 376, 100, 16, {24, 128, 130, 255});
    for (int i = -1; i <= 1; i++) // the folds of her pavadai
      InkCurve({500 + i * 30.0f, 322}, {500 + i * 44.0f, 350},
               {500 + i * 60.0f, 380}, 1.2f, Faded(COMIC_INK, 0.4f));
    for (int side = -1; side <= 1; side += 2) // her toes peeking out
      ComicEllipse({500 + side * 26.0f, 392}, 12, 6, SKIN_DARK, 1.4f);
    DrawNewsprint();
  }

  // Little Anu's hand, reaching in from the bottom: glass bangles, the
  // stones she's holding. In the finale it turns over.
  void DrawHand() const {
    const Vector2 elbow = {hand.x * 0.4f + 120, 780};
    const float w = 40;
    DrawLineEx(elbow, hand, w + 6, PERSON_INK);
    DrawLineEx(elbow, hand, w, SKIN);
    DrawLineEx({elbow.x + 6, elbow.y}, {hand.x + 8, hand.y + 30}, 8,
               Faded(SKIN_DARK, 0.6f));
    // glass bangles at the wrist
    const Vector2 d = {hand.x - elbow.x, hand.y - elbow.y};
    const float len = std::sqrt(d.x * d.x + d.y * d.y);
    const Vector2 u = {d.x / len, d.y / len};
    const Color BANGLES[] = {{220, 40, 60, 255}, {40, 160, 80, 255},
                             {240, 190, 40, 255}};
    for (int k = 0; k < 3; k++) {
      const Vector2 c = {hand.x - u.x * (52 + k * 8), hand.y - u.y * (52 + k * 8)};
      DrawEllipseV(c, 26, 9, PERSON_INK);
      DrawEllipseV(c, 24, 6.5f, BANGLES[k]);
    }
    const bool back = finale > 0.5f;
    // the palm (or the back of the hand), the fingers
    for (int k = 0; k < 4; k++) {
      const Vector2 f = {hand.x - 22 + k * 15.0f, hand.y - 30 + std::fabs(k - 1.5f) * 4};
      DrawEllipseV(f, 9, 15, PERSON_INK);
      DrawEllipseV(f, 6.8f, 12.8f, back ? SKIN_DARK : SKIN);
      if (back)
        DrawEllipseV({f.x, f.y - 7}, 3.6f, 4.4f, {220, 170, 140, 255});
    }
    DrawEllipseV({hand.x + 30, hand.y - 2}, 10, 17, PERSON_INK); // thumb
    DrawEllipseV({hand.x + 30, hand.y - 2}, 7.6f, 14.6f, SKIN);
    DrawEllipseV(hand, 32, 24, PERSON_INK);
    DrawEllipseV(hand, 29.5f, 21.5f, back ? SKIN_DARK : SKIN);
    // the stones she's holding, peeking out
    if (finale <= 0)
      for (int i = 0; i < held; i++)
        DrawPebble({hand.x - 14 + (i % 3) * 13.0f, hand.y - 4 + (i / 3) * 10.0f},
                   8, i);
  }
};

//------------------------------------------------------------------------------
// streets_meena: Meena now, a boy on her hip and a bag of vegetables. Before
// anyone speaks, the boy stares at Anu: tap him and he hides, giggling.
//------------------------------------------------------------------------------
const float MEET_CAMERA = 1990;
const float MEENA_STOPS = 470, ANU_STANDS = 210;

void DrawMeenaWithBoy(float x, float time, float walked, bool boyLaughing,
                      bool waving, Vector2 hand) {
  const float bounce = walked > 0 ? 3 * std::fabs(std::sin(walked / 26)) : 0;
  const float feet = STREET_FEET - bounce;
  const float H = 278;
  // the yellow cloth bag, greens and drumsticks poking out of it
  const Vector2 bagTop = {x - 0.15f * H, feet - 0.47f * H};
  InkLine({bagTop.x - 4, bagTop.y + 22}, {bagTop.x + 2, bagTop.y - 4}, 3);
  for (int k = 0; k < 3; k++)
    InkLine({bagTop.x - 20 + k * 10.0f, bagTop.y + 26},
            {bagTop.x - 30 + k * 16.0f, bagTop.y - 10 - k * 6}, 4,
            k == 1 ? Color{120, 150, 60, 255} : LEAF);
  ComicRect({bagTop.x - 32, bagTop.y + 20, 56, 62}, {246, 210, 60, 255}, 2.2f);
  DrawMeena(x, feet, time, hand, {-0.5f, 0.3f});
  // her boy on her hip
  const Folk boy = MeenasBoy(140);
  DrawFolk(boy, x + 0.17f * H, feet - 0.37f * H, time,
           waving ? Vector2{0.3f, 1.0f + 0.08f * std::sin(time * 12)}
                  : Vector2{0.18f, 0.55f},
           -0.8f, boyLaughing);
  if (boyLaughing) // hiding his face in her shoulder
    DrawCircleV({x + 0.14f * H, feet - 0.66f * H}, 14, SKIN);
  DrawCircleV({x + 0.2f * H, feet - 0.4f * H}, 9, PERSON_INK); // her hand
  DrawCircleV({x + 0.2f * H, feet - 0.4f * H}, 7.4f, SKIN);
}

// The page of Meena's maths book, with little Anu's elephant in the margin
void DrawMathsBook(float slide, float time) {
  if (slide <= 0)
    return;
  const Rectangle r = {390 + (1 - slide) * 400, 40, 300, 210};
  const float tilt = 0;
  (void)tilt;
  ComicRect({r.x - 10, r.y - 10, r.width + 20, r.height + 20}, {60, 110, 170, 255},
            3);
  DrawRectangleRec(r, {250, 248, 238, 255});
  for (float x = r.x + 12; x < r.x + r.width; x += 12)
    DrawLineEx({x, r.y}, {x, r.y + r.height}, 1, {190, 210, 230, 255});
  for (float y = r.y + 12; y < r.y + r.height; y += 12)
    DrawLineEx({r.x, y}, {r.x + r.width, y}, 1, {190, 210, 230, 255});
  const Color pen = {40, 52, 110, 255};
  DrawString("12 + 7 = 19", r.x + 16, r.y + 14, 22, pen, HAND_FONT);
  DrawString("36 - 9 = 27", r.x + 16, r.y + 50, 22, pen, HAND_FONT);
  DrawString("8 x 3 = 24", r.x + 16, r.y + 86, 22, pen, HAND_FONT);
  DrawChildsElephant({r.x + 190, r.y + 140}, 34 + std::sin(time * 2), {150, 60, 160, 255},
                     3);
  DrawSparkle({r.x + 270, r.y + 30}, 6 + 2 * std::sin(time * 3), {250, 200, 60, 255});
}

class MeetMeenaScene : public Scene {
public:
  explicit MeetMeenaScene(const Step &step) : lines(step.lines) {
    if (lines.empty())
      lines = {{"Meena", "Do you still draw?"}};
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.5f);
    if (boyLaughs > 0)
      boyLaughs -= dt;
    switch (stage) {
    case ARRIVING:
      walked += 160 * dt;
      meenaX = std::fmax(MEENA_STOPS, 840 - walked);
      if (meenaX <= MEENA_STOPS) {
        stage = STARING;
        stageTime = 0;
      }
      break;
    case STARING: // the boy looks at Anu. A tap and he hides, giggling.
      stageTime += dt;
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
          CheckCollisionPointRec(GetMousePosition(), BoyArea())) {
        boyLaughs = 1.6f;
        tapped = true;
        PlayEffect("click", 0.4f, 1.8f);
      } else if (stageTime > (tapped ? 2.4f : 4.5f) ||
                 (stageTime > 1 && !tapped && ContinuePressed())) {
        stage = TALKING;
        talk.Start(lines);
      }
      break;
    case TALKING:
      if (talk.Update()) {
        stage = LEAVING;
        stageTime = 0;
      }
      break;
    case LEAVING:
      stageTime += dt;
      if (stageTime > 1.2f)
        meenaX += 150 * dt;
      done = stageTime > 5.5f;
      break;
    }
  }

  void Draw() override {
    const float glow[REMEMBERED_COUNT] = {};
    DrawOldStreet(MEET_CAMERA, time, glow, false);
    const float facing = stage == LEAVING && stageTime > 2.5f ? 0.8f : 0.6f;
    DrawAnu(ANU_STANDS, STREET_FEET, time, {0.13f, 0.5f}, {facing, 0.2f});
    // Meena points at the temple when she says "Come for the festival"
    const bool pointing = stage == TALKING && talk.current + 1 == lines.size();
    const bool walking = stage == ARRIVING || (stage == LEAVING && stageTime > 1.2f);
    DrawMeenaWithBoy(meenaX, time, walking ? time * 160 : 0,
                     boyLaughs > 0, stage == LEAVING,
                     pointing ? Vector2{0.3f, 0.98f} : Vector2{0.12f, 0.45f});
    if (stage == STARING && !tapped && stageTime > 0.8f) {
      const Rectangle b = BoyArea();
      DrawHint({b.x + b.width / 2, b.y + b.height / 2}, time);
    }
    // the elephant in the maths book, while they talk about it
    const bool elephant = stage == TALKING && talk.current >= 2 &&
                          talk.current + 1 < lines.size();
    elephantSlide = Approach(elephantSlide, elephant ? 1.0f : 0.0f, 6);
    DrawMathsBook(elephantSlide, time);
    talk.Draw({ANU_STANDS + 6, STREET_FEET - 252},
              {meenaX + 2, STREET_FEET - 240}, time);
  }

private:
  enum Stage { ARRIVING, STARING, TALKING, LEAVING } stage = ARRIVING;
  std::vector<Line> lines;
  Balloons talk;
  float time = 0, stageTime = 0, walked = 0, meenaX = 840;
  float boyLaughs = 0, elephantSlide = 0;
  bool tapped = false;

  Rectangle BoyArea() const {
    return {meenaX + 0.17f * 278 - 50, STREET_FEET - 0.37f * 278 - 130, 100,
            140};
  }
};

//------------------------------------------------------------------------------
// paper_boats: rain, and the street turned into a river. Fold a boat from a
// notebook page (three drags), draw it a face (a tap), then steer it down
// the street past the stones. Meena's boat bobs along beside it.
//------------------------------------------------------------------------------
const Vector2 PAPER_CENTRE = {330, 560};
const float FLOAT_SECONDS = 20;
const float RIVER_TOP = 452, RIVER_BOTTOM = 700;

// Folding: each stage wants a drag one way
const Vector2 FOLD_WAYS[3] = {{0, 1}, {0.3f, 1}, {1, 0}};

class PaperBoatsScene : public Scene {
public:
  explicit PaperBoatsScene(const Step &step) {
    lines = step.lines;
    if (lines.empty())
      lines = {{"Meena", "Yours has a face on it."},
               {"Anu", "So it can see where it's going."}};
    for (int i = 0; i < 14; i++) // stones and bricks in the water
      rocks.push_back({500.0f + i * 290 + 60 * std::sin(i * 2.7f),
                       RIVER_TOP + 50 + std::fmod(i * 97.0f, 170.0f), i});
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("wind", 0.4f); // the rain (there's no rain sound: "pour" is
                                // coffee being poured, so not that)
    if (stage == FOLDING)
      UpdateFolding(dt);
    else if (stage == FACE)
      UpdateFace(dt);
    else if (stage == TALK) {
      if (talk.Update()) {
        stage = FLOATING;
        stageTime = 0;
      }
    } else
      UpdateFloating(dt);
  }

  void Draw() override {
    if (stage == FLOATING) {
      DrawRiver();
    } else {
      DrawThinnai();
      DrawPaper(PAPER_CENTRE, 1, FoldShown(), faceDrawn);
      if (stage == FOLDING && !dragging && foldAnim <= 0) {
        // which way to drag: an arrow along the fold
        const Vector2 way = FOLD_WAYS[fold];
        const float n = std::sqrt(way.x * way.x + way.y * way.y);
        const Vector2 u = {way.x / n, way.y / n};
        const Vector2 from = {PAPER_CENTRE.x - u.x * 80, PAPER_CENTRE.y - 70 - u.y * 30};
        const float slide = std::fmod(time, 1.4f) / 1.4f;
        const Vector2 p = {from.x + u.x * 120 * slide, from.y + u.y * 120 * slide};
        DrawHint(p, time, 1 - slide * 0.6f);
      }
      if (stage == FACE && faceDrawn <= 0)
        DrawHint({PAPER_CENTRE.x, PAPER_CENTRE.y + 40}, time);
      talk.Draw({170, 400}, {560, 360}, time);
    }
    if (time < 1)
      DrawCurtain(1 - time, {255, 236, 200, 255});
    if (stage == FLOATING && stageTime > FLOAT_SECONDS)
      DrawGoOn(time);
  }

private:
  enum Stage { FOLDING, FACE, TALK, FLOATING } stage = FOLDING;
  std::vector<Line> lines;
  Balloons talk;
  float time = 0, stageTime = 0;
  int fold = 0;            // which fold is being made (0..2), 3 = a boat
  float foldProgress = 0;  // how far the current fold has been dragged
  float foldAnim = 0;      // finishing a fold by itself, after a drag
  bool dragging = false;
  Vector2 dragFrom = {};
  float faceDrawn = 0; // 0..1, the face being drawn on
  // floating
  struct Rock {
    float x, y;
    int seed;
  };
  std::vector<Rock> rocks;
  float scroll = 0;
  Vector2 boat = {260, 560};
  float spin = 0, spinSpeed = 0, bump = 0;
  float splashAt = -10;
  Vector2 splash = {};

  // How folded the paper is: 0..3, with fractions mid-fold
  float FoldShown() const {
    return std::fmin(3.0f, fold + std::fmax(foldProgress, foldAnim));
  }

  void UpdateFolding(float dt) {
    const Vector2 mouse = GetMousePosition();
    if (foldAnim > 0) { // the fold finishing itself
      foldAnim += dt * 3.5f;
      if (foldAnim >= 1) {
        foldAnim = 0;
        foldProgress = 0;
        fold++;
        PlayEffect("page", 0.6f, 1.2f);
        if (fold == 3) {
          stage = FACE;
          stageTime = 0;
        }
      }
      return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      dragging = true;
      dragFrom = mouse;
    }
    if (dragging) {
      const Vector2 way = FOLD_WAYS[fold];
      const float n = std::sqrt(way.x * way.x + way.y * way.y);
      float along = ((mouse.x - dragFrom.x) * way.x + (mouse.y - dragFrom.y) * way.y) / n;
      if (fold == 2) // opening it out: either way across
        along = std::fabs(mouse.x - dragFrom.x);
      foldProgress = std::fmax(0, std::fmin(1, along / 160));
      if (foldProgress > 0.02f)
        PlayLoop("page", 0.3f);
      if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        dragging = false;
        if (foldProgress > 0.22f)
          foldAnim = foldProgress; // generous: it finishes the fold itself
        else
          foldProgress = 0;
      }
    }
    if (IsKeyPressed(KEY_SPACE) && !dragging) // keys: one press, one fold
      foldAnim = 0.01f;
  }

  void UpdateFace(float dt) {
    stageTime += dt;
    if (faceDrawn <= 0 && stageTime > 0.3f &&
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE)))
      faceDrawn = 0.01f;
    if (faceDrawn > 0) {
      faceDrawn = std::fmin(1, faceDrawn + dt * 1.2f);
      if (faceDrawn < 1)
        PlayLoop("pencil", 0.6f);
      else if (!talk.active) {
        stage = TALK;
        talk.Start(lines);
      }
    }
  }

  void UpdateFloating(float dt) {
    stageTime += dt;
    const float flow = 110 + 30 * Between(stageTime, 0, 3);
    scroll += flow * dt;
    // steer: the boat drifts towards the pointer
    const Vector2 mouse = GetMousePosition();
    Vector2 want = {std::fmax(140.0f, std::fmin(560.0f, mouse.x)),
                    std::fmax(RIVER_TOP + 40, std::fmin(RIVER_BOTTOM - 20, mouse.y))};
    if (IsKeyDown(KEY_UP))
      want.y = boat.y - 60;
    if (IsKeyDown(KEY_DOWN))
      want.y = boat.y + 60;
    boat.x += (want.x - boat.x) * std::fmin(1.0f, dt * 2.2f);
    boat.y += (want.y - boat.y) * std::fmin(1.0f, dt * 2.6f);
    boat.y = std::fmax(RIVER_TOP + 40, std::fmin(RIVER_BOTTOM - 20, boat.y));
    // bumping a stone only spins it round a bit
    bump = std::fmax(0, bump - dt);
    for (const Rock &r : rocks) {
      const float dx = boat.x - (r.x - scroll), dy = boat.y - r.y;
      const float d2 = dx * dx + dy * dy;
      if (d2 < 56 * 56) {
        const float d = std::fmax(1, std::sqrt(d2));
        boat.x += dx / d * (56 - d);
        boat.y += dy / d * (56 - d);
        if (bump <= 0) {
          spinSpeed += dy > 0 ? 5 : -5;
          bump = 0.6f;
          splash = boat;
          splashAt = time;
          PlayEffect("click", 0.5f, 0.6f);
        }
      }
    }
    spin += spinSpeed * dt;
    spinSpeed *= 1 - std::fmin(1.0f, dt * 2);
    spin *= 1 - std::fmin(1.0f, dt * 1.5f);
    if (stageTime > FLOAT_SECONDS && ContinuePressed())
      done = true;
  }

  // The street from the thinnai, in the rain: the houses across the street
  // and the water running between
  void DrawRainyStreet(float camera, float riverTop, float houses) const {
    const Rectangle all = FullScreen();
    DrawBandGradient(0, riverTop, {120, 140, 170, 255}, {170, 180, 190, 255});
    BeginTone(TONE_LINES, 5);
    DrawBand(0, riverTop, Faded({60, 70, 110, 255}, 0.35f));
    EndTone();
    // Each house has a whole-number place along the street, so its look
    // never changes (working it out from x flickered: a hair either side of
    // a whole number gave a different house every other frame)
    const float spacing = 280;
    const int base = (int)std::floor(camera / spacing);
    const float shift = camera - base * spacing;
    const int first = (int)std::floor(all.x / spacing) - 1;
    for (int k = first; k * spacing - shift < all.x + all.width; k++) {
      const int place = base + k;
      DrawStreetHouse(k * spacing - shift + 20, riverTop, 240, houses,
                      (place % 6 + 6) % 6, time, false);
    }
    DrawBandGradient(riverTop, 720 - riverTop, {150, 116, 80, 255},
                     {120, 90, 66, 255});
    DrawBand(riverTop - 1, 3, COMIC_INK);
    DrawToneRect({all.x, riverTop, all.width, 720 - riverTop}, {60, 40, 30, 255},
                 0, 0.5f, TONE_LINES, 6);
    for (int i = 0; i < 22; i++) { // ripples flowing by
      const float x = all.x + std::fmod(i * 131.0f - camera * 1.3f + 100000,
                                        all.width + 60) - 30;
      const float y = riverTop + 16 + std::fmod(i * 53.0f, 720 - riverTop - 20);
      InkCurve({x, y}, {x + 18, y - 5}, {x + 36, y}, 1.6f, {214, 190, 150, 255});
    }
    for (int i = 0; i < 10; i++) { // rain rings on the water
      const float age = std::fmod(time * 1.3f + i * 0.37f, 1.0f);
      const float x = all.x + std::fmod(i * 211.0f + std::floor(time * 1.3f + i * 0.37f) * 97, all.width);
      const float y = riverTop + 20 + std::fmod(i * 61.0f, 720 - riverTop - 30);
      DrawEllipseLines((int)x, (int)y, 6 + age * 22, 2 + age * 7,
                       Faded({230, 220, 200, 255}, 1 - age));
    }
  }

  // Before: on the thinnai, out of the rain, with the notebook
  void DrawThinnai() const {
    DrawRainyStreet(time * 4, 270, 200);
    DrawRain(time, 0.9f);
    const Rectangle all = FullScreen();
    // the eaves above, dripping
    DrawBand(0, 40, {214, 92, 52, 255});
    DrawBand(40, 10, TEAK_DARK);
    DrawBand(49, 3, COMIC_INK);
    for (int i = 0; i < 12; i++) {
      const float x = all.x + 40 + i * 110.0f;
      const float y = 56 + std::fmod(time * 220 + i * 37, 260.0f);
      DrawCircleV({x, y}, 3, {220, 236, 255, 220});
    }
    // the red-oxide thinnai we're sitting on
    DrawBandGradient(400, 320, {200, 64, 46, 255}, {170, 50, 38, 255});
    DrawBand(398, 4, COMIC_INK);
    DrawToneRect({all.x, 400, all.width, 320}, COMIC_INK, 0.05f, 0.4f,
                 TONE_DOTS, 6);
    DrawLineEx({all.x, 404}, {all.x + all.width, 404}, 3, {240, 120, 90, 255});
    // Meena, her plain boat done already, waiting
    DrawFolk(LittleMeena(200), 590, 560, time, {-0.02f, 0.36f}, -0.6f,
             stage == TALK && talk.current == 0);
    DrawBoat({590, 498}, 0.3f, 0.05f * std::sin(time * 2), false, 1);
    // little Anu, beside the paper
    DrawLittleAnu(110, 560, time, 200);
    DrawNewsprint();
  }

  // The paper, folding: a page of a ruled notebook -> folded in half -> a
  // hat -> a boat. `face`: how much of its face is drawn.
  void DrawPaper(Vector2 c, float s, float shown, float face) const {
    const Color paper = {250, 248, 238, 255}, back = {226, 222, 210, 255};
    const Color rule = {170, 196, 220, 255};
    const float w = 150 * s, h = 100 * s;
    if (shown < 1) { // folding the top half down over the bottom half
      const float p = shown;
      ComicRect({c.x - w, c.y, 2 * w, h}, paper, 2.4f);
      for (int k = 1; k < 5; k++)
        DrawLineEx({c.x - w + 2, c.y + k * h / 5}, {c.x + w - 2, c.y + k * h / 5},
                   1.2f, rule);
      const float flap = -h * std::cos(p * PI); // above the crease, then below
      const Rectangle r = flap < 0 ? Rectangle{c.x - w, c.y + flap, 2 * w, -flap}
                                   : Rectangle{c.x - w, c.y, 2 * w, flap};
      ComicRect(r, flap < 0 ? paper : back, 2.4f);
      if (flap < 0) {
        for (int k = 1; k < 5; k++)
          if (k * h / 5 < -flap)
            DrawLineEx({c.x - w + 2, c.y - k * h / 5}, {c.x + w - 2, c.y - k * h / 5},
                       1.2f, rule);
        DrawLineEx({c.x - w + 30, c.y + flap}, {c.x - w + 30, c.y}, 1.4f,
                   {230, 140, 140, 255}); // the margin
      }
      return;
    }
    if (shown < 2) { // the top corners fold down to the middle: a hat
      const float p = shown - 1;
      const Vector2 rect[7] = {{-w, h},   {-w, 0}, {-w / 2, 0}, {0, 0},
                               {w / 2, 0}, {w, 0},  {w, h}};
      const Vector2 hat[7] = {{-w, h},          {-w, h * 0.45f},
                              {-w / 2, -h * 0.3f}, {0, -h * 1.05f},
                              {w / 2, -h * 0.3f},  {w, h * 0.45f}, {w, h}};
      Vector2 pts[7];
      for (int i = 0; i < 7; i++)
        pts[i] = {c.x + hat[i].x * p + rect[i].x * (1 - p),
                  c.y + hat[i].y * p + rect[i].y * (1 - p)};
      ComicPoly(pts, 7, back, 2.4f);
      DrawLineEx({c.x - w, c.y + h * 0.45f * p + h * (1 - p) * 0.0f},
                 {c.x + w, c.y + h * 0.45f * p}, 2, Faded(COMIC_INK, 0.5f * p));
      InkLine({c.x, c.y - h * 1.05f * p}, {c.x - w * 0.9f, c.y + h * 0.42f}, 1.4f,
              Faded(COMIC_INK, 0.5f * p));
      InkLine({c.x, c.y - h * 1.05f * p}, {c.x + w * 0.9f, c.y + h * 0.42f}, 1.4f,
              Faded(COMIC_INK, 0.5f * p));
      return;
    }
    if (shown < 3) { // pulled open: the hat squashes, and pops into a boat
      const float p = shown - 2;
      if (p < 0.55f) {
        const float q = p / 0.55f;
        const float sx = 1 - 0.45f * q, sy = 1 - 0.3f * q;
        const Vector2 hat[5] = {{c.x - w * sx, c.y + h * sy},
                                {c.x - w * sx, c.y + h * 0.45f * sy},
                                {c.x, c.y - h * 1.05f * sy},
                                {c.x + w * sx, c.y + h * 0.45f * sy},
                                {c.x + w * sx, c.y + h * sy}};
        ComicPoly(hat, 5, back, 2.4f);
        return;
      }
      const float q = (p - 0.55f) / 0.45f;
      DrawBoat({c.x, c.y + 40}, s * (0.6f + 0.4f * q + 0.12f * std::sin(q * PI)),
               0, false, face);
      return;
    }
    DrawBoat({c.x, c.y + 40}, s, 0.04f * std::sin(time * 2), face > 0, face);
  }

  // A paper boat: hull and sail; with a face on the hull, drawn as far as
  // `face` (0..1)
  static void DrawBoat(Vector2 c, float s, float tilt, bool withFace, float face) {
    const float ct = std::cos(tilt), st = std::sin(tilt);
    auto P = [&](float x, float y) {
      return Vector2{c.x + (x * ct - y * st) * s, c.y + (x * st + y * ct) * s};
    };
    const Color paper = {250, 248, 238, 255}, shade = {214, 210, 198, 255};
    const Vector2 sail[3] = {P(-70, 0), P(0, -120), P(70, 0)};
    ComicPoly(sail, 3, shade, 2.4f);
    DrawLineEx(P(0, -118), P(0, -4), 1.6f, Faded(COMIC_INK, 0.5f));
    const Vector2 hull[4] = {P(-180, -20), P(180, -20), P(120, 60), P(-120, 60)};
    ComicPoly(hull, 4, paper, 2.6f);
    for (int k = 1; k < 3; k++)
      DrawLineEx(P(-150 + k * 10, -20 + k * 26), P(150 - k * 10, -20 + k * 26),
                 1.2f, {170, 196, 220, 255});
    if (!withFace && face <= 0)
      return;
    // so it can see where it's going: two eyes and a smile, in pencil
    const Color pencil = {54, 54, 64, 255};
    const float w = std::fmax(1.5f, 4 * s);
    if (face > 0.1f)
      DrawCircleV(P(-40, 10), 9 * s * Between(face, 0.1f, 0.3f), pencil);
    if (face > 0.3f)
      DrawCircleV(P(40, 10), 9 * s * Between(face, 0.3f, 0.5f), pencil);
    if (face > 0.5f) {
      const float t = Between(face, 0.5f, 1);
      Vector2 prev = P(-40, 30);
      for (int i = 1; i <= (int)(10 * t); i++) {
        const float u = i / 10.0f;
        const Vector2 p = P(-40 + 80 * u, 30 + 16 * std::sin(u * PI));
        DrawLineEx(prev, p, w, pencil);
        prev = p;
      }
    }
  }

  void DrawRiver() {
    DrawRainyStreet(scroll, RIVER_TOP, 300);
    for (const Rock &r : rocks) {
      const float x = r.x - scroll;
      if (x < FullScreen().x - 60 || x > FullScreen().x + FullScreen().width + 60)
        continue;
      if (r.seed % 4 == 3) { // a brick, or someone's lost slipper
        ComicRect({x - 26, r.y - 12, 52, 22}, r.seed % 8 == 3
                                                  ? Color{60, 140, 220, 255}
                                                  : Color{196, 84, 52, 255},
                  2.4f);
      } else {
        DrawPebble({x, r.y}, 26, r.seed);
      }
      DrawEllipseLines((int)x, (int)(r.y + 14), 34, 8, {230, 214, 180, 255});
    }
    // the two girls, running along the thinnai under banana leaves
    const float run = std::fabs(std::sin(time * 9)) * 8;
    const float meenaBoatX = 180 + 40 * std::sin(stageTime * 0.7f);
    const float meenaBoatY = RIVER_TOP + 150 + 40 * std::sin(stageTime * 0.9f + 1);
    DrawFolk(LittleMeena(130), meenaBoatX - 10, RIVER_TOP + 6 - run, time,
             {0.05f, 1.15f}, 0.6f, true);
    DrawBananaLeaf({meenaBoatX - 6, RIVER_TOP - 136 - run});
    DrawLittleAnu(boat.x - 20, RIVER_TOP + 6 - std::fabs(std::sin(time * 9 + 1)) * 8,
                  time, 130);
    DrawBananaLeaf({boat.x - 14, RIVER_TOP - 140 - run});
    // the boats
    DrawBoat({meenaBoatX, meenaBoatY}, 0.4f, 0.08f * std::sin(time * 2.4f), false, 0);
    DrawBoat(boat, 0.46f, 0.06f * std::sin(time * 2) + spin, true, 1);
    if (time - splashAt < 0.6f) {
      const float a = (time - splashAt) / 0.6f;
      for (int k = 0; k < 6; k++) {
        const float ang = PI + k * PI / 5;
        DrawCircleV({splash.x + std::cos(ang) * 40 * a,
                     splash.y + std::sin(ang) * 30 * a + 40 * a * a},
                    4 * (1 - a), {230, 240, 255, 255});
      }
    }
    DrawRain(time, 1);
    DrawNewsprint();
    if (stageTime < 2.5f) // steer: the boat follows the pointer
      DrawHint(boat, time, 1 - Between(stageTime, 1.5f, 2.5f));
  }

  static void DrawBananaLeaf(Vector2 c) {
    DrawComicLeaf({c.x - 60, c.y + 10}, {c.x + 66, c.y + 6}, 46, LEAF, 2);
    DrawLineEx({c.x - 60, c.y + 10}, {c.x + 66, c.y + 6}, 2, LEAF_DARK);
    InkLine({c.x, c.y + 8}, {c.x + 4, c.y + 80}, 3, {90, 140, 60, 255});
  }
};

std::unique_ptr<Scene> MakeStreetWalk(const Step &) {
  return std::unique_ptr<Scene>(new StreetWalkScene());
}
std::unique_ptr<Scene> MakeFiveStones(const Step &step) {
  return std::unique_ptr<Scene>(new FiveStonesScene(step));
}
std::unique_ptr<Scene> MakeMeetMeena(const Step &step) {
  return std::unique_ptr<Scene>(new MeetMeenaScene(step));
}
std::unique_ptr<Scene> MakePaperBoats(const Step &step) {
  return std::unique_ptr<Scene>(new PaperBoatsScene(step));
}

} // namespace

PLAY_SCENE("streets_walk", MakeStreetWalk)
PLAY_SCENE("five_stones", MakeFiveStones)
PLAY_SCENE("streets_meena", MakeMeetMeena)
PLAY_SCENE("paper_boats", MakePaperBoats)
