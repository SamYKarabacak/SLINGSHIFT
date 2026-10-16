#pragma once
#include "physics.hpp"
#include "raylib.h"
#include <array>
#include <string>
#include <vector>

namespace sling {
inline constexpr Color ink{7,11,23,255}, panel{15,24,40,255};
inline constexpr Color paper{233,241,246,255}, muted{143,165,184,255};
inline constexpr Color mint{111,240,205,255}, gold{255,197,114,255};
inline constexpr Color violet{174,148,255,255}, danger{255,131,136,255};
struct Palette { Color dark, light, glow; };
Palette palette(int index);
Color alpha(Color c, float a);
Vector2 point(Vec p);

class Art {
public:
    void load();
    void unload();
    void background(float width, float height, float time, bool motion) const;
    void planet(Vector2 centre, float radius, int type, BodyKind kind, float time, bool effects) const;
    void gate(Vector2 centre, float radius, float time, bool effects) const;
    void ship(Vector2 centre, Vec velocity, float size, float thrust, float time) const;
    void star(Vector2 centre, float radius, Color colour) const;
    void text(const std::string& value, float x, float y, float size, Color colour=paper, bool mono=false) const;
    float textWidth(const std::string& value, float size, bool mono=false) const;
    void centred(const std::string& value, float x, float y, float size, Color colour=paper, bool mono=false) const;
    void wrapped(const std::string& value, Rectangle area, float size, Color colour=muted) const;
private:
    struct Star { float x,y, radius,phase; };
    Font interface{}, telemetry{};
    std::array<Texture2D,5> planets{};
    std::vector<Star> stars;
};
} // namespace sling
