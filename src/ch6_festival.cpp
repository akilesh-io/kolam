// ch6_festival.cpp - Chapter 6, "Festival Night": the temple street on
// festival eve.
//
//   festival_tower         the terrace at night: light the last lamp on the
//                          parapet, and the tower lights up; the drums start
//   festival_ring_toss     the festival street with Meena: aim and flick the
//                          rings. Miss, miss, and then...
//   festival_windmills     the paper windmill stall: blow on them, pick one,
//                          and a second for Meena's boy
//   festival_windmill_run  a memory: Appa running with little Anu so the
//                          windmill spins. Hold to run; faster, faster.
//   festival_lamps         at the temple, light the row of lamps with Amma.
//                          "Still the same."
//   festival_book          late, the hall at night: turn the kolam book's
//                          pages to the BIG one, and rest a hand on it
#include <algorithm>
#include <cmath>

#include "streets_kit.h"

#include "rlgl.h" // to draw Appa bigger, and the page turning

using namespace streets;

namespace {

// Something drawn `zoom` times bigger around (x, y): for Appa, who is only
// ever drawn small
void BeginZoom(float x, float y, float zoom) {
  rlPushMatrix();
  rlTranslatef(x, y, 0);
  rlScalef(zoom, zoom, 1);
  rlTranslatef(-x, -y, 0);
}
void EndZoom() { rlPopMatrix(); }

// Lights over everything at night: a blue printing plate over the picture,
// as Chapter 1's nights are printed
void NightPlate(Color plate = {150, 155, 215, 255}) {
  BeginBlendMode(BLEND_MULTIPLIED);
  DrawRectangleRec(FullScreen(), plate);
  EndBlendMode();
}

// A warm pool of light, added on top
void LightPool(Vector2 c, float r, Color warm, float amount) {
  BeginBlendMode(BLEND_ADDITIVE);
  DrawCircleGradient(c, r, Faded(warm, 0.35f * amount), Faded(warm, 0));
  EndBlendMode();
}

//------------------------------------------------------------------------------
// The festival street at night: lit houses, strings of bulbs, the crowd, the
// temple tower lit up at the end of the street
//------------------------------------------------------------------------------
void DrawFestivalStreet(float camera, float time) {
  const Rectangle all = FullScreen();
  DrawStreetSky(430, true, time, camera);
  LightPool({560 - camera * 0.15f, 200}, 340, {255, 180, 90, 255}, 1.3f);
  DrawTempleTower(560 - camera * 0.15f, 360, 1.45f, 1, time);
  DrawFarVillage(430, true, time, camera);
  DrawStreetGround(560, camera, false);
  const float spacing = 260, shift = std::fmod(camera, spacing);
  { // each house keeps its whole-number place (see StablePlace in streets_kit)
    const int base = StablePlace(camera, spacing, all.x);
    int j = 0;
    for (float x = all.x - spacing - shift; x < all.x + all.width + spacing;
         x += spacing, j++) {
      const int i = base + j;
      DrawStreetHouse(x + 10, 560, 236, 240, ((i % 6) + 6) % 6, time, true);
    }
  }
  DrawCrowd(520, camera, time, true);
  NightPlate();
  // the bulbs and paper flags strung across the street
  for (float x = all.x - spacing - shift; x < all.x + all.width + spacing;
       x += spacing) {
    DrawBunting({x, 300}, {x + spacing, 310}, 18, time);
    DrawBulbString({x, 330}, {x + spacing, 340}, 24, time, (int)x);
  }
  // the windows' and doors' light falling out onto the street
  BeginBlendMode(BLEND_ADDITIVE);
  DrawRectangleGradientV((int)all.x, 440, (int)all.width, 200,
                         {255, 170, 70, 0}, {255, 170, 70, 50});
  EndBlendMode();
}

//------------------------------------------------------------------------------
// festival_tower: the terrace at night. Every lamp on the parapet is lit but
// the last. Light it (drag the flame from Anu's lamp to it, or tap it), and
// the whole tower lights up; the drums start.
//------------------------------------------------------------------------------
const int PARAPET_LAMPS = 7;
const float PARAPET_Y = 492;
const Vector2 ANU_LAMP_HAND = {0.3f, 0.6f};
const float TERRACE_ANU_X = 120, TERRACE_ANU_FEET = 712;

float parapetLit[PARAPET_LAMPS];
float terraceTime = 0;

Vector2 ParapetLamp(int i) { return {230.0f + i * 55, PARAPET_Y}; }

Vector2 HandAt(float x, float feet, float height, Vector2 frac) {
  return {x + frac.x * height, feet - frac.y * height};
}

void DrawTerracePeople(float time) {
  for (int i = 0; i < PARAPET_LAMPS; i++)
    DrawAgal(ParapetLamp(i), 1.3f, parapetLit[i], time);
  DrawAnu(TERRACE_ANU_X, TERRACE_ANU_FEET, time, ANU_LAMP_HAND, {0.7f, -0.3f});
  DrawAgal(HandAt(TERRACE_ANU_X, TERRACE_ANU_FEET, 285, ANU_LAMP_HAND), 1.2f, 1,
           time);
}

class TowerScene : public Scene {
public:
  TowerScene() {
    for (int i = 0; i < PARAPET_LAMPS; i++)
      parapetLit[i] = i == 0 ? 0.0f : 1.0f;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    terraceTime = time;
    PlayAmbience(lit ? "crowd" : "crickets", lit ? 0.35f : 0.5f);
    const Vector2 mouse = GetMousePosition();
    const Vector2 last = ParapetLamp(0);
    const Vector2 hand = HandAt(TERRACE_ANU_X, TERRACE_ANU_FEET, 285, ANU_LAMP_HAND);
    if (!lit) {
      // take the flame from her lamp, carry it to the dark one
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
          (Near(mouse, hand, 60) || Near(mouse, last, 50)))
        carrying = true;
      if (carrying && !IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        carrying = false;
      if ((carrying && Near(mouse, last, 34)) ||
          (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Near(mouse, last, 50)) ||
          IsKeyPressed(KEY_SPACE)) {
        lit = true;
        litAt = time;
        carrying = false;
        PlayEffect("temple_bells", 0.8f);
      }
    } else {
      parapetLit[0] = std::fmin(1, parapetLit[0] + dt * 2);
      // the drums: a beat, and another
      const float t = time - litAt;
      const int beat = (int)(t / BEAT);
      if (beat != lastBeat && t > 0.6f) {
        lastBeat = beat;
        PlayEffect("click", 0.25f + 0.2f * (beat % 2), 0.5f + 0.1f * (beat % 3));
      }
      if (t > 6 && ContinuePressed())
        done = true;
    }
  }

  void Draw() override {
    DrawPlace("terrace", NIGHT, time, DrawTerracePeople);
    const float t = lit ? time - litAt : 0;
    // the tower, lit all at once: a glow, and the drums' beat going out
    if (lit) {
      const float up = Between(t, 0, 1.5f);
      LightPool({490, 300}, 260 * up + 40, {255, 190, 100, 255}, 1.6f);
      for (int k = 0; k < 3; k++) {
        const float age = std::fmod(t + k * BEAT, BEAT * 3) / (BEAT * 3);
        if (t < 0.6f + k * BEAT)
          continue;
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRing({490, 300}, 60 + age * 260, 64 + age * 262, 0, 360, 48,
                 Faded({255, 220, 150, 255}, 0.5f * (1 - age)));
        EndBlendMode();
      }
      // every tier's bulbs blaze on, from the bottom up
      for (int i = 0; i < 6; i++) {
        const float w = 100 - i * 14.0f, y = 382 - i * 30.0f;
        if (t < i * 0.18f)
          continue;
        for (float x = 490 - w / 2; x <= 490 + w / 2; x += 7)
          DrawCircleV({x, y + 1}, 2.2f,
                      std::sin(time * 5 + x + i) > -0.5f ? BULB
                                                          : Color{255, 160, 90, 255});
      }
    }
    for (int i = 0; i < PARAPET_LAMPS; i++)
      DrawAgalGlow(ParapetLamp(i), 1.3f, parapetLit[i], time);
    const Vector2 hand = HandAt(TERRACE_ANU_X, TERRACE_ANU_FEET, 285, ANU_LAMP_HAND);
    DrawAgalGlow(hand, 1.2f, 1, time);
    if (carrying) { // the flame on a little wick, following the finger
      const Vector2 m = GetMousePosition();
      DrawAgalGlow({m.x, m.y + 10}, 0.8f, 1, time);
      DrawCircleV(m, 4, {255, 200, 80, 255});
      DrawCircleV(m, 2, {255, 250, 220, 255});
    }
    if (!lit && time > 1.2f)
      DrawHint({ParapetLamp(0).x, ParapetLamp(0).y - 8}, time);
    if (lit && t > 6)
      DrawGoOn(time);
    if (time < 1)
      DrawCurtain(1 - time);
  }

private:
  const float BEAT = 0.5f;
  float time = 0, litAt = 0;
  bool lit = false, carrying = false;
  int lastBeat = -1;

