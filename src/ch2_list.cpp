// ch2_list.cpp - Chapter 2, "The List": shopping with Amma, ten days before
// Pongal. Every step is a place you play in, with very few words:
//   list_setting_out  fold the cloth bags; take Amma's paper list (not the
//                     phone)
//   list_market       tap the right stall for each line of the list: it goes
//                     in the bag and gets a pencil tick
//   list_flowers      measure two muzham of jasmine; then reach for the
//                     flowers you used to want
//   list_kanakambaram (memory) pin orange flowers into little Anu's plaits
//   list_bangles      choose glass bangles, slowly, one at a time
//   list_rain         (memory) shake six bangles on a small wrist: rain
//   list_home         unpack the bag: everything but the flowers
//   list_terrace      night; the bangles, and nothing to say
#include <cmath>
#include <vector>

#include "lp_common.h"

namespace {
using namespace lp;

//------------------------------------------------------------------------------
// Shared: Amma's list, the cloth bag, people drawn smaller (further away)
//------------------------------------------------------------------------------
enum Item {
  MALLIPOO,
  BANANA_LEAVES,
  TURMERIC,
  CAMPHOR,
  OIL,
  COCONUTS,
  FLOWERS, // the last line: only a little drawing of flowers
  ITEM_COUNT
};
const char *const ITEM_TEXT[ITEM_COUNT] = {
    "Mallipoo, 2 muzham", "Banana leaves", "Turmeric, kumkum",
    "Camphor, wicks",     "Lamp oil",      "Coconuts, 4", ""};
const Color TURMERIC_YELLOW = {246, 196, 30, 255};
const Color KUMKUM_RED = {204, 30, 50, 255};
const Color COCONUT_BROWN = {130, 82, 44, 255};
const Color OIL_AMBER = {232, 164, 40, 255};
const Color JASMINE = {252, 250, 240, 255};
const Color BAG_CLOTH = {238, 228, 204, 255};
const Color BAG_PRINT = {58, 108, 180, 255};

// The words a scene says come from its step in story.cpp (all the words are
// there); `fallback` only if the step has fewer lines
static std::string Said(const Step &step, size_t i, const char *fallback) {
  return i < step.lines.size() ? step.lines[i].text : fallback;
}

// A pencil doodle of three little flowers: the last line of the list
static void DrawFlowerDoodle(Vector2 c, float s, Color ink) {
  for (int k = 0; k < 3; k++) {
    const Vector2 f = {c.x + (k - 1) * 13 * s, c.y + (k == 1 ? -5 : 1) * s};
    InkLine({f.x, f.y + 4 * s}, {f.x + (k - 1) * 2 * s, f.y + 14 * s}, 1.2f,
            ink);
    for (int p = 0; p < 5; p++) {
      const float a = p * 2 * PI / 5 - PI / 2;
      DrawRing({f.x + 3.6f * s * std::cos(a), f.y + 3.6f * s * std::sin(a)},
               2.2f * s, 3.2f * s, 0, 360, 10, ink);
    }
  }
}

// A small picture of each thing on the list, so it needn't be read
static void DrawItemIcon(int item, Vector2 c, float s) {
  switch (item) {
  case MALLIPOO: // a string of jasmine buds
    for (int i = 0; i < 6; i++) {
      const Vector2 p = {c.x - 12 * s + i * 5 * s,
                         c.y + 3 * s * std::sin(i * 1.1f)};
      ComicEllipse(p, 2.8f * s, 2 * s, JASMINE, 1);
    }
    break;
  case BANANA_LEAVES:
    DrawComicLeaf({c.x - 10 * s, c.y + 6 * s}, {c.x + 12 * s, c.y - 6 * s},
                  9 * s, LEAF_LIGHT, 1.2f);
    break;
  case TURMERIC: {
    const Vector2 y[3] = {{c.x - 12 * s, c.y + 6 * s},
                          {c.x - 1 * s, c.y + 6 * s},
                          {c.x - 6.5f * s, c.y - 6 * s}};
    const Vector2 r[3] = {{c.x + 1 * s, c.y + 6 * s},
                          {c.x + 12 * s, c.y + 6 * s},
                          {c.x + 6.5f * s, c.y - 6 * s}};
    ComicPoly(y, 3, TURMERIC_YELLOW, 1.2f);
    ComicPoly(r, 3, KUMKUM_RED, 1.2f);
    break;
  }
  case CAMPHOR:
    for (int i = 0; i < 3; i++)
      ComicRect({c.x - 12 * s + i * 7 * s, c.y - 2 * s, 6 * s, 6 * s},
                COMIC_PAPER, 1);
    InkCurve({c.x + 8 * s, c.y + 5 * s}, {c.x + 14 * s, c.y - 2 * s},
             {c.x + 10 * s, c.y - 8 * s}, 1.6f, {200, 190, 170, 255});
    break;
  case OIL:
    ComicRect({c.x - 5 * s, c.y - 4 * s, 10 * s, 13 * s}, OIL_AMBER, 1.2f);
    ComicRect({c.x - 2.5f * s, c.y - 9 * s, 5 * s, 5 * s}, {200, 60, 40, 255},
              1);
    break;
  case COCONUTS:
    ComicCircle({c.x - 5 * s, c.y + 2 * s}, 6 * s, COCONUT_BROWN, 1.2f);
    ComicCircle({c.x + 5 * s, c.y + 2 * s}, 6 * s, COCONUT_BROWN, 1.2f);
    break;
  default:
    DrawFlowerDoodle(c, s * 0.8f, PENCIL);
    break;
  }
}

// Amma's list on its scrap of paper, top left at `at`, a little tilted.
// `ticks`: how far each line's pencil tick is drawn (0..1); `glow`: lines
// to light up (0..1).
static void DrawShoppingList(Vector2 at, float scale, float tilt,
                             const float *ticks, const float *glow = nullptr) {
  const float w = 210, lineH = 36, h = 30 + lineH * ITEM_COUNT;
  rlPushMatrix();
  rlTranslatef(at.x, at.y, 0);
  rlRotatef(tilt, 0, 0, 1);
  rlScalef(scale, scale, 1);
  DrawRectangleRec({4, 5, w, h}, Faded(COMIC_INK, 0.35f)); // its shadow
  DrawRectangleRec({0, 0, w, h}, LIST_PAPER);
  for (int i = 0; i < 9; i++) // torn top edge
    FillTriangle({i * w / 9, 0}, {(i + 1) * w / 9, 0},
                 {(i + 0.5f) * w / 9, -4.0f - (i % 3)}, LIST_PAPER);
  InkRect({0, 0, w, h}, 1.6f);
  for (int i = 0; i < ITEM_COUNT; i++) {
    const float y = 22 + i * lineH;
    DrawLineEx({8, y + lineH - 6}, {w - 8, y + lineH - 6}, 1,
               {170, 196, 220, 255});
    if (glow && glow[i] > 0)
      DrawRectangleRec({4, y - 2, w - 8, lineH - 4},
                       Faded({255, 220, 120, 255}, 0.5f * glow[i]));
    if (i != FLOWERS)
      DrawItemIcon(i, {24, y + 13}, 0.9f);
    if (i == FLOWERS)
      DrawFlowerDoodle({80, y + 12}, 1, BALLPOINT);
    else
      DrawString(ITEM_TEXT[i], 44, y, 21, BALLPOINT, HAND_FONT);
    if (ticks)
      DrawTick({w - 22, y + 14}, ticks[i], 16);
  }
  rlPopMatrix();
}

// The cotton cloth bag, standing, its bottom middle at `base`. `has`: which
// things are in it (they peek out of the top).
static void DrawClothBag(Vector2 base, float s, const bool *has, float time) {
  const float w = 110 * s, h = 120 * s;
  const float top = base.y - h;
  // what's inside, peeking out (drawn first: the bag's front covers them)
  if (has) {
    if (has[BANANA_LEAVES])
      for (int k = 0; k < 3; k++)
        DrawComicLeaf({base.x - 20 * s + k * 8 * s, top + 20 * s},
                      {base.x - 40 * s + k * 18 * s, top - 60 * s}, 22 * s,
                      k % 2 ? LEAF_GREEN : LEAF_LIGHT, 1.4f);
    if (has[OIL]) {
      ComicRect({base.x + 18 * s, top - 26 * s, 16 * s, 40 * s}, OIL_AMBER,
                1.6f);
      ComicRect({base.x + 21 * s, top - 34 * s, 10 * s, 9 * s},
                {200, 60, 40, 255}, 1.2f);
    }
    if (has[COCONUTS])
      for (int k = 0; k < 2; k++)
        ComicCircle({base.x - 22 * s + k * 26 * s, top + 2 * s}, 15 * s,
                    COCONUT_BROWN, 1.8f);
    if (has[TURMERIC]) {
      ComicRect({base.x - 6 * s, top - 10 * s, 18 * s, 18 * s},
                TURMERIC_YELLOW, 1.4f);
      ComicRect({base.x + 6 * s, top - 6 * s, 16 * s, 16 * s}, KUMKUM_RED,
                1.4f);
    }
    if (has[CAMPHOR])
      ComicRect({base.x - 40 * s, top - 4 * s, 20 * s, 12 * s}, COMIC_PAPER,
                1.4f);
  }
  // the bag: a soft trapezoid of cotton with a printed band, two handles
  for (int k = -1; k <= 1; k += 2)
    InkCurve({base.x + k * 32 * s, top + 4}, {base.x + k * 26 * s, top - 46 * s},
             {base.x + k * 4 * s, top + 4}, 4.5f * s, BAG_CLOTH);
  const Vector2 body[4] = {{base.x - w / 2, top},
                           {base.x + w / 2, top},
                           {base.x + w / 2 + 6 * s, base.y},
                           {base.x - w / 2 - 6 * s, base.y}};
  ComicPoly(body, 4, BAG_CLOTH, 2.4f);
  DrawToneRect({base.x - w / 2, top + h * 0.45f, w, h * 0.22f}, BAG_PRINT, 1,
               1, TONE_DOTS, 4);
  InkLine({base.x - w / 2 - 2, top + h * 0.45f},
          {base.x + w / 2 + 2, top + h * 0.45f}, 1.4f);
  InkLine({base.x - w / 2 - 4, top + h * 0.67f},
          {base.x + w / 2 + 4, top + h * 0.67f}, 1.4f);
  DrawToneRect({base.x + w * 0.2f, top, w * 0.3f + 6 * s, h}, COMIC_INK, 0.15f,
               0.35f, TONE_LINES, 4);
  InkCurve({base.x - w * 0.2f, top + 10 * s},
           {base.x - w * 0.24f, top + h * 0.5f},
           {base.x - w * 0.16f, base.y - 8 * s}, 1.0f); // a fold
  if (has && has[MALLIPOO]) // jasmine draped over the edge
    for (int i = 0; i < 9; i++) {
      const float t = i / 8.0f;
      ComicEllipse({base.x - w * 0.42f + t * w * 0.5f,
                    top + 4 + 16 * s * std::sin(PI * t) +
                        std::sin(time * 2 + i) * 0.6f},
                   4 * s, 3 * s, JASMINE, 1);
    }
}

// People a little further away: drawn smaller about their feet
static void DrawAnuAt(float x, float feetY, float s, float time,
                      Vector2 hand = {0.13f, 0.5f},
                      Vector2 look = {0.2f, 0.3f}) {
  PushZoom({x, feetY}, s);
  DrawAnu(x, feetY, time, hand, look);
  PopZoom();
}
static void DrawAmmaAt(float x, float feetY, float s, float time) {
  PushZoom({x, feetY}, s);
  DrawAmma(x, feetY, time, false);
  PopZoom();
}
// Where Anu's right hand is, for `hand` as given to DrawAnu
static Vector2 AnuHand(float x, float feetY, float s, Vector2 hand) {
  return {x + hand.x * 285 * s, feetY - hand.y * 285 * s};
}
// ...and the `hand` that puts it at a point on screen
static Vector2 AnuHandTo(float x, float feetY, float s, Vector2 p) {
  return {(p.x - x) / (285 * s), (feetY - p.y) / (285 * s)};
}

// The list and the bag carry over from one scene to the next
struct Shopping {
  bool bought[ITEM_COUNT] = {};
};
Shopping shopping;

//------------------------------------------------------------------------------
// 1. Setting out. The front step, morning. Fold two cloth bags into the
// third. Anu has the list on her phone; Amma holds out the paper one.
//------------------------------------------------------------------------------
const float SO_ANU_X = 292, SO_ANU_FEET = 650;
const float SO_AMMA_X = 486, SO_AMMA_FEET = 640;
const Vector2 SO_PAPER = {440, 476}; // in Amma's hands

class SettingOutScene : public Scene {
public:
  explicit SettingOutScene(const Step &s) : step(s) {
    shopping = Shopping();
    for (int i = 0; i < 3; i++)
      bags[i].at = {572.0f + i * 62, 468};
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.5f);
    for (Bag &b : bags) {
      if (b.folding)
        b.fold = std::fmin(1, b.fold + dt * 1.8f);
    }
    const bool bagsDone = bags[0].fold >= 1 && bags[1].fold >= 1;
    if (bagsDone && askedAt < 0)
      askedAt = time;
    if (taking)
      take = std::fmin(1, take + dt * 0.9f);
    if (take >= 1)
      afterTake += dt;
    done = afterTake > 1.4f;

