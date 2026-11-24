// ch5_preparations.cpp - Chapter 5, "Preparations": home, four days before
// Pongal. Played, with very few words:
//   prep_mango_leaves  hang mango leaves along the string over the front door
//                      (standing on the thinnai: Appa tied them too high)
//   prep_ladder        (memory) hand Appa the leaves up the ladder, checking
//                      each one first
//   prep_amma_looks    the new doorway kolam; Amma comes out, looks, and
//                      steps carefully around it. No words.
//   prep_pongal        hold to turn up the fire until the milk boils over;
//                      then hold to say "Pongalo pongal."
#include <cmath>
#include <vector>

#include "lp_common.h"

namespace {
using namespace lp;

static std::string Said(const Step &step, size_t i, const char *fallback) {
  return i < step.lines.size() ? step.lines[i].text : fallback;
}

//------------------------------------------------------------------------------
// The front door without its thoranam (the front step in explore_scene.cpp
// has one up already): drawn over it, the same door, so the leaves can go up
// one at a time. Coordinates are the front step's.
//------------------------------------------------------------------------------
const Color DOOR_TEAK = {164, 94, 46, 255};
const Color DOOR_TEAK_DARK = {112, 58, 30, 255};
const Color DOOR_TEAK_LIGHT = {212, 140, 74, 255};
const Color DOOR_BRASS = {244, 188, 56, 255};
const float STRING_X0 = 282, STRING_X1 = 438, STRING_Y = 224;
const int LEAF_SLOTS = 9;

static void DrawBareDoor() {
  // the wall above the frame, with its shading, where the string was
  const Rectangle strip = {266, 204, 188, 20};
  DrawRectangleRec(strip, LIME_PLASTER);
  DrawToneRect(strip, {200, 116, 40, 255}, 0.7f * (1 - 1 / 90.0f),
               0.7f * (1 - 21 / 90.0f), TONE_DOTS, 5);
  DrawToneRect({strip.x, 203, strip.width, 18}, COMIC_INK, 0.55f, 0,
               TONE_LINES, 4);
  for (float x = FullScreen().x + 10; x < 456; x += 44) // rafter ends
    if (x > 256)
      ComicRect({x, 203, 10, 8}, DOOR_TEAK, 1.4f);
  // the carved frame, the two leaves of the door, brass studs and rings
  ComicRect({276, 220, 168, 340}, DOOR_TEAK_DARK, 3.6f);
  for (int i = 0; i < 5; i++)
    InkCircle({300.0f + i * 30, 226}, 3.2f, 1.3f);
  DrawRectangle(290, 232, 140, 328, {52, 28, 20, 255});
  for (int side = 0; side < 2; side++) {
    const float x = 294 + side * 67.0f;
    ComicRect({x, 238, 65, 320}, DOOR_TEAK, 2.6f);
    const Rectangle PANELS[2] = {{x + 9, 252, 47, 118}, {x + 9, 388, 47, 152}};
    for (const Rectangle &p : PANELS) {
      DrawRectangleRec(p, DOOR_TEAK_DARK);
      DrawRectangleRec({p.x, p.y, p.width, 4}, DOOR_TEAK_LIGHT);
      DrawRectangleRec({p.x, p.y, 4, p.height}, DOOR_TEAK_LIGHT);
      const Rectangle in = {p.x + 4, p.y + 4, p.width - 8, p.height - 8};
      DrawRectangleRec(in, DOOR_TEAK);
      InkCurve({in.x + 10, in.y + 8}, {in.x + 18, in.y + in.height / 2},
               {in.x + 12, in.y + in.height - 8}, 1.0f, DOOR_TEAK_DARK);
      InkRect(p, 1.8f);
      InkRect(in, 1.0f);
      for (int k = 0; k < 4; k++) {
        const Vector2 s = {p.x + (k % 2) * p.width, p.y + (k / 2) * p.height};
        ComicCircle(s, 3.4f, DOOR_BRASS, 1.2f);
      }
    }
  }
  DrawLineEx({360, 238}, {360, 558}, 2.6f, COMIC_INK);
  for (float x : {351.0f, 369.0f}) {
    DrawRing({x, 404}, 5, 9.6f, 0, 360, 20, COMIC_INK);
    DrawRing({x, 404}, 6, 8.6f, 0, 360, 20, DOOR_BRASS);
    ComicCircle({x, 396}, 3, DOOR_BRASS, 1.2f);
  }
  DrawToneRect({290, 232, 140, 70}, COMIC_INK, 0.5f, 0, TONE_LINES, 4);
}

// Where leaf `i` of the thoranam hangs from the string
static Vector2 LeafSlot(int i) {
  const float t = (float)i / (LEAF_SLOTS - 1);
  return {STRING_X0 + t * (STRING_X1 - STRING_X0),
          STRING_Y + 7 * std::sin(PI * t)};
}

// The string, the leaves tied on so far (`on`), and once it's all done
// (`flowers` 0..1) the marigolds between them
static void DrawThoranamSoFar(const bool *on, float flowers, float time) {
  DrawSplineSegmentBezierQuadratic({STRING_X0 - 4, STRING_Y - 2},
                                   {(STRING_X0 + STRING_X1) / 2, STRING_Y + 14},
                                   {STRING_X1 + 4, STRING_Y - 2}, 2.2f,
                                   COMIC_INK);
  ComicCircle({STRING_X0 - 4, STRING_Y - 2}, 3, {150, 150, 160, 255}, 1); // nails
  ComicCircle({STRING_X1 + 4, STRING_Y - 2}, 3, {150, 150, 160, 255}, 1);
  for (int i = 0; i < LEAF_SLOTS; i++)
    if (on[i])
      DrawMangoLeaf(LeafSlot(i), 30, 1.5f * std::sin(time * 2 + i), i);
  for (int i = 1; i < 4 && flowers > 0; i++) {
    const float t = i / 4.0f;
    const Vector2 c = {STRING_X0 + t * (STRING_X1 - STRING_X0),
                       STRING_Y + 7 * std::sin(PI * t) + 2};
    const float r = 7.6f * Clamp01(flowers * 3 - (i - 1) * 0.6f);
    if (r > 0.5f)
      DrawFlower(c, r, MARIGOLD, {226, 96, 20, 255}, 8, i);
  }
}

// A bundle of mango leaves, as carried: a fan of leaves tied at the stems
static void DrawLeafBundle(Vector2 c, int count, float time) {
  for (int k = 0; k < count; k++) {
    const float a = -PI / 2 + (k - (count - 1) / 2.0f) * 0.24f;
    const Vector2 tip = {c.x + 50 * std::cos(a), c.y + 50 * std::sin(a)};
    DrawComicLeaf(c, {tip.x + std::sin(time + k), tip.y}, 18,
                  k % 2 ? LEAF_GREEN : LEAF_LIGHT, 1.4f);
  }
  DrawLineEx({c.x - 6, c.y + 2}, {c.x + 6, c.y + 2}, 3, {230, 220, 190, 255});
}

//------------------------------------------------------------------------------
// 1. Mango leaves. Anu, up on the thinnai, ties the leaves on: drag each one
// from Amma's bundle to the string (or tap the bundle). Amma's joke, in
// three short lines while you work.
//------------------------------------------------------------------------------
const float ML_ANU_X = 214, ML_ANU_FEET = 466; // standing on the thinnai
const float ML_AMMA_X = 540, ML_AMMA_FEET = 626;
const Vector2 ML_BUNDLE = {520, 486}; // in Amma's hands

class MangoLeavesScene : public Scene {
public:
  explicit MangoLeavesScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.5f);
    const Vector2 m = Mouse();
    tying = std::fmax(0, tying - dt * 1.6f);
    if (placed < LEAF_SLOTS) {
      if (Pressed() && Dist(m, ML_BUNDLE) < 95) { // (easy for a fingertip)
        holding = true;
        pressAt = m;
      }
      if (holding && Released()) {
        holding = false;
        int best = -1;
        float bestD = 140; // anywhere near the string will do
        for (int i = 0; i < LEAF_SLOTS; i++) {
          const float d = Dist(m, LeafSlot(i));
          if (!on[i] && d < bestD) {
            best = i;
            bestD = d;
          }
        }
        if (best < 0 && m.y < STRING_Y + 140) // up by the door: the next one
          for (int i = 0; i < LEAF_SLOTS && best < 0; i++)
            if (!on[i])
              best = i;
        if (best < 0 && Dist(m, pressAt) < 12) // a tap: the next one along
          for (int i = 0; i < LEAF_SLOTS && best < 0; i++)
            if (!on[i])
              best = i;
        if (best >= 0) {
          on[best] = true;
          placed++;
          lastSlot = best;
          tying = 1;
          PlayEffect("click", 0.5f, 1.0f + 0.05f * placed);
        }
      }
    } else {
      flowers = std::fmin(1, flowers + dt * 0.8f);
      doneTime += dt;
      if (doneTime > 1.6f && ContinuePressed())
        done = true;
    }
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    if (holding) {
      DrawComicLeaf({Mouse().x, Mouse().y - 14}, {Mouse().x + 3, Mouse().y + 18},
                    12, LEAF_LIGHT, 1.4f);
    } else if (placed < LEAF_SLOTS && time > 1) {
      DrawHint(ML_BUNDLE, 50, time);
      for (int i = 0; i < LEAF_SLOTS; i++)
        if (!on[i]) {
          DrawHint(LeafSlot(i), 18, time + 1, {200, 255, 170, 255});
          break;
        }
    }
    // Amma's joke, a line at a time while the leaves go up
    const Vector2 amma = {ML_AMMA_X - 4, ML_AMMA_FEET - 296};
    const Vector2 anu = {ML_ANU_X - 6, ML_ANU_FEET - 270};
    const float a = time - 0.8f;
    DrawBubble(Said(step, 0, "Your Appa always tied these. Too high."),
               {470, 120}, amma, Clamp01(a * 3) * Clamp01((4.5f - a) * 3), 20);
    if (placed >= 3)
      DrawBubble(Said(step, 1, "So no one could reach them."), {250, 110}, anu,
                 Clamp01((Age(3)) * 3) * Clamp01((3.5f - Age(3)) * 3), 20);
    if (placed >= 3 && Age(3) > 3.5f)
      DrawBubble(Said(step, 2, "So I couldn't reach them."), {490, 120}, amma,
                 Clamp01((Age(3) - 3.7f) * 3) * Clamp01((8.5f - Age(3)) * 3),
                 20);
    if (placed == LEAF_SLOTS && doneTime > 1.6f)
      DrawGoOn(time);
    if (placed == LEAF_SLOTS && doneTime < 1.5f)
      for (int k = 0; k < 3; k++)
        DrawSparkle({STRING_X0 + 40 + k * 38, STRING_Y + 10},
                    8 * std::sin(PI * doneTime / 1.5f), {255, 250, 220, 255});
  }

