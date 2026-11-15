// explore_scene.cpp - walking around Anu's home, a village on the Bhavani
// between Mettupalayam and Sathyamangalam.
//
// The house has four rooms. Each room is drawn here (simple shapes for now,
// until the artists' paintings replace them). ROOM_OBJECTS says where each
// clickable thing is; story.cpp says what happens when it is clicked.
// Finding a memory brings colour back to the world. Now and then work calls;
// Anu can answer or let it go, and neither changes anything.
#include <cmath>

#include "game.h"

enum Room { FRONT_STEP, HALL, KITCHEN, TERRACE, ROOM_COUNT };
const char *ROOM_NAMES[ROOM_COUNT] = {"Front step", "Hall", "Kitchen",
                                      "Terrace"};
// The names story.cpp uses for them, e.g. Talk(...).At("terrace", NIGHT)
const char *PLACE_IDS[ROOM_COUNT] = {"front_step", "hall", "kitchen",
                                     "terrace"};

struct RoomObject {
  const char *object;
  Room room;
  Rectangle area;
};

// Where every clickable object is. Story steps refer to these by name.
const RoomObject ROOM_OBJECTS[] = {
    {"old_kolam", FRONT_STEP, {200, 560, 320, 80}},
    {"tulsi", FRONT_STEP, {60, 390, 120, 190}},
    {"bicycle", FRONT_STEP, {520, 400, 190, 170}},
    {"family_photo", HALL, {455, 110, 120, 150}},
    {"swing", HALL, {110, 300, 270, 200}},
    {"height_marks", HALL, {575, 150, 50, 400}}, // the kitchen doorframe
    {"calendar", HALL, {372, 130, 66, 96}},
    {"amma", KITCHEN, {370, 220, 140, 380}},
    {"grinding_stone", KITCHEN, {60, 480, 200, 110}},
    {"flour_tin", KITCHEN, {560, 170, 90, 100}},
    {"coffee_filter", KITCHEN, {270, 320, 80, 100}},
    {"vessels", KITCHEN, {50, 96, 480, 56}},
    {"kite_reel", TERRACE, {60, 450, 190, 140}},
    {"storage_boxes", TERRACE, {270, 500, 180, 120}},
    {"temple", TERRACE, {430, 190, 120, 200}},
};

//------------------------------------------------------------------------------
// Drawing the rooms
//------------------------------------------------------------------------------
// A wooden window with grills, the evening outside it
static void DrawWindow(Rectangle r, Color sky, float time) {
  DrawRectangleRec({r.x - 10, r.y - 10, r.width + 20, r.height + 20},
                   {104, 70, 46, 255});
  DrawRectangleGradientV((int)r.x, (int)r.y, (int)r.width, (int)r.height, sky,
                         {255, 214, 160, 255});
  DrawPalm(r.x + r.width * 0.7f, r.y + r.height + 20, r.height * 0.9f, time,
           {110, 80, 60, 255}, {70, 110, 70, 255});
  for (int i = 1; i < 5; i++)
    DrawLineEx({r.x + i * r.width / 5, r.y},
               {r.x + i * r.width / 5, r.y + r.height}, 3, {60, 64, 70, 255});
  DrawRectangleRec({r.x - 14, r.y + r.height + 6, r.width + 28, 8},
                   {84, 56, 38, 255}); // sill
}

