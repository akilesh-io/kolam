// scenes.cpp - the intro animation, dialogue scenes, Amma's list (the end of
// Chapter 1), the doorstep drawing, and MakeScene(), which makes the right
// scene for each story step.
// (Exploring is in explore_scene.cpp, the minigames in minigames.cpp, and the
// kolam book in puzzle_scene.cpp.)
#include <cmath>

#include "game.h"

//------------------------------------------------------------------------------
// Intro: plays assets/krita.gif once, pauses a second on the last frame, then
// the home screen appears. Any click or key skips it.
//------------------------------------------------------------------------------
class IntroScene : public Scene {
public:
  IntroScene() {
    if (const EmbeddedFile *gif = FindAsset("assets/krita.gif")) {
      animation = LoadImageAnimFromMemory(".gif", gif->data, (int)gif->size,
                                          &frameCount);
    }
    if (frameCount > 0)
      texture = LoadTextureFromImage(animation);
  }

  ~IntroScene() override {
    if (texture.id != 0)
      UnloadTexture(texture);
    UnloadImage(animation);
  }

  void Update() override {
    const float FRAME_SECONDS = 0.05f;  // how long each GIF frame shows
    const float LAST_FRAME_SECONDS = 1; // pause on the last frame
    const float NO_GIF_SECONDS = 3.5f;  // if the GIF couldn't be loaded
    const float dt = GetFrameTime();
    time += dt;
    frameTimer += dt;

    // Next frame: copy that frame's pixels from the GIF into the texture
    if (frame < frameCount - 1 && frameTimer >= FRAME_SECONDS) {
      frame++;
      frameTimer = 0;
      const int frameBytes = animation.width * animation.height * 4;
      UpdateTexture(texture,
                    (unsigned char *)animation.data + frameBytes * frame);
    }

    const bool finished = frameCount <= 1 ? time > NO_GIF_SECONDS
                                          : (frame == frameCount - 1 &&
                                             frameTimer > LAST_FRAME_SECONDS);
    if (finished || ContinuePressed())
      done = true;
  }

  void Draw() override {
    ClearBackground(BLACK);
    if (texture.id != 0) {
      DrawTexturePro(texture,
                     {0, 0, (float)texture.width, (float)texture.height},
                     {0, 0, STAGE_SIZE, STAGE_SIZE}, {0, 0}, 0, WHITE);
    }
    DrawTextCentered("Press SPACE to skip", STAGE_SIZE / 2.0f, 676, 20,
                     Faded(LIGHTGRAY, time - 1.0f)); // fades in after 1s
  }

private:
  Image animation = {};
  Texture2D texture = {};
  int frameCount = 0;
  int frame = 0;
  float frameTimer = 0;
  float time = 0;
};

//------------------------------------------------------------------------------
// Dialogue: shows the lines of a Talk step one at a time, typing them out.
// A Talk with no lines is a quiet moment at its place: it moves on with a
// click, or by itself after a while.
//------------------------------------------------------------------------------
class DialogueScene : public Scene {
public:
  explicit DialogueScene(const Step &talk) : step(talk) {}

  void Update() override {
    const float QUIET_SECONDS = 6; // a moment with no words
    time += GetFrameTime();
    // At night, crickets; by day, birds; a flashback has only its words
    if (!step.place.empty())
      PlayAmbience(step.night ? "crickets" : "birds_day", 0.5f);
    else
      PlayAmbience("");
    if (step.lines.empty())
      done = time > QUIET_SECONDS || (time > 1.5f && ContinuePressed());
    else
      done = AdvanceDialogue(step.lines, current, typed);
  }

  void Draw() override {
    if (!step.place.empty()) // where it happens, e.g. the terrace at night
      DrawPlace(step.place, step.night, time);
    else if (step.memory) // a warm sunset for flashbacks
      DrawBandGradient(0, STAGE_SIZE, {255, 196, 120, 255},
                       {236, 112, 99, 255});
    else
      DrawBandGradient(0, STAGE_SIZE, NIGHT_TOP, NIGHT_BOTTOM);
    if (!step.lines.empty())
      DrawDialogueBox(step.lines[current], typed);
    else if (time > 1.5f) // a quiet "go on"
      DrawRing({660, 660}, 10, 13, 0, 360, 24,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 3)));
  }

