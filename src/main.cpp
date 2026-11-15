// main.cpp - the window, the main loop and every screen of the game.
//
// The game is always on exactly one screen:
//   Intro -> Home -> (ConfirmNewGame) -> Playing <-> Paused
// Settings can be opened from Home or Paused, and goes back to where it came
// from. While Playing, `scene` is the current step of the current chapter
// (see Chapters() in story.cpp).
// Progress is saved automatically at the start of every step.
#include <algorithm>
#include <cstring>

#include "game.h"

enum class Screen {
  Intro,
  Home,
  ConfirmNewGame,
  Chapters, // chapter select
  Settings,
  Playing,
  Paused
};

// The buttons of each menu, in order (the numbers Menu::Update() returns)
enum HomeRow {
  HOME_CONTINUE,
  HOME_NEW_GAME,
  HOME_CHAPTERS,
  HOME_SETTINGS,
  HOME_EXIT
};
enum ConfirmRow { CONFIRM_KEEP, CONFIRM_START_OVER };
// Settings -> Screen shape (settings.screenShape)
enum ScreenShape {
  SHAPE_16_9,
  SHAPE_FILL,
  SHAPE_4_3,
  SHAPE_SQUARE,
  SHAPE_COUNT
};
const char *SHAPE_NAMES[SHAPE_COUNT] = {"16:9", "Fill screen", "4:3", "Square"};
enum PauseRow { PAUSE_RESUME, PAUSE_SAVE, PAUSE_SETTINGS, PAUSE_MAIN_MENU };
enum SettingsRow {
  MASTER_VOLUME,
  MUSIC_VOLUME,
  TEXT_SPEED,
  DISPLAY,
  SCREEN_SHAPE,
  SCREEN_EDGES,
  RESET,
  BACK
};

static Screen screen = Screen::Intro;
static Screen settingsReturnTo =
    Screen::Home;                    // where "Back" goes from Settings
static std::unique_ptr<Scene> scene; // the intro, or the current story step
static int chapter = 0;              // index into Chapters()
static int step = -1;                // step in the chapter; -1 = its title card
static SaveGame save;                // what "Continue" loads
static Menu menu;                    // the buttons of the current screen
static float homeTime = 0;           // drives the home screen animation
static float savedMessageTimer = 0;  // "Game saved" in the pause menu
static bool quit = false;
static bool devMode = false; // "./run.sh --dev" (see Developer mode below)
static Music music;
static Shader colourShader;      // turns the screen grey or colourful
static float shownColour = 1.0f; // eases towards the scene's colour

// The game is drawn on a square `canvas`, as big in pixels as the game really
// appears on screen, so nothing is stretched and text stays sharp. The game's
// code always thinks in 720x720 units; a camera zooms that up to the canvas.
float screenWidth = 1280; // see game.h
static RenderTexture2D canvas;
const float MAX_PIXELS_PER_UNIT = 3; // 2160 px tall; huge screens stretch a bit
// For "soft edges": the canvas shrunk smaller and smaller, which blurs it
static RenderTexture2D blurSteps[3];
const Color EDGE_TINT = {80, 80, 80, 255}; // how dark the soft edges are

// The screen shader, run on the finished picture: `colour` mixes each pixel
// between grey (0) and its real colour (1). It also makes every pixel fully
// opaque, so see-through drawing never darkens the screen. Phones use
// OpenGL ES, which needs the older shader language.
#define SHADER_BODY                                                            \
  "uniform sampler2D texture0;\n"                                              \
  "uniform float colour;\n"                                                    \
  "vec3 Shade(vec2 uv) {\n"                                                    \
  "  vec3 c = TEXTURE(texture0, uv).rgb;\n"                                    \
  "  float grey = dot(c, vec3(0.299, 0.587, 0.114));\n"                        \
  "  return mix(vec3(grey), c, colour);\n"                                     \
  "}\n"
#if defined(PLATFORM_ANDROID)
static const char *COLOUR_SHADER =
    "#version 100\nprecision mediump float;\n#define TEXTURE texture2D\n"
    "varying vec2 fragTexCoord;\n" SHADER_BODY
    "void main() { gl_FragColor = vec4(Shade(fragTexCoord), 1.0); }\n";
#else
static const char *COLOUR_SHADER =
    "#version 330\n#define TEXTURE texture\n"
    "in vec2 fragTexCoord;\nout vec4 finalColor;\n" SHADER_BODY
    "void main() { finalColor = vec4(Shade(fragTexCoord), 1.0); }\n";
