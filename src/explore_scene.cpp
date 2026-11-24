// explore_scene.cpp - walking around Anu's home, a village on the Bhavani
// between Mettupalayam and Sathyamangalam.
//
// The house has four rooms. Each room is drawn here (simple shapes for now,
// until the artists' paintings replace them). ROOM_OBJECTS says where each
// clickable thing is; story.cpp says what happens when it is clicked.
// Finding a memory brings colour back to the world. Now and then work calls;
// Anu can answer or let it go, and neither changes anything.
#include <cmath>

#include "game.h"

enum Room { FRONT_STEP, HALL, KITCHEN, TERRACE, ROOM_COUNT };
// The names story.cpp uses for them, e.g. Talk(...).At("terrace", NIGHT)
const char *PLACE_IDS[ROOM_COUNT] = {"front_step", "hall", "kitchen",
                                     "terrace"};

struct RoomObject {
  const char *object;
  Room room;
  Rectangle area;
};

// Where every clickable object is. Story steps refer to these by name.
const RoomObject ROOM_OBJECTS[] = {
    {"old_kolam", FRONT_STEP, {200, 560, 320, 80}},
    {"tulsi", FRONT_STEP, {60, 390, 120, 190}},
    {"bicycle", FRONT_STEP, {520, 400, 190, 170}},
    {"family_photo", HALL, {455, 110, 120, 150}},
    {"swing", HALL, {110, 300, 270, 200}},
    {"height_marks", HALL, {575, 150, 50, 400}}, // the kitchen doorframe
    {"calendar", HALL, {372, 130, 66, 96}},
    {"amma", KITCHEN, {370, 220, 140, 380}},
    {"grinding_stone", KITCHEN, {60, 480, 200, 110}},
    {"flour_tin", KITCHEN, {560, 170, 90, 100}},
    {"coffee_filter", KITCHEN, {270, 320, 80, 100}},
    {"vessels", KITCHEN, {50, 96, 480, 56}},
    {"kite_reel", TERRACE, {60, 450, 190, 140}},
    {"storage_boxes", TERRACE, {270, 500, 180, 120}},
    {"temple", TERRACE, {430, 190, 120, 200}},
};

// How the rooms connect: click a door to walk through it. The hall (koodam)
// is the middle of the house, as in a village house: the front door opens
// into it, the curtained doorway goes on to the kitchen, and the stairs go up
// to the terrace (maadi). The way you came in is behind you, at the bottom of
// the picture. So every room is one or two doors from every other.
// `area` is where the door is in the `from` room; it never overlaps an object
// above (the hall's doorway leaves the doorframe to the pencil marks).
// Some doors depend on the screen's width. The hall's front door is left of
// the stage, so only wide screens show it; on narrower ones you walk out
// through the threshold behind you instead.
enum DoorShows { ALWAYS, WIDE_SCREENS, NARROW_SCREENS };
struct Door {
  Room from, to;
  const char *name; // shown when the mouse is over it: where it leads
  Rectangle area;
  DoorShows shows = ALWAYS;
};
const Rectangle HALL_FRONT_DOOR = {-240, 138, 136, 422}; // drawn in DrawHall
const Door DOORS[] = {
    {FRONT_STEP, HALL, "Into the hall", {276, 220, 168, 338}}, // front door
    {HALL, KITCHEN, "Into the kitchen", {626, 150, 92, 395}},  // the curtain
    {HALL, TERRACE, "Up to the terrace", {412, 266, 152, 279}}, // the stairs
    {HALL, FRONT_STEP, "Out to the front step", HALL_FRONT_DOOR, WIDE_SCREENS},
    {HALL, FRONT_STEP, "Out to the front step", {220, 616, 280, 104},
     NARROW_SCREENS}, // the threshold behind you
    {KITCHEN, HALL, "Back to the hall", {230, 620, 260, 100}},
    {TERRACE, HALL, "Down to the hall", {476, 616, 230, 104}},
};

// Is the screen wide enough to show the hall's front door?
static bool WideHall() { return FullScreen().x <= HALL_FRONT_DOOR.x - 4; }

// Can this door be used on this screen?
static bool DoorShown(const Door &d) {
  if (d.shows == WIDE_SCREENS)
    return WideHall();
  if (d.shows == NARROW_SCREENS)
    return !WideHall();
  return true;
}

//------------------------------------------------------------------------------
// Drawing the rooms
//------------------------------------------------------------------------------
// The home is drawn like a page of an old Tamil children's comic: flat printed
// colours, bold brush ink (heavier on the lower right, away from the light),
// halftone dots for shade and skies, and newsprint paper. The ink and dot
// helpers (InkLine, ComicRect, BeginTone...) are at the end of art.cpp.

// The printer's colours for the house
const Color TEAK = {164, 94, 46, 255};
const Color TEAK_DARK = {112, 58, 30, 255};
const Color TEAK_LIGHT = {212, 140, 74, 255};
const Color BRASS_YELLOW = {244, 188, 56, 255};
const Color BRASS_SHADE = {196, 126, 36, 255};
const Color STEEL = {206, 212, 222, 255};
const Color RED_OXIDE = {204, 64, 44, 255};
const Color LIME_WASH = {252, 222, 140, 255};
const Color SHUTTER_GREEN = {48, 152, 110, 255};
const Color SUNSET_DOTS = {224, 62, 100, 255}; // magenta dots on yellow
const Color PALM_TRUNK = {150, 98, 56, 255};
const Color PALM_GREEN = {58, 152, 74, 255};

// A sunset sky as it's printed: warm yellow under a screen of magenta dots.
// By day the low sun sits in it, its rays fanning out under the dots.
static void DrawSunsetSky(float bottom, Vector2 sun, float sunRadius,
                          bool night, float time) {
  DrawBandGradient(0, bottom, {255, 178, 104, 255}, {255, 230, 160, 255});
  for (int i = 0; i < 12 && !night; i++) { // turning very slowly
    const float a = i * PI / 6 + time * 0.01f, b = a + PI / 12;
    if (std::sin(a) > 0.1f && std::sin(b) > 0.1f)
      continue; // pointing down, behind the hills: never seen
    FillTriangle(sun, {sun.x + std::cos(a) * 1500, sun.y + std::sin(a) * 1500},
                 {sun.x + std::cos(b) * 1500, sun.y + std::sin(b) * 1500},
                 {255, 242, 196, 110});
  }
  BeginTone(TONE_DOTS, 7); // at night the dots are printed in deep blue
  const Color dots = night ? Color{14, 18, 64, 255} : SUNSET_DOTS;
  DrawBandGradient(0, bottom * 0.8f, Faded(dots, 0.8f), Faded(dots, 0));
  EndTone();
  if (night)
    return;
  DrawCircleGradient(sun, sunRadius * 2.4f, {255, 246, 206, 170},
                     {255, 246, 206, 0});
  ComicCircle(sun, sunRadius, {255, 242, 186, 255}, 2.2f);
}

// A long low cloud: a row of puffs on a flat bottom, inked round the outside,
// lit pink from underneath
static void DrawLongCloud(float x, float y, float length) {
  for (int pass = 0; pass < 2; pass++) {
    const float grow = pass == 0 ? 2.4f : 0;
    const Color c = pass == 0 ? COMIC_INK : Color{255, 232, 214, 255};
    for (int k = 0; k < 6; k++) {
      const float r = 7.0f + (k % 3 == 1 ? 6 : (k % 2) * 3);
      DrawEllipse((int)(x - length / 2 + (k + 0.5f) * length / 6),
                  (int)(y - r * 0.5f), length / 9 + grow, r + grow, c);
    }
    DrawRectangleRec({x - length / 2 - grow, y - 3, length + 2 * grow,
                      7 + grow},
                     c);
  }
  DrawRectangleRec({x - length / 2 + 4, y + 1, length - 8, 3},
                   {250, 156, 140, 255});
}

// A comic cloud: round puffs on a flat bottom, inked round the outside
static void DrawCloud(float x, float y, float size) {
  const Vector2 PUFFS[] = {{-0.75f, -0.2f}, {-0.35f, -0.45f}, {0.1f, -0.6f},
                           {0.55f, -0.4f}, {0.85f, -0.18f}};
  const float RADII[] = {0.3f, 0.4f, 0.48f, 0.38f, 0.28f};
  for (int pass = 0; pass < 2; pass++) {
    const float grow = pass == 0 ? 2.6f : 0;
    const Color c = pass == 0 ? COMIC_INK : Color{255, 238, 222, 255};
    for (int i = 0; i < 5; i++)
      DrawCircleV({x + PUFFS[i].x * size, y + PUFFS[i].y * size},
                  RADII[i] * size + grow, c);
    DrawRectangleRec(
        {x - 1.05f * size - grow, y - 0.2f * size, 2.15f * size + 2 * grow,
         0.2f * size + grow},
        c);
  }
  // the underside lit pink by the sunset
  DrawRectangleRec({x - 1.05f * size, y - 0.1f * size, 2.15f * size,
                    0.1f * size},
                   {252, 172, 160, 255});
}

// A wooden window with iron bars and green shutters folded back. From inside
// it shows the evening outside; from the street, the dim room behind it.
static void DrawWindow(Rectangle r, bool lookingOut, float time) {
  const float shutter = r.width * 0.32f, h = r.height + 12;
  for (int side = 0; side < 2; side++) {
    const float x = side == 0 ? r.x - 14 - shutter : r.x + r.width + 14;
    ComicRect({x, r.y - 6, shutter, h}, SHUTTER_GREEN, 2.2f);
    for (int k = 0; k < 2; k++)
      InkRect({x + 5, r.y - 6 + 6 + k * h / 2, shutter - 10, h / 2 - 12}, 1.3f);
  }
  ComicRect({r.x - 10, r.y - 10, r.width + 20, r.height + 20}, TEAK, 2.6f);
  if (lookingOut) {
    DrawRectangleGradientV((int)r.x, (int)r.y, (int)r.width, (int)r.height,
                           {255, 190, 116, 255}, {255, 236, 176, 255});
    DrawToneRect({r.x, r.y, r.width, r.height * 0.75f}, SUNSET_DOTS, 0.6f, 0,
                 TONE_DOTS, 5);
    DrawComicPalm(r.x + r.width * 0.45f, r.y + r.height, r.height * 0.75f,
                  time, PALM_TRUNK, PALM_GREEN);
  } else { // a dim room, a yellow cotton curtain half across
    DrawRectangleRec(r, {72, 52, 62, 255});
    DrawToneRect(r, COMIC_INK, 0.25f, 0.6f, TONE_LINES, 4);
    const Rectangle curtain = {r.x, r.y, r.width * 0.42f, r.height};
    DrawRectangleRec(curtain, {240, 190, 60, 255});
    for (int i = 1; i < 4; i++)
      InkLine({curtain.x + i * curtain.width / 4, r.y + 4},
              {curtain.x + i * curtain.width / 4 + 2, r.y + r.height - 4},
              1.4f);
    DrawToneRect(curtain, {196, 90, 30, 255}, 0.1f, 0.5f, TONE_DOTS, 4);
  }
  for (int i = 1; i < 5; i++) { // iron bars, a glint down each
    const float x = r.x + i * r.width / 5;
    DrawLineEx({x, r.y}, {x, r.y + r.height}, 3.4f, COMIC_INK);
    DrawLineEx({x - 0.6f, r.y + 2}, {x - 0.6f, r.y + r.height - 2}, 1,
               {170, 170, 182, 255});
  }
  InkRect(r, 2.0f);
  ComicRect({r.x - 16, r.y + r.height + 8, r.width + 32, 9}, TEAK_LIGHT,
            2.2f); // the sill
}

// Amma's little clay lamp; `glow` draws its light as warm dots
static void DrawClayLamp(Vector2 c, float time) {
  BeginTone(TONE_DOTS, 4);
  DrawCircleGradient({c.x, c.y - 10}, 30, {255, 214, 90, 220},
                     {255, 214, 90, 0});
  EndTone();
  const float flicker = 1.5f * std::sin(time * 9) + std::sin(time * 23);
  FillTriangle({c.x - 3.5f, c.y - 4}, {c.x + 3.5f, c.y - 4},
               {c.x + 0.6f * flicker, c.y - 17 - flicker}, {255, 176, 40, 255});
  DrawCircleV({c.x, c.y - 5}, 3.6f, {255, 176, 40, 255});
  DrawCircleV({c.x, c.y - 6}, 1.8f, {255, 246, 200, 255});
  DrawCircleSector(c, 9, 0, 180, 12, {196, 92, 52, 255}); // the agal
  DrawRing(c, 8.5f, 10.5f, 0, 180, 12, COMIC_INK);
  DrawLineEx({c.x - 10, c.y}, {c.x + 10, c.y}, 1.6f, COMIC_INK);
}