private:
  const Step &step;
  size_t current = 0; // the line being shown
  float typed = 0;    // how many characters of it are shown so far
  float time = 0;     // for the place's gentle movement (steam, the swing...)
};

//------------------------------------------------------------------------------
// Amma's list: the end of Chapter 1. On the doorstep at dawn, Amma's kolam
// notebook lies open at the festival kolam: the pattern draws itself, but
// it isn't time yet. Amma says one line, and hands Anu a shopping list; its
// items appear one by one, the last with a little drawing of flowers.
// Then "Tomorrow.", and the end of the chapter.
//------------------------------------------------------------------------------
enum ListStage { NOTEBOOK, AMMA_SAYS, SHOPPING_LIST, TOMORROW };

const Rectangle LEFT_PAGE = {90, 90, 270, 420};
const Rectangle RIGHT_PAGE = {360, 90, 270, 420};
const Color PAPER = {246, 238, 216, 255};
const Color PAPER_LINE = {170, 196, 220, 255};
const Color AMMAS_INK = {40, 52, 110, 255}; // her blue ballpoint
const float ITEM_SECONDS = 0.45f;           // between list items appearing

class ListScene : public Scene {
public:
  explicit ListScene(const Step &s) : step(s) {
    amma = {step.lines[0]};
    KolamGrid grid;
    const Page &page = Pages()[step.page];
    if (grid.Load(page) && grid.CountSolutions(true) > 0) {
      const float cell = std::fmin(220.0f / grid.width, 300.0f / grid.height);
      const Vector2 origin = {
          RIGHT_PAGE.x + (RIGHT_PAGE.width - cell * grid.width) / 2,
          RIGHT_PAGE.y + 40 +
              (RIGHT_PAGE.height - 40 - cell * grid.height) / 2};
      loops = LoopPaths(grid, origin, cell, cell * 0.02f);
      for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++)
          dots.push_back(
              {origin.x + (x + 0.5f) * cell, origin.y + (y + 0.5f) * cell});
      }
    }
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    stageTime += dt;
    PlayAmbience(stage == TOMORROW ? "" : "morning", 0.45f);
    switch (stage) {
    case NOTEBOOK: // the pattern draws itself; a click after it's drawn
      if (stageTime > 3 && ContinuePressed())
        Next(AMMA_SAYS);
      break;
    case AMMA_SAYS:
      if (AdvanceDialogue(amma, ammaLine, typed))
        Next(SHOPPING_LIST);
      break;
    case SHOPPING_LIST:
      if (stageTime > ItemCount() * ITEM_SECONDS + 1 && ContinuePressed())
        Next(TOMORROW);
      break;
    case TOMORROW:
      if (stageTime > 5 && ContinuePressed())
        done = true;
      break;
    }
  }

  void Draw() override {
    DrawPlace("front_step", NIGHT, time); // just before dawn
    if (stage == NOTEBOOK || stage == AMMA_SAYS) {
      DrawNotebook();
      if (stage == AMMA_SAYS) {
        DrawDim();
        DrawDialogueBox(amma[0], typed);
      }
    } else {
      DrawNotebook();
      DrawDim();
      DrawList();
    }
    if (stage == NOTEBOOK && stageTime > 3)
      DrawRing({660, 660}, 10, 13, 0, 360, 24,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 3)));
    if (stage == TOMORROW) {
      DrawBand(0, STAGE_SIZE,
               Faded({20, 18, 30, 255}, std::fmin(1.0f, stageTime)));
      DrawTextCentered("Tomorrow.", 360, 300, 56, Faded(FLOUR, stageTime - 1),
                       HAND_FONT);
      DrawTextCentered("End of Chapter 1", 360, 390, 20,
                       Faded(MUTED, stageTime - 3));
    }
  }

