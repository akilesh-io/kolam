// puzzle_scene.cpp - one page of Amma's kolam book.
// The player clicks between two dots to add or remove a wall until all the
// coloured lines join into ONE line. Then the line is drawn in a single stroke.
#include <cmath>

#include "game.h"

// Amma's pages: old paper and ink
const Color PAPER = {243, 234, 210, 255};
const Color INK = {40, 42, 92, 255};
const Color INK_WALL = {105, 70, 50, 255};
const Color DOT = {70, 55, 45, 255};
const Color FAINT = {120, 100, 80, 255};
// Little Anu's pages: school notebook and crayons
const Color NOTEBOOK = {250, 247, 238, 255};
const Color CRAYON = {150, 60, 160, 255};

// One colour per separate line, so the player can see how many there are.
// With more lines than colours, the colours repeat.
const Color INK_COLORS[] = {{150, 40, 55, 255},  {40, 90, 150, 255},
                            {30, 125, 110, 255}, {190, 125, 20, 255},
                            {85, 120, 40, 255},  {120, 60, 140, 255}};
const Color CRAYON_COLORS[] = {{235, 110, 40, 255}, {225, 70, 130, 255},
                               {60, 160, 80, 255},  {50, 120, 220, 255},
                               {200, 170, 30, 255}, {140, 80, 200, 255}};

const Rectangle PAGE_AREA = {40, 40, 640, 640};
const Rectangle GRID_AREA = {80, 178, 560, 404};
const int LINE_COLOR_COUNT = sizeof(INK_COLORS) / sizeof(INK_COLORS[0]);
// How long the line takes to draw itself once the page is solved
const float SOLVED_DRAW_SECONDS = 2.4f;

class PuzzleScene : public Scene {
public:
  explicit PuzzleScene(int pageIndex)
      : page(Pages()[pageIndex]), pageNumber(pageIndex + 1),
        inkColor(page.byLittleAnu ? CRAYON : INK),
        wobble(page.byLittleAnu ? 1.6f : 0) { // a child's wobbly hand
    layoutOk = grid.Load(page, &layoutError);
    PlayEffect("page", 0.8f); // the book opens at this page
    if (!layoutOk)
      return;
    cell = std::fmin(
        std::fmin(GRID_AREA.width / grid.width, GRID_AREA.height / grid.height),
        100.0f);
    origin = {GRID_AREA.x + (GRID_AREA.width - cell * grid.width) / 2,
              GRID_AREA.y + (GRID_AREA.height - cell * grid.height) / 2};
    lines = LoopPaths(grid, origin, cell, wobble);
  }

  ~PuzzleScene() override { SetMouseCursor(MOUSE_CURSOR_DEFAULT); }

