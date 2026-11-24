// minigames.cpp - Hyderabad (notifications), the night bus home, Appa's kite
// (a memory), and filter coffee for Amma at dawn.
#include <cmath>

#include "game.h"

//------------------------------------------------------------------------------
// Busy: Hyderabad, 11:47 pm. Anu is a designer and good at it: work means
// clicking a card on the screen she is designing, which then wants to be
// somewhere else, again and again, while messages pile up beside it. Amma's
// calls arrive in the same pile. Nothing here is bad; she was just busy when
// Amma called. At midnight, or as soon as Amma's voice message is tapped,
// everything else falls away and only the voice message is left.
//------------------------------------------------------------------------------
struct Notification {
  std::string from;
  std::string text;
  Color color;
  bool fromAmma = false;
  bool voiceMessage = false; // Amma's voice message
  float y = -60;             // slides down into place from above the screen
};

const float BUSY_SECONDS = 20; // short, so a playtest of the demo is quick
const Rectangle LAPTOP_SCREEN = {166, 186, 388, 268};
const Color CHAT_PURPLE = {120, 80, 200, 255}; // the work chat app
const Color AMMA_GREEN = {40, 170, 90, 255};

class BusyScene : public Scene {
public:
  BusyScene() { colour = 0.6f; } // a little muted, not grey

  void Update() override {
    const float dt = GetFrameTime();
    if (finished) {
      PlayAmbience("city_night", 0.25f); // the room goes quiet
      endTimer += dt;
      if (endTimer > 1.0f && (Clicked(VOICE_MESSAGE) || ContinuePressed()))
        done = true;
      return;
    }

    time += dt;
    PlayAmbience("city_night", 0.5f);
    spawnTimer -= dt;
    if (spawnTimer <= 0) {
      Spawn();
      spawnTimer = std::fmax(0.4f, 1.2f - time * 0.04f); // faster and faster
    }
    for (size_t i = 0; i < pile.size(); i++)
      pile[i].y += (70 + i * 64 - pile[i].y) * std::fmin(1.0f, dt * 12);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      const Vector2 mouse = GetMousePosition();
      bool clickedNotification = false;
      for (size_t i = 0; i < pile.size(); i++) {
        if (!CheckCollisionPointRec(mouse, NotificationRect(i)))
          continue;
        clickedNotification = true;
        if (pile[i].fromAmma && !pile[i].voiceMessage)
          ammaSwiped++;             // it leaves a mark (see Draw)
        if (pile[i].voiceMessage) { // she stops
          finished = true;
          return;
        }
        pile.erase(pile.begin() + i);
        break;
      }
      // Work: line up the box on the slide. Then it wants to go somewhere
      // else. And again. It never ends.
      if (!clickedNotification && CheckCollisionPointRec(mouse, WorkBox())) {
        PlayEffect("click", 0.6f, 0.9f + GetRandomValue(0, 20) / 100.0f);
        workBox = (workBox + 1 + GetRandomValue(0, 4)) % 6;
        boxesMoved++;
      }
    }
    if (time >= BUSY_SECONDS)
      finished = true;
  }

  void Draw() override {
    DrawRoom();

    // Her laptop, open on the app she is designing
    DrawRectangleRounded({150, 170, 420, 300}, 0.03f, 4, {36, 38, 44, 255});
    DrawRectangleRec(LAPTOP_SCREEN, {246, 246, 250, 255});
    DrawRectangle((int)LAPTOP_SCREEN.x, (int)LAPTOP_SCREEN.y,
                  (int)LAPTOP_SCREEN.width, 24, {226, 228, 236, 255});
    DrawString(TextFormat("Onboarding  v%d", 7 + boxesMoved / 4), 176, 189, 16,
               {60, 60, 80, 255});
    DrawRectangle((int)LAPTOP_SCREEN.x, 210, 26, 244, {232, 234, 240, 255});
    for (int i = 0; i < 5; i++) // the tool bar
      DrawRectangleRounded({LAPTOP_SCREEN.x + 6, 222.0f + i * 26, 14, 14}, 0.3f,
                           4, {170, 172, 186, 255});
    for (int i = 0; i < 3; i++) { // three phone screens she is working on
      const Rectangle phone = {204.0f + i * 112, 222, 84, 220};
      DrawRectangleRounded(phone, 0.12f, 6, {255, 255, 255, 255});
      DrawRectangleRoundedLinesEx(phone, 0.12f, 6, 1, {200, 202, 214, 255});
      DrawCircle((int)(phone.x + 42), (int)phone.y + 40, 14,
                 i == 1 ? Color{250, 196, 120, 255}
                        : Color{200, 220, 250, 255});
      DrawRectangle((int)phone.x + 14, (int)phone.y + 66, 56, 6,
                    {200, 202, 214, 255});
      DrawRectangle((int)phone.x + 20, (int)phone.y + 78, 44, 5,
                    {220, 222, 230, 255});
    }
    // the card that keeps needing to move, with a soft glow where it goes
    const Rectangle box = WorkBox();
    DrawRectangleRounded(box, 0.3f, 6, {90, 110, 220, 255});
    DrawRectangle((int)box.x + 10, (int)box.y + 14, 40, 6,
                  {255, 255, 255, 200});
    if (boxesMoved == 0) // "click here", until the first time
      DrawRing({box.x + box.width / 2, box.y + box.height / 2}, 30, 33, 0, 360,
               32,
               Faded({90, 110, 220, 255}, 0.5f + 0.4f * std::sin(time * 4)));
    // each call from Amma she swiped away: a small green mark that stays
    for (int i = 0; i < ammaSwiped; i++)
      DrawCircle(530 - i * 16, 440, 5, AMMA_GREEN);
    DrawRectangleRounded({110, 466, 500, 18}, 0.5f, 4, {70, 72, 80, 255});
    DrawRectangle(300, 468, 120, 4, {100, 102, 110, 255});

    // Clock: 11:47 -> 11:59
    const int minute =
        47 + (int)(12 * std::fmin(time, BUSY_SECONDS) / BUSY_SECONDS);
    const float left = FullScreen().x + 30; // the screen's top-left corner
    DrawString(TextFormat("11:%02d PM", minute), left, 24, 30, FLOUR);
    DrawString("Hyderabad", left, 58, 18, MUTED);

    for (size_t i = 0; i < pile.size(); i++) {
      const Rectangle r = NotificationRect(i);
      DrawRectangleRounded({r.x + 3, r.y + 4, r.width, r.height}, 0.2f, 6,
                           {0, 0, 0, 50});
      DrawRectangleRounded(r, 0.2f, 6, {250, 250, 252, 245});
      DrawCircle((int)r.x + 24, (int)r.y + 28, 12, pile[i].color);
      DrawString(pile[i].from, r.x + 46, r.y + 6, 17, {30, 30, 40, 255});
      DrawString(pile[i].text, r.x + 46, r.y + 28, 16, {90, 90, 100, 255});
    }

    if (finished) { // the room goes dark; only Amma's voice message is left
      DrawBand(0, 720, Faded(BLACK, std::fmin(0.85f, endTimer)));
      const Color white = Faded(FLOUR, endTimer - 0.5f);
      DrawRectangleRounded(VOICE_MESSAGE, 0.3f, 8,
                           Faded(FLOUR, (endTimer - 0.5f) * 0.12f));
      DrawCircle(250, 360, 22, Faded(AMMA_GREEN, endTimer - 0.5f));
      DrawTriangle({243, 349}, {243, 371}, {262, 360}, white); // play
      DrawString("Amma", 290, 334, 20, white);
      for (int i = 0; i < 24; i++) { // the voice message's waveform
        const float h = 4 + 14 * std::fabs(std::sin(i * 1.7f));
        DrawRectangle(290 + i * 9, (int)(372 - h / 2), 5, (int)h,
                      Faded(MUTED, endTimer - 0.5f));
      }
      DrawString("0:42", 515, 364, 15, Faded(MUTED, endTimer - 0.5f));
    }
  }

