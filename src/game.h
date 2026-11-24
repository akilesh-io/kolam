// game.h - everything the game's .cpp files share.
//
// clang-format off
// Where things live:
//   art.cpp            the people (Amma, Anu, little Anu, Appa), drawn with shapes
//   main.cpp           window, main loop, screens (home, settings, pause)
//   story.cpp          ALL game content: dialogue, puzzle pages, and their order
//   scenes.cpp         the intro, dialogue, Amma's list, the doorstep drawing
//   explore_scene.cpp  walking around Anu's home: rooms, objects, memories, the phone
//   minigames.cpp      Hyderabad, the voice message, the bus, the kite, coffee
//   puzzle_scene.cpp   a page of Amma's kolam book (the puzzle)
//   kolam.cpp          the kolam puzzle rules (KolamGrid) and drawing kolam lines
//   ui.cpp             fonts, menus, buttons, text and other small drawing helpers
//   audio.cpp          sounds: each place's background sound, and effects
//   save.cpp           settings, the save game, and what Anu has found (Progress)
// clang-format on
//
// The files the game uses (fonts, music, the intro gif) are in assets/.
// xmake.lua builds them into the game, so the finished game is one file.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "raylib.h"

// Phones: Android, and iPhone (PLATFORM_IOS, set by xmake.lua: raylib runs
// there on SDL's iOS layer, with OpenGL ES 2). The game plays the same on
// both: touch, no pointer, always fullscreen.
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
#define PLATFORM_PHONE
#endif

// The game's screen is 720 units tall, and as wide as Settings -> Screen shape
// says: 1280 units for 16:9 (the default), 960 for 4:3, 720 for a square.
// Everything is laid out on the "stage": the 720x720 square in the middle of
// the screen, from (0, 0) to (720, 720). On wider screens there is more room
// left and right of it; FullScreen() is the whole screen, in stage units (its
// left edge is negative). Use it for backgrounds, so they reach the edges.
// main.cpp scales all of this to the real window and keeps it sharp.
const int STAGE_SIZE = 720;
extern float screenWidth; // in stage units, set by main.cpp every frame
inline Rectangle FullScreen() {
  return {(STAGE_SIZE - screenWidth) / 2, 0, screenWidth, (float)STAGE_SIZE};
}

// Shared colours
const Color NIGHT_TOP = {16, 18, 42, 255};
const Color NIGHT_BOTTOM = {52, 34, 62, 255};
const Color PANEL = {26, 26, 52, 245};
const Color FLOUR = {240, 234, 222, 255}; // rice-flour white
const Color KUMKUM = {186, 64, 46, 255};  // red accent
const Color MUTED = {160, 150, 175, 255};

//------------------------------------------------------------------------------
// Story content (story.cpp)
//------------------------------------------------------------------------------
struct Line {
  std::string speaker; // empty = narration
  std::string text;
  // Above 0: a hard thing to say. The words only come out while the player
  // holds the button, taking this many seconds.
  float hold = 0;
};

// One page of the kolam book. kolam.cpp explains the layout characters.
struct Page {
  std::string title;
  std::string note; // handwritten note at the top of the page
  bool byLittleAnu; // drawn by Anu as a kid: crayon on notebook paper
  bool symmetric;   // every change is mirrored to all four corners
  std::vector<std::string> layout;
  // How the page is played: false = put up walls between the dots until the
  // lines join into one; true = draw the line yourself ("pour" it, holding
  // and dragging), going round Amma's ink, until it closes round every dot
  bool draw = false;
};

// Something Anu can click on while exploring. Where it is drawn is decided in
// explore_scene.cpp (by `object`); what happens is decided in story.cpp.
struct Hotspot {
  // e.g. "appa_photo" (see ROOM_OBJECTS in explore_scene.cpp)
  std::string object;
  std::string name;        // shown when the mouse is over it
  std::vector<Line> lines; // what is said when it is clicked
  std::string memory;      // a memory Anu gets back (brings colour back), or ""
  bool endsScene = false;  // clicking it moves the story on
};

enum class StepType {
  Talk,
  VoiceMessage,
  Puzzle,
  Doorstep,
  Busy,
  Travel,
  Coffee,
  Explore,
  Kite,
  List, // Amma's notebook, then her shopping list: the end of Chapter 1
  Play  // a scene made in its own file, found by name (see PLAY_SCENE)
};