private:
  const Step &step;
  bool on[LEAF_SLOTS] = {};
  int placed = 0, lastSlot = 0;
  bool holding = false;
  Vector2 pressAt = {};
  float time = 0, tying = 0, flowers = 0, doneTime = 0;
  float placedAt[LEAF_SLOTS + 1] = {};
  static MangoLeavesScene *current;

  float Age(int count) { // seconds since the count-th leaf went up
    if (placedAt[count] == 0 && placed >= count)
      placedAt[count] = time;
    return placed >= count ? time - placedAt[count] : -1;
  }

  static void DrawPeople(float t) {
    MangoLeavesScene &s = *current;
    DrawBareDoor();
    DrawThoranamSoFar(s.on, s.flowers, t);
    DrawAmma(ML_AMMA_X, ML_AMMA_FEET, t, false);
    DrawLeafBundle(ML_BUNDLE, s.placed < LEAF_SLOTS ? 5 : 0, t);
    // Anu reaches up to tie each one; while a leaf is held, her hand
    // follows it a little
    Vector2 hand = {0.13f, 0.5f};
    const Vector2 up = AnuHandTo(LeafSlot(s.lastSlot));
    if (s.tying > 0)
      hand = Lerp2(hand, up, Clamp01(s.tying * 2));
    else if (s.holding)
      hand = Lerp2(hand, AnuHandTo(Mouse()), 0.5f);
    hand.x = std::fmax(-0.1f, std::fmin(0.42f, hand.x));
    hand.y = std::fmax(0.3f, std::fmin(0.95f, hand.y));
    DrawAnu(ML_ANU_X, ML_ANU_FEET, t, hand, {0.4f, -0.6f});
  }
  static Vector2 AnuHandTo(Vector2 p) {
    return {(p.x - ML_ANU_X) / 285, (ML_ANU_FEET - p.y) / 285};
  }
};
MangoLeavesScene *MangoLeavesScene::current = nullptr;