private:
  const Step &step;
  std::vector<Line> amma; // what Amma says, as a one-line dialogue
  size_t ammaLine = 0;
  float typed = 0;
  ListStage stage = NOTEBOOK;
  float time = 0, stageTime = 0;
  std::vector<std::vector<Vector2>> loops; // Amma's kolam, solved
  std::vector<Vector2> dots;

  void Next(ListStage next) {
    stage = next;
    stageTime = 0;
    if (next == SHOPPING_LIST) // the list, unfolded
      PlayEffect("page", 0.8f);
  }

  int ItemCount() const { return (int)step.lines.size() - 1; }

  static void DrawPaper(Rectangle r) {
    DrawRectangleRec(r, PAPER);
    for (float y = r.y + 50; y < r.y + r.height - 10; y += 26)
      DrawLineEx({r.x + 14, y}, {r.x + r.width - 14, y}, 1, PAPER_LINE);
  }

  // The notebook, open: her note on the left, the kolam on the right
  void DrawNotebook() const {
    DrawRectangleRounded({LEFT_PAGE.x - 14, LEFT_PAGE.y - 10,
                          LEFT_PAGE.width + RIGHT_PAGE.width + 28,
                          LEFT_PAGE.height + 24},
                         0.04f, 6, {120, 46, 40, 255}); // the cloth cover
    DrawPaper(LEFT_PAGE);
    DrawPaper(RIGHT_PAGE);
    DrawRectangleGradientH((int)RIGHT_PAGE.x - 16, (int)RIGHT_PAGE.y, 32,
                           (int)RIGHT_PAGE.height, {0, 0, 0, 0},
                           {0, 0, 0, 40}); // the fold
    const Page &page = Pages()[step.page];
    DrawString(page.title, LEFT_PAGE.x + 22, LEFT_PAGE.y + 18, 28, AMMAS_INK,
               HAND_FONT);
    DrawTextWrapped(page.note, LEFT_PAGE.x + 22, LEFT_PAGE.y + 70, 22,
                    LEFT_PAGE.width - 44, AMMAS_INK, HAND_FONT);
    // the kolam, drawing itself slowly, as if remembered
    const float drawn = std::fmin(1.0f, time / 2.5f);
    for (const Vector2 &dot : dots)
      DrawCircleV(dot, 3, AMMAS_INK);
    for (const std::vector<Vector2> &loop : loops)
      DrawPath(loop, (size_t)(drawn * loop.size()), drawn >= 1, 2.5f,
               Faded(AMMAS_INK, 0.85f));
  }

  // The list, in Amma's hand, the items appearing one by one
  void DrawList() const {
    const Rectangle slip = {230, 70, 260, 110.0f + ItemCount() * 40};
    DrawRectangleRec({slip.x + 6, slip.y + 8, slip.width, slip.height},
                     {0, 0, 0, 60}); // shadow
    DrawRectangleRec(slip, PAPER);
    for (int i = 0; i < 9; i++) // torn top edge
      DrawTriangle({slip.x + i * 29, slip.y},
                   {slip.x + i * 29 + 14, slip.y - 6},
                   {slip.x + i * 29 + 29, slip.y}, PAPER);
    for (int i = 0; i < ItemCount(); i++) {
      const float a = (stageTime - i * ITEM_SECONDS) / ITEM_SECONDS;
      if (a <= 0)
        continue;
      const float y = slip.y + 50 + i * 40;
      DrawLineEx({slip.x + 16, y + 34}, {slip.x + slip.width - 16, y + 34}, 1,
                 PAPER_LINE);
      const std::string &item = step.lines[i + 1].text;
      DrawString(item, slip.x + 28, y, 26, Faded(AMMAS_INK, a), HAND_FONT);
      if (i == ItemCount() - 1) // a small drawing of flowers beside it
        DrawFlowers(slip.x + 48 + MeasureString(item, 26, HAND_FONT), y + 16,
                    Faded(AMMAS_INK, a));
    }
  }

  // Three little five-petal flowers, as a ballpoint draws them
  static void DrawFlowers(float x, float y, Color ink) {
    const Vector2 FLOWERS[] = {{0, 0}, {22, -6}, {40, 4}};
    for (Vector2 f : FLOWERS) {
      for (int p = 0; p < 5; p++) {
        const float a = p * 2 * PI / 5;
        DrawRing({x + f.x + 6 * std::cos(a), y + f.y + 6 * std::sin(a)}, 3,
                 4.5f, 0, 360, 12, ink);
      }
      DrawCircleV({x + f.x, y + f.y}, 2, ink);
    }
  }
};