// One step of the story. The save game remembers which step you are on.
const bool NIGHT = true, DAY = false; // for Step::night and At()

struct Step {
  StepType type;
  std::string label;             // shown in the pause menu
  std::vector<Line> lines;       // Talk: the dialogue
  int page = -1;                 // Puzzle, List: index into Pages()
  bool lookOnly = false;         // Puzzle: only looked at, not solved (yet)
  bool memory = false;           // Talk: a flashback, shown in full colour
  std::vector<Hotspot> hotspots; // Explore: what can be clicked
  bool night = false;            // Talk/Explore: evening or night lighting
  std::string place;             // Talk: where it happens ("" = no place)
  bool withAmma = false;         // Memory at a place: little Anu and Amma
  std::string play;              // Play: the scene's name, e.g. "ring_toss"

  // Talk("...", {...}).At("terrace", NIGHT): the dialogue plays over that
  // place (see DrawPlace). Places: front_step, hall, kitchen, terrace.
  Step At(const std::string &where, bool atNight) const {
    Step step = *this;
    step.place = where;
    step.night = atNight;
    return step;
  }
  // Memory(...).At("front_step", NIGHT).WithAmma(): little Anu and Amma are
  // there, as they were
  Step WithAmma() const {
    Step step = *this;
    step.withAmma = true;
    return step;
  }
};

// The story is told in chapters. Each one opens with a title card.
struct Chapter {
  std::string id;      // never shown; the save game remembers chapters by it
  std::string heading; // "Chapter 1"
  std::string title;   // "Home"
  std::string where;   // "Hyderabad, eleven days before Pongal"
  std::vector<Step> steps;
};

const std::vector<Page> &Pages();
const std::vector<Chapter> &Chapters();
// Work calls while Anu is exploring: what is said if she answers
const std::vector<std::vector<Line>> &WorkCalls();
// The memory Anu gets back by flying Appa's kite (it isn't a Hotspot)
const char *const KITE_MEMORY = "Appa's kite";

//------------------------------------------------------------------------------
// Scenes (scenes.cpp, explore_scene.cpp, minigames.cpp, puzzle_scene.cpp)
//------------------------------------------------------------------------------
// A scene loads what it needs in its constructor, frees it in its destructor,
// and sets `done` when the game should move on to the next step.
// `colour` is how colourful the screen is: 0 = grey, 1 = full colour.
class Scene {
public:
  virtual ~Scene() = default;
  virtual void Update() = 0;
  virtual void Draw() = 0;
  bool done = false;
  float colour = 1.0f;
};

std::unique_ptr<Scene> MakeScene(const Step &step); // any story step

// Scenes for Play("name", ...) steps live in their own files (one per
// chapter, e.g. ch2_list.cpp) and register themselves by name:
//   static std::unique_ptr<Scene> MakeRingToss(const Step &step) {...}
//   PLAY_SCENE("ring_toss", MakeRingToss)
// The step's label, lines and night are the scene's to use. When a Play step
// with .memory set ends, its label becomes a memory Anu has back.
using SceneMaker = std::unique_ptr<Scene> (*)(const Step &step);
bool RegisterPlayScene(const char *name, SceneMaker make);
SceneMaker FindPlayScene(const std::string &name); // nullptr if none
#define PLAY_SCENE(name, maker)                                                \
  static const bool playSceneRegistered_##maker =                             \
      RegisterPlayScene(name, maker);
std::unique_ptr<Scene> MakeIntroScene();
// lookOnly: the page can be looked at but not changed (it waits for later).
// practice: played from the Mechanics page, outside the story; Esc closes it
std::unique_ptr<Scene> MakePuzzleScene(int pageIndex, bool lookOnly = false,
                                       bool practice = false);
std::unique_ptr<Scene> MakeExploreScene(const Step &step);
std::unique_ptr<Scene> MakeBusyScene();
std::unique_ptr<Scene> MakeKiteScene();
std::unique_ptr<Scene> MakeTravelScene();
std::unique_ptr<Scene> MakeCoffeeScene();
std::unique_ptr<Scene> MakeVoiceMessageScene(const Step &step);
std::unique_ptr<Scene> MakeChapterCard(const Chapter &chapter);
// Draws one of the house's places, e.g. behind dialogue (explore_scene.cpp),
// with `people` drawn into it (before night shades it), if any
void DrawPlace(const std::string &place, bool night, float time,
               void (*people)(float time) = nullptr);
