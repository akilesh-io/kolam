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
  List // Amma's notebook, then her shopping list: the end of Chapter 1
};

// One step of the story. The save game remembers which step you are on.
const bool NIGHT = true, DAY = false; // for Step::night and At()

struct Step {
  StepType type;
  std::string label;             // shown in the pause menu
  std::vector<Line> lines;       // Talk: the dialogue
  int page = -1;                 // Puzzle, List: index into Pages()
  bool memory = false;           // Talk: a flashback, shown in full colour
  std::vector<Hotspot> hotspots; // Explore: what can be clicked
  bool night = false;            // Talk/Explore: evening or night lighting
  std::string place;             // Talk: where it happens ("" = no place)

  // Talk("...", {...}).At("terrace", NIGHT): the dialogue plays over that
  // place (see DrawPlace). Places: front_step, hall, kitchen, terrace.
  Step At(const std::string &where, bool atNight) const {
    Step step = *this;
    step.place = where;
    step.night = atNight;
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
std::unique_ptr<Scene> MakeIntroScene();
std::unique_ptr<Scene> MakePuzzleScene(int pageIndex);
std::unique_ptr<Scene> MakeExploreScene(const Step &step);
std::unique_ptr<Scene> MakeBusyScene();
std::unique_ptr<Scene> MakeKiteScene();
std::unique_ptr<Scene> MakeTravelScene();
std::unique_ptr<Scene> MakeCoffeeScene();
std::unique_ptr<Scene> MakeVoiceMessageScene(const Step &step);
std::unique_ptr<Scene> MakeChapterCard(const Chapter &chapter);
// Draws one of the house's places, e.g. behind dialogue (explore_scene.cpp)
void DrawPlace(const std::string &place, bool night, float time);
// The people (art.cpp), drawn from the feet up. `stirring`: Amma's arm is
// out to a pot on her right; otherwise her hands are together.
void DrawAmma(float x, float feetY, float time, bool stirring);
void DrawAnuSitting(float x, float groundY, float time); // on her heels
void DrawLittleAnu(float x, float feetY, float time);    // holding the reel
void DrawAppa(float x, float feetY, float time);
// Scenery (art.cpp)
void DrawSoftShadow(float x, float y, float width);
void DrawHills(float topY, float bottomY, float rise, int seed, Color color);
void DrawPalm(float x, float groundY, float height, float time, Color trunk,
              Color leaves);
void DrawJasmine(Rectangle area, float time);
void DrawPot(float x, float y, float r, bool brass);
void DrawTiledFloor(float y, Color tile, Color line);

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

//------------------------------------------------------------------------------
// Settings, the save game, and what Anu has found (save.cpp)
//------------------------------------------------------------------------------
struct Settings {
  int masterVolume = 80; // 0..100
  int musicVolume = 70;  // 0..100
  int textSpeed = 2;     // 0 Slow, 1 Normal, 2 Fast, 3 Instant
  bool fullscreen = false;
  int screenShape = 0; // 0 = 16:9, 1 = fill the screen, 2 = 4:3, 3 = square
  // When the window's shape is different, fill the sides with a soft glow of
  // the scene (true) or with black bars (false)
  bool softEdges = true;
};
extern Settings settings;

const int TEXT_SPEED_COUNT = 4;
const char *TextSpeedName();
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

// Chapters the player has reached stay unlocked (even after New Game or the
// end), so they can be replayed from Home -> Chapters, starting with the
// memories Anu had when she first got there.
bool ChapterUnlocked(int chapter);
void UnlockChapter(int chapter); // also remembers today's Progress for it
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
struct Menu {
  std::vector<MenuItem> items;
  int selected = 0;
  float top = 0;
  float width = 320;

  void Set(std::vector<MenuItem> newItems, float newTop, float newWidth = 320);
  int Update(); // returns the index of the chosen item, or -1
  void Draw() const;
  Rectangle ItemRect(int i) const;
};

// A single button: a rounded box with a centred label
void DrawButton(Rectangle rect, const std::string &label, Color fill);
bool Clicked(Rectangle rect); // was the mouse clicked inside it this frame?
bool ContinuePressed();       // click, Space or Enter: "go on"
bool BackPressed();           // Esc, Backspace or Android's back button

// The dialogue box used by story scenes and while exploring. `typed` is how
// many characters are shown so far (the typewriter effect).
void DrawDialogueBox(const Line &line, float typed);
// Types out `lines` one at a time; ContinuePressed() first shows the whole
// line, then moves to the next one. Returns true once the last line is
// dismissed.
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