    if (!Pressed())
      return;
    const Vector2 m = Mouse();
    for (int i = 0; i < 2; i++) { // the third bag is the one they go into
      if (!bags[i].folding && Dist(m, {bags[i].at.x, bags[i].at.y - 40}) < 44) {
        bags[i].folding = true;
        PlayEffect("page", 0.6f, 0.8f + 0.2f * i);
        return;
      }
    }
    if (askedAt >= 0 && !taking &&
        (Dist(m, SO_PAPER) < 60 || Dist(m, Phone()) < 50)) {
      taking = true;
      PlayEffect("page", 0.7f);
    }
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    if (askedAt >= 0 && take < 0.5f)
      DrawBubble(Said(step, 0, "On the paper."), {560, 250}, {SO_AMMA_X - 6, 352},
                 Clamp01((time - askedAt) * 3) * (1 - take * 2));
    if (time < 4 && bags[0].fold == 0)
      DrawHint({bags[0].at.x, bags[0].at.y - 40}, 40, time);
  }

private:
  struct Bag {
    Vector2 at;
    bool folding = false;
    float fold = 0; // 0 flat, 1 folded and tucked into the third bag
  };
  const Step &step;
  Bag bags[3];
  float time = 0;
  float askedAt = -1;
  bool taking = false;
  float take = 0; // the phone going away and the paper coming over
  float afterTake = 0;
  static SettingOutScene *current;

  Vector2 Phone() const {
    return AnuHand(SO_ANU_X, SO_ANU_FEET, 1, PhoneHand());
  }
  Vector2 PhoneHand() const { return {0.2f, 0.62f}; }

  // A cloth bag lying on the thinnai, folding up into a small square.
  // `fat`: how many folded bags are tucked inside it.
  static void DrawFlatBag(Vector2 at, float fold, int colour, int fat) {
    const Color CLOTH[3] = {{238, 228, 204, 255},
                            {150, 200, 170, 255},
                            {246, 206, 110, 255}};
    const float w = (56 + fat * 6) * (1 - 0.5f * fold),
                h = (62 + fat * 4) * (1 - 0.6f * fold);
    const float top = at.y - h;
    if (fold < 0.5f) // the handles, lying loose above it
      for (int k = -1; k <= 1; k += 2) {
        const Vector2 a = {at.x + k * w * 0.32f, top + 3},
                      c = {at.x + k * w * 0.32f + k * 10, top - 22},
                      b = {at.x + k * w * 0.06f, top + 3};
        InkCurve(a, c, b, 6.5f);
        InkCurve(a, c, b, 3.2f, CLOTH[colour]);
      }
    const Vector2 body[4] = {{at.x - w / 2 + 3, top},
                             {at.x + w / 2 - 3, top},
                             {at.x + w / 2 + 2, at.y},
                             {at.x - w / 2 - 2, at.y}};
    ComicPoly(body, 4, CLOTH[colour], 2.2f);
    DrawToneRect({at.x - w / 2, top + h * 0.5f, w, h * 0.2f},
                 colour == 0 ? BAG_PRINT : Color{150, 40, 50, 255}, 1, 1,
                 TONE_DOTS, 4);
    InkCurve({at.x - w * 0.3f, top + 6}, {at.x - w * 0.22f, top + h * 0.5f},
             {at.x - w * 0.32f, at.y - 4}, 1.0f); // a crease
    if (fat > 0)
      InkCurve({at.x - w * 0.36f, top + 10}, {at.x, top + 2},
               {at.x + w * 0.36f, top + 10}, 1.2f);
    if (fold > 0.3f)
      InkLine({at.x - w / 2 + 4, top + h / 2}, {at.x + w / 2 - 4, top + h / 2},
              1.2f);
  }

  static void DrawPeople(float t) {
    SettingOutScene &s = *current;
    // the cloth bags on the thinnai: two fold up and hop into the third
    for (int i = 2; i >= 0; i--) {
      const Bag &b = s.bags[i];
      if (i == 2) {
        const int fat = (s.bags[0].fold >= 1) + (s.bags[1].fold >= 1);
        DrawFlatBag(b.at, 0, 2, fat);
        continue;
      }
      if (b.fold < 0.6f) {
        DrawFlatBag(b.at, b.fold / 0.6f, i, 0);
      } else { // hopping over
        const float hop = (b.fold - 0.6f) / 0.4f;
        const Vector2 p = Lerp2(b.at, {s.bags[2].at.x, s.bags[2].at.y - 30},
                                hop);
        if (hop < 1)
          DrawFlatBag({p.x, p.y - 60 * std::sin(PI * hop)}, 1, i, 0);
      }
    }
    // Amma, the paper list in her hands
    DrawAmma(SO_AMMA_X, SO_AMMA_FEET, t, false);
    // Anu: the phone up, then away in her pocket; then the paper
    Vector2 hand = s.PhoneHand();
    if (s.take > 0) {
      const float away = Clamp01(s.take * 2.5f);
      const Vector2 pocket = {0.12f, 0.45f};
      const Vector2 reach = AnuHandTo(SO_ANU_X, SO_ANU_FEET, 1, SO_PAPER);
      const Vector2 holding = {0.24f, 0.56f};
      if (s.take < 0.4f)
        hand = Lerp2(hand, pocket, away);
      else if (s.take < 0.75f)
        hand = Lerp2(pocket, reach, Clamp01((s.take - 0.4f) / 0.3f));
      else
        hand = Lerp2(reach, holding, Clamp01((s.take - 0.75f) / 0.25f));
    }
    const Vector2 look = {0.7f, 0.25f};
    DrawAnu(SO_ANU_X, SO_ANU_FEET, t, hand, look);
    const Vector2 h = AnuHand(SO_ANU_X, SO_ANU_FEET, 1, hand);
    if (s.take < 0.4f) { // the phone, its screen a list of its own
      const Vector2 p = {h.x - 6, h.y - 30};
      ComicRect({p.x - 13, p.y, 26, 44}, {40, 40, 52, 255}, 2);
      DrawRectangleRec({p.x - 10, p.y + 4, 20, 34}, {200, 226, 255, 255});
      for (int i = 0; i < 5; i++)
        DrawRectangleRec({p.x - 7, p.y + 8 + i * 6, 12.0f - (i % 2) * 4, 2},
                         {90, 110, 160, 255});
      DrawCircleV({p.x + 8, p.y + 6}, 1.5f, {30, 200, 90, 255});
    }
    // the paper: in Amma's hands, then Anu's
    Vector2 paper = SO_PAPER;
    if (s.take >= 0.4f)
      paper = h;
    if (s.take < 0.75f || true) {
      const float wiggle =
          s.askedAt >= 0 && s.take == 0 ? 2 * std::sin(t * 6) : 0;
      rlPushMatrix();
      rlTranslatef(paper.x, paper.y - 18, 0);
      rlRotatef(-8 + wiggle, 0, 0, 1);
      DrawRectangleRec({-15, -20, 32, 42}, LIST_PAPER);
      InkRect({-15, -20, 32, 42}, 1.4f);
      for (int i = 0; i < 6; i++)
        DrawLineEx({-11, -14.0f + i * 6}, {6.0f - (i % 3) * 4, -14.0f + i * 6},
                   1.2f, BALLPOINT);
      rlPopMatrix();
    }
    if (s.askedAt >= 0 && s.take == 0)
      DrawHint(SO_PAPER, 36, t);
  }
};
SettingOutScene *SettingOutScene::current = nullptr;

//------------------------------------------------------------------------------
// The market street, drawn like the home: brush ink, flat colour, dots
//------------------------------------------------------------------------------
const float STALL_TABLE_Y = 430;
const float AWNING_TOP = STALL_TABLE_Y - 190, AWNING_BOTTOM = STALL_TABLE_Y - 142;
const Color AWNING_COLOURS[][2] = {{{232, 64, 60, 255}, {252, 236, 200, 255}},
                                   {{48, 140, 110, 255}, {252, 236, 200, 255}},
                                   {{240, 170, 40, 255}, {200, 60, 50, 255}},
                                   {{60, 100, 180, 255}, {252, 236, 200, 255}},
                                   {{214, 70, 130, 255}, {250, 220, 120, 255}}};