// The people (art.cpp), drawn from the feet up. `stirring`: Amma's arm is
// out to a pot on her right; otherwise her hands are together.
void DrawAmma(float x, float feetY, float time, bool stirring);
void DrawAnuSitting(float x, float groundY, float time); // on her heels
// Anu (285 tall) and her friend Meena (278), standing. `hand`: where the
// right hand is, as fractions of their height from the feet ({0.13, 0.5} is
// resting; {0.3, 0.8} reaching out and up). `look`: where they look, -1..1.
void DrawAnu(float x, float feetY, float time, Vector2 hand = {0.13f, 0.5f},
             Vector2 look = {0.2f, 0.3f});
void DrawMeena(float x, float feetY, float time, Vector2 hand = {0.12f, 0.5f},
               Vector2 look = {-0.2f, 0.3f});
// Little Anu, hands out in front (holding the kite reel, or rice flour).
// 95 tall beside Appa in the kite memory; taller when she's nearer
void DrawLittleAnu(float x, float feetY, float time, float height = 95);
void DrawAppa(float x, float feetY, float time);
// Scenery (art.cpp)
void DrawSoftShadow(float x, float y, float width);
void DrawHills(float topY, float bottomY, float rise, int seed, Color color);
void DrawPalm(float x, float groundY, float height, float time, Color trunk,
              Color leaves);
void DrawJasmine(Rectangle area, float time);
void DrawPot(float x, float y, float r, bool brass);
void DrawTiledFloor(float y, Color tile, Color line);

// Comic-book printing (art.cpp): the home is drawn like an old Tamil
// children's comic. Ink outlines are heavier on the lower right, the side
// away from the light; fills are printed a touch off register.
const Color COMIC_INK = {32, 24, 28, 255};      // the black plate
const Color COMIC_PAPER = {252, 244, 222, 255}; // newsprint white
// Halftone: between BeginTone() and EndTone(), every shape is printed as a
// screen of dots (or lines, or paper grain), and its colour's alpha says how
// much ink: 0 none, 1 solid. A gradient gives dots that grow, and
// DrawCircleGradient a glow of dots. `cell` is the dot spacing, in stage units.
// Don't stack two screens with different cells: they make a moire pattern.
enum ToneStyle { TONE_DOTS, TONE_LINES, TONE_GRAIN };
void BeginTone(ToneStyle style = TONE_DOTS, float cell = 6);
void EndTone();
// A rectangle of dots (or lines) from `top` to `bottom` amounts of ink
void DrawToneRect(Rectangle r, Color ink, float top, float bottom,
                  ToneStyle style = TONE_DOTS, float cell = 6);
void DrawNewsprint(); // paper grain over the whole screen
// Brush strokes: thick in the middle, thin where the brush lifts
void InkLine(Vector2 a, Vector2 b, float weight, Color ink = COMIC_INK);
void InkCurve(Vector2 a, Vector2 control, Vector2 b, float weight,
              Color ink = COMIC_INK);
// Outlines, heavier on the lower right
void InkRect(Rectangle r, float weight, Color ink = COMIC_INK);
void InkCircle(Vector2 c, float r, float weight, Color ink = COMIC_INK);
void InkEllipse(Vector2 c, float rx, float ry, float weight,
                Color ink = COMIC_INK);
void InkPoly(const Vector2 *points, int count, float weight,
             Color ink = COMIC_INK); // closed, any shape
// Filled and inked in one go (ComicPoly: convex shapes only)
void ComicRect(Rectangle r, Color fill, float weight);
void ComicCircle(Vector2 c, float r, Color fill, float weight);
void ComicEllipse(Vector2 c, float rx, float ry, Color fill, float weight);
void ComicPoly(const Vector2 *points, int count, Color fill, float weight);
// Hatching: brush strokes slanting up to the right, `gap` apart
void InkHatch(Rectangle area, float gap, float weight, Color ink = COMIC_INK);
void DrawSparkle(Vector2 centre, float size, Color color); // a 4-point glint
void FillTriangle(Vector2 a, Vector2 b, Vector2 c, Color color); // any order
void DrawComicLeaf(Vector2 stem, Vector2 tip, float width, Color fill,
                   float weight);