static void DrawFrontStep(float time) {
  const Rectangle all = FullScreen();
  const float left = all.x, right = all.x + all.width;
  // the evening sky over the blue Nilgiri hills, palms against it
  DrawBandGradient(0, 196, {232, 148, 112, 255}, {255, 214, 168, 255});
  DrawCircleGradient({560, 160}, 130, {255, 240, 200, 150}, {255, 240, 200, 0});
  DrawHills(140, 196, 80, 1, {160, 158, 196, 255});
  DrawHills(170, 196, 40, 4, {124, 128, 170, 255});
  DrawPalm(left + 120, 196, 170, time, {110, 80, 60, 255}, {64, 104, 66, 255});
  DrawPalm(660, 196, 150, time, {110, 80, 60, 255}, {64, 104, 66, 255});
  DrawPalm(right - 90, 196, 190, time, {110, 80, 60, 255}, {64, 104, 66, 255});
  // the roof: rows of clay tiles
  DrawBand(150, 48, {168, 66, 48, 255});
  for (int row = 0; row < 3; row++) {
    for (float x = left - 20 + row * 9; x < right; x += 18)
      DrawCircleSector({x, 158.0f + row * 14}, 9, 0, 180, 8,
                       {144, 52, 38, 255});
  }
  // the lime-washed wall, with old patches, and the shade under the roof
  DrawBandGradient(198, 362, {244, 218, 158, 255}, {226, 188, 124, 255});
  DrawBand(198, 18, {80, 40, 20, 50});
  const Vector2 PATCHES[] = {{120, 300},  {480, 260}, {650, 380},
                             {-150, 330}, {860, 280}, {210, 420}};
  for (Vector2 p : PATCHES)
    DrawEllipse((int)p.x, (int)p.y, 50, 22, {255, 255, 240, 40});
  DrawWindow({-190, 260, 110, 130}, {236, 150, 112, 255}, time);
  DrawWindow({820, 260, 110, 130}, {236, 150, 112, 255}, time);
  DrawJasmine({450, 214, 110, 150}, time); // Amma's malli, by the door
  DrawJasmine({left + 10, 214, 140, 120}, time);
  // thinnai: the raised seats either side of the door
  const Color thinnai = {164, 62, 50, 255}, edge = {196, 92, 72, 255};
  DrawRectangleRec({left, 470, 260 - left, 90}, thinnai);
  DrawRectangleRec({460, 470, right - 460, 90}, thinnai);
  DrawRectangleRec({left, 466, 260 - left, 10}, edge);
  DrawRectangleRec({460, 466, right - 460, 10}, edge);
  // the door: a carved frame, two panels with brass studs
  DrawRectangle(276, 220, 168, 340, {74, 42, 28, 255});
  DrawRectangle(290, 232, 140, 328, {98, 58, 36, 255});
  for (int side = 0; side < 2; side++) {
    const int x = 298 + side * 66;
    DrawRectangle(x, 240, 58, 312, {120, 72, 44, 255});
    DrawRectangleLines(x + 8, 252, 42, 120, {86, 50, 30, 255});
    DrawRectangleLines(x + 8, 390, 42, 150, {86, 50, 30, 255});
    for (int i = 0; i < 4; i++)
      DrawCircle(x + 29, 280 + i * 70, 4, {232, 192, 84, 255});
  }
  // thoranam: mango leaves on a sagging string over the door
  for (int i = 0; i < 10; i++) {
    const float x = 280 + i * 18.0f, y = 224 + 6 * std::sin(PI * i / 9);
    const float sway = 1.5f * std::sin(time * 2 + i);
    DrawTriangle({x - 7, y}, {x + sway, y + 30}, {x + 7, y},
                 i % 2 ? Color{60, 130, 60, 255} : Color{86, 150, 70, 255});
  }
  DrawSplineSegmentBezierQuadratic({276, 222}, {360, 236}, {444, 222}, 2,
                                   {120, 90, 40, 255});
  // tulsi maadam: the little shrine planter, whitewashed with a red border
  DrawSoftShadow(120, 572, 130);
  DrawRectangle(80, 470, 80, 100, {236, 228, 210, 255});
  DrawRectangle(130, 470, 30, 100, {214, 204, 186, 255});
  DrawRectangle(74, 460, 92, 14, {196, 76, 56, 255});
  DrawRectangle(96, 490, 48, 40, {196, 76, 56, 255});
  DrawCircleGradient({120, 510}, 12, {255, 200, 90, 200}, {255, 200, 90, 0});
  for (int i = 0; i < 16; i++) {
    const float x = 120 + 28 * std::sin(i * 2.4f),
                y = 444 - (i % 5) * 12 + 2 * std::sin(time + i);
    DrawCircle((int)x, (int)y, 9 + (i % 3) * 2,
               i % 2 ? Color{56, 128, 66, 255} : Color{84, 156, 80, 255});
  }
  // Anu's old red bicycle, leaning on the thinnai
  DrawSoftShadow(615, 572, 190);
  DrawRing({560, 540}, 26, 30, 0, 360, 32, {40, 40, 40, 255});
  DrawRing({670, 540}, 26, 30, 0, 360, 32, {40, 40, 40, 255});
  for (int i = 0; i < 6; i++) { // spokes
    const float a = i * PI / 3;
    DrawLineV({560, 540}, {560 + 26 * std::cos(a), 540 + 26 * std::sin(a)},
              {120, 120, 120, 255});
    DrawLineV({670, 540}, {670 + 26 * std::cos(a), 540 + 26 * std::sin(a)},
              {120, 120, 120, 255});
  }
  const Color red = {200, 44, 52, 255};
  DrawLineEx({560, 540}, {610, 480}, 6, red);
  DrawLineEx({610, 480}, {670, 540}, 6, red);
  DrawLineEx({600, 540}, {610, 480}, 6, red);
  DrawLineEx({560, 540}, {600, 540}, 6, red);
  DrawLineEx({660, 460}, {670, 540}, 6, red);
  DrawLineEx({645, 455}, {675, 455}, 5, {40, 40, 40, 255});
  DrawEllipse(604, 476, 14, 5, {50, 40, 36, 255}); // seat
  DrawCircle(668, 462, 4, {230, 200, 90, 255});    // the bell
  // the step, the threshold stone, the packed-earth street
  DrawBand(560, 20, {178, 152, 122, 255});
  DrawRectangle(270, 560, 180, 20, {208, 198, 182, 255});
  DrawBandGradient(580, 140, {164, 134, 104, 255}, {132, 106, 82, 255});
  for (int i = 0; i < 30; i++) // pebbles
    DrawEllipse((int)(left + std::fmod(i * 97.0f, all.width)),
                600 + (i * 37) % 110, 3, 2, {120, 96, 74, 255});
  // yesterday's kolam, half swept away
  for (int i = 0; i < 5; i++)
    DrawRing({260.0f + i * 50, 610}, 14, 17, 180 + i * 40, 360 + i * 20, 16,
             {238, 232, 222, 150});
  for (int i = 0; i < 5; i++)
    DrawCircle(260 + i * 50, 610, 3, {238, 232, 222, 170});
}

