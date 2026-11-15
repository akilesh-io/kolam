// art.cpp - the people of the game, drawn with shapes until the hand-drawn
// art arrives: Amma, Anu, little Anu and Appa. Each one is drawn from the
// feet up, `height` units tall, so the same drawing works big or small.
// The screen shader (main.cpp) adds the ink outlines and the paper.
#include <cmath>

#include "game.h"

// Skin, hair and cloth colours, each with a darker shade for the side away
// from the light (the light comes from the upper left)
const Color SKIN_LIGHT = {176, 118, 80, 255};
const Color SKIN_SHADE = {142, 90, 60, 255};
const Color HAIR_BLACK = {34, 26, 28, 255};
const Color HAIR_SHINE = {70, 58, 64, 255};
const Color HAIR_GREY = {150, 146, 150, 255};
const Color ZARI = /* gold thread */ {226, 178, 70, 255};
const Color JASMINE = {252, 250, 236, 255};
const Color KUMKUM_RED = {200, 40, 40, 255};

// A point of a person, in their own units: x across (0 = middle, + = right)
// and y up from the feet, both as fractions of their height
struct Body {
  Vector2 feet;
  float height;
  Vector2 At(float x, float y) const {
    return {feet.x + x * height, feet.y - y * height};
  }
  float Size(float s) const { return s * height; }
};

static void Ellipse(const Body &b, float x, float y, float w, float h,
                    Color c) {
  const Vector2 p = b.At(x, y);
  DrawEllipse((int)p.x, (int)p.y, b.Size(w), b.Size(h), c);
}

static void Circle(const Body &b, float x, float y, float r, Color c) {
  DrawCircleV(b.At(x, y), b.Size(r), c);
}

// A four-sided shape through the given points (any order around the edge)
static void Quad(const Body &b, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                 Color c) {
  const Vector2 a = b.At(p1.x, p1.y), bb = b.At(p2.x, p2.y),
                cc = b.At(p3.x, p3.y), d = b.At(p4.x, p4.y);
  // raylib wants triangles in counter-clockwise order; draw both ways round
  // so the order of the points never matters
  DrawTriangle(a, bb, cc, c);
  DrawTriangle(a, cc, bb, c);
  DrawTriangle(a, cc, d, c);
  DrawTriangle(a, d, cc, c);
}

static void Limb(const Body &b, Vector2 from, Vector2 to, float thickness,
                 Color c) {
  DrawLineEx(b.At(from.x, from.y), b.At(to.x, to.y), b.Size(thickness), c);
  Circle(b, to.x, to.y, thickness / 2, c); // rounded ends
  Circle(b, from.x, from.y, thickness / 2, c);
}

static void Shadow(const Body &b, float width) {
  Ellipse(b, 0, 0, width, 0.025f, {0, 0, 0, 60});
}