  void Update() override {
    const float dt = GetFrameTime();
    if (messageTimer > 0)
      messageTimer -= dt;
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const bool next = ContinuePressed();

    if (!layoutOk) { // never trap the player on a broken page
      if (next)
        done = true;
      return;
    }

    if (solved) {
      solvedTime += dt;
      if (next && solvedTime < SOLVED_DRAW_SECONDS)
        solvedTime = SOLVED_DRAW_SECONDS; // skip the animation
      else if (next)
        done = true;
      return;
    }

    if (IsKeyPressed(KEY_R))
      grid.Reset();
#if defined(_DEBUG)
    if (IsKeyPressed(KEY_N))
      grid.CountSolutions(true); // debug: solve this page
#endif

    hovering = SpotUnderMouse(&hover);
    const bool canChange = hovering && grid.IsEditable(hover.x, hover.y);
    SetMouseCursor(canChange ? MOUSE_CURSOR_POINTING_HAND
                             : MOUSE_CURSOR_DEFAULT);

    if (clicked && canChange) {
      grid.Toggle(hover.x, hover.y);
      PlayEffect("scribble", 0.6f);
    } else if (clicked && hovering) {
      message = page.byLittleAnu ? "That part is already drawn in crayon."
                                 : "That one is in Amma's ink. It stays.";
      messageTimer = 1.8f;
    }

    lines = LoopPaths(grid, origin, cell, wobble);
    if (lines.size() == 1) {
      solved = true;
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
  }

  void Draw() override {

    // Page
    ClearBackground({58, 40, 30, 255});             // wooden table
    DrawRectangle(46, 48, 640, 640, {0, 0, 0, 70}); // shadow
    DrawRectangleRec(PAGE_AREA, page.byLittleAnu ? NOTEBOOK : PAPER);
    if (page.byLittleAnu) { // ruled lines and a margin
      for (int y = 110; y < 670; y += 26)
        DrawLine(40, y, 680, y, {170, 200, 230, 140});
      DrawLine(68, 40, 68, 680, {230, 140, 140, 180});
    }

    // Header
    DrawString("AMMA'S KOLAM BOOK", 80, 52, 15, FAINT);
    const char *pageLabel =
        TextFormat("page %d of %d", pageNumber, (int)Pages().size());
    DrawString(pageLabel, 640 - MeasureString(pageLabel, 15), 52, 15, FAINT);
    DrawString(page.title, 80, 72, 34, inkColor, HAND_FONT);
    DrawTextWrapped(page.note, 80, 114, 22, 560, Faded(inkColor, 0.85f),
                    HAND_FONT);

    if (!layoutOk) {
      DrawTextWrapped("This page is torn: " + layoutError, 80, 300, 22, 560,
                      KUMKUM);
      return;
    }

    DrawWalls();
    DrawLines();
    if (!solved)
      DrawChangeableSpots();
    for (int y = 0; y < grid.height; y++) {
      for (int x = 0; x < grid.width; x++) {
        DrawCircleV(ToScreen({2 * x + 1, 2 * y + 1}),
                    std::fmax(3.0f, cell * 0.065f),
                    page.byLittleAnu ? CRAYON : DOT);
      }
    }

    // Footer
    if (!solved) {
      DrawString(
          TextFormat("%d separate lines. Make them one.", (int)lines.size()),
          80, 592, 24, inkColor);
      DrawString(page.symmetric ? "Click between dots to add or remove a wall "
                                  "(mirrored on every side).  R: start over"
                                : "Click between two dots to add or remove a "
                                  "wall.   R: start over",
                 80, 626, 16, FAINT);
    } else {
      DrawString("One unbroken line.", 80, 592, 24, inkColor);
      if (solvedTime >= SOLVED_DRAW_SECONDS) {
        const float blink = 0.55f + 0.45f * std::sin((float)GetTime() * 3);
        DrawString(pageNumber < (int)Pages().size() ? "Click to turn the page"
                                                    : "Click to close the book",
                   80, 626, 17, Faded(FAINT, blink));
      }
    }
    if (messageTimer > 0) {
      DrawString(message, 640 - MeasureString(message, 17), 596, 17,
                 Faded(KUMKUM, messageTimer));
    }
  }

private:
  const Page &page;
  int pageNumber;
  Color inkColor; // Amma's ink, or little Anu's crayon
  float wobble;   // how shaky the line is
  KolamGrid grid;
  bool layoutOk = false;
  std::string layoutError;

  Vector2 origin = {0, 0}; // screen position of grid point (0, 0)
  float cell = 0;          // pixels between two dots
  std::vector<std::vector<Vector2>> lines;

  bool hovering = false;
  GridPoint hover = {0, 0};
  bool solved = false;
  float solvedTime = 0;
  std::string message;
  float messageTimer = 0;

  Vector2 ToScreen(GridPoint p) const {
    return {origin.x + p.x * cell / 2, origin.y + p.y * cell / 2};
  }

  // The walls the player can change are drawn thicker than Amma's ink
  float EditableWallThickness() const { return std::fmax(3.0f, cell * 0.06f); }

  // Finds the spot between two dots closest to the mouse, if it's close enough
  bool SpotUnderMouse(GridPoint *spot) const {
    const Vector2 mouse = GetMousePosition();
    float best = cell * 0.36f;
    bool found = false;
    for (int y = 1; y < 2 * grid.height; y++) {
      for (int x = 1; x < 2 * grid.width; x++) {
        if (!grid.IsBetweenDots(x, y))
          continue;
        const Vector2 s = ToScreen({x, y});
        const float distance = std::hypot(mouse.x - s.x, mouse.y - s.y);
        if (distance < best) {
          best = distance;
          *spot = {x, y};
          found = true;
        }
      }
    }
    return found;
  }

  // A wall is a short bar between two dots, across the kolam line's path.
  void DrawWall(GridPoint p, float thick, Color color) const {
    const Vector2 s = ToScreen(p);
    const float half = cell * 0.275f;
    const bool betweenLeftAndRight = p.x % 2 == 0;
    const Vector2 a = betweenLeftAndRight ? Vector2{s.x, s.y - half}
                                          : Vector2{s.x - half, s.y};
    const Vector2 b = betweenLeftAndRight ? Vector2{s.x, s.y + half}
                                          : Vector2{s.x + half, s.y};
    DrawLineEx(a, b, thick, color);
    DrawCircleV(a, thick / 2, color);
    DrawCircleV(b, thick / 2, color);
  }

  void DrawWalls() const {
    for (int y = 1; y < 2 * grid.height; y++) {
      for (int x = 1; x < 2 * grid.width; x++) {
        if (!grid.IsBetweenDots(x, y) || !grid.IsWall(x, y))
          continue;
        if (grid.IsEditable(x, y))
          DrawWall({x, y}, EditableWallThickness(), KUMKUM);
        else
          DrawWall({x, y}, std::fmax(2.0f, cell * 0.035f), INK_WALL);
      }
    }
  }

  // A ring on every spot the player can change; the hovered one shows a
  // preview of the wall (on symmetric pages, in every corner)
  void DrawChangeableSpots() const {
    const bool canChange = hovering && grid.IsEditable(hover.x, hover.y);
    const std::vector<GridPoint> hovered =
        canChange ? grid.MirrorGroup(hover.x, hover.y)
                  : std::vector<GridPoint>();
    for (int y = 1; y < 2 * grid.height; y++) {
      for (int x = 1; x < 2 * grid.width; x++) {
        if (!grid.IsEditable(x, y))
          continue;
        bool isHovered = false;
        for (const GridPoint &p : hovered)
          isHovered = isHovered || p == GridPoint{x, y};
        const float r = cell * 0.2f;
        DrawRing(ToScreen({x, y}), r - 1.5f, r, 0, 360, 32,
                 Faded(KUMKUM, isHovered ? 0.9f : 0.4f));
        if (isHovered)
          DrawWall({x, y}, EditableWallThickness(),
                   Faded(KUMKUM, grid.IsWall(x, y) ? 0.25f : 0.45f));
      }
    }
  }

  void DrawLines() const {
    const float thick = std::fmax(3.0f, cell * 0.055f);
    if (!solved) {
      for (size_t i = 0; i < lines.size(); i++) {
        const int c = (int)i % LINE_COLOR_COUNT;
        const Color color = page.byLittleAnu ? CRAYON_COLORS[c] : INK_COLORS[c];
        DrawPath(lines[i], lines[i].size(), true, thick, color);
      }
      return;
    }

    // Solved: draw the one line in a single stroke, like a real kolam
    const std::vector<Vector2> &line = lines[0];
    const float t = std::fmin(solvedTime / SOLVED_DRAW_SECONDS, 1.0f);
    const float eased = 1 - (1 - t) * (1 - t); // fast at first, then slower
    const size_t shown = (size_t)(eased * line.size());
    DrawPath(line, line.size(), true, thick, Faded(inkColor, 0.18f));
    DrawPath(line, shown, t >= 1, thick * 1.15f, inkColor);
    if (t < 1 && shown < line.size())
      DrawCircleV(line[shown], thick, inkColor); // the "hand" drawing it
  }
};

std::unique_ptr<Scene> MakePuzzleScene(int pageIndex) {
  return std::unique_ptr<Scene>(new PuzzleScene(pageIndex));
}