private:
  std::vector<Notification> pile; // newest first
  float time = 0;
  float spawnTimer = 0.5f;
  int workBox = 0; // where the card on the screen is (one of 6 spots)
  int boxesMoved = 0;
  int ammaSwiped = 0; // Amma's missed calls swiped away
  int ammaCalls = 0;  // how many times Amma has called
  bool finished = false;
  float endTimer = 0;
  const Rectangle VOICE_MESSAGE = {210, 320, 350, 80};

  Rectangle WorkBox() const {
    return {200.0f + (workBox % 3) * 110, 380.0f - (workBox / 3) * 150, 60, 36};
  }

  // Her flat at night: the city through a big window, sketches and notes
  // pinned on the wall, a warm desk lamp, tea, a plant in a bottle
  void DrawRoom() const {
    const Rectangle all = FullScreen();
    DrawBandGradient(0, 300, {52, 46, 58, 255}, {70, 60, 66, 255});
    const Rectangle window = {all.x + 40, 40, 360, 230};
    DrawRectangleGradientV((int)window.x, (int)window.y, (int)window.width,
                           (int)window.height, {22, 30, 58, 255},
                           {60, 56, 96, 255});
    for (int i = 0; i < 9; i++) { // the city, its windows lit
      const float x = window.x + i * 40, h = 60 + (i * 53 % 110);
      DrawRectangle((int)x, (int)(window.y + window.height - h), 36, (int)h,
                    {34, 40, 66, 255});
      for (int w = 0; w < 8; w++)
        if ((i * 5 + w * 3) % 4 == 0)
          DrawRectangle((int)x + 6 + (w % 3) * 10,
                        (int)(window.y + window.height - h + 8 + (w / 3) * 16),
                        5, 7, {240, 206, 130, 255});
    }
    DrawCircle((int)window.x + 300, (int)window.y + 40, 14,
               {240, 236, 220, 255}); // the moon
    DrawRectangleLinesEx(window, 8, {40, 34, 40, 255});
    DrawLineEx({window.x + window.width / 2, window.y},
               {window.x + window.width / 2, window.y + window.height}, 6,
               {40, 34, 40, 255});
    // her sketches and printouts, pinned up, and sticky notes
    const Color PINNED[] = {{250, 240, 220, 255},
                            {220, 236, 250, 255},
                            {250, 226, 226, 255},
                            {236, 250, 226, 255}};
    for (int i = 0; i < 4; i++) {
      const Rectangle r = {730.0f + i * 70, 60.0f + (i % 2) * 30, 60, 80};
      DrawRectangleRec(r, PINNED[i]);
      DrawCircle((int)(r.x + r.width / 2), (int)r.y + 6, 3, KUMKUM);
      DrawRectangle((int)r.x + 10, (int)r.y + 20, 40, 24,
                    Faded({90, 110, 220, 255}, 0.4f));
      DrawRectangle((int)r.x + 10, (int)r.y + 52, 30, 4, {150, 150, 160, 255});
    }
    for (int i = 0; i < 3; i++)
      DrawRectangle(750 + i * 52, 190 + (i % 2) * 10, 36, 36,
                    {250, 220, 100, 255});
    // the desk, in lamp light
    DrawBandGradient(300, 420, {92, 66, 50, 255}, {54, 40, 32, 255});
    DrawCircleGradient({720, 340}, 300, {255, 210, 140, 70},
                       {255, 210, 140, 0});
    // the desk lamp
    DrawLineEx({880, 470}, {850, 340}, 6, {50, 50, 56, 255});
    DrawLineEx({850, 340}, {800, 300}, 6, {50, 50, 56, 255});
    DrawTriangle({780, 290}, {760, 330}, {830, 318}, {60, 60, 66, 255});
    DrawEllipse(880, 472, 30, 8, {50, 50, 56, 255});
    // tea in a glass, steaming; a money plant in a bottle
    DrawRectangle(100, 420, 34, 46, {220, 200, 170, 200});
    DrawRectangle(104, 432, 26, 32, {176, 120, 76, 230});
    for (int i = 0; i < 2; i++) {
      const float t = std::fmod(time * 0.5f + i * 0.5f, 1.0f);
      DrawCircle((int)(116 + 6 * std::sin(t * 6 + i)), (int)(410 - t * 50),
                 5 + t * 6, Faded(FLOUR, 0.25f * (1 - t)));
    }
    DrawRectangle(all.x + 60, 400, 30, 70, {120, 190, 160, 160});
    for (int i = 0; i < 6; i++)
      DrawEllipse((int)(all.x + 75 + 22 * std::sin(i * 2.1f)), 390 - i * 14, 12,
                  7, {70, 150, 80, 255});
  }

  // Beside her laptop, just over its right edge. A square screen has no room
  // beside it, so there they sit further over the laptop.
  Rectangle NotificationRect(size_t i) const {
    const float width = 268;
    const Rectangle all = FullScreen();
    const float x = std::fmin(530, all.x + all.width - width - 12);
    return {x, pile[i].y, width, 56};
  }

  void Spawn() {
    static const std::vector<Notification> WORK = {
        {"Rohit (manager)", "v7 looks great. tiny fix on icons?", CHAT_PURPLE},
        {"#design", "@anu the new onboarding is so clean", CHAT_PURPLE},
        {"Priya", "can I steal your colours for mine", CHAT_PURPLE},
        {"Rohit (manager)", "also the font feels a bit off", CHAT_PURPLE},
        {"Calendar", "Design review, 10:00 AM", {60, 140, 220, 255}},
        {"Mail", "Re: Final screens (final)", {220, 80, 60, 255}},
        {"Design app", "3 new comments on Onboarding", {230, 110, 60, 255}},
        {"#design", "who took the last samosa", CHAT_PURPLE},
        {"Tasks", "2 tasks due tomorrow", {40, 110, 220, 255}},
        {"Food app", "Your biryani is 2 min away!", {240, 120, 40, 255}},
    };

    Notification n = WORK[GetRandomValue(0, (int)WORK.size() - 1)];
    // Amma calls three times; the third time she leaves a voice message
    if ((ammaCalls == 0 && time > 3) || (ammaCalls == 1 && time > 8)) {
      n = {"Amma", "Missed call", AMMA_GREEN, true};
      ammaCalls++;
    } else if (ammaCalls == 2 && time > 13) {
      n = {"Amma", "Voice message (0:42)", AMMA_GREEN, true, true};
      ammaCalls++;
    }
    if (n.fromAmma)
      PlayEffect("vibrate", 0.8f);
    else
      PlayEffect("notify", 0.25f, 0.9f + GetRandomValue(0, 20) / 100.0f);
    pile.insert(pile.begin(), n);
    if (pile.size() > 9)
      pile.pop_back(); // the oldest fall off the bottom, unread
  }
};

//------------------------------------------------------------------------------
// Kite: a memory of Pongal when Anu was eight, on the terrace with Appa.
// Hold the mouse button (or Space / Up) to pull the string, let go to give
// slack; move the mouse (or Left / Right) to steer. Pulling too hard makes
// the kite wobble and dive. Letting go just as a gust comes lifts it. Keep the
// kite up in the high wind for KITE_GOAL_SECONDS.
//------------------------------------------------------------------------------
const float KITE_GOAL_SECONDS = 7;
const float HIGH_WIND_Y = 210; // above this line the wind is strong and steady
const float GUST = 1.4f;       // wind stronger than this is a gust