// A string of mango leaves (thoranam) with marigolds, over a doorway
static void DrawThoranam(float x0, float x1, float y, float time) {
  DrawSplineSegmentBezierQuadratic({x0 - 4, y - 2}, {(x0 + x1) / 2, y + 14},
                                   {x1 + 4, y - 2}, 2.2f, COMIC_INK);
  const int count = (int)((x1 - x0) / 18) + 1;
  for (int i = 0; i < count; i++) {
    const float t = (float)i / (count - 1);
    const float x = x0 + t * (x1 - x0), sag = y + 7 * std::sin(PI * t);
    const float sway = 1.5f * std::sin(time * 2 + i);
    DrawComicLeaf({x, sag}, {x + sway, sag + 30}, 11,
                  i % 2 ? Color{56, 140, 58, 255} : Color{96, 168, 64, 255},
                  1.4f);
  }
  for (int i = 1; i < 4; i++) { // marigolds between the leaves
    const float t = i / 4.0f;
    const Vector2 c = {x0 + t * (x1 - x0), y + 7 * std::sin(PI * t) + 2};
    DrawCircleSector({c.x + 0.6f, c.y + 0.6f}, 7.6f, 0, 360, 12, COMIC_INK);
    for (int p = 0; p < 8; p++) {
      const float a = p * PI / 4;
      DrawCircleSector({c.x + 3.4f * std::cos(a), c.y + 3.4f * std::sin(a)},
                       3.2f, 0, 360, 6, {250, 150, 30, 255});
    }
    DrawCircleSector(c, 2.6f, 0, 360, 6, {226, 96, 20, 255});
  }
}

// Kolam lines in rice flour: drawn by a hand that isn't as steady as it was
static void DrawShakyRing(Vector2 c, float rx, float ry, float from, float to,
                          int seed, Color flour) {
  const int n = 14;
  Vector2 prev = {};
  for (int i = 0; i <= n; i++) {
    const float a = from + (to - from) * i / n;
    const float wobble = 1.2f * std::sin(seed * 7.1f + i * 2.3f);
    const Vector2 p = {c.x + (rx + wobble) * std::cos(a),
                       c.y + (ry + wobble * 0.6f) * std::sin(a)};
    if (i > 0)
      DrawLineEx(prev, p, 2.6f, flour);
    prev = p;
  }
}

static void DrawFrontStep(float time, bool night) {
  const Rectangle all = FullScreen();
  const float left = all.x, right = all.x + all.width;
  // the evening sky over the blue Nilgiri hills, palms against it
  DrawSunsetSky(200, {560, 118}, 28, night, time);
  if (!night) {
    for (int i = 0; i < 3; i++)
      DrawCloud(left + std::fmod(i * 520 + time * 5, all.width + 300) - 150,
                54.0f + (i % 2) * 26, 30.0f + i * 6);
  }
  DrawComicHills(140, 200, 80, 1, {156, 164, 226, 255}, {86, 96, 176, 255});
  DrawComicHills(172, 200, 40, 4, {112, 128, 200, 255}, {60, 72, 150, 255});
  DrawComicPalm(left + 120, 200, 170, time, PALM_TRUNK, PALM_GREEN);
  DrawComicPalm(660, 200, 150, time, PALM_TRUNK, PALM_GREEN);
  DrawComicPalm(right - 90, 200, 190, time, PALM_TRUNK, PALM_GREEN);

  // the roof: rows of half-round clay tiles, the eaves, the rafter ends
  DrawBand(146, 50, {220, 100, 58, 255});
  for (int row = 0; row < 3; row++) {
    const float y = 158.0f + row * 14;
    DrawToneRect({left, y - 12, all.width, 13}, {120, 28, 20, 255}, 0, 0.6f,
                 TONE_DOTS, 4);
    for (float x = left - 20 + row * 9; x < right + 20; x += 18) {
      DrawCircleSector({x, y}, 9, 0, 180, 6, {214, 92, 52, 255});
      DrawRing({x, y}, 8.2f, 10.2f, 0, 180, 6, COMIC_INK);
    }
  }
  DrawBand(145, 3, COMIC_INK);
  DrawBand(192, 10, TEAK_DARK);
  DrawBand(191, 2, COMIC_INK);
  DrawBand(201, 3, COMIC_INK);

  // the lime-washed wall: flat yellow, dots in the shade under the eaves and
  // at its foot, old patches and a crack or two
  DrawBand(203, 360, LIME_WASH);
  DrawToneRect({left, 203, all.width, 90}, {200, 116, 40, 255}, 0.7f, 0,
               TONE_DOTS, 5);
  DrawToneRect({left, 203, all.width, 18}, COMIC_INK, 0.55f, 0, TONE_LINES,
               4);
  DrawToneRect({left, 380, all.width, 100}, {200, 116, 40, 255}, 0, 0.55f,
               TONE_DOTS, 5);
  for (float x = left + 10; x < right; x += 44) // rafter ends
    ComicRect({x, 203, 10, 8}, TEAK, 1.4f);
  const Vector2 PATCHES[] = {{120, 300},  {480, 400}, {650, 380},
                             {-150, 330}, {860, 420}, {210, 440}};
  for (Vector2 p : PATCHES) { // fresh lime over old, inked on the shadow side
    DrawEllipse((int)p.x, (int)p.y, 30, 10, {255, 236, 180, 255});
    InkCurve({p.x - 20, p.y + 8}, {p.x + 6, p.y + 14}, {p.x + 28, p.y + 3},
             1.2f);
  }
  InkLine({-110, 236}, {-98, 268}, 1.6f); // cracks in the plaster
  InkLine({-98, 268}, {-104, 296}, 1.4f);
  InkLine({-98, 268}, {-80, 280}, 1.2f);
  InkLine({770, 228}, {782, 256}, 1.6f);
  InkLine({782, 256}, {776, 284}, 1.3f);
  DrawWindow({-190, 260, 110, 130}, false, time);
  DrawWindow({820, 260, 110, 130}, false, time);
  DrawComicJasmine({450, 214, 110, 150}, time); // Amma's malli, by the door
  DrawComicJasmine({left + 10, 214, 140, 120}, time);

  // a little arched niche in the wall with Amma's clay lamp, lit at dusk
  DrawCircleSector({227, 380}, 23, 180, 360, 16, {120, 62, 44, 255});
  DrawRectangleRec({204, 380, 46, 36}, {120, 62, 44, 255});
  DrawRing({227, 380}, 22, 25, 180, 360, 16, COMIC_INK);
  DrawLineEx({205, 380}, {205, 416}, 2.4f, COMIC_INK);
  DrawLineEx({249, 380}, {249, 416}, 3.4f, COMIC_INK);
  DrawClayLamp({227, 408}, time);
  ComicRect({198, 414, 58, 7}, {236, 196, 118, 255}, 1.8f);

  // thinnai: the raised red-oxide seats either side of the door
  for (int side = 0; side < 2; side++) {
    const float x0 = side == 0 ? left - 4 : 460;
    const float x1 = side == 0 ? 260 : right + 4;
    ComicRect({x0, 478, x1 - x0, 82}, {184, 56, 42, 255}, 3);
    DrawToneRect({x0, 480, x1 - x0, 80}, COMIC_INK, 0.05f, 0.5f, TONE_DOTS,
                 5);
    ComicRect({x0, 464, x1 - x0, 14}, {232, 104, 74, 255}, 2.4f);
  }

  // the door: a carved teak frame, two leaves with panels and brass studs
  ComicRect({276, 220, 168, 340}, TEAK_DARK, 3.6f);
  for (int i = 0; i < 5; i++) // little rosettes carved in the lintel
    InkCircle({300.0f + i * 30, 226}, 3.2f, 1.3f);
  DrawRectangle(290, 232, 140, 328, {52, 28, 20, 255});
  for (int side = 0; side < 2; side++) {
    const float x = 294 + side * 67.0f;
    ComicRect({x, 238, 65, 320}, TEAK, 2.6f);
    const Rectangle PANELS[2] = {{x + 9, 252, 47, 118}, {x + 9, 388, 47, 152}};
    for (const Rectangle &p : PANELS) { // lit at the top and left
      DrawRectangleRec(p, TEAK_DARK);
      DrawRectangleRec({p.x, p.y, p.width, 4}, TEAK_LIGHT);
      DrawRectangleRec({p.x, p.y, 4, p.height}, TEAK_LIGHT);
      const Rectangle in = {p.x + 4, p.y + 4, p.width - 8, p.height - 8};
      DrawRectangleRec(in, TEAK);
      InkCurve({in.x + 10, in.y + 8}, {in.x + 18, in.y + in.height / 2},
               {in.x + 12, in.y + in.height - 8}, 1.0f, TEAK_DARK);
      InkCurve({in.x + 26, in.y + 14}, {in.x + 32, in.y + in.height / 2},
               {in.x + 27, in.y + in.height - 14}, 0.9f, TEAK_DARK);
      InkRect(p, 1.8f);
      InkRect(in, 1.0f);
      for (int k = 0; k < 4; k++) { // brass studs at the corners
        const Vector2 s = {p.x + (k % 2) * p.width, p.y + (k / 2) * p.height};
        ComicCircle(s, 3.4f, BRASS_YELLOW, 1.2f);
        DrawCircleV({s.x - 1, s.y - 1}, 1, {255, 252, 230, 255});
      }
    }
  }
  DrawLineEx({360, 238}, {360, 558}, 2.6f, COMIC_INK); // between the leaves
  for (float x : {351.0f, 369.0f}) { // brass ring handles
    DrawRing({x, 404}, 5, 9.6f, 0, 360, 20, COMIC_INK);
    DrawRing({x, 404}, 6, 8.6f, 0, 360, 20, BRASS_YELLOW);
    ComicCircle({x, 396}, 3, BRASS_YELLOW, 1.2f);
  }
  DrawToneRect({290, 232, 140, 70}, COMIC_INK, 0.5f, 0, TONE_LINES, 4);
  DrawThoranam(282, 438, 224, time);

  // the step, and the threshold stone painted with turmeric and kumkum
  DrawBand(560, 22, {204, 178, 142, 255});
  DrawBand(559, 2.5f, COMIC_INK);
  DrawBand(581, 3, COMIC_INK);
  ComicRect({270, 560, 180, 21}, {216, 210, 198, 255}, 2.6f);
  DrawRectangleRec({274, 566, 172, 8}, {248, 196, 40, 255});
  for (int i = 0; i < 5; i++)
    DrawCircleV({292.0f + i * 34, 570}, 2.6f, {210, 30, 50, 255});

  // tulsi maadam: the little shrine planter, whitewashed with a red border
  DrawComicShadow(120, 576, 140);
  DrawRectangle(84, 474, 72, 92, {252, 248, 236, 255});
  DrawRectangle(134, 474, 22, 92, {214, 206, 190, 255});
  InkRect({84, 474, 72, 92}, 3);
  ComicRect({76, 460, 88, 16}, RED_OXIDE, 2.4f);
  ComicRect({78, 562, 84, 10}, RED_OXIDE, 2.0f);
  ComicRect({100, 492, 40, 38}, RED_OXIDE, 1.8f);
  DrawRectangle(105, 497, 30, 28, {84, 40, 32, 255});
  BeginTone(TONE_DOTS, 3);
  DrawCircleGradient({120, 514}, 14, {255, 210, 90, 230}, {255, 210, 90, 0});
  EndTone();
  for (int i = 0; i < 4; i++) { // painted kolam dots
    DrawCircleV({92, 486.0f + i * 18}, 2, RED_OXIDE);
    DrawCircleV({146, 486.0f + i * 18}, 2, RED_OXIDE);
  }
  const Vector2 root = {120, 462};
  for (int i = 0; i < 3; i++)
    InkLine(root, {100.0f + i * 20, 404.0f + (i % 2) * 8}, 2.2f,
            {70, 90, 40, 255});
  for (int i = 0; i < 16; i++) {
    const Vector2 p = {120 + 26 * std::sin(i * 2.4f),
                       448 - (i % 5) * 11 + 2 * std::sin(time + i)};
    const float dx = p.x - root.x, dy = p.y - root.y;
    const float len = std::fmax(1, std::sqrt(dx * dx + dy * dy));
    DrawComicLeaf(p, {p.x + dx / len * 14, p.y + dy / len * 14 - 3}, 10,
                  i % 2 ? Color{48, 128, 62, 255} : Color{84, 164, 76, 255},
                  1.2f);
  }
  for (int i = 0; i < 3; i++) // purple flower spikes
    InkLine({102.0f + i * 18, 402.0f + (i % 2) * 6},
            {100.0f + i * 19, 390.0f + (i % 2) * 6}, 3, {130, 70, 140, 255});

  // Anu's old red bicycle, leaning on the thinnai
  DrawComicShadow(615, 576, 200);
  const Color red = {222, 40, 50, 255};
  for (float cx : {560.0f, 670.0f}) {
    const Vector2 c = {cx, 540};
    for (int i = 0; i < 12; i++) { // spokes
      const float a = i * PI / 6;
      DrawLineEx(c, {c.x + 22 * std::cos(a), c.y + 22 * std::sin(a)}, 1,
                 {80, 80, 92, 255});
    }
    DrawRing(c, 21, 24.5f, 0, 360, 24, STEEL);
    DrawRing(c, 20.5f, 21.5f, 0, 360, 24, COMIC_INK);
    DrawRing(c, 24.5f, 31.5f, 0, 360, 24, COMIC_INK); // the tyre
    DrawRing(c, 27, 28.4f, 200, 250, 8, {140, 140, 150, 255});
    DrawRing(c, 33.5f, 37, 200, 340, 14, STEEL); // mudguard
    DrawRing(c, 32.5f, 33.5f, 200, 340, 14, COMIC_INK);
    DrawRing(c, 37, 38.4f, 200, 340, 14, COMIC_INK);
    ComicCircle(c, 3.6f, STEEL, 1.2f);
  }
  const Vector2 rear = {560, 540}, crank = {604, 540}, seat = {612, 482};
  const Vector2 head = {658, 470}, headLow = {662, 490}, fork = {670, 540};
  const Vector2 TUBES[][2] = {{seat, crank}, {seat, head},   {crank, headLow},
                              {rear, crank}, {rear, seat},   {head, fork}};
  for (int pass = 0; pass < 2; pass++) {
    for (const auto &t : TUBES) { // ink underneath, then the red paint
      DrawLineEx(t[0], t[1], pass == 0 ? 8.5f : 5, pass == 0 ? COMIC_INK : red);
      DrawCircleV(t[0], pass == 0 ? 4.2f : 2.5f, pass == 0 ? COMIC_INK : red);
    }
  }
  DrawLineEx({seat.x - 1, seat.y + 4}, {head.x - 3, head.y + 2}, 1.2f,
             {255, 170, 170, 255}); // shine
  const Vector2 guard[4] = {{556, 531}, {612, 529}, {614, 547}, {558, 549}};
  ComicPoly(guard, 4, {56, 56, 66, 255}, 1.6f); // the chain guard
  DrawLineEx({562, 534}, {606, 532}, 1.2f, {150, 150, 166, 255});
  InkLine(crank, {612, 558}, 3);
  ComicRect({606, 556, 14, 5}, {60, 60, 66, 255}, 1.2f); // pedal
  DrawLineEx(seat, {610, 470}, 5, COMIC_INK);
  const Vector2 saddle[4] = {{592, 464}, {626, 466}, {618, 476}, {598, 474}};
  ComicPoly(saddle, 4, {84, 50, 36, 255}, 1.8f);
  DrawLineEx({548, 490}, {606, 486}, 3.2f, COMIC_INK); // the carrier
  DrawLineEx({552, 490}, {560, 532}, 2, COMIC_INK);
  DrawLineEx(head, {656, 456}, 4.5f, COMIC_INK);
  DrawSplineSegmentBezierQuadratic({638, 454}, {656, 446}, {682, 458}, 5,
                                   COMIC_INK); // handlebar
  DrawSplineSegmentBezierQuadratic({639, 454}, {656, 447}, {681, 458}, 2.4f,
                                   STEEL);
  ComicRect({676, 455, 10, 6}, {40, 40, 44, 255}, 1.2f); // grip
  ComicCircle({666, 449}, 4.4f, STEEL, 1.3f);            // the bell
  DrawCircleV({664.5f, 447.5f}, 1.2f, COMIC_PAPER);

  // the packed-earth street; in front of the door, the patch Amma sprinkles
  // with water every morning, for the kolam
  DrawBandGradient(582, 140, {190, 150, 106, 255}, {164, 124, 86, 255});
  DrawToneRect({left, 582, all.width, 140}, {110, 70, 40, 255}, 0.05f, 0.45f,
               TONE_DOTS, 5);
  const Color wet = {158, 118, 82, 255};
  BeginTone(TONE_DOTS, 4);
  DrawEllipse(362, 620, 214, 48, Faded({90, 60, 40, 255}, 0.35f));
  EndTone();
  DrawEllipse(360, 618, 204, 42, wet);
  for (int i = 0; i < 30; i++) { // pebbles, not on the wet patch
    const float x = left + std::fmod(i * 97.0f, all.width);
    const float y = 600.0f + (i * 37) % 112;
    const float dx = (x - 360) / 214, dy = (y - 620) / 48;
    if (dx * dx + dy * dy < 1.2f)
      continue;
    ComicEllipse({x, y}, 3.5f, 2.4f, {214, 192, 160, 255}, 1.2f);
  }
  for (float x : {left + 40, 240.0f, 480.0f, 740.0f}) { // tufts of grass
    for (int k = -1; k <= 1; k++)
      InkLine({x + k * 3.0f, 586}, {x + k * 7.0f, 570.0f + std::abs(k) * 5},
              2.4f, {64, 120, 48, 255});
  }
  InkCurve({left + 20, 690}, {360, 676}, {right - 20, 694}, 1.4f,
           {130, 92, 62, 255}); // cart tracks
  InkCurve({left + 60, 708}, {380, 696}, {right - 40, 712}, 1.4f,
           {130, 92, 62, 255});

  // yesterday's kolam in rice flour: shaky lines, half swept away
  const Color flour = {252, 248, 236, 255};
  for (int row = 0; row < 2; row++) {
    for (int i = 0; i < 5; i++) {
      const Vector2 d = {270.0f + i * 45 + row * 22, 604.0f + row * 26};
      DrawCircleV(d, 2.8f, flour);
      DrawShakyRing(d, 15, 9, 0, 2 * PI, i + row * 5, flour);
    }
  }
  for (int i = 0; i < 4; i++) // the loops joining the rows
    DrawShakyRing({303.0f + i * 45, 617}, 26, 16, -PI * 0.3f, PI * 0.3f,
                  i + 20, flour);
  for (int k = 0; k < 5; k++) { // the broom swept the right side away
    const float x = 382 + k * 16.0f;
    DrawSplineSegmentBezierQuadratic({x, 586}, {x + 44, 618}, {x + 10, 652},
                                     12, wet);
    DrawSplineSegmentBezierQuadratic({x + 6, 588}, {x + 48, 620},
                                     {x + 14, 650}, 1.2f, Faded(flour, 0.35f));
  }
}