#endif

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
static std::vector<MenuItem> SettingsItems() {
  const std::string display = settings.fullscreen ? "Fullscreen" : "Windowed";
  return {
      {"Master volume:  < " + std::to_string(settings.masterVolume) + "% >"},
      {"Music volume:  < " + std::to_string(settings.musicVolume) + "% >"},
      {"Text speed:  < " + std::string(TextSpeedName()) + " >"},
      {"Display:  < " + display + " >"},
      {"Screen shape:  < " + std::string(SHAPE_NAMES[settings.screenShape]) +
       " >"},
      {std::string("Screen edges:  < ") +
       (settings.softEdges ? "Soft glow" : "Black bars") + " >"},
      {"Reset to defaults"},
      {"Back"},
  };
}

static int ClampVolume(int volume) {
  return std::max(0, std::min(100, volume));
}

static void ChangeSetting(int row, int direction) {
  if (row == MASTER_VOLUME)
    settings.masterVolume = ClampVolume(settings.masterVolume + direction * 10);
  if (row == MUSIC_VOLUME)
    settings.musicVolume = ClampVolume(settings.musicVolume + direction * 10);
  if (row == TEXT_SPEED)
    settings.textSpeed =
        (settings.textSpeed + direction + TEXT_SPEED_COUNT) % TEXT_SPEED_COUNT;
  if (row == DISPLAY)
    settings.fullscreen = !settings.fullscreen;
  if (row == SCREEN_SHAPE)
    settings.screenShape =
        (settings.screenShape + direction + SHAPE_COUNT) % SHAPE_COUNT;
  if (row == SCREEN_EDGES)
    settings.softEdges = !settings.softEdges;
}

// Volume and fullscreen are re-applied every frame, so changes show at once
static void ApplySettings() {
  SetMasterVolume(settings.masterVolume / 100.0f);
  // the song plays softer under a place's own sounds
  const float duck = AmbiencePlaying() ? 0.35f : 1.0f;
  SetMusicVolume(music, duck * settings.musicVolume / 100.0f);
#if !defined(PLATFORM_ANDROID) // a phone is always fullscreen
  if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE) != settings.fullscreen)
    ToggleBorderlessWindowed();
#endif
}

//------------------------------------------------------------------------------
// Moving between screens
//------------------------------------------------------------------------------
static void ShowScreen(Screen next) {
  screen = next;
  SetMouseCursor(MOUSE_CURSOR_DEFAULT);

  if (next == Screen::Home) {
    save = LoadGame();
    MenuItem continueItem = {"Continue", "No saved game yet", save.exists};
    if (save.exists)
      continueItem.detail = save.label + "  -  saved " + TimeAgo(save.savedAt);
    menu.Set({continueItem, {"New Game"}, {"Chapters"}, {"Settings"}, {"Exit"}},
             386);
  }
  if (next == Screen::ConfirmNewGame)
    menu.Set({{"Keep my saved game"}, {"Start over"}}, 412, 280);
  if (next ==
      Screen::Chapters) { // every chapter; the ones reached can be played
    std::vector<MenuItem> items;
    for (int i = 0; i < (int)Chapters().size(); i++) {
      const Chapter &c = Chapters()[i];
      const bool reached = ChapterUnlocked(i);
      items.push_back({c.heading + ": " + c.title,
                       reached ? c.where : "Not reached yet", reached});
    }
    items.push_back({"Back"});
    menu.Set(items, 190, 460);
  }
  if (next == Screen::Settings)
    menu.Set(SettingsItems(), 150, 440);
  if (next == Screen::Paused)
    menu.Set({{"Resume"}, {"Save game"}, {"Settings"}, {"Main menu"}}, 290,
             280);
}

// Plays a step of a chapter; step -1 is the chapter's title card
static void PlayStep(int chapterIndex, int stepIndex) {
  chapter = chapterIndex;
  step = stepIndex;
  if (step < 0) {
    UnlockChapter(chapter); // it can now be replayed from Home -> Chapters
    scene = MakeChapterCard(Chapters()[chapter]);
  } else {
    scene = MakeScene(Chapters()[chapter].steps[step]);
  }
  SaveGameAt(chapter, step); // autosave
  ShowScreen(Screen::Playing);
}