  static bool Near(Vector2 a, Vector2 b, float r) {
    const float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy < r * r;
  }
};

//------------------------------------------------------------------------------
// festival_ring_toss: a ring toss stall on the festival street. Press, drag
// towards the pegs and let go: the ring flies where the arc shows. Anu misses
// twice (she always did), and then the third lands.
//------------------------------------------------------------------------------
struct Peg {
  Vector2 top; // where a ring goes over it
  float scale;
  int prize; // what's beside it: 0 an elephant, 1 a parrot, 2 a soda bottle
};

const Color RING_COLOURS[] = {{230, 60, 60, 255}, {250, 200, 40, 255},
                              {60, 170, 90, 255}};
const Vector2 RING_HAND = {0.22f, 0.6f};
const float TOSS_ANU_X = 70, TOSS_ANU_FEET = 770;
const float TOSS_MEENA_X = 650, TOSS_MEENA_FEET = 780;

// A ring of bamboo, as an ellipse `squash` tall, inked
void DrawTossRing(Vector2 c, float r, float squash, Color colour) {
  Vector2 prev = {};
  for (int pass = 0; pass < 2; pass++) {
    for (int i = 0; i <= 24; i++) {
      const float a = i * 2 * PI / 24;
      const Vector2 p = {c.x + r * std::cos(a), c.y + r * squash * std::sin(a)};
      if (i > 0)
        DrawLineEx(prev, p, pass == 0 ? r * 0.32f + 3 : r * 0.32f,
                   pass == 0 ? COMIC_INK : colour);
      prev = p;
    }
  }
}

// A little clay elephant, painted: the prize
void DrawClayElephant(Vector2 c, float s) {
  const Color clay = {120, 150, 200, 255};
  for (int k = 0; k < 4; k++)
    ComicRect({c.x - 14 * s + k * 8 * s, c.y, 6 * s, 12 * s}, clay, 1.2f);
  ComicEllipse({c.x, c.y}, 20 * s, 13 * s, clay, 1.6f);
  ComicCircle({c.x + 18 * s, c.y - 8 * s}, 10 * s, clay, 1.6f);
  InkCurve({c.x + 25 * s, c.y - 4 * s}, {c.x + 32 * s, c.y + 8 * s},
           {c.x + 26 * s, c.y + 14 * s}, 4 * s, COMIC_INK);
  InkCurve({c.x + 25 * s, c.y - 4 * s}, {c.x + 32 * s, c.y + 8 * s},
           {c.x + 26 * s, c.y + 14 * s}, 2 * s, clay);
  ComicEllipse({c.x + 12 * s, c.y - 8 * s}, 6 * s, 8 * s, {240, 120, 150, 255}, 1.2f);
  DrawCircleV({c.x + 21 * s, c.y - 11 * s}, 1.6f * s, COMIC_INK);
  ComicRect({c.x - 8 * s, c.y - 14 * s, 16 * s, 6 * s}, {240, 190, 50, 255}, 1.2f);
}

void DrawPrize(int prize, Vector2 at, float s, float time) {
  if (prize == 0) {
    DrawClayElephant(at, s);
  } else if (prize == 1) { // a green clay parrot on a perch
    ComicEllipse({at.x, at.y - 4 * s}, 10 * s, 15 * s, {70, 180, 90, 255}, 1.4f);
    ComicCircle({at.x + 2 * s, at.y - 20 * s}, 7 * s, {70, 180, 90, 255}, 1.4f);
    const Vector2 beak[3] = {{at.x + 8 * s, at.y - 22 * s},
                             {at.x + 14 * s, at.y - 17 * s},
                             {at.x + 7 * s, at.y - 15 * s}};
    ComicPoly(beak, 3, {230, 60, 50, 255}, 1.2f);
    DrawCircleV({at.x + 4 * s, at.y - 22 * s}, 1.4f * s, COMIC_INK);
  } else { // a bottle of soda with a marble in its neck
    ComicRect({at.x - 7 * s, at.y - 26 * s, 14 * s, 30 * s}, {170, 220, 210, 255},
              1.4f);
    ComicRect({at.x - 4 * s, at.y - 36 * s, 8 * s, 10 * s}, {170, 220, 210, 255},
              1.2f);
    DrawCircleV({at.x, at.y - 30 * s}, 3 * s, {200, 230, 255, 255});
    DrawLineEx({at.x - 3 * s, at.y - 22 * s}, {at.x - 3 * s, at.y}, 1.5f,
               Faded(WHITE, 0.7f));
  }
  (void)time;
}

class RingTossScene : public Scene {
public:
  explicit RingTossScene(const Step &step) {
    worst = LineAt(step.lines, 0, {"Meena", "You were the worst at this."});
    lucky = LineAt(step.lines, 1, {"Meena", "...Fine. Lucky."});
    for (int r = 0; r < 3; r++) {
      const float y = 452 + r * 44.0f, gap = 84 + r * 18.0f;
      for (int k = -1; k <= 1; k++)
        pegs.push_back({{360 + k * gap, y - 26 - r * 4}, 0.8f + r * 0.12f,
                        (k + 1 + r) % 3});
    }
    talk.Start({worst});
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.45f);
    if (meenaLaughs > 0)
      meenaLaughs -= dt;
    if (stage == TALK_FIRST) {
      if (talk.Update())
        stage = AIMING;
      return;
    }
    if (stage == LUCKY) {
      stageTime += dt;
      if (stageTime > 1.4f && !talk.active && !saidLucky) {
        talk.Start({lucky});
        saidLucky = true;
      }
      if (saidLucky && talk.Update())
        done = true;
      return;
    }
    if (stage == FLYING) {
      UpdateFlight(dt);
      return;
    }
    // aiming: press, drag towards the pegs, let go
    const Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      aiming = true;
      pressAt = mouse;
    }
    if (aiming) {
      aim = AimFrom(mouse);
      if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        aiming = false;
        const float dx = mouse.x - pressAt.x, dy = mouse.y - pressAt.y;
        if (dx * dx + dy * dy > 30 * 30)
          Throw(aim);
      }
    }
    if (IsKeyPressed(KEY_SPACE) && !aiming)
      Throw(pegs[4].top);
  }

  void Draw() override {
    DrawFestivalStreet(0, time);
    DrawStall();
    // rings already on pegs, and the one landed
    for (size_t i = 0; i < pegs.size(); i++)
      if ((int)i == ringedPeg && stage != FLYING)
        DrawTossRing({pegs[i].top.x, pegs[i].top.y + 22 * pegs[i].scale +
                                         wobble * 3 * std::sin(time * 30)},
                     20 * pegs[i].scale, 0.36f, RING_COLOURS[2]);
    DrawPeople();
    if (stage == FLYING)
      DrawFlyingRing();
    else if (stage == AIMING)
      DrawTossRing(Hand(), 18, 0.5f, RING_COLOURS[throws % 3]);
    if (stage == AIMING && aiming)
      DrawAim();
    if (stage == AIMING && !aiming && throws == 0)
      DrawDragHint();
    if (stage == LUCKY && stageTime > 0.2f) { // the prize, handed over
      const float t = Between(stageTime, 0.2f, 1.1f);
      const Vector2 from = {pegs[ringedPeg].top.x + 30, pegs[ringedPeg].top.y + 10};
      const Vector2 to = {Hand().x + 4, Hand().y - 6};
      const Vector2 p = {from.x + (to.x - from.x) * t,
                         from.y + (to.y - from.y) * t - 90 * std::sin(t * PI)};
      DrawClayElephant(p, 0.9f + 0.4f * t);
      DrawConfetti(to, stageTime - 1.1f, 3);
    }
    talk.Draw({TOSS_ANU_X + 10, TOSS_ANU_FEET - 250},
              {TOSS_MEENA_X - 4, TOSS_MEENA_FEET - 240}, time);
    if (time < 1)
      DrawCurtain(1 - time);
  }

