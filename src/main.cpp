#include "game.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

static sling::Game game;
#if defined(__EMSCRIPTEN__) && defined(SLINGSHIFT_TEST_HOOKS)
extern "C" {
EMSCRIPTEN_KEEPALIVE const char* sling_qa_state() {
    static char data[1000];
    const auto aim=game.currentAim(); const auto p=game.currentPointer();
    const auto start=game.launchPixel(),end=game.solutionPixel();
    const auto solution=sling::levels()[game.levelIndex].solution;
    std::snprintf(data,sizeof(data),"{\"screen\":%d,\"mode\":%d,\"level\":%d,\"time\":%.9f,\"outcome\":%d,\"corrected\":%d,\"paused\":%d,\"dragging\":%d,\"x\":%.5f,\"y\":%.5f,\"aimX\":%.5f,\"aimY\":%.5f,\"pointerX\":%.3f,\"pointerY\":%.3f,\"stars\":%d,\"unlocked\":%d,\"startX\":%.3f,\"startY\":%.3f,\"pullX\":%.3f,\"pullY\":%.3f,\"pixelScale\":%.6f,\"solutionX\":%.6f,\"solutionY\":%.6f}",
        static_cast<int>(game.screen),static_cast<int>(game.mode),game.levelIndex,game.ship.time,static_cast<int>(game.ship.outcome),game.ship.corrected,game.isPaused(),game.isDragging(),game.ship.position.x,game.ship.position.y,aim.x,aim.y,p.x,p.y,game.savedProgress().totalStars(),game.savedProgress().unlocked(),start.x,start.y,end.x,end.y,game.pixelScale(),solution.x,solution.y);
    return data;
}
EMSCRIPTEN_KEEPALIVE void sling_qa_level(int index) { game.loadLevel(index); }
EMSCRIPTEN_KEEPALIVE void sling_qa_route() { game.startVerifiedRoute(); }
}
EM_JS(void, registerQA, (), {
    window.SlingQA={state:function(){return JSON.parse(UTF8ToString(_sling_qa_state()));},level:function(index){_sling_qa_level(index);},route:function(){_sling_qa_route();}};
});
#endif
static void frame() {
    #ifdef __EMSCRIPTEN__
    double w=0,h=0;
    emscripten_get_element_css_size("#canvas",&w,&h);
    if(w>0 && h>0 && (GetScreenWidth()!=static_cast<int>(w) || GetScreenHeight()!=static_cast<int>(h)))
        SetWindowSize(static_cast<int>(w),static_cast<int>(h));
    #endif
    game.frame();
}
#ifdef __EMSCRIPTEN__
int main() {
#else
int main(int argc,char** argv) {
#endif
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(1280,800,"SLINGSHIFT");
    if(!IsWindowReady()) return 1;
    SetWindowMinSize(360,360);
    SetExitKey(KEY_NULL);
    #ifndef __EMSCRIPTEN__
    // Keep assets relative to the executable, including when launched from another directory.
    ChangeDirectory(GetApplicationDirectory());
    Image icon=LoadImage("assets/icon.png");
    if(icon.data) { SetWindowIcon(icon); UnloadImage(icon); }
    #endif
    game.init();
    #if defined(__EMSCRIPTEN__) && defined(SLINGSHIFT_TEST_HOOKS)
    registerQA();
    #endif
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(frame,0,1);
    #else
    bool smoke=false;
    bool verifyRoute=false;
    const char* capture=nullptr;
    for(int i=1;i<argc;++i) {
        if(std::strcmp(argv[i],"--smoke")==0) smoke=true;
        if(std::strcmp(argv[i],"--capture")==0 && i+1<argc) capture=argv[++i];
        if(std::strcmp(argv[i],"--level")==0 && i+1<argc) game.loadLevel(std::atoi(argv[++i])-1);
        if(std::strcmp(argv[i],"--route")==0) { game.startVerifiedRoute(); verifyRoute=true; }
    }
    SetTargetFPS(60);
    int count=0;
    while(!WindowShouldClose() && !game.wantsQuit()) {
        frame();
        if(capture && count==90) { Image image=LoadImageFromScreen(); ExportImage(image,capture); UnloadImage(image); }
        if(!smoke) ++count;
        if(smoke && ++count>=360) break;
    }
    game.close();
    CloseWindow();
    if(smoke && verifyRoute) {
        std::printf("Route smoke: outcome %d, arrival time %.6f s\n",static_cast<int>(game.ship.outcome),game.ship.time);
        return game.ship.outcome==sling::Outcome::Arrived?0:1;
    }
    #endif
    return 0;
}
