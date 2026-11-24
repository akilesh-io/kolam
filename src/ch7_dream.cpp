// ch7_dream.cpp - Chapter 7, "The Little Girl": a dream on festival eve.
// The kolam book opens into little Anu's crayon drawing, come alive.
//   crayon_world  the drawing starts as outlines on paper; scrub to colour it
//                 in (sky, sun, house, roof, grass), each part waking up as it
//                 is coloured. Little Anu is on the step: "You're late."
//   big_one       page 6 of the kolam book (the puzzle), with little Anu and
//                 Anu beside it in the crayon world
//   together      the finished kolam on the notebook; hold to go over it once
//                 more, "Just don't lift", and little Anu adds a flower
//   alarm_450     4:50 am in the hall: the alarm rings, and Anu is already
//                 awake. Slide it off.
// The crayon world is drawn differently from the comic-book home on purpose:
// it is a seven-year-old's drawing.
#include "finale.h"

using namespace finale;

namespace {

const Color CRAYON_PAPER = {250, 246, 232, 255};
const Color PAPER_RULE = {150, 186, 222, 80};
const Color OUTLINE = {64, 54, 104, 255}; // a dark purple crayon
const Color CRAYON_WHITE = {255, 255, 252, 255};

// The drawing's parts, coloured in by scrubbing
enum Part { SKY, SUN, WALLS, ROOF, GRASS, PART_COUNT };
const Color PART_COLOURS[PART_COUNT] = {{96, 170, 238, 255},
                                        {252, 196, 40, 255},
                                        {244, 136, 170, 255},
                                        {226, 64, 52, 255},
                                        {98, 182, 74, 255}};
const float PART_CELL[PART_COUNT] = {30, 15, 19, 19, 30}; // crayon cell size
const float PART_ANGLE[PART_COUNT] = {-0.35f, 0.5f, 0.8f, -0.8f, 1.25f};
const float COLOURED_ENOUGH = 0.55f; // then the rest colours itself

// Where things are in the drawing (stage units)
const Rectangle WALLS_RECT = {330, 300, 220, 190};
const Vector2 ROOF_POINTS[3] = {{306, 304}, {440, 196}, {574, 304}};
const Rectangle CHIMNEY = {498, 204, 26, 56};
const Rectangle DOOR = {410, 396, 60, 94};
const Rectangle WINDOW = {350, 330, 48, 46};
const Rectangle STEP = {388, 490, 104, 16};
const Vector2 SUN_CENTRE = {612, 98};
const float SUN_RADIUS = 46;
const Vector2 LITTLE_ANU = {440, 504}; // standing on the step
const float LITTLE_ANU_HEIGHT = 150;

float HillY(float x) {
  return 336 - 72 * std::fabs(std::sin((x + 120) * 0.0045f));
}

// The crayon drawing: its parts, and how much of each is coloured in
class CrayonWorld {
public:
  struct Cell {
    Vector2 c;
    float cover = 0; // 0 = paper, 1 = coloured
    float wait = -1; // colouring itself in after this long (-1 = no)
  };

  CrayonWorld() {
    for (int p = 0; p < PART_COUNT; p++) {
      const float s = PART_CELL[p];
      for (float y = s / 2; y < 720; y += s) {
        for (float x = -300 + s / 2; x < 1020; x += s) {
          if (PartAt({x, y}) == p)
            cells[p].push_back({{x, y}});
        }
      }
    }
  }

  static int PartAt(Vector2 p) {
    if (std::hypot(p.x - SUN_CENTRE.x, p.y - SUN_CENTRE.y) < SUN_RADIUS)
      return SUN;
    if (CheckCollisionPointRec(p, WALLS_RECT) ||
        CheckCollisionPointRec(p, STEP))
      return WALLS;
    if (CheckCollisionPointTriangle(p, ROOF_POINTS[0], ROOF_POINTS[1], ROOF_POINTS[2]) ||
        CheckCollisionPointRec(p, CHIMNEY))
      return ROOF;
    return p.y < HillY(p.x) ? SKY : GRASS;
  }

  bool Complete(int p) const { return complete[p]; }
  int CompleteCount() const {
    int n = 0;
    for (int p = 0; p < PART_COUNT; p++)
      n += Complete(p) ? 1 : 0;
    return n;
  }
  float SinceComplete(int p) const { return Complete(p) ? time - doneAt[p] : 0; }

  void CompleteAll() {
    for (int p = 0; p < PART_COUNT; p++) {
      for (Cell &c : cells[p])
        c.cover = 1;
      complete[p] = true;
      doneAt[p] = time - 10;
    }
  }

  // Scrubbing from `from` to `to` colours what's under the crayon. Returns
  // the parts that just became coloured enough.
  std::vector<int> Scrub(Vector2 from, Vector2 to) {
    const float BRUSH = 34;
    std::vector<int> finished;
    const float len = std::hypot(to.x - from.x, to.y - from.y);
    const int steps = 1 + (int)(len / 10);
    for (int p = 0; p < PART_COUNT; p++) {
      if (Complete(p))
        continue;
      for (int s = 0; s <= steps; s++) {
        const Vector2 m = {from.x + (to.x - from.x) * s / steps,
                           from.y + (to.y - from.y) * s / steps};
        for (Cell &c : cells[p]) {
          if (c.wait < 0 && c.cover <= 0 &&
              std::fabs(c.c.x - m.x) < BRUSH && std::fabs(c.c.y - m.y) < BRUSH &&
              std::hypot(c.c.x - m.x, c.c.y - m.y) < BRUSH)
            c.wait = 0;
        }
      }
      if (Coloured(p) >= COLOURED_ENOUGH) { // the rest colours itself
        for (Cell &c : cells[p]) {
          if (c.wait < 0 && c.cover <= 0)
            c.wait = std::hypot(c.c.x - to.x, c.c.y - to.y) / 900.0f;
        }
        doneAt[p] = time;
        complete[p] = true;
        finished.push_back(p);
      }
    }
    return finished;
  }