private:
  enum Stage { TALK_FIRST, AIMING, FLYING, LUCKY } stage = TALK_FIRST;
  Line worst, lucky;
  Balloons talk;
  std::vector<Peg> pegs;
  float time = 0, stageTime = 0, meenaLaughs = 0;
  bool aiming = false, saidLucky = false;
  Vector2 pressAt = {}, aim = {};
  int throws = 0, ringedPeg = -1;
  // the ring in the air
  Vector2 from = {}, to = {}, bounceTo = {};
  float flight = 0, wobble = 0;
  int outcome = 0; // 0 lands on peg `target`, 1 bounces off, 2 falls short
  int target = 0;

  Vector2 Hand() const {
    return HandAt(TOSS_ANU_X, TOSS_ANU_FEET, 285, RING_HAND);
  }

  // Where the ring will go: the drag, made longer, from Anu's hand
  Vector2 AimFrom(Vector2 mouse) const {
    const Vector2 h = Hand();
    Vector2 a = {h.x + (mouse.x - pressAt.x) * 2.2f,
                 h.y + (mouse.y - pressAt.y) * 2.2f};
    a.x = std::fmax(150, std::fmin(580, a.x));
    a.y = std::fmax(380, std::fmin(560, a.y));
    return a;
  }

  int NearestPeg(Vector2 p, float *distance) const {
    int best = 0;
    float bestD = 1e9f;
    for (size_t i = 0; i < pegs.size(); i++) {
      const float dx = pegs[i].top.x - p.x, dy = pegs[i].top.y - p.y;
      const float d = dx * dx + dy * dy;
      if (d < bestD) {
        bestD = d;
        best = (int)i;
      }
    }
    *distance = std::sqrt(bestD);
    return best;
  }

  void Throw(Vector2 at) {
    float d = 0;
    target = NearestPeg(at, &d);
    from = Hand();
    flight = 0;
    stage = FLYING;
    PlayEffect("gust", 0.3f, 1.6f);
    if (throws == 0) { // she always was the worst at this
      outcome = 1;
      to = {pegs[target].top.x + 14, pegs[target].top.y - 4};
      bounceTo = {to.x + 90, to.y - 10};
    } else if (throws == 1) {
      outcome = 2;
      to = {at.x * 0.7f + 360 * 0.3f, 575};
      bounceTo = {to.x + 30, 760};
    } else if (d < 150) {
      outcome = 0;
      to = pegs[target].top;
    } else {
      outcome = 1;
      to = at;
      bounceTo = {to.x + 70, to.y - 20};
    }
    throws++;
  }

  void UpdateFlight(float dt) {
    flight += dt;
    const float FLY = 0.65f;
    if (flight < FLY)
      return;
    const float after = flight - FLY;
    if (outcome == 0) {
      if (ringedPeg != target) {
        ringedPeg = target;
        wobble = 1;
        PlayEffect("bell", 0.6f, 1.4f);
        PlayEffect("click", 0.6f, 1.2f);
      }
      wobble = std::fmax(0, wobble - dt * 1.5f);
      if (after > 0.5f) {
        stage = LUCKY;
        stageTime = 0;
      }
      return;
    }
    if (after < dt * 1.5f) {
      PlayEffect("click", 0.5f, outcome == 1 ? 1.5f : 0.8f);
      if (throws == 1)
        meenaLaughs = 2.2f;
    }
    if (after > 0.8f)
      stage = AIMING;
  }

  Vector2 RingAt(float *scale, float *squash) const {
    const float FLY = 0.65f;
    const float t = std::fmin(1.0f, flight / FLY);
    const float far = outcome == 2 ? 0.95f : pegs[target].scale;
    *scale = 1 + (far - 1) * t;
    *squash = 0.35f + 0.25f * std::fabs(std::sin(flight * 14));
    if (flight < FLY)
      return {from.x + (to.x - from.x) * t,
              from.y + (to.y - from.y) * t - 190 * std::sin(t * PI)};
    const float after = std::fmin(1.0f, (flight - FLY) / 0.6f);
    if (outcome == 0) {
      *squash = 0.36f;
      return {to.x, to.y + 22 * pegs[target].scale * Between(after, 0, 0.5f)};
    }
    // bounced off, or short: off it goes
    return {to.x + (bounceTo.x - to.x) * after,
            to.y + (bounceTo.y - to.y) * after - 50 * std::sin(after * PI)};
  }

  void DrawFlyingRing() const {
    float scale = 1, squash = 0.5f;
    const Vector2 p = RingAt(&scale, &squash);
    DrawTossRing(p, 18 * scale, squash, RING_COLOURS[(throws - 1) % 3]);
  }

  void DrawAim() const {
    const Vector2 h = Hand();
    for (int i = 1; i < 14; i++) { // the arc, dotted
      const float t = i / 14.0f;
      const Vector2 p = {h.x + (aim.x - h.x) * t,
                         h.y + (aim.y - h.y) * t - 190 * std::sin(t * PI)};
      DrawCircleV(p, 4.5f, Faded(COMIC_INK, 0.6f));
      DrawCircleV(p, 3, Faded(COMIC_PAPER, 0.9f));
    }
    DrawEllipseLines((int)aim.x, (int)aim.y, 24, 9, COMIC_PAPER);
    DrawEllipseLines((int)aim.x, (int)aim.y, 25, 10, COMIC_INK);
  }

  void DrawDragHint() const {
    const Vector2 h = Hand();
    const float slide = std::fmod(time, 1.5f) / 1.5f;
    DrawHint({h.x + 120 * slide, h.y - 90 * slide}, time, 1 - slide * 0.6f);
  }

  // The stall: a red cloth backdrop, a stepped table with pegs and prizes,
  // the stall keeper with his rings on a stick, a string of bulbs
  void DrawStall() const {
    ComicRect({150, 250, 420, 220}, {196, 40, 52, 255}, 3);
    DrawToneRect({150, 250, 420, 220}, {90, 0, 20, 255}, 0.1f, 0.5f, TONE_DOTS, 5);
    for (int k = 0; k < 12; k++) { // the gold border
      DrawCircleV({168 + k * 35.0f, 264}, 7.5f, COMIC_INK);
      DrawCircleV({168 + k * 35.0f, 264}, 6, {244, 192, 64, 255});
    }
    Folk keeper;
    keeper.height = 250;
    keeper.head = 0.08f;
    keeper.hair = Folk::BALD;
    keeper.grey = true;
    keeper.moustache = true;
    keeper.skirt = true;
    keeper.top = {240, 236, 226, 255};
    keeper.bottom = {80, 110, 160, 255};
    keeper.border = {60, 80, 130, 255};
    DrawFolk(keeper, 520, 560, time, {0.2f, 0.75f}, -0.6f, meenaLaughs > 0);
    for (int k = 0; k < 4; k++) // the rings on his stick
      DrawTossRing({520 + 0.2f * 250, 560 - 0.75f * 250 + 18 + k * 9}, 14, 0.3f,
                   RING_COLOURS[k % 3]);
    // the table, three steps, the pegs and prizes on them
    for (int r = 0; r < 3; r++) {
      const float y = 452 + r * 44.0f, w = 300 + r * 70.0f;
      ComicRect({360 - w / 2, y, w, 44}, r % 2 ? Color{236, 200, 120, 255}
                                               : Color{250, 220, 140, 255},
                2.4f);
      DrawToneRect({360 - w / 2, y + 26, w, 18}, {120, 60, 20, 255}, 0, 0.5f,
                   TONE_DOTS, 4);
      for (int k = -1; k <= 1; k++) {
        const Peg &p = pegs[r * 3 + k + 1];
        const float s = p.scale;
        ComicRect({p.top.x - 4 * s, p.top.y, 8 * s, 30 * s}, TEAK, 1.4f);
        ComicCircle(p.top, 6 * s, {214, 150, 90, 255}, 1.4f);
        DrawPrize(p.prize, {p.top.x + 34 * s, p.top.y + 22 * s}, s, time);
      }
    }
    ComicRect({170, 584, 380, 140}, {120, 70, 40, 255}, 3);
    DrawToneRect({176, 590, 368, 130}, COMIC_INK, 0.25f, 0.45f, TONE_LINES, 7);
    DrawBulbString({140, 250}, {580, 250}, 10, time, 2);
  }

  void DrawPeople() const {
    const bool won = stage == LUCKY;
    DrawAnu(TOSS_ANU_X, TOSS_ANU_FEET, time,
            stage == FLYING ? Vector2{0.32f, 0.78f} : RING_HAND, {0.7f, -0.2f});
    if (won && stageTime > 1.1f) // the clay elephant, hers now
      DrawClayElephant({Hand().x + 4, Hand().y - 6}, 1.3f);
    const Folk boy = MeenasBoy(140);
    const float H = 278;
    DrawMeena(TOSS_MEENA_X, TOSS_MEENA_FEET, time,
              meenaLaughs > 0 ? Vector2{0.05f, 0.7f} : Vector2{0.12f, 0.45f},
              {-0.6f, 0.0f});
    DrawFolk(boy, TOSS_MEENA_X + 0.17f * H, TOSS_MEENA_FEET - 0.37f * H, time,
             {0.25f, 0.8f}, -0.7f, meenaLaughs > 0 || won);
    DrawCircleV({TOSS_MEENA_X + 0.2f * H, TOSS_MEENA_FEET - 0.4f * H}, 9,
                PERSON_INK);
    DrawCircleV({TOSS_MEENA_X + 0.2f * H, TOSS_MEENA_FEET - 0.4f * H}, 7.4f, SKIN);
  }
};