//------------------------------------------------------------------------------
// 2. Memory: the ladder. Appa up the ladder, little Anu at the bottom with
// the basket, handing up one leaf at a time. She checks each one first:
// a torn or yellow one, handed up anyway, just flutters back down.
//------------------------------------------------------------------------------
const Vector2 LD_APPA_FEET = {236, 524}; // on a rung
const float LD_APPA_SCALE = 2.0f;        // art.cpp draws him small
const Vector2 LD_APPA_HAND = {322, 230}; // his hand up at the string
const float LD_ANU_X = 130, LD_ANU_FEET = 668, LD_ANU_H = 190;
const Vector2 LD_BASKET = {66, 640};
const int LD_LEAVES = 10;
const int LD_NEEDED = 6; // three are up already

class LadderScene : public Scene {
public:
  explicit LadderScene(const Step &) {
    for (int i = 0; i < 3; i++)
      on[i] = true;
    for (int i = 0; i < LD_LEAVES; i++) {
      leaves[i].bad = i == 2 || i == 5 || i == 8 ? (i % 2) + 1 : 0;
      leaves[i].at = Fanned(i);
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.5f);
    const Vector2 m = Mouse();
    for (Leaf &l : leaves) {
      if (l.state == FALLING) { // back down to the ground, see-sawing
        l.t += dt;
        l.at.y += 120 * dt;
        l.at.x += 60 * std::sin(l.t * 4) * dt;
        if (l.at.y > 690)
          l.state = GONE;
      } else if (l.state == RETURNING) {
        l.at = Lerp2(l.at, Fanned(&l - leaves), std::fmin(1, dt * 10));
        if (Dist(l.at, Fanned(&l - leaves)) < 2)
          l.state = IN_BASKET;
      }
    }
    if (handedUp < LD_NEEDED) {
      if (Pressed() && held < 0) {
        float bestD = 80; // (easy for a fingertip)
        for (int i = 0; i < LD_LEAVES; i++)
          if (leaves[i].state == IN_BASKET && Dist(m, Tip(i)) < bestD) {
            bestD = Dist(m, Tip(i));
            held = i;
          }
        if (held >= 0)
          leaves[held].state = HELD;
      }
      if (held >= 0) {
        leaves[held].at = {m.x, m.y + 30};
        if (Released()) {
          Leaf &l = leaves[held];
          // anywhere up near Appa will do: he reaches down for it
          if (Dist(m, LD_APPA_HAND) < 170 || m.y < LD_APPA_HAND.y + 40) {
            if (l.bad) { // he looks at it, and lets it go
              l.state = FALLING;
              l.t = 0;
              PlayEffect("page", 0.3f, 1.6f);
            } else {
              l.state = GONE;
              for (int i = 0; i < LEAF_SLOTS; i++)
                if (!on[i]) {
                  on[i] = true;
                  break;
                }
              handedUp++;
              tiedAt = time;
              PlayEffect("click", 0.5f, 1.1f + 0.05f * handedUp);
            }
          } else {
            l.state = RETURNING;
          }
          held = -1;
        }
      }
    } else {
      flowers = std::fmin(1, flowers + dt * 0.8f);
      doneTime += dt;
      if (doneTime > 1.6f && ContinuePressed())
        done = true;
    }
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    if (held >= 0) // the one she's checking, up close
      DrawLeafUp(leaves[held], 1.3f);
    if (handedUp < LD_NEEDED && held < 0 && time > 1)
      DrawHint({LD_BASKET.x + 10, LD_BASKET.y - 60}, 60, time);
    if (held >= 0)
      DrawHint(LD_APPA_HAND, 60, time, {255, 240, 180, 255});
    if (handedUp >= LD_NEEDED && doneTime > 1.6f)
      DrawGoOn(time);
  }

private:
  enum State { IN_BASKET, HELD, RETURNING, FALLING, GONE };
  struct Leaf {
    Vector2 at;  // its stem
    int bad = 0; // 0 good, 1 torn, 2 gone yellow
    State state = IN_BASKET;
    float t = 0;
  };
  Leaf leaves[LD_LEAVES];
  bool on[LEAF_SLOTS] = {};
  int held = -1, handedUp = 0;
  float time = 0, tiedAt = -10, flowers = 0, doneTime = 0;
  static LadderScene *current;