static void DrawHall(float time) {
  // turquoise walls, evening light from the window
  DrawBandGradient(0, 545, {134, 198, 192, 255}, {100, 160, 156, 255});
  DrawBand(0, 26, {0, 0, 0, 30});
  DrawWindow({50, 110, 160, 150}, {240, 160, 120, 255}, time);
  DrawCircleGradient({130, 190}, 230, {255, 214, 150, 70}, {255, 214, 150, 0});
  // Amma's almirah (wide screens): dark wood, a mirror strip, brass handles
  DrawSoftShadow(-170, 562, 150);
  DrawRectangle(-230, 146, 119, 414, {96, 60, 38, 255});
  DrawRectangle(-218, 162, 46, 386, {130, 84, 52, 255});
  DrawRectangle(-168, 162, 46, 386, {130, 84, 52, 255});
  DrawRectangleGradientV(-210, 180, 30, 150, {220, 230, 236, 255},
                         {160, 176, 186, 255});
  DrawCircle(-178, 360, 5, {232, 192, 84, 255});
  DrawCircle(-162, 360, 5, {232, 192, 84, 255});
  // a wall clock
  DrawCircle(870, 150, 34, {120, 80, 50, 255});
  DrawCircle(870, 150, 28, {246, 240, 226, 255});
  DrawLineEx({870, 150}, {870, 130}, 3, {40, 40, 40, 255});
  DrawLineEx({870, 150}, {885, 158}, 3, {40, 40, 40, 255});
  // skirting and the red oxide floor, with a shine
  DrawBand(545, 15, {84, 134, 128, 255});
  DrawTiledFloor(560, {158, 60, 48, 255}, {132, 46, 38, 255});
  DrawEllipse(300, 630, 260, 26, {255, 230, 200, 30});
  // window light falling across the floor
  DrawTriangle({60, 560}, {300, 720}, {380, 720}, {255, 214, 150, 30});
  DrawTriangle({60, 560}, {380, 720}, {200, 560}, {255, 214, 150, 30});
  // ceiling fan
  DrawLineEx({360, 0}, {360, 40}, 3, {80, 80, 84, 255});
  DrawCircle(360, 44, 10, {100, 100, 106, 255});
  for (int i = 0; i < 3; i++) {
    const float a = time * 6 + i * 2.094f;
    DrawLineEx({360, 44}, {360 + 90 * std::cos(a), 44 + 10 * std::sin(a)}, 9,
               {96, 96, 102, 255});
  }
  // the Tamil calendar, a god's picture on top, the fourteenth circled
  DrawRectangle(375, 135, 60, 86, {250, 248, 240, 255});
  DrawRectangle(375, 135, 60, 26, {236, 120, 50, 255});
  DrawCircle(405, 148, 8, {250, 210, 90, 255});
  DrawString("14", 388, 170, 30, {40, 40, 40, 255});
  DrawRing({405, 186}, 20, 23, 0, 360, 24, {200, 40, 50, 255});
  // the family photo: a day at the beach, Appa, Amma and small Anu
  DrawRectangle(463, 118, 104, 134, {74, 48, 32, 255});
  DrawRectangle(469, 124, 92, 122, {246, 240, 226, 255}); // the white border
  DrawRectangleGradientV(475, 130, 80, 60, {150, 196, 220, 255},
                         {196, 220, 226, 255});          // sky
  DrawRectangle(475, 172, 80, 14, {90, 150, 180, 255});  // sea
  DrawRectangle(475, 186, 80, 54, {226, 204, 160, 255}); // sand
  const Color skin = {150, 100, 70, 255};
  DrawCircle(492, 160, 8, skin); // Appa, tall, a blue shirt
  DrawRectangle(484, 168, 16, 46, {70, 100, 160, 255});
  DrawRectangle(484, 214, 16, 22, {240, 236, 226, 255});
  DrawCircle(536, 166, 7, skin); // Amma, a green saree
  DrawRectangle(529, 173, 15, 60, {46, 120, 80, 255});
  DrawCircle(514, 196, 6, skin); // small Anu between them, a pink frock
  DrawTriangle({514, 202}, {505, 226}, {523, 226}, {226, 110, 150, 255});
  DrawEllipse(515, 245, 52, 4, {0, 0, 0, 40});
  // the oonjal: a wooden swing on chains
  DrawSoftShadow(245, 600, 300);
  for (int x : {140, 350}) {
    for (int y = 0; y < 390; y += 12)
      DrawRing({(float)x, (float)y + 6}, 3, 5, 0, 360, 8, {70, 70, 74, 255});
  }
  DrawRectangle(110, 390, 270, 26, {120, 72, 42, 255});
  for (int i = 0; i < 4; i++) // grain
    DrawLine(120, 396 + i * 5, 370, 396 + i * 5, {100, 58, 34, 255});
  DrawRectangle(110, 416, 270, 8, {80, 48, 30, 255});
  // the doorway to the kitchen: a wooden frame, a cotton curtain, and on
  // the frame, pencil marks of how tall Anu was each year
  DrawRectangle(590, 150, 120, 395, {58, 44, 36, 255});
  for (int i = 0; i < 5; i++)
    DrawRectangle(610 + i * 18, 156, 16, 300 + (int)(6 * std::sin(time + i)),
                  i % 2 ? Color{230, 226, 214, 255} : Color{70, 110, 170, 255});
  DrawRectangle(586, 140, 132, 14, {110, 72, 46, 255});
  DrawRectangle(586, 140, 22, 405, {124, 82, 52, 255});
  DrawRectangle(696, 140, 22, 405, {124, 82, 52, 255});
  for (int i = 0; i < 11; i++)
    DrawLine(589, 510 - i * 30, 600, 510 - i * 30, {60, 60, 70, 160});
}

