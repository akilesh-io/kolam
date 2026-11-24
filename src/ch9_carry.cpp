// ch9_carry.cpp - Chapter 9, "What We Carry": after the festival.
//   pack_bag        three days later the kolam is still on the step, walked
//                   over, half rubbed away. Amma holds out a tiffin box, then
//                   a small tin of rice flour: drag them into Anu's bag.
//                   "Amma, I live in a flat." "Flats have doors."
//   hyderabad_back  Hyderabad, 11:47 pm: the same desk as Chapter 1, now warm.
//                   The work goes where it should; Rohit asks about sweets.
//                   Then the blank sticky note on the screen's edge: draw on
//                   it, and any scribble becomes a small pencil flower.
//   call_amma       this time Anu calls first: tap Amma's name. The end.
#include "finale.h"

using namespace finale;

namespace {

const Color STEEL_9 = {206, 212, 222, 255};
const Color STEEL_SHADE_9 = {150, 158, 172, 255};
const Color DRY_GROUND = {182, 142, 99, 255};

// A three-tier steel tiffin box, wrapped in a checked cloth tied on top
void DrawTiffin(Vector2 c, float s = 1) {
  ComicRect({c.x - 18 * s, c.y - 26 * s, 36 * s, 46 * s}, STEEL_9, 2);
  for (int i = 1; i < 3; i++)
    InkLine({c.x - 18 * s, c.y - 26 * s + i * 15 * s},
            {c.x + 18 * s, c.y - 26 * s + i * 15 * s}, 1.4f);
  DrawRectangleRec({c.x + 8 * s, c.y - 24 * s, 6 * s, 42 * s}, STEEL_SHADE_9);
  // the cloth, knotted on top
  const Vector2 cloth[4] = {{c.x - 22 * s, c.y + 4 * s},
                            {c.x + 22 * s, c.y + 4 * s},
                            {c.x + 24 * s, c.y + 22 * s},
                            {c.x - 24 * s, c.y + 22 * s}};
  ComicPoly(cloth, 4, {200, 60, 70, 255}, 1.8f);
  for (int i = 0; i < 4; i++)
    InkLine({c.x - 16 * s + i * 11 * s, c.y + 5 * s},
            {c.x - 17 * s + i * 11 * s, c.y + 21 * s}, 1, {250, 230, 220, 255});
  ComicEllipse({c.x - 8 * s, c.y - 32 * s}, 9 * s, 6 * s, {200, 60, 70, 255}, 1.6f);
  ComicEllipse({c.x + 8 * s, c.y - 32 * s}, 9 * s, 6 * s, {200, 60, 70, 255}, 1.6f);
}

// The small tin of rice flour (as in Chapter 8)
void DrawSmallTin(Vector2 c, float s = 1) {
  ComicRect({c.x - 13 * s, c.y - 6 * s, 26 * s, 18 * s}, STEEL_9, 2);
  ComicEllipse({c.x, c.y - 6 * s}, 13 * s, 4.5f * s, {236, 238, 242, 255}, 1.6f);
  DrawRectangleRec({c.x - 13 * s, c.y, 26 * s, 3 * s}, STEEL_SHADE_9);
}

//------------------------------------------------------------------------------
// The step: the tiffin and the flour, into the bag
//------------------------------------------------------------------------------
const Vector2 AMMA_9 = {330, 560};
const Vector2 HELD_9 = {426, 372};   // in Amma's outstretched hand
const Vector2 ANU_9 = {590, 676};
const Vector2 BAG_MOUTH = {476, 640}; // the open bag, on the street
const Vector2 KOLAM_9 = {362, 620};

struct StepState {
  int packed = 0;       // how many things are in the bag
  bool offering = true; // Amma's hand out
  Vector2 item = HELD_9;
  bool showItem = true;
  float zip = 0; // the bag closing
};
StepState stepState;

void DrawBag(float zip) {
  const Vector2 c = BAG_MOUTH;
  // a soft travel bag, dark blue, its mouth open (then zipped)
  DrawComicShadow(c.x, c.y + 44, 150);
  const Vector2 body[6] = {{c.x - 70, c.y + 4}, {c.x + 70, c.y + 4},
                           {c.x + 78, c.y + 30}, {c.x + 62, c.y + 46},
                           {c.x - 62, c.y + 46}, {c.x - 78, c.y + 30}};
  ComicPoly(body, 6, {54, 70, 128, 255}, 2.6f);
  DrawToneRect({c.x - 70, c.y + 26, 140, 20}, COMIC_INK, 0, 0.4f, TONE_DOTS, 4);
  ComicEllipse(c, 70, 12 * (1 - zip) + 2, zip > 0.9f ? Color{70, 88, 150, 255}
                                                    : Color{24, 26, 44, 255},
               2.4f);
  InkLine({c.x - 66, c.y}, {c.x + 66, c.y}, 1.5f, {200, 200, 210, 255}); // zip
  InkCurve({c.x - 40, c.y + 2}, {c.x - 20, c.y - 40}, {c.x, c.y + 2}, 4,
           {40, 50, 90, 255}); // the handles
  InkCurve({c.x + 4, c.y + 2}, {c.x + 24, c.y - 40}, {c.x + 44, c.y + 2}, 4,
           {40, 50, 90, 255});
}

void DrawStepPeople9(float time) {
  const StepState &st = stepState;
  // the kolam, three days old: walked over, half rubbed away, still there
  DrawEllipse((int)KOLAM_9.x, (int)KOLAM_9.y - 2, 206, 44, DRY_GROUND);
  DrawGroundKolam(KOLAM_9, 330, 0.55f);
  for (int i = 0; i < 6; i++) { // footprints across it, in and out
    const float t = i / 5.0f;
    const Vector2 f = {230 + t * 250 + (i % 2) * 8, 650 - t * 52 + (i % 2) * 10};
    DrawEllipse((int)f.x, (int)f.y, 9, 4, Faded({130, 96, 66, 255}, 0.55f));
    DrawEllipse((int)f.x + 6, (int)f.y - 5, 3, 2, Faded({130, 96, 66, 255}, 0.55f));
  }
  DrawAmma(AMMA_9.x, AMMA_9.y, time, st.offering);
  DrawBag(st.zip);
  Mirrored(ANU_9.x, [&]() {
    DrawAnu(ANU_9.x, ANU_9.y, time, {0.13f, 0.5f}, {0.6f, -0.1f});
  });
}

class PackBagScene : public Scene {
public:
  explicit PackBagScene(const Step &s) : step(s) { stepState = StepState(); }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    PlayAmbience("birds_day", 0.45f);
    StepState &st = stepState;
    const Vector2 m = GetMousePosition();
    switch (stage) {
    case HOLDING: // drag what Amma holds out into the bag (or tap it)
      if (stageTime > 0.8f && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
          std::hypot(m.x - st.item.x, m.y - st.item.y) < 60) {
        dragging = true;
        pressedAt = m;
        grab = {m.x - st.item.x, m.y - st.item.y};
      }
      if (dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        st.item = {m.x - grab.x, m.y - grab.y};
      if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        dragging = false;
        const bool tapped = std::hypot(m.x - pressedAt.x, m.y - pressedAt.y) < 12;
        const bool inBag = std::fabs(st.item.x - BAG_MOUTH.x) < 100 &&
                           std::fabs(st.item.y - BAG_MOUTH.y) < 80;
        if (tapped || inBag) {
          from = st.item;
          Next(DROPPING);
        }
      }
      if (!dragging)
        st.item = {st.item.x + (HELD_9.x - st.item.x) * std::fmin(1.0f, dt * 10),
                   st.item.y + (HELD_9.y - st.item.y) * std::fmin(1.0f, dt * 10)};
      break;
    case DROPPING: { // it drops into the bag
      const float t = Between(stageTime, 0, 0.45f);
      st.item = {from.x + (BAG_MOUTH.x - from.x) * t,
                 from.y + (BAG_MOUTH.y - from.y) * t - 40 * std::sin(t * PI)};
      if (stageTime > 0.45f) {
        PlayEffect("click", 0.5f, 0.8f);
        st.packed++;
        st.showItem = false;
        st.offering = false;
        if (st.packed == 1)
          Next(NEXT_ITEM);
        else {
          talk.Start(step.lines);
          Next(TALK);
        }
      }
      break;
    }
    case NEXT_ITEM: // and one more thing
      if (stageTime > 1.0f) {
        st.offering = true;
        st.showItem = true;
        st.item = HELD_9;
        Next(HOLDING);
      }
      break;
    case TALK:
      if (talk.Update())
        Next(ZIP);
      break;
    case ZIP:
      st.zip = Between(stageTime, 0.2f, 0.9f);
      if (stageTime > 0.2f && stageTime - dt <= 0.2f)
        PlayEffect("scribble", 0.5f, 0.7f); // the zip
      if (stageTime > 2.2f)
        done = true;
      break;
    }
  }

