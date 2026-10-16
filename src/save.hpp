#pragma once
#include <array>
#include <string>

namespace sling {
struct Progress {
    std::array<int,12> stars{};
    std::array<double,12> best{};
    float volume{.65f};
    bool ambience{true}, motion{true}, glow{true}, tutorialDone{};
    int unlocked() const;
    int totalStars() const;
    std::string encode() const;
    bool decode(const std::string& text);
    void load();
    bool save() const;
};
}
