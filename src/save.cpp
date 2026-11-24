// save.cpp - settings, the save game, and what Anu has found (Progress).
// Settings and the save game are small "key=value" text files in:
//   Linux:   $XDG_DATA_HOME/kolam/ or ~/.local/share/kolam/
//   Windows: %APPDATA%/Kolam/
//   macOS:   ~/Library/Application Support/Kolam/
//   Android: the app's private storage
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>

#include "game.h"

#if defined(PLATFORM_ANDROID)
#include <android_native_app_glue.h>
extern "C" struct android_app *GetAndroidApp(void); // from raylib
#endif

Settings settings;
Progress progress;

//------------------------------------------------------------------------------
// What Anu has found
//------------------------------------------------------------------------------
bool HasMemory(const std::string &memory) {
  for (const std::string &m : progress.memories) {
    if (m == memory)
      return true;
  }
  return false;
}

void AddMemory(const std::string &memory) {
  if (!memory.empty() && !HasMemory(memory))
    progress.memories.push_back(memory);
}

// A Memory(...) step: once played, Anu has that memory back (by its label)
bool IsFlashback(const Step &step) {
  return step.memory &&
         (step.type == StepType::Talk || step.type == StepType::Play);
}

int MemoryCount() {
  int count = 1; // KITE_MEMORY, from flying Appa's kite
  for (const Chapter &chapter : Chapters()) {
    for (const Step &step : chapter.steps) {
      for (const Hotspot &hotspot : step.hotspots)
        count += hotspot.memory.empty() ? 0 : 1;
      count += IsFlashback(step) ? 1 : 0; // remembered as its label
    }
  }
  return count;
}

float WorldColour() {
  // The present day is only a little muted at first: memories make familiar
  // details noticeable again. Work calls never take colour away.
  const float MUTED_START = 0.55f;
  const float found =
      std::min(1.0f, (float)progress.memories.size() / MemoryCount());
  return MUTED_START + (1 - MUTED_START) * found;
}

//------------------------------------------------------------------------------
// Files
//------------------------------------------------------------------------------
static std::string SaveFolder() {
  std::string folder;
#if defined(_WIN32)
  if (const char *appData = std::getenv("APPDATA"))
    folder = std::string(appData) + "/Kolam";
#elif defined(__APPLE__)
  if (const char *home = std::getenv("HOME"))
    folder = std::string(home) + "/Library/Application Support/Kolam";
#elif defined(PLATFORM_ANDROID)
  folder = GetAndroidApp()->activity->internalDataPath;
#else
  const char *xdg = std::getenv("XDG_DATA_HOME");
  const char *home = std::getenv("HOME");
  if (xdg && *xdg)
    folder = std::string(xdg) + "/kolam";
  else if (home)
    folder = std::string(home) + "/.local/share/kolam";
#endif
  if (folder.empty())
    return ""; // current folder
  if (!DirectoryExists(folder.c_str()))
    MakeDirectory(folder.c_str());
  return folder + "/";
}

static std::map<std::string, std::string> ReadFile(const std::string &name) {
  std::map<std::string, std::string> values;
  std::ifstream file(SaveFolder() + name);
  std::string line;
  while (std::getline(file, line)) {
    const size_t equals = line.find('=');
    if (equals != std::string::npos)
      values[line.substr(0, equals)] = line.substr(equals + 1);
  }
  return values;
}

static void WriteFile(const std::string &name,
                      const std::map<std::string, std::string> &values) {
  std::ofstream file(SaveFolder() + name);
  for (const auto &entry : values)
    file << entry.first << "=" << entry.second << "\n";
}

// Reads a number, keeping `value` unchanged if the key is missing.
static void ReadNumber(const std::map<std::string, std::string> &values,
                       const char *key, int &value, int min, int max) {
  auto found = values.find(key);
  if (found == values.end())
    return;
  const int number = std::atoi(found->second.c_str());
  value = number < min ? min : (number > max ? max : number);
}

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
void LoadSettings() {
  const auto values = ReadFile("settings.cfg");
  int fullscreen = settings.fullscreen ? 1 : 0;
  int softEdges = settings.softEdges ? 1 : 0;
  ReadNumber(values, "master_volume", settings.masterVolume, 0, 100);
  ReadNumber(values, "music_volume", settings.musicVolume, 0, 100);
  ReadNumber(values, "text_speed", settings.textSpeed, 0, TEXT_SPEED_COUNT - 1);
  ReadNumber(values, "fullscreen", fullscreen, 0, 1);
  ReadNumber(values, "soft_edges", softEdges, 0, 1);
  ReadNumber(values, "screen_shape", settings.screenShape, 0, 3);
  ReadNumber(values, "text_size", settings.textSize, 0, TEXT_SIZE_COUNT - 1);
  const char *GESTURE_KEYS[4] = {"learned_tap", "learned_hold", "learned_drag",
                                 "learned_swipe"};
  for (int g = 0; g < 4; g++)
    ReadNumber(values, GESTURE_KEYS[g], settings.gestureUses[g], 0, 99);
  int version = 1;
  ReadNumber(values, "settings_version", version, 1, 99);
#if defined(PLATFORM_PHONE)
  // Settings saved by the first phone builds (version 1) kept 16:9, which
  // leaves black bars on a phone: fill the screen, and start text Large
  if (version < 2) {
    settings.screenShape = 1;
    settings.textSize = 1;
  }
#endif
  settings.fullscreen = fullscreen == 1;
  settings.softEdges = softEdges == 1;
}