// A head: face, hair with a centre parting, ears with gold studs. `turn`
// from -1 (facing left) to 1 (facing right) moves the face that way.
// `eyesClosed` for a smile with closed eyes.
static void Head(const Body &b, float y, float turn, Color hair, bool grey,
                 bool eyesClosed = false) {
  const float r = 0.062f;
  Circle(b, 0, y, r * 1.08f, hair); // the back of the hair
  Ellipse(b, 0.012f, y - 0.006f, r * 0.86f, r * 1.02f, SKIN_LIGHT); // face
  Ellipse(b, 0.045f, y - 0.01f, r * 0.35f, r * 0.85f,
          Faded(SKIN_SHADE, 0.6f)); // shade on the right cheek
  // hair over the forehead, parted in the middle
  Ellipse(b, -0.024f, y + 0.045f, r * 0.62f, r * 0.42f, hair);
  Ellipse(b, 0.03f, y + 0.045f, r * 0.62f, r * 0.42f, hair);
  Ellipse(b, -0.02f, y + 0.055f, r * 0.3f, r * 0.12f, HAIR_SHINE);
  if (grey) { // a few grey streaks
    Limb(b, {-0.03f, y + 0.06f}, {-0.05f, y + 0.02f}, 0.004f, HAIR_GREY);
    Limb(b, {0.035f, y + 0.06f}, {0.055f, y + 0.025f}, 0.004f, HAIR_GREY);
  }
  // face: eyes, nose, mouth, turned a little
  const float fx = 0.012f + turn * 0.02f;
  if (eyesClosed) {
    for (int side = -1; side <= 1; side += 2) {
      const Vector2 e = b.At(fx + side * 0.024f, y + 0.005f);
      DrawRing(e, b.Size(0.008f), b.Size(0.011f), 200, 340, 8, HAIR_BLACK);
    }
  } else {
    Circle(b, fx - 0.024f, y + 0.004f, 0.0075f, HAIR_BLACK);
    Circle(b, fx + 0.024f, y + 0.004f, 0.0075f, HAIR_BLACK);
  }
  Limb(b, {fx + 0.004f, y - 0.002f}, {fx + 0.008f, y - 0.018f}, 0.004f,
       SKIN_SHADE); // nose
  const Vector2 m = b.At(fx + 0.004f, y - 0.034f);
  DrawRing(m, b.Size(0.012f), b.Size(0.016f), 20, 160, 8,
           {120, 50, 50, 255}); // a small smile
  // ears and gold studs
  Circle(b, -0.052f, y - 0.004f, 0.011f, SKIN_SHADE);
  Circle(b, -0.052f, y - 0.018f, 0.006f, ZARI);
}

//------------------------------------------------------------------------------
// Amma: a cotton saree (green, with a maroon and gold border), the pallu over
// her left shoulder, greying hair in a bun with jasmine, a red pottu.
//------------------------------------------------------------------------------
void DrawAmma(float x, float feetY, float time, bool stirring) {
  const Body b = {{x, feetY}, 300};
  const float breathe = 0.003f * std::sin(time * 1.6f);
  const Color saree = {46, 122, 88, 255}, sareeShade = {34, 96, 70, 255};
  const Color border = {150, 34, 54, 255};

  Shadow(b, 0.24f);
  // the saree: wide at the hem, pleats down the front
  Quad(b, {-0.2f, 0}, {0.21f, 0}, {0.13f, 0.5f}, {-0.13f, 0.5f}, saree);
  Quad(b, {0.06f, 0}, {0.21f, 0}, {0.13f, 0.5f}, {0.05f, 0.5f}, sareeShade);
  for (int i = 0; i < 4; i++) // pleats
    Limb(b, {-0.02f + i * 0.02f, 0.45f}, {-0.05f + i * 0.035f, 0.02f}, 0.004f,
         sareeShade);
  Quad(b, {-0.2f, 0}, {0.21f, 0}, {0.205f, 0.045f}, {-0.195f, 0.045f}, border);
  Quad(b, {-0.197f, 0.04f}, {0.207f, 0.04f}, {0.206f, 0.05f}, {-0.196f, 0.05f},
       ZARI);
  // the blouse and the waist
  Quad(b, {-0.12f, 0.5f}, {0.12f, 0.5f}, {0.13f, 0.78f + breathe},
       {-0.13f, 0.78f + breathe}, {130, 30, 50, 255});
  Ellipse(b, 0.02f, 0.53f, 0.1f, 0.025f, SKIN_SHADE); // a little bare waist
  // the pallu: from the left shoulder down across to the right hip
  Quad(b, {-0.14f, 0.79f + breathe}, {-0.04f, 0.8f + breathe}, {0.14f, 0.5f},
       {0.06f, 0.44f}, border);
  Limb(b, {-0.04f, 0.8f + breathe}, {0.14f, 0.5f}, 0.008f, ZARI);
  // arms
  Limb(b, {-0.14f, 0.74f}, {-0.17f, 0.5f}, 0.045f, SKIN_LIGHT);
  if (stirring) { // the right arm forward, to the pot, moving gently
    const float stir = 0.015f * std::sin(time * 3);
    Limb(b, {0.13f, 0.74f}, {0.2f, 0.6f}, 0.045f, SKIN_SHADE);
    Limb(b, {0.2f, 0.6f}, {0.32f + stir, 0.64f}, 0.04f, SKIN_SHADE);
  } else { // hands together in front, holding the end of the pallu
    Limb(b, {0.13f, 0.74f}, {0.07f, 0.55f}, 0.045f, SKIN_SHADE);
    Limb(b, {-0.17f, 0.5f}, {-0.02f, 0.53f}, 0.04f, SKIN_LIGHT);
  }
  for (int i = 0; i < 3; i++) // glass bangles
    Limb(b, {-0.172f + i * 0.004f, 0.53f}, {-0.165f + i * 0.004f, 0.52f}, 0.01f,
         i == 1 ? ZARI : Color{40, 120, 80, 255});
  // neck, head, the bun with jasmine
  Quad(b, {-0.025f, 0.78f}, {0.03f, 0.78f}, {0.025f, 0.84f}, {-0.02f, 0.84f},
       SKIN_SHADE);
  Circle(b, -0.075f, 0.88f + breathe, 0.04f, HAIR_BLACK); // the bun
  for (int i = 0; i < 7; i++) {                           // jasmine string
    const float a = 1.2f + i * 0.45f;
    Circle(b, -0.075f + 0.045f * std::cos(a),
           0.88f + breathe + 0.045f * std::sin(a), 0.009f, JASMINE);
  }
  Head(b, 0.9f + breathe, stirring ? 0.6f : 0.1f, HAIR_BLACK, true);
  Circle(b, 0.012f + (stirring ? 0.012f : 0.002f), 0.925f + breathe, 0.006f,
         KUMKUM_RED); // pottu
}

