// lp_common.h - small drawing and input helpers shared by the scenes of
// Chapter 2 "The List" (ch2_list.cpp) and Chapter 5 "Preparations"
// (ch5_preparations.cpp). Everything is inline and file-local in spirit.
#pragma once

#include <cmath>
#include <string>

#include "game.h"
#include "rlgl.h"

namespace lp {

const Color SKIN = {184, 124, 86, 255}; // as the people in art.cpp
const Color SKIN_DARK = {150, 96, 68, 255};
const Color PENCIL = {70, 66, 74, 255};
const Color BALLPOINT = {40, 52, 110, 255}; // Amma's blue pen
const Color LIST_PAPER = {250, 244, 226, 255};
const Color MARIGOLD = {250, 150, 30, 255};
const Color KANAKAMBARAM = {255, 120, 40, 255};
const Color LEAF_GREEN = {56, 140, 58, 255};
const Color LEAF_LIGHT = {96, 168, 64, 255};
const Color WOOD = {164, 94, 46, 255};
const Color WOOD_DARK = {112, 58, 30, 255};
const Color WOOD_LIGHT = {212, 140, 74, 255};
const Color LIME_PLASTER = {252, 222, 140, 255};

// Draws everything until PopZoom() bigger by `scale`, around `centre` (a
// close-up of a place, or a person drawn bigger than art.cpp draws them)
inline void PushZoom(Vector2 centre, float scale) {
  rlPushMatrix();
  rlTranslatef(centre.x, centre.y, 0);
  rlScalef(scale, scale, 1);
  rlTranslatef(-centre.x, -centre.y, 0);
}
inline void PopZoom() { rlPopMatrix(); }
// Where a point drawn inside PushZoom(centre, scale) lands on the stage
inline Vector2 Zoomed(Vector2 p, Vector2 centre, float scale) {
  return {centre.x + (p.x - centre.x) * scale,
          centre.y + (p.y - centre.y) * scale};
}

inline float Dist(Vector2 a, Vector2 b) {
  return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
inline Vector2 Lerp2(Vector2 a, Vector2 b, float t) {
  return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}
inline Vector2 Mouse() { return GetMousePosition(); }
inline bool Pressed() { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }
inline bool Released() { return IsMouseButtonReleased(MOUSE_BUTTON_LEFT); }
inline bool Held() {
  return IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE) ||
         IsKeyDown(KEY_ENTER);
}
inline float Clamp01(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }

// A comic speech balloon: rounded paper, inked, with a tail pointing at
// whoever is talking (`speaker`). `centre` is the balloon's middle.
// `shown` 0..1 fades and pops it in.
inline void DrawBubble(const std::string &text, Vector2 centre,
                       Vector2 speaker, float shown = 1, float size = 22) {
  if (shown <= 0)
    return;
  const float pop = 0.85f + 0.15f * Clamp01(shown * 1.5f);
  const float w = (MeasureString(text, size) + 36) * pop, h = (size + 26) * pop;
  const Rectangle r = {centre.x - w / 2, centre.y - h / 2, w, h};
  const Vector2 base = {centre.x + (speaker.x - centre.x) * 0.15f,
                        centre.y + (speaker.y > centre.y ? h / 2 - 4
                                                         : -h / 2 + 4)};
  const Vector2 tip = Lerp2(base, speaker, 0.75f);
  const Color paper = Faded(COMIC_PAPER, shown * 3);
  const Color ink = Faded(COMIC_INK, shown * 3);
  // ink first, a little bigger, then the paper over it
  DrawRectangleRounded({r.x - 3, r.y - 3, r.width + 6, r.height + 6}, 0.6f, 10,
                       ink);
  FillTriangle({base.x - 13, base.y}, {base.x + 13, base.y},
               {tip.x + 1.5f, tip.y + 1.5f}, ink);
  DrawRectangleRounded(r, 0.6f, 10, paper);
  FillTriangle({base.x - 9, base.y}, {base.x + 9, base.y}, tip, paper);
  DrawRectangleRec({base.x - 9, base.y - 4, 18, 6}, paper);
  DrawTextCentered(text, centre.x, centre.y - size * 0.55f * pop, size * pop,
                   Faded({40, 34, 44, 255}, shown * 3));
}

// The quiet pulsing ring that says "click to go on"
inline void DrawGoOn(float time) {
  DrawRing({680, 680}, 10, 13, 0, 360, 24,
           Faded(COMIC_PAPER, 0.45f + 0.4f * std::sin(time * 3)));
  DrawRing({680, 680}, 13, 14.5f, 0, 360, 24, Faded(COMIC_INK, 0.5f));
}

// A soft glow of dots around something that can be picked up or tapped
inline void DrawHint(Vector2 c, float r, float time, Color color = {255, 246,
                                                                    196, 255}) {
  const float pulse = 0.5f + 0.5f * std::sin(time * 3.2f);
  BeginBlendMode(BLEND_ADDITIVE);
  DrawCircleGradient(c, r * (1.1f + 0.15f * pulse), Faded(color, 0.25f + 0.2f * pulse),
                     Faded(color, 0));
  EndBlendMode();
}

// A pencil tick, drawn in as `t` goes 0..1
inline void DrawTick(Vector2 at, float t, float size = 14, Color c = PENCIL) {
  if (t <= 0)
    return;
  const Vector2 a = {at.x - size * 0.5f, at.y}, b = {at.x - size * 0.1f, at.y + size * 0.45f},
                c2 = {at.x + size * 0.7f, at.y - size * 0.6f};
  const float t1 = Clamp01(t * 2.5f), t2 = Clamp01(t * 2.5f - 1);
  DrawLineEx(a, Lerp2(a, b, t1), 2.6f, c);
  if (t2 > 0)
    DrawLineEx(b, Lerp2(b, c2, t2), 2.6f, c);
}

// A small inked dot of colour: cheaper than ComicCircle (fewer segments),
// for things drawn by the hundred (flowers in heaps and garlands)
inline void InkDot(Vector2 c, float r, Color fill, float ink = 1.2f) {
  const int seg = r < 6 ? 8 : 12;
  DrawCircleSector({c.x + ink * 0.3f, c.y + ink * 0.3f}, r + ink, 0, 360, seg,
                   COMIC_INK);
  DrawCircleSector(c, r, 0, 360, seg, fill);
}

// One flower head, comic style: inked petals round a dark middle
inline void DrawFlower(Vector2 c, float r, Color petal, Color middle,
                       int petals = 5, float turn = 0) {
  DrawCircleSector({c.x + 0.6f, c.y + 0.6f}, r * 1.05f, 0, 360, 12, COMIC_INK);
  for (int p = 0; p < petals; p++) {
    const float a = turn + p * 2 * PI / petals;
    DrawCircleSector({c.x + r * 0.5f * std::cos(a), c.y + r * 0.5f * std::sin(a)},
                     r * 0.48f, 0, 360, 8, petal);
  }
  DrawCircleSector(c, r * 0.3f, 0, 360, 8, middle);
}

// Kanakambaram: a little spike of soft orange flowers (as pinned in a plait)
inline void DrawKanakambaram(Vector2 c, float r, float turn = 0) {
  for (int i = 0; i < 4; i++) {
    const float a = turn + i * PI / 2 + 0.4f;
    const Vector2 p = {c.x + r * 0.55f * std::cos(a), c.y + r * 0.55f * std::sin(a)};
    DrawCircleSector({p.x + 0.5f, p.y + 0.5f}, r * 0.62f, 0, 360, 8, COMIC_INK);
  }
  for (int i = 0; i < 4; i++) {
    const float a = turn + i * PI / 2 + 0.4f;
    const Vector2 p = {c.x + r * 0.55f * std::cos(a), c.y + r * 0.55f * std::sin(a)};
    DrawCircleSector(p, r * 0.5f, 0, 360, 8, KANAKAMBARAM);
    DrawCircleSector({p.x - r * 0.12f, p.y - r * 0.12f}, r * 0.2f, 0, 360, 6,
                     {255, 190, 120, 255});
  }
  DrawCircleSector(c, r * 0.18f, 0, 360, 6, {200, 70, 20, 255});
}

// A mango leaf hanging from `stem`, swaying
inline void DrawMangoLeaf(Vector2 stem, float length, float sway, int i) {
  DrawComicLeaf(stem, {stem.x + sway, stem.y + length}, length * 0.37f,
                i % 2 ? LEAF_GREEN : LEAF_LIGHT, 1.4f);
}

// Part of an ellipse's outline, from angle `from` to `to` (radians; 0 is
// to the right, PI/2 down)
inline void DrawEllipseArc(Vector2 c, float rx, float ry, float from, float to,
                           float thick, Color color, int segments = 14) {
  Vector2 points[33];
  if (segments > 32)
    segments = 32;
  for (int i = 0; i <= segments; i++) {
    const float a = from + (to - from) * i / segments;
    points[i] = {c.x + rx * std::cos(a), c.y + ry * std::sin(a)};
  }
  DrawSplineLinear(points, segments + 1, thick, color); // one call, mitred
}

// A glass bangle, seen tilted: an inked ring of bright glass with a glint.
// `half`: 0 the whole ring, -1 only its far half (drawn behind an arm),
// 1 only its near half (drawn in front of it). Vertical ellipses (rx < ry)
// sit on a wrist; flat ones (rx > ry) are stacked on a pole.
inline void DrawBangle(Vector2 c, float rx, float ry, Color glass,
                       int half = 0) {
  const bool upright = ry > rx;
  // the near half of an upright ring is its left side; of a flat one, its
  // bottom
  float from = 0, to = 2 * PI;
  if (half != 0) {
    const float nearMid = upright ? PI : PI / 2;
    from = half > 0 ? nearMid - PI / 2 : nearMid + PI / 2;
    to = from + PI;
  }
  DrawEllipseArc(c, rx, ry, from, to, 4.6f, COMIC_INK);
  DrawEllipseArc(c, rx, ry, from, to, 2.6f, glass);
  if (half >= 0) { // a glint on the near side
    const float g = upright ? PI * 0.85f : PI * 0.62f;
    DrawEllipseArc(c, rx, ry, g, g + 0.35f, 1.2f, {255, 255, 255, 220}, 4);
  }
}

// An arm reaching into the picture from off screen (`from`), as Anu sees
// her own: a sleeve, the forearm, an open hand at `hand`. `width` is the
// wrist's; `sleeve` covers that much of the arm (0..1).
inline Vector2 ArmDir(Vector2 from, Vector2 hand) {
  const float len = std::fmax(1, Dist(from, hand));
  return {(hand.x - from.x) / len, (hand.y - from.y) / len};
}
inline Vector2 ArmWrist(Vector2 from, Vector2 hand, float width) {
  const Vector2 d = ArmDir(from, hand);
  return {hand.x - d.x * width * 0.9f, hand.y - d.y * width * 0.9f};
}
inline void DrawPovArm(Vector2 from, Vector2 hand, float width, Color sleeve,
                       float sleeveUntil, float curl = 0) {
  const Vector2 d = ArmDir(from, hand), n = {-d.y, d.x};
  const Vector2 wrist = ArmWrist(from, hand, width);
  auto quad = [&](Vector2 a, Vector2 b, float wa, float wb, Color fill) {
    const Vector2 q[4] = {{a.x + n.x * wa, a.y + n.y * wa},
                          {b.x + n.x * wb, b.y + n.y * wb},
                          {b.x - n.x * wb, b.y - n.y * wb},
                          {a.x - n.x * wa, a.y - n.y * wa}};
    ComicPoly(q, 4, fill, 2.4f);
  };
  // fingers first (the palm covers their roots), then the forearm
  for (int f = 0; f < 4; f++) {
    const float spread = (f - 1.5f) * 0.22f;
    const Vector2 fd = {d.x * std::cos(spread) - d.y * std::sin(spread),
                        d.x * std::sin(spread) + d.y * std::cos(spread)};
    const float len = width * (f == 0 || f == 3 ? 0.95f : 1.15f) * (1 - curl * 0.5f);
    const Vector2 root = {hand.x + n.x * (f - 1.5f) * width * 0.26f,
                          hand.y + n.y * (f - 1.5f) * width * 0.26f};
    const Vector2 tip = {root.x + fd.x * len, root.y + fd.y * len};
    DrawLineEx(root, tip, width * 0.32f + 3.4f, COMIC_INK);
    DrawCircleV(tip, width * 0.16f + 1.7f, COMIC_INK);
    DrawLineEx(root, tip, width * 0.32f, SKIN);
    DrawCircleV(tip, width * 0.16f, SKIN);
  }
  quad(from, wrist, width * 1.25f, width * 0.62f, SKIN);
  // the palm, and the thumb along its near side
  ComicCircle(hand, width * 0.72f, SKIN, 2.2f);
  const Vector2 thumbRoot = {hand.x + n.x * width * 0.5f - d.x * width * 0.3f,
                             hand.y + n.y * width * 0.5f - d.y * width * 0.3f};
  const Vector2 thumbTip = {thumbRoot.x + (d.x + n.x * 0.9f) * width * 0.6f,
                            thumbRoot.y + (d.y + n.y * 0.9f) * width * 0.6f};
  DrawLineEx(thumbRoot, thumbTip, width * 0.34f + 3.4f, COMIC_INK);
  DrawLineEx(thumbRoot, thumbTip, width * 0.34f, SKIN);
  DrawCircleV(thumbTip, width * 0.17f, SKIN);
  DrawCircleV(hand, width * 0.6f, SKIN);
  // shade down the far side of the arm
  const Vector2 sh[4] = {{from.x - n.x * width * 1.2f, from.y - n.y * width * 1.2f},
                         {wrist.x - n.x * width * 0.6f, wrist.y - n.y * width * 0.6f},
                         {wrist.x - n.x * width * 0.25f, wrist.y - n.y * width * 0.25f},
                         {from.x - n.x * width * 0.5f, from.y - n.y * width * 0.5f}};
  FillTriangle(sh[0], sh[1], sh[2], SKIN_DARK);
  FillTriangle(sh[0], sh[2], sh[3], SKIN_DARK);
  if (sleeveUntil > 0) {
    const Vector2 cuff = Lerp2(from, wrist, sleeveUntil);
    quad({from.x - d.x * 40, from.y - d.y * 40}, cuff, width * 1.45f,
         width * 1.05f, sleeve);
  }
}

// Bangles round the wrist of an arm drawn with DrawPovArm: `count` of them,
// `colours` each, slid `slide[k]` along the arm (0 = at the wrist). Call with
// half = -1 before the arm (their far halves) and half = 1 after it.
inline void DrawArmBangles(Vector2 from, Vector2 hand, float width,
                           const Color *colours, const float *slide, int count,
                           int half) {
  const Vector2 d = ArmDir(from, hand);
  const Vector2 wrist = ArmWrist(from, hand, width);
  rlPushMatrix();
  rlTranslatef(wrist.x, wrist.y, 0);
  rlRotatef(std::atan2(d.y, d.x) * RAD2DEG, 0, 0, 1);
  for (int k = 0; k < count; k++) {
    // they sit along the forearm, towards the elbow (local -x)
    const float x = -k * width * 0.42f - width * 0.2f + slide[k];
    DrawBangle({x, 0}, width * 0.3f, width * 0.86f, colours[k], half);
  }
  rlPopMatrix();
}

} // namespace lp
