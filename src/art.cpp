// art.cpp - the people of the game, drawn with shapes until the hand-drawn
// art arrives: Amma, Anu, little Anu and Appa. Each one is drawn from the
// feet up, `height` units tall, so the same drawing works big or small.
//
// They are drawn in an anime (cel) style: clean dark outlines, flat colours
// with one shadow tone, big eyes with catch-lights, hair in locks with a band
// of shine. Everything that makes it that style is in `namespace anime`
// (outlines, faces, eyes, hair); the people below it only say what to draw
// where, so another style can replace the namespace and keep the people.
#include <cmath>

#include "game.h"

// Skin, hair and cloth colours, each with a darker shade for the side away
// from the light (the light comes from the upper left)
const Color SKIN_LIGHT = {184, 124, 86, 255};
const Color SKIN_SHADE = {150, 96, 68, 255};
const Color HAIR_BLACK = {48, 40, 62, 255}; // blue-black, the anime way
const Color HAIR_SHADE = {28, 22, 38, 255};
const Color HAIR_SHINE = {116, 112, 156, 255};
const Color HAIR_GREY = {186, 182, 196, 255};
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
  // How thick the ink outlines are: about the same on everyone, so small
  // people's faces don't drown in ink
  float Line() const { return std::fmax(1.0f, height * 0.0042f); }
};