//------------------------------------------------------------------------------
// Anu, grown up: a mustard kurta, dark leggings, hair in a loose ponytail.
// Sitting back on her heels, the way you sit to draw a kolam.
//------------------------------------------------------------------------------
void DrawAnuSitting(float x, float groundY, float time) {
  const Body b = {{x, groundY}, 260}; // as tall as she would be standing
  const float breathe = 0.003f * std::sin(time * 1.4f);
  const Color kurta = {214, 160, 58, 255}, kurtaShade = {182, 128, 40, 255};
  const Color leggings = {50, 54, 80, 255};

  Shadow(b, 0.2f);
  // folded legs: knees forward, feet tucked under
  Ellipse(b, 0.02f, 0.06f, 0.16f, 0.06f, leggings);
  Ellipse(b, -0.1f, 0.03f, 0.06f, 0.03f, SKIN_SHADE); // a heel
  // the kurta over the lap and body
  Quad(b, {-0.12f, 0.07f}, {0.15f, 0.06f}, {0.1f, 0.42f + breathe},
       {-0.1f, 0.42f + breathe}, kurta);
  Quad(b, {0.04f, 0.06f}, {0.15f, 0.06f}, {0.1f, 0.42f + breathe},
       {0.04f, 0.42f + breathe}, kurtaShade);
  Limb(b, {-0.03f, 0.4f}, {-0.03f, 0.3f}, 0.006f, {150, 40, 50, 255}); // trim
  // arms resting on the knees, a little flour on her fingers
  Limb(b, {-0.09f, 0.39f}, {-0.05f, 0.18f}, 0.04f, kurta);
  Limb(b, {-0.05f, 0.18f}, {0.08f, 0.13f}, 0.035f, SKIN_LIGHT);
  Limb(b, {0.09f, 0.39f}, {0.12f, 0.2f}, 0.04f, kurtaShade);
  Circle(b, 0.12f, 0.15f, 0.022f, SKIN_SHADE);
  Circle(b, 0.12f, 0.13f, 0.01f, FLOUR);
  // neck, ponytail, head
  Quad(b, {-0.02f, 0.41f}, {0.025f, 0.41f}, {0.02f, 0.46f}, {-0.017f, 0.46f},
       SKIN_SHADE);
  Limb(b, {-0.06f, 0.52f + breathe}, {-0.1f, 0.4f}, 0.04f, HAIR_BLACK);
  Head(b, 0.51f + breathe, 0.3f, HAIR_BLACK, false);
}