static void DrawKitchen(float time, bool ammaHere) {
  const Rectangle all = FullScreen();
  // walls, smoky near the ceiling
  DrawBandGradient(0, 600, {186, 152, 100, 255}, {234, 206, 148, 255});
  DrawWindow({110, 304, 120, 96}, {244, 170, 120, 255}, time);
  // wooden shelves of brass and steel, ladles hanging at the side
  for (int y : {150, 270}) {
    DrawRectangle(40, y, 640, 10, {110, 70, 45, 255});
    DrawRectangle(40, y + 10, 640, 4, {70, 44, 28, 255});
  }
  for (int i = 0; i < 8; i++)
    DrawPot(80 + i * 60.0f, 128, 22, i % 2 == 0);
  for (int i = 0; i < 6; i++) { // steel dabbas
    DrawRectangle(70 + i * 70, 222, 40, 48, {190, 192, 198, 255});
    DrawRectangle(90 + i * 70, 222, 20, 48, {160, 162, 170, 255});
    DrawRectangle(74 + i * 70, 226, 6, 40, {240, 242, 246, 255});
  }
  for (int i = 0; i < 4; i++) {
    const float x = 760 + i * 30;
    DrawLineEx({x, 150}, {x, 250}, 4, {170, 172, 178, 255});
    DrawEllipse((int)x, 256, 10, 7, {170, 172, 178, 255});
  }
  // the rice flour tin
  DrawRectangle(568, 190, 70, 80, {232, 232, 226, 255});
  DrawRectangle(612, 190, 26, 80, {204, 204, 198, 255});
  DrawString("maavu", 574, 218, 16, {120, 120, 120, 255});
  // the cooking platform: dark granite on cream tiles, across the kitchen
  DrawRectangleRec({all.x, 420, all.width, 16}, {58, 56, 60, 255});
  for (int i = 0; i < 40; i++)
    DrawPixel((int)(all.x + std::fmod(i * 41.0f, all.width)), 424 + i % 8,
              {150, 150, 150, 255});
  DrawRectangleRec({all.x, 436, all.width, 164}, {222, 214, 196, 255});
  for (float x = all.x; x < all.x + all.width; x += 40)
    DrawLine((int)x, 436, (int)x, 600, {196, 186, 166, 255});
  DrawLine((int)all.x, 518, (int)(all.x + all.width), 518,
           {196, 186, 166, 255});
  // the stove, and the pot of pongal, steaming
  DrawRectangle(500, 404, 170, 18, {40, 40, 44, 255});
  DrawCircleGradient({580, 404}, 30, {80, 140, 255, 120}, {80, 140, 255, 0});
  DrawPot(580, 380, 44, true);
  for (int i = 0; i < 5; i++) {
    const float t = std::fmod(time * 0.6f + i * 0.2f, 1.0f);
    DrawCircle((int)(570 + 20 * std::sin(t * 6 + i)), (int)(330 - t * 120),
               12 + t * 14, {255, 255, 255, (unsigned char)(110 * (1 - t))});
  }
  // the brass coffee filter on the platform, with its dented lid
  DrawSoftShadow(310, 422, 60);
  DrawRectangle(290, 360, 40, 60, {200, 156, 80, 255});
  DrawRectangle(318, 360, 12, 60, {170, 126, 60, 255});
  DrawRectangle(286, 340, 48, 22, {214, 170, 92, 255});
  DrawRectangle(296, 330, 28, 12, {214, 170, 92, 255});
  DrawCircle(310, 326, 5, {190, 146, 70, 255});
  DrawLine(292, 345, 302, 349, {150, 108, 50, 255}); // the dent
  DrawTiledFloor(600, {150, 120, 104, 255}, {124, 98, 84, 255});
  // the ammi kal: grinding stone and its roller
  DrawSoftShadow(160, 592, 210);
  DrawRectangle(70, 540, 180, 50, {124, 124, 128, 255});
  DrawRectangle(70, 540, 180, 8, {150, 150, 154, 255});
  DrawEllipse(160, 535, 60, 14, {150, 150, 154, 255});
  DrawEllipse(146, 531, 30, 5, {180, 180, 184, 255});
  if (ammaHere)
    DrawAmma(440, 600, time, true);
}