  static Vector2 Fanned(long i) { // standing up in the basket, fanned out
    return {LD_BASKET.x - 40 + i * 10.0f, LD_BASKET.y - 10 + (i % 2) * 4.0f};
  }
  Vector2 Tip(int i) const {
    const float a = -PI / 2 + (i - (LD_LEAVES - 1) / 2.0f) * 0.1f;
    return {leaves[i].at.x + 40 * std::cos(a), leaves[i].at.y + 40 * std::sin(a)};
  }

  static void DrawLeafUp(const Leaf &l, float s) { // by the stem, tip up
    const Color fill = l.bad == 2   ? Color{214, 190, 70, 255}
                       : l.bad == 1 ? Color{170, 160, 80, 255}
                                    : LEAF_LIGHT;
    const Vector2 tip = {l.at.x + 4 * s, l.at.y - 56 * s};
    DrawComicLeaf(l.at, tip, 22 * s, fill, 1.6f);
    if (l.bad == 1) { // torn: bites out of its edge
      DrawCircleV(Lerp2(l.at, tip, 0.45f), 6 * s, COMIC_INK);
      DrawCircleV(Lerp2(l.at, tip, 0.45f), 4.6f * s, {220, 190, 150, 255});
      DrawCircleV({Lerp2(l.at, tip, 0.65f).x + 6 * s, Lerp2(l.at, tip, 0.65f).y},
                  4 * s, {220, 190, 150, 255});
    }
    if (l.bad == 2)
      for (int k = 0; k < 3; k++)
        DrawCircleV(Lerp2(l.at, tip, 0.3f + k * 0.2f), 2.5f * s,
                    {150, 110, 50, 255});
  }

