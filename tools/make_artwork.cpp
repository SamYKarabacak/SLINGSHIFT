#include "art.hpp"
#include <filesystem>
#include <iostream>

// The promotional art is rendered from the same procedural assets as the game.
int main(int argc,char** argv) {
    const std::filesystem::path output=argc>1?argv[1]:"docs";
    std::filesystem::create_directories(output);
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN|FLAG_MSAA_4X_HINT);
    InitWindow(1260,1000,"SLINGSHIFT / artwork");
    if(!IsWindowReady()) return 1;
    sling::Art art; art.load();
    for(int frame=0;frame<3;++frame) {
        BeginDrawing();
        art.background(1260,1000,0,false);
        const Vector2 centre{850,620};
        for(int i=0;i<220;++i) {
            const float a=i/220.f*6.283185f,b=(i+1)/220.f*6.283185f;
            auto orbit=[&](float angle){float x=std::cos(angle)*470,y=std::sin(angle)*170;return Vector2{centre.x+x*.92f-y*.38f,centre.y+x*.38f+y*.92f};};
            DrawLineEx(orbit(a),orbit(b),1.7f,sling::alpha(sling::mint,.2f));
        }
        art.planet(centre,221,0,sling::BodyKind::Planet,0,true);
        art.planet({447,703},55,2,sling::BodyKind::Planet,0,true);
        for(int i=0;i<95;++i) {
            const float t=i/94.f;
            const Vector2 p{195+850*t,653-390*std::sin(t*1.5707963f)};
            DrawCircleV(p,2,sling::alpha(sling::mint,.28f+t*.4f));
        }
        art.ship({240,591},{1,-.65},19,1,0);
        art.gate({1045,263},29,0,true);
        art.text("AN ORBITAL PUZZLE GAME",81,81,18,sling::mint,true);
        art.text("SLINGSHIFT",72,129,132,sling::paper);
        art.text("FIND YOUR LINE.",81,290,30,sling::muted,true);
        DrawLineEx({81,885},{1179,885},1,sling::alpha(sling::muted,.22f));
        art.text("TWELVE SIGNALS. ONE WAY HOME.",81,918,19,sling::paper,true);
        EndDrawing();
    }
    Image cover=LoadImageFromScreen();
    const bool coverSaved=ExportImage(cover,(output/"cover.png").string().c_str());
    ImageResize(&cover,630,500); ExportImage(cover,(output/"cover-small.png").string().c_str());
    UnloadImage(cover);
    RenderTexture2D icon=LoadRenderTexture(256,256);
    BeginTextureMode(icon); ClearBackground(sling::ink);
    art.planet({164,167},105,0,sling::BodyKind::Planet,0,true);
    art.ship({58,89},{1,-.55},23,.8f,0);
    art.gate({192,45},14,0,true);
    EndTextureMode();
    Image iconImage=LoadImageFromTexture(icon.texture); ImageFlipVertical(&iconImage);
    std::filesystem::create_directories("assets");
    const bool iconSaved=ExportImage(iconImage,"assets/icon.png");
    UnloadImage(iconImage); UnloadRenderTexture(icon);
    art.unload(); CloseWindow();
    std::cout<<"Cover and icon: "<<(coverSaved && iconSaved?"rendered":"failed")<<'\n';
    return coverSaved && iconSaved?0:1;
}