static void DrawHallFrontDoor();

static void DrawHall(float time) {
  const Rectangle all = FullScreen();
  // turquoise walls; the ceiling's shade printed in dots, the window's
  // evening light in warm dots
  DrawBand(0, 548, {86, 192, 186, 255});
  DrawToneRect({all.x, 26, all.width, 170}, {20, 92, 100, 255}, 0.6f, 0,
               TONE_DOTS, 6);
  DrawToneRect({all.x, 380, all.width, 168}, {20, 92, 100, 255}, 0, 0.4f,
               TONE_DOTS, 6);
  BeginTone(TONE_DOTS, 6);
  DrawCircleGradient({130, 200}, 270, {255, 226, 120, 170},
                     {255, 226, 120, 0});
  EndTone();
  DrawBand(0, 26, {58, 148, 146, 255});
  DrawBand(24, 3, COMIC_INK);
  DrawWindow({50, 110, 160, 150}, true, time);

  // a wall clock
  ComicCircle({870, 150}, 34, TEAK, 2.6f);
  ComicCircle({870, 150}, 27, COMIC_PAPER, 1.4f);
  for (int i = 0; i < 12; i++) {
    const float a = i * PI / 6;
    DrawLineEx({870 + 21 * std::cos(a), 150 + 21 * std::sin(a)},
               {870 + 25 * std::cos(a), 150 + 25 * std::sin(a)}, 1.6f,
               COMIC_INK);
  }
  InkLine({870, 152}, {870, 130}, 3.2f);
  InkLine({868, 150}, {885, 159}, 3.2f);

  // a switchboard beyond the kitchen doorway (wide screens), its wire running
  // up the wall
  DrawLineEx({758, 26}, {758, 298}, 3.4f, COMIC_INK);
  DrawLineEx({758, 26}, {758, 298}, 1.6f, COMIC_PAPER);
  ComicRect({740, 296, 36, 46}, COMIC_PAPER, 2);
  for (int i = 0; i < 3; i++)
    DrawRectangleRec({745 + i * 10.0f, 303, 6, 10}, COMIC_INK);
  DrawCircleV({752, 328}, 2, COMIC_INK);
  DrawCircleV({764, 328}, 2, COMIC_INK);

  // the Tamil calendar on a nail: a picture on top, the date pad, the
  // fourteenth circled in red
  InkLine({405, 121}, {378, 136}, 1.4f);
  InkLine({405, 121}, {432, 136}, 1.4f);
  DrawCircleV({405, 121}, 2.4f, COMIC_INK);
  ComicRect({375, 135, 60, 86}, COMIC_PAPER, 2.2f);
  DrawRectangleRec({378, 138, 54, 24}, {240, 120, 40, 255});
  for (int i = 0; i < 8; i++) { // a glowing halo
    const float a = PI + i * PI / 7;
    DrawLineEx({405, 158}, {405 + 22 * std::cos(a), 158 + 22 * std::sin(a)},
               1.4f, {255, 214, 90, 255});
  }
  ComicCircle({405, 152}, 6.5f, {255, 208, 70, 255}, 1.2f);
  DrawEllipse(405, 159, 9, 3, {236, 90, 130, 255});
  DrawLineEx({376, 162}, {434, 162}, 1.6f, COMIC_INK);
  DrawString("14", 388, 168, 30, COMIC_INK);
  DrawRing({405, 186}, 19.5f, 22, -70, 280, 24, {214, 30, 44, 255});
  DrawRing({406, 184}, 20, 22, 260, 320, 8, {214, 30, 44, 255});

  // the family photo: a day at the beach, Appa, Amma and small Anu
  BeginTone(TONE_DOTS, 4); // its shadow on the wall
  DrawRectangle(469, 124, 104, 134, Faded(COMIC_INK, 0.55f));
  EndTone();
  ComicRect({463, 118, 104, 134}, TEAK_DARK, 3);
  DrawRectangle(469, 124, 92, 122, COMIC_PAPER); // the white border
  DrawRectangleGradientV(475, 130, 80, 44, {110, 196, 236, 255},
                         {190, 232, 244, 255}); // sky
  DrawRectangle(475, 172, 80, 14, {50, 140, 200, 255}); // sea
  for (int i = 0; i < 4; i++)
    DrawLineEx({480.0f + i * 20, 178}, {490.0f + i * 20, 178}, 1.2f,
               COMIC_PAPER);
  DrawRectangle(475, 186, 80, 54, {244, 212, 150, 255}); // sand
  const Color skin = {176, 116, 78, 255};
  ComicRect({484, 168, 16, 46}, {64, 104, 190, 255}, 1.2f); // Appa
  ComicRect({484, 214, 16, 22}, {250, 248, 236, 255}, 1.2f);
  ComicCircle({492, 160}, 7.5f, skin, 1.2f);
  DrawCircleSector({492, 158}, 7.5f, 180, 360, 10, COMIC_INK);
  ComicRect({529, 173, 15, 60}, {40, 140, 90, 255}, 1.2f); // Amma
  ComicCircle({536, 166}, 6.5f, skin, 1.2f);
  DrawCircleSector({536, 164}, 6.5f, 180, 360, 10, COMIC_INK);
  DrawCircleV({530, 163}, 3.4f, COMIC_INK); // her bun
  const Vector2 frock[3] = {{514, 202}, {504, 227}, {524, 227}};
  ComicPoly(frock, 3, {236, 104, 150, 255}, 1.2f); // small Anu
  ComicCircle({514, 196}, 5.5f, skin, 1.1f);
  DrawCircleSector({514, 194.5f}, 5.5f, 180, 360, 10, COMIC_INK);
  for (Vector2 face : {Vector2{493, 161}, Vector2{537, 167}, Vector2{515, 197}})
    for (int side = -1; side <= 1; side += 2) // everyone squinting
      DrawLineEx({face.x + side * 3.2f - 1.4f, face.y},
                 {face.x + side * 3.2f + 1.4f, face.y}, 1, COMIC_INK);
  InkRect({475, 130, 80, 110}, 1.2f);
  DrawLineEx({478, 150}, {498, 132}, 2.4f, {255, 255, 255, 150}); // glass
  DrawLineEx({478, 160}, {508, 133}, 1.2f, {255, 255, 255, 120});

  // skirting, and the red oxide floor: inked tile lines, dots, a shine
  DrawBand(545, 16, {50, 140, 136, 255});
  DrawBand(544, 2.5f, COMIC_INK);
  DrawBandGradient(561, 160, {204, 72, 50, 255}, {176, 54, 40, 255});
  const Color grout = {100, 26, 20, 255};
  for (float gap = 10, ty = 561 + gap; ty < 720; gap *= 1.35f, ty += gap)
    DrawLineEx({all.x, ty}, {all.x + all.width, ty}, 1.4f, grout);
  for (int i = -12; i <= 12; i++) // lines towards a point far behind
    DrawLineEx({360 + i * 40.0f, 561}, {360 + i * 140.0f, 720}, 1.4f, grout);
  DrawToneRect({all.x, 561, all.width, 160}, {80, 14, 10, 255}, 0, 0.4f,
               TONE_DOTS, 6);
  DrawBand(560, 3, COMIC_INK);
  BeginTone(TONE_DOTS, 6); // the window's light lying across the floor
  FillTriangle({40, 562}, {120, 720}, {212, 720}, {255, 220, 150, 170});
  FillTriangle({40, 562}, {212, 720}, {176, 562}, {255, 220, 150, 170});
  EndTone();
  for (int i = 0; i < 3; i++) { // the shine of the polish
    InkLine({520.0f + i * 14, 640}, {556.0f + i * 14, 604}, 2.6f,
            {255, 214, 200, 200});
    InkLine({-40.0f + i * 14, 690}, {-4.0f + i * 14, 654}, 2.6f,
            {255, 214, 200, 200});
  }

  // ceiling fan, its blades a blur of speed lines
  DrawLineEx({360, 26}, {360, 38}, 5, COMIC_INK);
  DrawLineEx({360, 26}, {360, 38}, 2.4f, {170, 176, 190, 255});
  for (int i = 0; i < 3; i++) {
    const float a = time * 6 + i * 2.094f;
    const Vector2 blade[4] = {
        {360 + 14 * std::cos(a - 0.4f), 44 + 5 * std::sin(a - 0.4f)},
        {360 + 92 * std::cos(a - 0.1f), 44 + 12 * std::sin(a - 0.1f)},
        {360 + 92 * std::cos(a + 0.1f), 44 + 12 * std::sin(a + 0.1f)},
        {360 + 14 * std::cos(a + 0.4f), 44 + 5 * std::sin(a + 0.4f)}};
    ComicPoly(blade, 4, {160, 166, 182, 255}, 1.8f);
  }
  for (int i = 0; i < 2; i++) {
    const float a = time * 6 + i * PI - 0.5f;
    const float b = a - 0.9f, m = (a + b) / 2;
    InkCurve({360 + 104 * std::cos(a), 46 + 15 * std::sin(a)},
             {360 + 118 * std::cos(m), 46 + 18 * std::sin(m)},
             {360 + 104 * std::cos(b), 46 + 15 * std::sin(b)}, 1.6f);
  }
  ComicEllipse({360, 44}, 15, 9, {176, 182, 196, 255}, 2.2f);

  // the oonjal: a wooden swing on chains
  DrawComicShadow(245, 584, 300); // on the floor under the swing
  for (float x : {140.0f, 350.0f}) {
    ComicRect({x - 8, 22, 16, 6}, BRASS_YELLOW, 1.4f);
    for (int y = 28; y < 384; y += 12) {
      if ((y / 12) % 2)
        DrawRing({x, y + 6.0f}, 2.4f, 5.6f, 0, 360, 12, COMIC_INK);
      else
        DrawRectangleRec({x - 1.6f, y + 0.0f, 3.2f, 12}, COMIC_INK);
    }
  }
  ComicRect({106, 388, 278, 28}, TEAK, 3);
  for (int i = 0; i < 3; i++) // the grain
    InkCurve({122, 395.0f + i * 7}, {245, 392.0f + i * 7 + (i % 2 ? 4 : -4)},
             {368, 396.0f + i * 7}, 1.1f, TEAK_DARK);
  ComicRect({106, 414, 278, 10}, TEAK_DARK, 2.4f);
  for (float x : {140.0f, 350.0f}) {
    ComicCircle({x, 388}, 5, BRASS_YELLOW, 1.4f);
    DrawSparkle({x - 2, 386}, 3.5f, {255, 252, 230, 255});
  }
  ComicCircle({106, 403}, 5.5f, BRASS_YELLOW, 1.4f);
  ComicCircle({384, 403}, 5.5f, BRASS_YELLOW, 1.4f);

  // the doorway to the kitchen: a teak frame, a cotton curtain, and on the
  // frame, pencil marks of how tall Anu was each year
  DrawRectangle(590, 150, 120, 395, {66, 44, 40, 255});
  DrawToneRect({590, 150, 120, 395}, COMIC_INK, 0.3f, 0.65f, TONE_LINES, 4);
  BeginTone(TONE_DOTS, 4); // the kitchen's warm light, low down
  DrawCircleGradient({650, 545}, 80, {255, 170, 70, 200}, {255, 170, 70, 0});
  EndTone();
  for (int i = 0; i < 6; i++) {
    const float x = 604 + i * 17.0f;
    const float bottom = 458 + 6 * std::sin(time + i);
    const Color cloth =
        i % 2 ? Color{248, 238, 212, 255} : Color{58, 106, 196, 255};
    DrawRectangleRec({x, 158, 17, bottom - 158}, cloth);
    DrawToneRect({x + 9, 160, 8, bottom - 160}, COMIC_INK, 0.1f, 0.4f,
                 TONE_DOTS, 4);
    InkLine({x, 160}, {x + 1, bottom}, 1.3f);
    DrawLineEx({x, bottom}, {x + 17, 458 + 6 * std::sin(time + i + 1)}, 2.2f,
               COMIC_INK);
  }
  DrawLineEx({598, 158}, {710, 158}, 4.5f, COMIC_INK); // the rod
  DrawLineEx({598, 157}, {710, 157}, 1.6f, BRASS_YELLOW);
  ComicRect({586, 140, 132, 14}, TEAK, 2.6f);
  ComicRect({586, 140, 22, 405}, TEAK, 2.6f);
  ComicRect({696, 140, 22, 405}, TEAK, 2.6f);
  DrawRectangleRec({588, 154, 4, 388}, TEAK_LIGHT);
  DrawRectangleRec({698, 154, 4, 388}, TEAK_LIGHT);
  for (int i = 0; i < 11; i++) // pencil, not ink
    DrawLineEx({589, 510.0f - i * 30}, {601.0f + (i % 3) * 2, 510.0f - i * 30},
               1.7f, {70, 70, 88, 230});

  // the front door, from inside, on screens wide enough to show all of it
  // (see DOORS). The back of the carved door is plain teak with battens and
  // an iron bolt; the evening comes in round its edges, across the floor.
  if (WideHall())
    DrawHallFrontDoor();
}

