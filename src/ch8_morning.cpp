// ch8_morning.cpp - Chapter 8, "Five in the Morning": Pongal morning.
//   flour_at_door  before dawn, Amma at the door holds the rice flour out
//                  this time, instead of bending. Take it (tap it, or drag it
//                  to Anu). No words.
//   (then the doorstep kolam, DoorstepScene in scenes.cpp)
//   pongal_street  the kolam on the step, a flower in its corner; Amma
//                  notices. Then the street at sunrise: hold to fan the fire
//                  under the pongal pot until it boils over, and the pots down
//                  the street boil over after it. "Pongalo pongal!"
#include "finale.h"

using namespace finale;

namespace {

const Color TEAK_8 = {164, 94, 46, 255};
const Color TEAK_DARK_8 = {112, 58, 30, 255};
const Color FLOUR_TIN = {214, 220, 228, 255};
const Color WET_GROUND = {158, 118, 82, 255};

// A small steel tin of rice flour, heaped
void DrawFlourTin(Vector2 c, float s = 1) {
  ComicRect({c.x - 15 * s, c.y - 8 * s, 30 * s, 22 * s}, FLOUR_TIN, 2);
  DrawRectangleRec({c.x - 15 * s, c.y - 2 * s, 30 * s, 3 * s},
                   {170, 178, 190, 255});
  DrawEllipse((int)c.x, (int)(c.y - 8 * s), 15 * s, 5 * s, FLOUR);
  DrawCircleSector({c.x, c.y - 8 * s}, 10 * s, 180, 360, 10, FLOUR);
  InkEllipse({c.x, c.y - 8 * s}, 15 * s, 5 * s, 1.6f);
}

//------------------------------------------------------------------------------
// At the door: the rice flour, held out
//------------------------------------------------------------------------------
const Vector2 AMMA_AT_DOOR = {330, 560};
const Vector2 ANU_AT_STEP = {548, 662};
const Vector2 TIN_HELD = {426, 372}; // in Amma's outstretched hand

struct DoorPeople {
  float time = 0;
  float handed = 0;  // 0 = Amma holds it out, 1 = Anu has it
  Vector2 tin = TIN_HELD;
  bool dragging = false;
};
DoorPeople doorPeople; // what DrawDoorPeople draws (DrawPlace takes a plain
                       // function)

// Amma, Anu reaching up from the street, and the tin between them
void DrawDoorPeople(float time) {
  const DoorPeople &p = doorPeople;
  DrawAmma(AMMA_AT_DOOR.x, AMMA_AT_DOOR.y, time, p.handed < 0.5f);
  // Anu faces Amma (drawn mirrored), her hand going up to the tin
  const float reach = p.handed < 0.5f ? Between(p.handed, 0, 0.5f)
                                      : 1 - Between(p.handed, 0.5f, 1);
  const Vector2 rest = {0.13f, 0.5f};
  const Vector2 up = {(ANU_AT_STEP.x - TIN_HELD.x) / 285 - 0.04f,
                      (ANU_AT_STEP.y - TIN_HELD.y) / 285 - 0.04f};
  const Vector2 hand = {rest.x + (up.x - rest.x) * reach,
                        rest.y + (up.y - rest.y) * reach};
  Mirrored(ANU_AT_STEP.x, [&]() {
    DrawAnu(ANU_AT_STEP.x, ANU_AT_STEP.y, time, hand, {0.7f, -0.3f});
  });
  DrawFlourTin(p.tin);
}

class FlourAtDoorScene : public Scene {
public:
  explicit FlourAtDoorScene(const Step &) { doorPeople = DoorPeople(); }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crickets", 0.35f);
    DoorPeople &p = doorPeople;
    const Vector2 m = GetMousePosition();
    if (stage == OFFERED) {
      if (time > 1.2f && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
          std::hypot(m.x - p.tin.x, m.y - p.tin.y) < 60) {
        p.dragging = true;
        grab = {m.x - p.tin.x, m.y - p.tin.y};
        pressedAt = m;
      }
      if (p.dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        p.tin = {m.x - grab.x, m.y - grab.y};
      if (p.dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        p.dragging = false;
        const bool tapped = std::hypot(m.x - pressedAt.x, m.y - pressedAt.y) < 12;
        const bool toAnu = std::fabs(p.tin.x - ANU_AT_STEP.x) < 130 &&
                           p.tin.y > ANU_AT_STEP.y - 300;
        if (tapped || toAnu) {
          stage = TAKING;
          stageTime = 0;
          from = p.tin;
          PlayEffect("pour", 0.25f, 1.6f); // flour shifting in the tin
        }
      }
      if (!p.dragging) // back into Amma's hand
        p.tin = {p.tin.x + (TIN_HELD.x - p.tin.x) * std::fmin(1.0f, dt * 10),
                 p.tin.y + (TIN_HELD.y - p.tin.y) * std::fmin(1.0f, dt * 10)};
    } else if (stage == TAKING) { // her hand comes up, takes it, comes down
      stageTime += dt;
      p.handed = Between(stageTime, 0, 1.6f);
      const Vector2 held = {ANU_AT_STEP.x - (0.2f * 285), // (mirrored)
                            ANU_AT_STEP.y - 0.52f * 285};
      const float a = Between(stageTime, 0, 0.6f), b = Between(stageTime, 0.8f, 1.6f);
      const Vector2 meet = {from.x + (TIN_HELD.x - from.x) * a,
                            from.y + (TIN_HELD.y - from.y) * a};
      p.tin = {meet.x + (held.x - meet.x) * b, meet.y + (held.y - meet.y) * b};
      if (stageTime > 3.2f)
        stage = LEAVING, stageTime = 0;
    } else {
      stageTime += dt;
      if (stageTime > 1.2f)
        done = true;
    }
    p.time = time;
  }