  void Draw() override {
    DrawPlace("front_step", DAY, time, DrawStepPeople9);
    const StepState &st = stepState;
    if (st.showItem) {
      if (st.packed == 0)
        DrawTiffin(st.item);
      else
        DrawSmallTin(st.item, 1.2f);
      if (stage == HOLDING && !dragging && stageTime > 0.8f)
        DrawHint(st.item, 32, time);
    }
    talk.Draw([](const std::string &who) {
      if (who == "Anu")
        return BubbleSpot{{560, 300}, {596, 396}};
      return BubbleSpot{{200, 230}, {326, 290}};
    });
    if (stage == ZIP)
      FadeScreen({20, 18, 30, 255}, Between(stageTime, 1.2f, 2.2f));
  }

private:
  enum Stage { HOLDING, DROPPING, NEXT_ITEM, TALK, ZIP };
  const Step &step;
  Conversation talk;
  Stage stage = HOLDING;
  float time = 0, stageTime = 0;
  bool dragging = false;
  Vector2 grab = {0, 0}, pressedAt = {0, 0}, from = {0, 0};

  void Next(Stage s) {
    stage = s;
    stageTime = 0;
  }
};

//------------------------------------------------------------------------------
// Hyderabad again: the same desk as on the first night, now warm
//------------------------------------------------------------------------------
const Rectangle LAPTOP_9 = {166, 186, 388, 268};
const Rectangle STICKY = {520, 192, 44, 44}; // on the edge of the screen
const Rectangle PHONE_9 = {590, 520, 52, 92}; // lying on the desk
const Color CHAT_PURPLE_9 = {120, 80, 200, 255};

