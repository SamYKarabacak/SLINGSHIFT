#include "physics.hpp"

namespace sling {
const std::vector<Level>& levels() {
    static const std::vector<Level> chart{
        {"First light", "01 / THE INNER REACH", "One ship. One gate. Let gravity do the work.",
         "Pull back from your ship, then release. A small upward angle is enough.",
         {130,310},{1060,280},28,6.0, {{{620,400},58,130000,0}}, {289.626785, -12.940028}},
        {"Blue bend", "02 / THE INNER REACH", "The shortest line is rarely the best route.",
         "Pass beneath the blue planet. Its pull will bring you back upward.",
         {120,320},{1070,265},26,4.1, {{{610,245},66,1400000,1}}, {289.125491, 23.367255}},
        {"Thread the needle", "03 / THE INNER REACH", "Two worlds. A narrow invitation between them.",
         "Keep your launch nearly level. The two gravity wells balance in the middle.",
         {110,270},{1090,270},25,4.1, {{{560,145},61,210000,2},{{640,405},66,240000,0}}, {289.652203, 0.522506}},
        {"Afterglow", "04 / THE AMBER BELT", "Skim a star without becoming part of it.",
         "Dip beneath the star. Its gravity will lift your path toward the gate.",
         {120,330},{1060,310},25,3.8, {{{620,340},72,3500000,3,BodyKind::Star}}, {276.799833, 80.604334}},
        {"Split current", "05 / THE AMBER BELT", "Choose the quiet passage through competing tides.",
         "Aim between the green planet and the star, slightly above the gate.",
         {100,330},{1080,230},24,4.0, {{{430,195},56,900000,0},{{790,370},69,1500000,3,BodyKind::Star}}, {288.788693, -18.509306}},
        {"Stone garden", "06 / THE AMBER BELT", "The belt has no gravity. The planet does.",
         "Curve below the small rocks. The large blue world will lift you toward the gate.",
         {100,320},{1080,250},24,4.0, {{{690,180},64,2200000,1},{{420,295},26,0,4,BodyKind::Rock},{{855,390},31,0,4,BodyKind::Rock}}, {289.978435, 13.995088}},
        {"Low orbit", "07 / THE OUTER REACH", "A close pass turns a little speed into a lot of distance.",
         "Sweep above the violet world. Let its gravity carry you back toward the far gate.",
         {150,270},{1050,270},23,4.2, {{{600,270},95,6000000,2},{{940,130},34,0,4,BodyKind::Rock}}, {266.925743, -115.286901}},
        {"Twin suns", "08 / THE OUTER REACH", "There is a safe line through the heat.",
         "Take the middle channel. Let the two suns pull against one another.",
         {95,290},{1100,235},23,4.0, {{{440,115},65,1700000,3,BodyKind::Star},{{760,425},71,1900000,3,BodyKind::Star}}, {290.523931, 0.001538}},
        {"Dogleg", "09 / THE OUTER REACH", "A late turn, written in gravity.",
         "A shallow climb passes the rock. The far planet turns the route toward the gate.",
         {100,370},{1080,260},23,5.6, {{{470,350},43,0,4,BodyKind::Rock},{{800,195},74,2600000,0}}, {169.085270, -76.885394}},
        {"Crosswind", "10 / THE DEEP FIELD", "Three wells. One continuous thread.",
         "Start almost level. Your path should weave between all three worlds.",
         {95,280},{1110,275},22,3.9, {{{360,135},52,1200000,2},{{620,405},67,1800000,1},{{870,145},52,1200000,0}}, {291.535021, 8.689763}},
        {"Cinder run", "11 / THE DEEP FIELD", "Leave a margin. Space is less empty out here.",
         "Pass above the first rock and below the star. Mind the blue world's pull.",
         {90,310},{1110,245},22,3.9, {{{350,365},33,0,4,BodyKind::Rock},{{585,180},73,1900000,3,BodyKind::Star},{{875,405},59,1600000,1},{{980,155},25,0,4,BodyKind::Rock}}, {289.367538, 7.744291}},
        {"Slingshift", "12 / THE DEEP FIELD", "You know the language now. Write your own way home.",
         "Aim through the central seam. Each planet hands you to the next.",
         {90,280},{1110,255},21,3.8, {{{340,150},65,1700000,0},{{570,400},73,2400000,2},{{830,165},67,2000000,1},{{1000,420},29,0,4,BodyKind::Rock}}, {289.598965, 16.417182}}
    };
    return chart;
}
} // namespace sling