// A striped canvas awning over a stall, from x0 to x1
static void DrawAwning(float x0, float x1, float top, float bottom, int colours,
                       float time) {
  const Color a = AWNING_COLOURS[colours % 5][0],
              b = AWNING_COLOURS[colours % 5][1];
  const int stripes = 6;
  for (int i = 0; i < stripes; i++) {
    const float sx0 = x0 + (x1 - x0) * i / stripes,
                sx1 = x0 + (x1 - x0) * (i + 1) / stripes;
    const Vector2 q[4] = {{sx0 + 8, top},
                          {sx1 + 8, top},
                          {sx1, bottom},
                          {sx0, bottom}};
    ComicPoly(q, 4, i % 2 ? b : a, 1.6f);
  }
  // a scalloped edge that lifts in the breeze
  for (int i = 0; i < stripes; i++) {
    const float cx = x0 + (x1 - x0) * (i + 0.5f) / stripes;
    const float lift = 1.5f * std::sin(time * 2.4f + i + x0 * 0.01f);
    DrawCircleSector({cx, bottom}, (x1 - x0) / stripes / 2 + 1.5f, 0, 180, 10,
                     COMIC_INK);
    DrawCircleSector({cx, bottom - 0.5f}, (x1 - x0) / stripes / 2 - 1, 0, 180,
                     10, i % 2 ? b : a);
    (void)lift;
  }
}
// The shade under a whole row of awnings, printed in one go
static void DrawAwningShade(float top, float bottom) {
  const Rectangle all = FullScreen();
  DrawToneRect({all.x, top, all.width, (bottom - top) * 0.5f}, COMIC_INK, 0.35f,
               0, TONE_DOTS, 4);
}

// A stall keeper behind a table: a simple comic half-figure. `nod` 0..1.
static void DrawKeeper(float x, float tableY, Color shirt, bool grey,
                       bool moustache, float nod, float time) {
  const float bob = 1.2f * std::sin(time * 1.3f + x) - 4 * nod;
  const Vector2 head = {x, tableY - 62 + bob};
  const Vector2 body[4] = {{x - 30, tableY + 2},
                           {x - 24, tableY - 40 + bob},
                           {x + 24, tableY - 40 + bob},
                           {x + 30, tableY + 2}};
  ComicPoly(body, 4, shirt, 2.2f);
  ComicRect({x - 5, tableY - 46 + bob, 10, 8}, SKIN_DARK, 1.2f); // neck
  ComicCircle(head, 17, SKIN, 2.2f);
  const Color hair = grey ? Color{200, 200, 206, 255} : Color{48, 40, 62, 255};
  DrawCircleSector({head.x, head.y - 2}, 17.5f, 180, 360, 14, hair);
  DrawRing({head.x, head.y}, 16.5f, 18.6f, 180, 360, 14, COMIC_INK);
  // eyes: happy closed arcs; a smile
  InkCurve({head.x - 9, head.y + 1}, {head.x - 6, head.y - 3},
           {head.x - 3, head.y + 1}, 1.6f);
  InkCurve({head.x + 3, head.y + 1}, {head.x + 6, head.y - 3},
           {head.x + 9, head.y + 1}, 1.6f);
  if (moustache)
    InkCurve({head.x - 8, head.y + 8}, {head.x, head.y + 5},
             {head.x + 8, head.y + 8}, 3.2f);
  InkCurve({head.x - 5, head.y + 11}, {head.x, head.y + 14},
           {head.x + 5, head.y + 11}, 1.4f);
  DrawCircleV({head.x, head.y - 7}, 1.8f, KUMKUM_RED); // a dot of kumkum
}

// The street behind the stalls: old shop fronts, signboards, bunting
static void DrawMarketBackdrop(float time) {
  const Rectangle all = FullScreen();
  const float street = AWNING_BOTTOM + 10; // where the shop fronts end
  DrawBandGradient(0, street, {150, 204, 236, 255}, {255, 238, 196, 255});
  DrawToneRect({all.x, 0, all.width, 150}, {60, 120, 200, 255}, 0.45f, 0,
               TONE_DOTS, 7);
  // the temple tower over the rooftops, far off
  for (int i = 0; i < 5; i++) {
    const float w = 90 - i * 15.0f, y = street - 150 - i * 26.0f;
    ComicRect({250 - w / 2, y, w, 26}, {226, 150, 120, 255}, 1.6f);
    for (int k = 0; k < (int)(w / 14); k++)
      DrawRectangleRec({250 - w / 2 + 5 + k * 14, y + 10, 6, 9},
                       {120, 56, 52, 255});
  }
  const Color WALLS[] = {{246, 170, 170, 255},
                         {170, 222, 196, 255},
                         {252, 214, 120, 255},
                         {176, 196, 240, 255},
                         {236, 196, 236, 255}};
  int i = 0;
  for (float x = all.x - 30; x < all.x + all.width; x += 150, i++) {
    const float h = 130 + (i * 37) % 50, top = street - 25 - h;
    ComicRect({x, top, 152, h + 30}, WALLS[i % 5], 2.4f);
    DrawToneRect({x, top, 152, 26}, COMIC_INK, 0.4f, 0, TONE_LINES, 4);
    // windows with green shutters, and a painted signboard
    for (int k = 0; k < 2; k++) {
      const Rectangle win = {x + 22 + k * 64, top + 34, 40, 40};
      ComicRect(win, {60, 50, 62, 255}, 2);
      DrawRectangleRec({win.x + 3, win.y + 3, 12, 34}, {48, 152, 110, 255});
      for (int b = 1; b < 3; b++)
        DrawLineEx({win.x + b * 13.3f, win.y}, {win.x + b * 13.3f, win.y + 40},
                   2, COMIC_INK);
    }
    const Color board = WALLS[(i + 2) % 5];
    ComicRect({x + 14, top + 84, 124, 26}, board, 2);
    for (int k = 0; k < 4; k++) // painted letters, too far off to read
      InkCurve({x + 26 + k * 26.0f, top + 102},
               {x + 34 + k * 26.0f, top + 86},
               {x + 42 + k * 26.0f, top + 100}, 2.4f,
               {150, 30, 40, 255});
  }
  // paper bunting across the street
  for (int row = 0; row < 2; row++) {
    const float y0 = 52.0f + row * 40;
    DrawSplineSegmentBezierQuadratic({all.x, y0}, {360, y0 + 40},
                                     {all.x + all.width, y0}, 1.6f, COMIC_INK);
    const int n = (int)(all.width / 34);
    for (int k = 0; k < n; k++) {
      const float t = (k + 0.5f) / n;
      const float x = all.x + t * all.width;
      const float y = y0 + 40 * 2 * t * (1 - t) * 2 * 0.5f * 2;
      const float flap = 2.5f * std::sin(time * 3 + k);
      const Vector2 tri[3] = {{x - 10, y}, {x + 10, y}, {x + flap, y + 20}};
      ComicPoly(tri, 3, AWNING_COLOURS[(k + row) % 5][k % 2 ? 0 : 1], 1.2f);
    }
  }
}

// The packed-earth street in front of the stalls
static void DrawMarketGround(float y) {
  const Rectangle all = FullScreen();
  DrawBandGradient(y, 720 - y, {206, 166, 120, 255}, {176, 132, 92, 255});
  DrawToneRect({all.x, y, all.width, 720 - y}, {110, 70, 40, 255}, 0.05f,
               0.4f, TONE_DOTS, 5);
  DrawBand(y - 1, 3, COMIC_INK);
  for (int i = 0; i < 26; i++) { // fallen petals and leaves
    const float x = all.x + std::fmod(i * 97.0f, all.width);
    const float py = y + 20 + (i * 53) % (int)(700 - y);
    if (i % 3 == 0)
      DrawCircleV({x, py}, 3, MARIGOLD);
    else if (i % 3 == 1)
      DrawComicLeaf({x, py}, {x + 10, py - 3}, 4, LEAF_LIGHT, 1);
    else
      ComicEllipse({x, py}, 3.5f, 2.4f, {226, 200, 170, 255}, 1.1f);
  }
}

//------------------------------------------------------------------------------
// 2. The market. Five stalls; the list. Tap the stall for a line of the list
// and what you buy goes in the bag, with a pencil tick.
//------------------------------------------------------------------------------
const float STALL_X[5] = {78, 214, 350, 486, 622};
const int STALL_ITEM[5] = {BANANA_LEAVES, TURMERIC, CAMPHOR, OIL, COCONUTS};
const Vector2 MARKET_LIST = {14, 474};
const float MARKET_LIST_SCALE = 0.86f;
const Vector2 MARKET_BAG = {612, 708};

// What each stall sells, laid out on its table; `lift` raises it a little
static void DrawGoods(int item, float x, float lift, float time) {
  const float y = STALL_TABLE_Y - lift;
  switch (item) {
  case BANANA_LEAVES: // a fan of big leaves standing in a bucket
    for (int k = 0; k < 5; k++) {
      const float a = (k - 2) * 0.32f;
      DrawComicLeaf({x, y - 10}, {x + 110 * std::sin(a), y - 10 - 115 * std::cos(a)
                                     + 2 * std::sin(time + k)},
                    34, k % 2 ? LEAF_GREEN : LEAF_LIGHT, 1.8f);
    }
    ComicRect({x - 22, y - 24, 44, 26}, {120, 120, 134, 255}, 2);
    break;
  case TURMERIC: // round trays heaped with turmeric and kumkum
    for (int k = 0; k < 2; k++) {
      const float cx = x - 30 + k * 60;
      const Color c = k == 0 ? TURMERIC_YELLOW : KUMKUM_RED;
      ComicEllipse({cx, y - 6}, 30, 8, {200, 200, 210, 255}, 1.8f);
      const Vector2 cone[3] = {{cx - 24, y - 8}, {cx + 24, y - 8},
                               {cx, y - 52}};
      ComicPoly(cone, 3, c, 1.8f);
      DrawToneRect({cx + 2, y - 50, 22, 42}, COMIC_INK, 0.2f, 0.4f, TONE_DOTS,
                   3);
    }
    ComicRect({x - 10, y - 30, 20, 22}, {244, 230, 200, 255}, 1.4f);
    break;
  case CAMPHOR: // brass lamps, a tray of camphor, bundles of cotton wicks
    for (int k = 0; k < 3; k++) {
      const float lx = x - 40 + k * 40;
      InkLine({lx, AWNING_BOTTOM + 8}, {lx, AWNING_BOTTOM + 32}, 1.4f);
      const float by = AWNING_BOTTOM + 34;
      const Vector2 bowl[3] = {{lx - 12, by}, {lx + 12, by}, {lx, by + 14}};
      ComicPoly(bowl, 3, {244, 188, 56, 255}, 1.4f);
    }
    ComicEllipse({x - 20, y - 6}, 34, 8, {200, 200, 210, 255}, 1.8f);
    for (int k = 0; k < 6; k++)
      ComicRect({x - 46 + (k % 3) * 16.0f, y - 22 - (k / 3) * 9.0f, 13, 10},
                COMIC_PAPER, 1.2f);
    for (int k = 0; k < 7; k++)
      InkLine({x + 28 + k * 3.0f, y - 4}, {x + 22 + k * 4.0f, y - 50}, 2.2f,
              {250, 246, 236, 255});
    InkLine({x + 22, y - 30}, {x + 54, y - 30}, 2.6f, KUMKUM_RED);
    break;
  case OIL: // bottles of lamp oil, a big tin
    ComicRect({x - 54, y - 70, 46, 68}, {200, 200, 210, 255}, 2.2f);
    ComicRect({x - 50, y - 52, 38, 26}, {240, 200, 60, 255}, 1.4f);
    for (int k = 0; k < 4; k++) {
      const float bx = x + 2 + k * 15.0f;
      ComicRect({bx, y - 40, 12, 38}, OIL_AMBER, 1.6f);
      ComicRect({bx + 3, y - 48, 6, 8}, {200, 60, 40, 255}, 1.2f);
      DrawLineEx({bx + 3, y - 36}, {bx + 3, y - 8}, 1.6f, {255, 230, 160, 255});
    }
    break;
  case COCONUTS: // a heap
    for (int row = 0; row < 3; row++)
      for (int k = 0; k < 4 - row; k++) {
        const Vector2 c = {x - 45 + row * 15 + k * 30.0f,
                           y - 14 - row * 24.0f};
        ComicCircle(c, 15, COCONUT_BROWN, 2);
        InkCurve({c.x - 8, c.y - 4}, {c.x - 2, c.y - 10}, {c.x + 6, c.y - 8},
                 1.0f, {90, 52, 30, 255});
      }
    break;
  }
}