// Comic versions of the scenery above, for the home
void DrawComicShadow(float x, float y, float width);
void DrawComicHills(float topY, float bottomY, float rise, int seed,
                    Color fill, Color shade);
void DrawComicPalm(float x, float groundY, float height, float time,
                   Color trunk, Color leaves);
void DrawComicJasmine(Rectangle area, float time);
void DrawComicPot(float x, float y, float r, bool brass);

//------------------------------------------------------------------------------
// Kolam puzzle rules (kolam.cpp)
//------------------------------------------------------------------------------
struct GridPoint {
  int x, y;
  bool operator==(const GridPoint &o) const { return x == o.x && y == o.y; }
};

class KolamGrid {
public:
  int width = 0;  // dots across
  int height = 0; // dots down

  bool Load(const Page &page, std::string *error = nullptr);
  void Reset() { rows = original; }

  bool IsBetweenDots(int x, int y) const; // a spot that can hold a wall
  bool IsWall(int x, int y) const;        // the border always counts as a wall
  bool IsEditable(int x, int y) const;
  void Toggle(int x, int y); // also toggles the mirrored spots
  std::vector<GridPoint> MirrorGroup(int x, int y) const;

  // The closed lines of the kolam. Solved = exactly one loop.
  std::vector<std::vector<GridPoint>> Loops() const;
  // Tries every arrangement of editable walls and returns how many make one
  // loop. With apply = true the grid is left solved.
  int CountSolutions(bool apply);

private:
  std::vector<std::string> rows;     // the layout, changed as the player plays
  std::vector<std::string> original; // the layout as written in story.cpp
  bool symmetric = false;
};

// Smooth screen-space curves for each loop. `origin` is where grid point (0,0)
// is on screen, `cell` the distance between dots, and `wobble` makes the line
// look hand-drawn.
std::vector<std::vector<Vector2>> LoopPaths(const KolamGrid &grid,
                                            Vector2 origin, float cell,
                                            float wobble = 0.0f);
// Draws the first `count` points of a path (closed = join the end to start)
void DrawPath(const std::vector<Vector2> &path, size_t count, bool closed,
              float thick, Color color);
// Prints whether every page and story step is fine. Returns the problem count.
int CheckPages();
// Testing (autotest.cpp): scripted input and screenshots, from the command
// line. AutotestFrame() plays the input due this frame; AutotestShots() saves
// the screenshots due, and is true once the last one is taken.
bool AutotestParse(int argc, char **argv);
void AutotestWindowSize(int *width, int *height); // 1280x720, or --size
void AutotestFrame();
bool AutotestShots();

//------------------------------------------------------------------------------
// Settings, the save game, and what Anu has found (save.cpp)
//------------------------------------------------------------------------------
struct Settings {
  int masterVolume = 80; // 0..100
  int musicVolume = 70;  // 0..100
  int textSpeed = 2;     // 0 Slow, 1 Normal, 2 Fast, 3 Instant
  // 0 Normal, 1 Large, 2 Larger: all the game's words, bigger. A phone's
  // screen is small, so phones start at Large.
#if defined(PLATFORM_PHONE)
  int textSize = 1;
#else
  int textSize = 0;
#endif
  bool fullscreen = false;
  // 0 = 16:9, 1 = fill the screen, 2 = 4:3, 3 = square. Phones fill their
  // (wide) screen; computers start at 16:9.
#if defined(PLATFORM_PHONE)
  int screenShape = 1;
#else
  int screenShape = 0;
#endif
  // When the window's shape is different, fill the sides with a soft glow of
  // the scene (true) or with black bars (false)
  bool softEdges = true;
  // How often the player has done each gesture (MOVE_TAP...): teaching
  // hints rest once it's done a few times
  int gestureUses[4] = {0, 0, 0, 0};
};
extern Settings settings;