// The flower the sticky note ends up with: a stem, two leaves and five
// petals, as one pencil line (so a scribble can turn into it)
std::vector<Vector2> FlowerLine(Vector2 c, float size) {
  std::vector<Vector2> p;
  for (int i = 0; i <= 12; i++) // the stem, up from the bottom
    p.push_back({c.x + 4 * std::sin(i * 0.3f) * size / 60,
                 c.y + size * (1.1f - i / 12.0f * 1.1f)});
  for (int i = 0; i <= 10; i++) { // a leaf, out and back
    const float t = i / 10.0f;
    const float a = t * PI;
    p.push_back({c.x + std::sin(a) * size * 0.45f * (1 - std::fabs(t - 0.5f)),
                 c.y + size * 0.6f - std::sin(a) * size * 0.18f -
                     t * size * 0.08f});
  }
  for (int i = 0; i <= 120; i++) { // five petals: a rose curve
    const float th = i / 120.0f * PI;
    const float r = size * 0.5f * std::fabs(std::cos(2.5f * th * 2));
    const float a = th * 2 - PI / 2;
    p.push_back({c.x + r * std::cos(a), c.y + r * std::sin(a)});
  }
  return p;
}

// `n` points evenly along a line
std::vector<Vector2> Resample(const std::vector<Vector2> &line, int n) {
  std::vector<Vector2> out;
  if (line.size() < 2) {
    for (int i = 0; i < n; i++)
      out.push_back(line.empty() ? Vector2{0, 0} : line[0]);
    return out;
  }
  float total = 0;
  for (size_t i = 1; i < line.size(); i++)
    total += std::hypot(line[i].x - line[i - 1].x, line[i].y - line[i - 1].y);
  size_t seg = 1;
  float walked = 0;
  for (int k = 0; k < n; k++) {
    const float want = total * k / (n - 1);
    while (seg < line.size() - 1) {
      const float len = std::hypot(line[seg].x - line[seg - 1].x,
                                   line[seg].y - line[seg - 1].y);
      if (walked + len >= want)
        break;
      walked += len;
      seg++;
    }
    const float len = std::fmax(0.001f, std::hypot(line[seg].x - line[seg - 1].x,
                                                  line[seg].y - line[seg - 1].y));
    const float t = std::fmin(1, std::fmax(0, (want - walked) / len));
    out.push_back({line[seg - 1].x + (line[seg].x - line[seg - 1].x) * t,
                   line[seg - 1].y + (line[seg].y - line[seg - 1].y) * t});
  }
  return out;
}

// The pencil flower, small, on the sticky note on the laptop's edge
void DrawSmallFlowerNote(float glow, float time) {
  const Rectangle r = STICKY;
  DrawRectangleRec({r.x + 2, r.y + 3, r.width, r.height}, {0, 0, 0, 60});
  DrawRectangleRec(r, {250, 222, 100, 255});
  DrawRectangleRec({r.x, r.y, r.width, 8}, {240, 206, 80, 255});
  if (glow > 0)
    DrawHint({r.x + r.width / 2, r.y + r.height / 2}, 30, time);
}

class HyderabadScene : public Scene {
public:
  explicit HyderabadScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    PlayAmbience("city_night", 0.3f);
    const Vector2 m = GetMousePosition();
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    for (float &c : cardMove)
      c = std::fmin(1, c + dt * 3);
    chatY += ((chatShown ? 70.0f : -80.0f) - chatY) * std::fmin(1.0f, dt * 10);
    if (replying)
      replyTyped += TextCharsPerSecond() * dt;