static void DrawHallFrontDoor() {
  const Rectangle fd = HALL_FRONT_DOOR;
  BeginTone(TONE_DOTS, 4);
  DrawCircleGradient({fd.x + fd.width / 2, fd.y + fd.height + 4}, 120,
                     {255, 214, 120, 210}, {255, 214, 120, 0});
  EndTone();
  ComicRect(fd, TEAK_DARK, 3.6f);
  DrawRectangleRec({fd.x + 10, fd.y + 12, fd.width - 20, fd.height - 12},
                   {255, 226, 150, 255}); // the light in the gaps
  for (int side = 0; side < 2; side++) {
    const Rectangle leaf = {fd.x + 12 + side * 57.0f, fd.y + 14, 55,
                            fd.height - 16};
    ComicRect(leaf, TEAK, 2.6f);
    DrawRectangleRec({leaf.x + 2, leaf.y + 2, 4, leaf.height - 4}, TEAK_LIGHT);
    for (int k = 0; k < 3; k++) // the battens across its back
      ComicRect({leaf.x + 4, leaf.y + 30 + k * 150.0f, leaf.width - 8, 14},
                TEAK_DARK, 1.6f);
  }
  DrawToneRect({fd.x + 12, fd.y + 14, 112, 70}, COMIC_INK, 0.45f, 0, TONE_LINES,
               4); // the lintel's shade
  const Color IRON = {70, 72, 84, 255};
  const float boltY = fd.y + 214;
  ComicRect({fd.x + 38, boltY, 60, 8}, IRON, 1.6f); // the bolt
  ComicRect({fd.x + 82, boltY - 7, 10, 22}, IRON, 1.2f);
  ComicCircle({fd.x + 46, boltY + 4}, 5, IRON, 1.2f);
  // its threshold, yellow with turmeric, dotted with kumkum
  ComicRect({fd.x - 6, fd.y + fd.height - 4, fd.width + 12, 12},
            {236, 176, 40, 255}, 2.2f);
  for (int i = 0; i < 3; i++)
    DrawCircleV({fd.x + 32 + i * 36.0f, fd.y + fd.height + 2}, 3,
                {200, 30, 44, 255});
}