//------------------------------------------------------------------------------
// The anime style
//------------------------------------------------------------------------------
namespace anime {

const Color INK = {58, 34, 40, 255}; // the outlines: a warm dark brown
const Color EYE_WHITE = {252, 248, 242, 255};
const Color IRIS = {128, 70, 42, 255};       // warm brown eyes,
const Color IRIS_DARK = {66, 34, 28, 255};   // darker under the lid
const Color IRIS_GLOW = {222, 150, 80, 255}; // and glowing at the bottom
const Color PUPIL = {34, 18, 20, 255};
const Color CATCH_LIGHT = {255, 255, 255, 255};
const Color LIPS = {136, 56, 58, 255};
const Color MOUTH = {122, 38, 50, 255};
const Color TONGUE = {230, 118, 118, 255};
const Color BLUSH = {244, 112, 112, 80};
const Color NOSE = {128, 78, 56, 255};
const Color SHADOW = {44, 20, 66, 72}; // cel shadow on cloth, see-through

//--- Points ------------------------------------------------------------------
static Vector2 Plus(Vector2 a, Vector2 b) { return {a.x + b.x, a.y + b.y}; }
static Vector2 Minus(Vector2 a, Vector2 b) { return {a.x - b.x, a.y - b.y}; }
static Vector2 Scaled(Vector2 a, float s) { return {a.x * s, a.y * s}; }
static Vector2 Unit(Vector2 a) {
  const float length = std::sqrt(a.x * a.x + a.y * a.y);
  return length > 0 ? Scaled(a, 1 / length) : Vector2{0, 0};
}
static float Dot(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }
// The point `t` (0 to 1) along a curve from `a`, bending towards `bend`, to
// `b`
static Vector2 Curve(Vector2 a, Vector2 bend, Vector2 b, float t) {
  const float s = 1 - t;
  return Plus(Plus(Scaled(a, s * s), Scaled(bend, 2 * s * t)),
              Scaled(b, t * t));
}

//--- Shapes with ink outlines -------------------------------------------------
// Each shape is drawn twice: grown by the line width in ink, then filled on
// top. Drawing the outlines of several shapes first and then their fills
// joins them under one outline (that's how hair is drawn).
const int MAX_POINTS = 24;

// A shape through points going round its edge; roughly convex, since it is
// filled as a fan from the points' middle
struct Shape {
  Vector2 p[2 * MAX_POINTS];
  int n = 0;
  void Add(Vector2 point) {
    if (n < 2 * MAX_POINTS)
      p[n++] = point;
  }
};

// A strip between two edges side by side, a[i] across from b[i]: a lock of
// hair, an eyelid, a fold of cloth. It can curve any way.
struct Strip {
  Vector2 a[MAX_POINTS], b[MAX_POINTS];
  int n = 0;
};

// The edge `p` (`n` points going round a shape) moved outwards by `by`
static void Grow(const Vector2 *p, int n, float by, Vector2 *out) {
  float area = 0; // its sign says which way round the points go
  for (int i = 0; i < n; i++)
    area += p[i].x * p[(i + 1) % n].y - p[(i + 1) % n].x * p[i].y;
  const float side = area > 0 ? 1.0f : -1.0f;
  for (int i = 0; i < n; i++) {
    const Vector2 in = Unit(Minus(p[i], p[(i + n - 1) % n]));
    const Vector2 on = Unit(Minus(p[(i + 1) % n], p[i]));
    const Vector2 n1 = {side * in.y, -side * in.x};
    const Vector2 n2 = {side * on.y, -side * on.x};
    const Vector2 dir = Unit(Plus(n1, n2));
    // out further at corners so the line keeps its width; not too far at
    // sharp points
    const float corner = std::fmax(0.45f, std::fmax(Dot(dir, n1), Dot(dir, n2)));
    out[i] = Plus(p[i], Scaled(dir, by / corner));
  }
}

static void Fill(const Shape &s, Color c) {
  Vector2 middle = {0, 0};
  for (int i = 0; i < s.n; i++)
    middle = Plus(middle, s.p[i]);
  middle = Scaled(middle, 1.0f / s.n);
  for (int i = 0; i < s.n; i++) {
    const Vector2 a = s.p[i], b = s.p[(i + 1) % s.n];
    // raylib draws one winding of triangle; draw both, so either way works
    DrawTriangle(middle, a, b, c);
    DrawTriangle(middle, b, a, c);
  }
}

static void Fill(const Strip &s, Color c) {
  for (int i = 0; i + 1 < s.n; i++) {
    const Vector2 a0 = s.a[i], a1 = s.a[i + 1], b0 = s.b[i], b1 = s.b[i + 1];
    DrawTriangle(a0, a1, b1, c);
    DrawTriangle(a0, b1, a1, c);
    DrawTriangle(a0, b1, b0, c);
    DrawTriangle(a0, b0, b1, c);
  }
}

static void Outline(const Shape &s, float line) {
  Shape grown = s;
  Grow(s.p, s.n, line, grown.p);
  Fill(grown, INK);
}

static void Outline(const Strip &s, float line) {
  Vector2 ring[2 * MAX_POINTS], grown[2 * MAX_POINTS];
  const int n = s.n;
  for (int i = 0; i < n; i++) {
    ring[i] = s.a[i];
    ring[2 * n - 1 - i] = s.b[i];
  }
  Grow(ring, 2 * n, line, grown);
  Strip g;
  g.n = n;
  for (int i = 0; i < n; i++) {
    g.a[i] = grown[i];
    g.b[i] = grown[2 * n - 1 - i];
  }
  Fill(g, INK);
}

template <typename T> static void Inked(const T &shape, Color c, float line) {
  Outline(shape, line);
  Fill(shape, c);
}

// A strip along the curve from `from`, bending towards `bend`, to `to`: `w0`
// wide at the start and `w1` at the end, and `bulge` wider in the middle (so
// one 0 wide at both ends is a leaf, a jasmine bud or a streak of shine)
static Strip Along(Vector2 from, Vector2 bend, Vector2 to, float w0, float w1,
                   float bulge = 0) {
  Strip s;
  s.n = 12;
  for (int i = 0; i < s.n; i++) {
    const float t = i / (s.n - 1.0f);
    const Vector2 p = Curve(from, bend, to, t);
    const Vector2 d = Unit(Plus(Scaled(Minus(bend, from), 1 - t),
                                Scaled(Minus(to, bend), t)));
    const float half =
        0.5f * (w0 + (w1 - w0) * t + bulge * 4 * t * (1 - t)) + 0.02f;
    s.a[i] = {p.x - d.y * half, p.y + d.x * half};
    s.b[i] = {p.x + d.y * half, p.y - d.x * half};
  }
  return s;
}

// An ellipse with an ink outline
static void InkEllipse(Vector2 p, float rx, float ry, Color c, float line) {
  DrawEllipseV(p, rx + line, ry + line, INK);
  DrawEllipseV(p, rx, ry, c);
}

//--- Bodies, in a person's own units (see Body) -------------------------------
static void Ellipse(const Body &b, float x, float y, float w, float h, Color c,
                    bool ink = true) {
  const Vector2 p = b.At(x, y);
  if (ink)
    DrawEllipseV(p, b.Size(w) + b.Line(), b.Size(h) + b.Line(), INK);
  DrawEllipseV(p, b.Size(w), b.Size(h), c);
}

static void Circle(const Body &b, float x, float y, float r, Color c,
                   bool ink = true) {
  Ellipse(b, x, y, r, r, c, ink);
}

// A four-sided shape through the given points, going round its edge
static void Quad(const Body &b, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                 Color c, bool ink = true) {
  Shape s;
  for (Vector2 p : {p1, p2, p3, p4})
    s.Add(b.At(p.x, p.y));
  if (ink)
    Outline(s, b.Line());
  Fill(s, c);
}

// A straight piece with rounded ends; `ink` false for lines inside a shape
static void Limb(const Body &b, Vector2 from, Vector2 to, float thickness,
                 Color c, bool ink = true) {
  const Vector2 f = b.At(from.x, from.y), t = b.At(to.x, to.y);
  const float r = b.Size(thickness) / 2;
  for (int pass = ink ? 0 : 1; pass < 2; pass++) {
    const float grow = pass == 0 ? b.Line() : 0;
    const Color col = pass == 0 ? INK : c;
    DrawLineEx(f, t, 2 * (r + grow), col);
    DrawCircleV(f, r + grow, col);
    DrawCircleV(t, r + grow, col);
  }
}

// A bent arm, from the shoulder through the elbow to a small hand, thinner
// towards the wrist and outlined as one piece. A sleeve covers the top
// `sleeve` of the upper arm (0 for none).
static void Arm(const Body &b, Vector2 shoulder, Vector2 elbow, Vector2 hand,
                float thickness, Color skin, float sleeve = 0,
                Color cloth = BLANK) {
  const Vector2 p0 = b.At(shoulder.x, shoulder.y), p1 = b.At(elbow.x, elbow.y),
                p2 = b.At(hand.x, hand.y);
  const float r = b.Size(thickness) / 2, line = b.Line();
  const Strip upper = Along(p0, Scaled(Plus(p0, p1), 0.5f), p1, 2 * r, 1.8f * r);
  const Strip lower =
      Along(p1, Scaled(Plus(p1, p2), 0.5f), p2, 1.8f * r, 1.45f * r);
  Outline(upper, line);
  Outline(lower, line);
  DrawCircleV(p0, r + line, INK);
  DrawCircleV(p1, 0.9f * r + line, INK);
  DrawCircleV(p2, r + line, INK); // the hand
  Fill(upper, skin);
  Fill(lower, skin);
  DrawCircleV(p0, r, skin);
  DrawCircleV(p1, 0.9f * r, skin);
  DrawCircleV(p2, r, skin);
  if (sleeve > 0) { // puffed a little at the shoulder
    const Vector2 end = Plus(p0, Scaled(Minus(p1, p0), sleeve));
    Inked(Along(Plus(p0, Scaled(Minus(p0, end), 0.25f)),
                Scaled(Plus(p0, end), 0.5f), end, 2.5f * r, 2.3f * r,
                0.5f * r),
          cloth, line);
  }
}

static void Shadow(const Body &b, float width) {
  Ellipse(b, 0, 0, width, 0.025f, {0, 0, 0, 60}, false);
}

// A piece of clothing: the shape through body points `pts` (going round its
// edge), its corners softened a little
static Shape Cloth(const Body &b, std::initializer_list<Vector2> pts) {
  Vector2 p[MAX_POINTS];
  int n = 0;
  for (Vector2 q : pts)
    if (n < MAX_POINTS)
      p[n++] = b.At(q.x, q.y);
  Shape s;
  const float soft = b.Size(0.008f);
  for (int i = 0; i < n; i++) {
    for (Vector2 next : {p[(i + n - 1) % n], p[(i + 1) % n]}) {
      const Vector2 d = Minus(next, p[i]);
      const float length = std::sqrt(Dot(d, d));
      s.Add(Plus(p[i], Scaled(d, length > 0 ? std::fmin(0.4f, soft / length) : 0)));
    }
  }
  return s;
}

// The shadow on a piece of clothing: right of a line from `xTop` (at its top)
// to `xBottom` (at its bottom), across in body units. It is a see-through
// wash, so borders and pleats drawn on the cloth go into shadow too.
static void Wash(const Body &b, const Shape &s, float xTop, float xBottom) {
  float top = s.p[0].y, bottom = s.p[0].y;
  for (int i = 0; i < s.n; i++) {
    top = std::fmin(top, s.p[i].y);
    bottom = std::fmax(bottom, s.p[i].y);
  }
  Shape dark = s;
  for (int i = 0; i < s.n; i++) {
    const float k = (s.p[i].y - top) / std::fmax(1.0f, bottom - top);
    const float split = b.At(xTop + (xBottom - xTop) * k, 0).x;
    dark.p[i].x = std::fmax(s.p[i].x, split);
  }
  Fill(dark, SHADOW);
}

// A band along a hem that curves from `left` through `mid` to `right` (body
// points), from `from` to `to` above it, coming in a little at the sides as
// the cloth narrows upwards: for borders
static Strip HemBand(const Body &b, Vector2 left, Vector2 mid, Vector2 right,
                     float from, float to) {
  const Vector2 bend = Minus(Scaled(mid, 2), Scaled(Plus(left, right), 0.5f));
  Strip s;
  s.n = 12;
  for (int i = 0; i < s.n; i++) {
    const Vector2 p = Curve(left, bend, right, i / (s.n - 1.0f));
    const float in = 0.12f * (p.x - mid.x);
    s.a[i] = b.At(p.x - in * from, p.y + from);
    s.b[i] = b.At(p.x - in * to, p.y + to);
  }
  return s;
}

// The point `t` of the way along a hem (see HemBand)
static Vector2 HemPoint(Vector2 left, Vector2 mid, Vector2 right, float t) {
  return Curve(left, Minus(Scaled(mid, 2), Scaled(Plus(left, right), 0.5f)),
               right, t);
}

//--- Heads -------------------------------------------------------------------
// A head seen a little from one side. Head points (u, v) are in radii of
// the skull from its middle: u across, v up. `turn` from -1 (facing left) to
// 1 (facing right) moves the face that way.
struct Head {
  Vector2 centre;
  float R, turn, line;
  Vector2 P(float u, float v) const {
    return {centre.x + u * R, centre.y - v * R};
  }
  Vector2 P(Vector2 uv) const { return P(uv.x, uv.y); }
  float S(float s) const { return s * R; }
  float Mid() const { return 0.34f * turn; } // the middle of the face, across
  // How much narrower things on the far side of the face look
  float Far(int side) const {
    return side * turn > 0 ? 1 - 0.38f * std::fabs(turn) : 1;
  }
  // A point on the front of the head, moved round as the head turns
  Vector2 Front(float u, float v) const {
    return P(u + Mid() * std::fmax(0.0f, 1 - u * u), v);
  }
};

// How someone's face is drawn. Sizes are in radii of the skull.
struct Face {
  float eyeWidth = 0.34f, eyeHeight = 0.38f;
  float eyeSpread = 0.44f; // from the middle of the face to each eye
  float eyeY = -0.2f;      // the eye line
  float jaw = 1;           // chin length: 1 grown up, less for a child
  float cheeks = 1;        // how wide the jaw is
  Vector2 look = {0, 0};   // where the eyes look, -1 to 1 (y down)
  bool lashes = false;     // a woman's lashes, flicked at the outer corner
  bool soft = false;       // lower lids: kind, older eyes
  bool happy = false;      // eyes closed in a smile (^ ^)
  bool openSmile = false;  // a happy open mouth
  float glow = 1;          // how much the bottom of the iris glows
  float blush = 1;         // how rosy the cheeks are
  float pottu = 0;         // the size of the kumkum on her forehead (0: none)
  float blink = 0;         // when the blinks come (each person differently)
};

// A strip on a head, in head points
static Strip HeadStrip(const Head &h, Vector2 from, Vector2 bend, Vector2 to,
                       float w0, float w1, float bulge = 0) {
  return Along(h.Front(from.x, from.y), h.Front(bend.x, bend.y),
               h.Front(to.x, to.y), h.S(w0), h.S(w1), h.S(bulge));
}

// A lock of hair from its root to its tip, `width` wide at its widest:
// narrow at the root, widest a third of the way down, then to a point
static Strip Lock(const Head &h, Vector2 root, Vector2 bend, Vector2 tip,
                  float width) {
  return HeadStrip(h, root, bend, tip, width * 0.35f, 0, width * 0.9f);
}

// The face's edge: the round skull, the cheeks and a soft chin. The chin
// moves and the far cheek narrows as the face turns.
static Shape FaceShape(const Head &h, float jaw, float cheeks) {
  // from the cheekbone down to the middle of the chin
  static const float JAW[7][2] = {{1.0f, 0.0f},    {0.99f, -0.3f},
                                  {0.93f, -0.6f},  {0.8f, -0.86f},
                                  {0.58f, -1.06f}, {0.3f, -1.18f},
                                  {0, -1.22f}};
  Vector2 edge[13]; // left cheekbone, round the chin, to the right one
  for (int k = 0; k < 13; k++) {
    const int i = k < 7 ? k : 12 - k;
    const int side = k < 7 ? -1 : 1;
    const float down = i / 6.0f; // 0 at the cheekbone, 1 at the chin
    const float squeeze = 1 - (1 - h.Far(side)) * 0.4f;
    edge[k] = {side * JAW[i][0] * (1 - (1 - cheeks) * down) * squeeze + 0.8f * h.Mid() * down * down,
               JAW[i][1] * jaw};
  }
  Shape s;
  for (int i = 0; i <= 8; i++) // over the top, from right to left
    s.Add(h.P(std::cos(i * PI / 8), std::sin(i * PI / 8)));
  for (int k = 0; k + 1 < 13; k++) { // cut the corners, for a smooth jaw
    s.Add(h.P(Plus(Scaled(edge[k], 0.75f), Scaled(edge[k + 1], 0.25f))));
    s.Add(h.P(Plus(Scaled(edge[k], 0.25f), Scaled(edge[k + 1], 0.75f))));
  }
  return s;
}

// One eye: `e` its middle, `w` by `h` big; `outer` which way its outer corner
// points (-1 left, 1 right)
static void Eye(Vector2 e, float w, float h, int outer, const Face &f,
                float line, bool blink) {
  const float o = (float)outer;
  // (x towards the outer corner, y down), in eye sizes
  auto At = [&](float x, float y) { return Vector2{e.x + o * x * w, e.y + y * h}; };
  const float lid = std::fmax(line * 1.1f, h * 0.2f);
  if (f.happy || blink) { // closed: curved up in a smile, or resting
    const float bend = f.happy ? -0.6f : 0.3f;
    Fill(Along(At(-0.52f, 0.12f), At(0, bend), At(0.56f, 0.02f), lid * 0.5f,
               lid * 0.7f, lid * 0.4f),
         INK);
    return;
  }
  const float top = f.soft ? -0.28f : -0.5f; // the upper lid
  const float middle = (top + 0.54f) / 2, half = (0.54f - top) / 2;
  DrawEllipseV(At(0, middle), w * 0.5f, h * half, EYE_WHITE);
  // the iris, looking where the face looks
  const Vector2 iris = {e.x + f.look.x * w * 0.14f,
                        e.y + h * (middle + 0.04f + f.look.y * 0.05f)};
  const float rx = w * 0.31f, ry = h * half * 0.94f;
  DrawEllipseV(iris, rx, ry, IRIS);
  DrawEllipseV({iris.x, iris.y + ry * 0.5f}, rx * 0.68f, ry * 0.36f,
               ColorLerp(IRIS, IRIS_GLOW, f.glow));
  DrawEllipseV({iris.x, iris.y - ry * 0.34f}, rx, ry * 0.6f, IRIS_DARK);
  DrawEllipseV({iris.x, iris.y + ry * 0.04f}, rx * 0.46f, ry * 0.5f, PUPIL);
  // the upper lid: a thick line, thickest at the outer corner
  const float inner = middle + 0.02f, outerY = middle - (f.soft ? 0.02f : 0.14f);
  const Vector2 bend = At(-0.02f, 2 * top - 0.5f * (inner + outerY));
  Fill(Along(At(-0.54f, inner), bend, At(0.56f, outerY), lid * 0.55f,
             lid * 1.2f),
       INK);
  if (f.lashes) // flicked out at the corner
    Fill(Along(At(0.46f, outerY - 0.02f), At(0.66f, outerY - 0.04f),
               At(0.8f, outerY - 0.3f), lid * 0.9f, 0.0f),
         INK);
  if (h > 6) { // the lower lashes, and the fold above the lid
    Fill(Along(At(0.08f, 0.6f), At(0.36f, 0.6f), At(0.52f, 0.3f), lid * 0.2f,
               lid * 0.45f),
         Faded(INK, 0.85f));
    Fill(Along(At(-0.1f, top - 0.16f), At(0.2f, top - 0.26f),
               At(0.46f, outerY - 0.2f), 0, 0, line * 0.5f),
         Faded(INK, 0.45f));
  }
  // catch-lights: the window's light, from the upper left
  DrawCircleV({iris.x - rx * 0.36f, iris.y - ry * 0.34f},
              std::fmax(0.9f, rx * 0.4f), CATCH_LIGHT);
  DrawCircleV({iris.x + rx * 0.42f, iris.y + ry * 0.42f},
              std::fmax(0.6f, rx * 0.17f), Faded(CATCH_LIGHT, 0.85f));
}

// Where the eye on `side` (-1 left, 1 right) is; the far one is closer in
static Vector2 EyeCentre(const Head &h, const Face &f, int side) {
  return h.P(h.Mid() + side * f.eyeSpread * (0.15f + 0.85f * h.Far(side)),
             f.eyeY);
}

// Both eyes and their brows; the far eye is narrower
static void Eyes(const Head &h, const Face &f, bool blink) {
  for (int side = -1; side <= 1; side += 2) {
    const float w = h.S(f.eyeWidth) * h.Far(side), eh = h.S(f.eyeHeight);
    const Vector2 e = EyeCentre(h, f, side);
    Eye(e, w, eh, side, f, h.line, blink);
    // the brow: thick at the inner end, thinning out, gently arched
    const float o = (float)side, y = e.y - eh * (f.soft ? 1.05f : 1.15f);
    Fill(Along({e.x - o * w * 0.4f, y + eh * 0.12f}, {e.x + o * w * 0.1f, y - eh * 0.22f},
               {e.x + o * w * 0.66f, y + eh * 0.12f},
               std::fmax(h.line, eh * (f.soft ? 0.11f : 0.15f)), h.line * 0.4f),
         HAIR_SHADE);
  }
}

// The nose, the mouth, the blush and the pottu
static void FaceDetails(const Head &h, const Face &f) {
  const float m = h.Mid(), t = h.turn;
  for (int side = -1; side <= 1; side += 2) { // blush on both cheeks
    const float squeeze = h.Far(side);
    DrawEllipseV(h.P(m + side * 0.54f * squeeze, f.eyeY - 0.36f),
                 h.S(0.19f) * squeeze, h.S(0.085f), Faded(BLUSH, f.blush));
  }
  // the nose: just a small tick of shadow, the anime way
  const float nose = m + 0.1f * t;
  Fill(Along(h.P(nose + 0.02f, -0.44f), h.P(nose + 0.1f, -0.5f),
             h.P(nose + 0.02f, -0.56f), h.line * 0.7f, h.line * 0.9f),
       NOSE);
  // the mouth
  const float mu = m + 0.06f * t, mv = -0.8f * f.jaw;
  if (f.openSmile) {
    Shape mouth;
    for (int i = 0; i <= 8; i++) {
      const float a = i * PI / 8; // the bottom half, left to right
      mouth.Add(h.P(mu - 0.17f * std::cos(a), mv - 0.2f * std::sin(a)));
    }
    mouth.Add(h.P(mu, mv + 0.03f));
    Inked(mouth, MOUTH, h.line * 0.7f);
    DrawEllipseV(h.P(mu + 0.02f, mv - 0.13f), h.S(0.085f), h.S(0.05f),
                 TONGUE);
  } else {
    Fill(Along(h.P(mu - 0.16f, mv + 0.04f), h.P(mu, mv - 0.1f),
               h.P(mu + 0.16f, mv + 0.04f), h.line * 0.5f, h.line * 0.5f,
               std::fmax(h.line * 0.6f, h.S(0.035f))),
         LIPS);
  }
  if (f.pottu > 0) // between the brows
    DrawCircleV(h.P(m + 0.03f * t, f.eyeY + f.eyeHeight * 1.2f + 0.03f),
                h.S(f.pottu), KUMKUM_RED);
}

// The face: skin with its cel shadow (a crescent down the right side and
// under the chin), then the eyes and the rest
static void DrawFace(const Head &h, const Face &f, float time) {
  const Shape face = FaceShape(h, f.jaw, f.cheeks);
  Inked(face, SKIN_SHADE, h.line);
  Shape lit = face;
  for (int i = 0; i < lit.n; i++) {
    const Vector2 d = Minus(face.p[i], h.centre);
    lit.p[i] = Plus(h.centre, {d.x * 0.92f - h.S(0.06f), d.y * 0.93f - h.S(0.04f)});
  }
  Fill(lit, SKIN_LIGHT);
  const bool blink = std::fmod(time + f.blink, 4.3f) < 0.13f;
  Eyes(h, f, blink);
  FaceDetails(h, f);
}

// Ears with an earring: a gold stud, or a jimikki (a little gold bell). The
// far ear hides once the face turns.
static void Ears(const Head &h, bool jimikki) {
  const float t = std::fabs(h.turn);
  for (int side = -1; side <= 1; side += 2) {
    const bool far = side * h.turn > 0;
    if (far && t > 0.3f)
      continue;
    const float u = side * (far ? 0.98f : 0.98f - 0.12f * t);
    const Vector2 e = h.P(u, -0.3f);
    InkEllipse(e, h.S(0.15f), h.S(0.22f), SKIN_LIGHT, h.line);
    DrawEllipseV({e.x - side * h.S(0.02f), e.y}, h.S(0.07f), h.S(0.12f),
                 SKIN_SHADE);
    const Vector2 lobe = h.P(u, -0.5f);
    if (jimikki) {
      const Vector2 bell = {lobe.x, lobe.y + h.S(0.2f)};
      DrawCircleSector(bell, h.S(0.13f) + h.line, 180, 360, 10, INK);
      DrawCircleSector(bell, h.S(0.13f), 180, 360, 10, ZARI);
      DrawCircleV({bell.x, bell.y + h.S(0.03f)}, h.S(0.035f), ZARI);
    }
    InkEllipse(lobe, h.S(0.065f), h.S(0.065f), ZARI, h.line * 0.6f);
  }
}

//--- Hair ---------------------------------------------------------------------
// A hairline parted at `part` (`partV` high), curving down to the temples at
// `templeV`: 11 head points from the left temple to the right
static void PartedHairline(float part, float partV, float templeV,
                           Vector2 *out) {
  for (int i = 0; i < 11; i++) {
    const float s = i / 5.0f - 1; // -1 left temple, 0 the parting, 1 right
    const float u = s < 0 ? part + (-1.02f - part) * -s
                          : part + (1.02f - part) * s;
    out[i] = {u, partV - (partV - templeV) * std::pow(std::fabs(s), 1.6f)};
  }
}

// The hair over the skull: from the hairline (11 head points, from the left
// temple to the right) up over the crown, `volume` radii out
static Strip HairCap(const Head &h, const Vector2 *hairline, float volume) {
  Strip s;
  s.n = 11;
  for (int i = 0; i < s.n; i++) {
    const float a = (196 - 212 * i / 10.0f) * DEG2RAD;
    s.a[i] = h.P(volume * std::cos(a), 0.03f + volume * std::sin(a));
    s.b[i] = h.P(hairline[i].x + h.Mid() * (1 - hairline[i].x * hairline[i].x),
                 hairline[i].y);
  }
  return s;
}

// A band round the skull between radii `r0` and `r1`, from angle `a0` to
// `a1` (degrees: 0 right, 90 up). For the hair's shadow.
static Strip Arc(const Head &h, float a0, float a1, float r0, float r1) {
  Strip s;
  s.n = 10;
  for (int i = 0; i < s.n; i++) {
    const float a = (a0 + (a1 - a0) * i / (s.n - 1.0f)) * DEG2RAD;
    s.a[i] = h.P(r0 * std::cos(a), 0.03f + r0 * std::sin(a));
    s.b[i] = h.P(r1 * std::cos(a), 0.03f + r1 * std::sin(a));
  }
  return s;
}

// Hair: its pieces are outlined together, then filled, so only its outside
// gets a line. Then its shadow (the side away from the light), a few strand
// lines, and the band of shine round the crown.
struct Hair {
  Strip pieces[12];
  int count = 0;
  void Add(const Strip &s) {
    if (count < 12)
      pieces[count++] = s;
  }
  void Draw(const Head &h, Color colour, float shadeFrom = 75) const {
    for (int i = 0; i < count; i++)
      Outline(pieces[i], h.line);
    for (int i = 0; i < count; i++)
      Fill(pieces[i], colour);
    Fill(Arc(h, shadeFrom, 8, 0.9f, 1.1f), HAIR_SHADE);
  }
};

// The band of shine round the crown (anime's "ring of light"): from angle
// `a0` to `a1` (degrees), between radii `r0` and `r1`, its lower edge cut
// into `spikes` points, as light catches the locks
static void Shine(const Head &h, float a0, float a1, int spikes, float r0,
                  float r1, Color c = HAIR_SHINE) {
  Strip s;
  s.n = 2 * spikes + 1;
  for (int i = 0; i < s.n; i++) {
    const float k = i / (s.n - 1.0f);
    const float a = (a0 + (a1 - a0) * k) * DEG2RAD;
    const float taper = std::sin(k * PI); // thin at both ends
    const float top = r0 - (r0 - r1) * 0.25f * (1 - taper);
    const float bottom = r0 - (r0 - r1) * taper * (i % 2 ? 1.0f : 0.45f);
    s.a[i] = h.P(top * std::cos(a), 0.03f + top * std::sin(a));
    s.b[i] = h.P(bottom * std::cos(a), 0.03f + bottom * std::sin(a));
  }
  Fill(s, c);
}

// A thin line along a lock, in the hair's shadow colour
static void Strand(const Head &h, Vector2 from, Vector2 bend, Vector2 to,
                   Color c = HAIR_SHADE) {
  Fill(HeadStrip(h, from, bend, to, 0.0f, 0.0f, std::fmax(0.05f, h.line / h.R)),
       c);
}

//--- Each person's hair ---------------------------------------------------------
// Amma: parted in the middle and drawn back into a bun, a string of jasmine
// round it, grey coming in at the temples. The bun goes behind the head.
static void AmmaBun(const Head &h) {
  const Vector2 c = h.P(-1.02f, -0.2f);
  const float r = h.S(0.5f);
  InkEllipse(c, r, r * 0.92f, HAIR_BLACK, h.line);
  DrawCircleSector(c, r * 0.92f, 300, 420, 12, HAIR_SHADE); // its far side
  for (int i = 0; i < 2; i++) // the coils
    DrawRing(c, r * (0.38f + i * 0.3f), r * (0.38f + i * 0.3f) + h.line * 0.8f,
             120 + i * 40, 290 + i * 30, 10, HAIR_SHADE);
  Fill(Along({c.x - r * 0.7f, c.y - r * 0.15f}, {c.x - r * 0.55f, c.y - r * 0.6f},
             {c.x - r * 0.1f, c.y - r * 0.78f}, 0, 0, r * 0.22f),
       HAIR_SHINE);
  for (int i = 0; i < 10; i++) { // jasmine buds, round the back of the bun
    const float a = (95 + i * 19) * DEG2RAD;
    const Vector2 d = {std::cos(a), -std::sin(a)};
    const Vector2 base = Plus(c, Scaled(d, r * 0.85f));
    const Vector2 tip = Plus(c, Scaled(d, r * 1.38f));
    Inked(Along(base, Scaled(Plus(base, tip), 0.5f), tip, 0, 0, r * 0.3f),
          JASMINE, h.line * 0.7f);
  }
}

static void AmmaHair(const Head &h, bool grey) {
  const float part = 0.05f;
  Vector2 line[11];
  PartedHairline(part, 0.68f, -0.32f, line);
  Hair hair;
  hair.Add(HairCap(h, line, 1.1f));
  // a loose lock by each temple
  hair.Add(Lock(h, {-0.78f, 0.42f}, {-1.04f, 0.0f}, {-0.93f, -0.5f}, 0.16f));
  hair.Add(Lock(h, {0.8f, 0.4f}, {1.04f, 0.0f}, {0.95f, -0.42f}, 0.14f));
  hair.Draw(h, HAIR_BLACK);
  // combed back from the parting
  Strand(h, {part - 0.06f, 0.95f}, {-0.5f, 0.85f}, {-0.86f, 0.25f});
  Strand(h, {part + 0.08f, 0.95f}, {0.55f, 0.85f}, {0.86f, 0.25f});
  if (grey) { // grey streaks from the parting and at the temples
    const Color g = HAIR_GREY;
    Fill(HeadStrip(h, {part - 0.1f, 0.92f}, {-0.6f, 0.8f}, {-0.95f, -0.1f}, 0.14f, 0.04f), g);
    Fill(HeadStrip(h, {part + 0.14f, 0.9f}, {0.66f, 0.76f}, {0.97f, -0.05f}, 0.12f, 0.03f), g);
    Fill(HeadStrip(h, {-0.84f, 0.2f}, {-1.02f, -0.1f}, {-0.94f, -0.42f}, 0.08f, 0), g);
    Fill(HeadStrip(h, {-0.3f, 1.02f}, {-0.75f, 0.85f}, {-1.0f, 0.3f}, 0.06f, 0.02f), g);
  }
  Shine(h, 148, 36, 5, 0.97f, 0.66f);
}

// Appa: short, combed to one side, a few locks over the forehead, grey at
// the temples
static void AppaHair(const Head &h, bool grey) {
  const Vector2 line[11] = {{-1.0f, -0.36f}, {-0.98f, 0.1f}, {-0.85f, 0.38f},
                            {-0.62f, 0.56f}, {-0.36f, 0.64f}, {-0.1f, 0.62f},
                            {0.2f, 0.58f},   {0.5f, 0.52f},   {0.75f, 0.38f},
                            {0.93f, 0.14f},  {1.0f, -0.1f}};
  Hair hair;
  hair.Add(HairCap(h, line, 1.1f));
  // the front, swept over to his left
  hair.Add(Lock(h, {-0.5f, 0.98f}, {-0.1f, 0.8f}, {0.22f, 0.4f}, 0.36f));
  hair.Add(Lock(h, {-0.05f, 1.02f}, {0.4f, 0.8f}, {0.6f, 0.36f}, 0.34f));
  hair.Add(Lock(h, {0.35f, 0.98f}, {0.8f, 0.75f}, {0.92f, 0.22f}, 0.3f));
  hair.Draw(h, HAIR_BLACK);
  Strand(h, {-0.3f, 0.95f}, {0.05f, 0.75f}, {0.18f, 0.48f});
  Strand(h, {0.15f, 0.98f}, {0.5f, 0.78f}, {0.6f, 0.45f});
  if (grey) { // grey at the temples
    Fill(HeadStrip(h, {-0.92f, 0.36f}, {-1.04f, 0.0f}, {-0.98f, -0.32f}, 0.2f, 0.12f), HAIR_GREY);
    Fill(HeadStrip(h, {0.86f, 0.32f}, {1.0f, 0.1f}, {0.98f, -0.08f}, 0.12f, 0.04f), HAIR_GREY);
  }
  Shine(h, 145, 50, 4, 0.98f, 0.72f);
}

// Little Anu: oiled and parted in the middle, a wisp or two over her forehead
// (the plaits are drawn with her body)
static void LittleAnuHair(const Head &h) {
  Vector2 line[11];
  PartedHairline(0.0f, 0.72f, -0.36f, line);
  Hair hair;
  hair.Add(HairCap(h, line, 1.1f));
  hair.Add(Lock(h, {-0.15f, 0.95f}, {-0.3f, 0.7f}, {-0.36f, 0.45f}, 0.26f));
  hair.Add(Lock(h, {0.1f, 0.95f}, {0.28f, 0.72f}, {0.3f, 0.5f}, 0.22f));
  hair.Draw(h, HAIR_BLACK);
  Shine(h, 150, 35, 5, 0.97f, 0.68f);
}

// Anu: a side parting and a long fringe swept across, locks down by her
// face, the rest in a loose ponytail (drawn behind her head)
static void AnuPonytail(const Body &b, const Head &h, float breathe) {
  const Strip tail = Along(h.P(-0.72f, 0.3f), b.At(-0.14f, 0.47f + breathe),
                           b.At(-0.105f, 0.36f), h.S(0.46f), h.S(0.08f));
  Inked(tail, HAIR_BLACK, h.line);
  Fill(Along(h.P(-0.86f, 0.12f), b.At(-0.142f, 0.45f + breathe),
             b.At(-0.115f, 0.38f), h.S(0.1f), 0),
       HAIR_SHINE);
  InkEllipse(h.P(-0.78f, 0.24f), h.S(0.16f), h.S(0.2f), {150, 40, 52, 255},
             h.line); // her hair tie
}

static void AnuHair(const Head &h) {
  Vector2 line[11];
  PartedHairline(-0.4f, 0.74f, -0.3f, line);
  Hair hair;
  hair.Add(HairCap(h, line, 1.13f));
  // the fringe, swept from the parting across her forehead
  hair.Add(Lock(h, {-0.42f, 1.0f}, {0.05f, 0.82f}, {0.3f, 0.3f}, 0.42f));
  hair.Add(Lock(h, {-0.05f, 1.05f}, {0.5f, 0.8f}, {0.68f, 0.22f}, 0.4f));
  hair.Add(Lock(h, {0.35f, 1.0f}, {0.85f, 0.7f}, {0.95f, 0.0f}, 0.34f));
  hair.Add(Lock(h, {-0.5f, 0.95f}, {-0.78f, 0.7f}, {-0.84f, 0.3f}, 0.26f));
  // locks down by her face
  hair.Add(Lock(h, {-0.82f, 0.5f}, {-1.05f, -0.1f}, {-0.86f, -0.72f}, 0.24f));
  hair.Add(Lock(h, {0.84f, 0.4f}, {1.06f, -0.1f}, {0.92f, -0.62f}, 0.2f));
  hair.Draw(h, HAIR_BLACK);
  Strand(h, {-0.2f, 0.95f}, {0.2f, 0.72f}, {0.36f, 0.42f});
  Strand(h, {0.15f, 0.98f}, {0.62f, 0.72f}, {0.72f, 0.35f});
  Shine(h, 150, 40, 5, 1.0f, 0.72f);
}

// A plait from `top` down to `end`, `width` wide, tied with a ribbon bow
static void Plait(Vector2 top, Vector2 end, float width, Color ribbon,
                  float line) {
  const int TWISTS = 5;
  const float length = std::sqrt(Dot(Minus(end, top), Minus(end, top)));
  for (int i = 0; i < TWISTS; i++) { // each twist a little to one side
    const float t = (i + 0.5f) / TWISTS;
    const Vector2 p = Plus(top, Scaled(Minus(end, top), t));
    const float w = width * (1 - 0.3f * t) / 2;
    const float side = (i % 2 ? 1.0f : -1.0f) * w * 0.18f;
    InkEllipse({p.x + side, p.y}, w, length / TWISTS * 0.66f, HAIR_BLACK, line);
    DrawEllipseV({p.x + side - w * 0.3f, p.y - length / TWISTS * 0.2f},
                 w * 0.28f, length / TWISTS * 0.22f, HAIR_SHINE);
  }
  // the bow: two loops, the knot, two tails
  const float s = width * 0.8f;
  for (int side = -1; side <= 1; side += 2)
    Inked(Along(end, {end.x + side * s * 0.8f, end.y - s * 0.7f},
                {end.x + side * s * 1.1f, end.y + s * 0.2f}, s * 0.2f, s * 0.5f,
                s * 0.4f),
          ribbon, line);
  for (int side = -1; side <= 1; side += 2)
    Inked(Along(end, {end.x + side * s * 0.3f, end.y + s * 0.7f},
                {end.x + side * s * 0.45f, end.y + s * 1.3f}, s * 0.25f,
                s * 0.35f),
          ribbon, line);
  InkEllipse(end, s * 0.28f, s * 0.28f, ribbon, line);
}

} // namespace anime