class MarketScene : public Scene {
public:
  explicit MarketScene(const Step &) {
    // played on its own (from Chapters), the bag starts empty
    for (int i = 0; i < ITEM_COUNT; i++)
      ticks[i] = shopping.bought[i] ? 1.0f : 0.0f;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.45f);
    hover = -1;
    const Vector2 m = Mouse();
    for (int s = 0; s < 5; s++) {
      if (std::fabs(m.x - STALL_X[s]) < 66 && m.y > AWNING_TOP &&
          m.y < STALL_TABLE_Y + 20)
        hover = s;
      const float want = hover == s ? 1.0f : 0.0f;
      lift[s] += (want - lift[s]) * std::fmin(1, dt * 10);
      nod[s] = std::fmax(0, nod[s] - dt * 2);
      wiggle[s] = std::fmax(0, wiggle[s] - dt * 2);
    }
    for (Flying &f : flying) {
      f.t += dt * 1.6f;
      if (f.t >= 1 && !f.landed) {
        f.landed = true;
        shopping.bought[f.item] = true;
        PlayEffect("pencil", 0.6f);
      }
    }
    for (int i = 0; i < ITEM_COUNT; i++)
      if (shopping.bought[i])
        ticks[i] = std::fmin(1, ticks[i] + dt * 2.5f);
    bool all = true;
    for (int s = 0; s < 5; s++)
      all = all && ticks[STALL_ITEM[s]] >= 1;
    if (all)
      allTime += dt;
    done = allTime > 1.6f;

    if (Pressed() && hover >= 0) {
      const int item = STALL_ITEM[hover];
      if (Taken(item)) {
        wiggle[hover] = 1; // already in the bag
      } else {
        Flying f;
        f.item = item;
        f.from = {STALL_X[hover], STALL_TABLE_Y - 40};
        flying.push_back(f);
        nod[hover] = 1;
        PlayEffect("click", 0.6f);
      }
    }
  }

  void Draw() override {
    DrawMarketBackdrop(time);
    const Rectangle all = FullScreen();
    // stalls beyond the stage on wide screens: just more market
    for (float x = 10 - 136; x > all.x - 140; x -= 136)
      DrawAwning(x - 64, x + 64, AWNING_TOP, AWNING_BOTTOM - 2,
                 (int)(-x / 136) + 2, time);
    for (float x = 622 + 136; x < all.x + all.width + 140; x += 136)
      DrawAwning(x - 64, x + 64, AWNING_TOP, AWNING_BOTTOM - 2,
                 (int)(x / 136) + 1, time);
    const Rectangle back = {all.x, AWNING_BOTTOM - 2, all.width,
                            STALL_TABLE_Y - AWNING_BOTTOM + 8};
    DrawRectangleRec(back, {92, 60, 52, 255});
    DrawToneRect(back, COMIC_INK, 0.55f, 0.15f, TONE_LINES, 4);
    const Color SHIRTS[5] = {{250, 250, 240, 255},
                             {120, 170, 220, 255},
                             {240, 200, 90, 255},
                             {200, 120, 160, 255},
                             {160, 210, 150, 255}};
    for (int s = 0; s < 5; s++)
      DrawKeeper(STALL_X[s] + (s % 2 ? 34 : -36), STALL_TABLE_Y, SHIRTS[s],
                 s % 2 == 0, s != 1, nod[s], time);
    for (int s = 0; s < 5; s++) {
      DrawAwning(STALL_X[s] - 66, STALL_X[s] + 66, AWNING_TOP, AWNING_BOTTOM, s,
                 time);
      for (int k = -1; k <= 1; k += 2) // the poles
        ComicRect({STALL_X[s] + k * 62 - 3, AWNING_BOTTOM - 2, 6,
                   STALL_TABLE_Y - AWNING_BOTTOM + 18},
                  WOOD, 1.4f);
    }
    DrawAwningShade(AWNING_TOP, AWNING_BOTTOM);
    DrawMarketGround(STALL_TABLE_Y + 24);
    for (int s = 0; s < 5; s++) {
      const float shake = wiggle[s] * 4 * std::sin(time * 40);
      if (hover == s && !Taken(STALL_ITEM[s]))
        DrawHint({STALL_X[s], STALL_TABLE_Y - 40}, 70, time);
      if (!Taken(STALL_ITEM[s]) || STALL_ITEM[s] != BANANA_LEAVES)
        DrawGoods(STALL_ITEM[s], STALL_X[s] + shake, lift[s] * 8, time);
      else
        DrawGoods(STALL_ITEM[s], STALL_X[s] + shake, -20, time);
      const Rectangle table = {STALL_X[s] - 62, STALL_TABLE_Y, 124, 22};
      ComicRect(table, WOOD_LIGHT, 2.2f);
      DrawToneRect({table.x, table.y + 10, table.width, 12}, COMIC_INK, 0, 0.5f,
                   TONE_DOTS, 4);
    }

    // Anu and Amma, in the crowd
    DrawAmmaAt(452, 740, 0.84f, time);
    const Vector2 look = hover >= 0 ? Vector2{(STALL_X[hover] - 330) / 300.0f,
                                              0.6f}
                                    : Vector2{0.2f, 0.5f};
    DrawAnuAt(332, 744, 0.84f, time, {0.14f, 0.5f}, look);

    DrawClothBag(MARKET_BAG, 1, shopping.bought, time);
    DrawShoppingList(MARKET_LIST, MARKET_LIST_SCALE, -3, ticks);
    for (const Flying &f : flying) { // bought things on their way to the bag
      if (f.t >= 1)
        continue;
      const Vector2 to = {MARKET_BAG.x, MARKET_BAG.y - 110};
      Vector2 p = Lerp2(f.from, to, f.t);
      p.y -= 120 * std::sin(PI * f.t);
      DrawItemIcon(f.item, p, 2.2f);
    }
    if (time < 5 && flying.empty())
      DrawHint({120, 520}, 110, time, {255, 230, 150, 255});
  }

private:
  struct Flying {
    int item;
    Vector2 from;
    float t = 0;
    bool landed = false;
  };
  std::vector<Flying> flying;
  float ticks[ITEM_COUNT] = {};
  float lift[5] = {}, nod[5] = {}, wiggle[5] = {};
  int hover = -1;
  float time = 0, allTime = 0;

  bool Taken(int item) const {
    for (const Flying &f : flying)
      if (f.item == item)
        return true;
    return shopping.bought[item];
  }
};

// A heap of flowers on the stall's table, made once: where each head lies
struct Heap {
  Vector2 centre;
  float width, height;
  std::vector<Vector3> heads; // x, y, size
  void Make(Vector2 c, float w, float h, int count, int seed) {
    centre = c;
    width = w;
    height = h;
    heads.clear();
    for (int i = 0; i < count; i++) {
      // a mound: wide at the bottom, rounded on top; back rows first
      const float u = std::fmod(std::sin(i * 12.9898f + seed) * 43758.5f, 1.0f);
      const float v = std::fmod(std::sin(i * 78.233f + seed * 3) * 12345.6f, 1.0f);
      const float x = (std::fabs(u) * 2 - 1);
      const float top = h * std::sqrt(std::fmax(0, 1 - x * x));
      heads.push_back({c.x + x * w / 2, c.y - std::fabs(v) * top, 0});
    }
    // draw from the back (top) to the front (bottom)
    for (size_t a = 0; a < heads.size(); a++)
      for (size_t b = a + 1; b < heads.size(); b++)
        if (heads[b].y < heads[a].y)
          std::swap(heads[a], heads[b]);
  }
};

// Strands of flowers hanging from the flower stall's awning, swaying
static void DrawHangingStrands(float y, float time) {
  const Rectangle all = FullScreen();
  int i = 0;
  for (float x = all.x + 20; x < all.x + all.width; x += 46, i++) {
    const int kind = i % 3;
    const float len = 90 + (i * 29) % 70;
    const float sway = 3 * std::sin(time * 1.6f + i * 0.9f);
    for (int k = 0; k < (int)(len / 9); k++) {
      const float t = k * 9 / len;
      const Vector2 p = {x + sway * t, y + k * 9.0f};
      if (kind == 0)
        InkDot(p, 5.2f, k % 2 ? MARIGOLD : Color{255, 200, 50, 255});
      else if (kind == 1)
        ComicEllipse(p, 3.4f, 4.6f, JASMINE, 1.1f);
      else
        InkDot(p, 4.6f, k % 3 ? Color{214, 40, 60, 255} : JASMINE);
    }
  }
}

// Big garlands hung on the stall's back wall, sagging in loops
static void DrawBackGarlands(float time) {
  const Rectangle all = FullScreen();
  int g = 0;
  for (float x = all.x + 10; x < all.x + all.width; x += 150, g++) {
    const float y = 180 + (g % 2) * 26, sag = 110 + (g % 3) * 16;
    const float sway = 2 * std::sin(time * 1.2f + g);
    for (int k = 0; k <= 22; k++) {
      const float t = k / 22.0f;
      const Vector2 p = {x + t * 130 + sway * std::sin(PI * t),
                         y + sag * 4 * t * (1 - t)};
      const Color c = g % 3 == 0   ? (k % 2 ? MARIGOLD : Color{255, 200, 50, 255})
                      : g % 3 == 1 ? (k % 4 == 0 ? JASMINE : Color{200, 30, 60, 255})
                                   : (k % 3 == 0 ? Color{200, 30, 60, 255} : JASMINE);
      InkDot(p, 8.5f, c, 1.4f);
    }
    // a tassel hanging from the middle
    for (int k = 0; k < 4; k++)
      InkDot({x + 65 + sway, y + sag + 12 + k * 13.0f}, 6, MARIGOLD);
    ComicCircle({x + 2, y - 4}, 4, WOOD, 1.2f); // nails
    ComicCircle({x + 128, y - 4}, 4, WOOD, 1.2f);
  }
}