class KiteScene : public Scene { // a memory, so in full colour (the default)
public:
  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    if (won) {
      wonTimer += dt;
      if (wonTimer > 2 && ContinuePressed()) {
        AddMemory(KITE_MEMORY);
        done = true;
      }
      return;
    }

    const float wind = Wind();
    PlayAmbience("wind", 0.35f + 0.25f * std::fmax(0, wind - 1));
    const bool wasPulling = pulling;
    pulling = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE) ||
              IsKeyDown(KEY_UP);
    if (wasPulling && !pulling && wind > GUST && tangled <= 0) {
      velocityY -= 150; // let go into a gust: it rises
      lift = 0.8f;
      PlayEffect("gust", 0.7f);
    }
    lift = std::fmax(0, lift - dt);
    // steer with the mouse, or with the arrow keys
    const Vector2 mouseDelta = GetMouseDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0)
      steerX = GetMousePosition().x;
    if (IsKeyDown(KEY_LEFT))
      steerX -= 300 * dt;
    if (IsKeyDown(KEY_RIGHT))
      steerX += 300 * dt;
    steerX = std::fmax(80, std::fmin(660, steerX));
    if (tangled > 0) { // stuck in the coconut tree: it slides back down
      tangled -= dt;
      kite.y += 120 * dt;
      pulling = false;
    } else if (pulling) {
      velocityY -= 260 * wind * dt;
      // pulling hard in a gust strains the string
      tension += (wind > 1.5f ? 0.9f : 0.25f) * dt;
    } else {
      velocityY += 90 * dt; // slack: it sinks slowly
      tension -= 0.6f * dt;
    }
    tension = std::fmax(0, tension);
    if (tension > 1) { // pulled too hard
      velocityY = 260;
      tension = 0;
      Say("Gently, kanna.");
    }

    velocityY *= 1 - 2.5f * dt; // air
    kite.y += velocityY * dt;
    // steer towards the mouse, and drift with the wind
    kite.x += ((steerX - kite.x) * 0.8f + 40 * (wind - 1)) * dt;
    kite.x = std::fmax(80, std::fmin(660, kite.x));
    kite.y = std::fmax(60, std::fmin(560, kite.y));

    // The coconut tree on the right catches careless kites
    if (tangled <= 0 && kite.x > 560 && kite.y > 330 && kite.y < 470) {
      tangled = 1.2f;
      Say("Aiyo, the coconut tree!");
    }

    if (kite.y < HIGH_WIND_Y)
      high += dt;
    const float highEnough = high / KITE_GOAL_SECONDS; // 0 to 1
    if (encouraged == 0 && highEnough > 0.3f) {
      Say("That's it.");
      encouraged = 1;
    }
    if (encouraged == 1 && highEnough > 0.65f) {
      Say("Higher than Selvi's!");
      encouraged = 2;
    }
    if (highEnough >= 1) {
      won = true;
      Say("See? It flies by itself now.");
    }
    if (sayTimer > 0)
      sayTimer -= dt;
  }

  void Draw() override {
    // Morning sky, sun, clouds, other kites
    DrawBandGradient(0, 720, {110, 180, 240, 255}, {250, 225, 180, 255});
    DrawCircle(600, 110, 46, {255, 236, 160, 255});
    for (int i = 0; i < 4; i++) {
      const float x = std::fmod(i * 230 + time * (12 + i * 4), 900.0f) - 120;
      DrawEllipse((int)x, 90 + i * 55, 70, 20, {255, 255, 255, 200});
      DrawEllipse((int)x + 40, 80 + i * 55, 50, 18, {255, 255, 255, 200});
    }
    const Color OTHER_KITES[] = {
        {230, 70, 90, 255}, {60, 160, 90, 255}, {250, 180, 40, 255}};
    for (int i = 0; i < 3; i++) {
      const Vector2 p = {150.0f + i * 190,
                         150.0f + i * 40 + 12 * std::sin(time * 1.3f + i)};
      DrawKite(p, 14, OTHER_KITES[i]);
    }
    DrawBand(HIGH_WIND_Y, 1, {255, 255, 255, 70});
    // a gust: streaks of wind across the sky
    const float gust = std::fmax(0, (Wind() - GUST) * 3);
    for (int i = 0; i < 6; i++) {
      const float x = std::fmod(i * 170 + time * 420, 1100.0f) - 250;
      const float y = 90.0f + i * 50;
      DrawLineEx({x, y}, {x + 90, y - 4}, 2,
                 Faded({255, 255, 255, 255}, 0.5f * std::fmin(1.0f, gust)));
    }

    // The blue hills far off, the village's palms, and the coconut tree by
    // the terrace that catches careless kites
    DrawHills(520, 600, 90, 2, {150, 172, 210, 255});
    DrawHills(555, 600, 50, 5, {120, 146, 190, 255});
    for (int i = 0; i < 7; i++)
      DrawPalm(FullScreen().x + 60 + i * 210, 600, 90, time, {120, 96, 80, 255},
               {80, 130, 90, 255});
    DrawPalm(629, 640, 280, time, {120, 84, 50, 255}, {50, 130, 60, 255});

    // Terrace, Appa and little Anu
    DrawBand(600, 120, {200, 120, 80, 255});
    DrawBand(590, 14, {225, 150, 105, 255});
    DrawAppa(120, 600, time);
    DrawLittleAnu(178, 600, time);
    const Vector2 reel = {192, 548}; // in little Anu's hands
    DrawCircle((int)reel.x, (int)reel.y, 9, {250, 220, 90, 255});

    // The string: tight when pulling, saggy when slack
    const Vector2 mid = {(reel.x + kite.x) / 2,
                         (reel.y + kite.y) / 2 + (pulling ? 10 : 70)};
    DrawSplineSegmentBezierQuadratic(reel, mid, kite, 1.5f,
                                     {255, 255, 255, 220});
    // pulled too hard, it wobbles
    const float wobble = tension > 0.7f ? 6 * std::sin(time * 40) : 0;
    DrawKite({kite.x + wobble, kite.y}, 24, {250, 120, 40, 255});
    if (lift > 0) // a lift from the gust
      DrawRing(kite, 34 + 30 * (0.8f - lift), 36 + 30 * (0.8f - lift), 0, 360,
               32, Faded({255, 255, 255, 255}, lift));
    if (tangled > 0)
      DrawString("tangled!", kite.x - 30, kite.y + 30, 18, KUMKUM);

    // How long it has stayed high
    DrawRectangleRounded({450, 24, 250, 18}, 1, 6, {255, 255, 255, 120});
    DrawRectangleRounded(
        {450, 24, 250 * std::fmin(1.0f, high / KITE_GOAL_SECONDS), 18}, 1, 6,
        {250, 120, 40, 255});

    // Tension
    DrawString("string", 30, 650, 16, {80, 40, 30, 255});
    DrawRectangle(90, 655, 150, 10, {120, 70, 50, 255});
    DrawRectangle(90, 655, (int)(150 * std::fmin(1.0f, tension)), 10,
                  tension > 0.7f ? KUMKUM : FLOUR);

    if (time < 6) {
      DrawTextCentered("Pongal, nineteen years ago.", 360, 300, 30,
                       {60, 40, 60, 255}, HAND_FONT);
      DrawTextCentered("Hold to pull. Let go as the wind gusts.", 360, 340, 17,
                       {60, 40, 60, 255});
    }
    if (sayTimer > 0 || won) {
      DrawRectangleRounded({20, 230, 300, 60}, 0.3f, 6, {255, 255, 255, 230});
      DrawString("Appa", 36, 234, 18, KUMKUM);
      DrawString(saying, 36, 256, 17, {40, 40, 60, 255});
    }
    if (won && wonTimer > 2)
      DrawTextCentered(std::string(tap()) + " to continue", 360, 690, 16,
                       {80, 40, 30, 255});
  }