static void DrawTerrace(float time) {
  const Rectangle all = FullScreen();
  // sunset over the Western Ghats; the Bhavani shining below them
  DrawBandGradient(0, 420, {236, 132, 104, 255}, {255, 210, 150, 255});
  DrawCircleGradient({360, 380}, 220, {255, 236, 180, 170}, {255, 236, 180, 0});
  DrawCircle(360, 380, 56, {255, 238, 186, 255});
  for (int i = 0; i < 3; i++) // long thin clouds
    DrawEllipse((int)(std::fmod(i * 330 + time * 6, 1300.0f) - 300),
                90 + i * 50, 160, 6, {255, 220, 200, 140});
  for (int i = 0; i < 4; i++) { // birds going home
    const float x = std::fmod(time * 30 + i * 140, 800.0f) - 40,
                y = 120 + i * 30 + 6 * std::sin(time * 3 + i);
    DrawLineEx({x - 10, y - 5}, {x, y}, 2, {60, 40, 50, 255});
    DrawLineEx({x, y}, {x + 10, y - 5}, 2, {60, 40, 50, 255});
  }
  DrawHills(330, 420, 110, 3, {180, 140, 170, 255});
  DrawHills(370, 420, 60, 6, {146, 116, 150, 255});
  DrawBandGradient(404, 16, {255, 214, 170, 255}, {230, 170, 140, 255});
  for (int i = 0; i < 8; i++) // the river glinting
    DrawLineEx({all.x + std::fmod(i * 170 + time * 10, all.width), 410},
               {all.x + std::fmod(i * 170 + time * 10, all.width) + 30, 410}, 2,
               {255, 246, 220, 200});
  // the temple tower far off: tiers getting smaller, a kalasam on top
  for (int i = 0; i < 6; i++) {
    const float w = 100 - i * 14, y = 382 - i * 30;
    DrawRectangleRec({490 - w / 2, y, w, 30}, {164, 100, 84, 255});
    DrawRectangleRec({490 - w / 2, y, w, 5}, {190, 124, 100, 255});
    for (int k = 0; k < (int)(w / 14); k++)
      DrawRectangle((int)(490 - w / 2 + 5 + k * 14), (int)y + 12, 6, 10,
                    {124, 72, 62, 255});
  }
  DrawEllipse(490, 226, 12, 7, {230, 180, 80, 255}); // the kalasam
  DrawTriangle({490, 206}, {482, 222}, {498, 222}, {230, 180, 80, 255});
  // palms and rooftops between here and the hills
  for (int i = 0; i < 6; i++)
    DrawPalm(all.x + 80 + i * 230, 440, 150 + (i * 37) % 50, time,
             {100, 72, 56, 255}, {58, 96, 62, 255});
  DrawBand(420, 100, {176, 112, 92, 255});
  const Color WALLS[] = {{214, 190, 160, 255},
                         {196, 214, 196, 255},
                         {230, 200, 150, 255},
                         {210, 180, 200, 255}};
  for (int i = 0; i < 10; i++) { // neighbours' houses, each a bit different
    const float x = all.x + i * 140 - 20, w = 110 + (i * 23) % 40,
                h = 50 + (i * 17) % 30;
    DrawRectangleRec({x, 520 - h, w, h}, WALLS[i % 4]);
    DrawTriangle({x - 8, 520 - h}, {x + w + 8, 520 - h}, {x + w / 2, 495 - h},
                 {150, 74, 58, 255});
    DrawRectangleRec({x + w * 0.4f, 530 - h, 16, 22}, {90, 70, 60, 255});
  }
  // the terrace: parapet and floor
  DrawBandGradient(520, 200, {200, 124, 94, 255}, {170, 100, 78, 255});
  DrawBand(496, 26, {222, 150, 112, 255});
  DrawBand(496, 6, {240, 176, 136, 255});
  DrawBand(522, 8, {0, 0, 0, 40});
  // the black water tank, ribbed
  DrawSoftShadow(650, 512, 130);
  DrawRectangleRounded({600, 360, 100, 150}, 0.2f, 8, {36, 36, 40, 255});
  for (int i = 0; i < 4; i++)
    DrawRectangle(600, 390 + i * 30, 100, 4, {56, 56, 62, 255});
  DrawRectangle(612, 370, 10, 130, {70, 70, 78, 255}); // shine
  // the clothes line: a saree and a towel lifting in the breeze
  DrawSplineSegmentBezierQuadratic({250, 300}, {420, 320}, {590, 330}, 2,
                                   {80, 80, 80, 255});
  const float flap = 6 * std::sin(time * 2.2f);
  DrawTriangle({300, 304}, {296 + flap, 400}, {362, 309}, {220, 60, 90, 255});
  DrawTriangle({362, 309}, {296 + flap, 400}, {358 + flap, 400},
               {220, 60, 90, 255});
  DrawLineEx({298 + flap, 392}, {358 + flap, 392}, 6, {240, 190, 70, 255});
  DrawRectangleRec({380 + flap * 0.4f, 312, 50, 64}, {70, 130, 200, 255});
  for (int i = 0; i < 3; i++)
    DrawLine((int)(384 + flap * 0.4f), 360 + i * 5, (int)(426 + flap * 0.4f),
             360 + i * 5, {240, 240, 240, 255});
  // old trunks brought up here: a blue tin trunk, a box of school things
  DrawSoftShadow(360, 606, 190);
  DrawRectangle(280, 540, 160, 64, {60, 96, 140, 255});
  DrawRectangle(280, 540, 160, 10, {80, 120, 166, 255});
  DrawRectangle(350, 556, 20, 16, {200, 170, 90, 255}); // the latch
  DrawEllipse(300, 590, 12, 5, {140, 80, 50, 180});     // rust
  DrawRectangle(300, 500, 110, 42, {186, 150, 104, 255});
  DrawRectangle(300, 500, 110, 6, {160, 124, 82, 255});
  DrawRectangle(318, 494, 18, 10, {226, 110, 70, 255}); // crayons peeking out
  DrawRectangle(338, 492, 8, 12, {80, 150, 210, 255});
  // Appa's kite reel (a wooden lattai), the old torn kite beside it
  DrawSoftShadow(130, 580, 150);
  DrawCircle(110, 540, 34, {204, 150, 80, 255});
  DrawCircle(110, 540, 26, {236, 220, 196, 255}); // wound thread
  for (int i = 0; i < 5; i++)
    DrawRing({110, 540}, 8 + i * 4, 9 + i * 4, 0, 360, 24,
             {214, 196, 170, 255});
  DrawCircle(110, 540, 8, {120, 80, 50, 255});
  DrawLineEx({80, 560}, {60, 590}, 6, {150, 104, 60, 255}); // handle
  DrawTriangle({190, 460}, {150, 520}, {230, 520}, {240, 120, 50, 255});
  DrawTriangle({150, 520}, {190, 590}, {230, 520}, {220, 100, 40, 255});
  DrawLineEx({190, 460}, {190, 590}, 2, {140, 90, 50, 255});   // the spine
  DrawLineEx({200, 470}, {215, 500}, 2, {255, 240, 220, 255}); // the tear
  DrawSplineSegmentBezierQuadratic({190, 590}, {220, 610}, {200, 640}, 2,
                                   {240, 200, 80, 255}); // tail
}