//------------------------------------------------------------------------------
// 3. The flower stall. Two muzham of jasmine: drag the string out along the
// table, past two arm-lengths. Then the seller's one line, and the flowers:
// reach for any of them. The orange ones are a memory.
//------------------------------------------------------------------------------
const float FS_TABLE_Y = 470;   // the front edge of the stall's table
const float FS_COIL_X = 70;     // the jasmine string comes off a coil here
const float MUZHAM = 150;       // an arm's length, elbow to fingertip
const Vector2 FS_BAG = {640, 712};
const Vector2 FS_LIST = {440, 536};
const Vector2 FS_SELLER = {380, 330}; // his head
const Vector2 ARM_FROM = {800, 800};  // Anu's arm comes in from here

class FlowerStallScene : public Scene {
public:
  explicit FlowerStallScene(const Step &s) : step(s) {
    for (int i = 0; i < MALLIPOO; i++) // (from Chapters: the rest is bought)
      shopping.bought[i] = true;
    for (int i = MALLIPOO + 1; i < FLOWERS; i++)
      shopping.bought[i] = true;
    for (int i = 0; i < ITEM_COUNT; i++)
      ticks[i] = shopping.bought[i] ? 1.0f : 0.0f;
    heaps[0].Make({226, 462}, 150, 74, 30, 1); // marigold
    heaps[1].Make({396, 464}, 130, 62, 24, 2); // roses
    heaps[2].Make({574, 460}, 150, 76, 30, 3); // kanakambaram
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.35f);
    const Vector2 m = Mouse();
    nod = std::fmax(0, nod - dt * 2);
    for (float &w : wiggle)
      w = std::fmax(0, w - dt * 2);
    for (int i = 0; i < ITEM_COUNT; i++)
      if (shopping.bought[i])
        ticks[i] = std::fmin(1, ticks[i] + dt * 2.5f);

    if (phase == MEASURING) {
      if (Pressed() && m.y > FS_TABLE_Y - 80 && m.y < FS_TABLE_Y + 70)
        dragging = true;
      if (dragging) {
        const float before = stringEnd;
        stringEnd = std::fmax(FS_COIL_X + 30, std::fmin(FS_COIL_X + 2.4f * MUZHAM,
                                                         m.x));
        for (int k = 1; k <= 2; k++) // past an arm's length: a little knot
          if (before < FS_COIL_X + k * MUZHAM && stringEnd >= FS_COIL_X + k * MUZHAM) {
            PlayEffect("click", 0.6f, 1.2f + 0.2f * k);
            nod = 1;
          }
      }
      if (Released() && dragging) {
        dragging = false;
        if (stringEnd >= FS_COIL_X + 2 * MUZHAM) { // snip: into the bag
          phase = SNIPPED;
          phaseTime = 0;
          PlayEffect("click", 0.8f, 0.7f);
        }
      }
    } else if (phase == SNIPPED) {
      if (phaseTime > 0.7f && !shopping.bought[MALLIPOO]) {
        shopping.bought[MALLIPOO] = true;
        PlayEffect("pencil", 0.6f);
      }
      if (phaseTime > 1.6f) {
        phase = CHOOSING;
        phaseTime = 0;
      }
    } else if (phase == CHOOSING) {
      hover = -1;
      for (int h = 0; h < 3; h++)
        if (std::fabs(m.x - heaps[h].centre.x) < heaps[h].width / 2 + 10 &&
            m.y > heaps[h].centre.y - heaps[h].height - 30 &&
            m.y < FS_TABLE_Y + 10)
          hover = h;
      if (phaseTime > 1.2f && Pressed() && hover >= 0 && reaching < 0) {
        reaching = hover;
        reachTime = 0;
      }
    }
    if (reaching >= 0) {
      reachTime += dt;
      if (reaching != 2 && reachTime > 1.4f) { // not those: the hand comes back
        reaching = -1;
      } else if (reaching != 2 && reachTime > 0.55f && reachTime - dt <= 0.55f) {
        wiggle[reaching] = 1;
      }
      if (reaching == 2 && reachTime > 0.6f && reachTime - dt <= 0.6f)
        PlayEffect("bell", 0.4f, 1.2f);
      if (reaching == 2) // the memory: colour floods in
        colour += (1 - colour) * std::fmin(1, dt * 2);
      done = reaching == 2 && reachTime > 2.4f;
    }
    phaseTime += dt;
  }

  void Draw() override {
    DrawMarketBackdrop(time);
    const Rectangle all = FullScreen();
    // the flower stall fills the view: its awning, strands hanging from it
    DrawRectangleRec({all.x, 60, all.width, FS_TABLE_Y - 60},
                     {96, 52, 60, 255});
    DrawToneRect({all.x, 60, all.width, FS_TABLE_Y - 60}, COMIC_INK, 0.5f,
                 0.1f, TONE_LINES, 4);
    for (float x = all.x - 40; x < all.x + all.width; x += 200)
      DrawAwning(x, x + 200, -10, 70, 4, time);
    DrawAwningShade(-10, 70);
    DrawBackGarlands(time);
    DrawHangingStrands(72, time);

    // the seller, his towel on his shoulder
    PushZoom({FS_SELLER.x, FS_TABLE_Y}, 2.2f);
    DrawKeeper(FS_SELLER.x, FS_TABLE_Y, {246, 246, 236, 255}, true, true,
               nod, time);
    const Vector2 towel[4] = {{FS_SELLER.x + 8, FS_TABLE_Y - 42},
                              {FS_SELLER.x + 22, FS_TABLE_Y - 42},
                              {FS_SELLER.x + 26, FS_TABLE_Y - 6},
                              {FS_SELLER.x + 14, FS_TABLE_Y - 4}};
    ComicPoly(towel, 4, {230, 120, 60, 255}, 1.4f);
    PopZoom();

    // the table, slanted towards us, and its heaps
    const Vector2 board[4] = {{all.x, FS_TABLE_Y - 70},
                              {all.x + all.width, FS_TABLE_Y - 70},
                              {all.x + all.width, FS_TABLE_Y},
                              {all.x, FS_TABLE_Y}};
    ComicPoly(board, 4, {226, 214, 190, 255}, 2);
    DrawComicLeaf({0, FS_TABLE_Y - 14}, {700, FS_TABLE_Y - 50}, 80,
                  {70, 150, 70, 255}, 2); // a banana leaf under the flowers
    DrawJasmineHeap(time);
    for (int h = 0; h < 3; h++)
      DrawHeap(h);
    ComicRect({all.x, FS_TABLE_Y, all.width, 34}, WOOD, 2.6f);
    DrawToneRect({all.x, FS_TABLE_Y + 14, all.width, 20}, COMIC_INK, 0, 0.5f,
                 TONE_DOTS, 4);
    if (phase == MEASURING) // two arm-lengths, notched in the table's edge
      for (int k = 1; k <= 2; k++) {
        const float x = FS_COIL_X + k * MUZHAM;
        const bool passed = stringEnd >= x;
        DrawTriangle({x, FS_TABLE_Y + 12}, {x - 9, FS_TABLE_Y + 30},
                     {x + 9, FS_TABLE_Y + 30},
                     passed ? Color{120, 200, 90, 255} : Faded(COMIC_PAPER, 0.9f));
      }
    DrawMarketGround(FS_TABLE_Y + 34);
    DrawJasmineString();

    // Amma beside Anu, the bag, the list
    DrawAmmaAt(70, 840, 1.0f, time);
    DrawClothBag(FS_BAG, 0.85f, shopping.bought, time);
    DrawShoppingList(FS_LIST, 0.62f, 4, ticks);

    if (phase == MEASURING && !dragging && stringEnd < FS_COIL_X + 2 * MUZHAM) {
      const float slide = std::fmod(time * 0.6f, 1.0f);
      DrawHint({stringEnd + 20 + slide * 100, FS_TABLE_Y + 10}, 30, time);
    }
    if (phase == CHOOSING) {
      DrawBubble(Said(step, 0, "You used to know exactly which flowers you wanted."),
                 {360, 150}, {FS_SELLER.x, FS_SELLER.y - 40},
                 Clamp01(phaseTime * 2) * (reaching == 2 ? 1 - reachTime : 1),
                 20);
      if (hover >= 0 && reaching < 0)
        DrawHint({heaps[hover].centre.x, heaps[hover].centre.y - 20}, 70, time);
      if (phaseTime > 7 && reaching < 0) // the orange ones catch the light
        for (int k = 0; k < 3; k++)
          DrawSparkle({heaps[2].centre.x - 40 + k * 40,
                       heaps[2].centre.y - 40 + 10 * std::sin(time * 2 + k)},
                      6 + 3 * std::sin(time * 4 + k), {255, 250, 220, 255});
    }
    // Anu's hand, reaching
    if (reaching >= 0) {
      const Vector2 target = {heaps[reaching].centre.x,
                              heaps[reaching].centre.y - 24};
      float t;
      if (reaching == 2)
        t = Between(reachTime, 0, 0.6f);
      else
        t = reachTime < 0.6f ? Between(reachTime, 0, 0.5f)
                             : 1 - Between(reachTime, 0.8f, 1.4f) * 1.0f;
      // not those: the hand stops short
      const float reach = reaching == 2 ? 1.0f : 0.92f;
      const Vector2 hand = Lerp2({760, 760}, Lerp2({760, 760}, target, reach), t);
      DrawPovArm(ARM_FROM, hand, 34, {214, 160, 58, 255}, 0.45f,
                 reaching == 2 ? Between(reachTime, 0.6f, 0.9f) : 0);
      if (reaching == 2 && reachTime > 0.6f)
        for (int k = 0; k < 4; k++)
          DrawKanakambaram({hand.x - 30 + k * 14, hand.y - 6 + (k % 2) * 6}, 9,
                           k * 0.7f);
    }
    if (reaching == 2) { // into the memory: a warm white bloom
      BeginBlendMode(BLEND_ADDITIVE);
      DrawRectangleRec(all, Faded({255, 220, 170, 255},
                                  Between(reachTime, 1.0f, 2.4f)));
      EndBlendMode();
    }
  }