private:
  Vector2 kite = {320, 480};
  float velocityY = 0;
  float tension = 0;
  float high = 0; // seconds spent above the strong-wind line
  float tangled = 0;
  float time = 0;
  bool pulling = false;
  bool won = false;
  float wonTimer = 0;
  std::string saying;
  float sayTimer = 0;
  int encouraged = 0; // how many encouragements Appa has given
  float steerX = 320; // where the kite is being steered to
  float lift = 0;     // above 0 just after letting go into a gust

  float Wind() const { // 1 on average, with gusts
    return 1.0f + 0.6f * std::sin(time * 0.9f) +
           0.4f * std::sin(time * 2.3f + 1);
  }

  void Say(const std::string &text) {
    saying = text;
    sayTimer = 3.5f;
  }

  static void DrawKite(Vector2 p, float size, Color color) {
    const Vector2 top = {p.x, p.y - size}, right = {p.x + size * 0.75f, p.y},
                  bottom = {p.x, p.y + size * 1.3f},
                  left = {p.x - size * 0.75f, p.y};
    DrawTriangle(top, left, right, color);
    DrawTriangle(left, bottom, right, Faded(color, 0.8f));
    const Vector2 tailEnd = {p.x - size * 0.3f, p.y + size * 2.2f};
    DrawLineEx(bottom, tailEnd, 2, color);
  }
};

std::unique_ptr<Scene> MakeBusyScene() {
  return std::unique_ptr<Scene>(new BusyScene());
}
std::unique_ptr<Scene> MakeKiteScene() {
  return std::unique_ptr<Scene>(new KiteScene());
}

//------------------------------------------------------------------------------
// Travel: the night bus from Hyderabad to Sathyamangalam, with no words at
// all. At dawn it passes Mettupalayam, under the Nilgiri hills.
// The window keeps fogging up: drag across it to wipe it, and watch the city
// turn into fields and the night into dawn. In the fog, kolam dots appear one
// by one, join into a kolam for a moment, and fade. Her phone lights up until
// the signal runs out; click it to turn it face down, if you like. At dawn,
// click to get off.
//------------------------------------------------------------------------------
const float JOURNEY_SECONDS = 25;
const Rectangle BUS_WINDOW = {70, 90, 580, 330};
const Rectangle PHONE = {455, 540, 80, 130};
const int FOG_COLUMNS = 24, FOG_ROWS = 14;
const float MOST_FOG = 0.6f; // how thick the fog gets (1 = can't see out)
const float HORIZON = 330;   // where the land meets the sky, through the window

class TravelScene : public Scene {
public:
  TravelScene() {
    colour = 0.5f;
    for (float &f : fog)
      f = MOST_FOG;
    // the kolam in the fog: the first page of Amma's book, solved
    KolamGrid grid;
    if (grid.Load(Pages()[0]) && grid.CountSolutions(true) > 0) {
      const float cell = 40;
      const Vector2 origin = {360 - cell * grid.width / 2,
                              230 - cell * grid.height / 2};
      fogKolam = LoopPaths(grid, origin, cell, 1.5f);
      for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++)
          fogDots.push_back(
              {origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell});
      }
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("bus", leaving > 0 ? 0 : 0.6f);
    if (leaving > 0) { // fading out as she gets off the bus
      leaving += dt;
      done = leaving > 1.5f;
      return;
    }

    // Wipe the fog where the mouse is dragged; it slowly comes back
    const Vector2 mouse = GetMousePosition();
    for (int y = 0; y < FOG_ROWS; y++) {
      for (int x = 0; x < FOG_COLUMNS; x++) {
        float &f = fog[y * FOG_COLUMNS + x];
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointCircle(mouse, FogCell(x, y), 44))
          f = 0;
        f = std::fmin(MOST_FOG, f + 0.05f * dt);
      }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mouse, PHONE))
      phoneDown = !phoneDown;
    buzzTimer -= dt;
    if (buzzTimer <= 0 && Signal() > 0) { // another notification
      buzz = 1.2f;
      if (!phoneDown)
        PlayEffect("notify", 0.15f);
      buzzTimer = 3.0f + GetRandomValue(0, 20) / 10.0f;
    }
    buzz = std::fmax(0, buzz - dt);

    // Night is muted; dawn brings the colour up
    colour = 0.5f + 0.35f * Between(Journey(), 0.6f, 1);

    if (Journey() >= 1 && ContinuePressed() &&
        !CheckCollisionPointRec(mouse, PHONE))
      leaving = 0.001f;
  }

  void Draw() override {
    const float j = Journey();
    DrawOutside(j);

    // Anu's reflection in the glass, only at night
    const float reflection = 0.12f * (1 - Between(j, 0.4f, 0.7f));
    DrawCircle(540, 230, 34, Faded(FLOUR, reflection));
    DrawRectangleRounded({500, 262, 80, 160}, 0.5f, 6,
                         Faded(FLOUR, reflection));

    // Fog on the glass
    const float cellW = BUS_WINDOW.width / FOG_COLUMNS,
                cellH = BUS_WINDOW.height / FOG_ROWS;
    for (int y = 0; y < FOG_ROWS; y++) {
      for (int x = 0; x < FOG_COLUMNS; x++) {
        const Vector2 c = FogCell(x, y);
        DrawRectangleRec({c.x - cellW / 2, c.y - cellH / 2, cellW, cellH},
                         Faded({200, 206, 218, 255}, fog[y * FOG_COLUMNS + x]));
      }
    }

    DrawFogKolam(j);

    DrawBusInside(j);
    DrawPhone();

    if (j >= 1 && leaving == 0) // the bus has stopped: a quiet "go on"
      DrawRing({660, 660}, 10, 13, 0, 360, 24,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 3)));
    if (leaving > 0)
      DrawBand(0, 720, Faded(FLOUR, leaving / 1.5f));
  }