//------------------------------------------------------------------------------
// festival_windmills: the paper windmill seller's pole, stuck all over with
// windmills, all turning. Wave across them and they spin faster. Tap one: it's
// Anu's. Tap another: it's for Meena's boy.
//------------------------------------------------------------------------------
const Vector2 BUNDLE = {420, 210};
const float SELLER_X = 380, STALL_FEET = 640;
const float MILLS_ANU_X = 120, MILLS_ANU_FEET = 735;
const float MILLS_MEENA_X = 640, MILLS_MEENA_FEET = 740;
const Vector2 MILL_ANU_HAND = {0.26f, 0.84f};

struct Mill {
  Vector2 at;
  float r, angle, spin;
  int colours;
  int owner = 0; // 0 on the pole, 1 Anu's, 2 the boy's
  float flight = 0;
  Vector2 from = {};
};

class WindmillStallScene : public Scene {
public:
  WindmillStallScene() {
    for (int i = 0; i < 15; i++) { // stuck all round the straw bundle
      const float a = -PI / 2 + (i - 7) * 0.42f;
      const float d = 70 + (i % 3) * 40;
      mills.push_back({{BUNDLE.x + std::cos(a) * d * 1.3f,
                        BUNDLE.y + 30 + std::sin(a) * d * 0.9f + (i % 2) * 40},
                       30 + (i % 3) * 5.0f, i * 0.7f, 2, i});
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.4f);
    const Vector2 mouse = GetMousePosition(), delta = GetMouseDelta();
    const float wave = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    float total = 0;
    for (Mill &m : mills) {
      const Vector2 c = Centre(m);
      const float dx = mouse.x - c.x, dy = mouse.y - c.y;
      const float near = std::fmax(0, 1 - std::sqrt(dx * dx + dy * dy) / 160);
      m.spin += wave * near * 0.9f;
      m.spin = std::fmin(40, m.spin);
      m.spin += (2 - m.spin) * std::fmin(1.0f, dt * 0.8f); // the breeze
      m.angle += m.spin * dt;
      m.flight = std::fmin(1, m.flight + dt * 2);
      total += m.spin;
    }
    PlayLoop("wind", std::fmin(0.6f, total / 200));
    if (bought < 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (int i = (int)mills.size() - 1; i >= 0; i--) {
        Mill &m = mills[i];
        const Vector2 c = Centre(m);
        const float dx = mouse.x - c.x, dy = mouse.y - c.y;
        if (m.owner == 0 && dx * dx + dy * dy < (m.r + 12) * (m.r + 12)) {
          m.from = c;
          m.flight = 0;
          m.owner = ++bought;
          m.spin = 20;
          PlayEffect("click", 0.6f, 1.2f);
          PlayEffect("bell", 0.25f, 2.0f); // the coin in his tin
          if (bought == 2)
            boyHappy = 0;
          break;
        }
      }
    }
    if (IsKeyPressed(KEY_SPACE) && bought < 2)
      for (Mill &m : mills)
        if (m.owner == 0) {
          m.from = Centre(m);
          m.flight = 0;
          m.owner = ++bought;
          break;
        }
    if (bought == 2) {
      boyHappy += dt;
      if (boyHappy > 2.5f && !leaving && ContinuePressed())
        leaving = true;
    }
    if (leaving) {
      leaveTime += dt;
      done = leaveTime > 1.3f;
    }
  }

  void Draw() override {
    DrawFestivalStreet(500, time);
    // the seller and his pole
    InkLine({BUNDLE.x + 6, STALL_FEET - 10}, {BUNDLE.x, BUNDLE.y + 40}, 9,
            {200, 160, 90, 255});
    Folk seller;
    seller.height = 240;
    seller.head = 0.08f;
    seller.hair = Folk::BALD;
    seller.grey = true;
    seller.moustache = true;
    seller.top = {250, 240, 220, 255};
    seller.bottom = {240, 236, 226, 255};
    seller.border = {200, 160, 60, 255};
    DrawFolk(seller, SELLER_X, STALL_FEET, time, {0.17f, 0.45f}, 0.3f,
             bought > 0 && boyHappy < 1.5f);
    ComicEllipse(BUNDLE, 46, 58, {236, 196, 96, 255}, 2.4f);
    BeginTone(TONE_LINES, 5);
    DrawEllipseV(BUNDLE, 44, 56, Faded({120, 70, 20, 255}, 0.4f));
    EndTone();
    for (const Mill &m : mills)
      if (m.owner == 0) {
        DrawWindmillStick(m.at, m.r, BUNDLE);
        DrawWindmill(m.at, m.r, m.angle, m.colours);
      }
    // Anu, Meena and the boy
    const bool anuHas = bought >= 1;
    DrawAnu(MILLS_ANU_X, MILLS_ANU_FEET, time,
            anuHas ? MILL_ANU_HAND : Vector2{0.13f, 0.5f}, {0.6f, -0.4f});
    const float H = 278;
    const bool boyHas = bought >= 2;
    DrawMeena(MILLS_MEENA_X, MILLS_MEENA_FEET, time, {0.12f, 0.45f},
              {-0.5f, 0.0f});
    const Vector2 boyHand = BoyHand();
    DrawFolk(MeenasBoy(140), MILLS_MEENA_X + 0.17f * H,
             MILLS_MEENA_FEET - 0.37f * H, time,
             boyHas ? Vector2{0.05f, 1.05f} : Vector2{0.18f, 0.55f}, -0.6f,
             boyHas);
    DrawCircleV({MILLS_MEENA_X + 0.2f * H, MILLS_MEENA_FEET - 0.4f * H}, 9,
                PERSON_INK);
    DrawCircleV({MILLS_MEENA_X + 0.2f * H, MILLS_MEENA_FEET - 0.4f * H}, 7.4f,
                SKIN);
    // the windmills that have been bought
    for (const Mill &m : mills) {
      if (m.owner == 0)
        continue;
      const Vector2 hand = m.owner == 1
                               ? HandAt(MILLS_ANU_X, MILLS_ANU_FEET, 285, MILL_ANU_HAND)
                               : boyHand;
      const float t = Between(m.flight, 0, 1);
      const Vector2 rest = {hand.x + 6, hand.y - 70};
      const Vector2 c = {m.from.x + (rest.x - m.from.x) * t,
                         m.from.y + (rest.y - m.from.y) * t - 60 * std::sin(t * PI)};
      if (t >= 1)
        DrawWindmillStick(c, m.r, {hand.x, hand.y + 6});
      DrawWindmill(c, m.r * (m.owner == 1 ? 1.2f : 1.0f), m.angle, m.colours);
      if (m.owner == 2)
        DrawConfetti(rest, boyHappy - 0.5f, 7);
    }
    if (bought < 2 && time > 1) { // tap one
      for (size_t i = 7; i < mills.size() + 7; i++) {
        const Mill &m = mills[i % mills.size()];
        if (m.owner == 0) {
          DrawHint(Centre(m), time);
          break;
        }
      }
    }
    if (bought == 2 && boyHappy > 2.5f && !leaving)
      DrawGoOn(time);
    if (time < 1)
      DrawCurtain(1 - time);
    DrawCurtain(Between(leaveTime, 0.1f, 1.3f), {255, 236, 200, 255});
  }

private:
  std::vector<Mill> mills;
  float time = 0, boyHappy = 0, leaveTime = 0;
  int bought = 0;
  bool leaving = false;

