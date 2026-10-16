#pragma once
#include "raylib.h"
#include <array>

namespace sling {
enum class Cue { Tap, Launch, Burn, Impact, Arrival };
class Audio {
public:
    void init();
    void update(float volume,bool ambience);
    void play(Cue cue);
    void close();
    bool available() const { return ready; }
private:
    bool ready{};
    std::array<Sound,5> sounds{};
    Sound bed{};
};
}
