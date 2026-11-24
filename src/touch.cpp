// touch.cpp - quick taps on a phone. raylib reads a finger once a frame:
// if it touched down and lifted again between two frames (a quick tap), the
// game never saw it down, and the tap was lost. Here every touch is seen as
// it arrives (then raylib handles it as usual); a tap that came and went
// unseen is played to the game as a short press: down this frame, up the
// next. On Android the touch events are seen through raylib's input
// callback; on iPhone, through SDL's (raylib runs on SDL there). Elsewhere
// these do nothing.
#include "game.h"

#if defined(PLATFORM_ANDROID)
#include <android_native_app_glue.h>
extern "C" struct android_app *GetAndroidApp(void); // in raylib
#elif defined(PLATFORM_IOS)
#include <SDL.h>
#endif

#if defined(PLATFORM_PHONE)
namespace {
int touchesDown = 0;           // fingers put down since the last frame
bool releaseNextFrame = false; // a quick tap was played: lift it now

#if defined(PLATFORM_ANDROID)
int32_t (*raylibInput)(struct android_app *, AInputEvent *) = nullptr;

int32_t SeeInput(struct android_app *app, AInputEvent *event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION &&
      (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) ==
          AMOTION_EVENT_ACTION_DOWN)
    touchesDown++;
  return raylibInput ? raylibInput(app, event) : 0;
}
#else // iPhone
int SeeEvent(void *, SDL_Event *event) {
  if (event->type == SDL_FINGERDOWN || event->type == SDL_MOUSEBUTTONDOWN)
    touchesDown++;
  return 1; // (an event watch can't stop the event; raylib still gets it)
}
#endif

void PlayButton(unsigned int type) { // raylib's MOUSE_BUTTON_DOWN/UP event
  AutomationEvent e = {};
  e.type = type;
  e.params[0] = MOUSE_BUTTON_LEFT;
  PlayAutomationEvent(e);
}
} // namespace

void InitTouch() {
#if defined(PLATFORM_ANDROID)
  struct android_app *app = GetAndroidApp();
  if (!app)
    return;
  raylibInput = app->onInputEvent;
  app->onInputEvent = SeeInput;
#else
  SDL_AddEventWatch(SeeEvent, nullptr);
#endif
}

void UpdateTouch() {
  const unsigned int MOUSE_UP_EVENT = 5, MOUSE_DOWN_EVENT = 6;
  if (releaseNextFrame) {
    PlayButton(MOUSE_UP_EVENT);
    releaseNextFrame = false;
  }
  // a finger went down since last frame, yet it isn't down: a quick tap
  if (touchesDown > 0 && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    PlayButton(MOUSE_DOWN_EVENT);
    releaseNextFrame = true;
  }
  touchesDown = 0;
}
#else
void InitTouch() {}
void UpdateTouch() {}
#endif