    switch (stage) {
    case WORK:
      // the card goes where it should, and stays there
      if (clicked && placed < 3 && CheckCollisionPointRec(m, CardRect())) {
        PlayEffect("click", 0.6f, 0.95f + placed * 0.08f);
        cardMove[placed] = 0;
        placed++;
      }
      if (!chatShown && stageTime > 2.2f) {
        chatShown = true;
        PlayEffect("notify", 0.35f);
      }
      if (chatShown && !replying && clicked && CheckCollisionPointRec(m, ChatRect())) {
        replying = true;
        PlayEffect("click", 0.5f);
      }
      if (placed == 3 && replying && replyTyped > ReplyLength() + 8) {
        Next(NOTE);
      }
      break;
    case NOTE: // the empty sticky note, glowing a little: tap it
      if (stageTime > 0.6f && clicked &&
          CheckCollisionPointRec(m, {STICKY.x - 16, STICKY.y - 16,
                                     STICKY.width + 32, STICKY.height + 32})) {
        PlayEffect("page", 0.5f, 1.3f);
        Next(DRAWING);
      }
      break;
    case DRAWING: { // up close: draw on it in pencil
      const Rectangle pad = NotePad();
      if (stageTime > 0.6f && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
          CheckCollisionPointRec(m, pad)) {
        if (scribble.empty() ||
            std::hypot(m.x - scribble.back().x, m.y - scribble.back().y) > 3) {
          if (!scribble.empty())
            inked += std::hypot(m.x - scribble.back().x, m.y - scribble.back().y);
          scribble.push_back(m);
          PlayLoop("pencil", 0.5f);
        }
      }
      // a scribble long enough, and the pencil lifted: it becomes a flower
      if (inked > 220 && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        from = Resample(scribble, 160);
        to = Resample(FlowerLine({pad.x + pad.width / 2, pad.y + pad.height * 0.42f},
                                 pad.width * 0.42f),
                      160);
        PlayEffect("bell", 0.35f, 1.3f);
        Next(BLOOM);
      }
      break;
    }
    case BLOOM:
      if (stageTime > 3.0f)
        Next(BACK);
      break;
    case BACK:
      if (stageTime > 2.5f)
        done = true;
      break;
    }
  }

  void Draw() override {
    DrawWarmFlat(time);
    DrawLaptop();
    DrawDeskThings(time);
    DrawSmallFlowerNote(stage == NOTE ? 1 : 0, time);
    if (stage == BACK || flowerDone) // the flower, small, on the note now
      DrawFlowerPath(Resample(to, 160), {STICKY.x + 22, STICKY.y + 21},
                     STICKY.width / NotePad().width, 1);
    DrawChat();
    // the clock: the same time as the first night
    const float left = FullScreen().x + 30;
    DrawString("11:47 PM", left, 24, 30, FLOUR);
    DrawString("Hyderabad", left, 58, 18, MUTED);

    if (stage == DRAWING || stage == BLOOM ||
        (stage == BACK && stageTime < 0.6f)) {
      const float a = stage == BACK ? 1 - Between(stageTime, 0, 0.6f)
                                    : Between(stage == DRAWING ? stageTime : 9, 0, 0.4f);
      DrawRectangleRec(FullScreen(), Faded({20, 16, 24, 255}, 0.6f * a));
      const Rectangle pad = NotePad();
      DrawRectangleRec({pad.x + 8, pad.y + 10, pad.width, pad.height},
                       Faded({0, 0, 0, 255}, 0.3f * a));
      DrawRectangleRec(pad, Faded({250, 222, 100, 255}, a));
      DrawRectangleRec({pad.x, pad.y, pad.width, 40}, Faded({240, 206, 80, 255}, a));
      const Color pencil = Faded({70, 66, 80, 255}, a);
      if (stage == DRAWING) {
        for (size_t i = 1; i < scribble.size(); i++)
          DrawLineEx(scribble[i - 1], scribble[i], 3, pencil);
        if (scribble.empty() && stageTime > 1.2f) // a pencil, waiting
          DrawPencil({pad.x + pad.width / 2, pad.y + pad.height / 2}, a);
      } else {
        // the scribble finds its way into a flower
        const float t = Between(stage == BLOOM ? stageTime : 9, 0, 1.6f);
        std::vector<Vector2> line;
        for (size_t i = 0; i < from.size(); i++)
          line.push_back({from[i].x + (to[i].x - from[i].x) * t,
                          from[i].y + (to[i].y - from[i].y) * t});
        for (size_t i = 1; i < line.size(); i++)
          DrawLineEx(line[i - 1], line[i], 3, pencil);
        if (t >= 1)
          DrawCircleV({pad.x + pad.width / 2, pad.y + pad.height * 0.42f}, 4, pencil);
      }
    }
    if (stage == BACK)
      flowerDone = true;
  }