private:
  float fog[FOG_COLUMNS * FOG_ROWS];
  float time = 0;
  float buzz = 0;         // the phone's screen is lit while this is above 0
  float buzzTimer = 2.0f; // until the next notification
  bool phoneDown = false; // turned face down
  float leaving = 0;      // above 0 once she gets off
  std::vector<std::vector<Vector2>> fogKolam; // the kolam her finger draws
  std::vector<Vector2> fogDots;

  // Kolam dots appear in the fog one by one, the line joins them for a
  // moment, then the fog takes it all back
  void DrawFogKolam(float j) const {
    const float fade = 1 - Between(j, 0.66f, 0.78f);
    if (fade <= 0)
      return;
    const Color finger = {16, 20, 36, 255}; // clear glass shows the dark
    for (size_t i = 0; i < fogDots.size(); i++) {
      const float appear = 0.28f + 0.16f * i / fogDots.size();
      DrawCircleV(
          fogDots[i], 4,
          Faded(finger, 0.75f * fade * Between(j, appear, appear + 0.02f)));
    }
    const float drawn = Between(j, 0.45f, 0.6f);
    for (const std::vector<Vector2> &loop : fogKolam)
      DrawPath(loop, (size_t)(drawn * loop.size()), drawn >= 1, 3,
               Faded(finger, 0.6f * fade));
  }

  float Journey() const { return std::fmin(1, time / JOURNEY_SECONDS); }

  // Signal bars: full in the city, gone by the middle of the night
  int Signal() const {
    return (int)std::ceil(4 * (1 - Between(Journey(), 0.2f, 0.6f)));
  }

  Vector2 FogCell(int x, int y) const {
    return {BUS_WINDOW.x + (x + 0.5f) * BUS_WINDOW.width / FOG_COLUMNS,
            BUS_WINDOW.y + (y + 0.5f) * BUS_WINDOW.height / FOG_ROWS};
  }

  // Through the window: night city, dark highway, fields, then the river at
  // dawn. Everything scrolls past; nearer things faster.
  void DrawOutside(float j) const {
    const float dawn = Between(j, 0.6f, 1);
    DrawBandGradient(0, HORIZON,
                     ColorLerp(NIGHT_TOP, {120, 150, 205, 255}, dawn),
                     ColorLerp(NIGHT_BOTTOM, {250, 190, 140, 255}, dawn));
    DrawCircle(560, 140, 18, Faded(FLOUR, 1 - dawn)); // the moon
    DrawCircle(200, (int)(HORIZON + 40 - 90 * Between(j, 0.75f, 1)), 34,
               {255, 214, 140, 255}); // the sun coming up

    // at dawn, past Mettupalayam: the blue Nilgiri hills, close now
    const float hills = Between(j, 0.55f, 0.8f);
    DrawHills(
        HORIZON - 40, HORIZON, 130, 7,
        Faded(ColorLerp({30, 34, 60, 255}, {110, 130, 180, 255}, dawn), hills));
    DrawHills(
        HORIZON - 10, HORIZON, 60, 9,
        Faded(ColorLerp({24, 28, 50, 255}, {90, 110, 160, 255}, dawn), hills));
    // the city, fading away behind her
    const float city = 1 - Between(j, 0.15f, 0.35f);
    for (int i = 0; i < 14; i++) {
      const float x = std::fmod(i * 70 + time * 90, 980.0f) - 130;
      const float h = 60 + (i * 37 % 110);
      DrawRectangle((int)x, (int)(HORIZON - h), 56, (int)h,
                    Faded({34, 40, 62, 255}, city));
      for (int w = 0; w < 4; w++) {
        if ((i + w) % 3 == 0)
          DrawRectangle((int)x + 10 + (w % 2) * 24,
                        (int)(HORIZON - h + 12 + (w / 2) * 22), 8, 10,
                        Faded({230, 200, 120, 255}, city));
      }
    }
    // land: dark at night, green-brown at dawn, the Bhavani shining near the
    // end
    DrawBand(HORIZON, 720 - HORIZON,
             ColorLerp({20, 22, 30, 255}, {96, 110, 70, 255}, dawn));
    DrawBand(
        HORIZON + 14, 6,
        Faded({255, 230, 190, 255},
              Between(j, 0.8f, 0.95f) * (0.6f + 0.3f * std::sin(time * 4))));
    // coconut palms in the fields
    const float fields = Between(j, 0.25f, 0.4f);
    const Color palm = ColorLerp({10, 12, 18, 255}, {40, 70, 40, 255}, dawn);
    for (int i = 0; i < 6; i++) {
      const float x = std::fmod(i * 160 + time * 160, 960.0f) - 120;
      const float top = HORIZON - 70 - (i * 23 % 40);
      DrawLineEx({x, HORIZON + 30}, {x + 6, top}, 5, Faded(palm, fields));
      for (int k = 0; k < 5; k++) {
        const float a = k * 1.25f;
        DrawLineEx({x + 6, top},
                   {x + 6 + 34 * std::cos(a), top + 12 - 18 * std::sin(a)}, 5,
                   Faded(palm, fields));
      }
    }
    // streetlights rushing past on the highway, at night
    const float lights = 1 - Between(j, 0.45f, 0.65f);
    const float lx = std::fmod(time * 520, 1300.0f) - 200;
    DrawLineEx({lx, 400}, {lx, 250}, 4, Faded({60, 60, 70, 255}, lights));
    DrawCircleV({lx, 250}, 30, Faded({255, 190, 100, 255}, lights * 0.25f));
    DrawCircleV({lx, 250}, 7, Faded({255, 210, 140, 255}, lights));
  }

  // The bus around the window, the seat in front, Anu's knees
  void DrawBusInside(float j) const {
    const Color wall =
        ColorLerp({26, 24, 32, 255}, {70, 62, 66, 255}, Between(j, 0.7f, 1));
    const Rectangle all = FullScreen();
    DrawRectangleRec({all.x, 0, all.width, BUS_WINDOW.y}, wall);
    DrawRectangleRec({all.x, BUS_WINDOW.y + BUS_WINDOW.height, all.width, 720},
                     wall);
    DrawRectangleRec({all.x, 0, BUS_WINDOW.x - all.x, 720}, wall);
    DrawRectangleRec({BUS_WINDOW.x + BUS_WINDOW.width, 0, 720, 720}, wall);
    DrawRectangleLinesEx(BUS_WINDOW, 8, {50, 48, 58, 255});
    for (int i = 0; i < 5; i++) { // the curtain, pushed to one side, pleated
      DrawRectangle(36 + i * 14, 80, 14, 350,
                    i % 2 ? Color{120, 44, 54, 255} : Color{96, 34, 44, 255});
      DrawCircle(43 + i * 14, 82, 4, {170, 170, 176, 255}); // rings on the rod
    }
    DrawRectangle(20, 76, 120, 4, {150, 150, 156, 255});
    // the seat in front: a high back with the white cloth buses put on it
    DrawRectangleRounded({90, 470, 300, 300}, 0.15f, 6, {70, 56, 92, 255});
    DrawRectangleRounded({100, 480, 280, 290}, 0.15f, 6, {82, 66, 106, 255});
    for (int i = 0; i < 6; i++) // stitched lines
      DrawLine(120 + i * 48, 560, 120 + i * 48, 720, {64, 50, 84, 255});
    DrawRectangleRounded({150, 462, 180, 70}, 0.2f, 6, {232, 228, 220, 255});
    for (int i = 0; i < 9; i++) // the cloth's fringe
      DrawLineEx({158.0f + i * 20, 532}, {160.0f + i * 20, 544}, 2,
                 {232, 228, 220, 255});
    DrawRectangle(180, 488, 120, 6, {200, 60, 70, 255}); // a red stripe
    // her lap: dupatta over her knees, her hands holding the phone
    DrawEllipse(470, 720, 170, 70, {214, 160, 58, 255});
    DrawEllipse(470, 708, 150, 40, {182, 128, 40, 255});
    DrawEllipse(450, 640, 26, 14, {176, 118, 80, 255});
    DrawEllipse(540, 640, 26, 14, {142, 90, 60, 255});
  }

  void DrawPhone() const {
    const float shake = buzz > 0.9f ? 2 * std::sin(time * 70) : 0;
    const Rectangle r = {PHONE.x + shake, PHONE.y, PHONE.width, PHONE.height};
    DrawRectangleRounded(r, 0.15f, 6, {20, 20, 24, 255});
    if (phoneDown) { // its back, with the camera
      DrawCircle((int)r.x + 20, (int)r.y + 20, 7, {50, 50, 56, 255});
      return;
    }
    const Rectangle screen = {r.x + 6, r.y + 8, r.width - 12, r.height - 16};
    DrawRectangleRounded(
        screen, 0.1f, 6,
        ColorLerp({30, 32, 40, 255}, {200, 210, 235, 255}, std::fmin(1, buzz)));
    if (buzz > 0) // a notification, no need to read it
      DrawRectangleRounded({screen.x + 4, screen.y + 30, screen.width - 8, 22},
                           0.3f, 4, Faded(CHAT_PURPLE, buzz));
    for (int i = 0; i < 4; i++) // signal bars
      DrawRectangle((int)screen.x + 44 + i * 5, (int)screen.y + 14 - i * 3, 3,
                    4 + i * 3, i < Signal() ? FLOUR : Faded(FLOUR, 0.2f));
  }
};

std::unique_ptr<Scene> MakeTravelScene() {
  return std::unique_ptr<Scene>(new TravelScene());
}