const int TEXT_SPEED_COUNT = 4;
const char *TextSpeedName();
const int TEXT_SIZE_COUNT = 3;
const char *TextSizeName(); // (settings_screen.cpp)
// The Settings screen (settings_screen.cpp). Update returns true when the
// player goes back; main.cpp then saves the settings.
void OpenSettingsScreen();
bool UpdateSettingsScreen();
void DrawSettingsScreen();
float TextCharsPerSecond();
void LoadSettings();
void SaveSettings();

// What Anu has found again so far. Saved with the game.
struct Progress {
  std::vector<std::string> memories; // memory names (Hotspot::memory)
  // Work calls at home, answered or not. Neither changes the story; the
  // total only decides which call comes next.
  int callsAnswered = 0;
  int callsIgnored = 0;
};
extern Progress progress;
bool HasMemory(const std::string &memory);
void AddMemory(const std::string &memory);
int MemoryCount(); // how many memories there are in the whole story
bool IsFlashback(const Step &step); // a Memory(...): it counts as one
// How much colour the present-day world has: a little muted at first, full
// colour once Anu has found every memory. Work calls don't change it.
float WorldColour();

// Where the player is: a chapter (index into Chapters()) and a step in it.
// Step -1 is the chapter's title card.
struct SaveGame {
  bool exists = false;
  int chapter = 0;
  int step = -1;
  std::string label;     // e.g. "Chapter 1: Home"
  long long savedAt = 0; // unix time
  Progress progress;     // what Anu had found when this was saved
};
SaveGame LoadGame();
void SaveGameAt(int chapter, int step);
void DeleteSave();
std::string TimeAgo(long long unixTime); // "5 min ago"

// Any chapter can be played from Chapters (on Home and in the pause menu).
// The ones the player has reached are remembered (even after New Game or the
// end), with the memories Anu had when she first got there, so replaying one
// starts with those.
bool ChapterUnlocked(int chapter); // reached before?
void UnlockChapter(int chapter);   // also remembers today's Progress for it
// What Anu has at the start of a chapter: what she had when she first got
// there, or for a chapter not reached yet, every memory from the ones before
Progress ProgressAtChapterStart(int chapter);

//------------------------------------------------------------------------------
// Fonts, menus, buttons and drawing helpers (ui.cpp)
//------------------------------------------------------------------------------
// Two fonts (in assets/fonts): UI_FONT is a rounded font for menus and
// dialogue, HAND_FONT is handwriting for the notes in the kolam book.
// `pixelScale` is how many real pixels one of the game's 720x720 units covers;
// the letters are made that much bigger so text stays sharp on big screens.
enum FontStyle { UI_FONT, HAND_FONT };
void LoadFonts(float pixelScale);
void UnloadFonts();
// Like raylib's DrawText/MeasureText, but with our fonts
void DrawString(const std::string &text, float x, float y, float size,
                Color color, FontStyle font = UI_FONT);
float MeasureString(const std::string &text, float size,
                    FontStyle font = UI_FONT);
void DrawTextCentered(const std::string &text, float centerX, float y,
                      float size, Color color, FontStyle font = UI_FONT);
void DrawTextWrapped(const std::string &text, float x, float y, float size,
                     float maxWidth, Color color, FontStyle font = UI_FONT);

struct MenuItem {
  std::string label;
  std::string detail; // optional small second line
  bool enabled = true;
};

// A vertical list of buttons. Works with the mouse and with Up/Down + Enter.
// A long list scrolls: with `rows` set, only that many items show at a time
// (Up/Down, the mouse wheel or the arrows above and below them scroll), and
// the last item (Back) always shows below them.
struct Menu {
  std::vector<MenuItem> items;
  int selected = 0;
  float top = 0;
  float width = 320;
  float centerX = STAGE_SIZE / 2.0f; // where its middle is (Set() resets it)
  int rows = 0;   // how many items show at once; 0 = all of them
  int scroll = 0; // the first item shown, when the list scrolls

  void Set(std::vector<MenuItem> newItems, float newTop, float newWidth = 320,
           int newRows = 0);
  int Update(); // returns the index of the chosen item, or -1
  void Draw() const;
  void Select(int i);      // selects item i, scrolling to it
  bool Shown(int i) const; // false while item i is scrolled out of sight
  Rectangle ItemRect(int i) const;
};