// Draws a room by day or by night, with Amma in it or not
static void DrawRoom(Room room, bool night, float time, bool ammaHere) {
  switch (room) {
  case FRONT_STEP:
    DrawFrontStep(time);
    break;
  case HALL:
    DrawHall(time);
    break;
  case KITCHEN:
    DrawKitchen(time, ammaHere);
    break;
  default:
    DrawTerrace(time);
    break;
  }
  if (night) { // dark blue over everything, with warm lamp light
    DrawBand(0, STAGE_SIZE, {10, 16, 50, 150});
    DrawCircleGradient({515, 270}, 160, {255, 190, 90, 60}, {255, 190, 90, 0});
  }
}

void DrawPlace(const std::string &place, bool night, float time) {
  for (int i = 0; i < ROOM_COUNT; i++) {
    if (place == PLACE_IDS[i])
      DrawRoom((Room)i, night, time, i == KITCHEN);
  }
}

//------------------------------------------------------------------------------
// The scene
//------------------------------------------------------------------------------
const float FIRST_CALL_SECONDS = 12; // the phone first rings after this long
const float CALL_GAP_SECONDS = 25;   // and then again after this long
const Rectangle ANSWER_BUTTON = {478, 560, 100, 40};
const Rectangle NOT_NOW_BUTTON = {588, 560, 110, 40};
// The doorframe up close: Anu's height each year, from 5 to 15, in pencil.
// Drag up the frame to trace them.
const float FRAME_X = 360;      // the middle of the post
const float MARKS_BOTTOM = 600; // age 5
const float MARK_GAP = 40;      // between one year and the next
const int MARK_COUNT = 11;      // ages 5 to 15

class ExploreScene : public Scene {
public:
  explicit ExploreScene(const Step &exploreStep) : step(exploreStep) {
    for (const Hotspot &h : step.hotspots) {
      const RoomObject *o = Find(h.object);
      if (o)
        roomHasSomething[o->room] = true;
      if (h.object == "amma")
        ammaHere = true;
    }
    // Start in the room of the first thing there is to click
    if (!step.hotspots.empty() && Find(step.hotspots[0].object))
      room = Find(step.hotspots[0].object)->room;
  }

