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
//   Memory(label, {...})                 a flashback dialogue, in full colour. Once
//                                        played, it is a memory Anu has back (its
//                                        label is its name): it brings colour back
//   Memory(...).At(place, ...).WithAmma()   ...at a place, with little Anu and
//                                        Amma there (on the front step)
//   Explore(label, night, {hotspots})    walk around the house and click things.
//                                        Each hotspot is:
//       {object, name, {lines}, memory, endsScene}
//       object     where it is (see ROOM_OBJECTS in explore_scene.cpp)
//       name       shown when the mouse is over it
//       memory     a memory Anu gets back, which brings colour back (or leave it out)
//       endsScene  true for the one object that moves the story on
//   Puzzle(n)                            page n of the kolam book (from Pages())
//   LookAt(n)                            page n, only looked at: it stays as
//                                        it is, for a later chapter
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
// After the layout, `true` makes it a drawing page: the player draws the line
// (holding and dragging) instead of putting up walls. On a drawing page each
// ? or * is a choice made while drawing, and the line must close round every
// dot. The other pages are wall pages.
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
       },
       true}, // a drawing page
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
       },
       true}, // a drawing page
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
       },
       true}, // a drawing page
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
       },
       true}, // a drawing page
      {"Vaasal - the doorway",
       "A small one for every day. Every house says good morning.",
       false,
       true,
       {
           "+.+.+.+",
           ".o?o?o.",
           "+.+.+.+",
           ".o?o?o.",
           "+.+.+.+",
           ".o?o?o.",
           "+.+.+.+",
       },
       true}, // a drawing page
  };
  return pages;
}

// A scene made in its own file (see PLAY_SCENE in game.h), e.g.
// Play("ring_toss", "Ring toss"). `lines`: words the scene may show.
// (Not `static`, so the compiler doesn't warn while no chapter uses it.)
Step Play(const std::string &name, const std::string &label,
          const std::vector<Line> &lines = {}) {
  Step step = {StepType::Play, label};
  step.play = name;
  step.lines = lines;
  return step;
}

// Remembered(Play(...)): a Play scene that is a flashback, in full colour.
// Once played, it is a memory Anu has back (its label is its name).
static Step Remembered(Step step) {
  step.memory = true;
  return step;
}

static Step Talk(const std::string &label, const std::vector<Line> &lines) {
  Step step = {StepType::Talk, label};
  step.lines = lines;
  return step;
}

// A Talk step that is a flashback: shown in full colour.
static Step Memory(const std::string &label, const std::vector<Line> &lines) {
  Step step = Talk(label, lines);
  step.memory = true;
  return step;
}

static Step Puzzle(int page) {
  Step step = {StepType::Puzzle,
               "Page " + std::to_string(page + 1) + ": " + Pages()[page].title};
  step.page = page;
  return step;
}