  Vector2 BoyHand() const {
    const float H = 278;
    return HandAt(MILLS_MEENA_X + 0.17f * H, MILLS_MEENA_FEET - 0.37f * H, 140,
                  {0.05f, 1.05f});
  }

  static Vector2 Centre(const Mill &m) { return m.at; }
};

//------------------------------------------------------------------------------
// festival_windmill_run: a memory. The street at dusk, years ago: Appa
// running with little Anu so her paper windmill will spin. Hold to run; the
// faster they go, the faster it spins. Keep it spinning flat out for a bit.
//------------------------------------------------------------------------------
const float RUN_FEET = 640;
const float APPA_X = 260, LITTLE_ANU_X = 420;
const float FLAT_OUT_SECONDS = 3;

class WindmillRunScene : public Scene {
public:
  explicit WindmillRunScene(const Step &step) {
    faster = LineAt(step.lines, 0, {"Anu", "Faster!"});
    faster.speaker = "Anu";
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("birds_day", 0.3f);
    says.Update();
    const bool running = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                         IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_RIGHT);
    if (flatOut >= FLAT_OUT_SECONDS) { // done: they slow, laughing
      speed = std::fmax(0.15f, speed - dt * 0.35f);
      endTime += dt;
      if (endTime > 3 && ContinuePressed())
        done = true;
    } else if (running) {
      speed = std::fmin(1, speed + dt * 0.45f);
    } else {
      speed = std::fmax(0, speed - dt * 0.5f);
    }
    if (speed > 0.45f && !saidFaster) {
      saidFaster = true;
      says.Say(faster, 2.6f);
    }
    if (speed > 0.85f)
      flatOut += dt;
    scroll += speed * 520 * dt;
    stride += speed * dt * 14;
    spin += (1.2f + speed * 30) * dt;
    if (speed > 0.1f)
      PlayLoop("wind", 0.15f + 0.4f * speed);
  }

  void Draw() override {
    const Rectangle all = FullScreen();
    // dusk, years ago: an orange sky printed with magenta dots
    DrawBandGradient(0, 470, {255, 170, 100, 255}, {255, 228, 160, 255});
    BeginTone(TONE_DOTS, 7);
    DrawBandGradient(0, 360, Faded({224, 62, 100, 255}, 0.75f),
                     Faded({224, 62, 100, 255}, 0));
    EndTone();
    DrawCircleGradient({560, 330}, 110, {255, 246, 206, 170}, {255, 246, 206, 0});
    ComicCircle({560, 330}, 40, {255, 242, 186, 255}, 2.2f);
    DrawTempleTower(160 - scroll * 0.03f, 390, 1.1f, 0.4f, time);
    DrawFarVillage(440, false, time, scroll);
    DrawStreetGround(560, scroll, false);
    const float spacing = 270, shift = std::fmod(scroll, spacing);
    const int base = StablePlace(scroll, spacing, all.x);
    int j = 0;
    for (float x = all.x - spacing - shift; x < all.x + all.width + spacing;
         x += spacing, j++) {
      const int i = base + j;
      DrawStreetHouse(x + 10, 560, 240, 240, ((i * 5) % 6 + 6) % 6, time, false);
      DrawBunting({x, 290}, {x + spacing, 300}, 16, time);
    }
    // speed lines and dust
    for (int i = 0; i < 8 && speed > 0.3f; i++) {
      const float y = 300 + i * 46.0f;
      const float x = all.x + std::fmod(i * 197.0f - scroll * 2.2f + 100000,
                                        all.width + 200) - 100;
      InkLine({x, y}, {x + 70 * speed, y}, 2.2f, Faded(COMIC_INK, speed - 0.3f));
    }
    for (int k = 0; k < 4 && speed > 0.2f; k++) {
      const float age = std::fmod(time * 3 + k * 0.25f, 1.0f);
      for (float fx : {APPA_X - 10, LITTLE_ANU_X - 20})
        DrawCircleV({fx - age * 80 * speed, RUN_FEET - 8 - age * 20},
                    6 + age * 10, Faded({230, 200, 160, 255}, (1 - age) * speed));
    }
    // Appa, running, cheering her on; little Anu holding the windmill up
    const float bobA = std::fabs(std::sin(stride)) * 14 * speed;
    const float bobL = std::fabs(std::sin(stride + 1.2f)) * 12 * speed;
    BeginZoom(APPA_X, RUN_FEET, 1.95f);
    DrawAppa(APPA_X, RUN_FEET - bobA / 1.95f, time);
    EndZoom();
    const float H = 190;
    DrawLittleAnu(LITTLE_ANU_X, RUN_FEET - bobL, time, H);
    const Vector2 hands = {LITTLE_ANU_X + 0.13f * H, RUN_FEET - bobL - 0.56f * H};
    const Vector2 mill = {hands.x + 40, hands.y - 150};
    DrawWindmillStick(mill, 52, hands);
    if (flatOut >= FLAT_OUT_SECONDS && endTime < 2.5f) { // a blur of colour
      BeginBlendMode(BLEND_ADDITIVE);
      DrawCircleGradient(mill, 90, {255, 230, 160, 120}, {255, 230, 160, 0});
      EndBlendMode();
    }
    DrawWindmill(mill, 52, spin, 0);
    if (speed > 0.6f) // whirring
      for (int k = 0; k < 3; k++)
        DrawRing(mill, 60 + k * 6, 62 + k * 6, spin * 60 + k * 120,
                 spin * 60 + k * 120 + 70, 10, Faded(COMIC_INK, speed - 0.5f));
    if (flatOut >= FLAT_OUT_SECONDS)
      DrawConfetti(mill, endTime, 11);
    says.Draw({LITTLE_ANU_X + 10, RUN_FEET - 180}, time);
    DrawNewsprint();
    if (time < 1)
      DrawCurtain(1 - time, {255, 236, 200, 255});
    if (speed < 0.05f && flatOut < FLAT_OUT_SECONDS && time > 0.8f) // hold
      DrawHint({LITTLE_ANU_X + 160, RUN_FEET - 40}, time);
    if (flatOut >= FLAT_OUT_SECONDS && endTime > 3)
      DrawGoOn(time);
  }

private:
  Line faster;
  Quip says;
  float time = 0, speed = 0, scroll = 0, stride = 0, spin = 0;
  float flatOut = 0, endTime = 0;
  bool saidFaster = false;
};

//------------------------------------------------------------------------------
// festival_lamps: the temple at night. Amma is lighting the row of lamps
// with the other women. Hold and sweep along the row to light them from
// Anu's end; Amma lights them from hers. When the row is lit she looks up,
// and sees Anu laughing, a windmill in her hand.
//------------------------------------------------------------------------------
const int ROW_LAMPS = 13;
const float ROW_Y = 648;
const float TEMPLE_ANU_X = 110, TEMPLE_AMMA_X = 600, TEMPLE_FEET = 630;

