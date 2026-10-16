#pragma once
#include "art.hpp"
#include "audio.hpp"
#include "save.hpp"
#include <deque>

namespace sling {
enum class Screen { Home, Chart, Play, Settings, Credits, Complete };
enum class Mode { Aiming, Flying, Won, Lost };
class Game {
public:
    void init();
    void frame();
    void close();
    void loadLevel(int index);
    void startVerifiedRoute() { screen=Screen::Play; aim=levels()[levelIndex].solution; launch(); }
    const Progress& savedProgress() const { return progress; }
    bool isPaused() const { return paused; }
    bool isDragging() const { return dragging; }
    Vec currentAim() const { return aim; }
    Vector2 currentPointer() const { return pointer; }
    Vector2 launchPixel() const { auto p=map(levels()[levelIndex].launch); return {p.x*uiScale,p.y*uiScale}; }
    Vector2 solutionPixel() const { const auto& l=levels()[levelIndex]; auto p=map(l.launch-l.solution/(portrait?3.0:2.15)); return {p.x*uiScale,p.y*uiScale}; }
    float pixelScale() const { return boardScale*uiScale; }
    int levelIndex{};
    Screen screen{Screen::Home};
    Mode mode{Mode::Aiming};
    Flight ship{};
private:
    Art art;
    Audio audio;
    Progress progress;
    Screen settingsReturn{Screen::Home};
    Screen previousScreen{Screen::Home};
    Vec aim{285,-48}, dragAnchor{}, correction{},aimBeforeDrag{};
    std::deque<Vec> trail;
    Trace preview, hintRoute;
    struct Particle { Vec position,velocity; float life,maxLife; Color colour; };
    std::vector<Particle> particles;
    float width{1280},height{800},uiScale{1},boardScale{1},clock{},resultTime{},accumulator{},shake{},flash{},transition{};
    Vector2 boardOrigin{},pointer{};
    bool portrait{},compact{},dragging{},paused{},showHint{},showField{},saveFailed{},pointerPressed{},pointerReleased{},pointerDown{};
    bool modalLayer{},keyboardNavigation{},tutorial{},quitRequested{},fineAiming{};
    int focus{},uiCount{},previousUiCount{},attempts{},tutorialStep{};
    bool keyboardActivate{};
    double lastPreview{-1};
    void layout();
    Vector2 map(Vec p) const;
    Vec unmap(Vector2 p) const;
    void launch();
    void finish();
    void update(float dt);
    void drawPlay();
    void drawHome();
    void drawChart();
    void drawSettings();
    void drawCredits();
    void drawComplete();
    void drawResult();
    void drawPause();
    void drawTrajectory(const Trace& trace,Color colour,bool ghost=false);
    void burst(Vec at,Color colour,int count);
    bool button(const std::string& label,Rectangle rect,bool primary=false,bool enabled=true);
    void card(Rectangle rect,Color border=alpha(muted,.17f));
    void store();
public:
    bool wantsQuit() const { return quitRequested; }
};
}