private:
  enum Phase { MEASURING, SNIPPED, CHOOSING };
  const Step &step;
  Phase phase = MEASURING;
  float time = 0, phaseTime = 0;
  float stringEnd = FS_COIL_X + 40;
  bool dragging = false;
  float nod = 0;
  float ticks[ITEM_COUNT] = {};
  Heap heaps[3];
  float wiggle[3] = {};
  int hover = -1;
  int reaching = -1; // which heap Anu's hand is going to
  float reachTime = 0;

  void DrawJasmineHeap(float t) {
    const Vector2 c = {FS_COIL_X - 10, FS_TABLE_Y - 40};
    for (int i = 0; i < 34; i++) { // loose buds, heaped
      const float a = i * 2.4f, r = 6 + (i % 7) * 7.0f;
      const Vector2 p = {c.x + r * std::cos(a) * 1.3f,
                         c.y + r * std::sin(a) * 0.45f - (50 - r) * 0.5f};
      ComicEllipse(p, 5.2f, 7, JASMINE, 1.2f);
    }
    // the coil of strung jasmine, its end over the table's edge
    for (int i = 0; i < 34; i++) {
      const float a = i * 0.55f;
      const float r = 8 + i * 0.9f;
      ComicEllipse({FS_COIL_X + r * std::cos(a), FS_TABLE_Y - 14 + r * 0.4f * std::sin(a)},
                   4.6f, 3.8f, JASMINE, 1.1f);
    }
    (void)t;
  }

  void DrawJasmineString() {
    float end = stringEnd;
    Vector2 offset = {0, 0};
    if (phase == SNIPPED) { // the cut length, flying to the bag
      const float t = Clamp01(phaseTime / 0.7f);
      const Vector2 to = {FS_BAG.x - 2 * MUZHAM * 0.5f, FS_BAG.y - 110};
      offset = {(to.x - FS_COIL_X) * t, (to.y - FS_TABLE_Y) * t - 140 * std::sin(PI * t)};
      end = FS_COIL_X + 2 * MUZHAM * (1 - 0.6f * t);
      if (t >= 1)
        return;
    } else if (phase == CHOOSING) {
      return;
    }
    for (float x = FS_COIL_X + 6; x < end; x += 8) {
      const float t = (x - FS_COIL_X) / (end - FS_COIL_X + 1);
      const float sag = 6 * std::sin(t * PI);
      ComicEllipse({x + offset.x, FS_TABLE_Y + 6 + 14 * t + sag + offset.y},
                   5, 4, JASMINE, 1.2f);
    }
  }

  void DrawHeap(int h) {
    const Heap &heap = heaps[h];
    const float lift = (hover == h && reaching < 0) ? 4 : 0;
    const float shake = wiggle[h] * 3 * std::sin(time * 40);
    if (reaching == 2 && h == 2 && reachTime > 0.6f)
      return; // in Anu's hand now
    for (size_t i = 0; i < heap.heads.size(); i++) {
      const Vector3 &p = heap.heads[i];
      const Vector2 c = {p.x + shake, p.y - lift};
      if (h == 0)
        DrawFlower(c, 15, i % 3 ? MARIGOLD : Color{255, 196, 40, 255},
                   {200, 90, 20, 255}, 7, i * 0.5f);
      else if (h == 1)
        DrawFlower(c, 15, {214, 40, 60, 255}, {140, 20, 40, 255}, 5, i * 0.7f);
      else
        DrawKanakambaram(c, 13, i * 0.6f);
    }
  }
};

//------------------------------------------------------------------------------
// 4. Memory: Kanakambaram. The front step, years ago. Little Anu stands very
// still while Amma pins orange flowers in her plaits: drag them from Amma's
// basket to the plaits, one at a time.
//------------------------------------------------------------------------------
const Vector2 KK_ZOOM_AT = {340, 600};
const float KK_ZOOM = 2.2f;
const float KK_ANU_X = 300, KK_ANU_FEET = 668, KK_ANU_H = 190;
const float KK_AMMA_X = 436, KK_AMMA_FEET = 666;
const Vector2 KK_BASKET = {404, 520}; // (unzoomed) in Amma's hands
const int KK_SPOTS = 6;

class KanakambaramScene : public Scene {
public:
  explicit KanakambaramScene(const Step &s) : step(s) {
    for (int i = 0; i < KK_SPOTS; i++) {
      const int side = i % 2 ? 1 : -1;
      const float t = 0.15f + 0.35f * (i / 2);
      // along the plait (art.cpp: from (0.1, 0.84) to (0.13, 0.66))
      spots[i] = {KK_ANU_X + side * (0.1f + 0.03f * t) * KK_ANU_H,
                  KK_ANU_FEET - (0.84f - 0.18f * t) * KK_ANU_H};
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.5f);
    for (float &w : wobble)
      w = std::fmax(0, w - dt * 1.5f);
    const Vector2 m = Mouse();
    const Vector2 basket = Zoomed(KK_BASKET, KK_ZOOM_AT, KK_ZOOM);
    if (placed < KK_SPOTS) {
      if (Pressed() && Dist(m, basket) < 60) {
        holding = true;
        pressAt = m;
      }
      if (holding && Released()) {
        holding = false;
        int best = -1;
        float bestD = 70;
        for (int i = 0; i < KK_SPOTS; i++) {
          const float d = Dist(m, Zoomed(spots[i], KK_ZOOM_AT, KK_ZOOM));
          if (!on[i] && d < bestD) {
            best = i;
            bestD = d;
          }
        }
        if (best < 0 && Dist(m, pressAt) < 12) // a tap: the next free spot
          for (int i = 0; i < KK_SPOTS && best < 0; i++)
            if (!on[i])
              best = i;
        if (best >= 0) {
          on[best] = true;
          placed++;
          wobble[0] = 1;
          PlayEffect("click", 0.5f, 1.3f + 0.1f * placed);
          if (placed == 4 || placed == KK_SPOTS)
            sayAt = time;
        }
      }
    } else if (time - sayAt > 2.2f && ContinuePressed()) {
      done = true;
    }
  }

  void Draw() override {
    current = this;
    PushZoom(KK_ZOOM_AT, KK_ZOOM);
    DrawPlace("front_step", DAY, time, DrawPeople);
    PopZoom();
    const Vector2 m = Mouse();
    if (holding)
      DrawKanakambaram(m, 12, time);
    else if (placed < KK_SPOTS && time > 1) {
      DrawHint(Zoomed(KK_BASKET, KK_ZOOM_AT, KK_ZOOM), 46, time);
      for (int i = 0; i < KK_SPOTS; i++)
        if (!on[i]) { // where the next one would go
          DrawHint(Zoomed(spots[i], KK_ZOOM_AT, KK_ZOOM), 22, time + i,
                   {255, 180, 110, 255});
          break;
        }
    }
    const Vector2 anuHead = Zoomed({KK_ANU_X, KK_ANU_FEET - KK_ANU_H}, KK_ZOOM_AT,
                                   KK_ZOOM);
    const Vector2 ammaHead = Zoomed({KK_AMMA_X, KK_AMMA_FEET - 290}, KK_ZOOM_AT,
                                    KK_ZOOM);
    if (placed >= 4 && placed < KK_SPOTS)
      DrawBubble(Said(step, 0, "More."), {anuHead.x - 120, anuHead.y - 40},
                 {anuHead.x - 30, anuHead.y + 10}, Clamp01((time - sayAt) * 3));
    if (placed == KK_SPOTS) {
      DrawBubble(Said(step, 1, "Any more and you'll fall over."),
                 {360, 90}, ammaHead, Clamp01((time - sayAt) * 3));
      if (time - sayAt > 2.2f)
        DrawGoOn(time);
    }
  }

private:
  const Step &step;
  Vector2 spots[KK_SPOTS];
  bool on[KK_SPOTS] = {};
  int placed = 0;
  bool holding = false;
  Vector2 pressAt = {};
  float time = 0, sayAt = -10;
  float wobble[1] = {};
  static KanakambaramScene *current;

  static void DrawPeople(float t) {
    const KanakambaramScene &s = *current;
    DrawAmma(KK_AMMA_X, KK_AMMA_FEET, t, false);
    // Amma's little cane basket of flower strings
    ComicEllipse({KK_BASKET.x, KK_BASKET.y + 6}, 26, 9, {196, 150, 80, 255}, 1.6f);
    for (int k = 0; k < 5; k++)
      DrawKanakambaram({KK_BASKET.x - 16 + k * 8, KK_BASKET.y - 2 + (k % 2) * 4}, 5,
                       k * 0.8f);
    const Vector2 front[4] = {{KK_BASKET.x - 26, KK_BASKET.y + 6},
                              {KK_BASKET.x + 26, KK_BASKET.y + 6},
                              {KK_BASKET.x + 18, KK_BASKET.y + 22},
                              {KK_BASKET.x - 18, KK_BASKET.y + 22}};
    ComicPoly(front, 4, {214, 168, 96, 255}, 1.6f);
    for (int k = 0; k < 4; k++)
      InkLine({KK_BASKET.x - 20 + k * 12, KK_BASKET.y + 8},
              {KK_BASKET.x - 16 + k * 10, KK_BASKET.y + 20}, 1, {150, 100, 50, 255});
    // little Anu, standing very still (a wobble when one goes in)
    const float sway = 2.5f * s.wobble[0] * std::sin(t * 20);
    DrawLittleAnu(KK_ANU_X + sway, KK_ANU_FEET, t, KK_ANU_H);
    for (int i = 0; i < KK_SPOTS; i++)
      if (s.on[i])
        DrawKanakambaram({s.spots[i].x + sway, s.spots[i].y}, 6.5f, i * 1.3f);
  }
};
KanakambaramScene *KanakambaramScene::current = nullptr;

//------------------------------------------------------------------------------
// 5. The bangle stall. Glass bangles on wooden poles, every colour. The
// first tap grabs the nearest green ones in a hurry; Amma says one thing;
// they go back. Then choose, slowly: one at a time, each one clinks.
//------------------------------------------------------------------------------
const int POLES = 7;
const Color GLASS[POLES] = {{220, 40, 60, 255},  {40, 160, 90, 255},
                            {50, 110, 220, 255}, {240, 190, 50, 255},
                            {140, 70, 190, 255}, {240, 110, 170, 255},
                            {250, 130, 40, 255}};
const int GREEN_POLE = 1;
const float POLE_Y0 = 96, POLE_Y1 = 452; // top and bottom of the poles
const Vector2 BG_ARM_FROM = {840, 660};
const Vector2 BG_HAND = {440, 590};
const float BG_WRIST_W = 30;
const int BG_CHOOSE = 6; // how many she chooses

static float PoleX(int i) { return 196 + i * 80.0f; }