class TempleLampsScene : public Scene {
public:
  explicit TempleLampsScene(const Step &step) {
    amma = LineAt(step.lines, 0, {"Amma", "Still the same."});
    for (int i = 0; i < ROW_LAMPS; i++)
      lit[i] = 0;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crowd", 0.3f);
    for (int i = 0; i < ROW_LAMPS; i++)
      if (lit[i] > 0)
        lit[i] = std::fmin(1, lit[i] + dt * 3);
    if (stage == LIGHTING) {
      // Amma, from her end, one at a time
      ammaTimer += dt;
      if (ammaTimer > 1.1f) {
        ammaTimer = 0;
        for (int i = ROW_LAMPS - 1; i >= 0; i--)
          if (lit[i] <= 0) {
            if (i >= ROW_LAMPS - 5)
              Light(i, 0.5f);
            break;
          }
      }
      // Anu, from hers: sweep along the row
      const Vector2 mouse = GetMousePosition();
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        for (int i = 0; i < ROW_LAMPS; i++) {
          const Vector2 p = Lamp(i);
          if (lit[i] <= 0 && std::fabs(mouse.x - p.x) < 26 &&
              std::fabs(mouse.y - p.y) < 70)
            Light(i, 0.9f);
        }
      if (IsKeyPressed(KEY_SPACE))
        for (int i = 0; i < ROW_LAMPS; i++)
          if (lit[i] <= 0) {
            Light(i, 0.9f);
            break;
          }
      bool all = true;
      for (int i = 0; i < ROW_LAMPS; i++)
        all = all && lit[i] > 0;
      if (all) {
        stage = LAUGHING;
        stageTime = 0;
      }
    } else if (stage == LAUGHING) {
      stageTime += dt;
      if (stageTime > 2.6f) {
        stage = TALK;
        talk.Start({amma});
      }
    } else if (stage == TALK) {
      if (talk.Update()) {
        stage = AFTER;
        stageTime = 0;
      }
    } else {
      stageTime += dt;
      done = stageTime > 2.5f;
    }
  }

  void Draw() override {
    DrawTemple();
    // the other women, far off, lighting the lamps along the wall
    Folk woman;
    woman.height = 170;
    woman.head = 0.075f;
    woman.hair = Folk::BUN;
    woman.jasmine = true;
    const Color SAREES[] = {{200, 60, 110, 255}, {60, 140, 120, 255},
                            {230, 160, 40, 255}};
    for (int k = 0; k < 3; k++) {
      woman.bottom = woman.top = SAREES[k];
      woman.border = {240, 200, 80, 255};
      const float x = -170.0f + k * 300 + (k == 1 ? 230 : 0);
      DrawFolk(woman, x, 540, time, {0.2f, 0.36f + 0.02f * std::sin(time + k)},
               k % 2 ? -0.4f : 0.4f);
    }
    // Meena and her boy, in the middle, the boy blowing on his windmill
    const float H = 278, mx = 350, mfeet = 560;
    DrawMeena(mx, mfeet, time, {0.12f, 0.45f}, {-0.6f, 0.0f});
    DrawFolk(MeenasBoy(140), mx + 0.17f * H, mfeet - 0.37f * H, time,
             {0.05f, 1.05f}, -0.6f, true);
    const Vector2 boyHand = HandAt(mx + 0.17f * H, mfeet - 0.37f * H, 140,
                                   {0.05f, 1.05f});
    DrawWindmillStick({boyHand.x + 4, boyHand.y - 60}, 22, boyHand);
    DrawWindmill({boyHand.x + 4, boyHand.y - 60}, 22, time * 9, 1);
    // Anu, her windmill in her hand; laughing once the row is lit
    const bool laughing = stage != LIGHTING;
    const Vector2 anuHand = HandAt(TEMPLE_ANU_X, TEMPLE_FEET, 285,
                                   laughing ? Vector2{0.2f, 0.8f}
                                            : Vector2{0.2f, 0.55f});
    DrawAnu(TEMPLE_ANU_X, TEMPLE_FEET, time,
            laughing ? Vector2{0.2f, 0.8f} : Vector2{0.2f, 0.55f},
            laughing ? Vector2{0.7f, -0.2f} : Vector2{0.5f, 0.6f});
    DrawWindmillStick({anuHand.x + 6, anuHand.y - 66}, 30, anuHand);
    DrawWindmill({anuHand.x + 6, anuHand.y - 66}, 30, time * (laughing ? 8 : 2), 0);
    if (laughing) { // a comic laugh: little strokes round her head
      for (int k = 0; k < 3; k++) {
        const float a = -2.4f + k * 0.45f, bounce = 3 * std::sin(time * 14 + k);
        const Vector2 c = {TEMPLE_ANU_X + 6, TEMPLE_FEET - 252};
        InkLine({c.x + std::cos(a) * (46 + bounce), c.y + std::sin(a) * (46 + bounce)},
                {c.x + std::cos(a) * (62 + bounce), c.y + std::sin(a) * (62 + bounce)},
                3);
      }
    }
    DrawAmma(TEMPLE_AMMA_X, TEMPLE_FEET, time, stage == LIGHTING);
    // the stone ledge and the row of lamps
    const Rectangle all = FullScreen();
    ComicRect({all.x - 4, ROW_Y, all.width + 8, 20}, {170, 164, 160, 255}, 2.4f);
    DrawToneRect({all.x, ROW_Y + 8, all.width, 12}, COMIC_INK, 0, 0.5f,
                 TONE_DOTS, 4);
    NightPlate({150, 150, 210, 255});
    for (int i = 0; i < ROW_LAMPS; i++)
      DrawAgal(Lamp(i), 1.3f, lit[i], time);
    for (int i = 0; i < ROW_LAMPS; i++)
      DrawAgalGlow(Lamp(i), 1.3f, lit[i], time);
    LightPool({TEMPLE_AMMA_X, 420}, 260, {255, 170, 80, 255}, 0.8f);
    LightPool({TEMPLE_ANU_X, 420}, 220, {255, 170, 80, 255}, 0.6f);
    if (stage == LIGHTING && time > 1) {
      int first = 0;
      while (first < ROW_LAMPS && lit[first] > 0)
        first++;
      if (first < ROW_LAMPS) {
        const float slide = std::fmod(time, 1.4f) / 1.4f;
        DrawHint({Lamp(first).x + slide * 110, ROW_Y - 14}, time, 1 - slide * 0.5f);
      }
    }
    talk.Draw({TEMPLE_ANU_X, TEMPLE_FEET - 250},
              {TEMPLE_AMMA_X - 4, TEMPLE_FEET - 262}, time);
    if (time < 1)
      DrawCurtain(1 - time);
    if (stage == AFTER)
      DrawCurtain(Between(stageTime, 0.8f, 2.5f));
  }

private:
  enum Stage { LIGHTING, LAUGHING, TALK, AFTER } stage = LIGHTING;
  Line amma;
  Balloons talk;
  float lit[ROW_LAMPS];
  float time = 0, stageTime = 0, ammaTimer = 0;

  static Vector2 Lamp(int i) { return {40.0f + i * 53, ROW_Y}; }

  void Light(int i, float volume) {
    lit[i] = 0.01f;
    PlayEffect("bell", 0.25f * volume, 1.6f + 0.05f * i);
  }