static void DrawKitchen(float time, bool ammaHere) {
  const Rectangle all = FullScreen();
  // ochre walls; years of cooking smoke printed in brown dots up top
  DrawBand(0, 602, {246, 212, 140, 255});
  DrawToneRect({all.x, 0, all.width, 240}, {96, 54, 30, 255}, 0.75f, 0,
               TONE_DOTS, 6);
  BeginTone(TONE_DOTS, 6); // and above the stove
  DrawCircleGradient({585, 250}, 170, {96, 54, 30, 130}, {96, 54, 30, 0});
  EndTone();
  DrawWindow({110, 304, 120, 96}, true, time);

  // wooden shelves of brass and steel, ladles hanging at the side
  for (float y : {150.0f, 270.0f}) {
    DrawToneRect({40, y + 12, 640, 16}, COMIC_INK, 0.55f, 0, TONE_DOTS, 4);
    for (float x : {70.0f, 360.0f, 650.0f}) {
      const Vector2 bracket[3] = {{x, y + 12}, {x + 18, y + 12}, {x, y + 34}};
      ComicPoly(bracket, 3, TEAK_DARK, 1.6f);
    }
    ComicRect({36, y, 648, 12}, TEAK, 2.4f);
    DrawLineEx({38, y + 2}, {682, y + 2}, 1.4f, TEAK_LIGHT);
  }
  for (int i = 0; i < 8; i++)
    DrawComicPot(80 + i * 60.0f, 128, 22, i % 2 == 0);
  for (int i = 0; i < 6; i++) { // steel dabbas
    const float x = 70 + i * 70.0f;
    DrawRectangleRec({x, 226, 40, 44}, STEEL);
    DrawRectangleRec({x + 27, 226, 13, 44}, {150, 158, 176, 255});
    DrawRectangleRec({x + 5, 230, 5, 36}, COMIC_PAPER);
    InkRect({x, 226, 40, 44}, 2);
    ComicRect({x - 2, 220, 44, 8}, {176, 184, 198, 255}, 1.6f);
  }
  ComicRect({744, 142, 116, 6}, TEAK, 1.6f);
  for (int i = 0; i < 4; i++) {
    const float x = 760 + i * 30.0f;
    DrawLineEx({x, 148}, {x, 250}, 5.5f, COMIC_INK);
    DrawLineEx({x - 0.5f, 149}, {x - 0.5f, 249}, 2.6f, STEEL);
    ComicEllipse({x, 256}, 10, 7, STEEL, 1.6f);
  }

  // the rice flour tin, with Amma's label
  ComicRect({564, 184, 78, 10}, {222, 222, 214, 255}, 2);
  ComicRect({596, 177, 14, 7}, {222, 222, 214, 255}, 1.4f);
  DrawRectangleRec({568, 194, 70, 76}, {242, 240, 230, 255});
  DrawRectangleRec({614, 194, 24, 76}, {204, 200, 192, 255});
  DrawRectangleRec({573, 198, 5, 68}, COMIC_PAPER);
  InkRect({568, 194, 70, 76}, 2.4f);
  ComicRect({576, 208, 56, 34}, {252, 232, 120, 255}, 1.6f);
  DrawTextCentered("maavu", 604, 209, 24, {186, 34, 34, 255}, HAND_FONT);
  DrawEllipse(650, 268, 9, 2, COMIC_PAPER); // a little spilt flour

  // the cooking platform: black granite on cream tiles, across the kitchen
  DrawRectangleRec({all.x, 436, all.width, 166}, {242, 234, 214, 255});
  for (float x = all.x; x < all.x + all.width; x += 40)
    DrawLineEx({x, 436}, {x, 600}, 1.3f, {150, 128, 104, 255});
  DrawLineEx({all.x, 518}, {all.x + all.width, 518}, 1.3f,
             {150, 128, 104, 255});
  DrawToneRect({all.x, 436, all.width, 30}, COMIC_INK, 0.6f, 0, TONE_DOTS, 4);
  DrawRectangleRec({all.x, 418, all.width, 18}, {50, 48, 58, 255});
  DrawBand(419, 2, {150, 150, 172, 255});
  DrawBand(417, 2, COMIC_INK);
  DrawBand(435, 3, COMIC_INK);
  for (int i = 0; i < 50; i++)
    DrawCircleV({all.x + std::fmod(i * 41.0f, all.width), 424.0f + i % 9},
                0.9f, {190, 190, 200, 255});

  // the gas stove, a blue flame peeking round the pot of pongal, steam
  ComicRect({500, 400, 170, 18}, {60, 60, 70, 255}, 2.4f);
  ComicCircle({516, 409}, 4, STEEL, 1.2f);
  ComicCircle({654, 409}, 4, STEEL, 1.2f);
  for (int k = 0; k < 12; k++) {
    const float x = 532 + k * 9.0f;
    const float h = 9 + 3 * std::sin(time * 14 + k * 1.7f);
    const float lick = std::sin(time * 9 + k);
    FillTriangle({x - 4, 401}, {x + 4, 401}, {x + lick, 401 - h},
                 {70, 150, 255, 255});
    FillTriangle({x - 2, 401}, {x + 2, 401}, {x, 404 - h},
                 {200, 236, 255, 255});
  }
  DrawComicPot(580, 380, 44, true);
  for (int i = 0; i < 3; i++) { // steam: inked white wisps drifting up
    const float t = std::fmod(time * 0.5f + i / 3.0f, 1.0f);
    const float x = 560 + i * 11.0f, y = 330 - t * 28;
    const float a = std::sin(PI * t);
    for (int pass = 0; pass < 2; pass++) {
      const float w = pass == 0 ? 4.6f : 2.4f;
      const Color c = pass == 0 ? Faded(COMIC_INK, 0.8f * a)
                                : Faded(COMIC_PAPER, a);
      InkCurve({x, y}, {x + 7, y - 8}, {x, y - 16}, w, c);
      InkCurve({x, y - 16}, {x - 7, y - 24}, {x, y - 32}, w, c);
    }
  }

  // the brass coffee filter on the platform, with its dented lid
  DrawComicShadow(310, 422, 66);
  DrawRectangleRec({288, 378, 44, 42}, BRASS_YELLOW);
  DrawRectangleRec({320, 378, 12, 42}, BRASS_SHADE);
  InkRect({288, 378, 44, 42}, 2.2f);
  DrawRectangleRec({292, 350, 36, 30}, BRASS_YELLOW);
  DrawRectangleRec({318, 350, 10, 30}, BRASS_SHADE);
  InkRect({292, 350, 36, 30}, 2);
  ComicRect({286, 340, 48, 11}, {250, 204, 96, 255}, 2);
  ComicEllipse({310, 338}, 13, 5, BRASS_YELLOW, 1.6f);
  ComicCircle({310, 330}, 4.5f, BRASS_YELLOW, 1.4f);
  InkCurve({291, 344}, {297, 349}, {303, 345}, 1.6f); // the dent
  DrawSparkle({297, 388}, 6, {255, 252, 230, 255});

  // the floor
  DrawBandGradient(602, 120, {184, 142, 112, 255}, {156, 118, 92, 255});
  const Color grout = {104, 72, 54, 255};
  for (float gap = 10, ty = 602 + gap; ty < 720; gap *= 1.35f, ty += gap)
    DrawLineEx({all.x, ty}, {all.x + all.width, ty}, 1.3f, grout);
  for (int i = -12; i <= 12; i++)
    DrawLineEx({360 + i * 40.0f, 602}, {360 + i * 140.0f, 720}, 1.3f, grout);
  DrawToneRect({all.x, 602, all.width, 120}, {70, 40, 26, 255}, 0, 0.4f,
               TONE_DOTS, 6);
  DrawBand(600, 3, COMIC_INK);

  // the ammi kal: a granite grinding stone and its roller
  DrawComicShadow(160, 596, 220);
  ComicRect({70, 546, 180, 44}, {120, 120, 134, 255}, 2.8f);
  const Vector2 slab[4] = {{78, 530}, {242, 530}, {250, 546}, {70, 546}};
  ComicPoly(slab, 4, {164, 164, 178, 255}, 2.2f);
  BeginTone(TONE_GRAIN, 2);
  DrawRectangle(70, 530, 180, 60, Faded(COMIC_INK, 0.2f));
  EndTone();
  DrawEllipse(204, 538, 14, 4, {252, 244, 218, 255}); // a little batter
  ComicEllipse({160, 527}, 58, 10, {146, 146, 160, 255}, 2.4f);
  DrawEllipse(150, 523, 40, 3, {200, 200, 210, 255});

  // sugarcane for Pongal, leaning in the corner (wide screens)
  for (int i = 0; i < 2; i++) {
    const Vector2 foot = {930.0f + i * 24, 642}, top = {888.0f + i * 34, 250};
    DrawLineEx(foot, top, 15, COMIC_INK);
    DrawLineEx(foot, top, 10, {150, 66, 128, 255});
    DrawLineEx({foot.x - 2, foot.y}, {top.x - 2, top.y}, 2.4f,
               {200, 130, 180, 255}); // shine
    for (int k = 1; k < 9; k++) { // the joints
      const float t = k / 9.0f;
      const Vector2 p = {foot.x + (top.x - foot.x) * t,
                         foot.y + (top.y - foot.y) * t};
      DrawLineEx({p.x - 7, p.y}, {p.x + 7, p.y + 1}, 2.4f, COMIC_INK);
      DrawLineEx({p.x - 5, p.y - 2.2f}, {p.x + 5, p.y - 1.4f}, 1.2f,
                 {226, 190, 150, 255});
    }
    DrawComicLeaf(top, {top.x - 64, top.y - 40}, 11, {110, 170, 70, 255},
                  1.4f);
    DrawComicLeaf(top, {top.x + 44, top.y - 66}, 11, {90, 150, 60, 255},
                  1.4f);
    DrawComicLeaf({top.x, top.y + 6}, {top.x - 30, top.y + 56}, 10,
                  {120, 176, 76, 255}, 1.4f);
  }

  // a low wooden manai to sit on, near the stove
  DrawComicShadow(625, 676, 150);
  ComicRect({568, 656, 14, 14}, TEAK_DARK, 1.6f);
  ComicRect({668, 656, 14, 14}, TEAK_DARK, 1.6f);
  ComicRect({560, 642, 130, 15}, TEAK, 2.4f);
  DrawLineEx({562, 645}, {688, 645}, 1.4f, TEAK_LIGHT);
  if (ammaHere)
    DrawAmma(440, 600, time, true);
}