  // How much of a part is coloured, counting only what every screen shows
  float Coloured(int p) const {
    int all = 0, coloured = 0;
    for (const Cell &c : cells[p]) {
      if (c.c.x < 0 || c.c.x > 720)
        continue;
      all++;
      coloured += c.wait >= 0 || c.cover > 0 ? 1 : 0;
    }
    return all ? (float)coloured / all : 1;
  }

  void Update(float dt) {
    time += dt;
    for (auto &part : cells) {
      for (Cell &c : part) {
        if (c.wait < 0)
          continue;
        c.wait -= dt;
        if (c.wait <= 0) {
          c.cover = std::fmin(1, c.cover + dt * 7);
          if (c.cover >= 1)
            c.wait = -1;
          else
            c.wait = 0;
        }
      }
    }
  }

  // The whole drawing. Nothing is drawn inside `hole` (where a page lies on
  // top), if it has a size.
  void Draw(Rectangle hole = {0, 0, 0, 0}) const {
    const Rectangle all = FullScreen();
    const bool holed = hole.width > 0;
    auto hidden = [&](Rectangle r) {
      return holed && CheckCollisionRecs(r, hole);
    };
    for (float y = 40; y < 720 && !holed; y += 34) { // a school notebook
      if (y == 40)
        DrawRectangleRec(all, CRAYON_PAPER);
      DrawLineEx({all.x, y}, {all.x + all.width, y}, 1.5f, PAPER_RULE);
    }
    const int boil = (int)(time * 5); // the lines wobble, alive

    // the colours, scribbled in (a little outside the lines)
    for (int p = 0; p < PART_COUNT; p++) {
      const float s = PART_CELL[p];
      for (size_t i = 0; i < cells[p].size(); i++) {
        const Cell &c = cells[p][i];
        // each scribble at its own slant, as a hand colours
        const float a = PART_ANGLE[p] + 0.3f * Noise(p + 7.0f, (float)i);
        const Vector2 dir = {std::cos(a), std::sin(a)};
        const Vector2 side = {-dir.y, dir.x};
        const Vector2 jog = {Noise(p + 3.0f, (float)i) * s * 0.25f,
                             Noise(p + 5.0f, (float)i) * s * 0.25f};
        if (c.cover <= 0 || c.c.x < all.x - s || c.c.x > all.x + all.width + s)
          continue;
        if (holed && CheckCollisionPointRec(c.c, {hole.x - s * 0.5f,
                                                  hole.y - s * 0.5f,
                                                  hole.width + s,
                                                  hole.height + s}))
          continue;
        const float shade = 1 + 0.1f * Noise((float)p, (float)i);
        Color col = PART_COLOURS[p];
        col.r = (unsigned char)std::fmin(255, col.r * shade);
        col.g = (unsigned char)std::fmin(255, col.g * shade);
        col.b = (unsigned char)std::fmin(255, col.b * shade);
        col = Faded(col, 0.85f * c.cover);
        Vector2 prev = {};
        for (int k = 0; k < 4; k++) {
          const float along = (k / 3.0f * 1.5f - 0.75f) * s;
          const float across = (k % 2 ? 0.36f : -0.36f) * s;
          const Vector2 q = {c.c.x + jog.x + dir.x * along + side.x * across +
                                 Noise(i * 0.1f, (float)k) * 3,
                             c.c.y + jog.y + dir.y * along + side.y * across +
                                 Noise(i * 0.1f, k + 5.0f) * 3};
          if (k > 0)
            DrawLineEx(prev, q, s * 0.5f, col);
          prev = q;
        }
      }
    }

    // the parts that came with the house when it was coloured
    if (Complete(WALLS) && !hidden(DOOR)) {
      CrayonFill(DOOR, {150, 84, 44, 255}, 7);
      CrayonFill(WINDOW, {150, 214, 250, 255}, 7);
      CrayonFill({STEP.x, STEP.y + 2, STEP.width, STEP.height - 2},
                 {170, 160, 160, 255}, 5);
    }

    // the outlines
    std::vector<Vector2> hills;
    for (float x = -320; x <= 1040; x += 40) {
      if (holed && x > hole.x + 20 && x < hole.x + hole.width - 20) {
        CrayonPath(hills, false, 4, OUTLINE, 3, boil);
        hills.clear();
        continue;
      }
      hills.push_back({x, HillY(x)});
    }
    CrayonPath(hills, false, 4, OUTLINE, 3, boil);
    if (!holed) {
      DrawSun(boil);
      DrawHouse(boil);
    }
    DrawPalm({-170, 560}, {-150, 330}, boil);
    if (!hidden({600, 290, 160, 240}))
      DrawPalm({690, 520}, {664, 316}, boil);
    // two clouds drifting, never coloured (paper white is a cloud's colour)
    for (int i = 0; i < 2; i++) {
      const float x = all.x + std::fmod(140 + i * 560 + time * (6 + i * 3),
                                        all.width + 240) - 120;
      const Vector2 c = {x, 70.0f + i * 70};
      if (hidden({c.x - 70, c.y - 30, 140, 60}))
        continue;
      DrawCloud(c, boil, i);
    }
    // birds, once there is a sky to fly in
    if (Complete(SKY)) {
      for (int i = 0; i < 3; i++) {
        const float x = all.x + std::fmod(time * 40 + i * 260, all.width + 100);
        const float y = 150 + i * 30 + 10 * std::sin(time + i);
        const float flap = 7 * std::sin(time * 8 + i);
        if (hidden({x - 20, y - 20, 40, 40}))
          continue;
        CrayonPath({{x - 14, y - flap}, {x, y}, {x + 14, y - flap}}, false, 3,
                   OUTLINE, 20.0f + i, boil);
      }
    }
    // flowers in the grass, once it's green
    const Vector2 FLOWERS[] = {{80, 600},  {250, 560}, {560, 590}, {640, 660},
                               {-90, 650}, {-200, 600}, {820, 600}, {930, 660},
                               {30, 690},  {700, 560}};
    const Color PETALS[] = {{240, 80, 120, 255},
                            {250, 200, 40, 255},
                            {160, 90, 220, 255},
                            {250, 120, 40, 255}};
    if (Complete(GRASS)) {
      const float grow = Between(SinceComplete(GRASS), 0, 1.2f);
      int k = 0;
      for (Vector2 f : FLOWERS) {
        k++;
        if (hidden({f.x - 20, f.y - 50, 40, 60}))
          continue;
        const float g = Between(grow * 1.6f - k * 0.06f, 0, 1);
        if (g <= 0)
          continue;
        const float sway = 3 * std::sin(time * 2 + k);
        CrayonLine(f, {f.x + sway, f.y - 30 * g}, 4, {60, 140, 60, 255}, k);
        DrawLittleFlower({f.x + sway, f.y - 30 * g}, 9 * g, PETALS[k % 4], 5,
                         1, time * 0.3f + k);
      }
    }
  }