  // The temple front: the gopuram towering over a wall in red and white
  // stripes, the doorway glowing, granite underfoot
  void DrawTemple() const {
    const Rectangle all = FullScreen();
    DrawStreetSky(300, true, time, 0);
    LightPool({360, 160}, 360, {255, 170, 90, 255}, 1.2f);
    DrawTempleTower(360, 260, 1.7f, 1, time);
    DrawBand(250, 290, {236, 214, 180, 255});
    for (float x = all.x; x < all.x + all.width; x += 56) // kaavi stripes
      DrawRectangleRec({x, 270, 26, 250}, {190, 96, 64, 255});
    DrawToneRect({all.x, 270, all.width, 250}, {90, 30, 20, 255}, 0.1f, 0.45f,
                 TONE_DOTS, 5);
    DrawBand(250, 22, {226, 160, 120, 255});
    DrawBand(249, 3, COMIC_INK);
    DrawBand(271, 2.5f, COMIC_INK);
    // the doorway, the lamp-lit sanctum far inside
    ComicRect({290, 290, 140, 250}, {60, 36, 30, 255}, 3);
    DrawCircleGradient({360, 440}, 70, {255, 200, 100, 255}, {255, 200, 100, 0});
    for (int k = 0; k < 3; k++)
      DrawAgal({330.0f + k * 30, 470}, 0.9f, 1, time);
    DrawThoranamLine(280, 440, 300);
    // the granite floor
    DrawBandGradient(520, 200, {176, 172, 174, 255}, {140, 136, 140, 255});
    DrawBand(519, 3, COMIC_INK);
    for (float x = all.x; x < all.x + all.width; x += 90)
      InkLine({x, 520}, {x - 60 + (x - 360) * 0.4f, 720}, 1.2f,
              Faded(COMIC_INK, 0.4f));
    DrawToneRect({all.x, 520, all.width, 200}, COMIC_INK, 0.05f, 0.35f,
                 TONE_DOTS, 6);
    DrawGroundKolam({360, 590}, 70, 1);
    // lamps on the wall's ledge, lit by the women
    for (int k = 0; k < 20; k++)
      DrawAgal({all.x + 30 + k * 66.0f, 520}, 0.8f, 1, time);
  }

  void DrawThoranamLine(float x0, float x1, float y) const {
    for (int i = 0; i < (int)((x1 - x0) / 12); i++) {
      const float lx = x0 + i * 12 + 4;
      DrawComicLeaf({lx, y}, {lx + std::sin(time * 2 + i), y + 22}, 9,
                    i % 2 ? LEAF : Color{96, 168, 64, 255}, 1.2f);
    }
  }
};

//------------------------------------------------------------------------------
// festival_book: late, the hall quiet. Amma's kolam book on the mat by a
// lamp. Turn its pages (drag across, or tap the right page) to the BIG one,
// little Anu's. Hold a hand on it, and Anu falls asleep there.
//------------------------------------------------------------------------------
const Rectangle LEFT_LEAF = {80, 120, 280, 400};
const Rectangle RIGHT_LEAF = {360, 120, 280, 400};
const int BIG_PAGE = 5;
const Color CRAYONS[] = {{235, 110, 40, 255}, {225, 70, 130, 255},
                         {60, 160, 80, 255},  {50, 120, 220, 255},
                         {200, 170, 30, 255}, {140, 80, 200, 255}};

struct BookPage {
  std::vector<std::vector<Vector2>> loops;
  std::vector<Vector2> dots;
  bool crayon = false;
};

BookPage MakeBookPage(int index, Rectangle leaf, bool solve) {
  BookPage p;
  if (index < 0 || index >= (int)Pages().size())
    return p;
  const Page &page = Pages()[index];
  p.crayon = page.byLittleAnu;
  KolamGrid grid;
  if (!grid.Load(page))
    return p;
  if (solve)
    grid.CountSolutions(true);
  const float cell = std::fmin((leaf.width - 50) / grid.width,
                               (leaf.height - 90) / grid.height);
  const Vector2 origin = {leaf.x + (leaf.width - cell * grid.width) / 2,
                          leaf.y + 60 + (leaf.height - 80 - cell * grid.height) / 2};
  p.loops = LoopPaths(grid, origin, cell, p.crayon ? 1.6f : 0);
  for (int y = 0; y < grid.height; y++)
    for (int x = 0; x < grid.width; x++)
      p.dots.push_back({origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell});
  return p;
}

class KolamBookScene : public Scene {
public:
  KolamBookScene() {
    for (int i = 0; i <= BIG_PAGE; i++) {
      // Amma's pages are finished; little Anu's BIG one never was
      right.push_back(MakeBookPage(i, RIGHT_LEAF, i != BIG_PAGE));
      left.push_back(MakeBookPage(i, LEFT_LEAF, i != BIG_PAGE));
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crickets", 0.45f * (1 - sleep * 0.6f));
    const Vector2 mouse = GetMousePosition();
    if (turning > 0) {
      turning += dt * 2.2f;
      if (turning >= 1) {
        turning = 0;
        page++;
      }
      return;
    }
    if (page < BIG_PAGE) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        pressAt = mouse;
      const bool swiped = IsMouseButtonReleased(MOUSE_BUTTON_LEFT) &&
                          pressAt.x - mouse.x > 60;
      const bool tapped = IsMouseButtonReleased(MOUSE_BUTTON_LEFT) &&
                          std::fabs(pressAt.x - mouse.x) < 10 &&
                          CheckCollisionPointRec(mouse, RIGHT_LEAF);
      if (swiped || tapped || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_RIGHT)) {
        turning = 0.001f;
        PlayEffect("page", 0.7f);
      }
      return;
    }
    // the BIG one: a hand on it, held, and she drifts off
    const bool holding = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE);
    handIn = Approach(handIn, holding || asleep ? 1.0f : 0.0f, 3);
    if (holding && handIn > 0.8f)
      sleep = std::fmin(1, sleep + dt / 2.5f); // (about 3 seconds)
    else if (!asleep)
      sleep = std::fmax(0, sleep - dt / 6); // (slowly back, if let go)
    idle = holding ? 0 : idle + dt;
    if (sleep >= 1)
      asleep = true;
    if (asleep) {
      afterSleep += dt;
      done = afterSleep > 5;
    }
  }

  void Draw() override {
    const Rectangle all = FullScreen();
    // the straw mat on the hall floor
    DrawRectangleRec(all, {214, 176, 110, 255});
    for (float y = 0; y < 720; y += 18)
      DrawRectangleRec({all.x, y, all.width, 7},
                       (int)(y / 18) % 4 == 1 ? Color{180, 70, 60, 255}
                                              : Color{196, 156, 92, 255});
    // the lamp beside the book
    const Vector2 lamp = {688, 84};
    // the book: two leaves and the spine
    ComicRect({LEFT_LEAF.x - 14, LEFT_LEAF.y - 14, 588, 428}, {120, 50, 40, 255}, 3);
    DrawLeaf(LEFT_LEAF, page > 0 ? &left[page - 1] : nullptr, page - 1);
    if (turning > 0)
      DrawLeaf(RIGHT_LEAF, &right[page + 1], page + 1);
    else
      DrawLeaf(RIGHT_LEAF, &right[page], page);
    if (turning > 0) { // the leaf turning over the spine
      const float sx = std::cos(turning * PI);
      rlPushMatrix();
      rlTranslatef(360, 0, 0);
      rlScalef(sx, 1, 1);
      rlTranslatef(-360, 0, 0);
      if (sx > 0)
        DrawLeaf(RIGHT_LEAF, &right[page], page);
      else // its back: the next left-hand page, mirrored back by the scale
        DrawLeafMirrored(page);
      rlPopMatrix();
    }
    DrawLineEx({360, 118}, {360, 522}, 4, COMIC_INK);
    BeginTone(TONE_DOTS, 4);
    DrawRectangleGradientH(330, 120, 30, 400, {0, 0, 0, 0}, {60, 30, 20, 120});
    DrawRectangleGradientH(360, 120, 30, 400, {60, 30, 20, 120}, {0, 0, 0, 0});
    EndTone();
    // her hand, resting on it
    if (handIn > 0.01f)
      DrawRestingHand(handIn);
    // night over everything, the lamp's light on the book
    NightPlate({150, 150, 205, 255});
    const float dim = 1 - 0.7f * sleep;
    DrawAgal(lamp, 1.6f, dim, time);
    DrawAgalGlow(lamp, 1.6f, dim, time);
    LightPool({420, 320}, 330, {255, 180, 90, 255}, dim);
    // the BIG one's crayon lines: they glow a little, as she drifts off
    if (page == BIG_PAGE && sleep > 0) {
      BeginBlendMode(BLEND_ADDITIVE);
      const BookPage &p = right[BIG_PAGE];
      for (size_t i = 0; i < p.loops.size(); i++)
        DrawPath(p.loops[i], p.loops[i].size(), true, 5,
                 Faded(CRAYONS[i % 6], 0.5f * sleep));
      EndBlendMode();
      for (int k = 0; k < 14; k++) { // crayon sparkles drifting up
        const float age = std::fmod(time * 0.25f + k * 0.071f, 1.0f);
        const Vector2 s = {RIGHT_LEAF.x + 30 + std::fmod(k * 53.0f, 220.0f),
                           RIGHT_LEAF.y + RIGHT_LEAF.height - 60 - age * 420};
        DrawSparkle(s, 5 * (1 - age) * sleep, Faded(CRAYONS[k % 6], sleep));
      }
    }
    DrawCurtain(sleep * 0.55f + Between(afterSleep, 1.5f, 5) * 0.45f,
                {8, 10, 30, 255});
    if (turning <= 0 && page < BIG_PAGE && time > 1) {
      const float slide = std::fmod(time, 1.4f) / 1.4f;
      DrawHint({RIGHT_LEAF.x + 220 - slide * 160, RIGHT_LEAF.y + 330}, time,
               1 - slide * 0.6f);
    }
    if (page == BIG_PAGE && turning <= 0 && !asleep) {
      // hold a hand on it (not a tap): the hand, and a ring that fills
      const Vector2 at = {RIGHT_LEAF.x + 150, RIGHT_LEAF.y + 260};
      if (handIn < 0.5f || idle > 3)
        DrawGesture(MOVE_HOLD, at, time);
      if (sleep > 0.01f) {
        DrawRing(at, 34, 40, 0, 360, 40, Faded(COMIC_PAPER, 0.3f));
        DrawRing(at, 34, 40, -90, -90 + 360 * sleep, 40,
                 {255, 220, 160, 255});
      }
    }
    if (time < 1)
      DrawCurtain(1 - time);
  }