static void DrawTerrace(float time, bool night) {
  const Rectangle all = FullScreen();
  // sunset over the Western Ghats; the Bhavani shining below them
  DrawSunsetSky(420, {190, 300}, 44, night, time); // setting over the hills
  if (!night) {
    for (int i = 0; i < 3; i++) // long low clouds
      DrawLongCloud(std::fmod(i * 330 + time * 6, 1300.0f) - 300,
                    84.0f + i * 50, 300);
    for (int i = 0; i < 4; i++) { // birds going home
      const float x = std::fmod(time * 30 + i * 140, 800.0f) - 40,
                  y = 120 + i * 30 + 6 * std::sin(time * 3 + i);
      const float flap = 4 * std::sin(time * 8 + i);
      InkCurve({x - 12, y - 4 - flap}, {x - 6, y - 7}, {x, y}, 2.2f);
      InkCurve({x, y}, {x + 6, y - 7}, {x + 12, y - 4 - flap}, 2.2f);
    }
  }
  DrawComicHills(330, 420, 110, 3, {190, 142, 200, 255}, {110, 78, 150, 255});
  DrawComicHills(370, 420, 60, 6, {150, 110, 180, 255}, {84, 58, 130, 255});
  DrawBand(404, 16, {255, 224, 168, 255});
  DrawBand(403, 2, COMIC_INK);
  for (int i = 0; i < 8; i++) { // the river glinting
    const float x = all.x + std::fmod(i * 170 + time * 10, all.width);
    InkLine({x, 412}, {x + 30, 412}, 2.4f, COMIC_PAPER);
  }

  // the temple tower far off: tiers getting smaller, gold on top
  for (int i = 0; i < 6; i++) {
    const float w = 100 - i * 14.0f, y = 382 - i * 30.0f;
    DrawRectangleRec({490 - w / 2, y, w, 30}, {226, 140, 112, 255});
    for (int k = 0; k < (int)(w / 14); k++)
      DrawRectangleRec({490 - w / 2 + 5 + k * 14, y + 12, 6, 10},
                       {120, 56, 52, 255});
    DrawToneRect({490 + w / 2 - w * 0.3f, y + 6, w * 0.3f, 24}, COMIC_INK,
                 0.4f, 0.4f, TONE_DOTS, 4);
    InkRect({490 - w / 2, y, w, 30}, 1.8f);
    ComicRect({490 - w / 2 - 3, y, w + 6, 6}, {246, 184, 146, 255}, 1.4f);
  }
  ComicRect({472, 214, 36, 18}, {226, 140, 112, 255}, 1.8f);
  for (int k = 0; k < 3; k++) { // the kalasams
    const float x = 480 + k * 10.0f, r = k == 1 ? 5.0f : 4.0f;
    FillTriangle({x - 1.5f, 212 - r}, {x + 1.5f, 212 - r},
                 {x, 202 - r - (k == 1 ? 4 : 0)}, {240, 186, 60, 255});
    ComicCircle({x, 212 - r * 0.6f}, r, {244, 192, 64, 255}, 1.2f);
  }

  // palms and rooftops between here and the hills (none hiding the tower)
  const float PALMS[] = {-430, -200, 20, 250, 680, 900, 1130};
  for (int i = 0; i < 7; i++)
    DrawComicPalm(PALMS[i], 440, 150 + (i * 37) % 50, time, PALM_TRUNK,
                  PALM_GREEN);
  DrawBand(420, 100, {190, 120, 94, 255});
  DrawToneRect({all.x, 420, all.width, 100}, {90, 40, 30, 255}, 0.2f, 0.5f,
               TONE_DOTS, 5);
  const Color WALLS[] = {{246, 222, 170, 255},
                         {190, 230, 206, 255},
                         {252, 214, 140, 255},
                         {226, 192, 226, 255}};
  for (int i = 0; i < 10; i++) { // neighbours' houses, each a bit different
    const float x = all.x + i * 140 - 20, w = 110 + (i * 23) % 40,
                h = 50 + (i * 17) % 30;
    ComicRect({x, 520 - h, w, h}, WALLS[i % 4], 2);
    const Vector2 roof[3] = {{x - 8, 520 - h}, {x + w + 8, 520 - h},
                             {x + w / 2, 495 - h}};
    ComicPoly(roof, 3, {196, 84, 60, 255}, 2);
    ComicRect({x + w * 0.4f, 530 - h, 16, 22}, {90, 60, 56, 255}, 1.4f);
  }

  // the terrace: parapet and floor
  DrawBandGradient(522, 200, {216, 130, 92, 255}, {190, 104, 76, 255});
  const Color grout = {120, 52, 36, 255};
  for (float gap = 10, ty = 522 + gap; ty < 720; gap *= 1.35f, ty += gap)
    DrawLineEx({all.x, ty}, {all.x + all.width, ty}, 1.3f, grout);
  for (int i = -12; i <= 12; i++)
    DrawLineEx({360 + i * 40.0f, 522}, {360 + i * 140.0f, 720}, 1.3f, grout);
  DrawToneRect({all.x, 522, all.width, 200}, {100, 36, 24, 255}, 0, 0.45f,
               TONE_DOTS, 6);
  for (Vector2 p : {Vector2{-180, 640}, Vector2{330, 704}, Vector2{860, 600}})
    DrawEllipse((int)p.x, (int)p.y, 60, 12, {196, 110, 80, 255}); // stains
  DrawBand(496, 26, {228, 152, 112, 255});
  for (float x = all.x + 20; x < all.x + all.width; x += 48)
    DrawEllipse((int)x, 511, 7, 5, {130, 64, 46, 255}); // holes in the wall
  DrawBand(493, 8, {248, 190, 150, 255});
  DrawBand(492, 2, COMIC_INK);
  DrawBand(500, 1.6f, COMIC_INK);
  DrawBand(521, 3, COMIC_INK);
  DrawToneRect({all.x, 524, all.width, 14}, COMIC_INK, 0.5f, 0, TONE_DOTS, 4);

  // the black water tank, ribbed, with a comic shine
  DrawComicShadow(650, 514, 140);
  DrawRectangleRounded({597, 358, 106, 155}, 0.2f, 8, COMIC_INK);
  DrawRectangleRounded({600, 360, 100, 150}, 0.2f, 8, {40, 40, 48, 255});
  for (int i = 0; i < 4; i++)
    DrawRectangle(600, 390 + i * 30, 100, 3, {84, 84, 98, 255});
  ComicEllipse({650, 362}, 30, 7, {60, 60, 72, 255}, 2);
  InkLine({614, 378}, {614, 494}, 5, {226, 234, 246, 255});
  InkLine({626, 384}, {626, 440}, 2.4f, {226, 234, 246, 255});

  // the clothes line: a saree and a towel lifting in the breeze
  ComicRect({246, 296, 6, 206}, TEAK, 1.6f);
  ComicRect({588, 326, 6, 176}, TEAK, 1.6f);
  DrawSplineSegmentBezierQuadratic({250, 300}, {420, 320}, {590, 330}, 2,
                                   COMIC_INK);
  const float flap = 6 * std::sin(time * 2.2f);
  const Vector2 saree[4] = {{300, 304}, {362, 309}, {358 + flap, 400},
                            {296 + flap, 400}};
  ComicPoly(saree, 4, {228, 58, 110, 255}, 2);
  DrawLineEx({298 + flap, 391}, {358 + flap, 391}, 7, {248, 196, 60, 255});
  DrawLineEx({298 + flap, 387}, {358 + flap, 387}, 1.4f, COMIC_INK);
  InkCurve({318, 308}, {316 + flap * 0.5f, 350}, {316 + flap, 396}, 1.2f);
  InkCurve({340, 310}, {339 + flap * 0.5f, 350}, {338 + flap, 396}, 1.2f);
  ComicRect({380 + flap * 0.4f, 312, 50, 64}, {60, 132, 216, 255}, 2);
  for (int i = 0; i < 3; i++)
    DrawRectangleRec({382 + flap * 0.4f, 358.0f + i * 6, 46, 2.5f},
                     COMIC_PAPER);
  for (float x : {318.0f, 348.0f, 392.0f, 420.0f}) // clothes pegs
    ComicRect({x, 302 + (x - 250) * 0.07f, 4, 10}, TEAK_LIGHT, 1);

  // old trunks brought up here: a blue tin trunk, a box of school things
  DrawComicShadow(360, 608, 200);
  ComicRect({280, 546, 160, 58}, {58, 108, 180, 255}, 2.8f);
  DrawLineEx({284, 584}, {436, 584}, 2, {120, 172, 226, 255});
  BeginTone(TONE_DOTS, 3); // rust
  DrawEllipse(300, 592, 16, 7, {160, 80, 40, 220});
  DrawEllipse(424, 566, 10, 5, {160, 80, 40, 200});
  EndTone();
  const Vector2 flapLeft[4] = {{300, 502}, {286, 486}, {316, 482}, {330, 502}};
  const Vector2 flapRight[4] = {{380, 502}, {394, 480}, {424, 486}, {410, 502}};
  ComicPoly(flapLeft, 4, {196, 150, 96, 255}, 1.8f);
  ComicPoly(flapRight, 4, {196, 150, 96, 255}, 1.8f);
  const Color CRAYONS[] = {{230, 60, 50, 255},
                           {60, 130, 226, 255},
                           {252, 204, 50, 255},
                           {70, 176, 84, 255}};
  for (int i = 0; i < 4; i++) { // crayons peeking out
    const float x = 322 + i * 11.0f, top = 490.0f + (i % 2) * 5;
    ComicRect({x, top, 8, 14}, CRAYONS[i], 1.2f);
    const Vector2 tip[3] = {{x, top}, {x + 8, top}, {x + 4, top - 7}};
    ComicPoly(tip, 3, CRAYONS[i], 1.2f);
  }
  ComicRect({300, 502, 110, 40}, {216, 170, 110, 255}, 2.4f);
  ComicRect({276, 536, 168, 14}, {80, 138, 206, 255}, 2.4f); // trunk's lid
  ComicRect({350, 548, 20, 16}, BRASS_YELLOW, 1.6f);          // the latch

  // Appa's kite reel (a wooden lattai), the old torn kite beside it
  DrawComicShadow(140, 586, 170);
  const Vector2 kiteTop = {190, 460}, kiteLeft = {150, 520},
                kiteRight = {230, 520}, kiteBottom = {190, 590};
  FillTriangle(kiteTop, kiteLeft, kiteRight, {248, 128, 44, 255});
  FillTriangle(kiteLeft, kiteBottom, kiteRight, {220, 64, 44, 255});
  const Vector2 kite[4] = {kiteTop, kiteRight, kiteBottom, kiteLeft};
  InkPoly(kite, 4, 2.4f);
  InkLine(kiteTop, kiteBottom, 1.8f, TEAK_DARK); // the spine
  InkCurve(kiteLeft, {190, 508}, kiteRight, 1.8f, TEAK_DARK);
  const Vector2 tear[4] = {{200, 470}, {207, 482}, {203, 490}, {213, 503}};
  for (int i = 0; i < 3; i++) {
    DrawLineEx(tear[i], tear[i + 1], 4.4f, COMIC_INK);
    DrawLineEx(tear[i], tear[i + 1], 2.2f, {255, 246, 226, 255});
  }
  InkCurve(kiteBottom, {222, 612}, {200, 646}, 4, COMIC_INK); // the tail
  InkCurve(kiteBottom, {222, 612}, {200, 646}, 2, {250, 226, 150, 255});
  for (Vector2 b : {Vector2{206, 606}, Vector2{208, 632}}) { // bows on the tail
    const Vector2 bowL[3] = {b, {b.x - 9, b.y - 5}, {b.x - 9, b.y + 5}};
    const Vector2 bowR[3] = {b, {b.x + 9, b.y - 5}, {b.x + 9, b.y + 5}};
    ComicPoly(bowL, 3, {250, 210, 60, 255}, 1.2f);
    ComicPoly(bowR, 3, {250, 210, 60, 255}, 1.2f);
  }
  DrawSplineSegmentBezierQuadratic({134, 520}, {160, 500}, {190, 522}, 1.2f,
                                   {252, 246, 230, 255}); // a loose thread
  ComicCircle({110, 540}, 34, {216, 162, 92, 255}, 2.8f);
  DrawCircleV({110, 540}, 26, {248, 238, 216, 255}); // wound thread
  for (int i = 0; i < 5; i++)
    DrawRing({110, 540}, 9 + i * 3.6f, 9.8f + i * 3.6f, 0, 360, 32,
             {206, 184, 150, 255});
  InkCircle({110, 540}, 26, 1.4f);
  ComicCircle({110, 540}, 8, TEAK_DARK, 1.4f);
  DrawLineEx({84, 562}, {60, 594}, 9, COMIC_INK); // the handle
  DrawLineEx({84, 562}, {60, 594}, 5.5f, TEAK);
}

// Night, as an old comic prints it: the whole picture through a blue plate
// (the skies print their dots in deep blue at night), then warm lamplight
// and the stars
static void DrawNight(Room room, float time) {
  const Rectangle all = FullScreen();
  BeginBlendMode(BLEND_MULTIPLIED);
  DrawRectangleRec(all, {90, 110, 196, 255});
  EndBlendMode();
  // where the lamplight comes from, and the stars over open sky
  Vector2 lamp = {515, 270};
  float skyBottom = 0;
  if (room == FRONT_STEP) {
    lamp = {227, 400}; // Amma's lamp in the niche
    skyBottom = 70;
  } else if (room == KITCHEN) {
    lamp = {580, 400}; // the stove
  } else if (room == TERRACE) {
    lamp = {490, 300}; // the temple, lit for the festival
    skyBottom = 250;
  }
  BeginBlendMode(BLEND_ADDITIVE);
  DrawCircleGradient(lamp, 190, {255, 170, 70, 70}, {255, 170, 70, 0});
  if (room == FRONT_STEP) // the hall light on behind the windows
    for (float x : {-190.0f, 820.0f})
      DrawRectangleRec({x, 260, 110, 130}, {255, 170, 60, 110});
  EndBlendMode();
  BeginTone(TONE_DOTS, 5);
  DrawCircleGradient(lamp, 110, {255, 214, 120, 150}, {255, 214, 120, 0});
  EndTone();
  for (int i = 0; i < 26 && skyBottom > 0; i++) {
    const float x = all.x + std::fmod(i * 173.0f, all.width);
    const float y = 12 + std::fmod(i * 67.0f, skyBottom - 12);
    const float twinkle = 0.6f + 0.4f * std::sin(time * 2 + i * 1.7f);
    DrawSparkle({x, y}, (2.5f + (i % 3)) * twinkle, {255, 246, 200, 230});
  }
  if (room == TERRACE) {
    // a crescent moon, inked, and the tower's festival lights twinkling
    const Vector2 moon = {170, 90};
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient(moon, 60, {255, 240, 200, 50}, {255, 240, 200, 0});
    EndBlendMode();
    Vector2 prevOut = {}, prevIn = {};
    for (int k = 0; k <= 16; k++) {
      const float t = -1 + k / 8.0f, s = std::sqrt(std::fmax(0, 1 - t * t));
      const Vector2 out = {moon.x - 22 * s, moon.y + 22 * t};
      const Vector2 in = {moon.x - 8 * s, moon.y + 22 * t};
      if (k > 0) {
        FillTriangle(prevOut, out, in, {255, 244, 196, 255});
        FillTriangle(prevOut, in, prevIn, {255, 244, 196, 255});
        DrawLineEx(prevOut, out, 2.4f, COMIC_INK);
        DrawLineEx(prevIn, in, 1.6f, COMIC_INK);
      }
      prevOut = out;
      prevIn = in;
    }
    for (int i = 0; i < 6; i++) {
      const float w = 100 - i * 14.0f, y = 382 - i * 30.0f;
      for (float x = 490 - w / 2; x <= 490 + w / 2; x += 7) {
        const bool on = std::sin(time * 3 + x * 0.7f + i) > -0.3f;
        DrawCircleV({x, y + 1}, on ? 1.8f : 1.1f,
                    on ? Color{255, 226, 120, 255} : Color{200, 140, 80, 255});
      }
    }
  }
  if (room == FRONT_STEP) // the morning star, before dawn
    DrawSparkle({150, 34}, 7 + std::sin(time * 1.5f), {255, 250, 220, 255});
}