  void Draw() override {
    DrawPlace("front_step", NIGHT, time, DrawDoorPeople);
    if (stage == OFFERED && time > 1.2f && !doorPeople.dragging)
      DrawHint(doorPeople.tin, 30, time);
    if (stage == LEAVING) // down to the step, to draw
      FadeScreen({15, 35, 100, 255}, Between(stageTime, 0, 1.1f));
  }

private:
  enum Stage { OFFERED, TAKING, LEAVING };
  Stage stage = OFFERED;
  float time = 0, stageTime = 0;
  Vector2 grab = {0, 0}, pressedAt = {0, 0}, from = {0, 0};
};

//------------------------------------------------------------------------------
// Pongal: the kolam on the step, then the street at sunrise
//------------------------------------------------------------------------------
const Vector2 KOLAM_AT = {362, 620};
const float KOLAM_WIDTH = 330;

float pongalTime = 0;     // for DrawStepPeople
float flowerGlint = 0;    // the flower catching the light

// The doorstep by day: Anu's kolam on the wet patch, Anu on her heels beside
// it, Amma at the door looking down at it
void DrawStepPeople(float time) {
  DrawEllipse((int)KOLAM_AT.x, (int)KOLAM_AT.y - 2, 204, 42, WET_GROUND);
  DrawGroundKolam(KOLAM_AT, KOLAM_WIDTH, 0, flowerGlint);
  DrawAnuSitting(150, 676, time);
  DrawAmma(360, 560, time, false);
}

// One house on the festival street, front on: lime-washed wall, tiled roof,
// a door with mango leaves over it, sugarcane leaning by it
void DrawStreetHouse(float x, float width, Color wall, float time, int seed) {
  const float top = 210, ground = 520;
  const float doorW = 74;
  ComicRect({x, top, width, ground - top}, wall, 3);
  DrawToneRect({x, top, width, 70}, {170, 90, 40, 255}, 0.55f, 0, TONE_DOTS, 5);
  // the roof: clay tiles, overhanging
  ComicRect({x - 10, top - 46, width + 20, 46}, {214, 92, 52, 255}, 3);
  for (int row = 0; row < 2; row++) {
    const float y = top - 30 + row * 16;
    for (float tx = x - 4 + row * 9; tx < x + width + 6; tx += 18) {
      DrawRing({tx, y}, 7.5f, 9.5f, 0, 180, 6, COMIC_INK);
    }
  }
  ComicRect({x - 10, top - 4, width + 20, 8}, TEAK_DARK_8, 2);
  // the door, open a little, warm inside
  const float dx = x + width / 2 - doorW / 2;
  ComicRect({dx - 8, 290, doorW + 16, ground - 290}, TEAK_DARK_8, 3);
  DrawRectangleRec({dx, 300, doorW, ground - 300}, {255, 196, 110, 255});
  ComicRect({dx, 300, doorW * 0.42f, ground - 300}, TEAK_8, 2.4f);
  ComicRect({dx + doorW * 0.58f, 300, doorW * 0.42f, ground - 300}, TEAK_8,
            2.4f);
  // a window beside it, shutters open
  const float wx = seed % 2 ? x + 22 : x + width - 66;
  ComicRect({wx, 312, 44, 56}, {70, 46, 38, 255}, 2.6f);
  for (int i = 1; i < 3; i++)
    InkLine({wx + i * 14.5f, 314}, {wx + i * 14.5f, 366}, 1.6f,
            {150, 100, 60, 255});
  ComicRect({wx - 12, 310, 11, 60}, {48, 152, 110, 255}, 1.8f);
  ComicRect({wx + 45, 310, 11, 60}, {48, 152, 110, 255}, 1.8f);
  // mango leaves over the door
  for (int i = 0; i < 6; i++) {
    const float lx = dx + 4 + i * (doorW - 8) / 5;
    const float sway = 1.5f * std::sin(time * 2 + i + seed);
    DrawComicLeaf({lx, 296}, {lx + sway, 322}, 9,
                  i % 2 ? Color{56, 140, 58, 255} : Color{96, 168, 64, 255},
                  1.2f);
  }
  // sugarcane, two stalks leaning by the door
  for (int k = 0; k < 2; k++) {
    const float sx = seed % 2 ? dx + doorW + 18 + k * 12 : dx - 22 - k * 12;
    const Vector2 a = {sx, ground}, b = {sx + 18 + k * 6, 260.0f + k * 20};
    InkLine(a, b, 7, COMIC_INK);
    InkLine(a, b, 4.5f, {120, 60, 110, 255});
    for (int j = 1; j < 6; j++) { // its joints
      const Vector2 c = {a.x + (b.x - a.x) * j / 6, a.y + (b.y - a.y) * j / 6};
      DrawLineEx({c.x - 4, c.y}, {c.x + 4, c.y - 1}, 2, COMIC_INK);
    }
    DrawComicLeaf(b, {b.x + 30, b.y - 30}, 10, {96, 168, 64, 255}, 1.2f);
    DrawComicLeaf(b, {b.x - 26, b.y - 22}, 9, {56, 140, 58, 255}, 1.2f);
  }
}

// A clay pongal pot on three stones over a fire, turmeric tied round its
// neck. `foam` 0..1 rises in it; above 1 it boils over (`over` seconds ago).
void DrawPongalPot(Vector2 base, float s, float fire, float foam, float over,
                   float time, int seed) {
  // the fire
  for (int i = 0; i < 5; i++) {
    const float flick = 0.7f + 0.3f * std::sin(time * 12 + i * 2 + seed);
    const float h = (16 + 22 * fire) * flick * s;
    const float fx = base.x + (i - 2) * 9 * s;
    FillTriangle({fx - 7 * s, base.y}, {fx + 7 * s, base.y}, {fx, base.y - h},
                 {250, 140, 30, 255});
    FillTriangle({fx - 4 * s, base.y}, {fx + 4 * s, base.y},
                 {fx, base.y - h * 0.6f}, {255, 220, 80, 255});
  }
  for (int i = -1; i <= 1; i++) // three stones
    ComicEllipse({base.x + i * 26 * s, base.y + 2}, 11 * s, 8 * s,
                 {150, 140, 130, 255}, 1.8f);
  // the pot
  const Vector2 c = {base.x, base.y - 40 * s};
  const float r = 34 * s;
  ComicCircle(c, r, {200, 104, 60, 255}, 2.6f);
  DrawToneRect({c.x - r, c.y, r * 2, r}, {120, 50, 30, 255}, 0, 0.55f,
               TONE_DOTS, 4);
  DrawCircleV({c.x - r * 0.4f, c.y - r * 0.35f}, r * 0.16f,
              {240, 170, 120, 255});
  // kolam lines painted on it
  for (int i = 0; i < 3; i++)
    DrawRing({c.x - r * 0.45f + i * r * 0.45f, c.y + r * 0.2f}, r * 0.12f,
             r * 0.18f, 0, 360, 10, FLOUR);
  // the neck, turmeric leaves tied round it
  const Rectangle neck = {c.x - r * 0.55f, c.y - r * 1.15f, r * 1.1f, r * 0.3f};
  ComicRect(neck, {210, 116, 70, 255}, 2.2f);
  DrawComicLeaf({c.x - r * 0.4f, c.y - r * 0.9f}, {c.x - r * 1.1f, c.y - r * 1.9f},
                12 * s, {84, 160, 70, 255}, 1.4f);
  DrawComicLeaf({c.x + r * 0.4f, c.y - r * 0.9f}, {c.x + r * 1.0f, c.y - r * 2.0f},
                12 * s, {56, 140, 58, 255}, 1.4f);
  // the milk: rising, frothing, and over
  const float rim = neck.y;
  const float rise = std::fmin(foam, 1.0f);
  DrawEllipse((int)c.x, (int)(rim + 2), neck.width / 2, 5 * s, FLOUR);
  if (rise > 0.2f) { // froth, a dome over the rim
    const float h = (rise - 0.2f) / 0.8f * 22 * s;
    for (int i = -2; i <= 2; i++)
      DrawCircleV({c.x + i * 7 * s, rim - h * (1 - 0.15f * std::fabs((float)i))},
                  8 * s, FLOUR);
    DrawEllipse((int)c.x, (int)rim, neck.width / 2 + 2, h * 0.6f + 4, FLOUR);
  }
  if (over > 0) { // it runs down the sides, and bubbles fly up
    const float run = std::fmin(1, over * 1.3f);
    for (int side = -1; side <= 1; side += 2) {
      for (int k = 0; k < 3; k++) {
        const float x0 = c.x + side * (neck.width / 2 - 2 - k * 6);
        const float len = run * (r * 1.4f - k * 10 * s);
        const Vector2 a = {x0, rim}, b = {x0 + side * r * 0.5f, rim + len};
        DrawLineEx(a, b, 7 * s, FLOUR);
        DrawCircleV(b, 4.5f * s, FLOUR);
      }
    }
    for (int i = 0; i < 8; i++) {
      const float t = std::fmod(over * 0.9f + i * 0.125f, 1.0f);
      const float a = -PI / 2 + Noise((float)seed, (float)i) * 1.1f;
      const Vector2 p = {c.x + std::cos(a) * t * 70 * s,
                         rim - 14 * s + std::sin(a) * t * 80 * s + t * t * 40 * s};
      DrawCircleV(p, (5 - 3 * t) * s, Faded(FLOUR, 1 - t));
    }
  }
  // steam
  for (int i = 0; i < 3; i++) {
    const float t = std::fmod(time * 0.4f + i / 3.0f + seed * 0.1f, 1.0f);
    DrawCircleV({c.x + 10 * std::sin(t * 5 + i) * s, rim - 20 * s - t * 90 * s},
                (6 + t * 12) * s, Faded(COMIC_PAPER, 0.35f * (1 - t) *
                                                     (0.3f + rise)));
  }
}

// The street's pots, down the street to the right: where, and how big
struct StreetPot {
  float x;
  float s;
};
// (in the order they boil over: next door first, then further away)
const StreetPot STREET_POTS[] = {{110, 0.9f}, {700, 0.9f}, {-120, 0.85f},
                                 {920, 0.8f}};
const Vector2 OUR_POT = {372, 664};
const float HOUSE_WIDTH = 225;
float HouseX(int i) { return -300 + i * HOUSE_WIDTH; }

class PongalScene : public Scene {
public:
  explicit PongalScene(const Step &s) : step(s) {
    for (int i = 0; i < 4; i++)
      neighbourOver[i] = -1;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    pongalTime = time;
    flowerGlint = 0;
    switch (stage) {
    case STEP:
      PlayAmbience("morning", 0.5f);
      flowerGlint = Between(stageTime, 0.8f, 1.6f);
      if (stageTime > 1.6f) {
        talk.Start(Lines(step.lines, 0, 3));
        Next(TALK);
      }
      break;
    case TALK:
      PlayAmbience("morning", 0.5f);
      flowerGlint = 0.6f + 0.4f * std::sin(time * 3);
      if (talk.Update())
        Next(TO_STREET);
      break;
    case TO_STREET:
      if (stageTime > 1.4f)
        Next(FANNING);
      break;
    case FANNING: { // hold: fan the fire, the milk rises
      PlayAmbience("crowd", 0.25f);
      const bool held = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                        IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER);
      fanning = held && stageTime > 0.5f;
      if (fanning) {
        heat = std::fmin(1.05f, heat + dt * 0.34f);
        PlayLoop("gust", 0.25f);
      } else {
        heat = std::fmax(0, heat - dt * 0.12f);
      }
      if (heat >= 1.05f) {
        PlayEffect("temple_bells", 0.6f);
        talk.Start(Lines(step.lines, 3, 4));
        Next(OVER);
      }
      break;
    }
    case OVER: // and then all down the street
      PlayAmbience("crowd", 0.55f);
      over += dt;
      for (int i = 0; i < 4; i++) {
        const float at = 0.9f + i * 0.7f;
        if (neighbourOver[i] < 0 && stageTime > at) {
          neighbourOver[i] = 0;
          PlayEffect("bell", 0.25f, 1.1f + i * 0.08f);
        }
        if (neighbourOver[i] >= 0)
          neighbourOver[i] += dt;
      }
      talk.Update();
      if (stageTime > 5.5f && !talk.active && ContinuePressed())
        done = true;
      if (stageTime > 12)
        done = true;
      break;
    }
  }