  float time = 0;

private:
  std::vector<Cell> cells[PART_COUNT];
  bool complete[PART_COUNT] = {};
  float doneAt[PART_COUNT] = {}; // when each was coloured enough

  // A small area coloured in all at once
  static void CrayonFill(Rectangle r, Color c, float step) {
    for (float y = r.y + step / 2; y < r.y + r.height; y += step)
      DrawLineEx({r.x + 2, y + 2}, {r.x + r.width - 2, y - 2}, step * 0.9f,
                 Faded(c, 0.85f));
  }

  static std::vector<Vector2> RectPoints(Rectangle r) {
    std::vector<Vector2> p;
    const Vector2 corners[4] = {{r.x, r.y},
                                {r.x + r.width, r.y},
                                {r.x + r.width, r.y + r.height},
                                {r.x, r.y + r.height}};
    for (int i = 0; i < 4; i++) {
      const Vector2 a = corners[i], b = corners[(i + 1) % 4];
      const int n = 1 + (int)(std::hypot(b.x - a.x, b.y - a.y) / 36);
      for (int k = 0; k < n; k++)
        p.push_back({a.x + (b.x - a.x) * k / n, a.y + (b.y - a.y) * k / n});
    }
    return p;
  }

  void DrawSun(int boil) const {
    const float since = SinceComplete(SUN);
    if (Complete(SUN)) { // rays, turning, and a smile: it's a child's sun
      for (int i = 0; i < 12; i++) {
        const float a = i * PI / 6 + time * 0.25f;
        const float r0 = SUN_RADIUS + 10, r1 = r0 + 22 * Between(since, 0, 0.8f);
        CrayonLine({SUN_CENTRE.x + r0 * std::cos(a),
                    SUN_CENTRE.y + r0 * std::sin(a)},
                   {SUN_CENTRE.x + r1 * std::cos(a),
                    SUN_CENTRE.y + r1 * std::sin(a)},
                   5, {250, 160, 30, 255}, i);
      }
      const float face = Between(since, 0.4f, 1.0f);
      if (face > 0) {
        DrawCircleV({SUN_CENTRE.x - 15, SUN_CENTRE.y - 8}, 4.5f * face, OUTLINE);
        DrawCircleV({SUN_CENTRE.x + 15, SUN_CENTRE.y - 8}, 4.5f * face, OUTLINE);
        std::vector<Vector2> smile;
        for (int k = 0; k <= 6; k++) {
          const float a = PI * (0.2f + 0.6f * k / 6);
          smile.push_back({SUN_CENTRE.x + 20 * std::cos(a),
                           SUN_CENTRE.y + 2 + 16 * std::sin(a) * face});
        }
        CrayonPath(smile, false, 4, OUTLINE, 9, boil);
      }
    }
    CrayonPath(CrayonEllipse(SUN_CENTRE, SUN_RADIUS, SUN_RADIUS, 4), true, 4,
               OUTLINE, 4, boil);
  }

  void DrawHouse(int boil) const {
    // smoke from the chimney once the roof is red: someone's cooking
    if (Complete(ROOF)) {
      for (int i = 0; i < 4; i++) {
        const float t = std::fmod(time * 0.35f + i * 0.25f, 1.0f);
        const Vector2 c = {CHIMNEY.x + 13 + 16 * std::sin(t * 6 + i),
                           CHIMNEY.y - 10 - t * 110};
        CrayonPath(CrayonEllipse(c, 9 + t * 10, 7 + t * 7, 30.0f + i, 10),
                   true, 3, Faded({150, 150, 170, 255}, 1 - t), 30.0f + i,
                   boil);
      }
    }
    CrayonPath(RectPoints(CHIMNEY), true, 4, OUTLINE, 5, boil);
    CrayonPath(RectPoints(WALLS_RECT), true, 4.5f, OUTLINE, 6, boil);
    CrayonPath({ROOF_POINTS[0], {373, 250}, ROOF_POINTS[1], {507, 250}, ROOF_POINTS[2]}, true, 4.5f,
               OUTLINE, 7, boil);
    CrayonPath(RectPoints(DOOR), true, 4, OUTLINE, 8, boil);
    DrawCircleV({DOOR.x + DOOR.width - 12, DOOR.y + 50}, 4, OUTLINE);
    CrayonPath(RectPoints(WINDOW), true, 4, OUTLINE, 10, boil);
    CrayonLine({WINDOW.x + WINDOW.width / 2, WINDOW.y},
               {WINDOW.x + WINDOW.width / 2, WINDOW.y + WINDOW.height}, 3.5f,
               OUTLINE, 11);
    CrayonLine({WINDOW.x, WINDOW.y + WINDOW.height / 2},
               {WINDOW.x + WINDOW.width, WINDOW.y + WINDOW.height / 2}, 3.5f,
               OUTLINE, 12);
    CrayonPath(RectPoints(STEP), true, 4, OUTLINE, 13, boil);
    // and once the house is coloured, a small kolam in front of the step
    if (Complete(WALLS)) {
      const float g = Between(SinceComplete(WALLS), 0.3f, 1.3f);
      for (int i = 0; i < 3; i++) {
        const Vector2 d = {406.0f + i * 34, 526};
        DrawCircleV(d, 3, Faded(CRAYON_WHITE, g));
        CrayonPath(CrayonEllipse(d, 14, 8, 40.0f + i, 12), true, 3,
                   Faded(CRAYON_WHITE, g), 40.0f + i, boil);
        CrayonPath(CrayonEllipse(d, 14, 8, 40.0f + i, 12), true, 1.5f,
                   Faded(OUTLINE, 0.35f * g), 40.0f + i, boil);
      }
    }
  }