//------------------------------------------------------------------------------
// Coffee: 5 am, before Amma wakes, Anu makes her filter coffee. No words.
//   1. Hold to pour the decoction into the tumbler (about a third).
//   2. Hold to pour the milk (nearly to the top).
//   3. "Stretch" it: hold to pour from the tumbler into the davara below, and
//      move the mouse up and down to pour from higher or lower. Too low and it
//      doesn't mix; too high and it splashes; in between it froths up. Let go
//      to pour it back the other way.
//   4. Served.
// There is no score. Nothing can go wrong for good; it just takes a little
// longer.
//------------------------------------------------------------------------------
enum CoffeeStage { DECOCTION, MILK, STRETCH, SERVED };

const Vector2 TUMBLER = {360, 470}; // where the tumbler stands (its base)
const Vector2 DECOCTION_AT = {190, 470};
const Vector2 MILK_AT = {540, 470};
const float COUNTER_Y = 470;
const float POUR_SPEED = 0.35f;  // of a tumbler, per second
const float MIX_HEIGHT = 110;    // poured from lower than this doesn't mix
const float SPLASH_HEIGHT = 230; // poured from higher than this splashes
const Color DECOCTION_BROWN = {70, 40, 25, 255};
const Color MILK_WHITE = {245, 240, 228, 255};
const Color COFFEE_TAN = {176, 120, 76, 255};
const Color STEEL = {190, 192, 198, 255};

class CoffeeScene : public Scene {
public:
  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    const bool holding = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const Vector2 mouse = GetMousePosition();
    PlayAmbience("morning", 0.35f);
    if (pouring)
      PlayLoop("pour", stage == STRETCH ? 0.5f + height / 600 : 0.6f);

    switch (stage) {
    case DECOCTION:
    case MILK: {
      pouring = holding;
      if (pouring) {
        float &level = stage == DECOCTION ? decoction : milk;
        level = std::fmin(1.1f, level + POUR_SPEED * dt);
      }
      const float enough = stage == DECOCTION ? 0.25f : 0.5f;
      const float poured = stage == DECOCTION ? decoction : milk;
      if (!holding && poured >= enough)
        stage = (CoffeeStage)(stage + 1);
      break;
    }
    case STRETCH:
      // the held vessel follows the mouse up and down
      height = std::fmax(60, std::fmin(300, COUNTER_Y + 60 - mouse.y));
      pouring = holding && inTop > 0;
      if (pouring) {
        const float amount = std::fmin(inTop, 0.8f * dt);
        inTop -= amount;
        if (height >= MIX_HEIGHT) // froths, about three pours' worth
          froth += amount * 0.4f;
        if (height > SPLASH_HEIGHT)
          splash = 0.6f;
      }
      if (!holding && inTop <= 0) // poured it all: pour it back
        inTop = 1;
      if (splash > 0)
        splash -= dt;
      if (froth >= 1) {
        stage = SERVED;
        servedTime = time;
      }
      break;
    case SERVED:
      if (time - servedTime > 1.5f && ContinuePressed())
        done = true;
      break;
    }
  }

  void Draw() override {
    DrawKitchenAtDawn();
    switch (stage) {
    case DECOCTION:
    case MILK:
      DrawPouring();
      break;
    case STRETCH:
      DrawStretching();
      break;
    case SERVED:
      DrawServed();
      break;
    }
  }