class BangleStallScene : public Scene {
public:
  explicit BangleStallScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.3f);
    shake = std::fmax(0, shake - dt * 1.5f);
    const Vector2 m = Mouse();
    hover = -1;
    for (int i = 0; i < POLES; i++)
      if (std::fabs(m.x - PoleX(i)) < 34 && m.y > POLE_Y0 - 10 && m.y < POLE_Y1 + 10)
        hover = i;
    for (int i = 0; i < POLES; i++)
      lift[i] += ((hover == i ? 1.0f : 0.0f) - lift[i]) * std::fmin(1, dt * 8);
    for (Moving &b : moving)
      b.t = std::fmin(1, b.t + dt * (hurry ? 3.0f : 1.6f));
    // bangles that have arrived clink once
    for (Moving &b : moving)
      if (b.t >= 1 && !b.clinked) {
        b.clinked = true;
        Clink();
      }

    if (phase == FIRST) {
      if (Pressed() && hover >= 0) { // grab the nearest green ones
        hurry = true;
        for (int k = 0; k < 4; k++)
          Add(GREEN_POLE, k * 0.08f);
        phase = HURRIED;
        phaseTime = 0;
      }
    } else if (phase == HURRIED) {
      if (phaseTime > 3.2f) { // put them back, and look properly
        moving.clear();
        returning = 4;
        returnTime = 0;
        hurry = false;
        phase = CHOOSING;
        phaseTime = 0;
        PlayEffect("click", 0.4f, 1.6f);
      }
    } else if (phase == CHOOSING) {
      if (Pressed() && hover >= 0 && Arrived() == (int)moving.size() &&
          (int)moving.size() < BG_CHOOSE) {
        Add(hover, 0);
      } else if (Pressed() && !moving.empty() && Arrived() == (int)moving.size() &&
                 Dist(m, ArmWrist(BG_ARM_FROM, BG_HAND, BG_WRIST_W)) < 60) {
        moving.pop_back(); // not that one after all
        PlayEffect("click", 0.4f, 1.8f);
      }
      if (Arrived() >= BG_CHOOSE) {
        phase = CHOSEN;
        phaseTime = 0;
      }
    } else if (phase == CHOSEN) {
      if (phaseTime > 0.6f && phaseTime - dt <= 0.6f) { // a turn of the wrist
        shake = 1;
        for (int k = 0; k < 3; k++)
          Clink();
      }
      if (phaseTime > 1.8f && ContinuePressed())
        done = true;
    }
    returnTime += dt;
    phaseTime += dt;
  }

  void Draw() override {
    const Rectangle all = FullScreen();
    // the stall: red cloth behind, a lamp, the poles of bangles
    DrawBand(0, 720, {176, 40, 50, 255});
    DrawToneRect(all, COMIC_INK, 0.45f, 0.1f, TONE_LINES, 5);
    for (float x = all.x; x < all.x + all.width; x += 60) // cloth folds
      InkCurve({x, 0}, {x + 14, 260}, {x - 4, 520}, 1.4f, {120, 20, 30, 255});
    for (float x = all.x - 40; x < all.x + all.width; x += 200)
      DrawAwning(x, x + 200, -10, 54, 2, time);
    DrawAwningShade(-10, 54);
    BeginTone(TONE_DOTS, 5); // a hanging bulb's warm light
    DrawCircleGradient({360, 60}, 300, {255, 220, 120, 160}, {255, 220, 120, 0});
    EndTone();
    InkLine({360, 40}, {360, 66}, 1.4f);
    ComicCircle({360, 74}, 9, {255, 246, 200, 255}, 1.6f);
    // the rack
    ComicRect({PoleX(0) - 50, POLE_Y1, PoleX(POLES - 1) - PoleX(0) + 100, 24},
              WOOD, 2.4f);
    for (int i = 0; i < POLES; i++) {
      ComicRect({PoleX(i) - 4, POLE_Y0, 8, POLE_Y1 - POLE_Y0}, WOOD_LIGHT, 1.6f);
      const int count = OnPole(i);
      for (int k = 0; k < count; k++) {
        float y = POLE_Y1 - 10 - k * 15.0f;
        if (k == count - 1)
          y -= lift[i] * 16;
        DrawBangle({PoleX(i), y}, 34, 8, GLASS[i]);
      }
      if (hover == i && phase != HURRIED && phase != CHOSEN)
        DrawSparkle({PoleX(i) + 24, POLE_Y1 - 10 - (count - 1) * 15.0f -
                                        lift[i] * 16 - 4},
                    7 + 2 * std::sin(time * 6), {255, 255, 240, 255});
    }
    // the counter in front, a mirror on it
    ComicRect({all.x, 500, all.width, 260}, {120, 74, 44, 255}, 3);
    DrawToneRect({all.x, 500, all.width, 260}, COMIC_INK, 0.1f, 0.6f,
                 TONE_DOTS, 5);
    DrawBand(500, 14, {160, 100, 60, 255});
    DrawBand(499, 2.5f, COMIC_INK);
    for (int k = 0; k < 9; k++) // loose bangles in a glass-topped tray
      DrawBangle({560.0f + (k % 3) * 40, 560.0f + (k / 3) * 26}, 18, 5,
                 GLASS[(k * 3) % POLES]);
    InkRect({530, 540, 140, 84}, 2.2f);
    ComicEllipse({90, 450}, 36, 48, {200, 220, 236, 255}, 2.4f); // a mirror
    ComicRect({84, 496, 12, 8}, WOOD, 1.4f);
    InkLine({72, 432}, {84, 416}, 2, {255, 255, 255, 200});

    // Amma, beside Anu
    DrawAmmaAt(10, 790, 1.0f, time);

    // Anu's arm, the bangles she has on
    const Vector2 hand = {BG_HAND.x + 6 * shake * std::sin(time * 30), BG_HAND.y};
    Color colours[BG_CHOOSE + 4];
    float slide[BG_CHOOSE + 4];
    int count = 0;
    for (const Moving &b : moving)
      if (b.t >= 1) {
        colours[count] = GLASS[b.pole];
        slide[count] = 2 * shake * std::sin(time * 25 + count);
        count++;
      }
    DrawArmBangles(BG_ARM_FROM, hand, BG_WRIST_W, colours, slide, count, -1);
    DrawPovArm(BG_ARM_FROM, hand, BG_WRIST_W, {214, 160, 58, 255}, 0.35f);
    DrawArmBangles(BG_ARM_FROM, hand, BG_WRIST_W, colours, slide, count, 1);
    for (const Moving &b : moving) // on their way, over the hand
      if (b.t < 1) {
        const Vector2 from = {PoleX(b.pole), POLE_Y1 - 10 - OnPole(b.pole) * 15.0f};
        const Vector2 to = ArmWrist(BG_ARM_FROM, hand, BG_WRIST_W);
        Vector2 p = Lerp2(from, to, Between(b.t, 0, 1));
        p.y -= 50 * std::sin(PI * b.t);
        const float turn = Between(b.t, 0.5f, 1);
        DrawBangle(p, 34 - 25 * turn, 8 + 18 * turn, GLASS[b.pole]);
      }
    if (returning > 0 && returnTime < 0.8f) // the green ones going back
      for (int k = 0; k < returning; k++) {
        const float t = Between(returnTime, k * 0.08f, 0.6f + k * 0.08f);
        const Vector2 to = {PoleX(GREEN_POLE), POLE_Y1 - 10 - (OnPole(GREEN_POLE) + k) * 15.0f};
        Vector2 p = Lerp2(ArmWrist(BG_ARM_FROM, hand, BG_WRIST_W), to, t);
        p.y -= 50 * std::sin(PI * t);
        DrawBangle(p, 9 + 25 * t, 26 - 18 * t, GLASS[GREEN_POLE]);
      }

    const Vector2 ammaHead = {10, 790 - 290};
    if (phase == HURRIED || (phase == CHOOSING && phaseTime < 3))
      DrawBubble(Said(step, 0, "You used to take so long to choose."),
                 {300, 400}, {ammaHead.x + 40, ammaHead.y - 10},
                 Clamp01((phase == HURRIED ? phaseTime - 0.5f : 3 - phaseTime) * 3),
                 20);
    if (phase == FIRST && time > 1.2f)
      DrawHint({PoleX(3), 300}, 120, time);
    if (phase == CHOSEN && phaseTime > 1.8f)
      DrawGoOn(time);
  }

private:
  enum Phase { FIRST, HURRIED, CHOOSING, CHOSEN };
  struct Moving {
    int pole;
    float t; // 0 on the pole .. 1 on the wrist
    bool clinked = false;
  };
  const Step &step;
  Phase phase = FIRST;
  std::vector<Moving> moving; // chosen bangles, in the order chosen
  float time = 0, phaseTime = 0, returnTime = 10;
  int returning = 0;
  bool hurry = false;
  float shake = 0;
  int hover = -1;
  float lift[POLES] = {};

  void Add(int pole, float delay) {
    Moving b;
    b.pole = pole;
    b.t = -delay * 3;
    moving.push_back(b);
  }
  int Arrived() const {
    int n = 0;
    for (const Moving &b : moving)
      n += b.t >= 1;
    return n;
  }
  int OnPole(int pole) const { // how many are still on a pole
    int n = 12 + (pole * 5) % 4;
    for (const Moving &b : moving)
      n -= b.pole == pole;
    if (pole == GREEN_POLE && returnTime < 0.8f)
      n -= returning;
    return n;
  }
  void Clink() {
    PlayEffect("bell", 0.25f, 2.2f + 0.3f * (GetRandomValue(0, 10) / 10.0f));
  }
};

//------------------------------------------------------------------------------
// 6. Memory: Glass bangles. A small wrist, six bangles, one of every colour.
// Shake them (drag back and forth): they sound like rain, and then it rains.
//------------------------------------------------------------------------------
const Vector2 RN_ARM_FROM = {-110, 880};
const Vector2 RN_HAND = {320, 540};
const float RN_WRIST_W = 30;
const int RN_SHAKES = 16; // shakes until it really rains