// After a step: the next step, or the next chapter, or the end of the story
static void NextStep() {
  if (step + 1 < (int)Chapters()[chapter].steps.size()) {
    PlayStep(chapter, step + 1);
  } else if (chapter + 1 < (int)Chapters().size()) {
    PlayStep(chapter + 1, -1);
  } else { // the end: back home (chapters stay unlocked for replaying)
    DeleteSave();
    scene.reset();
    ShowScreen(Screen::Home);
  }
}

// "Chapter 1: Home - The night bus", for the pause menu
static std::string WhereLabel() {
  const Chapter &c = Chapters()[chapter];
  const std::string label = c.heading + ": " + c.title;
  return step < 0 ? label : label + "  -  " + c.steps[step].label;
}

static void StartNewGame() {
  progress = Progress();
  PlayStep(0, -1);
}

static void OpenSettings(Screen from) {
  settingsReturnTo = from;
  ShowScreen(Screen::Settings);
}

//------------------------------------------------------------------------------
// Update: react to input on the current screen
//------------------------------------------------------------------------------
static void UpdateSettingsScreen() {
  int direction = 0; // -1 = lower / previous, +1 = higher / next
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    direction = -1;
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    direction = 1;

  const int chosen = menu.Update();
  if (BackPressed() || chosen == BACK) {
    SaveSettings();
    ShowScreen(settingsReturnTo);
    return;
  }
  if (chosen == RESET)
    settings = Settings();
  if (chosen >= 0) {
    // Clicking the left half of a row turns it down, the right half up
    const bool leftHalf = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                          GetMousePosition().x < STAGE_SIZE / 2.0f;
    direction = leftHalf ? -1 : 1;
  }
  if (direction != 0)
    ChangeSetting(menu.selected, direction);
  menu.items = SettingsItems(); // refresh the values shown
}

static void UpdateScreen() {
  const float dt = GetFrameTime();

  switch (screen) {
  case Screen::Intro:
    scene->Update();
    if (scene->done) {
      scene.reset();
      ShowScreen(Screen::Home);
    }
    break;

  case Screen::Home:
    homeTime += dt;
    switch (menu.Update()) {
    case HOME_CONTINUE:
      progress = save.progress;
      PlayStep(save.chapter, save.step);
      break;
    case HOME_NEW_GAME:
      if (save.exists)
        ShowScreen(Screen::ConfirmNewGame);
      else
        StartNewGame();
      break;
    case HOME_CHAPTERS:
      ShowScreen(Screen::Chapters);
      break;
    case HOME_SETTINGS:
      OpenSettings(Screen::Home);
      break;
    case HOME_EXIT:
      quit = true;
      break;
    }
    break;

  case Screen::ConfirmNewGame: {
    homeTime += dt;
    const int chosen = menu.Update();
    if (BackPressed() || chosen == CONFIRM_KEEP)
      ShowScreen(Screen::Home);
    if (chosen == CONFIRM_START_OVER)
      StartNewGame();
    break;
  }

  case Screen::Chapters: {
    homeTime += dt;
    const int chosen = menu.Update();
    if (BackPressed() || chosen == (int)Chapters().size()) {
      ShowScreen(Screen::Home);
    } else if (chosen >= 0) { // replay it, with what Anu had found by then
      progress = ProgressAtChapterStart(chosen);
      PlayStep(chosen, -1);
    }
    break;
  }

  case Screen::Settings:
    homeTime += dt;
    UpdateSettingsScreen();
    break;

  case Screen::Playing:
    if (BackPressed()) {
      ShowScreen(Screen::Paused);
      break;
    }
    scene->Update();
    if (scene->done)
      NextStep();
    break;

  case Screen::Paused: {
    savedMessageTimer -= dt;
    const int chosen = menu.Update();
    if (BackPressed() || chosen == PAUSE_RESUME)
      ShowScreen(Screen::Playing);
    if (chosen == PAUSE_SAVE) {
      SaveGameAt(chapter, step);
      savedMessageTimer = 2.5f;
    }
    if (chosen == PAUSE_SETTINGS)
      OpenSettings(Screen::Paused);
    if (chosen == PAUSE_MAIN_MENU) { // progress was saved when this step began
      scene.reset();
      ShowScreen(Screen::Home);
    }
    break;
  }
  }
}

