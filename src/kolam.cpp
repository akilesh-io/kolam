// kolam.cpp - the rules of the kolam puzzle and how kolam lines are drawn.
//
// How the puzzle works (the "mirror model" of sikku kolams):
// The grid uses "doubled" coordinates, the same as the layout strings in
// story.cpp: dots sit at (odd, odd), and the spots between two neighbouring
// dots sit at (even, odd) or (odd, even). The kolam line moves diagonally from
// spot to spot. At a gap it crosses straight through; at a wall (or the border)
// it bounces back. Following the line until it returns to where it started
// gives one closed loop. The page is solved when there is only one loop.
#include <cmath>
#include <cstdio>

#include "game.h"

bool KolamGrid::Load(const Page &page, std::string *error) {
  auto fail = [&](const std::string &message) {
    if (error)
      *error = message;
    return false;
  };
  const std::vector<std::string> &layout = page.layout;
  if (layout.size() < 3 || layout.size() % 2 == 0)
    return fail("the layout needs an odd number of rows");
  for (const std::string &row : layout) {
    if (row.size() != layout[0].size() || row.size() % 2 == 0) {
      return fail("every row needs the same, odd number of characters");
    }
  }

  rows = original = layout;
  symmetric = page.symmetric;
  width = (int)layout[0].size() / 2;
  height = (int)layout.size() / 2;

  for (int y = 0; y < (int)rows.size(); y++) {
    for (int x = 0; x < (int)rows[y].size(); x++) {
      const char c = rows[y][x];
      const std::string where =
          " at row " + std::to_string(y) + ", column " + std::to_string(x);
      if (x % 2 == 1 && y % 2 == 1 && c != 'o')
        return fail("expected a dot 'o'" + where);
      if (!IsBetweenDots(x, y))
        continue;
      if (c != '#' && c != '.' && c != '?' && c != '*')
        return fail(std::string("unknown character '") + c + "'" + where);
      for (const GridPoint &p : MirrorGroup(x, y)) {
        if (rows[p.y][p.x] != c)
          return fail("symmetric page is not the same in every corner" + where);
      }
    }
  }
  return true;
}

bool KolamGrid::IsBetweenDots(int x, int y) const {
  const bool inside = x > 0 && y > 0 && x < 2 * width && y < 2 * height;
  return inside && (x + y) % 2 == 1;
}

bool KolamGrid::IsWall(int x, int y) const {
  if (!IsBetweenDots(x, y))
    return true; // the border
  return rows[y][x] == '#' || rows[y][x] == '*';
}

bool KolamGrid::IsEditable(int x, int y) const {
  return IsBetweenDots(x, y) && (rows[y][x] == '?' || rows[y][x] == '*');
}

std::vector<GridPoint> KolamGrid::MirrorGroup(int x, int y) const {
  std::vector<GridPoint> group = {{x, y}};
  if (!symmetric)
    return group;
  const GridPoint mirrored[] = {
      {2 * width - x, y}, {x, 2 * height - y}, {2 * width - x, 2 * height - y}};
  for (const GridPoint &p : mirrored) {
    bool alreadyIn = false;
    for (const GridPoint &g : group)
      alreadyIn = alreadyIn || g == p;
    if (!alreadyIn)
      group.push_back(p);
  }
  return group;
}

void KolamGrid::Toggle(int x, int y) {
  if (!IsEditable(x, y))
    return;
  const char newValue = IsWall(x, y) ? '?' : '*';
  for (const GridPoint &p : MirrorGroup(x, y))
    rows[p.y][p.x] = newValue;
}

std::vector<std::vector<GridPoint>> KolamGrid::Loops() const {
  // Every dot has four short strokes around it (one per corner), and each
  // stroke belongs to exactly one loop. `used` remembers strokes already
  // followed, indexed by the stroke's middle point in quadrupled coordinates.
  const int usedWidth = 4 * width + 1;
  std::vector<bool> used(usedWidth * (4 * height + 1), false);
  auto strokeId = [&](GridPoint p, int dx, int dy) {
    return (2 * p.y + dy) * usedWidth + (2 * p.x + dx);
  };

  std::vector<std::vector<GridPoint>> loops;
  for (int dotY = 1; dotY < 2 * height; dotY += 2) {
    for (int dotX = 1; dotX < 2 * width; dotX += 2) {
      for (int side : {-1, 1}) {
        for (int upDown : {-1, 1}) {
          // Start at the spot beside the dot, heading towards the spot
          // above/below it.
          const GridPoint start = {dotX + side, dotY};
          const int startDx = -side;
          const int startDy = upDown;
          if (used[strokeId(start, startDx, startDy)])
            continue;

          std::vector<GridPoint> loop;
          GridPoint p = start;
          int dx = startDx;
          int dy = startDy;
          do {
            loop.push_back(p);
            used[strokeId(p, dx, dy)] = true;
            p = {p.x + dx, p.y + dy};
            if (IsWall(p.x, p.y)) {
              if (p.x % 2 == 0)
                dx = -dx; // wall between left and right dots
              else
                dy = -dy; // wall between upper and lower dots
            }
          } while (!(p == start && dx == startDx && dy == startDy));
          loops.push_back(loop);
        }
      }
    }
  }
  return loops;
}