  void Update() override {
    const float dt = GetFrameTime();
    time += dt;
    if (memoryTimer > 0)
      memoryTimer -= dt;
    // A memory coming back floods the world with colour for a moment, then it
    // settles, a little more colourful than before
    colour = memoryTimer > 1.5f ? 1.0f : WorldColour();
    // Birds outside; quieter indoors. On the terrace, the festival far off.
    if (room == TERRACE)
      PlayAmbience("crowd", 0.18f);
    else
      PlayAmbience("birds_day", room == FRONT_STEP ? 0.55f : 0.25f);
    if (ringing)
      PlayLoop("vibrate", 0.6f);
    hover = nullptr;

    // 1. Someone is talking
    if (!talk.empty()) {
      if (AdvanceDialogue(talk, talkLine, typed))
        FinishTalk();
      return;
    }

    // 2. Tracing the pencil marks on the doorframe
    if (tracing) {
      UpdateTracing();
      return;
    }

    // 3. The phone is ringing
    if (ringing) {
      UpdatePhone();
      return;
    }
    phoneTimer -= dt;
    if (phoneTimer <= 0 && callsThisScene < (int)WorkCalls().size()) {
      const int callNumber = progress.callsAnswered + progress.callsIgnored;
      call = callNumber % WorkCalls().size();
      ringing = true;
      callsThisScene++;
      phoneTimer = CALL_GAP_SECONDS;
    }

    // 4. Looking around: room tabs and objects
    for (int i = 0; i < ROOM_COUNT; i++) {
      if (Clicked(TabRect(i)))
        room = (Room)i;
    }
    const Vector2 mouse = GetMousePosition();
    for (const Hotspot &h : step.hotspots) {
      const RoomObject *o = Find(h.object);
      if (o && o->room == room && CheckCollisionPointRec(mouse, o->area))
        hover = &h;
    }
    SetMouseCursor(hover ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      current = hover;
      if (hover->object == "temple")
        PlayEffect("temple_bells", 0.7f);
      if (hover->object == "height_marks") { // up close first
        tracing = true;
        traced = 0;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
      } else {
        Say(hover->lines);
      }
    }
  }

  void Draw() override {
    DrawRoom(room, step.night, time, ammaHere);

    if (tracing)
      DrawTracing();
    if (hover && talk.empty() && !tracing)
      DrawHoverLabel();
    DrawHud();
    if (!talk.empty())
      DrawDialogueBox(talk[talkLine], typed);
    if (ringing)
      DrawPhone();
    if (memoryTimer > 0)
      DrawMemoryBanner();
  }

private:
  const Step &step;
  Room room = FRONT_STEP;
  bool roomHasSomething[ROOM_COUNT] = {}; // for the dots on the room tabs
  bool ammaHere = false;
  float time = 0;

  const Hotspot *hover = nullptr;   // what the mouse is over
  const Hotspot *current = nullptr; // the object whose lines are being said
  std::vector<Line> talk;           // the lines being said
  size_t talkLine = 0;
  float typed = 0;

  std::string memoryName; // the memory that just came back
  float memoryTimer = 0;

  bool tracing = false; // the doorframe is up close
  float traced = 0;     // how far up it the finger has gone

  bool ringing = false;
  size_t call = 0; // which of WorkCalls() is ringing
  float phoneTimer = FIRST_CALL_SECONDS;
  int callsThisScene = 0;

  static float MarkY(int age) { return MARKS_BOTTOM - (age - 5) * MARK_GAP; }