private:
  enum Stage { WORK, NOTE, DRAWING, BLOOM, BACK };
  const Step &step;
  Stage stage = WORK;
  float time = 0, stageTime = 0;
  int placed = 0;
  float cardMove[3] = {1, 1, 1};
  bool chatShown = false, replying = false;
  float chatY = -80, replyTyped = 0;
  std::vector<Vector2> scribble, from, to;
  float inked = 0;
  bool flowerDone = false;

  void Next(Stage s) {
    stage = s;
    stageTime = 0;
  }

  static Rectangle NotePad() { return {210, 150, 300, 300}; }

  size_t ReplyLength() const {
    return step.lines.size() > 1 ? step.lines[1].text.size() : 0;
  }

  // Where the card is: in the tray at the bottom, or (placed) on its screen
  Rectangle CardRect() const { return {LAPTOP_9.x + 34, LAPTOP_9.y + 214, 50, 30}; }
  static Rectangle Slot(int i) {
    return {204.0f + i * 112 + 12, 222 + 100.0f, 60, 36};
  }

  Rectangle ChatRect() const {
    const Rectangle all = FullScreen();
    const float x = std::fmin(540, all.x + all.width - 290);
    return {x, chatY, 278, replying ? 116.0f : 60.0f};
  }

  void DrawChat() const {
    if (chatY < -70 || step.lines.empty())
      return;
    const Rectangle r = ChatRect();
    DrawRectangleRounded({r.x + 3, r.y + 4, r.width, r.height}, 0.15f, 6,
                         {0, 0, 0, 50});
    DrawRectangleRounded(r, 0.15f, 6, {250, 250, 252, 245});
    DrawCircle((int)r.x + 24, (int)r.y + 28, 12, CHAT_PURPLE_9);
    DrawString(step.lines[0].speaker, r.x + 46, r.y + 6, 17, {30, 30, 40, 255});
    DrawTextWrapped(step.lines[0].text, r.x + 46, r.y + 28, 15, r.width - 56,
                    {90, 90, 100, 255});
    if (!replying) {
      if (stage == WORK && chatShown)
        DrawRing({r.x + r.width - 16, r.y + 16}, 5, 7, 0, 360, 16,
                 Faded(CHAT_PURPLE_9, 0.5f + 0.4f * std::sin(time * 4)));
      return;
    }
    // her reply, typed out, in a bubble on the right
    const std::string &reply = step.lines[1].text;
    const std::string shown =
        reply.substr(0, std::min(reply.size(), (size_t)replyTyped));
    const Rectangle b = {r.x + 30, r.y + 62, r.width - 40, 46};
    DrawRectangleRounded(b, 0.3f, 6, {226, 236, 255, 255});
    DrawTextWrapped(shown, b.x + 10, b.y + 5, 15, b.width - 20, {40, 50, 90, 255});
  }

  void DrawLaptop() const {
    DrawRectangleRounded({150, 170, 420, 300}, 0.03f, 4, {36, 38, 44, 255});
    DrawRectangleRec(LAPTOP_9, {246, 246, 250, 255});
    DrawRectangle((int)LAPTOP_9.x, (int)LAPTOP_9.y, (int)LAPTOP_9.width, 24,
                  {226, 228, 236, 255});
    DrawString("Onboarding  v12", 176, 189, 16, {60, 60, 80, 255});
    if (placed == 3) { // done, and it's good
      const Vector2 c = {LAPTOP_9.x + LAPTOP_9.width - 20, LAPTOP_9.y + 12};
      DrawCircleV(c, 8, {40, 170, 90, 255});
      DrawLineEx({c.x - 4, c.y}, {c.x - 1, c.y + 3}, 2, WHITE);
      DrawLineEx({c.x - 1, c.y + 3}, {c.x + 4, c.y - 3}, 2, WHITE);
    }
    DrawRectangle((int)LAPTOP_9.x, 210, 26, 244, {232, 234, 240, 255});
    for (int i = 0; i < 5; i++)
      DrawRectangleRounded({LAPTOP_9.x + 6, 222.0f + i * 26, 14, 14}, 0.3f, 4,
                           {170, 172, 186, 255});
    for (int i = 0; i < 3; i++) { // three phone screens, each with a place
      const Rectangle phone = {204.0f + i * 112, 222, 84, 200};
      DrawRectangleRounded(phone, 0.12f, 6, {255, 255, 255, 255});
      DrawRectangleRoundedLinesEx(phone, 0.12f, 6, 1, {200, 202, 214, 255});
      DrawCircle((int)(phone.x + 42), (int)phone.y + 40, 14,
                 i == 1 ? Color{250, 196, 120, 255} : Color{200, 220, 250, 255});
      DrawRectangle((int)phone.x + 14, (int)phone.y + 66, 56, 6,
                    {200, 202, 214, 255});
      const Rectangle slot = Slot(i);
      if (i >= placed || cardMove[i] < 1)
        DrawRectangleRoundedLinesEx(slot, 0.3f, 6, 1.5f, {170, 180, 230, 255});
      if (i < placed) { // the card, sliding up into its place
        const Rectangle from = CardRect();
        const float t = Between(cardMove[i], 0, 1);
        const Rectangle c = {from.x + (slot.x - from.x) * t,
                             from.y + (slot.y - from.y) * t,
                             from.width + (slot.width - from.width) * t,
                             from.height + (slot.height - from.height) * t};
        DrawRectangleRounded(c, 0.3f, 6, {90, 110, 220, 255});
        DrawRectangle((int)c.x + 10, (int)c.y + 14, 30, 6, {255, 255, 255, 200});
      }
    }
    if (placed < 3) { // the next card, waiting in the tray
      const Rectangle c = CardRect();
      DrawRectangleRounded(c, 0.3f, 6, {90, 110, 220, 255});
      DrawRectangle((int)c.x + 8, (int)c.y + 12, 26, 5, {255, 255, 255, 200});
      if (placed == 0)
        DrawRing({c.x + c.width / 2, c.y + c.height / 2}, 30, 33, 0, 360, 32,
                 Faded({90, 110, 220, 255}, 0.5f + 0.4f * std::sin(time * 4)));
    }
    DrawRectangleRounded({110, 466, 500, 18}, 0.5f, 4, {70, 72, 80, 255});
    DrawRectangle(300, 468, 120, 4, {100, 102, 110, 255});
  }
  void DrawPencil(Vector2 at, float a) const {
    const float bob = 6 * std::sin(time * 3);
    const Vector2 tip = {at.x, at.y + bob};
    const Vector2 end = {at.x + 70, at.y - 90 + bob};
    DrawLineEx(tip, end, 12, Faded({240, 190, 60, 255}, a));
    DrawLineEx({tip.x, tip.y}, {tip.x + 9, tip.y - 12}, 12,
               Faded({236, 206, 160, 255}, a));
    DrawCircleV(tip, 2.5f, Faded({70, 66, 80, 255}, a));
    DrawLineEx({end.x - 6, end.y + 8}, end, 12, Faded({230, 120, 140, 255}, a));
  }

  // A line of points from the big note, shrunk onto the small one
  static void DrawFlowerPath(const std::vector<Vector2> &line, Vector2 at,
                             float scale, float a) {
    const Rectangle pad = NotePad();
    const Vector2 c = {pad.x + pad.width / 2, pad.y + pad.height * 0.42f};
    for (size_t i = 1; i < line.size(); i++)
      DrawLineEx({at.x + (line[i - 1].x - c.x) * scale,
                  at.y + (line[i - 1].y - c.y) * scale},
                 {at.x + (line[i].x - c.x) * scale, at.y + (line[i].y - c.y) * scale},
                 1.3f, Faded({70, 66, 80, 255}, a));
  }