// A single button: a rounded box with a centred label
void DrawButton(Rectangle rect, const std::string &label, Color fill);
bool Clicked(Rectangle rect); // was the mouse clicked inside it this frame?
bool ContinuePressed();       // click, Space or Enter: "go on"
bool BackPressed();           // Esc, Backspace or Android's back button
// Played by touch (a phone, or "--touch" when testing): the words say "Tap"
// instead of "Click", and the phone's Back button instead of Esc
bool TouchUi();
// Teaching the gestures (gestures.cpp). DrawGesture: an animated hand doing
// it at `at` (drag and swipe go along `way`), with its word under it.
// GestureDone: the player did it; after a few times it's GestureLearned,
// and first-time hints for it rest. Swiped: a swipe ended this frame.
enum Move { MOVE_TAP, MOVE_HOLD, MOVE_DRAG, MOVE_SWIPE,
               MOVE_COUNT };
void UpdateGestures(); // at the start of every frame
bool Swiped();
bool GestureLearned(Move g);
void GestureDone(Move g);
// (`ink`: the colour of its word and marks; FLOUR suits a dark scene)
void DrawGesture(Move g, Vector2 at, float time, float alpha = 1,
                 Vector2 way = {-1, 0}, Color ink = FLOUR);
void DrawHandIcon(Vector2 tip, float size, float alpha); // (cursor.cpp)
// Quick taps on a phone (touch.cpp): InitTouch() after the window opens,
// UpdateTouch() at the start of every frame
void InitTouch();
void UpdateTouch();
void SetTouchUi(bool on);
const char *Tap(); // "Tap" or "Click", to start a sentence
const char *tap(); // "tap" or "click", inside one
// A round button in the top-left corner of the screen, while playing: pause
// (in the story) or back (a game from the Mechanics page). Returns true when
// it's tapped.
bool NavButton();
// The (i) button beside it: "How to play" (help.cpp)
bool InfoButton();
void DrawInfoButton();
bool UpdateHelpScreen(); // true when closed
void DrawHelpScreen(float time);

// Lines that have just been rubbed out (by Undo or the eraser), fading away
struct RubOut {
  struct Fading {
    std::vector<Vector2> path;
    float thick;
    Color colour;
    float left; // seconds
  };
  std::vector<Fading> fading;
  void Fade(const std::vector<Vector2> &path, float thick, Color colour);
  void DrawFading(); // and counts their time down
};
// The eraser: how close to a line a finger must pass to wipe it
const float ERASER_RADIUS = 16;
// A ring round the finger while the eraser is pressed down
void DrawEraserRing();
// How far point p is from the line from a to b
float DistanceToSegment(Vector2 p, Vector2 a, Vector2 b);
// How far point p is from a path, and (if `index`) the point of it nearest
float DistanceToPath(Vector2 p, const std::vector<Vector2> &path,
                     size_t *index = nullptr);
void DrawNavButton(bool back);

// The dialogue box used by story scenes and while exploring. `typed` is how
// many characters are shown so far (the typewriter effect).
// `top`: where the box starts; it sits low (430) unless people would be
// behind it, then high (20)
void DrawDialogueBox(const Line &line, float typed, float top = 430);
// Types out `lines` one at a time; ContinuePressed() first shows the whole
// line, then moves to the next one. Returns true once the last line is
// dismissed.
// What the line being said waits for: words still typing, the player to
// hold (a hard line), or to go on (on a phone: a swipe; a tap only shows the
// whole line). AdvanceDialogue sets it; main.cpp draws DrawDialogueHint.
enum DialogueWait { WAIT_NONE, WAIT_TYPING, WAIT_HOLD, WAIT_GO_ON };
DialogueWait DialogueWaiting();
void UpdateDialogueWait(); // at the start of every frame
void DrawDialogueHint(float time);
bool AdvanceDialogue(const std::vector<Line> &lines, size_t &current,
                     float &typed);

Color Faded(Color color, float alpha);
// 0 before `from`, 1 after `to`, and a smooth slope in between: for fading
// and easing things over time
float Between(float x, float from, float to);
void DrawPanel(Rectangle rect);
void DrawDim(); // darken everything drawn so far (the whole screen)
// A stripe across the whole screen's width, from `y` down: for backgrounds
// (sky, walls, floors), so they reach the edges on wide screens
void DrawBand(float y, float height, Color color);
void DrawBandGradient(float y, float height, Color top, Color bottom);
void DrawHomeBackground(float time); // night sky with kolams drawing themselves
// The home screen's layout. On a wide screen (a phone held sideways, or a
// 16:9 window) the title and menu sit on the left and the kolam on the
// right; on a narrower one they're stacked in the middle, the kolam between
// the title and the menu.
bool HomeIsWide();
float HomeColumnX();         // the middle of the title and menu
Rectangle HomeKolamArea();   // where the kolam draws itself