class RainScene : public Scene {
public:
  explicit RainScene(const Step &s) : step(s) {
    for (int i = 0; i < 160; i++)
      drops.push_back({(float)GetRandomValue(-300, 1000),
                       (float)GetRandomValue(-720, 720)});
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience(rain > 0.4f ? "wind" : "birds_day", 0.3f + 0.2f * rain);
    const Vector2 m = Mouse();
    const float dx = m.x - lastX;
    lastX = m.x;
    // shaking: the hand follows the mouse side to side while held
    if (Held() || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      offset += dx * 0.6f;
      if (dx * direction < -0.5f) { // turned back: a shake, if it went far
        if (run > 30) {
          shakes++;
          Clink();
        }
        run = 0;
      }
      if (std::fabs(dx) > 0.5f)
        direction = dx > 0 ? 1.0f : -1.0f;
      run += std::fabs(dx);
    }
    offset *= 1 - std::fmin(1, dt * 6); // and springs back
    offset = std::fmax(-60, std::fmin(60, offset));
    for (int k = 0; k < 6; k++) // the bangles lag behind the wrist
      slide[k] += ((-offset * 0.15f * (k + 1) / 6) - slide[k]) * std::fmin(1, dt * 12);
    rain += ((float)shakes / RN_SHAKES - rain) * std::fmin(1, dt * 1.5f);
    rain = std::fmin(1, rain);
    if (rain > 0.55f && saidAt < 0)
      saidAt = time;
    for (Vector2 &d : drops) {
      d.y += 900 * dt;
      d.x -= 90 * dt;
      if (d.y > 720) {
        d.y -= 760;
        d.x = (float)GetRandomValue(-300, 1000);
      }
    }
    done = saidAt >= 0 && time - saidAt > 2.5f && ContinuePressed();
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    const Rectangle all = FullScreen();
    // the sky clouds over, and the rain comes
    DrawRectangleRec(all, Faded({70, 90, 130, 255}, 0.35f * rain));
    BeginTone(TONE_DOTS, 6);
    DrawRectangleGradientV((int)all.x, 0, (int)all.width, 220,
                           Faded({40, 50, 90, 255}, 0.7f * rain),
                           Faded({40, 50, 90, 255}, 0));
    EndTone();
    const int shown = (int)(drops.size() * rain);
    for (int i = 0; i < shown; i++) {
      const Vector2 d = drops[i];
      DrawLineEx(d, {d.x - 5, d.y + 22}, 2, Faded({220, 236, 255, 255}, 0.8f));
    }
    for (int i = 0; i < shown / 4; i++) { // rings in the puddles on the street
      const float t = std::fmod(time * 1.3f + i * 0.37f, 1.0f);
      const Vector2 c = {all.x + std::fmod(i * 131.0f, all.width), 600.0f + (i * 47) % 110};
      DrawEllipseArc(c, 4 + 14 * t, 1.5f + 4 * t, 0, 2 * PI, 1.4f,
                     Faded({230, 240, 255, 255}, 1 - t), 12);
    }
    // the small wrist and its bangles
    const Vector2 hand = {RN_HAND.x + offset, RN_HAND.y + offset * 0.1f};
    const Color colours[6] = {GLASS[0], GLASS[1], GLASS[2],
                              GLASS[3], GLASS[4], GLASS[6]};
    DrawArmBangles(RN_ARM_FROM, hand, RN_WRIST_W, colours, slide, 6, -1);
    DrawPovArm(RN_ARM_FROM, hand, RN_WRIST_W, {120, 60, 160, 255}, 0.25f);
    DrawArmBangles(RN_ARM_FROM, hand, RN_WRIST_W, colours, slide, 6, 1);
    if (shakes < 2 && time > 1.5f) { // which way to shake
      const float s = 20 * std::sin(time * 5);
      for (int k = -1; k <= 1; k += 2)
        FillTriangle({RN_HAND.x + k * 70 + s, RN_HAND.y - 70},
                     {RN_HAND.x + k * 56 + s, RN_HAND.y - 80},
                     {RN_HAND.x + k * 56 + s, RN_HAND.y - 60},
                     Faded(COMIC_PAPER, 0.8f));
    }
    if (saidAt >= 0)
      DrawBubble(Said(step, 0, "Listen. They sound like rain."), {420, 420},
                 {RN_HAND.x + 40, RN_HAND.y - 10}, Clamp01((time - saidAt) * 3));
    if (saidAt >= 0 && time - saidAt > 2.5f)
      DrawGoOn(time);
  }

private:
  const Step &step;
  std::vector<Vector2> drops;
  float time = 0, rain = 0, saidAt = -1;
  float lastX = 0, offset = 0, run = 0, direction = 1;
  float slide[6] = {};
  int shakes = 0;
  static RainScene *current;

  void Clink() {
    PlayEffect(shakes % 2 ? "bell" : "click", 0.3f,
               2.0f + GetRandomValue(0, 10) / 10.0f);
  }
  static void DrawPeople(float t) {
    // Amma on the step, watching; looking up at the sky
    DrawAmma(500, 600, t, false);
    (void)current;
  }
};
RainScene *RainScene::current = nullptr;

//------------------------------------------------------------------------------
// 7. Coming home. Unpack the bag: each thing out onto the thinnai, its line
// on the list lit. Everything but the last line, the little flowers.
//------------------------------------------------------------------------------
const int UNPACK_ORDER[6] = {BANANA_LEAVES, TURMERIC, CAMPHOR, OIL, COCONUTS,
                             MALLIPOO};
const Vector2 CH_BAG = {640, 470};
const Vector2 CH_LIST = {22, 250};

class ComingHomeScene : public Scene {
public:
  explicit ComingHomeScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.45f);
    for (float &g : glow)
      g = std::fmax(0, g - dt * 0.6f);
    flop = std::fmax(0, flop - dt * 1.5f);
    const bool onBag = Dist(Mouse(), {CH_BAG.x, CH_BAG.y - 50}) < 70;
    if (out < 6) {
      if (Pressed() && onBag && time - lastOut > 0.35f) {
        glow[UNPACK_ORDER[out]] = 1;
        out++;
        lastOut = time;
        PlayEffect("page", 0.4f, 1.4f);
      }
    } else if (!empty) {
      if (Pressed() && onBag && time - lastOut > 0.35f) { // nothing left
        empty = true;
        flop = 1;
        glow[FLOWERS] = 1.6f;
        line = 0;
        lineAt = time;
        PlayEffect("page", 0.5f, 0.7f);
      }
    } else if (time - lineAt > 1.2f && ContinuePressed()) {
      if (line < 1) {
        line++;
        lineAt = time;
      } else {
        done = true;
      }
    }
  }

  void Draw() override {
    current = this;
    DrawPlace("front_step", DAY, time, DrawPeople);
    float ticks[ITEM_COUNT];
    for (int i = 0; i < ITEM_COUNT; i++)
      ticks[i] = i == FLOWERS ? 0.0f : 1.0f;
    DrawShoppingList(CH_LIST, 0.95f, -4, ticks, glow);
    if (out == 0 && time > 1)
      DrawHint({CH_BAG.x, CH_BAG.y - 50}, 60, time);
    if (empty) {
      if (line == 0)
        DrawBubble(Said(step, 0, "We didn't get the flowers."), {360, 150},
                   {270, 360}, Clamp01((time - lineAt) * 3));
      else
        DrawBubble(Said(step, 1, "Not those flowers."), {520, 170}, {480, 340},
                   Clamp01((time - lineAt) * 3));
      if (time - lineAt > 1.2f)
        DrawGoOn(time);
    }
  }

private:
  const Step &step;
  float time = 0, lastOut = -1, lineAt = 0, flop = 0;
  float glow[ITEM_COUNT] = {};
  int out = 0, line = -1;
  bool empty = false;
  static ComingHomeScene *current;

  static void DrawPeople(float t) {
    const ComingHomeScene &s = *current;
    // what's come out of the bag, along the thinnai
    for (int k = 0; k < s.out; k++) {
      const float age = Clamp01((t - s.lastOut) / 0.35f);
      const float landing = k == s.out - 1 ? age : 1;
      const Vector2 to = {476.0f + k * 22, 456};
      Vector2 p = Lerp2({CH_BAG.x, CH_BAG.y - 80}, to, landing);
      p.y -= 60 * std::sin(PI * landing);
      DrawItemIcon(UNPACK_ORDER[k], p, 1.6f);
    }
    bool has[ITEM_COUNT] = {};
    for (int k = s.out; k < 6; k++)
      has[UNPACK_ORDER[k]] = true;
    PushZoom(CH_BAG, 1 + 0.08f * std::sin(s.flop * 12) * s.flop);
    DrawClothBag(CH_BAG, 0.8f, has, t);
    PopZoom();
    DrawAnu(286, 650, t, {0.14f, 0.5f}, {0.6f, 0.2f});
    DrawAmma(440, 640, t, false);
  }
};
ComingHomeScene *ComingHomeScene::current = nullptr;

//------------------------------------------------------------------------------
// 8. The terrace, at night. Anu, the bangles. Shake them if you like.
//------------------------------------------------------------------------------
class TerraceBanglesScene : public Scene {
public:
  explicit TerraceBanglesScene(const Step &) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crickets", 0.5f);
    const Vector2 m = Mouse();
    const float dx = m.x - lastX;
    lastX = m.x;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && time > 0.5f) {
      jiggle += dx * 0.004f;
      if (dx * direction < -0.5f && std::fabs(dx) > 2) {
        clinks++;
        glint = 1;
        PlayEffect("bell", 0.2f, 2.2f + GetRandomValue(0, 10) / 12.0f);
      }
      if (std::fabs(dx) > 0.5f)
        direction = dx > 0 ? 1.0f : -1.0f;
    }
    jiggle *= 1 - std::fmin(1, dt * 5);
    jiggle = std::fmax(-0.06f, std::fmin(0.06f, jiggle));
    glint = std::fmax(0, glint - dt * 2);
    done = (clinks >= 4 || time > 9) && time > 3 && ContinuePressed();
  }

  void Draw() override {
    current = this;
    DrawPlace("terrace", NIGHT, time, DrawPeople);
    const Vector2 w = Wrist();
    if (glint > 0)
      for (int k = 0; k < 3; k++)
        DrawSparkle({w.x - 8 + k * 8, w.y - 10 + k * 7}, 7 * glint,
                    {255, 250, 220, 255});
    if (time < 4)
      DrawHint(w, 30, time);
    if ((clinks >= 4 || time > 9) && time > 3)
      DrawGoOn(time);
  }

private:
  float time = 0, lastX = 0, jiggle = 0, direction = 1, glint = 0;
  int clinks = 0;
  static TerraceBanglesScene *current;

  Vector2 Hand() const {
    return {0.2f + jiggle, 0.74f + 0.01f * std::sin(time * 0.8f)};
  }
  Vector2 Wrist() const { return AnuHand(360, 660, 1, Hand()); }

  static void DrawPeople(float t) {
    const TerraceBanglesScene &s = *current;
    DrawAnu(360, 660, t, s.Hand(), {0.5f, -0.1f});
    const Vector2 w = s.Wrist();
    const Color colours[3] = {GLASS[3], GLASS[2], GLASS[5]};
    for (int k = 0; k < 3; k++)
      DrawBangle({w.x - 2 + k * 3.5f + s.jiggle * 60, w.y + 8 + k * 3}, 3.2f, 7.5f,
                 colours[k]);
  }
};
TerraceBanglesScene *TerraceBanglesScene::current = nullptr;

template <class T> std::unique_ptr<Scene> Make(const Step &step) {
  return std::unique_ptr<Scene>(new T(step));
}

std::unique_ptr<Scene> MakeSettingOut(const Step &step) {
  return std::unique_ptr<Scene>(new SettingOutScene(step));
}
std::unique_ptr<Scene> MakeMarket(const Step &step) {
  return std::unique_ptr<Scene>(new MarketScene(step));
}

} // namespace

PLAY_SCENE("list_setting_out", MakeSettingOut)
PLAY_SCENE("list_market", MakeMarket)
static std::unique_ptr<Scene> (*const MakeFlowerStall)(const Step &) =
    Make<FlowerStallScene>;
static std::unique_ptr<Scene> (*const MakeKanakambaram)(const Step &) =
    Make<KanakambaramScene>;
static std::unique_ptr<Scene> (*const MakeBangleStall)(const Step &) =
    Make<BangleStallScene>;
static std::unique_ptr<Scene> (*const MakeRain)(const Step &) = Make<RainScene>;
static std::unique_ptr<Scene> (*const MakeComingHome)(const Step &) =
    Make<ComingHomeScene>;
static std::unique_ptr<Scene> (*const MakeTerraceBangles)(const Step &) =
    Make<TerraceBanglesScene>;
PLAY_SCENE("list_flowers", MakeFlowerStall)
PLAY_SCENE("list_kanakambaram", MakeKanakambaram)
PLAY_SCENE("list_bangles", MakeBangleStall)
PLAY_SCENE("list_rain", MakeRain)
PLAY_SCENE("list_home", MakeComingHome)
PLAY_SCENE("list_terrace", MakeTerraceBangles)