//------------------------------------------------------------------------------
// Amma: a cotton saree (green, with a maroon and gold border), the pallu over
// her left shoulder, greying hair in a bun with jasmine, a red pottu.
//------------------------------------------------------------------------------
void DrawAmma(float x, float feetY, float time, bool stirring) {
  using namespace anime;
  const Body b = {{x, feetY}, 300};
  const float breathe = 0.003f * std::sin(time * 1.6f);
  const float shoulders = 0.78f + breathe;
  const Color saree = {46, 122, 88, 255}, pleat = {30, 92, 66, 255};
  const Color border = {150, 34, 54, 255}, blouse = {130, 30, 50, 255};

  Shadow(b, 0.24f);
  // the saree: wide at the hem, pleats down the front, a maroon and gold
  // border along the hem
  const Vector2 hemL = {-0.205f, 0.012f}, hemMid = {0.005f, -0.006f},
                hemR = {0.215f, 0.012f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape skirt =
      Cloth(b, {{-0.125f, 0.5f}, {-0.148f, 0.36f}, hem(0), hem(0.25f),
                hem(0.5f), hem(0.75f), hem(1), {0.152f, 0.36f}, {0.125f, 0.5f}});
  Inked(skirt, saree, b.Line());
  for (int i = 0; i < 4; i++) // pleats
    Limb(b, {-0.02f + i * 0.02f, 0.45f}, {-0.05f + i * 0.035f, 0.02f}, 0.004f,
         pleat, false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0, 0.045f), border);
  Fill(HemBand(b, hemL, hemMid, hemR, 0.038f, 0.05f), ZARI);
  Wash(b, skirt, 0.05f, 0.06f);
  // the blouse, and a little bare waist
  const Shape top =
      Cloth(b, {{-0.12f, 0.5f}, {-0.132f, 0.7f}, {-0.122f, shoulders},
                {-0.07f, shoulders + 0.012f}, {0.07f, shoulders + 0.012f},
                {0.127f, shoulders}, {0.134f, 0.7f}, {0.12f, 0.5f}});
  Inked(top, blouse, b.Line());
  Wash(b, top, 0.06f, 0.05f);
  Ellipse(b, 0.02f, 0.53f, 0.1f, 0.025f, SKIN_SHADE, false);
  // the pallu: from the left shoulder down across to the right hip, its
  // gold edge and a few gold motifs
  const Shape pallu =
      Cloth(b, {{-0.14f, shoulders + 0.01f}, {-0.04f, shoulders + 0.02f},
                {0.14f, 0.5f}, {0.06f, 0.44f}});
  Inked(pallu, border, b.Line());
  Limb(b, {-0.042f, shoulders + 0.015f}, {0.135f, 0.5f}, 0.008f, ZARI, false);
  for (int i = 0; i < 3; i++)
    Circle(b, -0.06f + i * 0.055f, 0.7f - i * 0.09f, 0.006f, ZARI, false);
  Wash(b, pallu, -0.065f, 0.105f);
  // arms, in short blouse sleeves, with glass bangles on the left wrist
  if (stirring) { // the right arm forward, to the pot, moving gently
    const float stir = 0.015f * std::sin(time * 3);
    Arm(b, {0.13f, 0.74f}, {0.2f, 0.6f}, {0.32f + stir, 0.64f}, 0.044f,
        SKIN_SHADE, 0.45f, ColorLerp(blouse, BLACK, 0.2f));
    Arm(b, {-0.14f, 0.74f}, {-0.155f, 0.62f}, {-0.17f, 0.5f}, 0.044f,
        SKIN_LIGHT, 0.45f, blouse);
  } else { // hands together in front, holding the end of the pallu
    Arm(b, {0.13f, 0.74f}, {0.1f, 0.645f}, {0.07f, 0.55f}, 0.044f, SKIN_SHADE,
        0.45f, ColorLerp(blouse, BLACK, 0.2f));
    Arm(b, {-0.14f, 0.74f}, {-0.17f, 0.5f}, {-0.02f, 0.53f}, 0.042f,
        SKIN_LIGHT, 0.2f, blouse);
  }
  for (int i = 0; i < 3; i++) { // glass bangles, on her left wrist
    const Color bangle = i == 1 ? ZARI : Color{40, 120, 80, 255};
    if (stirring)
      Limb(b, {-0.178f, 0.54f - i * 0.009f}, {-0.16f, 0.54f - i * 0.009f},
           0.006f, bangle, false);
    else
      Limb(b, {-0.064f + i * 0.009f, 0.51f}, {-0.062f + i * 0.009f, 0.546f},
           0.006f, bangle, false);
  }
  // neck, then the head: the bun goes behind it
  Quad(b, {-0.021f, 0.77f}, {0.025f, 0.77f}, {0.022f, 0.85f}, {-0.018f, 0.85f},
       SKIN_SHADE);
  const Head h = {b.At(0, 0.9f + breathe), b.Size(0.068f),
                  stirring ? 0.6f : 0.1f, b.Line()};
  Face face; // kind, older eyes
  face.lashes = true;
  face.soft = true;
  face.glow = 0.35f;
  face.eyeWidth = 0.31f;
  face.eyeHeight = 0.3f;
  face.jaw = 1.06f;
  face.cheeks = 0.94f;
  face.blush = 0.6f;
  face.look = stirring ? Vector2{0.7f, 0.4f} : Vector2{-0.3f, 0.5f};
  face.pottu = 0.085f;
  AmmaBun(h);
  DrawFace(h, face, time);
  Ears(h, true);
  AmmaHair(h, true);
}