  static void DrawPeople(float t) {
    LadderScene &s = *current;
    DrawBareDoor();
    DrawThoranamSoFar(s.on, s.flowers, t);
    // the ladder against the wall, left of the door
    const Vector2 foot[2] = {{196, 600}, {262, 600}}, top[2] = {{214, 190}, {270, 190}};
    for (int k = 0; k < 2; k++) {
      DrawLineEx(foot[k], top[k], 10, COMIC_INK);
      DrawLineEx(foot[k], top[k], 6, {200, 160, 100, 255});
    }
    for (int r = 0; r < 10; r++) {
      const float u = (r + 0.5f) / 10;
      InkLine(Lerp2(foot[0], top[0], u), Lerp2(foot[1], top[1], u), 4.5f);
      DrawLineEx(Lerp2(foot[0], top[0], u), Lerp2(foot[1], top[1], u), 2.4f,
                 {220, 180, 120, 255});
    }
    // Appa on it, reaching up to the string
    PushZoom(LD_APPA_FEET, LD_APPA_SCALE);
    DrawAppa(LD_APPA_FEET.x, LD_APPA_FEET.y, t);
    PopZoom();
    if (t - s.tiedAt < 0.6f) // a leaf in his hand, being tied
      DrawComicLeaf({LD_APPA_HAND.x, LD_APPA_HAND.y + 10},
                    {LD_APPA_HAND.x + 2, LD_APPA_HAND.y + 40}, 11, LEAF_LIGHT,
                    1.4f);
    // little Anu and her basket of leaves
    DrawLittleAnu(LD_ANU_X, LD_ANU_FEET, t, LD_ANU_H);
    for (int i = 0; i < LD_LEAVES; i++) {
      const Leaf &l = s.leaves[i];
      if (l.state == IN_BASKET || l.state == RETURNING)
        DrawLeafUp(l, 0.7f);
      if (l.state == FALLING)
        DrawLeafUp(l, 0.7f);
    }
    ComicEllipse({LD_BASKET.x, LD_BASKET.y}, 54, 14, {196, 150, 80, 255}, 2);
    const Vector2 front[4] = {{LD_BASKET.x - 54, LD_BASKET.y},
                              {LD_BASKET.x + 54, LD_BASKET.y},
                              {LD_BASKET.x + 42, LD_BASKET.y + 36},
                              {LD_BASKET.x - 42, LD_BASKET.y + 36}};
    ComicPoly(front, 4, {214, 168, 96, 255}, 2);
    for (int k = 0; k < 6; k++)
      InkLine({LD_BASKET.x - 44 + k * 18, LD_BASKET.y + 4},
              {LD_BASKET.x - 36 + k * 14, LD_BASKET.y + 32}, 1.2f,
              {150, 100, 50, 255});
  }
};
LadderScene *LadderScene::current = nullptr;

//------------------------------------------------------------------------------
// 3. Amma looks. The doorway kolam Anu has just drawn (the page "Vaasal",
// solved), in rice flour on the wet patch. Amma comes out, looks, says
// nothing, and steps carefully around it. No words.
//------------------------------------------------------------------------------
const int VAASAL_PAGE = 6;
const Vector2 KOLAM_AT = {360, 622}; // its middle, on the ground
const float KOLAM_SQUASH = 0.36f;     // seen from the side, lying flat

class AmmaLooksScene : public Scene {
public:
  explicit AmmaLooksScene(const Step &) {
    KolamGrid grid;
    if ((int)Pages().size() > VAASAL_PAGE && grid.Load(Pages()[VAASAL_PAGE])) {
      grid.CountSolutions(true);
      const float cell = 70;
      const Vector2 origin = {KOLAM_AT.x - grid.width * cell / 2,
                              KOLAM_AT.y - grid.height * cell / 2};
      paths = LoopPaths(grid, origin, cell, 1.2f);
      for (int y = 0; y < grid.height; y++)
        for (int x = 0; x < grid.width; x++)
          dots.push_back(Flat({origin.x + (x + 0.5f) * cell,
                               origin.y + (y + 0.5f) * cell}));
      for (auto &path : paths) {
        for (Vector2 &p : path)
          p = Flat(p);
        if (!path.empty())
          path.push_back(path.front()); // closed
      }
    }
  }