//------------------------------------------------------------------------------
// Draw: everything is drawn on the 720x720 canvas
//------------------------------------------------------------------------------
static void DrawTitle() {
  DrawTextCentered("Kolam", 360, 30, 84, FLOUR);
  DrawTextCentered("a story about finding your way back", 360, 116, 22, MUTED,
                   HAND_FONT);
}

static void DrawScreen() {
  switch (screen) {
  case Screen::Intro:
  case Screen::Playing:
    scene->Draw();
    break;

  case Screen::Home:
    DrawHomeBackground(homeTime);
    DrawTitle();
    menu.Draw();
#if defined(PLATFORM_ANDROID)
    DrawTextCentered("The Back button pauses the game", 360, 688, 15,
                     Faded(MUTED, 0.7f));
#else
    DrawTextCentered("Esc pauses the game  -  F11 toggles fullscreen", 360, 688,
                     15, Faded(MUTED, 0.7f));
#endif
    break;

  case Screen::ConfirmNewGame:
    DrawHomeBackground(homeTime);
    DrawTitle();
    DrawDim();
    DrawPanel({160, 260, 400, 280});
    DrawTextCentered("Start a new game?", 360, 280, 30, FLOUR);
    DrawTextCentered("This replaces your saved game:", 360, 326, 17, MUTED);
    DrawTextCentered(save.label, 360, 348, 20, FLOUR);
    menu.Draw();
    break;

  case Screen::Chapters:
    DrawHomeBackground(homeTime);
    DrawDim();
    DrawPanel({130, 120, 460, 480});
    DrawTextCentered("Chapters", 360, 136, 36, FLOUR);
    menu.Draw();
    break;

  case Screen::Settings:
    if (settingsReturnTo == Screen::Paused)
      scene->Draw();
    else
      DrawHomeBackground(homeTime);
    DrawDim();
    DrawPanel({130, 76, 460, 600});
    DrawTextCentered("Settings", 360, 88, 36, FLOUR);
    menu.Draw();
    DrawTextCentered("Left/Right or click to change  -  Esc to go back", 360,
                     640, 15, MUTED);
    break;

  case Screen::Paused:
    scene->Draw();
    DrawDim();
    DrawPanel({200, 180, 320, 400});
    DrawTextCentered("Paused", 360, 196, 38, FLOUR);
    DrawTextCentered(WhereLabel(), 360, 244, 15, MUTED);
    menu.Draw();
    if (savedMessageTimer > 0) {
      DrawTextCentered("Game saved", 360, 526, 22,
                       Faded(FLOUR, savedMessageTimer));
      DrawTextCentered("Continue restarts this scene", 360, 552, 15,
                       Faded(MUTED, savedMessageTimer));
    }
    break;
  }

  if (devMode) // a reminder in the screen's bottom-right corner
    DrawString("DEV  F5: restart", FullScreen().x + FullScreen().width - 130,
               STAGE_SIZE - 20, 14, Faded(GRAY, 0.8f));
#if defined(_DEBUG) // which story step this is, in the corner
  if (screen == Screen::Playing)
    DrawString(TextFormat("chapter %d, step %d: %s", chapter, step,
                          WhereLabel().c_str()),
               FullScreen().x + 4, 2, 13, GRAY);
#endif
}

//------------------------------------------------------------------------------
// Developer mode ("./run.sh --dev"): saving a file in src/ or assets/, or
// pressing F5, closes the game so run.sh can rebuild it and open it again
// right where you were. The game tells run.sh how to reopen through a file.
//------------------------------------------------------------------------------
static long newestChange = 0;   // the newest file time seen in src/ and assets/
static float checkTimer = 0;    // how long until we look at the files again
static bool restartNow = false; // ends the main loop
const char *DEV_RESTART_FILE = "build/dev-restart";

// The time of the most recently changed file the game is built from
static long NewestFileChange() {
  long newest = GetFileModTime("xmake.lua");
  for (const char *folder : {"src", "assets"}) {
    FilePathList files = LoadDirectoryFilesEx(folder, nullptr, true);
    for (unsigned int i = 0; i < files.count; i++)
      newest = std::max(newest, GetFileModTime(files.paths[i]));
    UnloadDirectoryFiles(files);
  }
  return newest;
}