//------------------------------------------------------------------------------
// Anu, grown up: a mustard kurta, dark leggings, hair in a loose ponytail.
// Sitting back on her heels, the way you sit to draw a kolam.
//------------------------------------------------------------------------------
void DrawAnuSitting(float x, float groundY, float time) {
  using namespace anime;
  const Body b = {{x, groundY}, 260}; // as tall as she would be standing
  const float breathe = 0.003f * std::sin(time * 1.4f);
  const float shoulders = 0.42f + breathe;
  const Color kurta = {214, 160, 58, 255}, trim = {150, 40, 50, 255};
  const Color leggings = {50, 54, 80, 255};

  Shadow(b, 0.2f);
  // folded legs: knees forward, feet tucked under
  Ellipse(b, 0.02f, 0.06f, 0.16f, 0.06f, leggings);
  Ellipse(b, 0.07f, 0.045f, 0.09f, 0.028f, {38, 40, 62, 255}, false);
  Ellipse(b, -0.1f, 0.03f, 0.06f, 0.03f, SKIN_SHADE); // a heel
  // the kurta over the lap and body, a maroon trim at the neck and hem
  const Vector2 hemL = {-0.12f, 0.07f}, hemMid = {0.015f, 0.052f},
                hemR = {0.15f, 0.06f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape body =
      Cloth(b, {{-0.1f, shoulders}, {-0.125f, 0.25f}, hem(0), hem(0.33f),
                hem(0.67f), hem(1), {0.135f, 0.25f}, {0.1f, shoulders},
                {0.05f, shoulders + 0.012f}, {-0.05f, shoulders + 0.012f}});
  Inked(body, kurta, b.Line());
  Limb(b, {-0.03f, 0.4f}, {-0.03f, 0.3f}, 0.006f, trim, false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0.006f, 0.018f), trim);
  Wash(b, body, 0.04f, 0.04f);
  // arms resting on the knees, a little flour on her fingers
  Arm(b, {0.09f, 0.39f}, {0.12f, 0.27f}, {0.12f, 0.16f}, 0.04f, SKIN_SHADE,
      0.95f, ColorLerp(kurta, BLACK, 0.2f));
  Circle(b, 0.12f, 0.135f, 0.01f, FLOUR, false);
  Arm(b, {-0.09f, 0.39f}, {-0.05f, 0.18f}, {0.08f, 0.13f}, 0.037f,
      SKIN_LIGHT, 0.85f, kurta);
  // neck, ponytail, head
  Quad(b, {-0.018f, 0.4f}, {0.024f, 0.4f}, {0.02f, 0.47f}, {-0.016f, 0.47f},
       SKIN_SHADE);
  const Head h = {b.At(0.005f, 0.51f + breathe), b.Size(0.07f), 0.3f,
                  b.Line()};
  Face face;
  face.lashes = true;
  face.eyeWidth = 0.36f;
  face.eyeHeight = 0.42f;
  face.cheeks = 0.92f;
  face.look = {0.6f, 0.5f}; // down at her kolam
  face.pottu = 0.05f;
  face.blink = 1.7f;
  AnuPonytail(b, h, breathe);
  DrawFace(h, face, time);
  Ears(h, false);
  AnuHair(h);
}