int KolamGrid::CountSolutions(bool apply) {
  // One editable spot per mirror group (the rest of the group follows it).
  std::vector<GridPoint> choices;
  for (int y = 0; y < (int)rows.size(); y++) {
    for (int x = 0; x < (int)rows[y].size(); x++) {
      if (!IsEditable(x, y))
        continue;
      bool covered = false;
      for (const GridPoint &c : choices) {
        for (const GridPoint &p : MirrorGroup(c.x, c.y))
          covered = covered || p == GridPoint{x, y};
      }
      if (!covered)
        choices.push_back({x, y});
    }
  }

  // Every choice is a wall or a gap, so there are 2^choices arrangements. Each
  // bit of `mask` is one choice. (Each extra choice doubles the time this
  // takes, which is fine for the 1-10 choices a page has.)
  const std::vector<std::string> before = rows;
  std::vector<std::string> firstSolution;
  int solutions = 0;
  for (long long mask = 0; mask < (1LL << choices.size()); mask++) {
    for (size_t i = 0; i < choices.size(); i++) {
      const bool wantWall = (mask >> i) & 1;
      if (IsWall(choices[i].x, choices[i].y) != wantWall)
        Toggle(choices[i].x, choices[i].y);
    }
    if (Loops().size() == 1) {
      if (solutions == 0)
        firstSolution = rows;
      solutions++;
    }
  }
  rows = (apply && solutions > 0) ? firstSolution : before;
  return solutions;
}

std::vector<std::vector<Vector2>>
LoopPaths(const KolamGrid &grid, Vector2 origin, float cell, float wobble) {
  std::vector<std::vector<Vector2>> paths;
  for (const std::vector<GridPoint> &loop : grid.Loops()) {
    std::vector<Vector2> path;
    for (const GridPoint &p : loop)
      path.push_back({origin.x + p.x * cell / 2, origin.y + p.y * cell / 2});

    // Round off the corners three times (Chaikin's method) so the zig-zag
    // becomes the smooth curves of a hand-drawn kolam.
    for (int round = 0; round < 3; round++) {
      std::vector<Vector2> smoother;
      for (size_t i = 0; i < path.size(); i++) {
        const Vector2 a = path[i];
        const Vector2 b = path[(i + 1) % path.size()];
        smoother.push_back(
            {0.75f * a.x + 0.25f * b.x, 0.75f * a.y + 0.25f * b.y});
        smoother.push_back(
            {0.25f * a.x + 0.75f * b.x, 0.25f * a.y + 0.75f * b.y});
      }
      path = smoother;
    }

    const float seed = (float)paths.size();
    for (size_t i = 0; i < path.size(); i++) {
      path[i].x += std::sin(i * 0.21f + seed * 1.7f) * wobble;
      path[i].y += std::cos(i * 0.17f + seed * 2.3f) * wobble;
    }
    paths.push_back(path);
  }
  return paths;
}

void DrawPath(const std::vector<Vector2> &path, size_t count, bool closed,
              float thick, Color color) {
  if (count > path.size())
    count = path.size();
  const size_t segments = closed ? count : (count > 0 ? count - 1 : 0);
  for (size_t i = 0; i < segments; i++) {
    DrawLineEx(path[i], path[(i + 1) % path.size()], thick, color);
    DrawCircleV(path[i], thick / 2, color); // round joints
  }
}

int CheckPages() {
  int problems = 0;
  for (size_t i = 0; i < Pages().size(); i++) {
    KolamGrid grid;
    std::string error;
    printf("Page %zu \"%s\": ", i + 1, Pages()[i].title.c_str());
    if (!grid.Load(Pages()[i], &error)) {
      printf("BROKEN LAYOUT - %s\n", error.c_str());
      problems++;
      continue;
    }
    const int startLoops = (int)grid.Loops().size();
    const int solutions = grid.CountSolutions(false);
    printf("%dx%d dots, starts as %d lines, %d solution(s)", grid.width,
           grid.height, startLoops, solutions);
    if (startLoops == 1)
      printf("  <-- ALREADY SOLVED");
    if (solutions == 0)
      printf("  <-- CAN'T BE SOLVED");
    if (startLoops == 1 || solutions == 0)
      problems++;
    printf("\n");
  }
  // The story: every chapter has steps and its own id, and no step would
  // show nothing (a Talk with no lines needs a place to show)
  for (size_t c = 0; c < Chapters().size(); c++) {
    const Chapter &chapter = Chapters()[c];
    const char *name = chapter.title.c_str();
    if (chapter.steps.empty()) {
      printf("Chapter \"%s\": has no steps\n", name);
      problems++;
    }
    for (size_t other = 0; other < c; other++) {
      if (Chapters()[other].id == chapter.id) {
        printf("Chapter \"%s\": id \"%s\" is used twice\n", name,
               chapter.id.c_str());
        problems++;
      }
    }
    for (const Step &step : chapter.steps) {
      if (step.type == StepType::Talk && step.lines.empty() &&
          step.place.empty()) {
        printf("Chapter \"%s\", \"%s\": a Talk with no lines or place\n", name,
               step.label.c_str());
        problems++;
      }
      if (step.type == StepType::List &&
          (step.lines.size() < 2 || step.page < 0 ||
           step.page >= (int)Pages().size())) {
        printf("Chapter \"%s\", \"%s\": a list needs a page, Amma's words "
               "and items\n",
               name, step.label.c_str());
        problems++;
      }
      if (step.type == StepType::Explore && step.hotspots.empty()) {
        printf("Chapter \"%s\", \"%s\": an Explore with nothing to click\n",
               name, step.label.c_str());
        problems++;
      }
    }
  }
  printf(problems ? "%d problem(s)\n" : "All pages OK\n", problems);
  return problems;
}