private:
  CoffeeStage stage = DECOCTION;
  float decoction = 0, milk = 0; // how much is poured, of a tumbler
  float inTop = 1;    // stretching: how much is still in the held vessel
  float height = 120; // how high above the davara it is held
  float froth = 0;    // 0 to 1
  float splash = 0;
  bool pouring = false;
  float time = 0, servedTime = 0;

  Color CoffeeColour() const {
    const float milkiness = milk / (milk + decoction + 0.001f);
    return ColorLerp(DECOCTION_BROWN, COFFEE_TAN, milkiness * 1.3f);
  }

  // A pulsing ring: "hold here". Shown until the player first does it.
  void DrawHint(Vector2 at) const {
    DrawRing(at, 26, 30, 0, 360, 32,
             Faded(FLOUR, 0.3f + 0.3f * std::sin(time * 4)));
  }

  // A steel tumbler: shaded, with a rolled rim
  static void DrawTumbler(Vector2 base, float fill, Color liquid) {
    const Rectangle cup = {base.x - 22, base.y - 80, 44, 80};
    DrawRectangleGradientH((int)cup.x, (int)cup.y, (int)cup.width,
                           (int)cup.height, {226, 228, 234, 255},
                           {150, 152, 160, 255});
    const float h = 74 * std::fmin(1, fill);
    if (h > 0) {
      DrawRectangle((int)cup.x + 4, (int)(base.y - 4 - h), 36, (int)h, liquid);
      DrawEllipse((int)base.x, (int)(base.y - 4 - h), 18, 3,
                  ColorBrightness(liquid, 0.15f));
    }
    DrawRectangle((int)cup.x + 6, (int)cup.y + 6, 5, 66, {250, 250, 252, 160});
    DrawEllipse((int)base.x, (int)cup.y, 23, 4, {200, 202, 210, 255});
    DrawEllipse((int)base.x, (int)cup.y, 19, 2, {120, 122, 130, 255});
  }

  // The davara: a wide steel bowl with a lip
  static void DrawDavara(Vector2 base, float fill, Color liquid) {
    DrawRectangleRounded({base.x - 50, base.y - 40, 100, 40}, 0.4f, 6, STEEL);
    DrawRectangleGradientH((int)base.x - 46, (int)base.y - 36, 92, 30,
                           {226, 228, 234, 255}, {150, 152, 160, 255});
    DrawEllipse((int)base.x, (int)base.y - 40, 54, 7, {214, 216, 222, 255});
    DrawEllipse((int)base.x, (int)base.y - 40, 46, 5, {130, 132, 140, 255});
    if (fill > 0)
      DrawEllipse((int)base.x, (int)base.y - 39, 44 * std::fmin(1, fill + 0.3f),
                  4, liquid);
  }

  // The two-tier brass filter: the decoction in its bottom half
  static void DrawFilter(Vector2 base) {
    DrawRectangleGradientH((int)base.x - 24, (int)base.y - 50, 48, 50,
                           {222, 180, 100, 255}, {160, 116, 56, 255});
    DrawRectangleGradientH((int)base.x - 20, (int)base.y - 84, 40, 34,
                           {222, 180, 100, 255}, {160, 116, 56, 255});
    DrawEllipse((int)base.x, (int)base.y - 50, 26, 4, {190, 146, 70, 255});
    DrawEllipse((int)base.x, (int)base.y - 86, 24, 5, {230, 190, 110, 255});
    DrawCircle((int)base.x, (int)base.y - 92, 5, {200, 156, 76, 255});
    DrawRectangle((int)base.x - 18, (int)base.y - 80, 4, 72,
                  {250, 230, 180, 140}); // shine
  }

  // A steel milk pan with a long handle
  static void DrawMilkPan(Vector2 base) {
    DrawLineEx({base.x + 36, base.y - 30}, {base.x + 90, base.y - 44}, 7,
               {120, 80, 50, 255});
    DrawRectangleRounded({base.x - 40, base.y - 40, 80, 40}, 0.3f, 6, STEEL);
    DrawRectangleGradientH((int)base.x - 36, (int)base.y - 34, 72, 30,
                           {226, 228, 234, 255}, {150, 152, 160, 255});
    DrawEllipse((int)base.x, (int)base.y - 40, 42, 5, {214, 216, 222, 255});
  }

  // A falling stream from `from` down to `to`
  static void DrawStream(Vector2 from, Vector2 to, Color liquid) {
    DrawLineEx(from, to, 6, liquid);
    DrawLineEx({from.x + 1, from.y}, {to.x + 1, to.y}, 2,
               ColorBrightness(liquid, 0.25f));
  }

  // The kitchen at five: lamp light inside, the sky only just turning blue
  void DrawKitchenAtDawn() const {
    DrawBandGradient(0, COUNTER_Y, {70, 58, 56, 255}, {120, 98, 80, 255});
    const Rectangle window = {250, 60, 220, 160};
    DrawRectangleGradientV((int)window.x, (int)window.y, (int)window.width,
                           (int)window.height, {30, 40, 90, 255},
                           {150, 120, 150, 255});
    DrawCircle(300, 90, 2, FLOUR); // the last star
    DrawPalm(420, 236, 150, time, {20, 24, 40, 255}, {24, 30, 50, 255});
    DrawRectangleLinesEx(window, 10, {84, 56, 38, 255});
    for (int i = 1; i < 4; i++)
      DrawLineEx({window.x + i * 55, window.y},
                 {window.x + i * 55, window.y + window.height}, 3,
                 {50, 52, 60, 255});
    // shelves with brass and steel, either side of the window
    for (int side = 0; side < 2; side++) {
      const float x = side ? 520 : 40;
      DrawRectangle((int)x, 200, 170, 8, {100, 64, 42, 255});
      for (int i = 0; i < 3; i++)
        DrawPot(x + 30 + i * 55, 182, 18, (i + side) % 2 == 0);
    }
    for (int i = 0; i < 3; i++) { // ladles on hooks
      DrawLineEx({60.0f + i * 26, 250}, {60.0f + i * 26, 330}, 4,
                 {170, 172, 178, 255});
      DrawEllipse(60 + i * 26, 334, 9, 6, {170, 172, 178, 255});
    }
    // the little lamp in the corner, its glow
    DrawCircleGradient({640, 300}, 220, {255, 196, 110, 80},
                       {255, 196, 110, 0});
    DrawPot(640, 440, 12, true);
    DrawRectangle(637, 400, 6, 34, {214, 160, 64, 255});
    DrawCircle(640, (int)(394 + std::sin(time * 9)), 5, {255, 204, 84, 255});
    // the counter: dark granite, cream tiles below
    DrawBand(COUNTER_Y, 14, {58, 56, 60, 255});
    DrawBand(COUNTER_Y + 14, 720 - COUNTER_Y - 14, {200, 190, 170, 255});
    const Rectangle all = FullScreen();
    for (float x = all.x; x < all.x + all.width; x += 48)
      DrawLine((int)x, (int)COUNTER_Y + 14, (int)x, 720, {170, 160, 140, 255});
    for (float y = COUNTER_Y + 62; y < 720; y += 48)
      DrawLine((int)all.x, (int)y, (int)(all.x + all.width), (int)y,
               {170, 160, 140, 255});
    DrawBand(COUNTER_Y + 14, 10, {0, 0, 0, 40});
  }

  void DrawPouring() const {
    const bool decoctionStage = stage == DECOCTION;
    // the one not being poured stays on the counter
    const Vector2 rest = decoctionStage ? MILK_AT : DECOCTION_AT;
    if (decoctionStage)
      DrawMilkPan(rest);
    else
      DrawFilter(rest);

    Vector2 pot = decoctionStage ? DECOCTION_AT : MILK_AT;
    if (pouring) // lifted over the tumbler
      pot = {TUMBLER.x + (decoctionStage ? -60 : 60), TUMBLER.y - 150};
    const Color liquid = decoctionStage ? DECOCTION_BROWN : MILK_WHITE;
    if (decoctionStage)
      DrawFilter(pot);
    else
      DrawMilkPan(pot);
    if (pouring)
      DrawStream({pot.x + (decoctionStage ? 24 : -38), pot.y - 30},
                 {TUMBLER.x, TUMBLER.y - 80 + 10}, liquid);

    DrawTumbler(TUMBLER, decoction + milk, CoffeeColour());
    if ((decoctionStage ? decoction : milk) == 0)
      DrawHint(decoctionStage ? Vector2{DECOCTION_AT.x, DECOCTION_AT.y - 45}
                              : Vector2{MILK_AT.x, MILK_AT.y - 20});
    if (decoction + milk > 1) // a little overflow, nothing more
      DrawRectangle((int)TUMBLER.x + 20, (int)TUMBLER.y - 60, 4, 60,
                    CoffeeColour());
  }

  void DrawStretching() const {
    const Vector2 davara = {TUMBLER.x, COUNTER_Y + 60};
    const Vector2 held = {TUMBLER.x + 30, davara.y - 40 - height};
    DrawDavara(davara, 1 - inTop, CoffeeColour());
    if (pouring)
      DrawStream({held.x - 22, held.y - 70}, {davara.x, davara.y - 36},
                 CoffeeColour());
    DrawTumbler(held, inTop, CoffeeColour());
    // froth gathering on top
    for (int i = 0; i < (int)(froth * 12); i++)
      DrawCircle((int)davara.x - 30 + i * 5, (int)davara.y - 40 + (i % 2) * 2,
                 5, Faded(MILK_WHITE, 0.9f));
    if (pouring && height < MIX_HEIGHT) // too low: it just sits there
      DrawTriangle({held.x + 50, held.y - 100}, {held.x + 40, held.y - 84},
                   {held.x + 60, held.y - 84},
                   Faded(FLOUR, 0.3f + 0.3f * std::sin(time * 6)));
    if (splash > 0) // drops on the counter
      for (int i = 0; i < 6; i++)
        DrawCircle((int)davara.x - 70 + i * 28,
                   (int)(davara.y - 10 - 30 * splash * (i % 3)), 3,
                   CoffeeColour());
    if (froth == 0) { // "hold, and lift": a ring with arrows up and down
      DrawHint({held.x, held.y - 40});
      DrawTriangle({held.x + 50, held.y - 80}, {held.x + 42, held.y - 66},
                   {held.x + 58, held.y - 66}, Faded(FLOUR, 0.5f));
      DrawTriangle({held.x + 42, held.y - 14}, {held.x + 50, held.y},
                   {held.x + 58, held.y - 14}, Faded(FLOUR, 0.5f));
    }
  }

  void DrawServed() const {
    const float t = time - servedTime;
    DrawTumbler({TUMBLER.x, COUNTER_Y + 10}, 1, CoffeeColour());
    for (int i = 0; i < 7; i++) // a frothy top
      DrawCircle((int)TUMBLER.x - 16 + i * 5, (int)COUNTER_Y - 68 + (i % 2) * 2,
                 4, MILK_WHITE);
    for (int i = 0; i < 3; i++) { // steam
      const float s = std::fmod(t * 0.5f + i * 0.33f, 1.0f);
      DrawCircle((int)(TUMBLER.x - 10 + i * 10 + 6 * std::sin(s * 6 + i)),
                 (int)(COUNTER_Y - 80 - s * 90), 6 + s * 8,
                 Faded(FLOUR, 0.35f * (1 - s)));
    }
    if (t > 1.5f)
      DrawRing({660, 660}, 10, 13, 0, 360, 24,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 3)));
  }
};

std::unique_ptr<Scene> MakeCoffeeScene() {
  return std::unique_ptr<Scene>(new CoffeeScene());
}

//------------------------------------------------------------------------------
// Voice message: Amma's message plays on Anu's phone, its words shown under
// the waveform. Then the reply box: every key or click types a little more
// of the reply she always sends. When it's done, she deletes it. Instead,
// the bus ticket: "Book".
//------------------------------------------------------------------------------
enum VoiceStage { LISTENING, REPLYING, DELETING, BOOKING };

const Rectangle PHONE_SCREEN = {220, 40, 280, 600};
const Rectangle BOOK_BUTTON = {260, 470, 200, 50};
const std::string USUAL_REPLY = "busy ma, call u later";