//------------------------------------------------------------------------------
// Little Anu, eight: a pink pattu pavadai with a gold border, two plaits with
// red ribbons. Holding the kite reel in both hands.
//------------------------------------------------------------------------------
void DrawLittleAnu(float x, float feetY, float time) {
  const Body b = {{x, feetY}, 95};
  const float bounce = 0.01f * std::fabs(std::sin(time * 4)); // excited
  const Color pavadai = {226, 84, 132, 255}, shade = {190, 60, 108, 255};

  Shadow(b, 0.28f);
  Limb(b, {-0.05f, 0.02f}, {-0.05f, 0.1f}, 0.05f, SKIN_SHADE); // feet
  Limb(b, {0.05f, 0.02f}, {0.05f, 0.1f}, 0.05f, SKIN_SHADE);
  Quad(b, {-0.22f, 0.06f}, {0.22f, 0.06f}, {0.1f, 0.52f + bounce},
       {-0.1f, 0.52f + bounce}, pavadai);
  Quad(b, {0.06f, 0.06f}, {0.22f, 0.06f}, {0.1f, 0.52f}, {0.05f, 0.52f}, shade);
  Quad(b, {-0.22f, 0.06f}, {0.22f, 0.06f}, {0.215f, 0.12f}, {-0.215f, 0.12f},
       ZARI);
  Quad(b, {-0.11f, 0.5f + bounce}, {0.11f, 0.5f + bounce},
       {0.1f, 0.72f + bounce}, {-0.1f, 0.72f + bounce},
       {120, 60, 160, 255}); // the blouse
  // both hands out in front, holding the reel
  Limb(b, {-0.1f, 0.68f + bounce}, {0.1f, 0.56f}, 0.06f, SKIN_LIGHT);
  Limb(b, {0.1f, 0.68f + bounce}, {0.16f, 0.56f}, 0.06f, SKIN_SHADE);
  // plaits with ribbons
  for (int side = -1; side <= 1; side += 2) {
    Limb(b, {side * 0.09f, 0.92f + bounce}, {side * 0.13f, 0.66f + bounce},
         0.05f, HAIR_BLACK);
    Circle(b, side * 0.13f, 0.66f + bounce, 0.03f, KUMKUM_RED);
  }
  // a big head, as children have: drawn as if for someone 1.7 times taller,
  // centred where her head goes. Eyes closed, smiling at the kite.
  const Body head = {b.At(0, 0.86f + bounce), b.height * 1.7f};
  Head(head, 0, 0.4f, HAIR_BLACK, false, true);
}