  void Update() override {
    time += GetFrameTime();
    PlayAmbience("birds_day", 0.5f);
    done = time > WALK_END + 0.8f && ContinuePressed();
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    if (time > WALK_END + 0.8f)
      DrawGoOn(time);
  }

private:
  std::vector<std::vector<Vector2>> paths;
  std::vector<Vector2> dots;
  float time = 0;
  static AmmaLooksScene *current;
  // her walk: out of the door, a pause, around the kolam, off down the street
  const float OUT = 1.2f, LOOK = 3.6f, WALK_END = 8.5f;

  static Vector2 Flat(Vector2 p) {
    return {p.x, KOLAM_AT.y + (p.y - KOLAM_AT.y) * KOLAM_SQUASH};
  }

  // Amma's feet over time (and how big, nearer the front)
  Vector2 AmmaAt(float t) const {
    const Vector2 PATH[] = {{360, 566}, {360, 580}, {360, 580}, {470, 590},
                            {540, 640}, {560, 690}, {760, 700}, {1160, 700}};
    const float TIMES[] = {0, OUT, LOOK, LOOK + 1.2f, LOOK + 2.2f,
                           LOOK + 3.2f, LOOK + 4.2f, WALK_END};
    for (int i = 0; i < 7; i++)
      if (t < TIMES[i + 1])
        return Lerp2(PATH[i], PATH[i + 1],
                     Between(t, TIMES[i], TIMES[i + 1]) * 0.7f +
                         0.3f * Clamp01((t - TIMES[i]) / (TIMES[i + 1] - TIMES[i])));
    return PATH[7];
  }

  static void DrawPeople(float t) {
    const AmmaLooksScene &s = *current;
    // the wet patch, fresh, and the new kolam on it
    DrawEllipse(360, 618, 214, 46, {158, 118, 82, 255});
    const Color flour = {252, 248, 236, 255};
    for (const Vector2 &d : s.dots)
      DrawEllipse((int)d.x, (int)d.y, 3.2f, 1.6f, flour);
    for (const auto &path : s.paths) // closed loops, one mitred line each
      DrawSplineLinear(path.data(), (int)path.size(), 2.6f, flour);
    // Anu, sitting back on her heels beside it, the flour bowl by her
    DrawAnuSitting(170, 660, t);
    ComicEllipse({236, 662}, 18, 6, {200, 200, 210, 255}, 1.6f);
    DrawEllipse(236, 658, 12, 3, flour);
    // Amma: out of the door, she stops and looks down; then round it,
    // small careful steps
    if (t < s.OUT + 0.6f) { // the door stands open behind her
      DrawRectangle(290, 232, 140, 328, {52, 28, 20, 255});
      DrawToneRect({290, 232, 140, 328}, {255, 200, 120, 255}, 0, 0.25f,
                   TONE_DOTS, 5);
    }
    const Vector2 feet = s.AmmaAt(t);
    const bool walking = (t > 0.1f && t < s.OUT) || (t > s.LOOK && t < s.WALK_END);
    const float step = walking ? std::fabs(std::sin(t * 7)) * 4 : 0;
    const float scale = 1 + (feet.y - 580) / 600;
    PushZoom(feet, scale);
    DrawAmma(feet.x, feet.y - step, t, false);
    PopZoom();
    if (t > s.OUT + 0.3f && t < s.LOOK) // she looks a while
      for (int k = 0; k < 3; k++)
        DrawSparkle({300.0f + k * 60, 620.0f + (k % 2) * 8},
                    5 * std::sin(PI * Clamp01((t - s.OUT - 0.3f - k * 0.3f) / 1.4f)),
                    {255, 252, 230, 255});
  }
};
AmmaLooksScene *AmmaLooksScene::current = nullptr;

//------------------------------------------------------------------------------
// 4. The pongal pot. Hold to turn the fire up; the milk rises, and sinks a
// little when you let go. "Watch it..." At the rim it boils over, the way
// it's meant to: "Pongalo pongal!" Then hold to say it back.
//------------------------------------------------------------------------------
const Vector2 POT = {584, 356}; // the body's middle
const float POT_R = 60;
const float RIM_Y = POT.y - POT_R * 1.02f;