  void DrawPalm(Vector2 base, Vector2 top, int boil) const {
    const bool green = Complete(GRASS);
    const float g = green ? Between(SinceComplete(GRASS), 0, 0.8f) : 0;
    const Vector2 mid = {(base.x + top.x) / 2 + 18, (base.y + top.y) / 2};
    if (g > 0) {
      for (int k = 0; k < 8; k++) {
        const float t = k / 8.0f;
        const Vector2 p = {base.x + (top.x - base.x) * t + 18 * std::sin(t * PI),
                           base.y + (top.y - base.y) * t};
        DrawLineEx({p.x - 9, p.y}, {p.x + 9, p.y - 6}, 12,
                   Faded({170, 110, 60, 255}, 0.85f * g));
      }
    }
    std::vector<Vector2> left, right;
    for (int k = 0; k <= 8; k++) {
      const float t = k / 8.0f;
      const float x = base.x + (top.x - base.x) * t + 18 * std::sin(t * PI);
      const float y = base.y + (top.y - base.y) * t;
      const float w = 10 - 3 * t;
      left.push_back({x - w, y});
      right.insert(right.begin(), {x + w, y});
    }
    std::vector<Vector2> trunk = left;
    trunk.insert(trunk.end(), right.begin(), right.end());
    CrayonPath(trunk, false, 4, OUTLINE, base.x, boil);
    (void)mid;
    for (int i = 0; i < 6; i++) { // the leaves: long drooping strokes
      const float a = -PI / 2 + (i - 2.5f) * 0.62f + 0.05f * std::sin(time + i);
      const Vector2 tip = {top.x + 78 * std::cos(a),
                           top.y + 78 * std::sin(a) + 34};
      const Vector2 bend = {top.x + 50 * std::cos(a), top.y + 50 * std::sin(a) - 8};
      std::vector<Vector2> leaf;
      for (int k = 0; k <= 6; k++) {
        const float t = k / 6.0f;
        const float u = 1 - t;
        leaf.push_back({u * u * top.x + 2 * u * t * bend.x + t * t * tip.x,
                        u * u * top.y + 2 * u * t * bend.y + t * t * tip.y});
      }
      if (g > 0)
        CrayonPath(leaf, false, 12, Faded({70, 160, 70, 255}, g), i + base.x,
                   0);
      CrayonPath(leaf, false, 3.5f, OUTLINE, i + base.x, boil);
    }
  }

  static void DrawCloud(Vector2 c, int boil, int i) {
    std::vector<Vector2> edge;
    for (int k = 0; k <= 20; k++) { // one bumpy line around the puffs
      const float a = PI + k * PI / 20;
      const float bump = 1 + 0.18f * std::fabs(std::sin(k * 1.6f));
      edge.push_back({c.x + 64 * std::cos(a) * bump * 0.95f,
                      c.y + 8 + 26 * std::sin(a) * bump});
    }
    const Vector2 middle = {c.x, c.y + 2};
    for (size_t k = 0; k < edge.size(); k++) // paper white inside
      FillTriangle(middle, edge[k], edge[(k + 1) % edge.size()], CRAYON_WHITE);
    CrayonPath(edge, true, 3.5f, OUTLINE, 50.0f + i, boil);
  }
};

// Little Anu, facing left (towards Anu), with her notebook in her hands
void DrawLittleAnuWithBook(Vector2 feet, float height, float time,
                           float bookOut = 0) {
  Mirrored(feet.x, [&]() {
    DrawLittleAnu(feet.x, feet.y, time, height);
    const Vector2 hands = {feet.x + 0.13f * height, feet.y - 0.56f * height};
    const Vector2 book = {hands.x + bookOut * 40, hands.y - bookOut * 20};
    const float w = 30 + bookOut * 16, h = 22 + bookOut * 10;
    DrawRectangleRec({book.x - w / 2 - 2, book.y - h / 2 - 2, w + 4, h + 4},
                     {226, 84, 132, 255}); // the pink cover
    DrawRectangleRec({book.x - w / 2, book.y - h / 2, w, h}, CRAYON_PAPER);
    DrawLineEx({book.x, book.y - h / 2}, {book.x, book.y + h / 2}, 1.5f,
               {200, 180, 170, 255});
    for (int i = 0; i < 3; i++)
      DrawCircleV({book.x + 5 + i * 4.0f, book.y - 2}, 1.2f, {150, 60, 160, 255});
  });
}

//------------------------------------------------------------------------------
// The crayon world: colour it in. Then little Anu looks up.
//------------------------------------------------------------------------------
class CrayonWorldScene : public Scene {
public:
  explicit CrayonWorldScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    world.Update(dt);
    PlayAmbience(world.Complete(SKY) ? "birds_day" : "", 0.35f);
    const Vector2 mouse = GetMousePosition();