static void DrawDoorways(Room room, bool night, float time); // with the doors

// Draws a room by day or by night, with Amma in it or not, and anyone else
// who is there (`people`, e.g. in a flashback), before the night shades it
static void DrawRoom(Room room, bool night, float time, bool ammaHere,
                     void (*people)(float time) = nullptr) {
  switch (room) {
  case FRONT_STEP:
    DrawFrontStep(time, night);
    break;
  case HALL:
    DrawHall(time);
    break;
  case KITCHEN:
    DrawKitchen(time, ammaHere);
    break;
  default:
    DrawTerrace(time, night);
    break;
  }
  DrawDoorways(room, night, time); // the doors the pictures don't have
  if (people)
    people(time);
  if (night)
    DrawNight(room, time);
  else // (the grain doesn't show at night)
    DrawNewsprint();
}

void DrawPlace(const std::string &place, bool night, float time,
               void (*people)(float time)) {
  for (int i = 0; i < ROOM_COUNT; i++) {
    if (place == PLACE_IDS[i])
      DrawRoom((Room)i, night, time, i == KITCHEN, people);
  }
}

//------------------------------------------------------------------------------
// The scene
//------------------------------------------------------------------------------
const float FIRST_CALL_SECONDS = 12; // the phone first rings after this long
const float CALL_GAP_SECONDS = 25;   // and then again after this long
const Rectangle ANSWER_BUTTON = {478, 560, 100, 40};
const Rectangle NOT_NOW_BUTTON = {588, 560, 110, 40};
// The doorframe up close: Anu's height each year, from 5 to 15, in pencil.
// Drag up the frame to trace them.
const float FRAME_X = 360;      // the middle of the post
const float MARKS_BOTTOM = 600; // age 5
const float MARK_GAP = 40;      // between one year and the next
const int MARK_COUNT = 11;      // ages 5 to 15
const float WALK_SECONDS = 0.25f; // a moment's shade, walking through a door

// How many doors it takes to walk from one room to another
static int Distance(Room from, Room to) {
  int doors[ROOM_COUNT];
  for (int &d : doors)
    d = ROOM_COUNT; // further than any real walk
  doors[from] = 0;
  for (int pass = 1; pass < ROOM_COUNT; pass++) {
    for (const Door &d : DOORS) {
      if (DoorShown(d) && doors[d.from] + 1 < doors[d.to])
        doors[d.to] = doors[d.from] + 1;
    }
  }
  return doors[to];
}

// Light falling on the floor from a doorway behind you, printed as warm dots:
// a band as wide as the door at the bottom of the screen, narrowing and fading
// into the room. `stripes` > 0: it comes through a blue-and-white curtain.
static void DrawDoorLight(float left, float right, float far, int stripes,
                          float time, bool night) {
  const int BANDS = 5;
  const float squeeze = 0.22f * (right - left); // narrower further in
  BeginTone(TONE_DOTS, 5);
  for (int b = 0; b < BANDS; b++) {
    const float y0 = 720 - (720 - far) * b / BANDS; // nearer edge
    const float y1 = 720 - (720 - far) * (b + 1) / BANDS;
    const float in0 = squeeze * b / BANDS, in1 = squeeze * (b + 1) / BANDS;
    const int parts = stripes > 0 ? stripes : 1;
    for (int s = 0; s < parts; s++) {
      const float sway = stripes > 0 ? 4 * std::sin(time * 1.3f + s) : 0;
      const float f0 = (float)s / parts, f1 = (float)(s + 1) / parts;
      const float l0 = left + in0, r0 = right - in0;
      const float l1 = left + in1, r1 = right - in1;
      Color c = {255, 220, 140, 255}; // evening light
      if (stripes > 0)
        c = s % 2 ? Color{110, 160, 236, 255} : Color{255, 240, 200, 255};
      const float strength = night ? 0.35f : 0.7f; // the dots' size
      c = Faded(c, strength * (1 - (float)b / BANDS));
      const Vector2 nearL = {l0 + (r0 - l0) * f0, y0},
                    nearR = {l0 + (r0 - l0) * f1, y0},
                    farL = {l1 + (r1 - l1) * f0 + sway * b / BANDS, y1},
                    farR = {l1 + (r1 - l1) * f1 + sway * b / BANDS, y1};
      FillTriangle(farL, nearL, nearR, c);
      FillTriangle(farL, nearR, farR, c);
    }
  }
  EndTone();
}

// The doorposts' feet at the bottom of the screen, behind you
static void DrawPostFeet(float left, float right, float width, float top) {
  for (float x : {left, right}) {
    ComicRect({x, top, width, 720 - top + 4}, TEAK, 2.6f);
    DrawRectangleRec({x + 2, top + 2, 4, 720 - top}, TEAK_LIGHT);
  }
}

// The doors the room pictures don't have themselves, drawn in the same comic
// ink: the stairs up from the hall, and the doorways behind you at the bottom
// of the hall, kitchen and terrace. (The front door and the hall's curtained
// doorway are in the pictures.) Drawn before the night plate, which shades
// them like the rest of the room.
static void DrawDoorways(Room room, bool night, float time) {
  if (room == HALL) {
    // The stairs up to the terrace, through a narrow doorway: whitewashed
    // steps climbing into the light from the terrace door at the top
    DrawRectangle(426, 280, 124, 265, {66, 44, 40, 255});
    for (int i = 0; i < 8; i++) {
      const float top = 513 - i * 32.0f, right = 550 - i * 15.0f;
      const unsigned char shade = (unsigned char)(196 + i * 6); // lighter up
      ComicRect({426, top, right - 426, 32},
                {shade, (unsigned char)(shade - 30), (unsigned char)(shade - 70),
                 255},
                1.6f);
      DrawRectangleRec({428, top + 2, right - 430, 4}, {252, 232, 190, 255});
    }
    // under the stairs, and the wall beside them: hatched shade
    DrawToneRect({426, 280, 124, 265}, COMIC_INK, 0.55f, 0.15f, TONE_LINES, 4);
    BeginTone(TONE_DOTS, 4); // the terrace's light, from the top
    DrawCircleGradient({500, 286}, 90, {255, 214, 120, 200},
                       {255, 214, 120, 0});
    EndTone();
    InkLine({548, 472}, {459, 282}, 3.4f); // the rail
    DrawLineEx({547, 470}, {458, 280}, 1.4f, TEAK_LIGHT);
    ComicRect({412, 266, 152, 14}, TEAK, 2.6f); // lintel
    ComicRect({412, 266, 14, 279}, TEAK, 2.6f); // posts
    ComicRect({550, 266, 14, 279}, TEAK, 2.6f);
    DrawRectangleRec({414, 280, 4, 262}, TEAK_LIGHT);
    DrawRectangleRec({552, 280, 4, 262}, TEAK_LIGHT);

    if (WideHall()) // the front door is on the left wall (see DrawHall)
      return;
    // Narrower screens: behind you, the front door, open: the evening light
    // falling in, and its threshold, yellow with turmeric, dotted with kumkum
    DrawDoorLight(236, 484, 604, 0, time, night);
    ComicRect({240, 694, 240, 30}, {236, 176, 40, 255}, 2.6f);
    DrawRectangleRec({242, 696, 236, 4}, {252, 214, 110, 255});
    for (int i = 0; i < 3; i++) {
      ComicCircle({290.0f + i * 70, 709}, 5, {200, 30, 44, 255}, 1.2f);
      if (i < 2)
        DrawCircleV({325.0f + i * 70, 709}, 2.5f, COMIC_PAPER);
    }
    DrawPostFeet(222, 480, 18, 676);
  } else if (room == KITCHEN) {
    // Behind you, the doorway to the hall: light through its striped curtain,
    // and the wooden threshold
    DrawDoorLight(246, 474, 610, 5, time, night);
    ComicRect({250, 698, 220, 26}, TEAK, 2.6f);
    DrawRectangleRec({252, 700, 216, 4}, TEAK_LIGHT);
    DrawPostFeet(234, 470, 16, 680);
  } else if (room == TERRACE) {
    // Behind you, the top of the stairs down to the hall: a low wall around
    // it, the steps going down, darker, into the lamp light below
    for (int k = 0; k < 4; k++) { // each step: its tread, then its riser
      const float shade = 1 - k * 0.18f; // darker further down
      const float y = 634 + k * 22.0f;
      ComicRect({490, y, 202, 10},
                {(unsigned char)(228 * shade), (unsigned char)(152 * shade),
                 (unsigned char)(112 * shade), 255},
                1.6f);
      ComicRect({490, y + 10, 202, 12},
                {(unsigned char)(150 * shade), (unsigned char)(80 * shade),
                 (unsigned char)(60 * shade), 255},
                1.6f);
    }
    DrawToneRect({490, 634, 202, 86}, COMIC_INK, 0.1f, 0.6f, TONE_LINES, 4);
    BeginTone(TONE_DOTS, 4); // the hall's lamp light, coming up
    DrawRectangleGradientV(490, 670, 202, 50, {255, 196, 120, 0},
                           {255, 196, 120, 200});
    EndTone();
    const Color wall = {228, 152, 112, 255}; // as the parapet
    ComicRect({476, 616, 230, 18}, wall, 2.6f);
    ComicRect({476, 616, 14, 108}, wall, 2.6f);
    ComicRect({692, 616, 14, 108}, wall, 2.6f);
    DrawRectangleRec({478, 618, 226, 4}, {248, 190, 150, 255});
  }
}