  void Draw() override {
    if (stage == STEP || stage == TALK ||
        (stage == TO_STREET && stageTime < 0.7f)) {
      DrawPlace("front_step", DAY, time, DrawStepPeople);
      talk.Draw([](const std::string &who) {
        if (who == "Anu")
          return BubbleSpot{{200, 420}, {160, 520}};
        return BubbleSpot{{480, 160}, {372, 296}};
      });
      if (stage == TO_STREET)
        FadeScreen(COMIC_PAPER, Between(stageTime, 0, 0.7f));
      return;
    }
    DrawStreet();
    talk.Draw([](const std::string &) {
      return BubbleSpot{{560, 320}, {576, 428}};
    });
    if (stage == FANNING && !fanning && heat < 0.5f)
      DrawHint({OUR_POT.x, OUR_POT.y - 10}, 34, time);
    if (stage == TO_STREET)
      FadeScreen(COMIC_PAPER, 1 - Between(stageTime, 0.7f, 1.4f));
  }

private:
  enum Stage { STEP, TALK, TO_STREET, FANNING, OVER };
  const Step &step;
  Conversation talk;
  Stage stage = STEP;
  float time = 0, stageTime = 0;
  float heat = 0;
  bool fanning = false;
  float over = 0;
  float neighbourOver[4];

