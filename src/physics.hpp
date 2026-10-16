#pragma once

#include <cmath>
#include <span>
#include <vector>

namespace sling {

struct Vec {
    double x{}, y{};
    constexpr Vec operator+(Vec b) const { return {x+b.x, y+b.y}; }
    constexpr Vec operator-(Vec b) const { return {x-b.x, y-b.y}; }
    constexpr Vec operator*(double s) const { return {x*s, y*s}; }
    constexpr Vec operator/(double s) const { return {x/s, y/s}; }
    Vec& operator+=(Vec b) { x+=b.x; y+=b.y; return *this; }
};
constexpr double dot(Vec a, Vec b) { return a.x*b.x+a.y*b.y; }
inline double length(Vec v) { return std::sqrt(dot(v,v)); }
inline bool finite(Vec v) { return std::isfinite(v.x) && std::isfinite(v.y); }
inline Vec limited(Vec v, double maximum) {
    const double n=length(v);
    return n>maximum ? v*(maximum/n) : v;
}

enum class BodyKind { Planet, Star, Rock };
struct Body {
    Vec position;
    double radius;
    double mu; // Gravitational parameter, in game-distance^3 / second^2.
    int palette{};
    BodyKind kind{BodyKind::Planet};
};

struct Level {
    const char* name;
    const char* region;
    const char* brief;
    const char* hint;
    Vec launch;
    Vec gate;
    double gateRadius{25};
    double par{7};
    std::vector<Body> bodies;
    Vec solution; // A reproducible route, checked by the level verifier.
};

inline constexpr double stepSeconds=1.0/120.0;
inline constexpr double shipRadius=5.0;
inline constexpr double softening=12.0;
inline constexpr double maxLaunchSpeed=430.0;
inline constexpr double maxCorrection=82.0;
inline constexpr double timeLimit=22.0;
inline constexpr double worldWidth=1200.0, worldHeight=540.0;

enum class Outcome { Flying, Arrived, Impact, Escaped, Expired, Invalid };
struct Flight {
    Vec position, velocity;
    double time{};
    Outcome outcome{Outcome::Flying};
    int hitBody{-1};
    bool corrected{};
};
struct Trace {
    std::vector<Vec> points;
    Flight end;
};

Vec acceleration(Vec position, std::span<const Body> bodies);
double potential(Vec position, std::span<const Body> bodies);
// Returns the first intersection on [a,b], or infinity. Includes tangencies.
double segmentCircle(Vec a, Vec b, Vec centre, double radius);
void advance(Flight& ship, const Level& level, double dt=stepSeconds);
bool correct(Flight& ship, Vec impulse);
Trace predict(Flight ship, const Level& level, int stride=10);
int starsFor(const Flight& ship, const Level& level);
const std::vector<Level>& levels();

} // namespace sling