class PongalScene : public Scene {
public:
  explicit PongalScene(const Step &s) : step(s) {
    // the dialogue at the end: everything after "Watch it..."
    for (size_t i = 1; i < step.lines.size(); i++)
      endLines.push_back(step.lines[i]);
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.35f);
    if (!boiled) {
      const bool hold = Held() && time > 0.6f;
      heat += ((hold ? 1.0f : 0.15f) - heat) * std::fmin(1, dt * (hold ? 1.5f : 0.8f));
      if (hold)
        PlayLoop("pour", 0.12f + 0.2f * heat); // the fire's roar
      idle = hold ? 0 : idle + dt;
      if (hold)
        held = true;
      // the milk warms, then rises; with less fire it settles back, slowly
      // (about 3 seconds of holding boils it)
      warm = std::fmin(1, warm + (heat - 0.2f) * dt * 0.8f);
      warm = std::fmax(0, warm);
      rise += ((warm > 0.35f ? (warm - 0.35f) * 2.6f * heat : 0) - rise * 0.1f) * dt;
      rise = std::fmax(0, rise);
      if (rise > 0.7f && watchAt < 0)
        watchAt = time;
      if (rise >= 1) {
        boiled = true;
        boilAt = time;
        PlayEffect("temple_bells", 0.5f);
      }
    } else {
      heat += (0.2f - heat) * std::fmin(1, dt);
      if (time - boilAt > 2.0f && !talked)
        talked = AdvanceDialogue(endLines, line, typed);
      if (talked)
        done = true;
    }
  }

  void Draw() override {
    current = this;
    DrawPlace("kitchen", DAY, time, DrawPeople);
    if (!boiled && time > 0.8f) {
      // how close it is to boiling: a ring round the pot fills
      const float near = Clamp01(rise);
      if (near > 0.02f) {
        DrawRing(POT, POT_R + 14, POT_R + 20, 0, 360, 48, Faded(COMIC_PAPER, 0.3f));
        DrawRing(POT, POT_R + 14, POT_R + 20, -90, -90 + 360 * near, 48,
                 {255, 200, 120, 255});
      }
      // hold: shown until the fire has been turned up, and again if the
      // player stops for a while
      if (!held || idle > 4)
        DrawGesture(MOVE_HOLD, {POT.x, 430}, time);
    }
    if (watchAt >= 0 && !boiled)
      DrawBubble(Said(step, 0, "Watch it. Watch it..."), {300, 150},
                 {440, 316}, Clamp01((time - watchAt) * 3), 20);
    if (boiled && time - boilAt > 2.0f && !endLines.empty() &&
        line < endLines.size()) { // in a balloon, like the rest of the story
      const Line &l = endLines[line];
      const bool anu = l.speaker == "Anu";
      const Vector2 who = anu ? Vector2{250, 350} : Vector2{440, 316};
      const Vector2 at = anu ? Vector2{240, 150} : Vector2{440, 150};
      const size_t shown = std::min(l.text.size(), (size_t)typed);
      DrawBubble(l.text.substr(0, shown), at, who, 1, 24);
      if (l.hold > 0 && shown < l.text.size()) // "hold to say it"
        DrawRing({at.x, at.y + 50}, 9, 12, 0, 360, 24,
                 Faded(COMIC_INK, 0.4f + 0.4f * std::sin(time * 4)));
    }
  }