class VoiceMessageScene : public Scene {
public:
  explicit VoiceMessageScene(const Step &s) : step(s) { colour = 0.6f; }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    PlayAmbience("city_night", 0.25f);
    switch (stage) {
    case LISTENING: // each line stays a moment, longer for longer lines
      if (stageTime > LineSeconds() ||
          (stageTime > 0.6f && ContinuePressed())) {
        stageTime = 0;
        if (++line >= step.lines.size())
          stage = REPLYING;
      }
      break;
    case REPLYING: // typing, a few letters per key or click
      if ((GetKeyPressed() != 0 || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) &&
          typed < USUAL_REPLY.size()) {
        typed = std::min(USUAL_REPLY.size(), typed + 3);
        PlayEffect("click", 0.4f, 1.3f + GetRandomValue(0, 20) / 100.0f);
      }
      sinceTyped = typed == USUAL_REPLY.size() ? sinceTyped + dt : 0;
      if (sinceTyped > 1.2f) { // she looks at it for a moment
        stage = DELETING;
        stageTime = 0;
      }
      break;
    case DELETING: // her thumb holds backspace
      deleted += 18 * dt;
      if (deleted >= USUAL_REPLY.size() + 12) { // a moment with it empty
        stage = BOOKING;
        stageTime = 0;
      }
      break;
    case BOOKING:
      if (stageTime > 0.8f && Clicked(BOOK_BUTTON))
        done = true;
      break;
    }
  }

  void Draw() override {
    // her room at midnight: the city's glow, the desk lamp, the phone
    DrawBandGradient(0, 720, {26, 26, 40, 255}, {44, 36, 44, 255});
    for (int i = 0; i < 26; i++) { // city lights, out of focus
      const float x =
          FullScreen().x + std::fmod(i * 131.0f, FullScreen().width);
      DrawCircleGradient({x, 120.0f + (i * 47) % 200}, 18 + (i % 4) * 6,
                         Faded({250, 200, 120, 255}, 0.15f),
                         Faded({250, 200, 120, 255}, 0));
    }
    DrawCircleGradient({640, 600}, 300, {255, 200, 130, 50},
                       {255, 200, 130, 0});
    // her hand around the phone
    const Color hand = {176, 118, 80, 255};
    DrawEllipse(212, 520, 30, 70, hand);
    DrawRectangleRounded({PHONE_SCREEN.x - 10, PHONE_SCREEN.y - 10,
                          PHONE_SCREEN.width + 20, PHONE_SCREEN.height + 20},
                         0.08f, 8, {10, 10, 12, 255});
    DrawRectangleRounded(PHONE_SCREEN, 0.06f, 8, {236, 232, 226, 255});
    for (int i = 0; i < 4; i++) // her fingers around the edge
      DrawEllipse(507, 430 + i * 30, 13, 11, ColorBrightness(hand, -0.05f * i));
    DrawString("11:59", PHONE_SCREEN.x + 18, PHONE_SCREEN.y + 6, 14,
               {80, 80, 90, 255});
    DrawRectangle((int)(PHONE_SCREEN.x + PHONE_SCREEN.width - 40),
                  (int)PHONE_SCREEN.y + 9, 22, 10, {80, 80, 90, 255});

    if (stage == BOOKING) {
      DrawTicket();
      return;
    }
    // the chat with Amma
    DrawRectangle((int)PHONE_SCREEN.x, (int)PHONE_SCREEN.y + 20,
                  (int)PHONE_SCREEN.width, 50, {40, 170, 90, 255});
    DrawCircle(254, 85, 14, {236, 232, 226, 255});
    DrawCircle(254, 81, 5, {176, 118, 80, 255}); // her photo, tiny
    DrawCircle(254, 93, 8, {46, 120, 80, 255});
    DrawString("Amma", 278, 74, 22, {255, 255, 255, 255});

    // the voice message bubble, its waveform filling as it plays
    const float played =
        stage == LISTENING
            ? (line + std::fmin(1.0f, stageTime / LineSeconds())) /
                  step.lines.size()
            : 1.0f;
    DrawRectangleRounded({236, 120, 220, 56}, 0.4f, 6, {255, 255, 255, 255});
    for (int i = 0; i < 24; i++) {
      const float h = 4 + 16 * std::fabs(std::sin(i * 1.7f));
      const bool heard = i < played * 24;
      DrawRectangle(250 + i * 8, (int)(148 - h / 2), 4, (int)h,
                    heard ? Color{40, 170, 90, 255}
                          : Color{190, 190, 196, 255});
    }
    if (stage == LISTENING && line < step.lines.size())
      DrawTextWrapped(step.lines[line].text, 244, 200, 22, 232,
                      {60, 60, 70, 255});

    // the reply box
    if (stage != LISTENING) {
      DrawRectangleRounded({232, 570, 256, 50}, 0.5f, 6, {255, 255, 255, 255});
      const int shown =
          stage == REPLYING
              ? (int)typed
              : std::max(0, (int)USUAL_REPLY.size() - (int)deleted);
      if (shown > 0)
        DrawString(USUAL_REPLY.substr(0, shown), 248, 584, 19,
                   {40, 40, 50, 255});
      const bool cursorOn = std::fmod(time, 1.0f) < 0.5f;
      if (cursorOn) // a blinking cursor
        DrawRectangle(250 +
                          (int)MeasureString(USUAL_REPLY.substr(0, shown), 19),
                      582, 2, 24, {40, 40, 50, 255});
      if (stage == REPLYING && typed == 0) // "type": a pulsing ring
        DrawRing({360, 595}, 30, 33, 0, 360, 32,
                 Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 4)));
    }
  }

private:
  const Step &step;
  VoiceStage stage = LISTENING;
  size_t line = 0;
  size_t typed = 0; // letters of the reply typed
  float deleted = 0;
  float sinceTyped = 0;
  float time = 0, stageTime = 0;

  float LineSeconds() const {
    return 1.4f +
           0.05f *
               step.lines[std::min(line, step.lines.size() - 1)].text.size();
  }

  void DrawTicket() const {
    const float a = stageTime * 2;
    const Color ink = Faded({40, 40, 50, 255}, a);
    // the ticket: a coloured band, the route as a dotted line, a tear line
    DrawRectangleRounded({236, 60, 248, 300}, 0.08f, 6,
                         Faded({255, 255, 255, 255}, a));
    DrawRectangleRounded({236, 60, 248, 10}, 1, 4,
                         Faded({214, 120, 60, 255}, a));
    for (int i = 0; i < 12; i++)
      DrawCircle(258, 228 + i * 6, 1.5f, Faded({180, 180, 190, 255}, a));
    DrawCircle(258, 226, 5, Faded({40, 170, 90, 255}, a));
    DrawCircle(258, 302, 5, Faded(KUMKUM, a));
    DrawString("Hyderabad", 274, 216, 18, ink);
    DrawString("Sathyamangalam", 274, 292, 18, ink);
    for (int i = 0; i < 20; i++)
      DrawRectangle(244 + i * 12, 340, 6, 2, Faded({200, 200, 210, 255}, a));
    DrawString("Night bus", 248, 80, 30, ink);
    DrawString("Departs tonight, 9:30 PM", 248, 130, 18, ink);
    DrawString("1 seat, by the window", 248, 160, 18,
               Faded({110, 110, 120, 255}, a));
    const bool hover = CheckCollisionPointRec(GetMousePosition(), BOOK_BUTTON);
    DrawButton(BOOK_BUTTON, "Book",
               Faded(hover ? KUMKUM : Color{40, 170, 90, 255}, a));
  }
};

std::unique_ptr<Scene> MakeVoiceMessageScene(const Step &step) {
  return std::unique_ptr<Scene>(new VoiceMessageScene(step));
}