private:
  std::vector<BookPage> right, left;
  int page = 0;
  float time = 0, turning = 0, handIn = 0, sleep = 0, afterSleep = 0;
  float idle = 0; // since the hand was last held on the BIG one
  bool asleep = false;
  Vector2 pressAt = {};

  void DrawLeaf(Rectangle leaf, const BookPage *p, int index) const {
    const bool crayon = p && p->crayon;
    DrawRectangleRec(leaf, crayon ? Color{250, 247, 238, 255}
                                  : Color{243, 234, 210, 255});
    if (crayon) // a school notebook's ruled lines
      for (float y = leaf.y + 30; y < leaf.y + leaf.height; y += 24)
        DrawLineEx({leaf.x, y}, {leaf.x + leaf.width, y}, 1,
                   {170, 196, 220, 255});
    InkRect(leaf, 2);
    if (!p || index < 0)
      return;
    // the page's title, small, in the writer's hand
    DrawString(Pages()[index].title, leaf.x + 20, leaf.y + 16, 20,
               crayon ? Color{150, 60, 160, 255} : Color{40, 52, 110, 255},
               HAND_FONT);
    const Color dot = crayon ? Color{150, 60, 160, 255} : Color{70, 55, 45, 255};
    for (const Vector2 &d : p->dots)
      DrawCircleV(d, 3.2f, dot);
    for (size_t i = 0; i < p->loops.size(); i++)
      DrawPath(p->loops[i], p->loops[i].size(), true, crayon ? 4 : 3,
               crayon ? CRAYONS[i % 6] : Color{40, 42, 92, 255});
  }

  void DrawLeafMirrored(int index) const {
    // seen from behind while it turns: the next page's left side, which the
    // scale has already flipped; draw it pre-flipped so it reads the right way
    rlPushMatrix();
    rlTranslatef(360, 0, 0);
    rlScalef(-1, 1, 1);
    rlTranslatef(-360, 0, 0);
    DrawLeaf(LEFT_LEAF, &left[index], index);
    rlPopMatrix();
  }

  // Anu's hand, slid in from the bottom right, resting on the BIG page
  void DrawRestingHand(float in) const {
    const float breathe = 2 * std::sin(time * 1.3f);
    const Vector2 palm = {RIGHT_LEAF.x + 170 + (1 - in) * 300,
                          RIGHT_LEAF.y + 280 + (1 - in) * 220 + breathe};
    const Vector2 wrist = {palm.x + 54, palm.y + 50};
    const Vector2 elbow = {palm.x + 330, palm.y + 300};
    // the forearm, out of her kurta's sleeve
    DrawLineEx(wrist, elbow, 84, PERSON_INK);
    DrawLineEx(wrist, elbow, 78, SKIN_DARK);
    DrawLineEx({wrist.x - 6, wrist.y - 10}, {elbow.x - 14, elbow.y - 20}, 40,
               SKIN);
    const Vector2 cuff = {wrist.x + 150, wrist.y + 138};
    DrawLineEx(cuff, {cuff.x + 200, cuff.y + 190}, 118, PERSON_INK);
    DrawLineEx(cuff, {cuff.x + 200, cuff.y + 190}, 110, {214, 160, 58, 255});
    DrawLineEx({cuff.x - 4, cuff.y - 4}, {cuff.x + 8, cuff.y + 8}, 112,
               {150, 40, 50, 255}); // the sleeve's maroon trim
    // a thin gold bangle
    DrawEllipseV({wrist.x + 14, wrist.y + 12}, 40, 16, PERSON_INK);
    DrawEllipseV({wrist.x + 14, wrist.y + 12}, 37, 11, {240, 190, 60, 255});
    // the back of the hand, the fingers spread a little on the page
    for (int k = 0; k < 4; k++) {
      const float a = -2.55f + k * 0.3f;
      const Vector2 knuckle = {palm.x + std::cos(a) * 30, palm.y + std::sin(a) * 30};
      const Vector2 tip = {palm.x + std::cos(a) * (84 - std::fabs(k - 1.2f) * 8),
                           palm.y + std::sin(a) * (84 - std::fabs(k - 1.2f) * 8)};
      DrawLineEx(knuckle, tip, 25, PERSON_INK);
      DrawCircleV(tip, 12.5f, PERSON_INK);
      DrawLineEx(knuckle, tip, 20, SKIN);
      DrawCircleV(tip, 10, SKIN);
      DrawEllipseV({tip.x + std::cos(a) * -3, tip.y + std::sin(a) * -3}, 5, 4,
                   {222, 170, 140, 255}); // a nail
    }
    const Vector2 thumb = {palm.x + 26, palm.y - 58};
    DrawLineEx(palm, thumb, 28, PERSON_INK);
    DrawCircleV(thumb, 14, PERSON_INK);
    DrawLineEx(palm, thumb, 22, SKIN);
    DrawCircleV(thumb, 11, SKIN);
    DrawEllipseV({palm.x + 14, palm.y + 10}, 58, 46, PERSON_INK);
    DrawEllipseV({palm.x + 14, palm.y + 10}, 54, 42, SKIN);
    InkCurve({palm.x - 10, palm.y - 8}, {palm.x + 4, palm.y + 6},
             {palm.x + 30, palm.y + 14}, 1.4f, Faded(PERSON_INK, 0.4f));
  }
};

std::unique_ptr<Scene> MakeTower(const Step &) {
  return std::unique_ptr<Scene>(new TowerScene());
}
std::unique_ptr<Scene> MakeRingToss(const Step &step) {
  return std::unique_ptr<Scene>(new RingTossScene(step));
}
std::unique_ptr<Scene> MakeWindmillStall(const Step &) {
  return std::unique_ptr<Scene>(new WindmillStallScene());
}
std::unique_ptr<Scene> MakeWindmillRun(const Step &step) {
  return std::unique_ptr<Scene>(new WindmillRunScene(step));
}
std::unique_ptr<Scene> MakeTempleLamps(const Step &step) {
  return std::unique_ptr<Scene>(new TempleLampsScene(step));
}
std::unique_ptr<Scene> MakeKolamBook(const Step &) {
  return std::unique_ptr<Scene>(new KolamBookScene());
}

} // namespace

PLAY_SCENE("festival_tower", MakeTower)
PLAY_SCENE("festival_ring_toss", MakeRingToss)
PLAY_SCENE("festival_windmills", MakeWindmillStall)
PLAY_SCENE("festival_windmill_run", MakeWindmillRun)
PLAY_SCENE("festival_lamps", MakeTempleLamps)
PLAY_SCENE("festival_book", MakeKolamBook)