// An arm from `shoulder` to `hand` (body units), the elbow bent a little
// downwards between them
static void ReachArm(const Body &b, Vector2 shoulder, Vector2 hand,
                     float thickness, Color skin, float sleeve,
                     Color sleeveColor) {
  const Vector2 elbow = {(shoulder.x + hand.x) / 2 + 0.01f,
                         (shoulder.y + hand.y) / 2 - 0.05f};
  anime::Arm(b, shoulder, elbow, hand, thickness, skin, sleeve, sleeveColor);
}

//------------------------------------------------------------------------------
// Anu, grown up, standing: the mustard kurta to her knees over dark
// leggings, hair in a loose ponytail. `hand`: where her right hand is, in
// fractions of her height from her feet (e.g. {0.25, 0.75} reaching out and
// up). `look`: where she looks, -1..1 each way.
//------------------------------------------------------------------------------
void DrawAnu(float x, float feetY, float time, Vector2 hand, Vector2 look) {
  using namespace anime;
  const Body b = {{x, feetY}, 285};
  const float breathe = 0.003f * std::sin(time * 1.4f);
  const float shoulders = 0.79f + breathe;
  const Color kurta = {214, 160, 58, 255}, trim = {150, 40, 50, 255};
  const Color leggings = {50, 54, 80, 255};

  Shadow(b, 0.2f);
  // legs and flat sandals
  Limb(b, {-0.045f, 0.03f}, {-0.05f, 0.42f}, 0.05f, leggings);
  Limb(b, {0.045f, 0.03f}, {0.05f, 0.42f}, 0.05f, leggings);
  Ellipse(b, -0.055f, 0.012f, 0.045f, 0.016f, {120, 70, 40, 255});
  Ellipse(b, 0.055f, 0.012f, 0.045f, 0.016f, {120, 70, 40, 255});
  // the kurta: down to the knees, slit at the sides, a maroon trim
  const Vector2 hemL = {-0.135f, 0.35f}, hemMid = {0.0f, 0.335f},
                hemR = {0.14f, 0.35f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape body =
      Cloth(b, {{-0.11f, shoulders}, {-0.12f, 0.6f}, hem(0), hem(0.33f),
                hem(0.67f), hem(1), {0.125f, 0.6f}, {0.11f, shoulders},
                {0.05f, shoulders + 0.012f}, {-0.05f, shoulders + 0.012f}});
  Inked(body, kurta, b.Line());
  Limb(b, {-0.02f, shoulders - 0.01f}, {-0.02f, shoulders - 0.1f}, 0.006f, trim,
       false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0.006f, 0.02f), trim);
  Wash(b, body, 0.04f, 0.05f);
  // arms: the left one easy at her side, the right one wherever `hand` is
  ReachArm(b, {0.1f, shoulders - 0.04f}, hand, 0.04f, SKIN_SHADE, 0.5f,
           ColorLerp(kurta, BLACK, 0.2f));
  Arm(b, {-0.1f, shoulders - 0.04f}, {-0.135f, 0.62f}, {-0.13f, 0.47f}, 0.038f,
      SKIN_LIGHT, 0.5f, kurta);
  // neck, ponytail, head
  Quad(b, {-0.018f, shoulders - 0.01f}, {0.022f, shoulders - 0.01f},
       {0.02f, shoulders + 0.06f}, {-0.016f, shoulders + 0.06f}, SKIN_SHADE);
  const Head h = {b.At(0.004f, shoulders + 0.1f), b.Size(0.066f),
                  0.25f + 0.3f * look.x, b.Line()};
  Face face;
  face.lashes = true;
  face.eyeWidth = 0.36f;
  face.eyeHeight = 0.42f;
  face.cheeks = 0.92f;
  face.look = look;
  face.pottu = 0.05f;
  face.blink = 1.7f;
  // the ponytail, behind her head, down to her shoulder blades
  const Strip tail = Along(h.P(-0.72f, 0.3f), b.At(-0.15f, 0.82f + breathe),
                           b.At(-0.115f, 0.68f + breathe), h.S(0.46f),
                           h.S(0.1f));
  Inked(tail, HAIR_BLACK, h.line);
  Fill(Along(h.P(-0.86f, 0.12f), b.At(-0.152f, 0.8f + breathe),
             b.At(-0.125f, 0.71f + breathe), h.S(0.1f), 0),
       HAIR_SHINE);
  InkEllipse(h.P(-0.78f, 0.24f), h.S(0.16f), h.S(0.2f), {150, 40, 52, 255},
             h.line); // her hair tie
  DrawFace(h, face, time);
  Ears(h, false);
  AnuHair(h);
}

//------------------------------------------------------------------------------
// Meena, Anu's childhood friend: an orange cotton saree with a teal border,
// a long plait with jasmine. `hand` and `look` as for DrawAnu.
//------------------------------------------------------------------------------
void DrawMeena(float x, float feetY, float time, Vector2 hand, Vector2 look) {
  using namespace anime;
  const Body b = {{x, feetY}, 278};
  const float breathe = 0.003f * std::sin(time * 1.5f + 1);
  const float shoulders = 0.78f + breathe;
  const Color saree = {236, 128, 48, 255}, pleat = {204, 100, 34, 255};
  const Color border = {24, 128, 130, 255}, blouse = {24, 110, 116, 255};

  // her plait, behind her, down to her waist
  const Head h = {b.At(0, 0.9f + breathe), b.Size(0.068f),
                  0.1f + 0.3f * look.x, b.Line()};
  Plait(h.P(-0.2f, -0.4f), b.At(-0.06f, 0.5f), h.S(0.42f), border, h.line);
  Shadow(b, 0.22f);
  const Vector2 hemL = {-0.19f, 0.012f}, hemMid = {0.005f, -0.006f},
                hemR = {0.2f, 0.012f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape skirt =
      Cloth(b, {{-0.12f, 0.5f}, {-0.14f, 0.36f}, hem(0), hem(0.25f), hem(0.5f),
                hem(0.75f), hem(1), {0.145f, 0.36f}, {0.12f, 0.5f}});
  Inked(skirt, saree, b.Line());
  for (int i = 0; i < 4; i++) // pleats
    Limb(b, {-0.02f + i * 0.02f, 0.45f}, {-0.05f + i * 0.035f, 0.02f}, 0.004f,
         pleat, false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0, 0.04f), border);
  Wash(b, skirt, 0.05f, 0.06f);
  const Shape top =
      Cloth(b, {{-0.115f, 0.5f}, {-0.125f, 0.7f}, {-0.117f, shoulders},
                {-0.065f, shoulders + 0.012f}, {0.065f, shoulders + 0.012f},
                {0.122f, shoulders}, {0.128f, 0.7f}, {0.115f, 0.5f}});
  Inked(top, blouse, b.Line());
  Wash(b, top, 0.06f, 0.05f);
  const Shape pallu =
      Cloth(b, {{-0.135f, shoulders + 0.01f}, {-0.04f, shoulders + 0.02f},
                {0.135f, 0.5f}, {0.06f, 0.44f}});
  Inked(pallu, saree, b.Line());
  Limb(b, {-0.042f, shoulders + 0.015f}, {0.13f, 0.5f}, 0.008f, border, false);
  Wash(b, pallu, -0.06f, 0.1f);
  ReachArm(b, {0.125f, 0.74f}, hand, 0.042f, SKIN_SHADE, 0.4f,
           ColorLerp(blouse, BLACK, 0.2f));
  Arm(b, {-0.13f, 0.74f}, {-0.16f, 0.6f}, {-0.15f, 0.47f}, 0.04f, SKIN_LIGHT,
      0.4f, blouse);
  Quad(b, {-0.02f, 0.77f}, {0.024f, 0.77f}, {0.021f, 0.85f}, {-0.017f, 0.85f},
       SKIN_SHADE);
  Face face; // bright, quick to laugh
  face.lashes = true;
  face.eyeWidth = 0.35f;
  face.eyeHeight = 0.4f;
  face.cheeks = 0.95f;
  face.blush = 0.8f;
  face.look = look;
  face.pottu = 0.07f;
  DrawFace(h, face, time);
  Ears(h, true);
  LittleAnuHair(h);
}