    if (stage == COLOURING) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        const Vector2 from =
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ? mouse : lastMouse;
        const int part = CrayonWorld::PartAt(mouse);
        if (!world.Complete(part)) {
          PlayLoop("pencil", 0.4f);
          scrubbed += std::hypot(mouse.x - from.x, mouse.y - from.y);
        }
        for (int done : world.Scrub(from, mouse)) {
          (void)done;
          PlayEffect("bell", 0.35f, 0.9f + 0.12f * world.CompleteCount());
        }
      }
      if (world.CompleteCount() == PART_COUNT) {
        stage = WAKING;
        stageTime = 0;
      }
    } else if (stage == WAKING) { // the drawing settles; she looks up
      stageTime += dt;
      if (stageTime > 1.6f) {
        talk.Start(step.lines);
        stage = TALKING;
      }
    } else if (stage == TALKING) {
      if (talk.Update()) {
        stage = BOOK;
        stageTime = 0;
      }
    } else if (stage == BOOK) { // she holds out the notebook: open it
      stageTime += dt;
      if (stageTime > 0.8f && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        PlayEffect("page", 0.9f);
        stage = LEAVING;
        stageTime = 0;
      }
    } else {
      stageTime += dt;
      if (stageTime > 1.0f)
        done = true;
    }
    lastMouse = mouse;
  }

  void Draw() override {
    world.Draw();
    const float bookOut =
        stage == BOOK || stage == LEAVING ? Between(stageTime, 0, 0.6f) : 0;
    // she draws in her notebook (slowly) until the world is coloured
    const float herTime = stage == COLOURING ? time * 0.35f : time;
    DrawLittleAnuWithBook(LITTLE_ANU, LITTLE_ANU_HEIGHT, herTime, bookOut);
    // Anu, grown up, at the edge of the drawing, looking at her
    DrawAnu(150, 704, time, {0.13f, 0.5f}, {0.7f, 0.15f});

    if (stage == COLOURING)
      DrawCrayonCursor();
    if (stage == BOOK)
      DrawHint(BookAt(), 26, time, OUTLINE);
    talk.Draw([](const std::string &who) {
      if (who == "Anu")
        return BubbleSpot{{200, 330}, {160, 420}};
      return BubbleSpot{{540, 250}, {452, 364}};
    });
    if (stage == LEAVING) // the page turns: into the book
      FadeScreen(CRAYON_PAPER, Between(stageTime, 0, 0.9f));
  }

private:
  enum Stage { COLOURING, WAKING, TALKING, BOOK, LEAVING };
  const Step &step;
  CrayonWorld world;
  Conversation talk;
  Stage stage = COLOURING;
  float time = 0, stageTime = 0;
  float scrubbed = 0; // how far the crayon has gone
  Vector2 lastMouse = {0, 0};

  static Vector2 BookAt() {
    // where her (mirrored) hands hold the notebook out
    return {LITTLE_ANU.x - 0.13f * LITTLE_ANU_HEIGHT - 40,
            LITTLE_ANU.y - 0.56f * LITTLE_ANU_HEIGHT - 20};
  }

  // A crayon in the hand, in the colour of what's under it; before the first
  // scribble, a ghost crayon shows how (scribbling on the sun)
  void DrawCrayonCursor() const {
    const Vector2 mouse = GetMousePosition();
    const int part = CrayonWorld::PartAt(mouse);
    if (scrubbed < 40 && time > 2.5f) {
      const float t = std::fmod(time - 2.5f, 2.4f);
      const Vector2 ghost = {SUN_CENTRE.x - 24 + 48 * std::fabs(std::sin(t * 5)),
                             SUN_CENTRE.y - 20 + t * 16};
      DrawCrayon(ghost, PART_COLOURS[SUN], 0.55f * Between(t, 0, 0.3f) *
                                               (1 - Between(t, 2.0f, 2.4f)));
    }
    if (!world.Complete(part)) {
      DrawCrayon(mouse, PART_COLOURS[part], 1);
      SceneDrawsOwnCursor(); // the crayon is the pointer here
    }
  }

  // A wax crayon with a paper wrapper, its tip at `tip`
  static void DrawCrayon(Vector2 tip, Color c, float alpha) {
    const Vector2 dir = {0.55f, -0.83f}; // up and to the right
    const Vector2 side = {-dir.y * 7, dir.x * 7};
    auto at = [&](float along, float across) {
      return Vector2{tip.x + dir.x * along + side.x * across,
                     tip.y + dir.y * along + side.y * across};
    };
    FillTriangle(tip, at(16, -1), at(16, 1), Faded(c, alpha));
    const Vector2 body[4] = {at(16, -1), at(70, -1), at(70, 1), at(16, 1)};
    FillTriangle(body[0], body[1], body[2], Faded(c, alpha));
    FillTriangle(body[0], body[2], body[3], Faded(c, alpha));
    const Vector2 wrap[4] = {at(30, -1.05f), at(62, -1.05f), at(62, 1.05f),
                             at(30, 1.05f)};
    FillTriangle(wrap[0], wrap[1], wrap[2], Faded(CRAYON_PAPER, alpha));
    FillTriangle(wrap[0], wrap[2], wrap[3], Faded(CRAYON_PAPER, alpha));
    DrawLineEx(at(36, 0), at(56, 0), 3, Faded(c, alpha));
    for (int i = 0; i < 4; i++)
      DrawLineEx(i < 2 ? body[i] : body[i], body[(i + 1) % 4], 2,
                 Faded(OUTLINE, alpha));
    DrawLineEx(tip, at(16, -1), 2, Faded(OUTLINE, alpha));
    DrawLineEx(tip, at(16, 1), 2, Faded(OUTLINE, alpha));
  }
};