//------------------------------------------------------------------------------
// Appa: a white veshti with a thin gold border, a light blue half-sleeve
// shirt, a moustache, glasses, greying hair. Watching the kite.
//------------------------------------------------------------------------------
void DrawAppa(float x, float feetY, float time) {
  const Body b = {{x, feetY}, 150};
  const float sway = 0.004f * std::sin(time * 1.2f);
  const Color veshti = {246, 242, 230, 255}, veshtiShade = {214, 208, 194, 255};
  const Color shirt = {150, 186, 220, 255}, shirtShade = {118, 152, 190, 255};

  Shadow(b, 0.22f);
  Limb(b, {-0.05f, 0.01f}, {-0.05f, 0.06f}, 0.05f, SKIN_SHADE); // feet
  Limb(b, {0.06f, 0.01f}, {0.06f, 0.06f}, 0.05f, SKIN_SHADE);
  Quad(b, {-0.13f, 0.04f}, {0.14f, 0.04f}, {0.13f, 0.5f}, {-0.12f, 0.5f},
       veshti);
  Quad(b, {0.04f, 0.04f}, {0.14f, 0.04f}, {0.13f, 0.5f}, {0.04f, 0.5f},
       veshtiShade);
  Limb(b, {0.0f, 0.5f}, {0.02f, 0.05f}, 0.006f, ZARI); // the border, folded
  // shirt, a little belly
  Quad(b, {-0.15f, 0.48f}, {0.16f, 0.48f}, {0.15f, 0.8f + sway},
       {-0.14f, 0.8f + sway}, shirt);
  Quad(b, {0.05f, 0.48f}, {0.16f, 0.48f}, {0.15f, 0.8f + sway},
       {0.05f, 0.8f + sway}, shirtShade);
  Ellipse(b, 0.01f, 0.56f, 0.14f, 0.05f, shirt);
  for (int i = 0; i < 4; i++) // buttons
    Circle(b, 0.0f, 0.55f + i * 0.06f, 0.006f, {240, 240, 240, 255});
  // one arm down, one pointing up at the kite
  Limb(b, {-0.15f, 0.76f}, {-0.17f, 0.52f}, 0.05f, SKIN_LIGHT);
  Limb(b, {0.15f, 0.76f}, {0.24f, 0.86f}, 0.05f, SKIN_SHADE);
  Limb(b, {0.24f, 0.86f}, {0.3f, 1.0f}, 0.045f, SKIN_SHADE);
  Quad(b, {-0.025f, 0.79f}, {0.03f, 0.79f}, {0.025f, 0.84f}, {-0.02f, 0.84f},
       SKIN_SHADE);
  Head(b, 0.9f + sway, 0.5f, HAIR_BLACK, true);
  // moustache and glasses
  Ellipse(b, 0.024f, 0.88f + sway, 0.03f, 0.008f, HAIR_BLACK);
  for (int side = -1; side <= 1; side += 2) {
    const Vector2 e = b.At(0.022f + side * 0.024f, 0.905f + sway);
    DrawRing(e, b.Size(0.014f), b.Size(0.018f), 0, 360, 16, {60, 50, 40, 255});
  }
}

//------------------------------------------------------------------------------
// Scenery: things the places share. Backgrounds reach across the whole screen
// (FullScreen()), not just the stage.
//------------------------------------------------------------------------------

// A soft dark ellipse under something, so it sits on the ground
void DrawSoftShadow(float x, float y, float width) {
  DrawEllipse((int)x, (int)y, width / 2, width / 12, {0, 0, 0, 55});
  DrawEllipse((int)x, (int)y, width / 3, width / 18, {0, 0, 0, 35});
}

// A range of hills whose tops roll along at about `topY`, filled down to
// `bottomY`. `seed` gives each range its own shape.
void DrawHills(float topY, float bottomY, float rise, int seed, Color color) {
  const Rectangle all = FullScreen();
  const float step = 4;
  for (float x = all.x; x < all.x + all.width; x += step) {
    const float y = topY - rise * (0.5f + 0.3f * std::sin(x * 0.006f + seed) +
                                   0.2f * std::sin(x * 0.017f + seed * 2.3f));
    DrawRectangleRec({x, y, step + 1, bottomY - y}, color);
  }
}