  // Hold and drag up the doorframe; the marks it passes come clear. Past the
  // last one, the object's lines play.
  void UpdateTracing() {
    const Vector2 mouse = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        std::fabs(mouse.x - FRAME_X) < 90) {
      const float before = traced;
      traced = std::fmax(traced, MARKS_BOTTOM + 20 - mouse.y);
      if (traced > before)
        PlayLoop("pencil", 0.5f); // a finger along the wood
    }
    if (traced > MARKS_BOTTOM + 20 - MarkY(15) + 30) {
      tracing = false;
      Say(current->lines);
    }
  }

  void DrawTracing() const {
    DrawDim();
    // the post, close up: old paint, wood showing through
    DrawRectangle((int)FRAME_X - 50, 0, 100, 660, {124, 82, 52, 255});
    DrawRectangle((int)FRAME_X - 50, 0, 14, 660, {146, 100, 64, 255});
    DrawRectangle((int)FRAME_X + 38, 0, 12, 660, {100, 64, 40, 255});
    const Color pencil = {54, 54, 64, 255};
    for (int age = 5; age < 5 + MARK_COUNT; age++) {
      const float y = MarkY(age);
      const bool found = MARKS_BOTTOM + 20 - y <= traced;
      const Color c = Faded(pencil, found ? 0.9f : 0.25f);
      DrawLineEx({FRAME_X - 40, y}, {FRAME_X + 20 + (age % 3) * 6, y}, 2, c);
      DrawString(TextFormat("Anu %d", age), FRAME_X - 150, y - 16, 24,
                 Faded(FLOUR, found ? 0.9f : 0.2f), HAND_FONT);
    }
    // the finger's path: the dust wiped off
    if (traced > 0)
      DrawRectangle((int)FRAME_X - 6, (int)(MARKS_BOTTOM + 20 - traced), 12,
                    (int)traced, {255, 240, 210, 40});
    if (traced == 0) { // "drag up": a ring and an arrow
      DrawRing({FRAME_X, MARKS_BOTTOM + 20}, 22, 26, 0, 360, 32,
               Faded(FLOUR, 0.4f + 0.4f * std::sin(time * 4)));
      DrawTriangle({FRAME_X, MARKS_BOTTOM - 40},
                   {FRAME_X - 10, MARKS_BOTTOM - 22},
                   {FRAME_X + 10, MARKS_BOTTOM - 22}, Faded(FLOUR, 0.6f));
    }
  }

  static const RoomObject *Find(const std::string &object) {
    for (const RoomObject &o : ROOM_OBJECTS) {
      if (object == o.object)
        return &o;
    }
    return nullptr;
  }

  static Rectangle TabRect(int i) { return {16.0f + i * 176, 668, 164, 42}; }

  void Say(const std::vector<Line> &lines) {
    talk = lines;
    talkLine = 0;
    typed = 0;
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
  }

  // After an object's lines: maybe a memory comes back, maybe the story moves
  // on
  void FinishTalk() {
    talk.clear();
    if (!current)
      return;
    if (!current->memory.empty() && !HasMemory(current->memory)) {
      AddMemory(current->memory);
      PlayEffect("bell", 0.45f);
      memoryName = current->memory;
      memoryTimer = 3.5f;
      colour = WorldColour();
    }
    if (current->endsScene)
      done = true;
    current = nullptr;
  }

  // Answer (the call plays) or "Not now". Neither changes anything.
  void UpdatePhone() {
    if (Clicked(ANSWER_BUTTON)) {
      ringing = false;
      progress.callsAnswered++;
      current = nullptr;
      Say(WorkCalls()[call]);
    } else if (Clicked(NOT_NOW_BUTTON)) {
      ringing = false;
      progress.callsIgnored++;
    }
  }

  // A glowing outline around the object under the mouse, and its name
  void DrawHoverLabel() const {
    const Rectangle r = Find(hover->object)->area;
    DrawRectangleRoundedLinesEx(r, 0.15f, 6, 3,
                                Faded(FLOUR, 0.6f + 0.3f * std::sin(time * 5)));
    const float w = MeasureString(hover->name, 20) + 24;
    const float x =
        std::fmax(10, std::fmin(710 - w, r.x + r.width / 2 - w / 2));
    const float y =
        r.y > 50 ? r.y - 40 : r.y + r.height + 8; // above if it fits
    DrawRectangleRounded({x, y, w, 32}, 0.5f, 6, {20, 20, 30, 220});
    DrawString(hover->name, x + 12, y + 4, 20, FLOUR);
  }

  void DrawHud() const {
    // Room tabs; a dot means there's something to find in that room
    DrawBand(660, 60, {20, 20, 30, 255});
    for (int i = 0; i < ROOM_COUNT; i++) {
      const Rectangle r = TabRect(i);
      const bool here = i == room;
      DrawButton(r, ROOM_NAMES[i], here ? KUMKUM : Faded(FLOUR, 0.08f));
      if (roomHasSomething[i] && !here)
        DrawCircle((int)(r.x + r.width - 14), (int)r.y + 12, 4, FLOUR);
    }
    // Memories found so far, as kolam dots, in the screen's top-left corner
    const float left = FullScreen().x + 10;
    const int total = MemoryCount();
    DrawRectangleRounded({left, 10, 34.0f + total * 18, 34}, 0.5f, 6,
                         {20, 20, 30, 170});
    for (int i = 0; i < total; i++) {
      const bool found = i < (int)progress.memories.size();
      DrawCircleV({left + 18 + i * 18, 27}, found ? 6 : 3,
                  found ? FLOUR : Faded(FLOUR, 0.4f));
    }
  }

  void DrawPhone() const {
    DrawRectangleRounded({460, 400, 250, 220}, 0.12f, 8, {24, 24, 30, 250});
    DrawString("Rohit (work)", 480, 416, 22, FLOUR);
    DrawString("calling...", 480, 444, 16, MUTED);
    for (int i = 0; i < 3; i++) // ringing
      DrawRing({585, 506}, 14.0f + i * 12, 16.0f + i * 12, 200, 340, 16,
               Faded(FLOUR, 0.5f * std::fmod(time * 1.5f - i * 0.3f, 1.0f)));
    DrawButton(ANSWER_BUTTON, "Answer", {60, 150, 90, 255});
    DrawButton(NOT_NOW_BUTTON, "Not now", Faded(FLOUR, 0.15f));
  }

  void DrawMemoryBanner() const {
    const float a =
        std::fmin(1.0f, memoryTimer); // fades out in the last second
    DrawRectangleRounded({160, 576, 400, 70}, 0.4f, 8,
                         Faded({20, 20, 30, 255}, 0.8f * a));
    DrawTextCentered("A memory comes back", 360, 582, 18, Faded(MUTED, a));
    DrawTextCentered(memoryName, 360, 604, 30, Faded(FLOUR, a), HAND_FONT);
  }
};

std::unique_ptr<Scene> MakeExploreScene(const Step &step) {
  return std::unique_ptr<Scene>(new ExploreScene(step));
}
