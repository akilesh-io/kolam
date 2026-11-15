// story.cpp - all of the game's content. Writers and designers only need this
// file. After editing it, run:  ./run.sh --check-pages
//
// clang-format off
// Chapters() (at the bottom): the story, in order. Each step is one of:
//   Talk(label, {{speaker, text}, ...})  dialogue; an empty speaker is narration
//   {speaker, text, 2.5f}                a hard line to say: the words only come
//                                        out while the player holds the button
//   VoiceMessage(label, {...})           lines played as a voice message on the phone
//   Talk(...).At(place, NIGHT or DAY)    ...shown over a place in the house:
//                                        front_step, hall, kitchen, terrace
//   Memory(label, {...})                 a flashback dialogue, in full colour
//   Explore(label, night, {hotspots})    walk around the house and click things.
//                                        Each hotspot is:
//       {object, name, {lines}, memory, endsScene}
//       object     where it is (see ROOM_OBJECTS in explore_scene.cpp)
//       name       shown when the mouse is over it
//       memory     a memory Anu gets back, which brings colour back (or leave it out)
//       endsScene  true for the one object that moves the story on
//   Puzzle(n)                            page n of the kolam book (from Pages())
//   Busy()  Travel()  Kite()  Coffee()  Doorstep()   the minigames
//   AmmasList(page, {Amma's words, items...})        her notebook, then her list
//   Talk(label, {}).At(place, ...)       a quiet moment at a place, no words
// The save game stores the chapter's id and the step number, so add new
// steps at the end of a chapter rather than in the middle, or old saves will
// resume in the wrong place.
//
// Pages(): the kolam book. A layout is a grid of characters:
//   o  a dot                     +  a corner (ignored)
//   #  a wall in ink             .  a gap in ink        (the player can't change these)
//   ?  a gap the player can change into a wall
//   *  a wall the player can change into a gap
//   The characters around the edge are ignored: the border is always a wall.
// The kolam line runs diagonally between the dots, crossing straight through
// gaps and bouncing off walls. A page is solved when it becomes ONE line.
// Symmetric pages need an odd number of dots both ways (otherwise a symmetric
// kolam can never be one line).
// clang-format on
#include "game.h"

const std::vector<Page> &Pages() {
  static const std::vector<Page> pages = {
      {"Pulli - the dots",
       "First the dots. The line goes around them, never through them.",
       false,
       false,
       {
           "+.+.+.+",
           ".o#o.o.",
           "+.+.+?+",
           ".o#o#o.",
           "+.+.+.+",
           ".o.o.o.",
           "+.+.+.+",
       }},
      {"Sikku - one line",
       "A good kolam never lifts the hand. One line, from start to end.",
       false,
       false,
       {
           "+.+.+.+",
           ".o.o.o.",
           "+.+?+#+",
           ".o.o?o.",
           "+#+.+.+",
           ".o?o.o.",
           "+.+.+.+",
       }},
      {"Both sides the same",
       "Whatever you do in one corner, the other corners do too.",
       false,
       true,
       {
           "+.+.+.+",
           ".o.o.o.",
           "+?+.+?+",
           ".o?o?o.",
           "+?+.+?+",
           ".o.o.o.",
           "+.+.+.+",
       }},
      {"Festival morning",
       "For the festival, draw it big. The bigger the kolam, the bigger the "
       "welcome.",
       false,
       true,
       {
           "+.+.+.+.+.+",
           ".o?o.o.o?o.",
           "+.+.+.+.+.+",
           ".o?o#o#o?o.",
           "+.+?+?+?+.+",
           ".o.o?o?o.o.",
           "+.+?+?+?+.+",
           ".o?o#o#o?o.",
           "+.+.+.+.+.+",
           ".o?o.o.o?o.",
           "+.+.+.+.+.+",
       }},
      {"MY KOLAM",
       "by Anu, age 7. it has TOO MANY lines. amma says fix it",
       true,
       false,
       {
           "+.+.+.+.+",
           ".o.o.o#o.",
           "+#+?+.+?+",
           ".o?o.o.o.",
           "+.+.+#+#+",
           ".o.o?o.o.",
           "+#+.+?+.+",
           ".o?o#o.o.",
           "+.+.+.+.+",
       }},
      {"the BIG one",
       "i will finish this one when i am big",
       true,
       true,
       {
           "+.+.+.+.+.+.+.+",
           ".o.o#o?o?o#o.o.",
           "+.+?+.+.+.+?+.+",
           ".o.o?o.o.o?o.o.",
           "+#+.+#+?+#+.+#+",
           ".o.o?o?o?o?o.o.",
           "+#+.+#+?+#+.+#+",
           ".o.o?o.o.o?o.o.",
           "+.+?+.+.+.+?+.+",
           ".o.o#o?o?o#o.o.",
           "+.+.+.+.+.+.+.+",
       }},
  };
  return pages;
}