//------------------------------------------------------------------------------
// The BIG one: page 6 of the kolam book, the puzzle as it is everywhere else,
// but lying in the crayon world, with the two of them beside it (on screens
// wide enough to show them).
//------------------------------------------------------------------------------
class BigOneScene : public Scene {
public:
  explicit BigOneScene(const Step &s) : page(MakePuzzleScene(s.page)) {
    world.CompleteAll();
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    world.Update(dt);
    PlayAmbience("birds_day", 0.25f);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
      hop = 1; // little Anu bounces at every change
    hop = std::fmax(0, hop - dt * 2.5f);
    page->Update();
    done = page->done;
  }

  void Draw() override {
    page->Draw(); // (it clears the screen and draws the page)
    const Rectangle all = FullScreen();
    if (all.x > -20) // a square screen: no room beside the page
      return;
    // the crayon world around the page, and a crayon shadow under it
    const Rectangle pageArea = {40, 40, 640, 640};
    DrawRectangleRec({all.x, 0, 40 - all.x, 720}, CRAYON_PAPER);
    DrawRectangleRec({680, 0, all.x + all.width - 680, 720}, CRAYON_PAPER);
    DrawRectangleRec({40, 0, 640, 40}, CRAYON_PAPER);
    DrawRectangleRec({40, 680, 640, 40}, CRAYON_PAPER);
    world.Draw({pageArea.x - 6, pageArea.y - 6, pageArea.width + 12,
                pageArea.height + 12});
    for (int i = 0; i < 3; i++) // the page's shadow, scribbled
      DrawLineEx({46.0f + i * 3, 686.0f + i * 3}, {686.0f + i * 3, 686.0f + i * 3},
                 3, Faded(OUTLINE, 0.25f));
    DrawLineEx({686, 46}, {686, 688}, 4, Faded(OUTLINE, 0.25f));
    // little Anu on the left, looking at the page; Anu on the right
    const float jump = 18 * std::sin(hop * PI);
    DrawLittleAnu(-130, 650 - jump, time, 230);
    DrawAnuSitting(860, 700, time);
  }

private:
  std::unique_ptr<Scene> page;
  CrayonWorld world;
  float time = 0;
  float hop = 0;
};

//------------------------------------------------------------------------------
// Together: the BIG one, finished, lying on the grass in the crayon world.
// "Crooked is fine. Just don't lift." Hold to go over it once more, in one
// line; little Anu adds a flower in the corner. Then the dream lets go.
//------------------------------------------------------------------------------
const Rectangle NOTEBOOK_PAGE = {196, 448, 328, 236};

class TogetherScene : public Scene {
public:
  explicit TogetherScene(const Step &s) : step(s) {
    world.CompleteAll();
    const float cell = 40;
    int across = 0, down = 0;
    std::vector<std::vector<Vector2>> loops =
        SolvedPage(5, {0, 0}, cell, 1.4f, &dots, &across, &down);
    // centre it on the page
    const Vector2 o = {
        NOTEBOOK_PAGE.x + (NOTEBOOK_PAGE.width - cell * across) / 2,
        NOTEBOOK_PAGE.y + 8 + (NOTEBOOK_PAGE.height - 8 - cell * down) / 2};
    for (Vector2 &d : dots)
      d = {d.x + o.x, d.y + o.y};
    if (!loops.empty()) {
      line = loops[0];
      for (Vector2 &p : line)
        p = {p.x + o.x, p.y + o.y};
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    world.Update(dt);
    PlayAmbience("birds_day", 0.3f);
    stageTime += dt;
    switch (stage) {
    case LOOKING:
      if (stageTime > 1.2f) {
        talk.Start(step.lines);
        Next(TALKING);
      }
      break;
    case TALKING:
      if (talk.Update())
        Next(TRACING);
      break;
    case TRACING: { // hold: the crayon goes round, never lifting
      const bool held = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                        IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER);
      if (held && stageTime > 0.3f) {
        traced += dt * std::fmax(440, Length() / 5); // ~5 s
        PlayLoop("pencil", 0.4f);
      }
      if (traced >= Length()) {
        PlayEffect("bell", 0.4f, 1.2f);
        Next(FLOWER);
      }
      break;
    }
    case FLOWER: // little Anu's flower, in the corner
      if (stageTime > 3.4f)
        Next(WAKING);
      break;
    case WAKING:
      if (stageTime > 2.2f)
        done = true;
      break;
    }
  }