//------------------------------------------------------------------------------
// Chapter card: the chapter's heading, title, and where and when it happens,
// fading in over a quiet background. Click (after a moment) or wait.
//------------------------------------------------------------------------------
class ChapterCard : public Scene {
public:
  explicit ChapterCard(const Chapter &c) : chapter(c) {}

  void Update() override {
    const float READ_SECONDS = 1.2f; // before a click can skip it
    const float SHOW_SECONDS = 3.5f; // then it moves on by itself
    time += GetFrameTime();
    PlayAmbience(""); // a quiet title card
    if ((time > READ_SECONDS && ContinuePressed()) || time > SHOW_SECONDS)
      done = true;
  }

  void Draw() override {
    DrawBandGradient(0, STAGE_SIZE, NIGHT_TOP, NIGHT_BOTTOM);
    // each line fades in a little after the one above it
    auto fade = [this](float delay) {
      return Faded(FLOUR, (time - delay) * 1.5f);
    };
    // a small kolam motif: dots and a looping line around them
    for (int i = 0; i < 3; i++) {
      const Vector2 dot = {330.0f + i * 30, 230};
      DrawCircleV(dot, 3, fade(0));
      DrawRing(dot, 9, 11, 0, 360 * std::fmin(1.0f, time), 24, fade(0));
    }
    DrawTextCentered(chapter.heading, 360, 280, 26, Faded(MUTED, time * 1.5f));
    DrawTextCentered(chapter.title, 360, 318, 64, fade(0.4f), HAND_FONT);
    DrawTextCentered(chapter.where, 360, 410, 22, fade(0.9f));
    if (time > 1.2f)
      DrawTextCentered("click to begin", 360, 640, 15,
                       Faded(MUTED, 0.5f + 0.4f * std::sin(time * 3)));
  }

private:
  const Chapter &chapter;
  float time = 0;
};

//------------------------------------------------------------------------------
// Doorstep: free drawing around a grid of dots, mirrored left/right and
// up/down. Strokes that touch a dot are rejected.
// After Finish comes the ending, with almost no words: the kolam lies on the
// doorstep, the sun comes up, colour floods back, and Amma comes to the door.
// It is the finale of Chapter 8 (not written yet); its full ending, with
// little Anu, comes with that chapter.
//------------------------------------------------------------------------------
const int DOORSTEP_DOTS = 6;    // dots across and down
const float DOT_SPACING = 120;  // pixels between dots
const float DOT_MARGIN = 50;    // from the screen edge to the first dot
const float TOUCH_DISTANCE = 7; // closer than this to a dot = touching it
// The centre of the dots, which strokes are mirrored around
const float MIRROR_CENTRE = DOT_MARGIN + DOT_SPACING * (DOORSTEP_DOTS - 1) / 2;

class DoorstepScene : public Scene {
public:
  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    PlayAmbience("morning", 0.45f);
    if (drawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        (GetMouseDelta().x != 0 || GetMouseDelta().y != 0))
      PlayLoop("pencil", 0.35f); // rice flour from her fingers
    if (messageTimer > 0)
      messageTimer -= dt;
    if (ending >= 0) {
      if (ending == 0)
        startColour = colour;
      ending += dt;
      colour = startColour + (1 - startColour) * Between(ending, 2.5f, 6);
      if (ending > END_SECONDS && ContinuePressed())
        done = true;
      return;
    }

    UpdateDrawing();