static Step Talk(const std::string &label, const std::vector<Line> &lines) {
  Step step = {StepType::Talk, label};
  step.lines = lines;
  return step;
}

// Memory() and Puzzle() aren't used in Chapter 1; they wait for later
// chapters (and aren't `static`, so the compiler doesn't warn about that).

// A Talk step that is a flashback: shown in full colour.
Step Memory(const std::string &label, const std::vector<Line> &lines) {
  Step step = Talk(label, lines);
  step.memory = true;
  return step;
}

Step Puzzle(int page) {
  Step step = {StepType::Puzzle,
               "Page " + std::to_string(page + 1) + ": " + Pages()[page].title};
  step.page = page;
  return step;
}

static Step Explore(const std::string &label, bool night,
                    const std::vector<Hotspot> &hotspots) {
  Step step = {StepType::Explore, label};
  step.night = night;
  step.hotspots = hotspots;
  return step;
}

static Step VoiceMessage(const std::string &label,
                         const std::vector<Line> &lines) {
  Step step = Talk(label, lines);
  step.type = StepType::VoiceMessage;
  return step;
}

static Step Busy() { return {StepType::Busy, "Hyderabad, 11:47 pm"}; }
static Step Travel() { return {StepType::Travel, "The night bus"}; }
static Step Kite() { return {StepType::Kite, "Appa's kite"}; }
static Step Coffee() { return {StepType::Coffee, "Coffee for Amma"}; }

// The doorstep drawing: the finale of Chapter 8 "Five in the Morning", not
// written yet. (Not `static`, so the compiler doesn't warn that it's unused.)
Step Doorstep() { return {StepType::Doorstep, "The doorstep"}; }

// Amma's kolam notebook, open at `page`'s pattern, then the shopping list she
// hands Anu. The first line is what Amma says; the rest are the list's items.
// The last item gets a small drawing of flowers.
static Step AmmasList(int page, const std::vector<Line> &lines) {
  Step step = {StepType::List, "Amma's list"};
  step.page = page;
  step.lines = lines;
  return step;
}

// Work calls while Anu is at home. Answering plays the call; "Not now" lets
// it go. Neither changes anything: Anu likes her work.
const std::vector<std::vector<Line>> &WorkCalls() {
  static const std::vector<std::vector<Line>> calls = {
      {{"Rohit", "Hey, sorry. Can you check that file when you get a chance?"},
       {"Anu", "I'll have a look later."}},
      {{"Rohit", "The client loved the new screens, by the way."},
       {"Anu", "Really? The blue ones?"},
       {"Rohit", "The blue ones."}},
      {{"Rohit", "Where did you keep the icon files?"},
       {"Anu", "Shared folder. 'Final final.'"}},
      {{"Rohit", "Enjoy the festival. Bring sweets."}, {"Anu", "Maybe."}},
  };
  return calls;
}