  void Draw() override {
    world.Draw();
    // the notebook, open on the grass
    DrawRectangleRec({NOTEBOOK_PAGE.x + 6, NOTEBOOK_PAGE.y + 8,
                      NOTEBOOK_PAGE.width, NOTEBOOK_PAGE.height},
                     Faded(OUTLINE, 0.25f));
    DrawRectangleRec({NOTEBOOK_PAGE.x - 6, NOTEBOOK_PAGE.y - 6,
                      NOTEBOOK_PAGE.width + 12, NOTEBOOK_PAGE.height + 12},
                     {226, 84, 132, 255});
    DrawRectangleRec(NOTEBOOK_PAGE, {252, 250, 242, 255});
    for (float y = NOTEBOOK_PAGE.y + 20; y < NOTEBOOK_PAGE.y + NOTEBOOK_PAGE.height;
         y += 18)
      DrawLineEx({NOTEBOOK_PAGE.x, y}, {NOTEBOOK_PAGE.x + NOTEBOOK_PAGE.width, y},
                 1, PAPER_RULE);
    DrawLineEx({NOTEBOOK_PAGE.x + NOTEBOOK_PAGE.width / 2, NOTEBOOK_PAGE.y},
               {NOTEBOOK_PAGE.x + NOTEBOOK_PAGE.width / 2,
                NOTEBOOK_PAGE.y + NOTEBOOK_PAGE.height},
               2, {220, 200, 190, 255});
    const Color purple = {150, 60, 160, 255}, orange = {240, 120, 40, 255};
    for (const Vector2 &d : dots)
      DrawCircleV(d, 3.5f, purple);
    if (!line.empty()) {
      DrawPath(line, line.size(), true, 4, Faded(purple, 0.9f));
      // the one going over it, together
      const size_t shown = PointsAlong(traced);
      if (shown > 1)
        DrawPath(line, shown, shown >= line.size(), 6.5f, Faded(orange, 0.9f));
      const Vector2 tip = line[std::min(shown, line.size() - 1)];
      if (stage == TRACING) {
        DrawCircleV(tip, 6, orange);
        DrawHint(tip, 14, time, OUTLINE);
      }
    }
    if (stage >= FLOWER) { // a flower in the corner, in pink
      const Vector2 corner = {NOTEBOOK_PAGE.x + NOTEBOOK_PAGE.width - 26,
                              NOTEBOOK_PAGE.y + NOTEBOOK_PAGE.height - 24};
      DrawLittleFlower(corner, 9,
                       {230, 70, 130, 255}, 3.5f,
                       Between(stage == FLOWER ? stageTime : 9, 0.5f, 2.5f));
    }
    // Anu on her heels at the page, little Anu beside her
    DrawAnuSitting(110, 700, time);
    const float bounce = stage == FLOWER ? 10 * std::fabs(std::sin(time * 6)) : 0;
    const float herTime = stage == FLOWER ? time : time * 0.5f;
    Mirrored(630, [&]() { DrawLittleAnu(630, 690 - bounce, herTime, 200); });

    talk.Draw([](const std::string &who) {
      if (who == "Anu")
        return BubbleSpot{{200, 330}, {120, 545}};
      return BubbleSpot{{520, 300}, {620, 490}};
    });
    if (stage == WAKING) // the dream lets go
      FadeScreen(CRAYON_PAPER, Between(stageTime, 0, 2));
  }

private:
  enum Stage { LOOKING, TALKING, TRACING, FLOWER, WAKING };
  const Step &step;
  CrayonWorld world;
  Conversation talk;
  Stage stage = LOOKING;
  float time = 0, stageTime = 0;
  std::vector<Vector2> dots;
  std::vector<Vector2> line; // the kolam's one line
  float traced = 0;          // how far along it the crayon has gone

  void Next(Stage s) {
    stage = s;
    stageTime = 0;
  }

  float Length() const {
    float l = 0;
    for (size_t i = 1; i < line.size(); i++)
      l += std::hypot(line[i].x - line[i - 1].x, line[i].y - line[i - 1].y);
    return line.empty() ? 0 : l;
  }

  size_t PointsAlong(float distance) const {
    float l = 0;
    for (size_t i = 1; i < line.size(); i++) {
      l += std::hypot(line[i].x - line[i - 1].x, line[i].y - line[i - 1].y);
      if (l > distance)
        return i;
    }
    return line.size();
  }
};

//------------------------------------------------------------------------------
// 4:50 am: the hall at night. Anu is sitting up on her mat, the kolam book on
// her lap, already awake when the alarm goes. Slide it off.
//------------------------------------------------------------------------------
const Rectangle PHONE = {470, 170, 210, 380};
const Rectangle SLIDER = {496, 450, 158, 50};

static void DrawAwakeAnu(float time) {
  // her mat, the book, the phone lighting up the floor
  const Vector2 mat[4] = {{120, 690}, {420, 690}, {460, 640}, {170, 640}};
  FillTriangle(mat[0], mat[1], mat[2], {150, 120, 70, 255});
  FillTriangle(mat[0], mat[2], mat[3], {150, 120, 70, 255});
  for (int i = 0; i < 6; i++)
    InkLine({140.0f + i * 50, 688}, {180.0f + i * 48, 642}, 1.2f,
            {110, 84, 44, 255});
  InkPoly(mat, 4, 2.4f);
  DrawAnuSitting(270, 676, time);
  // the kolam book in her hands, its red cloth cover
  const Vector2 book[4] = {{278, 630}, {326, 624}, {330, 640}, {282, 646}};
  ComicPoly(book, 4, {150, 46, 40, 255}, 2);
}