//------------------------------------------------------------------------------
// Little Anu, eight: a pink pattu pavadai with a gold border, two plaits with
// red ribbons. Holding the kite reel in both hands.
//------------------------------------------------------------------------------
void DrawLittleAnu(float x, float feetY, float time, float height) {
  using namespace anime;
  const Body b = {{x, feetY}, height};
  const float bounce = 0.01f * std::fabs(std::sin(time * 4)); // excited
  const Color pavadai = {226, 84, 132, 255}, fold = {196, 62, 112, 255};
  const Color blouse = {120, 60, 160, 255};

  Shadow(b, 0.28f);
  Limb(b, {-0.05f, 0.02f}, {-0.05f, 0.1f}, 0.05f, SKIN_SHADE); // feet
  Limb(b, {0.05f, 0.02f}, {0.05f, 0.1f}, 0.05f, SKIN_SHADE);
  // the pavadai, flaring out, a gold border with a green line in it
  const Vector2 hemL = {-0.22f, 0.065f}, hemMid = {0.0f, 0.045f},
                hemR = {0.22f, 0.065f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape skirt =
      Cloth(b, {{-0.1f, 0.52f + bounce}, {-0.152f, 0.3f}, hem(0), hem(0.25f),
                hem(0.5f), hem(0.75f), hem(1), {0.152f, 0.3f},
                {0.1f, 0.52f + bounce}});
  Inked(skirt, pavadai, b.Line());
  for (int i = -1; i <= 1; i++) // folds
    Limb(b, {i * 0.04f, 0.45f}, {i * 0.09f, 0.14f}, 0.008f, fold, false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0, 0.075f), ZARI);
  Fill(HemBand(b, hemL, hemMid, hemR, 0.03f, 0.045f), {40, 130, 90, 255});
  Wash(b, skirt, 0.05f, 0.06f);
  // the blouse
  const Shape top =
      Cloth(b, {{-0.11f, 0.5f + bounce}, {-0.105f, 0.69f + bounce},
                {-0.07f, 0.725f + bounce}, {0.07f, 0.725f + bounce},
                {0.105f, 0.69f + bounce}, {0.11f, 0.5f + bounce}});
  Inked(top, blouse, b.Line());
  Wash(b, top, 0.04f, 0.04f);
  // both hands out in front, holding the reel, in puffed sleeves
  Arm(b, {0.1f, 0.68f + bounce}, {0.13f, 0.62f}, {0.16f, 0.56f}, 0.055f,
      SKIN_SHADE, 0.5f, ColorLerp(blouse, BLACK, 0.2f));
  Arm(b, {-0.1f, 0.68f + bounce}, {0.0f, 0.6f}, {0.1f, 0.56f}, 0.055f,
      SKIN_LIGHT, 0.4f, blouse);
  // her plaits, in front of her shoulders
  for (int side = -1; side <= 1; side += 2)
    Plait(b.At(side * 0.1f, 0.84f + bounce), b.At(side * 0.13f, 0.66f + bounce),
          b.Size(0.06f), KUMKUM_RED, b.Line());
  // a big head, as children have
  const Head h = {b.At(0, 0.855f + bounce), b.Size(0.122f), 0.4f, b.Line()};
  Face face;
  face.eyeWidth = 0.42f;
  face.eyeHeight = 0.5f;
  face.eyeSpread = 0.46f;
  face.eyeY = -0.26f;
  face.jaw = 0.86f;
  face.look = {0.6f, -0.8f}; // up at the kite
  face.openSmile = true;
  face.pottu = 0.06f;
  face.blink = 2.6f;
  DrawFace(h, face, time);
  Ears(h, true);
  LittleAnuHair(h);
}

//------------------------------------------------------------------------------
// Appa: a white veshti with a thin gold border, a light blue half-sleeve
// shirt, a moustache, glasses, greying hair. Watching the kite.
//------------------------------------------------------------------------------
void DrawAppa(float x, float feetY, float time) {
  using namespace anime;
  const Body b = {{x, feetY}, 150};
  const float sway = 0.004f * std::sin(time * 1.2f);
  const float shoulders = 0.8f + sway;
  const Color veshti = {246, 242, 230, 255};
  const Color shirt = {150, 186, 220, 255}, shirtShade = {118, 152, 190, 255};

  Shadow(b, 0.22f);
  Limb(b, {-0.05f, 0.01f}, {-0.05f, 0.06f}, 0.05f, SKIN_SHADE); // feet
  Limb(b, {0.06f, 0.01f}, {0.06f, 0.06f}, 0.05f, SKIN_SHADE);
  // the veshti, its gold border down the fold and along the hem
  const Vector2 hemL = {-0.13f, 0.04f}, hemMid = {0.005f, 0.03f},
                hemR = {0.14f, 0.04f};
  auto hem = [&](float t) { return HemPoint(hemL, hemMid, hemR, t); };
  const Shape cloth = Cloth(b, {{-0.12f, 0.5f}, hem(0), hem(0.33f),
                                hem(0.67f), hem(1), {0.13f, 0.5f}});
  Inked(cloth, veshti, b.Line());
  Limb(b, {0.0f, 0.5f}, {0.02f, 0.05f}, 0.006f, ZARI, false);
  Fill(HemBand(b, hemL, hemMid, hemR, 0.012f, 0.022f), ZARI);
  Wash(b, cloth, 0.04f, 0.04f);
  // the shirt, a little belly
  const Shape top =
      Cloth(b, {{-0.15f, 0.48f}, {-0.155f, 0.58f}, {-0.148f, 0.7f},
                {-0.14f, shoulders}, {-0.08f, shoulders + 0.018f},
                {0.08f, shoulders + 0.018f}, {0.15f, shoulders},
                {0.162f, 0.7f}, {0.174f, 0.57f}, {0.16f, 0.48f},
                {0.005f, 0.47f}});
  Inked(top, shirt, b.Line());
  Limb(b, {0.0f, 0.49f}, {0.0f, 0.79f}, 0.004f, shirtShade, false); // placket
  for (int i = 0; i < 4; i++) // buttons
    Circle(b, 0.0f, 0.55f + i * 0.06f, 0.006f, {240, 240, 240, 255}, false);
  Quad(b, {-0.11f, 0.66f}, {-0.05f, 0.66f}, {-0.05f, 0.72f}, {-0.11f, 0.72f},
       shirt); // the pocket
  Wash(b, top, 0.05f, 0.05f);
  // one arm down, one pointing up at the kite
  Arm(b, {-0.15f, 0.76f}, {-0.16f, 0.64f}, {-0.17f, 0.52f}, 0.05f,
      SKIN_LIGHT, 0.5f, shirt);
  Arm(b, {0.15f, 0.76f}, {0.24f, 0.86f}, {0.3f, 1.0f}, 0.048f, SKIN_SHADE,
      0.6f, shirtShade);
  Quad(b, {-0.025f, 0.79f}, {0.03f, 0.79f}, {0.025f, 0.85f}, {-0.02f, 0.85f},
       SKIN_SHADE);
  // the collar
  Quad(b, {-0.06f, shoulders}, {0.0f, shoulders - 0.03f}, {0.0f, shoulders},
       {-0.04f, shoulders + 0.02f}, shirt);
  Quad(b, {0.0f, shoulders - 0.03f}, {0.065f, shoulders},
       {0.045f, shoulders + 0.02f}, {0.0f, shoulders}, shirtShade);
  const Head h = {b.At(0.005f, 0.9f + sway), b.Size(0.074f), 0.5f, b.Line()};
  Face face;
  face.eyeWidth = 0.34f;
  face.eyeHeight = 0.36f;
  face.cheeks = 1.05f;
  face.blush = 0.5f;
  face.look = {0.6f, -0.7f}; // up at the kite
  face.openSmile = true;
  face.blink = 0.9f;
  DrawFace(h, face, time);
  Ears(h, false);
  AppaHair(h, true);
  // moustache and glasses
  const float m = h.Mid();
  Inked(Along(h.P(m - 0.34f, -0.68f), h.P(m + 0.04f, -0.46f),
              h.P(m + 0.4f, -0.66f), h.S(0.08f), h.S(0.06f), h.S(0.14f)),
        HAIR_BLACK, h.line * 0.8f);
  const Color frame = {60, 50, 40, 255};
  Vector2 bridge[2];
  for (int side = -1; side <= 1; side += 2) {
    const Vector2 e = EyeCentre(h, face, side);
    const float w = h.S(0.56f) * h.Far(side), eh = h.S(0.46f);
    const Rectangle lens = {e.x - w / 2, e.y - eh / 2 - h.S(0.02f), w, eh};
    DrawRectangleRounded(lens, 0.6f, 6, {200, 230, 255, 40});
    DrawRectangleRoundedLinesEx(lens, 0.6f, 6, h.line * 0.8f, frame);
    DrawLineEx({e.x - w * 0.2f, e.y + eh * 0.25f},
               {e.x + w * 0.05f, e.y - eh * 0.3f}, h.line * 0.8f,
               {255, 255, 255, 120}); // a glint
    bridge[(side + 1) / 2] = {e.x - side * w / 2, e.y - eh * 0.1f};
  }
  DrawLineEx(bridge[0], bridge[1], h.line * 0.8f, frame);
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

//------------------------------------------------------------------------------
// Comic-book printing: the home is drawn like an old Tamil children's comic
// or magazine. Bold brush ink, heavier on the side away from the light (the
// lower right); flat printed colours; halftone dots and hatching for shade;
// and newsprint paper. These are new helpers, so the scenery above still looks
// the same everywhere else.
//------------------------------------------------------------------------------
#include "rlgl.h" // for how big one screen pixel is (the camera's zoom)

// Fills are printed a little off from the black plate, as cheap printing was
const Vector2 MISREGISTER = {-1.2f, -0.9f};

static Vector2 Add(Vector2 a, Vector2 b) { return {a.x + b.x, a.y + b.y}; }
static Vector2 Scale(Vector2 a, float s) { return {a.x * s, a.y * s}; }
static Vector2 Bezier(Vector2 a, Vector2 control, Vector2 b, float t) {
  const float u = 1 - t;
  return {u * u * a.x + 2 * u * t * control.x + t * t * b.x,
          u * u * a.y + 2 * u * t * control.y + t * t * b.y};
}

// A triangle with its points in any order (raylib only draws one way round)
void FillTriangle(Vector2 a, Vector2 b, Vector2 c, Color color) {
  if ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0)
    DrawTriangle(a, c, b, color);
  else
    DrawTriangle(a, b, c, color);
}