public:
  // Her flat, as on the first night, but warm: the lamp on, the city's lights
  // gold, Amma's tiffin and the tin of rice flour on the desk
  static void DrawWarmFlat(float time) {
    const Rectangle all = FullScreen();
    DrawBandGradient(0, 300, {84, 62, 62, 255}, {104, 78, 70, 255});
    const Rectangle window = {all.x + 40, 40, 360, 230};
    DrawRectangleGradientV((int)window.x, (int)window.y, (int)window.width,
                           (int)window.height, {30, 34, 70, 255},
                           {90, 70, 100, 255});
    for (int i = 0; i < 9; i++) { // the city, its windows lit
      const float x = window.x + i * 40, h = 60 + (i * 53 % 110);
      DrawRectangle((int)x, (int)(window.y + window.height - h), 36, (int)h,
                    {44, 44, 72, 255});
      for (int w = 0; w < 8; w++)
        if ((i * 5 + w * 3) % 3 == 0)
          DrawRectangle((int)x + 6 + (w % 3) * 10,
                        (int)(window.y + window.height - h + 8 + (w / 3) * 16),
                        5, 7, {255, 214, 130, 255});
    }
    DrawCircle((int)window.x + 300, (int)window.y + 40, 14, {250, 240, 210, 255});
    DrawRectangleLinesEx(window, 8, {60, 44, 40, 255});
    DrawLineEx({window.x + window.width / 2, window.y},
               {window.x + window.width / 2, window.y + window.height}, 6,
               {60, 44, 40, 255});
    // her sketches, pinned up; and now a small kolam among them, in pencil
    const Color PINNED[] = {{250, 240, 220, 255},
                            {220, 236, 250, 255},
                            {250, 226, 226, 255},
                            {236, 250, 226, 255}};
    for (int i = 0; i < 4; i++) {
      const Rectangle r = {730.0f + i * 70, 60.0f + (i % 2) * 30, 60, 80};
      DrawRectangleRec(r, PINNED[i]);
      DrawCircle((int)(r.x + r.width / 2), (int)r.y + 6, 3, KUMKUM);
      if (i == 2) { // the kolam
        for (int d = 0; d < 4; d++) {
          const Vector2 p = {r.x + 20 + (d % 2) * 20, r.y + 28 + (d / 2) * 20};
          DrawCircleV(p, 1.5f, {60, 60, 90, 255});
          DrawRing(p, 7, 8.2f, 0, 360, 14, {60, 60, 90, 255});
        }
        continue;
      }
      DrawRectangle((int)r.x + 10, (int)r.y + 20, 40, 24,
                    Faded({90, 110, 220, 255}, 0.4f));
      DrawRectangle((int)r.x + 10, (int)r.y + 52, 30, 4, {150, 150, 160, 255});
    }
    for (int i = 0; i < 3; i++)
      DrawRectangle(750 + i * 52, 190 + (i % 2) * 10, 36, 36,
                    {250, 220, 100, 255});
    // the desk, in warm lamp light
    DrawBandGradient(300, 420, {112, 80, 58, 255}, {70, 50, 38, 255});
    BeginBlendMode(BLEND_ADDITIVE);
    DrawCircleGradient({620, 330}, 420, {255, 190, 110, 60}, {255, 190, 110, 0});
    EndBlendMode();
    DrawCircleGradient({720, 340}, 300, {255, 210, 140, 90}, {255, 210, 140, 0});
    DrawLineEx({880, 470}, {850, 340}, 6, {50, 50, 56, 255});
    DrawLineEx({850, 340}, {800, 300}, 6, {50, 50, 56, 255});
    DrawTriangle({780, 290}, {760, 330}, {830, 318}, {60, 60, 66, 255});
    DrawCircleGradient({790, 330}, 30, {255, 236, 180, 200}, {255, 236, 180, 0});
    DrawEllipse(880, 472, 30, 8, {50, 50, 56, 255});
    // tea, steaming; the money plant
    DrawRectangle(100, 420, 34, 46, {220, 200, 170, 200});
    DrawRectangle(104, 432, 26, 32, {176, 120, 76, 230});
    for (int i = 0; i < 2; i++) {
      const float t = std::fmod(time * 0.5f + i * 0.5f, 1.0f);
      DrawCircle((int)(116 + 6 * std::sin(t * 6 + i)), (int)(410 - t * 50),
                 5 + t * 6, Faded(FLOUR, 0.25f * (1 - t)));
    }
    DrawRectangle((int)all.x + 60, 400, 30, 70, {120, 190, 160, 160});
    for (int i = 0; i < 6; i++)
      DrawEllipse((int)(all.x + 75 + 22 * std::sin(i * 2.1f)), 390 - i * 14,
                  12, 7, {70, 150, 80, 255});
  }

  // Amma's tiffin and the rice flour tin, and the phone, on the desk
  static void DrawDeskThings(float time, bool phone = true) {
    (void)time;
    DrawTiffin({70, 560}, 1.3f);
    DrawSmallTin({150, 580}, 1.4f);
    if (!phone) // (it's in her hand)
      return;
    DrawRectangleRounded(PHONE_9, 0.2f, 6, {30, 30, 36, 255});
    DrawRectangleRounded({PHONE_9.x + 4, PHONE_9.y + 6, PHONE_9.width - 8,
                          PHONE_9.height - 12},
                         0.15f, 6, {44, 48, 60, 255});
  }
};