private:
  const Step &step;
  std::vector<Line> endLines;
  size_t line = 0;
  float typed = 0;
  bool talked = false;
  float time = 0, heat = 0.15f, warm = 0, rise = 0;
  float watchAt = -1, boilAt = 0;
  bool boiled = false;
  float idle = 0;     // how long since the fire was last held up
  bool held = false;  // has it been, yet?
  static PongalScene *current;

  static void DrawPeople(float t) {
    const PongalScene &s = *current;
    // the flames under the pot, as high as the fire is turned up
    for (int k = 0; k < 13; k++) {
      const float x = 530 + k * 9.0f;
      const float h = (8 + 26 * s.heat) * (0.8f + 0.25f * std::sin(t * 14 + k * 1.7f));
      const float lick = 2 * std::sin(t * 9 + k);
      FillTriangle({x - 5, 401}, {x + 5, 401}, {x + lick, 401 - h},
                   {70, 150, 255, 255});
      FillTriangle({x - 2, 401}, {x + 2, 401}, {x, 404 - h * 0.6f},
                   {200, 236, 255, 255});
    }
    // the pongal pot: clay, kolam lines painted on, turmeric leaves tied
    // round its neck
    const Color clay = {200, 96, 52, 255}, clayDark = {150, 64, 36, 255};
    ComicCircle(POT, POT_R, clay, 3);
    DrawCircleSector({POT.x + POT_R * 0.2f, POT.y + POT_R * 0.15f}, POT_R * 0.9f,
                     -40, 120, 20, clayDark);
    DrawCircleV({POT.x - POT_R * 0.1f, POT.y - POT_R * 0.05f}, POT_R * 0.8f, clay);
    InkCircle(POT, POT_R, 3);
    for (int k = 0; k < 5; k++) // white kolam dots and loops painted on
      DrawCircleV({POT.x - 40 + k * 20, POT.y + 14}, 3, COMIC_PAPER);
    DrawEllipseArc({POT.x, POT.y + 14}, 48, 14, 0, PI, 2, COMIC_PAPER, 14);
    DrawEllipseArc({POT.x, POT.y + 14}, 48, 14, PI, 2 * PI, 2, COMIC_PAPER, 14);
    DrawToneRect({POT.x + 10, POT.y - 40, 50, 90}, COMIC_INK, 0.1f, 0.4f,
                 TONE_DOTS, 4);
    ComicRect({POT.x - 40, RIM_Y, 80, 16}, clay, 2.4f); // the neck
    ComicEllipse({POT.x, RIM_Y}, 44, 9, clayDark, 2.4f); // the mouth
    for (int k = 0; k < 5; k++) // turmeric leaves round the neck
      DrawComicLeaf({POT.x - 32 + k * 16, RIM_Y + 12},
                    {POT.x - 44 + k * 22, RIM_Y + 50}, 13,
                    k % 2 ? LEAF_GREEN : LEAF_LIGHT, 1.4f);
    // the milk: a white skin at the mouth, rising as a dome of foam,
    // and over the edge
    const Color milk = {250, 248, 238, 255};
    const float rise = std::fmin(1, s.rise);
    DrawEllipse((int)POT.x, (int)RIM_Y, 40, 7, milk);
    if (rise > 0.05f) {
      const float h = 34 * rise;
      DrawEllipse((int)POT.x, (int)(RIM_Y - h * 0.4f), 42 + 2, h + 2, COMIC_INK);
      DrawEllipse((int)POT.x, (int)(RIM_Y - h * 0.4f), 42, h, milk);
      DrawRectangle((int)POT.x - 44, (int)RIM_Y - 2, 88, 12, milk);
      for (int k = 0; k < 7; k++) { // bubbles
        const float bt = std::fmod(t * (1.5f + k * 0.2f) + k * 0.37f, 1.0f);
        InkCircle({POT.x - 30 + k * 10, RIM_Y - h * 0.4f - h * 0.6f * bt},
                  2 + 3 * bt * rise, 1.2f, {190, 190, 200, 255});
      }
    }
    if (s.boiled) { // boiling over: foam running down the sides
      const float over = Between(t - s.boilAt, 0, 1.6f);
      for (int k = 0; k < 6; k++) {
        const float x = POT.x - 46 + k * 18.4f;
        const float len = (30 + (k * 23) % 40) * over;
        DrawLineEx({x, RIM_Y}, {x + (k - 2.5f) * 2, RIM_Y + len}, 12.5f, COMIC_INK);
        DrawLineEx({x, RIM_Y}, {x + (k - 2.5f) * 2, RIM_Y + len}, 9, milk);
        DrawCircleV({x + (k - 2.5f) * 2, RIM_Y + len}, 5.5f, milk);
      }
      for (int k = 0; k < 6; k++) // steam
        DrawSparkle({POT.x - 50 + k * 20, RIM_Y - 50 - 30 * over - (k % 2) * 16},
                    6 * over, {255, 255, 255, 220});
    }
    // Anu, watching the pot from the other side of Amma
    DrawAnu(250, 600, t, {0.13f, 0.5f}, {0.7f, 0.1f});
  }
};
PongalScene *PongalScene::current = nullptr;

template <class T> std::unique_ptr<Scene> Make(const Step &step) {
  return std::unique_ptr<Scene>(new T(step));
}
static std::unique_ptr<Scene> (*const MakeMangoLeaves)(const Step &) =
    Make<MangoLeavesScene>;
static std::unique_ptr<Scene> (*const MakeLadder)(const Step &) =
    Make<LadderScene>;
static std::unique_ptr<Scene> (*const MakeAmmaLooks)(const Step &) =
    Make<AmmaLooksScene>;
static std::unique_ptr<Scene> (*const MakePongal)(const Step &) =
    Make<PongalScene>;

} // namespace

PLAY_SCENE("prep_mango_leaves", MakeMangoLeaves)
PLAY_SCENE("prep_ladder", MakeLadder)
PLAY_SCENE("prep_amma_looks", MakeAmmaLooks)
PLAY_SCENE("prep_pongal", MakePongal)