// A page of the kolam book that is only looked at, not solved. Clicking it
// says "Not yet."; it is finished in a later chapter.
static Step LookAt(int page) {
  Step step = Puzzle(page);
  step.lookOnly = true;
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

// The doorstep drawing: the finale of Chapter 8 "Five in the Morning"
static Step Doorstep() { return {StepType::Doorstep, "The doorstep"}; }

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

      // Chapter 2: shopping with Amma. One thing on the list can't be bought.
      {"the_list",
       "Chapter 2",
       "The List",
       "The market, ten days before Pongal",
       {
           // Played, not read: fold the bags, take the paper list (not
           // the phone); tap the stalls for the list's lines; measure the
           // jasmine; reach for the flowers; choose bangles slowly.
           Play("list_setting_out", "Setting out", {{"Amma", "On the paper."}}),
           Play("list_market", "The market"),
           Play("list_flowers", "The flower stall",
                {{"Flower seller", "You used to know exactly which flowers "
                                   "you wanted."}}),
           [] { // pin the orange flowers in little Anu's plaits
             Step s = Play("list_kanakambaram", "Kanakambaram",
                           {{"Anu", "More."},
                            {"Amma", "Any more and you'll fall over."}});
             s.memory = true;
             return s;
           }(),
           Play("list_bangles", "The bangle stall",
                {{"Amma", "You used to take so long to choose."}}),
           [] { // six bangles on a small wrist: shake them
             Step s = Play("list_rain", "Glass bangles",
                           {{"Anu", "Listen. They sound like rain."}});
             s.memory = true;
             return s;
           }(),
           // Unpack the bag: everything but the little flowers
           Play("list_home", "Coming home",
                {{"Anu", "We didn't get the flowers."},
                 {"Amma", "Not those flowers."}}),
           // The bangles, on the terrace at night. No words.
           Play("list_terrace", "The terrace, the bangles"),
       }},

      // Chapter 3: the old streets, and an old friend
      {"familiar_streets",
       "Chapter 3",
       "Familiar Streets",
       "The old streets, nine days before Pongal",
       {
           // No list today. Anu just walks the old street; what she passes
           // flickers back to how it was. Hold (or click) ahead to walk.
           Play("streets_walk", "No list today"),

           // Recess, under the tamarind tree: five stones with Meena
           Remembered(Play("five_stones", "Five stones",
                           {
                               {"Meena", "Four!"},
                               {"Anu", "It was a bad stone."},
                           })),

           // Meena now, a boy on her hip and a bag of vegetables
           Play("streets_meena", "Meena",
                {
                    {"Meena", "Do you still draw?"},
                    {"Anu", "Not really."},
                    {"Meena", "You drew on everything. My maths book still has "
                              "your elephant in it."},
                    {"Anu", "It was a good elephant.", 2.5f},
                    {"Meena", "It was a terrible elephant. Come for the "
                              "festival."},
                }),

           // Rain, and the street is a river: fold a boat, give it a face
           Remembered(Play("paper_boats", "Paper boats",
                           {
                               {"Meena", "Yours has a face on it."},
                               {"Anu", "So it can see where it's going."},
                           })),

           // Back home, the evening: the old trunks on the terrace
           Explore("Home, evening", true,
                   {
                       {"calendar", "The calendar", {{"", "Nine days."}}},
                       {"vessels",
                        "Brass and steel",
                        {{"", "ANU. Scratched in by someone about seven."}}},
                       {"temple",
                        "The temple tower",
                        {{"", "The first strings of lights."}}},
                       {"storage_boxes",
                        "Old trunks",
                        {{"", "Her old drawing book."},
                         {"", "An elephant. A terrible elephant."}},
                        "The drawing book",
                        true},
                   }),
       }},

      {"ammas_book",
       "Chapter 4",
       "Amma's Book",
       "Home, seven days before Pongal",
       {
           // A quiet afternoon. Work is done, and it went fine.
           Talk("After lunch",
                {
                    {"", "After lunch. Amma is asleep, and the fan ticks "
                         "round."},
                    {"", "Anu sends the last file, shuts the laptop, and sits "
                         "on the swing."},
                    {"", "On something hard. Amma's kolam book."},
                })
               .At("hall", DAY),

           // Amma's pages. Each one brings something back.
           Puzzle(0),
           Memory("Before the sun",
                  {
                      {"", "Anu is six. It's still dark, and Amma is putting "
                           "down the dots."},
                      {"Anu", "Why the dots first?"},
                      {"Amma", "So the line knows where to go."},
                  })
               .At("front_step", NIGHT) // before dawn
               .WithAmma(),

           Puzzle(1),
           Memory("Amma's hand",
                  {
                      {"", "Amma's hand closes around hers."},
                      {"Anu", "It's going crooked!"},
                      {"Amma", "Crooked is fine. Just don't lift."},
                  })
               .At("front_step", NIGHT)
               .WithAmma(),

           Puzzle(2),
           Memory("Rice flour",
                  {
                      {"", "Amma draws her side of the step. Anu copies it on "
                           "hers."},
                      {"Amma", "Who used up all the flour?"},
                      {"Anu", "It's for the ants."},
                  })
               .At("front_step", DAY)
               .WithAmma(),

           // The festival kolam from the end of Chapter 1, and what Amma
           // wrote beside it
           Puzzle(3),
           Memory("Festival eve",
                  {
                      {"", "Festival eve. Half asleep on Amma's lap, Anu "
                           "watches her write in the book."},
                      {"Anu", "What are you writing?"},
                      {"Amma", "Nothing. Sleep."},
                  })
               .At("hall", NIGHT),
           Talk("In the margin",
                {
                    {"", "In the margin of the festival page, very small:"},
                    {"", "\"Anu put the pulli by herself today. All crooked. "
                         "Best one on the street.\""},
                })
               .At("hall", DAY),

           // Little Anu's pages, in crayon. "amma says fix it"...
           Puzzle(4),
           Talk("Amma is up",
                {
                    {"", "Amma is up. She looks over Anu's shoulder."},
                    {"Amma", "Finally."},
                })
               .At("hall", DAY),

           // ...and the one she'd finish "when i am big". Not yet: it is
           // drawn in Chapter 8, "Five in the Morning".
           LookAt(5),
           Talk("When I'm big",
                {
                    {"Amma", "And that one?"},
                    {"Anu", "When I'm big."},
                    {"", "Amma laughs, and heads for the kitchen."},
                    // hold to say it: asking is harder than joking
                    {"Anu", "Amma. Wake me up at five?", 2.5f},
                    {"", "Amma stops in the doorway."},
                    {"Amma", "Then sleep early."},
                })
               .At("hall", DAY),

           Talk("That night",
                {
                    {"", "Anu sets an alarm for 4:50 am."},
                })
               .At("hall", NIGHT),
       }},

      // Chapter 5: getting the house and the street ready
      {"preparations",
       "Chapter 5",
       "Preparations",
       "Home, four days before Pongal",
       {
           // Tie the thoranam over the door, a leaf at a time
           Play("prep_mango_leaves", "Mango leaves",
                {{"Amma", "Your Appa always tied these. Too high."},
                 {"Anu", "So no one could reach them."},
                 {"Amma", "So I couldn't reach them."}}),
           [] { // hand Appa the leaves up the ladder, checking each one
             Step s = Play("prep_ladder", "The ladder");
             s.memory = true;
             return s;
           }(),

           Explore(
               "Getting ready", false,
               {
                   {"old_kolam", "The doorstep", {{"", "Swept. Waiting."}}},
                   {"tulsi",
                    "The tulsi plant",
                    {{"", "Jasmine round the pot. Amma's idea."}}},
                   {"vessels",
                    "Brass and steel",
                    {{"", "Polished. Even the one that says ANU."}},
                    "Polishing day"},
                   {"flour_tin",
                    "Rice flour for the kolam",
                    {{"", "Two more tins. Just in case."}}},
                   {"temple",
                    "The temple tower",
                    {{"", "Lights on every tier."}}},
                   {"amma",
                    "Amma",
                    {
                        {"Amma", "The doorway needs a kolam."},
                        {"Anu", "Me?"},
                        {"Amma", "Do you see anyone else?"},
                    },
                    "",
                    true},
               }),

           // A small kolam for the doorway: the first one she draws
           Puzzle(6),
           // Amma comes out, looks, and steps carefully around it. No words.
           Play("prep_amma_looks", "Amma looks"),

           // Hold to turn up the fire until the milk boils over
           Play("prep_pongal", "The pongal pot",
                {{"Amma", "Watch it. Watch it..."},
                 {"Amma", "Pongalo pongal!"},
                 {"Anu", "Pongalo pongal.", 2.5f}}),
       }},

      // Chapter 6: the festival eve, with Meena
      {"festival_night",
       "Chapter 6",
       "Festival Night",
       "The temple street, festival eve",
       {
           // The terrace at night: light the last lamp, and the tower lights
           // up. The drums start.
           Play("festival_tower", "The tower"),

           // The festival street with Meena: ring toss. Miss, miss...
           Play("festival_ring_toss", "The festival street",
                {
                    {"Meena", "You were the worst at this."},
                    {"Meena", "...Fine. Lucky."},
                }),

           // Paper windmills, all spinning: one for Anu, one for Meena's boy
           Play("festival_windmills", "The windmill stall"),
           // Appa running with little Anu, so the windmill will spin
           Remembered(Play("festival_windmill_run", "The paper windmill",
                           {{"Anu", "Faster!"}})),

           // At the temple, lighting the lamps with Amma and the other women
           Play("festival_lamps", "Still the same", {{"Amma", "Still the same."}}),

           // Late. The kolam book, open at the BIG one. She falls asleep with
           // her hand on it.
           Play("festival_book", "The kolam book"),
       }},

      // Chapter 7: a dream. Little Anu and Anu finish the BIG one together.
      // Little Anu's crayon drawing, come alive (ch7_dream.cpp).
      {"little_girl",
       "Chapter 7",
       "The Little Girl",
       "Festival eve, late",
       {
           // Colour the drawing in; then she looks up from her notebook
           [] {
             Step s = Play("crayon_world", "The crayon world",
                           {
                               {"Little Anu", "You're late."},
                               {"Anu", "I know."},
                               {"Little Anu", "Are you big?"},
                               {"Anu", "I think so.", 2.5f},
                               {"Little Anu", "Then we'll do it together. You "
                                              "do the hard bits."},
                           });
             s.memory = true;
             return s;
           }(),

           // The BIG one, finished at last: together (page 6, in the
           // crayon world)
           [] {
             Step s = Play("big_one", "Page 6: the BIG one");
             s.page = 5;
             return s;
           }(),

           // Go over it once more, without lifting; she adds a flower
           [] {
             Step s = Play("together", "Together",
                           {
                               {"Little Anu", "It's not crooked."},
                               {"Anu", "It's a little crooked."},
                               {"Little Anu", "Crooked is fine. Just don't "
                                              "lift."},
                           });
             s.memory = true;
             return s;
           }(),

           // The hall, before dawn: the alarm, and Anu already awake
           Play("alarm_450", "4:50 am"),
       }},

      // Chapter 8: Pongal morning. Anu draws the doorstep kolam
      // (ch8_morning.cpp, and the doorstep drawing in scenes.cpp).
      {"five_in_the_morning",
       "Chapter 8",
       "Five in the Morning",
       "Pongal morning",
       {
           // Before dawn: Amma holds the rice flour out this time. Take it.
           Play("flour_at_door", "At the door"),

           // Free drawing on the doorstep, and its ending at sunrise
           Doorstep(),

           // The kolam on the step, and the pots boiling over down the street
           Play("pongal_street", "Pongal",
                {
                    {"Amma", "There's a flower. In the corner."},
                    {"Anu", "Hm."},
                    {"Amma", "Mine never had a flower."},
                    {"Amma", "Pongalo pongal!"},
                }),
       }},

      // Chapter 9: going back, carrying it (ch9_carry.cpp)
      {"what_we_carry",
       "Chapter 9",
       "What We Carry",
       "After the festival",
       {
           // Three days later, the kolam still on the step. Amma's tiffin and
           // a tin of rice flour, into the bag.
           Play("pack_bag", "The step",
                {
                    {"Anu", "Amma, I live in a flat."},
                    {"Amma", "Flats have doors."},
                }),

           // The same desk as the first night, now warm; a flower on a
           // sticky note
           Play("hyderabad_back", "Hyderabad, 11:47 pm",
                {
                    {"Rohit", "Welcome back! Did you bring sweets?"},
                    {"Anu", "In the fridge. Don't take the round ones."},
                }),

           // This time, Anu calls first
           Play("call_amma", "A call",
                {
                    {"Amma", "Kanna? Is everything okay?"},
                    {"Anu", "Everything's fine. I just called.", 2.5f},
                    {"Amma", "..."},
                    {"Amma", "Eat something, okay?"},
                }),
       }},
  };
  return chapters;
}