//------------------------------------------------------------------------------
// A call: this time Anu calls first
//------------------------------------------------------------------------------
const Rectangle CALL_PHONE = {250, 110, 220, 420};

class CallScene : public Scene {
public:
  explicit CallScene(const Step &s) : step(s) {}

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    PlayAmbience("city_night", stage == END ? 0.1f : 0.25f);
    const Vector2 m = GetMousePosition();
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    switch (stage) {
    case DESK: // the phone on the desk: pick it up
      if (stageTime > 1.0f && clicked &&
          CheckCollisionPointRec(m, {PHONE_9.x - 20, PHONE_9.y - 20,
                                     PHONE_9.width + 40, PHONE_9.height + 40})) {
        PlayEffect("click", 0.5f);
        Next(CONTACTS);
      }
      break;
    case CONTACTS: // her contacts: Amma at the top
      if (stageTime > 0.5f && clicked && CheckCollisionPointRec(m, AmmaRow())) {
        PlayEffect("click", 0.6f);
        Next(RINGING);
      }
      break;
    case RINGING:
      if (stageTime > 2.2f) {
        talk.Start(step.lines);
        Next(TALKING);
      }
      break;
    case TALKING:
      if (talk.Update())
        Next(END);
      break;
    case END:
      if (stageTime > 7 || (stageTime > 4 && ContinuePressed()))
        done = true;
      break;
    }
  }

  void Draw() override {
    HyderabadScene::DrawWarmFlat(time);
    // the laptop, shut now, the sticky note with its flower on it
    DrawRectangleRounded({150, 440, 420, 30}, 0.3f, 4, {60, 62, 70, 255});
    DrawRectangleRounded({110, 466, 500, 18}, 0.5f, 4, {70, 72, 80, 255});
    const Rectangle note = {420, 420, 44, 44};
    DrawRectangleRec(note, {250, 222, 100, 255});
    DrawLittleFlower({note.x + 22, note.y + 18}, 6, {70, 66, 80, 255}, 1.4f);
    DrawLineEx({note.x + 22, note.y + 24}, {note.x + 22, note.y + 40}, 1.4f,
               {70, 66, 80, 255});
    HyderabadScene::DrawDeskThings(time, stage == DESK);
    const float left = FullScreen().x + 30;
    DrawString(stage >= TALKING ? "11:52 PM" : "11:51 PM", left, 24, 30, FLOUR);
    DrawString("Hyderabad", left, 58, 18, MUTED);
    if (stage == DESK) {
      DrawHint({PHONE_9.x + PHONE_9.width / 2, PHONE_9.y + PHONE_9.height / 2},
               48, time);
      return;
    }
    // the phone, up close
    const float rise = stage == CONTACTS ? Between(stageTime, 0, 0.35f) : 1;
    const float down = stage == END ? Between(stageTime, 0.5f, 1.5f) : 0;
    rlPushMatrix();
    rlTranslatef(0, (1 - rise) * 500 + down * 600, 0);
    DrawPhone();
    rlPopMatrix();
    talk.Draw([](const std::string &who) {
      if (who == "Anu")
        return BubbleSpot{{360, 620}, {360, 760}};
      return BubbleSpot{{560, 120}, {440, 190}};
    });
    if (stage == END) { // she sits back; the night goes quiet
      FadeScreen({20, 18, 30, 255}, 0.75f * Between(stageTime, 1.5f, 4));
      const float a = Between(stageTime, 2.5f, 4);
      DrawLittleFlower({360, 330}, 22, Faded({250, 222, 100, 255}, a), 4);
      DrawLineEx({360, 352}, {360, 400}, 4, Faded({250, 222, 100, 255}, a));
    }
  }