static void UpdateDevMode() {
  checkTimer -= GetFrameTime();
  bool changed = false;
  if (checkTimer <= 0) { // twice a second is plenty
    checkTimer = 0.5f;
    changed = NewestFileChange() > newestChange;
  }
  if (!IsKeyPressed(KEY_F5) && !changed)
    return;
  // In the story: save, so the next run carries on from this scene
  const bool inStory =
      screen == Screen::Playing || screen == Screen::Paused ||
      (screen == Screen::Settings && settingsReturnTo == Screen::Paused);
  if (inStory)
    SaveGameAt(chapter, step);
  SaveFileText(DEV_RESTART_FILE, (char *)(inStory ? "resume" : "home"));
  restartNow = true;
}

// When the game is opened in developer mode: skip the intro, and carry on
// from the saved scene if run.sh says we were in the middle of the story
static void StartDevMode(bool resume) {
  devMode = true;
  newestChange = NewestFileChange();
  scene.reset();
  const SaveGame saved = LoadGame();
  if (resume && saved.exists) {
    progress = saved.progress;
    PlayStep(saved.chapter, saved.step);
  } else {
    ShowScreen(Screen::Home);
  }
}

//------------------------------------------------------------------------------
// One frame: update, draw on the canvas, then show the canvas in the window
//------------------------------------------------------------------------------

// How wide the game's screen is (in stage units) for Settings -> Screen shape
static float ShapeWidth(float windowW, float windowH) {
  switch (settings.screenShape) {
  case SHAPE_FILL: // as wide as the window, from square up to very wide
    return std::max(720.0f, std::min(1680.0f, STAGE_SIZE * windowW / windowH));
  case SHAPE_4_3:
    return 960;
  case SHAPE_SQUARE:
    return 720;
  default:
    return 1280; // 16:9
  }
}

// Makes the canvas `width` x `height` pixels (only when that changes, e.g. when
// the window is resized), plus the blur steps, and makes the fonts match
static void ResizeCanvas(int width, int height) {
  if (canvas.id != 0 && canvas.texture.width == width &&
      canvas.texture.height == height)
    return;
  if (canvas.id != 0) {
    UnloadRenderTexture(canvas);
    for (RenderTexture2D &blur : blurSteps)
      UnloadRenderTexture(blur);
  }
  canvas = LoadRenderTexture(width, height);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
  int blurW = width, blurH = height;
  for (RenderTexture2D &blur : blurSteps) {
    blurW = std::max(4, blurW / 4); // each step a quarter of the last
    blurH = std::max(4, blurH / 4);
    blur = LoadRenderTexture(blurW, blurH);
    SetTextureFilter(blur.texture, TEXTURE_FILTER_BILINEAR);
  }
  LoadFonts(height / (float)STAGE_SIZE);
}

// Draws a render texture's whole picture into `dest`. (Render textures are
// stored upside down, hence the negative height.)
static void DrawRenderTexture(const RenderTexture2D &from, Rectangle dest,
                              Color tint) {
  const Texture2D &t = from.texture;
  DrawTexturePro(t, {0, 0, (float)t.width, -(float)t.height}, dest, {0, 0}, 0,
                 tint);
}

// Shrinks the finished canvas step by step; the last, tiny step is a soft blur
static void MakeBlur() {
  const RenderTexture2D *from = &canvas;
  for (RenderTexture2D &blur : blurSteps) {
    BeginTextureMode(blur);
    DrawRenderTexture(
        *from, {0, 0, (float)blur.texture.width, (float)blur.texture.height},
        WHITE);
    EndTextureMode();
    from = &blur;
  }
}