// The Mechanics page (mechanics.cpp): every page of the kolam book, all open,
// to play outside the story. Update returns the page chosen, MECHANICS_BACK,
// or -1 for nothing yet.
const int MECHANICS_BACK = -2;
const int MECHANICS_KINDS = 100; // + a KolamKind: one of the other kolams
void OpenMechanicsPage(int selectPage);
int UpdateMechanicsPage();
void DrawMechanicsPage(float time);
void MarkPageSolved(int page); // a tick on its card, until the game closes

// The other kinds of kolam (kolam_kinds.cpp), each a small game of its own
// on the Mechanics page, with Amma's note as a guide
enum KolamKind {
  KOLAM_PULLI,
  KOLAM_PADI,
  KOLAM_POO,
  KOLAM_LOTUS,
  KOLAM_FREEHAND,
  KOLAM_KIND_COUNT
};
const char *KolamKindName(int kind);           // "Pulli kolam"
const char *KolamKindTag(int kind);            // "join the dots"
void DrawAmmasKolam(int kind, Rectangle area); // her drawing of one
std::unique_ptr<Scene> MakeKolamKindScene(int kind);

// The mouse pointer (cursor.cpp): Anu's hand pinching rice flour
void InitCursor();         // hides the system pointer
void UpdateCursor();       // at the start of each frame
void PointAtSomething();   // this frame the mouse is over something clickable
void SceneDrawsOwnCursor(); // this frame a scene draws its own (a crayon)
// After the frame is drawn: `stageOrigin` is where stage (0, 0) is in the
// window, `scale` the window pixels per stage unit
void DrawCursor(Vector2 stageOrigin, float scale);

//------------------------------------------------------------------------------
// Sounds (audio.cpp), from assets/sounds/<name>.ogg
//------------------------------------------------------------------------------
// The place's background sound, looping; changing it fades one into the other.
// Calling it again with the same name only changes the volume. "" = silence.
void PlayAmbience(const std::string &name, float volume = 0.6f);
bool AmbiencePlaying();
void PlayEffect(const std::string &name, float volume = 1, float pitch = 1);
// Keeps a sound going while this is called every frame (pouring, drawing)
void PlayLoop(const std::string &name, float volume = 1);
void UpdateAudio(); // once a frame, in main.cpp
void UnloadAudio();

//------------------------------------------------------------------------------
// Files from assets/, built into the game (ui.cpp + the generated assets.cpp)
//------------------------------------------------------------------------------
struct EmbeddedFile {
  const char *path; // e.g. "assets/fonts/Kalam-Bold.ttf"
  const unsigned char *data;
  unsigned int size;
};
// The list of files, in the assets.cpp that xmake.lua generates
extern const EmbeddedFile EMBEDDED_FILES[];
// Finds a file by its path, or returns nullptr if it isn't there
const EmbeddedFile *FindAsset(const std::string &path);

// EMBED(name, "path") copies a file into the game when it compiles, using the
// assembler's .incbin (much faster than turning the file into a C++ array), and
// names its first byte and the byte after its last: name_start and name_end.
// It ends by switching back to the code section (.text), which works on every
// system; ".previous" would be neater, but Windows' object files reject it.
#if defined(_WIN32)
#define EMBED_SECTION ".section .rdata,\"dr\"\n"
#elif defined(__APPLE__)
#define EMBED_SECTION ".const_data\n"
#else
#define EMBED_SECTION ".section .rodata\n"
#endif
#define EMBED(name, path)                                                      \
  __asm__(EMBED_SECTION ".balign 16\n" #name "_start:\n.incbin \"" path        \
                        "\"\n" #name "_end:\n.text\n");                        \
  extern const unsigned char name##_start[] __asm__(#name "_start");           \
  extern const unsigned char name##_end[] __asm__(#name "_end");