// THE STORY, told in order, one chapter at a time. Each chapter opens with a
// title card:   {id, heading, title, where and when, {steps}}
// `id` is what the save game remembers. Chapter 1 is the demo: Hyderabad,
// the bus home, the village. Later chapters are planned in README.md.
//
// Keep the words few. The pictures and the animation carry the feeling;
// a line is only here when nothing else can say it.
const std::vector<Chapter> &Chapters() {
  static const std::vector<Chapter> chapters = {
      {"home",
       "Chapter 1",
       "Home",
       "Hyderabad, eleven days before Pongal",
       {
           // Her life: work she likes, and Amma's calls arriving in the middle
           Busy(),

           // Played on her phone. Then the reply she always sends... deleted.
           VoiceMessage("Amma's voice message",
                        {
                            {"Amma", "Kanna. It's Amma."},
                            {"Amma", "Pongal is on the fourteenth."},
                            {"Amma", "..."},
                            {"Amma", "The temple festival preparations have "
                                     "started."},
                            {"Amma", "The house is very quiet."},
                            {"Amma", "Eat something, okay?"},
                        }),

           // The night bus home. No words.
           Travel(),

           // Home: a village on the Bhavani, between Mettupalayam and
           // Sathyamangalam
           Explore(
               "Home", false,
               {
                   {"old_kolam",
                    "Yesterday's kolam",
                    {{"", "The lines are shaky."}}},
                   {"tulsi",
                    "The tulsi plant",
                    {{"", "Watered this morning."}}},
                   {"bicycle",
                    "Her red bicycle",
                    {{"", "The bell still works."}},
                    "The red bicycle"},
                   {"family_photo",
                    "The family photo",
                    {{"", "Appa, Amma, and a very small Anu. Everyone is "
                          "squinting."}},
                    "The beach photo"},
                   {"swing",
                    "The swing",
                    {{"", "It creaks the same way."}},
                    "The swing"},
                   {"calendar",
                    "The calendar",
                    {{"", "The fourteenth is circled. Next to it, in Amma's "
                          "hand: \"Anu?\""}}},
                   // Clicking the doorframe opens it up close: the player
                   // traces the pencil marks upwards, then this line plays
                   {"height_marks",
                    "Pencil marks on the doorframe",
                    {{"", "Nothing above fifteen."}},
                    "The doorframe"},
                   {"amma",
                    "Amma",
                    {
                        {"Amma", "Anu..."},
                        // hold to say it: it isn't easy, after so long away
                        {"Anu", "Hi, Amma.", 2.5f},
                        {"Amma", "You've become so thin."},
                        {"Amma", "Sit. I'll make pongal."},
                    },
                    "Amma's kitchen"},
                   {"grinding_stone",
                    "The grinding stone",
                    {{"", "Small hands used to push this round and round."}},
                    "Songs at the grinding stone"},
                   {"flour_tin",
                    "Rice flour for the kolam",
                    {{"", "Full to the top."}}},
                   {"coffee_filter",
                    "The coffee filter",
                    {{"", "The same dent on the lid."}}},
                   {"vessels",
                    "Brass and steel",
                    {{"", "Every vessel has a name scratched on it. One says "
                          "ANU."}}},
                   {"storage_boxes",
                    "Old trunks",
                    {{"", "Crayons. A school bag. Report cards, all of "
                          "them."}}},
                   {"temple",
                    "Bannari Amman's tower, far off",
                    {{"", "The evening bells. A loudspeaker crackles: the "
                          "festival, testing, one, two."}},
                    "Temple bells"},
                   {"kite_reel",
                    "Appa's kite reel",
                    {{"", "The thread is still wound tight."}},
                    "",
                    true},
               }),

           // A memory: Pongal, on this terrace, with Appa
           Kite(),

           // Back on the terrace at night, the reel in her hand. No words.
           Talk("The terrace, after", {}).At("terrace", NIGHT),

           // Early next morning, Anu makes Amma's coffee
           Coffee(),

           Talk("Five in the morning",
                {
                    {"", "Amma is at the door with the rice flour."},
                    {"", "She starts to bend. Her knees stop her."},
                    {"", "Anu puts the coffee in her hands, and takes the "
                         "flour."},
                    {"Amma", "What?"},
                    {"", "Anu nods at the doorstep."},
                })
               .At("front_step", NIGHT),

           // Amma's notebook, open at the festival kolam (page 4). Not yet.
           // Then the list, and "Tomorrow."
           AmmasList(3,
                     {
                         {"Amma", "Come tomorrow. We have a lot to buy."},
                         {"", "Mallipoo, 2 muzham"},
                         {"", "Banana leaves"},
                         {"", "Turmeric, kumkum"},
                         {"", "Camphor, cotton wicks"},
                         {"", "Lamp oil"},
                         {"", "Coconuts, 4"},
                         {"", "Flowers"},
                     }),
       }},
  };
  return chapters;
}
