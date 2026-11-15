// audio.cpp - the sounds of places (ambience) and of things (effects).
// Every sound is an .ogg file in assets/sounds/, built into the game.
// Where each one came from is listed in README.md (all CC0, from Freesound).
//
//   PlayAmbience("crickets")   the place's background sound, looping. Call it
//                              every frame or once; the old one fades out and
//                              the new one fades in. "" fades to silence.
//   PlayEffect("click")        a sound, once
//   PlayLoop("pour")           a sound that keeps playing for as long as it is
//                              asked for, every frame (pouring, drawing)
#include <cmath>
#include <map>
#include <set>

#include "game.h"

const float FADE_SPEED = 0.8f; // ambience volume change per second

struct Ambience {
  std::string name;
  Music music = {};
  float volume = 0; // now
  float wanted = 0; // fading towards this
};
static Ambience current, fading; // fading: the one going quiet
static std::map<std::string, Sound> sounds;
static std::set<std::string> loopsAsked;  // PlayLoop() since the last update
static std::set<std::string> loopsBefore; // ...and in the update before that

static const EmbeddedFile *SoundFile(const std::string &name) {
  return FindAsset("assets/sounds/" + name + ".ogg");
}

// Loads a sound the first time it's asked for. A missing file is silent.
static Sound GetSound(const std::string &name) {
  auto found = sounds.find(name);
  if (found != sounds.end())
    return found->second;
  Sound sound = {};
  if (const EmbeddedFile *file = SoundFile(name)) {
    Wave wave = LoadWaveFromMemory(".ogg", file->data, (int)file->size);
    sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
  }
  sounds[name] = sound;
  return sound;
}

void PlayEffect(const std::string &name, float volume, float pitch) {
  Sound sound = GetSound(name);
  if (sound.frameCount == 0)
    return;
  SetSoundVolume(sound, volume);
  SetSoundPitch(sound, pitch);
  PlaySound(sound);
}

void PlayLoop(const std::string &name, float volume) {
  loopsAsked.insert(name);
  Sound sound = GetSound(name);
  if (sound.frameCount == 0)
    return;
  SetSoundVolume(sound, volume);
  if (!IsSoundPlaying(sound))
    PlaySound(sound);
}

void PlayAmbience(const std::string &name, float volume) {
  if (name == current.name) {
    current.wanted = volume;
    return;
  }
  if (fading.music.ctxData)
    UnloadMusicStream(fading.music);
  fading = current;
  fading.wanted = 0;
  current = Ambience();
  current.name = name;
  current.wanted = volume;
  if (const EmbeddedFile *file = SoundFile(name)) {
    current.music =
        LoadMusicStreamFromMemory(".ogg", file->data, (int)file->size);
    SetMusicVolume(current.music, 0);
    PlayMusicStream(current.music);
  }
}

bool AmbiencePlaying() { return !current.name.empty(); }

void UpdateAudio() {
  const float step = FADE_SPEED * GetFrameTime();
  for (Ambience *a : {&current, &fading}) {
    if (!a->music.ctxData)
      continue;
    if (a->volume < a->wanted)
      a->volume = std::fmin(a->wanted, a->volume + step);
    else
      a->volume = std::fmax(a->wanted, a->volume - step);
    SetMusicVolume(a->music, a->volume);
    UpdateMusicStream(a->music);
  }
  // A loop that was going but wasn't asked for since the last update stops
  for (const std::string &name : loopsBefore) {
    if (!loopsAsked.count(name))
      StopSound(GetSound(name));
  }
  loopsBefore = loopsAsked;
  loopsAsked.clear();
}

void UnloadAudio() {
  for (Ambience *a : {&current, &fading}) {
    if (a->music.ctxData)
      UnloadMusicStream(a->music);
  }
  for (auto &entry : sounds)
    UnloadSound(entry.second);
  sounds.clear();
}