// A band along `points`, `widths[i]` wide at each one: brush strokes, palm
// fronds, leaves
static void Ribbon(const Vector2 *points, const float *widths, int count,
                   Color color) {
  Vector2 prevA = {}, prevB = {};
  for (int i = 0; i < count; i++) {
    const Vector2 from = points[i > 0 ? i - 1 : 0];
    const Vector2 to = points[i < count - 1 ? i + 1 : count - 1];
    const float dx = to.x - from.x, dy = to.y - from.y;
    const float len = std::fmax(0.001f, std::sqrt(dx * dx + dy * dy));
    const Vector2 side = {-dy / len * widths[i] / 2, dx / len * widths[i] / 2};
    const Vector2 a = Add(points[i], side), b = Add(points[i], Scale(side, -1));
    if (i > 0) {
      FillTriangle(prevA, prevB, a, color);
      FillTriangle(prevB, b, a, color);
    }
    prevA = a;
    prevB = b;
  }
}

// A small disc with only as many sides as it needs
static void Disc(Vector2 c, float r, Color color) {
  DrawCircleSector(c, r, 0, 360, (int)std::fmax(6, std::fmin(36, r * 1.5f)),
                   color);
}

//------------------------------------------------------------------------------
// Halftone: a shader that prints shapes as a screen of dots (or lines). The
// dots sit on one grid, turned 45 degrees and fixed to the stage, so shapes
// next to each other line up like one printed page. The shape's alpha is how
// much ink: small dots for a little, big merging dots for a lot.
//------------------------------------------------------------------------------
#define TONE_SHADER_BODY                                                       \
  "uniform float style;\n" /* 0 dots, 1 lines, 2 grain */                      \
  "uniform float cell;\n"  /* dot spacing, in stage units */                   \
  "uniform float pixel;\n" /* one screen pixel, in stage units */              \
  "float Ink(vec2 p, float amount) {\n"                                        \
  "  if (amount < 0.01) return 0.0;\n"                                         \
  "  vec2 q = vec2(p.x + p.y, p.y - p.x) * 0.70710678 / cell;\n"               \
  "  float soft = 0.7 * pixel / cell;\n"                                       \
  "  if (style > 1.5) {\n"                                                     \
  "    vec2 g = floor(p / cell);\n"                                            \
  "    float h = fract(sin(dot(g, vec2(12.9898, 78.233))) * 43758.5453);\n"    \
  "    return step(1.0 - amount, h);\n"                                        \
  "  }\n"                                                                      \
  "  if (style > 0.5) {\n"                                                     \
  "    float d = abs(fract(q.y) - 0.5), w = amount * 0.5;\n"                   \
  "    return 1.0 - smoothstep(w - soft, w + soft, d);\n"                      \
  "  }\n"                                                                      \
  "  float d = length(fract(q) - 0.5);\n"                                      \
  "  float r = amount < 0.785 ? sqrt(amount / 3.14159)\n"                      \
  "                           : 0.5 + (amount - 0.785) * 0.97;\n"              \
  "  return 1.0 - smoothstep(r - soft, r + soft, d);\n"                        \
  "}\n"
#if defined(PLATFORM_PHONE) // OpenGL ES 2: GLSL ES 1.00
static const char *TONE_VERTEX =
    "#version 100\n"
    "attribute vec3 vertexPosition;\nattribute vec2 vertexTexCoord;\n"
    "attribute vec4 vertexColor;\nuniform mat4 mvp;\n"
    "varying vec2 fragTexCoord;\nvarying vec4 fragColor;\n"
    "varying vec2 stagePos;\n"
    "void main() {\n  fragTexCoord = vertexTexCoord;\n"
    "  fragColor = vertexColor;\n  stagePos = vertexPosition.xy;\n"
    "  gl_Position = mvp * vec4(vertexPosition, 1.0);\n}\n";
static const char *TONE_FRAGMENT =
    "#version 100\n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n"
    "#else\nprecision mediump float;\n#endif\n"
    "varying vec2 fragTexCoord;\nvarying vec4 fragColor;\n"
    "varying vec2 stagePos;\n" TONE_SHADER_BODY
    "void main() {\n"
    "  gl_FragColor = vec4(fragColor.rgb, Ink(stagePos, fragColor.a));\n}\n";
#else
static const char *TONE_VERTEX =
    "#version 330\n"
    "in vec3 vertexPosition;\nin vec2 vertexTexCoord;\nin vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "out vec2 fragTexCoord;\nout vec4 fragColor;\nout vec2 stagePos;\n"
    "void main() {\n  fragTexCoord = vertexTexCoord;\n"
    "  fragColor = vertexColor;\n  stagePos = vertexPosition.xy;\n"
    "  gl_Position = mvp * vec4(vertexPosition, 1.0);\n}\n";
static const char *TONE_FRAGMENT =
    "#version 330\n"
    "in vec2 fragTexCoord;\nin vec4 fragColor;\nin vec2 stagePos;\n"
    "out vec4 finalColor;\n" TONE_SHADER_BODY
    "void main() {\n"
    "  finalColor = vec4(fragColor.rgb, Ink(stagePos, fragColor.a));\n}\n";
#endif

// Loaded the first time it's needed, and kept until the game closes
static Shader toneShader = {};
static bool toneReady = false;
static int toneLocations[3] = {}; // style, cell, pixel

void BeginTone(ToneStyle style, float cell) {
  if (toneShader.id == 0) {
    toneShader = LoadShaderFromMemory(TONE_VERTEX, TONE_FRAGMENT);
    // if it didn't compile, raylib hands back its plain shader: shapes then
    // just come out see-through, which still reads as shading
    toneReady = toneShader.id != rlGetShaderIdDefault();
    const char *names[3] = {"style", "cell", "pixel"};
    for (int i = 0; i < 3; i++)
      toneLocations[i] = GetShaderLocation(toneShader, names[i]);
  }
  if (!toneReady)
    return;
  const Matrix view = rlGetMatrixModelview(); // the camera's zoom
  const float zoom = std::sqrt(view.m0 * view.m0 + view.m1 * view.m1);
  const float values[3] = {(float)style, cell, 1 / std::fmax(0.1f, zoom)};
  BeginShaderMode(toneShader);
  for (int i = 0; i < 3; i++)
    SetShaderValue(toneShader, toneLocations[i], &values[i],
                   SHADER_UNIFORM_FLOAT);
}

void EndTone() {
  if (toneReady)
    EndShaderMode();
}

void DrawToneRect(Rectangle r, Color ink, float top, float bottom,
                  ToneStyle style, float cell) {
  BeginTone(style, cell);
  DrawRectangleGradientV((int)std::floor(r.x), (int)std::floor(r.y),
                         (int)std::ceil(r.width) + 1,
                         (int)std::ceil(r.height) + 1, Faded(ink, top),
                         Faded(ink, bottom));
  EndTone();
}

// Newsprint over the whole picture: a fine grain of specks. (The paper's warm
// white is in the colours themselves: use COMIC_PAPER rather than WHITE.)
void DrawNewsprint() {
  BeginTone(TONE_GRAIN, 1.6f);
  DrawRectangleRec(FullScreen(), {120, 92, 66, 8});
  EndTone();
}

//------------------------------------------------------------------------------
// Brush ink
//------------------------------------------------------------------------------
void InkCurve(Vector2 a, Vector2 control, Vector2 b, float weight,
              Color ink) {
  // as few pieces as look smooth: software rendering pays for each one
  const float reach = std::fabs(control.x - a.x) + std::fabs(control.y - a.y) +
                      std::fabs(b.x - control.x) + std::fabs(b.y - control.y);
  const int N = (int)std::fmax(3, std::fmin(10, reach / 8));
  Vector2 points[11];
  float widths[11];
  for (int i = 0; i <= N; i++) {
    const float t = (float)i / N;
    points[i] = Bezier(a, control, b, t);
    widths[i] = weight * (0.3f + 0.7f * std::sin(PI * t)); // the brush lifts
  }
  Ribbon(points, widths, N + 1, ink);
}

void InkLine(Vector2 a, Vector2 b, float weight, Color ink) {
  InkCurve(a, Scale(Add(a, b), 0.5f), b, weight, ink);
}

void InkRect(Rectangle r, float weight, Color ink) {
  const float thin = weight * 0.7f, thick = weight * 1.3f;
  const float x0 = r.x - thin / 2, y0 = r.y - thin / 2;
  const float w = r.width + thin / 2 + thick / 2,
              h = r.height + thin / 2 + thick / 2;
  DrawRectangleRec({x0, y0, w, thin}, ink);                          // top
  DrawRectangleRec({x0, y0, thin, h}, ink);                          // left
  DrawRectangleRec({x0, r.y + r.height - thick / 2, w, thick}, ink); // bottom
  DrawRectangleRec({r.x + r.width - thick / 2, y0, thick, h}, ink);  // right
}

// How many sides a round shape needs to look round
static int RoundSides(float rx, float ry) {
  return (int)std::fmax(10, std::fmin(64, (rx + ry) * 0.6f));
}

void InkEllipse(Vector2 c, float rx, float ry, float weight, Color ink) {
  const int n = RoundSides(rx, ry);
  Vector2 prevIn = {}, prevOut = {};
  for (int i = 0; i <= n; i++) {
    const float a = 2 * PI * i / n;
    const float ca = std::cos(a), sa = std::sin(a);
    // heaviest at the lower right, lightest at the upper left
    const float t = weight * (1 + 0.45f * std::cos(a - PI / 4)) / 2;
    const Vector2 in = {c.x + ca * (rx - t), c.y + sa * (ry - t)};
    const Vector2 out = {c.x + ca * (rx + t), c.y + sa * (ry + t)};
    if (i > 0) {
      FillTriangle(prevIn, prevOut, out, ink);
      FillTriangle(prevIn, out, in, ink);
    }
    prevIn = in;
    prevOut = out;
  }
}

void InkCircle(Vector2 c, float r, float weight, Color ink) {
  InkEllipse(c, r, r, weight, ink);
}

void InkPoly(const Vector2 *points, int count, float weight, Color ink) {
  float area = 0; // its sign says which way round the points go
  for (int i = 0; i < count; i++) {
    const Vector2 p = points[i], q = points[(i + 1) % count];
    area += p.x * q.y - q.x * p.y;
  }
  for (int i = 0; i < count; i++) {
    const Vector2 p = points[i], q = points[(i + 1) % count];
    const float dx = q.x - p.x, dy = q.y - p.y;
    const float len = std::fmax(0.001f, std::sqrt(dx * dx + dy * dy));
    const float nx = (area > 0 ? dy : -dy) / len;
    const float ny = (area > 0 ? -dx : dx) / len;
    // edges facing the lower right are heavier
    const float t = weight * (1 + 0.4f * (nx + ny) * 0.7071f);
    DrawLineEx(p, q, t, ink);
    Disc(p, t / 2, ink);
  }
}

void ComicRect(Rectangle r, Color fill, float weight) {
  DrawRectangleRec(
      {r.x + MISREGISTER.x, r.y + MISREGISTER.y, r.width, r.height}, fill);
  InkRect(r, weight);
}

void ComicEllipse(Vector2 c, float rx, float ry, Color fill, float weight) {
  const Vector2 m = Add(c, MISREGISTER);
  const int n = RoundSides(rx, ry);
  for (int i = 0; i < n; i++) { // a fan, as few sides as look round
    const float a = 2 * PI * i / n, b = 2 * PI * (i + 1) / n;
    FillTriangle(m, {m.x + rx * std::cos(a), m.y + ry * std::sin(a)},
                 {m.x + rx * std::cos(b), m.y + ry * std::sin(b)}, fill);
  }
  InkEllipse(c, rx, ry, weight);
}

