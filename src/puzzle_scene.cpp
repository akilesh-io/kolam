// puzzle_scene.cpp - one page of Amma's kolam book. A page is played one of
// two ways (Page::draw in story.cpp says which):
// - Walls: the player clicks between two dots to add or remove a wall until
//   all the coloured lines join into ONE line.
// - Drawing ("pour the line"): the player holds the button and drags, and the
//   line flows after the mouse from gap to gap, always in clean kolam curves.
//   Amma's ink decides the way at most gaps; at the others (the ? and * in the
//   layout) the line goes the way the finger went. Undo takes it back to the
//   last way chosen; the eraser (swipe over the line) cuts it back to there. The page is
//   done when the line closes round every dot. Amma's finished kolam is open beside the page to copy
//   from (in the page's corner when the screen has no room beside it).
// Either way, the finished line then draws itself in a single stroke.
// A "look only" page (LookAt() in story.cpp) is just looked at, as it is: it
// waits for a later chapter.
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
// (as big as the page allows: players found the dots small on a phone)
const Rectangle GRID_AREA = {60, 160, 600, 420};
// After this long without touching the page, the next step glows
const float STUCK_SECONDS = 8;
const int LINE_COLOR_COUNT = sizeof(INK_COLORS) / sizeof(INK_COLORS[0]);
// How long the line takes to draw itself once the page is solved
const float SOLVED_DRAW_SECONDS = 2.4f;
// A look-only page: how long it is looked at before a click closes it
const float LOOK_SECONDS = 2.5f;
// Drawing pages: how fast the line flows after the mouse (stage units a
// second), and how close to the line's end a press picks it up again
const float POUR_SPEED = 260;
const float GRAB_DISTANCE = 0.6f; // in cells
const float GRAB_DISTANCE_TOUCH = 0.95f; // a fingertip covers the line's end
// The buttons along the foot of the page: Start over on every page that can
// be played; on a drawing page also Undo, and Draw / Erase (the eraser)
const Rectangle START_OVER = {570, 630, 100, 38};
const Rectangle UNDO_BUTTON = {462, 630, 100, 38};
const Rectangle DRAW_TOOL = {80, 630, 92, 38};
const Rectangle ERASE_TOOL = {172, 630, 92, 38};

class PuzzleScene : public Scene {
public:
  PuzzleScene(int pageIndex, bool onlyLook, bool fromMechanics)
      : page(Pages()[pageIndex]), pageNumber(pageIndex + 1),
        lookOnly(onlyLook), practice(fromMechanics), inkColor(page.byLittleAnu ? CRAYON : INK),
        wobble(page.byLittleAnu ? 1.6f : 0) { // a child's wobbly hand
    layoutOk = grid.Load(page, &layoutError);
    PlayEffect("page", 0.8f); // the book opens at this page
    if (!layoutOk)
      return;
    cell = std::fmin(
        std::fmin(GRID_AREA.width / grid.width, GRID_AREA.height / grid.height),
        130.0f);
    origin = {GRID_AREA.x + (GRID_AREA.width - cell * grid.width) / 2,
              GRID_AREA.y + (GRID_AREA.height - cell * grid.height) / 2};
    drawing = page.draw && !lookOnly;
    if (drawing) {
      ResetDrawing();
      MakeReference();
    } else
      lines = LoopPaths(grid, origin, cell, wobble);
  }

  void Update() override {
    const float dt = GetFrameTime();
    if (messageTimer > 0)
      messageTimer -= dt;
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    const bool next = ContinuePressed();
    // how long since the player last touched the page (for stuck hints)
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || GetKeyPressed() != 0) {
      idle = 0;
      hint.valid = false;
    } else {
      idle += dt;
    }

    if (!layoutOk) { // never trap the player on a broken page
      if (next)
        done = true;
      return;
    }