class ExploreScene : public Scene {
public:
  explicit ExploreScene(const Step &exploreStep)
      : step(exploreStep), seen(exploreStep.hotspots.size(), false) {
    for (size_t i = 0; i < step.hotspots.size(); i++) {
      const Hotspot &h = step.hotspots[i];
      if (h.object == "amma")
        ammaHere = true;
      // a memory found before (the game was saved here) was seen then
      seen[i] = !h.memory.empty() && HasMemory(h.memory);
    }
    // Start in the room of the first thing there is to click
    if (!step.hotspots.empty() && Find(step.hotspots[0].object))
      room = Find(step.hotspots[0].object)->room;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    if (memoryTimer > 0)
      memoryTimer -= dt;
    if (walkFade > 0)
      walkFade -= dt;
    // A memory coming back floods the world with colour for a moment, then it
    // settles, a little more colourful than before
    colour = memoryTimer > 1.5f ? 1.0f : WorldColour();
    // Birds outside; quieter indoors. On the terrace, the festival far off.
    if (room == TERRACE)
      PlayAmbience("crowd", 0.18f);
    else
      PlayAmbience("birds_day", room == FRONT_STEP ? 0.55f : 0.25f);
    if (ringing)
      PlayLoop("vibrate", 0.6f);
    hover = nullptr;
    hoverDoor = nullptr;

    // 1. Someone is talking
    if (!talk.empty()) {
      if (AdvanceDialogue(talk, talkLine, typed))
        FinishTalk();
      return;
    }

    // 2. Tracing the pencil marks on the doorframe
    if (tracing) {
      UpdateTracing();
      return;
    }

    // 3. The phone is ringing
    if (ringing) {
      UpdatePhone();
      return;
    }
    phoneTimer -= dt;
    if (phoneTimer <= 0 && callsThisScene < (int)WorkCalls().size()) {
      const int callNumber = progress.callsAnswered + progress.callsIgnored;
      call = callNumber % WorkCalls().size();
      ringing = true;
      callsThisScene++;
      phoneTimer = CALL_GAP_SECONDS;
    }

    // 4. Looking around: the things in the room, and its doors
    idle = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ? 0 : idle + dt;
    const Vector2 mouse = GetMousePosition();
    for (const Hotspot &h : step.hotspots) {
      const RoomObject *o = Find(h.object);
      if (o && o->room == room && CheckCollisionPointRec(mouse, o->area))
        hover = &h;
    }
    for (const Door &d : DOORS) {
      if (!hover && d.from == room && DoorShown(d) &&
          CheckCollisionPointRec(mouse, d.area))
        hoverDoor = &d;
    }
    if (hover || hoverDoor)
      PointAtSomething();
    if (hoverDoor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      room = hoverDoor->to; // straight through: the new room works at once
      walkFade = WALK_SECONDS;
      hoverDoor = nullptr;
    }
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      current = hover;
      seen[hover - step.hotspots.data()] = true;
      if (hover->object == "temple")
        PlayEffect("temple_bells", 0.7f);
      if (hover->object == "height_marks") { // up close first
        tracing = true;
        traced = 0;
      } else {
        Say(hover->lines);
      }
    }
  }

  void Draw() override {
    DrawRoom(room, step.night, time, ammaHere);
    DrawWaitingHints();
    if (talk.empty() && !tracing && !ringing && idle > STUCK_SECONDS)
      DrawStuckHint();
    if (walkFade > 0) // walking through a door: a moment's shade
      DrawRectangleRec(FullScreen(), Faded({24, 16, 12, 255},
                                           0.55f * walkFade / WALK_SECONDS));

    if (tracing)
      DrawTracing();
    if (hover && talk.empty() && !tracing)
      DrawHoverLabel(Find(hover->object)->area, hover->name);
    else if (hoverDoor && talk.empty() && !tracing)
      DrawHoverLabel(hoverDoor->area, hoverDoor->name);
    DrawHud();
    if (!talk.empty())
      DrawDialogueBox(talk[talkLine], typed);
    if (ringing)
      DrawPhone();
    if (memoryTimer > 0)
      DrawMemoryBanner();
  }

private:
  const Step &step;
  Room room = FRONT_STEP;
  std::vector<bool> seen; // each hotspot: clicked yet?
  bool ammaHere = false;
  float time = 0;
  float idle = 0; // seconds without a click, while looking around
  // after this long, the next thing to look at glows (DrawStuckHint)
  static constexpr float STUCK_SECONDS = 10;
  float walkFade = 0; // counts down after walking through a door

  const Hotspot *hover = nullptr;   // what the mouse is over
  const Door *hoverDoor = nullptr;  // or the door it is over
  const Hotspot *current = nullptr; // the object whose lines are being said
  std::vector<Line> talk;           // the lines being said
  size_t talkLine = 0;
  float typed = 0;

  std::string memoryName; // the memory that just came back
  float memoryTimer = 0;

  bool tracing = false; // the doorframe is up close
  float traced = 0;     // how far up it the finger has gone

  bool ringing = false;
  size_t call = 0; // which of WorkCalls() is ringing
  float phoneTimer = FIRST_CALL_SECONDS;
  int callsThisScene = 0;

  static float MarkY(int age) { return MARKS_BOTTOM - (age - 5) * MARK_GAP; }

  // Hold and drag up the doorframe; the marks it passes come clear. Past the
  // last one, the object's lines play.
  void UpdateTracing() {
    const Vector2 mouse = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        std::fabs(mouse.x - FRAME_X) < 90) {
      const float before = traced;
      traced = std::fmax(traced, MARKS_BOTTOM + 20 - mouse.y);
      if (traced > before)
        PlayLoop("pencil", 0.5f); // a finger along the wood
    }
    if (traced > MARKS_BOTTOM + 20 - MarkY(15) + 30) {
      tracing = false;
      Say(current->lines);
    }
  }

  void DrawTracing() const {
    DrawDim();
    // the post, close up: old paint, wood showing through
    DrawRectangle((int)FRAME_X - 50, 0, 100, 660, {124, 82, 52, 255});
    DrawRectangle((int)FRAME_X - 50, 0, 14, 660, {146, 100, 64, 255});
    DrawRectangle((int)FRAME_X + 38, 0, 12, 660, {100, 64, 40, 255});
    const Color pencil = {54, 54, 64, 255};
    for (int age = 5; age < 5 + MARK_COUNT; age++) {
      const float y = MarkY(age);
      const bool found = MARKS_BOTTOM + 20 - y <= traced;
      const Color c = Faded(pencil, found ? 0.9f : 0.25f);
      DrawLineEx({FRAME_X - 40, y}, {FRAME_X + 20 + (age % 3) * 6, y}, 2, c);
      DrawString(TextFormat("Anu %d", age), FRAME_X - 150, y - 16, 24,
                 Faded(FLOUR, found ? 0.9f : 0.2f), HAND_FONT);
    }
    // the finger's path: the dust wiped off
    if (traced > 0)
      DrawRectangle((int)FRAME_X - 6, (int)(MARKS_BOTTOM + 20 - traced), 12,
                    (int)traced, {255, 240, 210, 40});
    if (traced == 0) { // "drag up": a ring and an arrow
      DrawRing({FRAME_X, MARKS_BOTTOM + 20}, 22, 26, 0, 360, 32,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 4)));
      DrawTriangle({FRAME_X, MARKS_BOTTOM - 40},
                   {FRAME_X - 10, MARKS_BOTTOM - 22},
                   {FRAME_X + 10, MARKS_BOTTOM - 22}, Faded(FLOUR, 0.6f));
    }
  }

  static const RoomObject *Find(const std::string &object) {
    for (const RoomObject &o : ROOM_OBJECTS) {
      if (object == o.object)
        return &o;
    }
    return nullptr;
  }

  void Say(const std::vector<Line> &lines) {
    talk = lines;
    talkLine = 0;
    typed = 0;
  }

  // After an object's lines: maybe a memory comes back, maybe the story moves
  // on
  void FinishTalk() {
    talk.clear();
    if (!current)
      return;
    if (!current->memory.empty() && !HasMemory(current->memory)) {
      AddMemory(current->memory);
      PlayEffect("bell", 0.45f);
      memoryName = current->memory;
      memoryTimer = 3.5f;
      colour = WorldColour();
    }
    if (current->endsScene)
      done = true;
    current = nullptr;
  }

  // Answer (the call plays) or "Not now". Neither changes anything.
  void UpdatePhone() {
    if (Clicked(ANSWER_BUTTON)) {
      ringing = false;
      progress.callsAnswered++;
      current = nullptr;
      Say(WorkCalls()[call]);
    } else if (Clicked(NOT_NOW_BUTTON)) {
      ringing = false;
      progress.callsIgnored++;
    }
  }

  // A glowing outline around the object or door under the mouse, and its name
  void DrawHoverLabel(Rectangle r, const std::string &name) const {
    DrawRectangleRoundedLinesEx(r, 0.15f, 6, 6, Faded(COMIC_INK, 0.6f));
    DrawRectangleRoundedLinesEx(r, 0.15f, 6, 3,
                                Faded(FLOUR, 0.6f + 0.3f * std::sin(time * 5)));
    const float w = MeasureString(name, 20) + 24;
    const Rectangle all = FullScreen();
    const float x = std::fmax(all.x + 10, std::fmin(all.x + all.width - 10 - w,
                                                    r.x + r.width / 2 - w / 2));
    const float y =
        r.y > 50 ? r.y - 40 : r.y + r.height + 8; // above if it fits
    DrawRectangleRounded({x, y, w, 32}, 0.5f, 6, {20, 20, 30, 220});
    DrawString(name, x + 12, y + 4, 20, FLOUR);
  }

  // Is anything in this room still waiting to be clicked?
  bool Waiting(Room r) const {
    for (size_t i = 0; i < step.hotspots.size(); i++) {
      const RoomObject *o = Find(step.hotspots[i].object);
      if (o && o->room == r && !seen[i])
        return true;
    }
    return false;
  }

  // A small soft dot on each door that leads toward something still waiting
  // (on the way to it: the first door of the shortest walk there)
  void DrawWaitingHints() const {
    for (const Door &d : DOORS) {
      if (d.from != room || !DoorShown(d))
        continue;
      bool toward = false;
      for (int r = 0; r < ROOM_COUNT; r++) {
        if (r != room && Waiting((Room)r) &&
            1 + Distance(d.to, (Room)r) == Distance(room, (Room)r))
          toward = true;
      }
      if (!toward)
        continue;
      const Vector2 p = {d.area.x + d.area.width - 14, d.area.y + 14};
      const float pulse = 0.5f + 0.5f * std::sin(time * 2.5f);
      DrawCircleGradient(p, 12, Faded(FLOUR, 0.2f + 0.25f * pulse),
                         Faded(FLOUR, 0));
      DrawCircleV(p, 3.5f, Faded(FLOUR, 0.9f));
    }
  }

  // Looking around for a while, with nothing clicked: the next thing still
  // waiting in this room glows, with a tapping hand on it; if this room has
  // nothing left, the hand shows on the door towards what's waiting
  void DrawStuckHint() const {
    const float pulse = 0.5f + 0.5f * std::sin(time * 4);
    for (size_t i = 0; i < step.hotspots.size(); i++) {
      const RoomObject *o = Find(step.hotspots[i].object);
      if (!o || o->room != room || seen[i])
        continue;
      const Rectangle r = o->area;
      DrawRectangleRoundedLinesEx({r.x - 4, r.y - 4, r.width + 8, r.height + 8},
                                  0.2f, 6, 4, Faded(FLOUR, 0.3f + 0.5f * pulse));
      DrawGesture(MOVE_TAP, {r.x + r.width / 2, r.y + r.height / 2}, time);
      return;
    }
    for (const Door &d : DOORS) {
      if (d.from != room || !DoorShown(d))
        continue;
      for (int r = 0; r < ROOM_COUNT; r++)
        if (r != room && Waiting((Room)r) &&
            1 + Distance(d.to, (Room)r) == Distance(room, (Room)r)) {
          const Rectangle a = d.area;
          DrawGesture(MOVE_TAP, {a.x + a.width / 2, a.y + a.height / 2}, time);
          return;
        }
    }
  }

  void DrawHud() const {
    // Memories found so far, as kolam dots, in the screen's top-left corner
    // (past the pause and (i) buttons)
    const float left = FullScreen().x + 160;
    const int total = MemoryCount();
    DrawRectangleRounded({left, 10, 34.0f + total * 18, 34}, 0.5f, 6,
                         {20, 20, 30, 170});
    for (int i = 0; i < total; i++) {
      const bool found = i < (int)progress.memories.size();
      DrawCircleV({left + 18 + i * 18, 27}, found ? 6 : 3,
                  found ? FLOUR : Faded(FLOUR, 0.4f));
    }
  }

  void DrawPhone() const {
    DrawRectangleRounded({460, 400, 250, 220}, 0.12f, 8, {24, 24, 30, 250});
    DrawString("Rohit (work)", 480, 416, 22, FLOUR);
    DrawString("calling...", 480, 444, 16, MUTED);
    for (int i = 0; i < 3; i++) // ringing
      DrawRing({585, 506}, 14.0f + i * 12, 16.0f + i * 12, 200, 340, 16,
               Faded(FLOUR, 0.5f * std::fmod(time * 1.5f - i * 0.3f, 1.0f)));
    DrawButton(ANSWER_BUTTON, "Answer", {60, 150, 90, 255});
    DrawButton(NOT_NOW_BUTTON, "Not now", Faded(FLOUR, 0.15f));
  }

  void DrawMemoryBanner() const {
    const float a =
        std::fmin(1.0f, memoryTimer); // fades out in the last second
    DrawRectangleRounded({160, 576, 400, 70}, 0.4f, 8,
                         Faded({20, 20, 30, 255}, 0.8f * a));
    DrawTextCentered("A memory comes back", 360, 582, 18, Faded(MUTED, a));
    DrawTextCentered(memoryName, 360, 604, 30, Faded(FLOUR, a), HAND_FONT);
  }
};

std::unique_ptr<Scene> MakeExploreScene(const Step &step) {
  return std::unique_ptr<Scene>(new ExploreScene(step));
}