    if (IsKeyPressed(KEY_H))
      mirrorLeftRight = !mirrorLeftRight;
    if (IsKeyPressed(KEY_V))
      mirrorUpDown = !mirrorUpDown;
    if ((IsKeyPressed(KEY_Z) || Clicked(UNDO_BUTTON)) && !strokes.empty())
      strokes.pop_back();
    if (IsKeyPressed(KEY_C) || Clicked(CLEAR_BUTTON))
      strokes.clear();
    if ((IsKeyPressed(KEY_ENTER) || Clicked(FINISH_BUTTON)) && !strokes.empty())
      ending = 0;
  }

  void Draw() override {
    if (ending >= 0) {
      DrawEnding();
      return;
    }
    ClearBackground({15, 35, 100, 255});

    // Amma's pattern, from thirty years of mornings, faintly under the dots:
    // a loop around each dot. Trace it, or don't.
    for (int y = 0; y < DOORSTEP_DOTS; y++) {
      for (int x = 0; x < DOORSTEP_DOTS; x++) {
        DrawRing(Dot(x, y), 34, 37, 0, 360, 40, {200, 200, 255, 28});
        DrawCircleV(Dot(x, y), 4, {200, 200, 255, 200});
      }
    }
    for (const auto &s : strokes)
      DrawMirrored(s, {220, 220, 255, 255});
    if (drawing)
      DrawMirrored(stroke, {150, 180, 255, 200});

    DrawString(TextFormat("Draw around the dots.   H: mirror left/right [%s]   "
                          "V: mirror up/down [%s]",
                          mirrorLeftRight ? "on" : "off",
                          mirrorUpDown ? "on" : "off"),
               20, 10, 15, Faded(FLOUR, 0.6f));
    DrawDoorstepButton(FINISH_BUTTON, "Finish");
    DrawDoorstepButton(UNDO_BUTTON, "Undo (Z)");
    DrawDoorstepButton(CLEAR_BUTTON, "Clear (C)");

    if (messageTimer > 0)
      DrawString(message, 20, 678, 20,
                 Faded({255, 255, 180, 255}, messageTimer));
  }

private:
  const Rectangle FINISH_BUTTON = {370, 670, 100, 40};
  const Rectangle UNDO_BUTTON = {485, 670, 100, 40};
  const Rectangle CLEAR_BUTTON = {600, 670, 100, 40};

  std::vector<std::vector<Vector2>> strokes;
  std::vector<Vector2> stroke; // the one being drawn
  bool drawing = false;
  bool mirrorLeftRight = true;
  bool mirrorUpDown = true;
  std::string message;
  float messageTimer = 0;
  float time = 0;
  float ending = -1; // seconds since Finish; -1 while still drawing
  float startColour = 0;
  const float END_SECONDS = 11; // then a click goes back to the home screen

  // The ending: the front step at dawn, with Anu's kolam on it
  void DrawEnding() const {
    const float t = ending;
    DrawPlace("front_step", false, time);
    DrawBand(580, 140, {150, 120, 95, 255}); // sweep away yesterday's kolam
    for (const auto &s : strokes)            // her kolam, lying on the ground
      DrawMirrored(s, FLOUR, {360, 648}, 0.24f, 2.5f);
    for (int y = 0; y < DOORSTEP_DOTS; y++) {
      for (int x = 0; x < DOORSTEP_DOTS; x++)
        DrawCircleV(OnGround(Dot(x, y), {360, 648}, 0.24f), 1.5f, FLOUR);
    }
    // Anu, sitting back on her heels beside it
    DrawAnuSitting(170, 672, time);
    if (t > 2) // Amma, at the door
      DrawAmma(360, 560, time, false);
    // Before sunrise it is dark blue; that lifts as the sun comes up
    DrawBand(0, 720,
             Faded({20, 26, 70, 255}, 0.75f * (1 - Between(t, 1.5f, 5))));
    // and the kolam drawing fades into this scene
    DrawBand(0, 720, Faded({15, 35, 100, 255}, 1 - Between(t, 0, 1.5f)));

    if (t > 5.5f && t < 9.5f) // the only words
      DrawTextCentered(
          "Amma: \"It's beautiful, ma.\"", 360, 676, 24,
          Faded(FLOUR, Between(t, 5.5f, 6.5f) * (1 - Between(t, 8.5f, 9.5f))));
    if (t > 9.5f) { // the title
      DrawBand(0, 720,
               Faded({20, 18, 30, 255}, Between(t, 9.5f, 10.5f) * 0.85f));
      DrawTextCentered("Kolam", 360, 300, 72, Faded(FLOUR, t - 10), HAND_FONT);
    }
  }

  // Press, drag and release the mouse to draw one stroke
  void UpdateDrawing() {
    const Vector2 mouse = GetMousePosition();
    const bool overButtons = mouse.y > FINISH_BUTTON.y - 10;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !overButtons) {
      stroke = {mouse};
      drawing = !TouchesDot(mouse, mouse);
      if (!drawing)
        Say("Go around the dots, not through them.");
    }
    if (drawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      if (TouchesDot(stroke.back(), mouse)) {
        drawing = false;
        Say("Go around the dots, not through them.");
      } else {
        stroke.push_back(mouse);
      }
    }
    if (drawing && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      drawing = false;
      if (stroke.size() >= 2)
        strokes.push_back(stroke);
    }
  }

  static Vector2 Dot(int x, int y) {
    return {DOT_MARGIN + x * DOT_SPACING, DOT_MARGIN + y * DOT_SPACING};
  }

  // Does the line from a to b come too close to any dot?
  bool TouchesDot(Vector2 a, Vector2 b) const {
    for (int y = 0; y < DOORSTEP_DOTS; y++) {
      for (int x = 0; x < DOORSTEP_DOTS; x++) {
        if (CheckCollisionPointLine(Dot(x, y), a, b, (int)TOUCH_DISTANCE) ||
            CheckCollisionPointCircle(Dot(x, y), b, TOUCH_DISTANCE))
          return true;
      }
    }
    return false;
  }

  void Say(const std::string &text) {
    message = text;
    messageTimer = 1.6f;
  }

  // Mirrors a point around the centre of the dots
  static Vector2 Flip(Vector2 p, bool flipX, bool flipY) {
    if (flipX)
      p.x = 2 * MIRROR_CENTRE - p.x;
    if (flipY)
      p.y = 2 * MIRROR_CENTRE - p.y;
    return p;
  }

  // Shrinks a point of the drawing onto the ground at `at`, flattened as if
  // seen from above at an angle. scale 1 = the drawing as it was drawn.
  static Vector2 OnGround(Vector2 p, Vector2 at, float scale) {
    if (scale == 1)
      return p;
    return {at.x + (p.x - MIRROR_CENTRE) * scale,
            at.y + (p.y - MIRROR_CENTRE) * scale * 0.5f};
  }

  // Draws a stroke plus its mirror images (shrunk onto the ground, if asked)
  void DrawMirrored(const std::vector<Vector2> &points, Color color,
                    Vector2 at = {0, 0}, float scale = 1,
                    float thickness = 6) const {
    for (int flipX = 0; flipX <= (mirrorLeftRight ? 1 : 0); flipX++) {
      for (int flipY = 0; flipY <= (mirrorUpDown ? 1 : 0); flipY++) {
        for (size_t i = 1; i < points.size(); i++) {
          DrawLineEx(OnGround(Flip(points[i - 1], flipX, flipY), at, scale),
                     OnGround(Flip(points[i], flipX, flipY), at, scale),
                     thickness, color);
        }
      }
    }
  }

  // Lights up red under the mouse
  static void DrawDoorstepButton(Rectangle button, const char *label) {
    const bool hover = CheckCollisionPointRec(GetMousePosition(), button);
    DrawButton(button, label, hover ? KUMKUM : Faded(FLOUR, 0.15f));
  }
};