    if (lookOnly) { // nothing to change here, not yet
      lookTime += dt;
      hovering = SpotUnderMouse(&hover);
      if (clicked && hovering) {
        message = "Not yet.";
        messageTimer = 1.8f;
      } else if (next && lookTime >= LOOK_SECONDS) {
        done = true;
      }
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

    if (drawing) {
      UpdateDrawing(dt, clicked);
      return;
    }

    if (IsKeyPressed(KEY_R) || Clicked(START_OVER)) {
      grid.Reset();
      lines = LoopPaths(grid, origin, cell, wobble);
      return;
    }
#if defined(_DEBUG)
    if (IsKeyPressed(KEY_N))
      grid.CountSolutions(true); // debug: solve this page
#endif

    hovering = SpotUnderMouse(&hover);
    const bool canChange = hovering && grid.IsEditable(hover.x, hover.y);
    if (canChange)
      PointAtSomething();

    if (clicked && canChange) {
      grid.Toggle(hover.x, hover.y);
      PlayEffect("scribble", 0.6f);
    } else if (clicked && hovering) {
      message = page.byLittleAnu ? "That part is already drawn in crayon."
                                 : "That one is in Amma's ink. It stays.";
      messageTimer = 1.8f;
    }

    lines = LoopPaths(grid, origin, cell, wobble);
    if (lines.size() == 1)
      solved = true;
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
    // (on a narrow screen Amma's kolam sits in the top-right corner, so the
    // page number goes after the book's name instead)
    const bool cornerTaken = drawing && !solved && ReferenceInCorner();
    if (cornerTaken)
      DrawString(pageLabel, 100 + MeasureString("AMMA'S KOLAM BOOK", 15), 52,
                 15, FAINT);
    else
      DrawString(pageLabel, 640 - MeasureString(pageLabel, 15), 52, 15, FAINT);
    DrawString(page.title, 80, 72, 34, inkColor, HAND_FONT);
    DrawTextWrapped(page.note, 80, 114, 22, cornerTaken ? 440 : 560,
                    Faded(inkColor, 0.85f),
                    HAND_FONT);

    if (!layoutOk) {
      DrawTextWrapped("This page is torn: " + layoutError, 80, 300, 22, 560,
                      KUMKUM);
      return;
    }

    if (drawing && !solved)
      DrawReference();
    DrawWalls();
    DrawLines();
    if (drawing && !solved) {
      rub.DrawFading();
      if (erasing)
        DrawEraserRing();
      else
        DrawTurnPreview();
    }
    if (!solved && !lookOnly && !drawing)
      DrawChangeableSpots();
    if (!solved && !lookOnly && layoutOk)
      DrawStuckHint();
    for (int y = 0; y < grid.height; y++) {
      for (int x = 0; x < grid.width; x++) {
        DrawCircleV(ToScreen({2 * x + 1, 2 * y + 1}),
                    std::fmax(3.0f, cell * 0.065f),
                    page.byLittleAnu ? CRAYON : DOT);
      }
    }

    // Footer
    if (lookOnly) { // left as it is: no line count, no instructions
      if (lookTime >= LOOK_SECONDS)
        DrawGoOn();
    } else if (drawing && !solved) {
      DrawString(closed    ? "Some dots are left out."
                 : erasing ? "Swipe over the line to rub it out."
                           : "Draw one line round every dot.",
                 80, 590, 24, inkColor);
      DrawPageButton(DRAW_TOOL, "Draw", !erasing);
      DrawPageButton(ERASE_TOOL, "Erase", erasing);
      DrawPageButton(UNDO_BUTTON, "Undo", false);
      DrawPageButton(START_OVER, "Start over", false);
    } else if (!solved) {
      DrawString(
          TextFormat("%d separate lines. Make them one.", (int)lines.size()),
          80, 592, 24, inkColor);
      DrawTextWrapped(std::string(Tap()) +
                          (page.symmetric
                               ? " between dots to add or remove a wall "
                                 "(mirrored on every side)."
                               : " between two dots to add or remove a wall."),
                      80, 626, 16, 470, FAINT);
      DrawPageButton(START_OVER, "Start over", false);
    } else {
      DrawString("One unbroken line.", 80, 592, 24, inkColor);
      if (solvedTime >= SOLVED_DRAW_SECONDS)
        DrawGoOn();
    }
    if (messageTimer > 0) // above the buttons, on the right
      DrawString(message, 640 - MeasureString(message, 17), 562, 17,
                 Faded(KUMKUM, messageTimer));
  }

private:
  const Page &page;
  int pageNumber;