class AlarmScene : public Scene {
public:
  explicit AlarmScene(const Step &) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("crickets", 0.4f);
    if (stage == WAITING) { // a moment of the dark, quiet hall
      if (time > 2.0f)
        stage = RINGING, stageTime = 0;
    } else if (stage == RINGING) {
      stageTime += dt;
      buzz -= dt;
      if (buzz <= 0) {
        PlayEffect("vibrate", 0.8f);
        buzz = 1.1f;
      }
      const Vector2 m = GetMousePosition();
      const Vector2 knob = Knob();
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
          CheckCollisionPointRec(m, {SLIDER.x - 10, SLIDER.y - 20,
                                     SLIDER.width + 20, SLIDER.height + 40}))
        dragging = true, grabX = m.x - knob.x;
      if (dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        slide = std::fmax(0, std::fmin(1, (m.x - grabX - SLIDER.x - 25) /
                                              (SLIDER.width - 50)));
      if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        dragging = false;
        if (slide < 0.7f) // a tap, or not far enough: it slides itself
          slide = std::fmax(slide, 0.71f);
      }
      if (!dragging && slide > 0 && slide < 1)
        slide = std::fmin(1, slide + dt * 3);
      if (slide >= 1) {
        PlayEffect("click", 0.6f);
        stage = QUIET, stageTime = 0;
      }
    } else {
      stageTime += dt;
      if (stageTime > 6 || (stageTime > 2.5f && ContinuePressed()))
        done = true;
    }
  }

  void Draw() override {
    DrawPlace("hall", NIGHT, time, DrawAwakeAnu);
    const float ring = stage == RINGING ? 1 : 0;
    const float away = stage == QUIET ? Between(stageTime, 0, 0.8f) : 0;
    // the phone on the floor, lighting her up: she's sitting up, awake
    const float shake = ring * 2 * std::sin(time * 60);
    const float light = stage == WAITING ? 0.35f
                        : stage == RINGING ? 1.0f
                                           : 1 - 0.6f * away;
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient({360, 600}, 230, Faded({120, 140, 200, 255}, 0.55f * light),
                       {120, 140, 200, 0});
    DrawCircleGradient({440 + shake, 660}, 70, Faded({170, 200, 255, 255}, 0.4f * light),
                       {170, 200, 255, 0});
    EndBlendMode();
    ComicRect({418 + shake, 652, 46, 14}, ring > 0 ? Color{190, 214, 255, 255}
                                                   : Color{40, 40, 50, 255},
              2);
    if (stage == WAITING)
      return;
    // up close: the alarm, 4:50, and its slider
    const float y = (stage == RINGING ? (1 - Between(stageTime, 0, 0.4f)) : away) * 560;
    rlPushMatrix();
    rlTranslatef(shake * 0.5f, y, 0);
    DrawRectangleRounded({PHONE.x + 6, PHONE.y + 8, PHONE.width, PHONE.height},
                         0.12f, 8, {0, 0, 0, 90});
    DrawRectangleRounded(PHONE, 0.12f, 8, {24, 24, 30, 255});
    const Rectangle screen = {PHONE.x + 10, PHONE.y + 12, PHONE.width - 20,
                              PHONE.height - 24};
    DrawRectangleRounded(screen, 0.1f, 8, {30, 40, 74, 255});
    DrawRectangleRoundedLinesEx(PHONE, 0.12f, 8, 3, COMIC_INK);
    DrawTextCentered("4:50", PHONE.x + PHONE.width / 2, PHONE.y + 70, 64,
                     FLOUR);
    // a bell, swinging
    const Vector2 bell = {PHONE.x + PHONE.width / 2, PHONE.y + 210};
    const float swing = 0.35f * std::sin(time * 18) * ring;
    rlPushMatrix();
    rlTranslatef(bell.x, bell.y - 26, 0);
    rlRotatef(swing * 57.3f, 0, 0, 1);
    DrawCircleSector({0, 20}, 26, 180, 360, 16, {250, 200, 90, 255});
    DrawRectangle(-26, 20, 52, 14, {250, 200, 90, 255});
    DrawEllipse(0, 34, 32, 6, {250, 200, 90, 255});
    DrawCircle(0, 42, 6, {250, 200, 90, 255});
    DrawCircle(0, -8, 5, {250, 200, 90, 255});
    rlPopMatrix();
    for (int side = -1; side <= 1; side += 2) // ringing marks
      for (int k = 0; k < 2; k++)
        DrawRing(bell, 44 + k * 12, 47 + k * 12, side > 0 ? 300 : 120,
                 side > 0 ? 360 : 180, 10,
                 Faded(FLOUR, ring * (0.5f + 0.5f * std::sin(time * 10 + k))));
    // the slider: a track with a knob; slide it to the end
    DrawRectangleRounded(SLIDER, 1, 12, {60, 70, 110, 255});
    for (int i = 0; i < 3; i++) { // chevrons, moving along
      const float x = SLIDER.x + 70 + i * 22 +
                      6 * std::fmod(time * 2 + i * 0.3f, 1.0f);
      const float a = 0.3f + 0.5f * std::fmod(time * 1.5f + i * 0.33f, 1.0f);
      DrawLineEx({x, SLIDER.y + 16}, {x + 9, SLIDER.y + 25}, 3, Faded(FLOUR, a));
      DrawLineEx({x + 9, SLIDER.y + 25}, {x, SLIDER.y + 34}, 3, Faded(FLOUR, a));
    }
    const Vector2 knob = Knob();
    DrawCircleV(knob, 21, {236, 92, 70, 255});
    DrawCircleLinesV(knob, 21, COMIC_INK);
    DrawLineEx({knob.x - 7, knob.y - 7}, {knob.x + 7, knob.y + 7}, 3, FLOUR);
    DrawLineEx({knob.x + 7, knob.y - 7}, {knob.x - 7, knob.y + 7}, 3, FLOUR);
    if (stage == RINGING && !dragging)
      DrawHint(knob, 24, time);
    rlPopMatrix();
  }

private:
  enum Stage { WAITING, RINGING, QUIET };
  Stage stage = WAITING;
  float time = 0, stageTime = 0;
  float buzz = 0;
  float slide = 0; // 0..1 along the slider
  bool dragging = false;
  float grabX = 0;

  Vector2 Knob() const {
    return {SLIDER.x + 25 + slide * (SLIDER.width - 50),
            SLIDER.y + SLIDER.height / 2};
  }
};

std::unique_ptr<Scene> MakeCrayonWorld(const Step &step) {
  return std::unique_ptr<Scene>(new CrayonWorldScene(step));
}
std::unique_ptr<Scene> MakeBigOne(const Step &step) {
  return std::unique_ptr<Scene>(new BigOneScene(step));
}
std::unique_ptr<Scene> MakeTogether(const Step &step) {
  return std::unique_ptr<Scene>(new TogetherScene(step));
}
std::unique_ptr<Scene> MakeAlarm(const Step &step) {
  return std::unique_ptr<Scene>(new AlarmScene(step));
}

} // namespace

PLAY_SCENE("crayon_world", MakeCrayonWorld)
PLAY_SCENE("big_one", MakeBigOne)
PLAY_SCENE("together", MakeTogether)
PLAY_SCENE("alarm_450", MakeAlarm)