void ComicCircle(Vector2 c, float r, Color fill, float weight) {
  ComicEllipse(c, r, r, fill, weight);
}

void ComicPoly(const Vector2 *points, int count, Color fill, float weight) {
  for (int i = 1; i + 1 < count; i++) // a fan: the shape must be convex
    FillTriangle(Add(points[0], MISREGISTER), Add(points[i], MISREGISTER),
        Add(points[i + 1], MISREGISTER), fill);
  InkPoly(points, count, weight);
}

// Short brush strokes slanting up to the right, across `area`: shadow
void InkHatch(Rectangle area, float gap, float weight, Color ink) {
  for (float k = gap / 2; k < area.width + area.height; k += gap) {
    // the line x + y = k (from the area's corner), cut to the area
    const float x0 = std::fmax(0, k - area.height);
    const float x1 = std::fmin(area.width, k);
    if (x1 - x0 < 2)
      continue;
    InkLine({area.x + x0, area.y + k - x0}, {area.x + x1, area.y + k - x1},
            weight, ink);
  }
}

// A four-pointed glint: on brass, glass and the stars
void DrawSparkle(Vector2 c, float size, Color color) {
  const float w = size * 0.22f;
  FillTriangle({c.x, c.y - size}, {c.x - w, c.y}, {c.x + w, c.y}, color);
  FillTriangle({c.x, c.y + size}, {c.x - w, c.y}, {c.x + w, c.y}, color);
  FillTriangle({c.x - size, c.y}, {c.x, c.y - w}, {c.x, c.y + w}, color);
  FillTriangle({c.x + size, c.y}, {c.x, c.y - w}, {c.x, c.y + w}, color);
}

// A leaf from its stem to its tip, inked, with a vein down the middle
void DrawComicLeaf(Vector2 stem, Vector2 tip, float width, Color fill,
                   float weight) {
  const float SHAPE[5] = {0.15f, 0.8f, 1.0f, 0.7f, 0.05f};
  Vector2 points[5];
  float widths[5], inked[5];
  for (int i = 0; i < 5; i++) {
    points[i] = Add(stem, Scale({tip.x - stem.x, tip.y - stem.y}, i / 4.0f));
    widths[i] = width * SHAPE[i];
    inked[i] = widths[i] + 2 * weight;
  }
  Ribbon(points, inked, 5, COMIC_INK);
  Ribbon(points, widths, 5, fill);
  DrawLineEx(stem, points[3], weight * 0.5f, COMIC_INK);
}

//------------------------------------------------------------------------------
// Comic scenery: the same pieces as the scenery above, inked and printed
//------------------------------------------------------------------------------
// A shadow on the ground, printed in dots
void DrawComicShadow(float x, float y, float width) {
  BeginTone(TONE_DOTS, 4);
  DrawEllipse((int)x, (int)y, width / 2, width / 11, Faded(COMIC_INK, 0.5f));
  DrawEllipse((int)x, (int)y, width / 2.8f, width / 18,
              Faded(COMIC_INK, 0.85f));
  EndTone();
}

// Hills (the same shapes as DrawHills), an inked ridge and dots at the foot
void DrawComicHills(float topY, float bottomY, float rise, int seed,
                    Color fill, Color shade) {
  const Rectangle all = FullScreen();
  const float step = 8;
  std::vector<Vector2> ridge;
  for (float x = all.x - step; x < all.x + all.width + step; x += step)
    ridge.push_back(
        {x, topY - rise * (0.5f + 0.3f * std::sin(x * 0.006f + seed) +
                           0.2f * std::sin(x * 0.017f + seed * 2.3f))});
  // the hills down to their foot, then dots growing towards it
  for (size_t i = 0; i + 1 < ridge.size(); i++) {
    const Vector2 a = ridge[i], b = ridge[i + 1];
    FillTriangle(a, {a.x, bottomY}, b, fill);
    FillTriangle(b, {a.x, bottomY}, {b.x, bottomY}, fill);
  }
  BeginTone(TONE_DOTS, 5);
  for (size_t i = 0; i + 1 < ridge.size(); i++) {
    const float y = std::fmax(ridge[i].y, ridge[i + 1].y); // under the ink
    DrawRectangleGradientV((int)ridge[i].x, (int)y, (int)step + 1,
                           (int)(bottomY - y), Faded(shade, 0.05f),
                           Faded(shade, 0.7f));
  }
  EndTone();
  std::vector<float> widths;
  for (size_t i = 0; i < ridge.size(); i++)
    widths.push_back(2.4f + 0.9f * std::sin(i * 0.37f + seed));
  Ribbon(ridge.data(), widths.data(), (int)ridge.size(), COMIC_INK);
}

// A coconut palm (the same shape as DrawPalm), inked, its fronds in leaflets
void DrawComicPalm(float x, float groundY, float height, float time,
                   Color trunk, Color leaves) {
  const float ink = std::fmax(1.4f, height * 0.013f);
  const float lean = height * 0.12f;
  const Vector2 foot = {x, groundY}, top = {x + lean, groundY - height};
  const Vector2 mid = {x + lean * 0.2f, groundY - height * 0.5f};
  const int N = 8;
  Vector2 points[N + 1], shadePoints[N + 1];
  float widths[N + 1], inked[N + 1], shadeWidths[N + 1];
  for (int i = 0; i <= N; i++) {
    const float t = (float)i / N;
    points[i] = Bezier(foot, mid, top, t);
    widths[i] = height * (0.055f - 0.022f * t);
    inked[i] = widths[i] + 2 * ink;
    shadePoints[i] = {points[i].x + widths[i] * 0.22f, points[i].y};
    shadeWidths[i] = widths[i] * 0.45f;
  }
  Ribbon(points, inked, N + 1, COMIC_INK);
  Ribbon(points, widths, N + 1, trunk);
  const Color trunkShade = {(unsigned char)(trunk.r * 0.72f),
                            (unsigned char)(trunk.g * 0.72f),
                            (unsigned char)(trunk.b * 0.72f), trunk.a};
  Ribbon(shadePoints, shadeWidths, N + 1, trunkShade);
  for (int i = 1; i < 6; i++) { // the rings on the trunk
    const Vector2 p = Bezier(foot, mid, top, i / 6.0f);
    const float half = height * (0.055f - 0.022f * i / 6.0f) / 2;
    InkLine({p.x - half, p.y - 1}, {p.x + half, p.y + 2}, ink * 0.9f);
  }
  // fronds: each rises a little from the top, then droops to its tip
  const Color leavesBack = {(unsigned char)(leaves.r * 0.78f),
                            (unsigned char)(leaves.g * 0.78f),
                            (unsigned char)(leaves.b * 0.78f), leaves.a};
  for (int i = 0; i < 8; i++) {
    const float side = (i % 2) ? 1.0f : -1.0f;
    const float reach = height * (0.2f + 0.07f * (i / 2));
    const float droop = height * (0.08f + 0.06f * (3 - i / 2) +
                                  0.01f * std::sin(time * 1.3f + i));
    const Vector2 tip = {top.x + side * reach, top.y + droop};
    const Vector2 bend = {top.x + side * reach * 0.5f,
                          top.y - height * (0.12f - 0.025f * (i / 2))};
    const int M = 14;
    Vector2 frond[M + 1];
    float leaf[M + 1], leafInk[M + 1], rib[M + 1];
    for (int k = 0; k <= M; k++) {
      const float t = (float)k / M;
      frond[k] = Bezier(top, bend, tip, t);
      // leaflets: the edge goes in and out
      const float w = height * 0.06f * std::pow(std::sin(PI * t), 0.7f);
      leaf[k] = (k % 2 && k < M) ? w * 0.45f : w;
      leafInk[k] = leaf[k] + 2 * ink;
      rib[k] = ink * 0.6f;
    }
    Ribbon(frond, leafInk, M + 1, COMIC_INK);
    Ribbon(frond, leaf, M + 1, i < 4 ? leavesBack : leaves);
    Ribbon(frond, rib, M + 1, COMIC_INK); // the midrib
  }
  const Color coconut = {150, 104, 46, 255};
  ComicCircle({top.x - height * 0.03f, top.y + height * 0.03f},
              height * 0.032f, coconut, ink);
  ComicCircle({top.x + height * 0.03f, top.y + height * 0.035f},
              height * 0.03f, coconut, ink);
}

// Jasmine (as DrawJasmine): a vine, inked leaves and small white flowers
void DrawComicJasmine(Rectangle area, float time) {
  for (int i = 0; i < 3; i++) // the vine
    InkCurve({area.x + area.width * Scatter(i + 900.5f), area.y},
             {area.x + area.width * Scatter(i + 910.5f),
              area.y + area.height * 0.5f},
             {area.x + area.width * Scatter(i + 920.5f), area.y + area.height},
             1.6f, {60, 80, 40, 255});
  for (int i = 0; i < 34; i++) {
    const Vector2 p = {area.x + Scatter(i + 0.5f) * area.width,
                       area.y + Scatter(i + 100.5f) * area.height};
    const float a = Scatter(i + 400.5f) * PI + 0.08f * std::sin(time + i);
    const Vector2 d = {std::cos(a) * 8, std::sin(a) * 8};
    const Vector2 leaf[3] = {Add(p, Scale(d, -1)), p, Add(p, d)};
    const float inked[3] = {2.2f, 10.5f, 2.2f}, fill[3] = {0.1f, 7.5f, 0.1f};
    Ribbon(leaf, inked, 3, COMIC_INK);
    Ribbon(leaf, fill, 3,
           i % 3 ? Color{56, 142, 62, 255} : Color{92, 172, 70, 255});
  }
  for (int i = 0; i < 16; i++) {
    const float x = area.x + Scatter(i + 200.5f) * area.width;
    const float y =
        area.y + Scatter(i + 300.5f) * area.height + std::sin(time + i) * 0.8f;
    for (int pass = 0; pass < 2; pass++) {
      for (int p = 0; p < 5; p++) {
        const float a = p * 2 * PI / 5 + i;
        Disc({x + 3.6f * std::cos(a), y + 3.6f * std::sin(a)},
             pass == 0 ? 3.4f : 2.5f,
             pass == 0 ? COMIC_INK : Color{255, 252, 240, 255});
      }
    }
    Disc({x, y}, 1.4f, {246, 196, 60, 255});
  }
}

// A brass or steel pot (as DrawPot): inked, a flat shadow on its lower
// right, and a glint
void DrawComicPot(float x, float y, float r, bool brass) {
  const Color body =
      brass ? Color{240, 184, 60, 255} : Color{204, 210, 218, 255};
  const Color shade =
      brass ? Color{190, 120, 36, 255} : Color{140, 148, 164, 255};
  const float ink = std::fmax(1.5f, r * 0.1f);
  DrawCircleV({x + ink * 0.35f, y + ink * 0.35f}, r + ink * 0.85f, COMIC_INK);
  DrawCircleV({x, y}, r, shade);
  DrawCircleV({x - r * 0.15f, y - r * 0.13f}, r * 0.82f, body);
  ComicRect({x - r * 0.45f, y - r * 1.14f, r * 0.9f, r * 0.27f}, body,
            ink * 0.8f);
  DrawEllipse((int)(x - r * 0.42f), (int)(y - r * 0.4f), r * 0.2f, r * 0.11f,
              {255, 252, 236, 255});
  DrawSparkle({x - r * 0.6f, y - r * 0.55f}, r * 0.32f, {255, 252, 236, 255});
}