  // A button drawn on the page, like a pencilled box; filled in when `on`
  void DrawPageButton(Rectangle r, const char *label, bool on) const {
    const bool over = !TouchUi() && CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.4f, 6,
                         Faded(inkColor, on ? 0.85f : over ? 0.14f : 0.06f));
    DrawRectangleRoundedLinesEx(r, 0.4f, 6, 1.5f, Faded(inkColor, 0.6f));
    DrawTextCentered(label, r.x + r.width / 2, r.y + 9, 18,
                     on ? PAPER : inkColor);
  }
  bool lookOnly;      // only looked at; nothing can be changed
  bool practice;      // from the Mechanics page: no story around it
  bool drawing = false; // a drawing page, played by drawing the line
  float lookTime = 0; // how long a look-only page has been open
  Color inkColor;     // Amma's ink, or little Anu's crayon
  float wobble;       // how shaky the line is
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

  //--------------------------------------------------------------------------
  // Drawing pages
  //--------------------------------------------------------------------------
  // The line is a list of spots (between two dots, or at the border); from
  // each it goes one diagonal step to the next
  struct PenStep {
    GridPoint at;
    int dx, dy;     // the way it leaves this spot
    bool choseHere; // the way at this spot was chosen on this visit
  };
  std::vector<PenStep> pen; // the line so far
  float penT = 0;           // how far it is along the step from pen.back()
  bool pouring = false;     // held down on the line's end: it follows
  bool aiming = false;      // just started: which way will it go?
  bool closed = false;      // it came back round to where it started
  bool finishing = false;   // let go: the line catches up to where it was
  Vector2 finishTo = {0, 0}; // where the finger lifted
  int finishSteps = 0;      // steps taken since
  RubOut rub;               // what Undo or the eraser took, fading away
  bool erasing = false;     // the eraser is chosen (instead of drawing)
  std::vector<Vector2> trail; // where the finger has been, while pouring
  int trailMark = 0;          // where on it the finger passed the last spot
  // Amma's finished kolam, to copy from: its line and dots, in cells (one
  // cell = the distance between two dots, from the grid's top-left corner)
  std::vector<std::vector<Vector2>> referenceLines;
  std::vector<Vector2> referenceDots;

  void MakeReference() {
    KolamGrid finished = grid;
    if (finished.CountSolutions(true) == 0)
      return; // CheckPages() reports a page with no solution
    referenceLines = LoopPaths(finished, {0, 0}, 1);
    for (int y = 0; y < grid.height; y++)
      for (int x = 0; x < grid.width; x++)
        referenceDots.push_back({x + 0.5f, y + 0.5f});
  }

  // Amma's book, open beside the page: in the room to the right of it on a
  // wide screen, or small in the page's bottom-right corner (beside the
  // words at the foot of the page) on a narrow one. The book's page is as
  // wide and tall as her kolam needs.
  // (narrow screens: the top-right corner of the page)
  bool ReferenceInCorner() const {
    return FullScreen().x + FullScreen().width - 690 < 170;
  }

  void DrawReference() const {
    if (referenceLines.empty())
      return;
    const Rectangle screen = FullScreen();
    const float room = screen.x + screen.width - 690; // right of the page
    const bool corner = ReferenceInCorner();
    const float maxW = corner ? 150 : std::fmin(room - 36, 300);
    const float maxH = corner ? 122 : 560;
    const float margin = corner ? 8 : 18;
    const float top = corner ? margin : 34; // room for a heading
    const float unit =
        std::fmin((maxW - 2 * margin) / grid.width,
                  (maxH - top - margin) / grid.height);
    const float w = unit * grid.width + 2 * margin;
    const float h = unit * grid.height + top + margin;
    const Rectangle card =
        corner ? Rectangle{672 - w, 44, w, h}
               : Rectangle{690 + (room - w) / 2, 360 - h / 2, w, h};
    const Vector2 at = {card.x + margin, card.y + top};

    // the open book: a page with the fold's shadow down its left edge
    DrawRectangle((int)card.x + 5, (int)card.y + 7, (int)card.width,
                  (int)card.height, {0, 0, 0, 80});
    DrawRectangleRec(card, PAPER);
    DrawRectangleGradientH((int)card.x, (int)card.y, (int)(card.width * 0.12f),
                           (int)card.height, {0, 0, 0, 45}, {0, 0, 0, 0});
    DrawRectangleLinesEx(card, 1, Faded(FAINT, 0.5f));

    const float thick = std::fmax(1.5f, unit * 0.06f);
    for (const std::vector<Vector2> &line : referenceLines) {
      std::vector<Vector2> placed;
      for (const Vector2 &p : line)
        placed.push_back({at.x + p.x * unit, at.y + p.y * unit});
      DrawPath(placed, placed.size(), true, thick, INK);
    }
    for (const Vector2 &dot : referenceDots)
      DrawCircleV({at.x + dot.x * unit, at.y + dot.y * unit},
                  std::fmax(1.2f, unit * 0.065f), DOT);
    if (!corner) // a small heading in her book
      DrawString("AMMA'S KOLAM", card.x + margin, card.y + 12, 13, FAINT);
  }
  std::vector<bool> chosen; // per spot: has the way there been chosen yet?

  void ResetDrawing() {
    grid.Reset();
    pen.clear();
    penT = 0;
    pouring = aiming = closed = finishing = false;
    chosen.assign((2 * grid.width + 1) * (2 * grid.height + 1), false);
  }

  int SpotIndex(GridPoint p) const { return p.y * (2 * grid.width + 1) + p.x; }
  static GridPoint Next(const PenStep &s) {
    return {s.at.x + s.dx, s.at.y + s.dy};
  }
  // A ? or * spot whose way hasn't been chosen yet
  bool Undecided(GridPoint p) const {
    return grid.IsEditable(p.x, p.y) && !chosen[SpotIndex(p)];
  }
  // A wall turns the line back: across it if it stands between left and
  // right dots, up/down if between upper and lower ones
  static void Bounce(GridPoint p, int &dx, int &dy) {
    if (p.x % 2 == 0)
      dx = -dx;
    else
      dy = -dy;
  }
  void Choose(GridPoint p, bool wall, bool isChosen) {
    if (isChosen && grid.IsWall(p.x, p.y) != wall)
      grid.Toggle(p.x, p.y); // and its mirror images, on symmetric pages
    for (const GridPoint &m : grid.MirrorGroup(p.x, p.y))
      chosen[SpotIndex(m)] = isChosen;
  }

  // The line reaches spot `p` going (dx, dy): which way does it leave?
  // The finger's trail is read in order, like the line follows it:
  // `trailMark` is where on the trail the finger passed the line's last
  // spot. FirstPass() finds where, after `from`, the finger first came
  // closest to spot `p` (or -1 if it hasn't come near it yet).
  int FirstPass(GridPoint p, int from) const {
    const Vector2 spot = ToScreen(p);
    const float step = cell * 0.707f;
    // (by the trail's segments, not just its points: on a slow frame the
    // points are far apart, and a spot can fall between two of them)
    for (int i = std::max(0, from); i + 1 < (int)trail.size(); i++) {
      const float nearest = DistanceToSegment(spot, trail[i], trail[i + 1]);
      if (nearest >= step * 0.45f)
        continue;
      // a pass, only if the finger has gone away from it since (a finger
      // still on its way there hasn't passed it yet)
      for (int j = i + 1; j < (int)trail.size(); j++)
        if (Distance(trail[j], spot) > nearest + step * 0.2f &&
            Distance(trail[j], spot) > step * 0.3f)
          return i;
      return -1;
    }
    return -1;
  }


  // Where the finger went after it passed spot `p`. A quick finger is often
  // steps ahead of the line (it may even have come back near `p` since), so
  // where it is now can mislead: find its first pass of `p` after the line's
  // last spot, and take where it was half a step after that. (If it never
  // came near `p`, where it is now.)
  Vector2 WhereFingerWent(GridPoint p) const {
    const int passed = FirstPass(p, trailMark);
    if (passed < 0)
      return GetMousePosition();
    const Vector2 spot = ToScreen(p);
    for (size_t i = passed + 1; i < trail.size(); i++)
      if (Distance(trail[i], spot) >= cell * 0.707f * 0.5f)
        return trail[i];
    return GetMousePosition();
  }

  PenStep Arrive(GridPoint p, int dx, int dy) {
    PenStep step = {p, dx, dy, false};
    if (Undecided(p)) { // the player's choice: the way the finger went
      int bdx = dx, bdy = dy;
      Bounce(p, bdx, bdy);
      const Vector2 went = WhereFingerWent(p);
      const float straight = Distance(went, ToScreen({p.x + dx, p.y + dy}));
      const float back = Distance(went, ToScreen({p.x + bdx, p.y + bdy}));
      Choose(p, back < straight, true);
      step.choseHere = true;
    }
    if (grid.IsWall(p.x, p.y)) // the border counts as a wall
      Bounce(p, step.dx, step.dy);
    return step;
  }

  static float Distance(Vector2 a, Vector2 b) {
    return std::hypot(a.x - b.x, a.y - b.y);
  }

  Vector2 PenTip() const {
    if (pen.empty())
      return {0, 0};
    const Vector2 a = ToScreen(pen.back().at);
    if (aiming)
      return a;
    const Vector2 b = ToScreen(Next(pen.back()));
    return {a.x + (b.x - a.x) * penT, a.y + (b.y - a.y) * penT};
  }

  // Where a new line can start: the spot nearest the mouse whose way Amma's
  // ink already decides (a ? or * spot would be a choice made blind)
  bool StartSpotNear(Vector2 mouse, GridPoint *spot) const {
    float best = cell * (TouchUi() ? 0.7f : 0.5f); // a fingertip is less exact
    bool found = false;
    for (int y = 0; y <= 2 * grid.height; y++) {
      for (int x = 0; x <= 2 * grid.width; x++) {
        if ((x + y) % 2 == 0 || grid.IsEditable(x, y))
          continue;
        const float d = Distance(mouse, ToScreen({x, y}));
        if (d < best) {
          best = d;
          *spot = {x, y};
          found = true;
        }
      }
    }
    return found;
  }

  void UpdateDrawing(float dt, bool clicked) {
    if (IsKeyPressed(KEY_R) || Clicked(START_OVER)) {
      ResetDrawing();
      return;
    }
#if defined(_DEBUG)
    if (IsKeyPressed(KEY_N)) { // debug: solve this page
      grid.CountSolutions(true);
      lines = LoopPaths(grid, origin, cell, wobble);
      solved = true;
      return;
    }
#endif
    const Vector2 mouse = GetMousePosition();
    GridPoint start = {0, 0};
    const bool nearEnd =
        !pen.empty() &&
        Distance(mouse, PenTip()) <
            cell * (TouchUi() ? GRAB_DISTANCE_TOUCH : GRAB_DISTANCE);
    // The tools and Undo
    if (Clicked(DRAW_TOOL) || Clicked(ERASE_TOOL)) {
      erasing = CheckCollisionPointRec(mouse, ERASE_TOOL);
      pouring = false;
      return;
    }
    if (Clicked(UNDO_BUTTON)) {
      UndoStretch();
      return;
    }
    const bool onButtons = mouse.y >= DRAW_TOOL.y - 6;
    if (erasing) { // swipe over the line: it's cut back to there
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !onButtons && !pen.empty() &&
          LineStepNear(mouse, nullptr, nullptr)) {
        RubOutFrom(mouse);
        PlayLoop("pencil", 0.3f);
      }
      return;
    }
    if (onButtons && clicked)
      return;
    const bool onLine = !pen.empty() && LineStepNear(mouse, nullptr, nullptr);
    if (clicked) {
      if (pen.empty() && StartSpotNear(mouse, &start)) {
        pen.push_back({start, 0, 0, false});
        aiming = true;
        pouring = true;
        trail.clear();
        trailMark = 0;
      } else if (nearEnd) {
        pouring = true;
        finishing = false;
        trail.clear();
        trailMark = 0;
      } else if (!pen.empty() && !onLine) {
        message = "Carry on from the end of the line.";
        messageTimer = 1.8f;
      }
    }
    if (pouring && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      // let go while the line is still catching up: it carries on to where
      // the finger lifted (a quick finger can be a step or two ahead), so
      // letting go right at the start still closes it
      pouring = false;
      finishing = !aiming && !closed;
      finishTo = mouse;
      finishSteps = 0;
    }
    if (finishing)
      Finish(dt);
    if (!pouring) {
      if (pen.empty() ? StartSpotNear(mouse, &start) : nearEnd)
        PointAtSomething();
      return;
    }
    PointAtSomething();

    // the finger's trail, for choosing the way at a spot (WhereFingerWent)
    if (trail.empty() || Distance(trail.back(), mouse) > 3)
      trail.push_back(mouse);
    if (trail.size() > 600) {
      trail.erase(trail.begin(), trail.begin() + 100);
      trailMark = std::max(0, trailMark - 100);
    }

    if (aiming) { // the first step goes the way the mouse moves
      const Vector2 from = ToScreen(pen[0].at);
      if (Distance(mouse, from) < cell * 0.2f)
        return;
      PenStep &first = pen[0];
      first.dx = mouse.x < from.x ? -1 : 1;
      first.dy = mouse.y < from.y ? -1 : 1;
      if (first.at.x + first.dx < 0 || first.at.x + first.dx > 2 * grid.width)
        first.dx = -first.dx;
      if (first.at.y + first.dy < 0 || first.at.y + first.dy > 2 * grid.height)
        first.dy = -first.dy;
      aiming = false;
      penT = 0;
      return;
    }

    // Follow the mouse: find the place on the line (this step, the one before
    // it, or the steps ahead as far as their way is already settled) nearest
    // the mouse, and move the line's end towards it. `target` counts in
    // steps: 0..1 along this one, -1..0 along the last, 1.. ahead. A quick
    // finger is often a few steps ahead; this way the line still follows.
    const Vector2 a = ToScreen(pen.back().at);
    const Vector2 b = ToScreen(Next(pen.back()));
    const float length = Distance(a, b);
    float nearest = 1e9f, target = penT;
    auto consider = [&](Vector2 from, Vector2 to, float startsAt) {
      const float lengthSquared =
          (to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y);
      float t = ((mouse.x - from.x) * (to.x - from.x) +
                 (mouse.y - from.y) * (to.y - from.y)) /
                lengthSquared;
      t = std::fmax(0.0f, std::fmin(1.0f, t));
      const float d = Distance(mouse, {from.x + (to.x - from.x) * t,
                                       from.y + (to.y - from.y) * t});
      if (d < nearest) {
        nearest = d;
        target = startsAt + t;
      }
    };
    consider(a, b, 0);
    { // the steps ahead, up to the next way still to choose (or the start)
      GridPoint at = Next(pen.back());
      int dx = pen.back().dx, dy = pen.back().dy;
      Vector2 from = b;
      for (int k = 1; k <= 4; k++) {
        if (Undecided(at) || (at == pen[0].at))
          break;
        if (grid.IsWall(at.x, at.y))
          Bounce(at, dx, dy);
        at = {at.x + dx, at.y + dy};
        const Vector2 to = ToScreen(at);
        consider(from, to, (float)k);
        from = to;
      }
    }
    // and by the finger's trail: if it has already passed the next spots, on
    // to them, however far it is now (it may even be back near the line)
    target = std::fmax(target, TrailAhead());
    // (at a square corner the mouse's place comes out a hair short of the
    // corner, so "nearly there" counts as there)
    const bool pastCorner = target > 0.995f;
    const float gap = (target - penT) * length; // + ahead, - behind
    // The line flows calmly after a slow finger, and hurries to catch up
    // with a quick one (never more than one step a frame)
    const float speed =
        std::fmax(POUR_SPEED, std::fmin(2600.0f, std::fmax(0.0f, gap) * 12));
    const float move = std::fmin(0.95f, speed * dt / length);

    if (!closed && (gap > cell * 0.06f || pastCorner)) {
      // on to the mouse, or (when it's past the corner) to the corner
      PlayLoop("pencil", 0.45f);
      penT += pastCorner ? move : std::fmin(move, gap / length);
      if (penT >= 1 && !ReadyToChoose())
        penT = 0.999f; // wait until it's clear which way the mouse went
      if (penT >= 1)
        ReachNextSpot();
    }
    // (going back is Undo's and the eraser's job: a finger crossing back over
    // the line, as a kolam's line does, must never rub it out)
  }

  // The step of the line nearest `p`, if it's close: the latest one, so as
  // little as possible is rubbed out where the line crosses itself. `step`
  // is its number (pen[step] to the next), `t` how far along it.
  bool LineStepNear(Vector2 p, int *step, float *t) const {
    const int count = (int)pen.size();
    for (int i = count - 1; i >= 0; i--) {
      const Vector2 a = ToScreen(pen[i].at);
      const Vector2 b = i == count - 1 ? PenTip() : ToScreen(pen[i + 1].at);
      if (aiming || DistanceToSegment(p, a, b) > cell * 0.3f)
        continue;
      if (step)
        *step = i;
      if (t) {
        const float length = Distance(a, b);
        const float along =
            length < 0.01f ? 0
                           : ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) /
                                 (length * length);
        // on the last step, t counts along the whole step, not just the
        // part drawn so far
        *t = std::fmax(0.0f, std::fmin(1.0f, along)) *
             (i == count - 1 ? penT : 1.0f);
      }
      return true;
    }
    return false;
  }

  // Undo: takes the line back to just before the last way the player chose
  // (and unchooses it), or all of it if there were no choices
  void UndoStretch() {
    if (pen.empty())
      return;
    int last = (int)pen.size() - 1;
    while (last > 0 && !pen[last].choseHere)
      last--;
    std::vector<Vector2> gone;
    for (size_t i = std::max(0, last - 1); i < pen.size(); i++)
      gone.push_back(ToScreen(pen[i].at));
    gone.push_back(closed ? ToScreen(pen[0].at) : PenTip());
    rub.Fade(Smooth(gone, false), std::fmax(3.0f, cell * 0.055f) * 1.1f,
             inkColor);
    PlayEffect("scribble", 0.6f, 0.8f);
    if (last <= 0) { // nothing chosen yet: the whole line goes
      ResetDrawing();
      return;
    }
    const float drawn = (float)(pen.size() - 1) + penT;
    TakeBack(drawn - (last - 0.001f)); // stops just short of that spot
    pouring = finishing = false;
  }

  // Rubs the line out from the point nearest `p` to its end (fading away)
  void RubOutFrom(Vector2 p) {
    int step = 0;
    float t = 0;
    if (!LineStepNear(p, &step, &t))
      return;
    // what goes: from there along the line to its end, for the fade
    std::vector<Vector2> gone;
    const Vector2 a = ToScreen(pen[step].at);
    const Vector2 b = step + 1 < (int)pen.size() ? ToScreen(pen[step + 1].at)
                                                 : ToScreen(Next(pen[step]));
    gone.push_back({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t});
    for (size_t i = step + 1; i < pen.size(); i++)
      gone.push_back(ToScreen(pen[i].at));
    gone.push_back(closed ? ToScreen(pen[0].at) : PenTip());
    rub.Fade(Smooth(gone, false), std::fmax(3.0f, cell * 0.055f) * 1.1f,
             inkColor);
    // then take it back, undoing the choices made on the way
    const float drawn = (float)(pen.size() - 1) + penT;
    TakeBack(drawn - (step + t));
    pouring = finishing = false;
  }

  // After letting go: on along the line towards `finishTo`, step by step
  // (a few at most), while each step heads closer to it
  void Finish(float dt) {
    const Vector2 a = ToScreen(pen.back().at), b = ToScreen(Next(pen.back()));
    // only on along this step if the finger went that way
    const bool wentOn = FirstPass(Next(pen.back()), trailMark) >= 0 ||
                        Distance(finishTo, b) < Distance(finishTo, a) - 1;
    if (penT < 0.5f && !wentOn) {
      finishing = false;
      return;
    }
    const float speed = 2000; // catching up
    penT += std::fmin(0.95f, speed * dt / Distance(a, b));
    if (penT < 1)
      return;
    if (!ReadyToChoose()) { // a way to choose: wait for the finger
      penT = 0.999f;
      finishing = false;
      return;
    }
    penT = 1;
    ReachNextSpot();
    if (closed) {
      finishing = false;
      return;
    }
    penT = 0;
    finishSteps++;
    // on while the finger's trail went further (or the next spot is nearer
    // where it lifted)
    const bool closer =
        Distance(ToScreen(Next(pen.back())), finishTo) <
        Distance(ToScreen(pen.back().at), finishTo) - 1;
    const bool trailWent = FirstPass(Next(pen.back()), trailMark) >= 0;
    if (finishSteps >= 60 || !(trailWent || (closer && finishSteps < 3)))
      finishing = false;
  }

  // Reaching a spot whose way isn't chosen yet: the mouse has to be clearly
  // past it, or the choice would be a guess
  bool ReadyToChoose() const {
    const GridPoint next = Next(pen.back());
    if (!Undecided(next))
      return true;
    const int passed = FirstPass(next, trailMark);
    if (passed < 0) // the finger hasn't been there: is it clearly away?
      return Distance(GetMousePosition(), ToScreen(next)) > cell * 0.15f;
    // it passed the spot: ready once it has gone on half a step
    for (size_t i = passed + 1; i < trail.size(); i++)
      if (Distance(trail[i], ToScreen(next)) >= cell * 0.707f * 0.5f)
        return true;
    return false;
  }

  // How many steps ahead the finger's trail says it has gone: the spots
  // along the line it has passed, in order, since the line's last spot
  // (stopping at a way still to choose, which it can't see past)
  float TrailAhead() const {
    GridPoint at = Next(pen.back());
    int dx = pen.back().dx, dy = pen.back().dy;
    int from = trailMark;
    float ahead = 0;
    for (int k = 1; k <= 12; k++) {
      const int passed = FirstPass(at, from);
      if (passed < 0)
        break;
      ahead = (float)k;
      from = passed;
      if (Undecided(at) || at == pen[0].at)
        break;
      if (grid.IsWall(at.x, at.y))
        Bounce(at, dx, dy);
      at = {at.x + dx, at.y + dy};
    }
    return ahead;
  }

  void ReachNextSpot() {
    const PenStep &last = pen.back();
    const PenStep step = Arrive(Next(last), last.dx, last.dy);
    const PenStep &first = pen[0];
    if (step.at == first.at && step.dx == first.dx && step.dy == first.dy) {
      closed = true; // round to the start
      penT = 1;
      pouring = false;
      if ((int)pen.size() == 4 * grid.width * grid.height) {
        // every dot is inside one line: Amma's kolam
        lines = LoopPaths(grid, origin, cell, wobble);
        solved = true;
      } else {
        PlayEffect("click", 0.5f);
      }
      return;
    }
    pen.push_back(step);
    penT -= 1;
    hint.valid = false;
    if (pen.size() == 4) // drawn a few steps: the player knows how to drag
      GestureDone(MOVE_DRAG);
    const int passed = FirstPass(step.at, trailMark);
    if (passed >= 0)
      trailMark = passed;
  }

  // Dragging back along the line rubs it out, undoing the choices on the way
  void TakeBack(float amount) {
    closed = false;
    trailMark = (int)trail.size(); // the trail so far no longer matches
    penT -= amount;
    while (penT < 0) {
      if (pen.size() == 1) { // all gone
        ResetDrawing();
        return;
      }
      if (pen.back().choseHere)
        Choose(pen.back().at, false, false);
      pen.pop_back();
      penT += 1;
    }
  }

  // Rounds the line's corners like LoopPaths() does (Chaikin's method)
  static std::vector<Vector2> Smooth(std::vector<Vector2> path, bool loop) {
    for (int round = 0; round < 3 && path.size() > 1; round++) {
      std::vector<Vector2> smoother;
      if (!loop)
        smoother.push_back(path.front());
      const size_t count = loop ? path.size() : path.size() - 1;
      for (size_t i = 0; i < count; i++) {
        const Vector2 a = path[i], b = path[(i + 1) % path.size()];
        smoother.push_back({0.75f * a.x + 0.25f * b.x, 0.75f * a.y + 0.25f * b.y});
        smoother.push_back({0.25f * a.x + 0.75f * b.x, 0.25f * a.y + 0.75f * b.y});
      }
      if (!loop)
        smoother.push_back(path.back());
      path = smoother;
    }
    return path;
  }

  void DrawPoured(float thick) const {
    const Vector2 mouse = GetMousePosition();
    GridPoint start = {0, 0};
    if (pen.empty()) { // where a line could start
      if (StartSpotNear(mouse, &start))
        DrawCircleV(ToScreen(start), thick * 1.4f, Faded(inkColor, 0.4f));
      return;
    }
    std::vector<Vector2> points;
    for (const PenStep &s : pen)
      points.push_back(ToScreen(s.at));
    if (!closed)
      points.push_back(PenTip());
    const std::vector<Vector2> path = Smooth(points, closed);
    DrawPath(path, path.size(), closed, thick * 1.1f, inkColor);
    if (!closed && !pouring) { // the end of the line, to carry on from
      const float pulse = 0.5f + 0.5f * std::sin((float)GetTime() * 4);
      DrawCircleV(PenTip(), thick * (1.3f + 0.5f * pulse), Faded(inkColor, 0.5f));
    }
    DrawCircleV(PenTip(), thick * 0.9f, inkColor);
  }

  //--------------------------------------------------------------------------
  // Help for a player who seems stuck, and for seeing the turns
  //--------------------------------------------------------------------------
  float idle = 0; // seconds since the page was last touched
  struct Hint {
    bool valid = false;   // worked out for the page as it is now
    bool found = false;   // a solution can still be reached from here
    KolamGrid solution;   // that solution (nearest to the page as it is)
  } hint;

  // The solution nearest the page as it is: every way of setting the open
  // spots (on a drawing page: the ways not chosen yet; on a wall page: all
  // the spots that can change), keeping the one that changes fewest
  bool NearestSolution(KolamGrid *out) const {
    std::vector<GridPoint> open; // one spot per mirror group
    for (int y = 1; y < 2 * grid.height; y++)
      for (int x = 1; x < 2 * grid.width; x++) {
        if (!grid.IsEditable(x, y) || (drawing && !Undecided({x, y})))
          continue;
        bool covered = false;
        for (const GridPoint &o : open)
          for (const GridPoint &m : grid.MirrorGroup(o.x, o.y))
            covered = covered || m == GridPoint{x, y};
        if (!covered)
          open.push_back({x, y});
      }
    if (open.size() > 14)
      return false;
    KolamGrid g = grid;
    long long best = -1;
    int fewest = 1 << 30;
    for (long long mask = 0; mask < (1LL << open.size()); mask++) {
      int changes = 0;
      for (size_t i = 0; i < open.size(); i++) {
        const bool wall = (mask >> i) & 1;
        if (g.IsWall(open[i].x, open[i].y) != wall)
          g.Toggle(open[i].x, open[i].y);
        changes += grid.IsWall(open[i].x, open[i].y) != wall;
      }
      if (changes < fewest && g.Loops().size() == 1) {
        fewest = changes;
        best = mask;
      }
    }
    if (best < 0)
      return false;
    for (size_t i = 0; i < open.size(); i++)
      if (g.IsWall(open[i].x, open[i].y) != (bool)((best >> i) & 1))
        g.Toggle(open[i].x, open[i].y);
    *out = g;
    return true;
  }

  // Where the line goes next from its end, as far as it's settled: the
  // first spot whose way is still to choose (and how it arrives there).
  // False if there's none ahead before the line comes round to its start.
  bool NextChoice(GridPoint *spot, int *dx, int *dy) const {
    if (pen.empty() || aiming || closed)
      return false;
    GridPoint at = Next(pen.back());
    int x = pen.back().dx, y = pen.back().dy;
    for (int k = 0; k < 400; k++) {
      if (Undecided(at)) {
        *spot = at;
        *dx = x;
        *dy = y;
        return true;
      }
      if (grid.IsWall(at.x, at.y))
        Bounce(at, x, y);
      if (at == pen[0].at && x == pen[0].dx && y == pen[0].dy)
        return false;
      at = {at.x + x, at.y + y};
    }
    return false;
  }

  // A short stretch of kolam curve: from `a` through `through` towards `b`
  void DrawCurveBit(Vector2 a, Vector2 through, Vector2 b, float thick,
                    Color c) const {
    const Vector2 from = {(a.x + through.x) / 2, (a.y + through.y) / 2};
    const Vector2 to = {(through.x + b.x) / 2, (through.y + b.y) / 2};
    std::vector<Vector2> path = Smooth({from, through, to}, false);
    path.push_back(b);
    DrawPath(path, path.size(), false, thick, c);
  }

  // While drawing, near a turn still to choose: both ways it could go,
  // faint, and the one the finger is heading for a little brighter. (A
  // fingertip covers the line's end: this shows the turn anyway.)
  void DrawTurnPreview() const {
    GridPoint q;
    int dx = 0, dy = 0;
    if (!pouring || !NextChoice(&q, &dx, &dy))
      return;
    const Vector2 at = ToScreen(q);
    if (Distance(PenTip(), at) > cell * 1.3f) // only when it's close
      return;
    int bdx = dx, bdy = dy;
    Bounce(q, bdx, bdy);
    const Vector2 from = ToScreen({q.x - dx, q.y - dy});
    const Vector2 straight = ToScreen({q.x + dx, q.y + dy});
    const Vector2 back = ToScreen({q.x + bdx, q.y + bdy});
    const Vector2 went = GetMousePosition();
    const bool toBack = Distance(went, back) < Distance(went, straight);
    const float thick = std::fmax(3.0f, cell * 0.055f);
    DrawCurveBit(from, at, straight, thick, Faded(inkColor, toBack ? 0.12f : 0.35f));
    DrawCurveBit(from, at, back, thick, Faded(inkColor, toBack ? 0.35f : 0.12f));
  }

  // Stuck for a while: show the next step
  void DrawStuckHint() {
    const float t = (float)GetTime();
    if (drawing && pen.empty() && !erasing &&
        (idle > 3 || !GestureLearned(MOVE_DRAG))) {
      // how to start: drag from the edge, along the first step
      GridPoint start = {0, 1};
      for (int x = 1; x < 2 * grid.width; x += 2) // a fixed spot on the top
        if (!grid.IsEditable(x, 0)) {
          start = {x, 0};
          break;
        }
      const Vector2 a = ToScreen(start);
      const Vector2 b = ToScreen({start.x + 1, start.y + 1});
      const float n = Distance(a, b);
      DrawGesture(MOVE_DRAG, a, t, 1, {(b.x - a.x) / n, (b.y - a.y) / n},
                  inkColor);
      return;
    }
    if (idle < STUCK_SECONDS && !(drawing && closed && idle > 2))
      return;
    if (!hint.valid) { // worked out once, until the page changes
      hint.found = NearestSolution(&hint.solution);
      hint.valid = true;
    }
    const float pulse = 0.5f + 0.5f * std::sin(t * 4);
    if (!hint.found || (drawing && closed)) {
      // something drawn earlier went the wrong way: Undo is the way out
      if (drawing) {
        const Rectangle u = UNDO_BUTTON;
        DrawRectangleRoundedLinesEx({u.x - 4, u.y - 4, u.width + 8, u.height + 8},
                                    0.4f, 6, 3, Faded(KUMKUM, pulse));
      }
      return;
    }
    if (!drawing) { // a wall page: one spot to change, glowing
      for (int y = 1; y < 2 * grid.height; y++)
        for (int x = 1; x < 2 * grid.width; x++)
          if (grid.IsEditable(x, y) &&
              grid.IsWall(x, y) != hint.solution.IsWall(x, y)) {
            const Vector2 at = ToScreen({x, y});
            DrawCircleV(at, cell * 0.22f, Faded({255, 210, 120, 255}, 0.35f * pulse));
            DrawGesture(MOVE_TAP, at, t, 1, {-1, 0}, inkColor);
            return;
          }
      return;
    }
    // a drawing page: the right way at the next turn, in gold
    GridPoint q;
    int dx = 0, dy = 0;
    if (!NextChoice(&q, &dx, &dy))
      return;
    int odx = dx, ody = dy;
    if (hint.solution.IsWall(q.x, q.y))
      Bounce(q, odx, ody);
    const Vector2 from = ToScreen({q.x - dx, q.y - dy});
    const float thick = std::fmax(3.0f, cell * 0.055f);
    DrawCurveBit(from, ToScreen(q), ToScreen({q.x + odx, q.y + ody}), thick * 1.6f,
                 Faded({230, 170, 60, 255}, 0.4f + 0.4f * pulse));
    if (!pouring) // and where to carry on from
      DrawRing(PenTip(), cell * 0.25f, cell * 0.25f + 3, 0, 360, 32,
               Faded(KUMKUM, pulse));
  }

  Vector2 ToScreen(GridPoint p) const {
    return {origin.x + p.x * cell / 2, origin.y + p.y * cell / 2};
  }

  // A blinking "go on" at the foot of the page
  void DrawGoOn() const {
    const float blink = 0.55f + 0.45f * std::sin((float)GetTime() * 3);
    const char *goOn = practice ? " to go back to the pages"
                       : pageNumber < (int)Pages().size() ? " to turn the page"
                                                          : " to close the book";
    DrawString(std::string(Tap()) + goOn,
               80, 626, 17, Faded(FAINT, blink));
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
        if (grid.IsEditable(x, y) && drawing)
          continue; // shown by the line itself
        if (grid.IsEditable(x, y) && !lookOnly)
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
    if (drawing && !solved) {
      DrawPoured(thick);
      return;
    }
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

std::unique_ptr<Scene> MakePuzzleScene(int pageIndex, bool lookOnly,
                                       bool practice) {
  return std::unique_ptr<Scene>(new PuzzleScene(pageIndex, lookOnly, practice));
}