static void UpdateDrawFrame() {
  UpdateMusicStream(music);
  UpdateAudio();
  // Places have their own sounds only while a story is being played
  if (screen != Screen::Playing && screen != Screen::Paused &&
      !(screen == Screen::Settings && settingsReturnTo == Screen::Paused))
    PlayAmbience("");
  if (devMode)
    UpdateDevMode();
  if (IsKeyPressed(KEY_F11)) {
    settings.fullscreen = !settings.fullscreen;
    SaveSettings();
  }
  ApplySettings();

  // The game's screen keeps its shape and appears as big as fits in the window
  const float windowW = (float)GetScreenWidth();
  const float windowH = (float)GetScreenHeight();
  screenWidth = ShapeWidth(windowW, windowH);
  const float scale = std::min(windowW / screenWidth, windowH / STAGE_SIZE);
  const Rectangle target = {(windowW - screenWidth * scale) / 2,
                            (windowH - STAGE_SIZE * scale) / 2,
                            screenWidth * scale, STAGE_SIZE * scale};
  const float stageLeft = (screenWidth - STAGE_SIZE) / 2; // in stage units

  // The mouse is made to report stage units, so scenes never need to know the
  // window's real size
  SetMouseOffset((int)-(target.x + stageLeft * scale), (int)-target.y);
  SetMouseScale(1 / scale, 1 / scale);

  // The canvas has as many pixels as the game covers in the window, so
  // nothing is stretched and text stays sharp (capped for huge screens)
  const float pixelsPerUnit =
      std::max(0.5f, std::min(MAX_PIXELS_PER_UNIT, scale));
  ResizeCanvas((int)(screenWidth * pixelsPerUnit),
               (int)(STAGE_SIZE * pixelsPerUnit));

  UpdateScreen();

  // Draw the game: the camera zooms stage units up to the canvas' pixels and
  // moves the stage to the middle of the canvas
  Camera2D camera = {};
  camera.zoom = pixelsPerUnit;
  camera.offset = {stageLeft * pixelsPerUnit, 0};
  BeginTextureMode(canvas);
  ClearBackground(BLACK);
  BeginMode2D(camera);
  DrawScreen();
  EndMode2D();
  EndTextureMode();

  // If the window's shape is different, there's space beside the game
  const bool bars = target.x > 0.5f || target.y > 0.5f;
  const bool softEdges = settings.softEdges && bars;
  if (softEdges)
    MakeBlur();

  // While playing, the story decides how colourful the world is; menus are
  // always in colour. The colour changes slowly, so it seems to flow back.
  const bool inStory =
      screen == Screen::Playing || screen == Screen::Paused ||
      (screen == Screen::Settings && settingsReturnTo == Screen::Paused);
  const float wanted = inStory ? scene->colour : 1.0f;
  shownColour += (wanted - shownColour) * std::min(1.0f, GetFrameTime() * 1.5f);

  BeginDrawing();
  ClearBackground(BLACK);
  SetShaderValue(colourShader, GetShaderLocation(colourShader, "colour"),
                 &shownColour, SHADER_UNIFORM_FLOAT);
  BeginShaderMode(colourShader);
  if (softEdges) { // the blurred game, bigger than the window, darkened
    const float cover =
        std::max(windowW / target.width, windowH / target.height) * 1.2f;
    const float w = target.width * cover, h = target.height * cover;
    DrawRenderTexture(blurSteps[2],
                      {(windowW - w) / 2, (windowH - h) / 2, w, h}, EDGE_TINT);
  }
  DrawRenderTexture(canvas, target, WHITE);
  EndShaderMode();
  EndDrawing();
}

int main(int argc, char **argv) {
  // "./run.sh --check-pages" checks the puzzle pages and story, then stops
  if (argc > 1 && strcmp(argv[1], "--check-pages") == 0)
    return CheckPages() == 0 ? 0 : 1;
  // "./run.sh --dev" passes --dev, and --resume to carry on after a restart
  bool dev = false, resume = false;
  for (int i = 1; i < argc; i++) {
    dev = dev || strcmp(argv[i], "--dev") == 0;
    resume = resume || strcmp(argv[i], "--resume") == 0;
  }

#if !defined(_DEBUG) // raylib's log messages only show in debug builds
  SetTraceLogLevel(LOG_NONE);
#endif

  LoadSettings();
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1280, 720, "Kolam"); // 16:9; resize it as you like
  SetWindowMinSize(360, 360);
  SetExitKey(KEY_NULL); // Esc pauses instead of quitting
  InitAudioDevice();

  colourShader = LoadShaderFromMemory(nullptr, COLOUR_SHADER);
  if (const EmbeddedFile *song = FindAsset("assets/sole-of-kolam.mp3"))
    music = LoadMusicStreamFromMemory(".mp3", song->data, (int)song->size);
  PlayMusicStream(music);
  SeekMusicStream(music, 2.0f); // skip the quiet start of the song
  scene = MakeIntroScene();
  if (dev)
    StartDevMode(resume);

  SetTargetFPS(60);
  while (!WindowShouldClose() && !quit && !restartNow)
    UpdateDrawFrame();

  SaveSettings();
  scene.reset();
  UnloadShader(colourShader);
  UnloadRenderTexture(canvas);
  for (RenderTexture2D &blur : blurSteps)
    UnloadRenderTexture(blur);
  UnloadMusicStream(music);
  UnloadAudio();
  UnloadFonts();
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