//------------------------------------------------------------------------------
std::unique_ptr<Scene> MakeChapterCard(const Chapter &chapter) {
  return std::unique_ptr<Scene>(new ChapterCard(chapter));
}

std::unique_ptr<Scene> MakeIntroScene() {
  return std::unique_ptr<Scene>(new IntroScene());
}

std::unique_ptr<Scene> MakeScene(const Step &step) {
  std::unique_ptr<Scene> scene;
  switch (step.type) {
  case StepType::Talk:
    scene.reset(new DialogueScene(step));
    break;
  case StepType::Puzzle:
    scene = MakePuzzleScene(step.page);
    break;
  case StepType::Doorstep:
    scene.reset(new DoorstepScene());
    break;
  case StepType::VoiceMessage:
    scene = MakeVoiceMessageScene(step);
    break;
  case StepType::Coffee:
    scene = MakeCoffeeScene();
    break;
  case StepType::List:
    scene.reset(new ListScene(step));
    break;
  case StepType::Explore:
    scene = MakeExploreScene(step);
    break;
  case StepType::Busy:
    return MakeBusyScene(); // Hyderabad: sets its own, muted, colour
  case StepType::Travel:
    return MakeTravelScene(); // the night bus: sets its own colour too
  case StepType::Kite:
    return MakeKiteScene(); // a memory: full colour
  }
  // Memories are always in full colour. The present day is as colourful as
  // Anu's world is right now.
  scene->colour = step.memory ? 1.0f : WorldColour();
  return scene;
}