// A coconut palm: a gently curving trunk and drooping fronds
void DrawPalm(float x, float groundY, float height, float time, Color trunk,
              Color leaves) {
  const float lean = height * 0.12f;
  Vector2 top = {x + lean, groundY - height};
  const Vector2 mid = {x + lean * 0.2f, groundY - height * 0.5f};
  DrawSplineSegmentBezierQuadratic({x, groundY}, mid, top, height * 0.05f,
                                   trunk);
  for (int i = 0; i < 4; i++) // rings on the trunk
    DrawCircleV(
        {x + lean * (0.1f + i * 0.2f), groundY - height * (0.15f + i * 0.2f)},
        height * 0.026f, Faded(BLACK, 0.12f));
  // fronds: each rises a little from the top, then droops to its tip
  for (int i = 0; i < 8; i++) {
    const float side = (i % 2) ? 1.0f : -1.0f;
    const float reach = height * (0.2f + 0.07f * (i / 2)); // sideways
    const float droop = height * (0.08f + 0.06f * (3 - i / 2) +
                                  0.01f * std::sin(time * 1.3f + i));
    const Vector2 tip = {top.x + side * reach, top.y + droop};
    const Vector2 bend = {top.x + side * reach * 0.5f,
                          top.y - height * (0.12f - 0.025f * (i / 2))};
    DrawSplineSegmentBezierQuadratic(top, bend, tip, height * 0.03f, leaves);
  }
  DrawCircleV(top, height * 0.04f, {110, 80, 40, 255}); // coconuts
}

// A number from 0 to 1 that looks random but is always the same for `i`
static float Scatter(float i) {
  const float v = std::sin(i * 12.9898f) * 43758.5453f;
  return v - std::floor(v);
}

// A jasmine creeper (Sathyamangalam's malli): leaves and small white flowers
void DrawJasmine(Rectangle area, float time) {
  for (int i = 0; i < 40; i++) {
    const float x = area.x + Scatter(i + 0.5f) * area.width;
    const float y = area.y + Scatter(i + 100.5f) * area.height;
    DrawEllipse((int)x, (int)y, 9, 5,
                i % 3 ? Color{58, 120, 60, 255} : Color{80, 146, 70, 255});
  }
  for (int i = 0; i < 18; i++) {
    const float x = area.x + Scatter(i + 200.5f) * area.width;
    const float y =
        area.y + Scatter(i + 300.5f) * area.height + std::sin(time + i) * 0.8f;
    for (int p = 0; p < 5; p++) {
      const float a = p * 2 * PI / 5;
      DrawCircleV({x + 3.5f * std::cos(a), y + 3.5f * std::sin(a)}, 2.6f,
                  {252, 250, 238, 255});
    }
    DrawCircleV({x, y}, 1.5f, {240, 220, 140, 255});
  }
}

// A brass or steel pot with a shine on its upper left
void DrawPot(float x, float y, float r, bool brass) {
  const Color body =
      brass ? Color{214, 160, 64, 255} : Color{196, 198, 204, 255};
  const Color shade =
      brass ? Color{170, 116, 40, 255} : Color{150, 152, 160, 255};
  DrawCircleV({x, y}, r, shade);
  DrawCircleV({x - r * 0.12f, y - r * 0.1f}, r * 0.86f, body);
  DrawEllipse((int)(x - r * 0.4f), (int)(y - r * 0.4f), r * 0.22f, r * 0.12f,
              {255, 250, 230, 200}); // shine
  DrawRectangleRec({x - r * 0.45f, y - r * 1.12f, r * 0.9f, r * 0.25f}, shade);
}

// A tiled floor from `y` down to the bottom, its lines closer together near
// the back, so it lies flat
void DrawTiledFloor(float y, Color tile, Color line) {
  const Rectangle all = FullScreen();
  DrawBandGradient(y, 720 - y, tile,
                   {(unsigned char)(tile.r * 0.8f),
                    (unsigned char)(tile.g * 0.8f),
                    (unsigned char)(tile.b * 0.8f), 255});
  for (float gap = 10, ty = y + gap; ty < 720; gap *= 1.35f, ty += gap)
    DrawLineEx({all.x, ty}, {all.x + all.width, ty}, 1.5f, line);
  for (int i = -12; i <= 12; i++) // lines towards a point far behind
    DrawLineEx({360 + i * 40.0f, y}, {360 + i * 140.0f, 720}, 1.5f, line);
}