  void Next(Stage s) {
    stage = s;
    stageTime = 0;
  }

  // The festival street at sunrise, our house nearest, the street going off
  // to the right; a kolam and a pot outside every door
  void DrawStreet() const {
    const Rectangle all = FullScreen();
    // sunrise: pale gold under rosy dots, the sun just up over the hills
    DrawBandGradient(0, 220, {255, 196, 140, 255}, {255, 238, 190, 255});
    BeginTone(TONE_DOTS, 7);
    DrawBandGradient(0, 170, Faded({236, 96, 120, 255}, 0.6f),
                     Faded({236, 96, 120, 255}, 0));
    EndTone();
    const Vector2 sun = {560, 96};
    DrawCircleGradient(sun, 120, {255, 246, 206, 160}, {255, 246, 206, 0});
    ComicCircle(sun, 36, {255, 236, 160, 255}, 2.2f);
    DrawComicHills(124, 200, 64, 3, {156, 164, 226, 255}, {86, 96, 176, 255});
    DrawComicHills(150, 200, 36, 6, {112, 128, 200, 255}, {60, 72, 150, 255});
    DrawComicPalm(all.x + 80, 200, 130, time, {150, 98, 56, 255},
                  {58, 152, 74, 255});
    DrawComicPalm(700, 200, 120, time, {150, 98, 56, 255}, {58, 152, 74, 255});
    // the houses, one after another down the street
    const Color WALLS[] = {{252, 222, 140, 255}, {246, 176, 186, 255},
                           {170, 214, 230, 255}, {200, 226, 150, 255},
                           {252, 222, 140, 255}, {246, 176, 186, 255}};
    for (int i = 0; i < 6; i++)
      DrawStreetHouse(HouseX(i), HOUSE_WIDTH + 1, WALLS[i], time, i);
    // the street: packed earth
    DrawBandGradient(520, 200, {204, 168, 120, 255}, {176, 136, 96, 255});
    DrawBand(519, 3, COMIC_INK);
    DrawToneRect({all.x, 520, all.width, 200}, {110, 70, 40, 255}, 0.05f, 0.4f,
                 TONE_DOTS, 5);
    // a kolam outside every door; ours (with the flower) is in front of the
    // house nearest us
    for (int i = 0; i < 6; i++) {
      const float cx = HouseX(i) + HOUSE_WIDTH / 2;
      if (i == 2) {
        DrawEllipse((int)cx, 568, 124, 30, WET_GROUND);
        DrawGroundKolam({cx, 570}, 210, 0, 0.5f + 0.5f * std::sin(time * 3));
        continue;
      }
      for (int k = 0; k < 3; k++) { // the neighbours' kolams, simpler
        const Vector2 d = {cx - 36 + k * 36, 562};
        DrawCircleV(d, 2, FLOUR);
        DrawEllipseLines((int)d.x, (int)d.y, 13, 7, FLOUR);
        DrawEllipseLines((int)d.x, (int)d.y, 12, 6, FLOUR);
      }
      for (int k = 0; k < 2; k++) {
        const Vector2 d = {cx - 18 + k * 36, 576};
        DrawCircleV(d, 2, FLOUR);
        DrawEllipseLines((int)d.x, (int)d.y, 13, 7, FLOUR);
        DrawEllipseLines((int)d.x, (int)d.y, 12, 6, FLOUR);
      }
    }
    // pots, down the street, boiling over one after another
    for (int i = 0; i < 4; i++) {
      const StreetPot &p = STREET_POTS[i];
      const float o = neighbourOver[i];
      DrawPongalPot({p.x, 652}, p.s, 0.6f, o >= 0 ? 1 : 0.7f, o > 0 ? o : 0,
                    time, i + 1);
    }
    // ours: Amma beside it, Anu by the fire with a palm-leaf fan
    DrawPongalPot(OUR_POT, 1.1f, heat, heat, over, time, 0);
    DrawAmma(574, 714, time, false);
    Mirrored(470, [&]() { DrawAnuSitting(470, 714, time); });
    const float wave = fanning ? std::sin(time * 18) * 0.5f : 0.1f;
    const Vector2 grip = {446, 664};
    const Vector2 fan = {grip.x - 34 * std::cos(-0.3f + wave),
                         grip.y + 34 * std::sin(-0.3f + wave)};
    InkLine(grip, fan, 3, {120, 80, 40, 255});
    ComicEllipse(fan, 20, 14, {214, 180, 100, 255}, 1.8f);
    for (int i = -2; i <= 2; i++)
      InkLine(fan, {fan.x + 18 * std::cos(-0.3f + wave + i * 0.4f),
                    fan.y + 13 * std::sin(-0.3f + wave + i * 0.4f)},
              1, {150, 110, 50, 255});
    // the neighbours call out too
    for (int i = 0; i < 4; i++) {
      const float o = neighbourOver[i];
      if (o <= 0 || o > 2.6f)
        continue;
      const StreetPot &p = STREET_POTS[i];
      const float a = Between(o, 0, 0.3f) * (1 - Between(o, 2.2f, 2.6f));
      DrawBubble("Pongalo pongal!", 99, {p.x, 420.0f - i * 14},
                 {p.x, 520}, false, a, false);
    }
    DrawNewsprint();
  }
};

std::unique_ptr<Scene> MakeFlourAtDoor(const Step &step) {
  return std::unique_ptr<Scene>(new FlourAtDoorScene(step));
}
std::unique_ptr<Scene> MakePongal(const Step &step) {
  return std::unique_ptr<Scene>(new PongalScene(step));
}

} // namespace

PLAY_SCENE("flour_at_door", MakeFlourAtDoor)
PLAY_SCENE("pongal_street", MakePongal)