private:
  enum Stage { DESK, CONTACTS, RINGING, TALKING, END };
  const Step &step;
  Conversation talk;
  Stage stage = DESK;
  float time = 0, stageTime = 0;

  void Next(Stage s) {
    stage = s;
    stageTime = 0;
  }

  static Rectangle AmmaRow() {
    return {CALL_PHONE.x + 16, CALL_PHONE.y + 110, CALL_PHONE.width - 32, 56};
  }

  void DrawPhone() const {
    const Rectangle p = CALL_PHONE;
    DrawRectangleRounded({p.x + 6, p.y + 8, p.width, p.height}, 0.12f, 8,
                         {0, 0, 0, 90});
    DrawRectangleRounded(p, 0.12f, 8, {24, 24, 30, 255});
    const Rectangle screen = {p.x + 10, p.y + 12, p.width - 20, p.height - 24};
    DrawRectangleRounded(screen, 0.1f, 8, {246, 246, 250, 255});
    const Color green = {40, 170, 90, 255};
    if (stage == CONTACTS) { // recent calls: Amma's three missed calls, still
      const Rectangle row = AmmaRow();
      DrawRectangleRounded(row, 0.3f, 6, {230, 246, 236, 255});
      DrawCircle((int)row.x + 26, (int)(row.y + row.height / 2), 16, green);
      DrawString("A", row.x + 20, row.y + 16, 20, WHITE);
      DrawString("Amma", row.x + 52, row.y + 16, 22, {30, 30, 40, 255});
      DrawCircleV({row.x + row.width - 24, row.y + row.height / 2}, 14, green);
      DrawHint({row.x + row.width - 24, row.y + row.height / 2}, 18, time,
               green);
      const char *others[] = {"Rohit", "Priya", "Meena"};
      for (int i = 0; i < 3; i++) {
        const float y = row.y + 70 + i * 50;
        DrawCircle((int)row.x + 26, (int)y + 20, 14, {190, 192, 206, 255});
        DrawString(others[i], row.x + 52, y + 8, 18, {120, 120, 130, 255});
      }
      return;
    }
    // calling, then on the call: Amma's name, the time ticking
    DrawCircle((int)(p.x + p.width / 2), (int)p.y + 110, 40, green);
    DrawTextCentered("A", p.x + p.width / 2, p.y + 92, 36, WHITE);
    DrawTextCentered("Amma", p.x + p.width / 2, p.y + 160, 28, {30, 30, 40, 255});
    if (stage == RINGING) {
      for (int k = 0; k < 3; k++) {
        const float t = std::fmod(time * 0.8f + k / 3.0f, 1.0f);
        DrawRing({p.x + p.width / 2, p.y + 110}, 40 + t * 40, 42 + t * 40, 0,
                 360, 32, Faded(green, 1 - t));
      }
    } else {
      const int secs = (int)(stage == TALKING ? stageTime : 30);
      DrawTextCentered(TextFormat("%d:%02d", secs / 60, secs % 60),
                       p.x + p.width / 2, p.y + 196, 20, {120, 120, 130, 255});
    }
    DrawCircleV({p.x + p.width / 2, p.y + p.height - 60}, 24,
                {220, 70, 60, 255});
  }
};

std::unique_ptr<Scene> MakePackBag(const Step &step) {
  return std::unique_ptr<Scene>(new PackBagScene(step));
}
std::unique_ptr<Scene> MakeHyderabadBack(const Step &step) {
  return std::unique_ptr<Scene>(new HyderabadScene(step));
}
std::unique_ptr<Scene> MakeCallAmma(const Step &step) {
  return std::unique_ptr<Scene>(new CallScene(step));
}

} // namespace

PLAY_SCENE("pack_bag", MakePackBag)
PLAY_SCENE("hyderabad_back", MakeHyderabadBack)
PLAY_SCENE("call_amma", MakeCallAmma)