void SaveSettings() {
  WriteFile("settings.cfg",
            {
                {"master_volume", std::to_string(settings.masterVolume)},
                {"music_volume", std::to_string(settings.musicVolume)},
                {"text_speed", std::to_string(settings.textSpeed)},
                {"fullscreen", settings.fullscreen ? "1" : "0"},
                {"soft_edges", settings.softEdges ? "1" : "0"},
                {"screen_shape", std::to_string(settings.screenShape)},
                {"text_size", std::to_string(settings.textSize)},
                {"learned_tap", std::to_string(settings.gestureUses[0])},
                {"learned_hold", std::to_string(settings.gestureUses[1])},
                {"learned_drag", std::to_string(settings.gestureUses[2])},
                {"learned_swipe", std::to_string(settings.gestureUses[3])},
                {"settings_version", "2"},
            });
}

const char *TextSpeedName() {
  const char *names[TEXT_SPEED_COUNT] = {"Slow", "Normal", "Fast", "Instant"};
  return names[settings.textSpeed];
}

float TextCharsPerSecond() {
  const float speeds[TEXT_SPEED_COUNT] = {20, 40, 80, 100000};
  return speeds[settings.textSpeed];
}

//------------------------------------------------------------------------------
// The save game
//------------------------------------------------------------------------------
// Progress as one line of text: "memory1,memory2|calls answered|calls ignored"
static std::string ProgressToText(const Progress &p) {
  std::string memories;
  for (const std::string &m : p.memories)
    memories += m + ",";
  return memories + "|" + std::to_string(p.callsAnswered) + "|" +
         std::to_string(p.callsIgnored);
}

static Progress ProgressFromText(const std::string &text) {
  Progress p;
  std::stringstream parts(text);
  std::string memories, answered, ignored;
  std::getline(parts, memories, '|');
  std::getline(parts, answered, '|');
  std::getline(parts, ignored, '|');
  std::stringstream list(memories);
  std::string memory;
  while (std::getline(list, memory, ','))
    if (!memory.empty())
      p.memories.push_back(memory);
  p.callsAnswered = std::atoi(answered.c_str());
  p.callsIgnored = std::atoi(ignored.c_str());
  return p;
}

// The chapter with this id, or -1. (Saves store ids, not numbers, so adding
// a chapter doesn't send old saves to the wrong place.)
static int ChapterIndex(const std::string &id) {
  for (size_t i = 0; i < Chapters().size(); i++)
    if (Chapters()[i].id == id)
      return (int)i;
  return -1;
}

SaveGame LoadGame() {
  SaveGame save;
  const auto values = ReadFile("savegame.txt");
  if (values.count("chapter") == 0)
    return save; // no save (or one from before chapters existed)
  save.chapter = ChapterIndex(values.at("chapter"));
  if (save.chapter < 0)
    return save; // that chapter was removed
  save.exists = true;
  const Chapter &chapter = Chapters()[save.chapter];
  ReadNumber(values, "step", save.step, -1, (int)chapter.steps.size() - 1);
  save.label = chapter.heading + ": " + chapter.title;
  if (values.count("saved_at"))
    save.savedAt = std::atoll(values.at("saved_at").c_str());
  if (values.count("progress"))
    save.progress = ProgressFromText(values.at("progress"));
  return save;
}

void SaveGameAt(int chapter, int step) {
  WriteFile("savegame.txt",
            {
                {"chapter", Chapters()[chapter].id},
                {"step", std::to_string(step)},
                {"saved_at", std::to_string((long long)std::time(nullptr))},
                {"progress", ProgressToText(progress)},
            });
}

//------------------------------------------------------------------------------
// Reached chapters: chapters.txt has one line per chapter reached,
// "<chapter id>=<Progress when it was first reached>". Every chapter can be
// played, reached or not; this only remembers how far Anu had got.
//------------------------------------------------------------------------------
bool ChapterUnlocked(int chapter) {
  return ReadFile("chapters.txt").count(Chapters()[chapter].id) > 0;
}

void UnlockChapter(int chapter) {
  auto values = ReadFile("chapters.txt");
  const std::string &id = Chapters()[chapter].id;
  if (values.count(id))
    return; // already unlocked; keep the progress from the first time
  values[id] = ProgressToText(progress);
  WriteFile("chapters.txt", values);
}

Progress ProgressAtChapterStart(int chapter) {
  const auto values = ReadFile("chapters.txt");
  auto found = values.find(Chapters()[chapter].id);
  if (found != values.end())
    return ProgressFromText(found->second);
  // Not reached yet: as if Anu had found every memory on the way here, so
  // the world has the colour it would have by now
  Progress p;
  auto add = [&p](const std::string &memory) {
    if (!memory.empty() &&
        std::find(p.memories.begin(), p.memories.end(), memory) ==
            p.memories.end())
      p.memories.push_back(memory);
  };
  for (int c = 0; c < chapter; c++) {
    for (const Step &step : Chapters()[c].steps) {
      if (step.type == StepType::Kite)
        add(KITE_MEMORY);
      for (const Hotspot &hotspot : step.hotspots)
        add(hotspot.memory);
      if (IsFlashback(step))
        add(step.label);
    }
  }
  return p;
}

void DeleteSave() { std::remove((SaveFolder() + "savegame.txt").c_str()); }

std::string TimeAgo(long long unixTime) {
  const long long seconds = (long long)std::time(nullptr) - unixTime;
  if (seconds < 60)
    return "just now";
  if (seconds < 3600)
    return std::to_string(seconds / 60) + " min ago";
  if (seconds < 86400)
    return std::to_string(seconds / 3600) + " h ago";
  if (seconds < 2 * 86400)
    return "yesterday";
  return std::to_string(seconds / 86400) + " days ago";
}
