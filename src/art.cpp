#include "art.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace sling {
Color alpha(Color c,float a) { c.a=static_cast<unsigned char>(std::clamp(a,0.f,1.f)*255); return c; }
Vector2 point(Vec p) { return {static_cast<float>(p.x),static_cast<float>(p.y)}; }
Palette palette(int index) {
    static constexpr std::array<Palette,5> colours{{
        {{19,67,72,255},{105,213,170,255},{83,230,183,255}},
        {{20,43,92,255},{103,178,232,255},{94,174,255,255}},
        {{49,30,87,255},{172,139,216,255},{174,141,255,255}},
        {{171,53,24,255},{255,224,146,255},{255,170,67,255}},
        {{41,47,61,255},{126,140,153,255},{131,147,162,255}}
    }};
    return colours[std::clamp(index,0,4)];
}
static Color mix(Color a,Color b,float t) {
    t=std::clamp(t,0.f,1.f);
    return {static_cast<unsigned char>(a.r+(b.r-a.r)*t),static_cast<unsigned char>(a.g+(b.g-a.g)*t),static_cast<unsigned char>(a.b+(b.b-a.b)*t),255};
}
static float noise3(float x,float y,float z,int seed) {
    const int ix=static_cast<int>(std::floor(x)),iy=static_cast<int>(std::floor(y)),iz=static_cast<int>(std::floor(z));
    auto smooth=[](float t){return t*t*(3-2*t);};
    const float u=smooth(x-ix),v=smooth(y-iy),w=smooth(z-iz);
    auto value=[&](int a,int b,int c) {
        unsigned int h=static_cast<unsigned int>(a)*374761393u+static_cast<unsigned int>(b)*668265263u+static_cast<unsigned int>(c)*2147483647u+static_cast<unsigned int>(seed)*1274126177u;
        h=(h^(h>>13))*1274126177u; h^=h>>16;
        return (h>>8)/16777216.f;
    };
    auto lerp=[](float a,float b,float t){return a+(b-a)*t;};
    const float a=lerp(value(ix,iy,iz),value(ix+1,iy,iz),u),b=lerp(value(ix,iy+1,iz),value(ix+1,iy+1,iz),u);
    const float c=lerp(value(ix,iy,iz+1),value(ix+1,iy,iz+1),u),d=lerp(value(ix,iy+1,iz+1),value(ix+1,iy+1,iz+1),u);
    return lerp(lerp(a,b,v),lerp(c,d,v),w);
}
static float terrain(float x,float y,float z,int seed) {
    return noise3(x,y,z,seed)*.57f+noise3(x*2,y*2,z*2,seed+1)*.28f+noise3(x*4,y*4,z*4,seed+2)*.15f;
}
void Art::load() {
    interface=LoadFontEx("assets/Interface.ttf",96,nullptr,0);
    telemetry=LoadFontEx("assets/Telemetry.ttf",40,nullptr,0);
    SetTextureFilter(interface.texture,TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(telemetry.texture,TEXTURE_FILTER_BILINEAR);
    for(int type=0;type<5;++type) {
        constexpr int size=512;
        Image im=GenImageColor(size,size,BLANK);
        auto* pixels=static_cast<Color*>(im.data);
        const auto p=palette(type);
        for(int y=0;y<size;++y) for(int x=0;x<size;++x) {
            const float nx=(x+0.5f-size/2)/(size/2.f),ny=(y+0.5f-size/2)/(size/2.f);
            const float rr=nx*nx+ny*ny;
            if(rr>1) continue;
            const float z=std::sqrt(1-rr);
            const float noise=terrain(nx*3.8f+7,ny*3.8f+11,z*3.8f+3,type+71);
            const float bands=std::sin(ny*28+noise*7);
            float surface=.43f+noise*.35f+bands*.13f;
            if(type==0) surface=.18f+std::clamp((noise-.41f)*3.5f,0.f,.72f);
            if(type==3) surface=.65f+noise*.32f;
            if(type==4) surface=.2f+noise*.65f;
            Color c=mix(p.dark,p.light,surface);
            const float light=type==3 ? .94f : .12f+.88f*std::max(0.f,-nx*.52f-ny*.5f+z*.69f);
            const float rim=std::pow(1-z,4)*.18f;
            c.r=static_cast<unsigned char>(std::min(255.f,c.r*light+p.glow.r*rim));
            c.g=static_cast<unsigned char>(std::min(255.f,c.g*light+p.glow.g*rim));
            c.b=static_cast<unsigned char>(std::min(255.f,c.b*light+p.glow.b*rim));
            c.a=static_cast<unsigned char>(std::clamp((1-std::sqrt(rr))*size/2,0.f,1.f)*255);
            pixels[y*size+x]=c;
        }
        planets[type]=LoadTextureFromImage(im);
        UnloadImage(im);
        SetTextureFilter(planets[type],TEXTURE_FILTER_BILINEAR);
    }
    unsigned int seed=0x51A6;
    auto random=[&seed](){ seed=seed*1664525u+1013904223u; return (seed>>8)/16777216.f; };
    for(int i=0;i<190;++i) stars.push_back({random(),random(),.5f+random()*1.1f,random()*6.28f});
}
void Art::unload() {
    for(auto texture:planets) UnloadTexture(texture);
    UnloadFont(interface); UnloadFont(telemetry);
}
void Art::background(float w,float h,float time,bool motion) const {
    DrawRectangle(0,0,static_cast<int>(w),static_cast<int>(h),ink);
    DrawCircleGradient(static_cast<int>(w*.72f),static_cast<int>(h*.36f),w*.56f,{18,29,55,95},{7,11,23,0});
    DrawCircleGradient(static_cast<int>(w*.19f),static_cast<int>(h*.85f),w*.4f,{17,45,48,48},{7,11,23,0});
    for(const auto& s:stars) {
        const float twinkle=motion ? .55f+.2f*std::sin(time*.55f+s.phase) : .6f;
        float x=s.x*w+(motion?std::sin(time*.018f+s.phase)*3:0);
        const float y=s.y*h;
        DrawCircleV({x,y},s.radius,alpha(s.phase<2 ? mint:paper,twinkle));
        if(s.radius>1.5f) {
            DrawLineEx({x-3,y},{x+3,y},.7f,alpha(paper,.2f));
            DrawLineEx({x,y-3},{x,y+3},.7f,alpha(paper,.2f));
        }
    }
}
void Art::planet(Vector2 c,float r,int type,BodyKind kind,float time,bool effects) const {
    const auto p=palette(type);
    if(effects && kind!=BodyKind::Rock) {
        DrawCircleGradient(static_cast<int>(c.x),static_cast<int>(c.y),r*(kind==BodyKind::Star?2.1f:1.7f),alpha(p.glow,.12f),alpha(p.glow,0));
        DrawCircleV(c,r+2,alpha(p.glow,.14f));
    }
    if(type==2) {
        for(int i=0;i<96;++i) {
            float a=i/96.f*6.283185f,b=(i+1)/96.f*6.283185f;
            DrawLineEx({c.x+std::cos(a)*r*1.6f,c.y+std::sin(a)*r*.4f},
                       {c.x+std::cos(b)*r*1.6f,c.y+std::sin(b)*r*.4f},r*.08f,alpha(p.light,.25f));
        }
    }
    DrawTexturePro(planets[std::clamp(type,0,4)],{0,0,512,512},{c.x-r,c.y-r,r*2,r*2},{0,0},0,WHITE);
    DrawCircleLinesV(c,r,alpha(p.glow,kind==BodyKind::Star?.7f:.35f));
    if(kind==BodyKind::Star && effects) {
        for(int i=0;i<3;++i) {
            float phase=std::fmod(time*.09f+i/3.f,1.f);
            DrawCircleLinesV(c,r+phase*r*.45f,alpha(p.glow,(1-phase)*.18f));
        }
    }
}
void Art::gate(Vector2 c,float r,float time,bool effects) const {
    if(effects) DrawCircleGradient(static_cast<int>(c.x),static_cast<int>(c.y),r*2.9f,alpha(mint,.13f),alpha(mint,0));
    DrawCircleLinesV(c,r*.72f,alpha(mint,.35f));
    DrawCircleLinesV(c,r,mint);
    for(int i=0;i<4;++i) {
        const float a=time*.28f+i*1.5707963f;
        const Vector2 p{c.x+std::cos(a)*r,c.y+std::sin(a)*r};
        DrawCircleV(p,2.7f,paper);
    }
    DrawCircleV(c,2,mint);
}
void Art::ship(Vector2 c,Vec velocity,float size,float thrust,float time) const {
    const float a=static_cast<float>(std::atan2(velocity.y,velocity.x));
    auto transform=[&](float x,float y){return Vector2{c.x+(x*std::cos(a)-y*std::sin(a))*size,c.y+(x*std::sin(a)+y*std::cos(a))*size};};
    if(thrust>0) {
        const float flicker=1.f+.2f*std::sin(time*63);
        DrawTriangle(transform(-.7f,-.35f),transform(-.7f,.35f),transform(-1.5f-thrust*flicker,0),alpha(mint,.7f));
    }
    DrawTriangle(transform(1.3f,0),transform(-.85f,-.75f),transform(-.4f,0),paper);
    DrawTriangle(transform(1.3f,0),transform(-.4f,0),transform(-.85f,.75f),{129,167,191,255});
    DrawCircleV(transform(.1f,0),size*.18f,mint);
}
void Art::star(Vector2 p,float r,Color c) const {
    Vector2 vertices[10];
    for(int i=0;i<10;++i) {
        const float a=-1.5707963f+i*.6283185f,rad=i%2 ? r*.43f:r;
        vertices[i]={p.x+std::cos(a)*rad,p.y+std::sin(a)*rad};
    }
    for(int i=0;i<10;++i) DrawTriangle(p,vertices[(i+1)%10],vertices[i],c);
}
void Art::text(const std::string& s,float x,float y,float size,Color colour,bool mono) const {
    DrawTextEx(mono?telemetry:interface,s.c_str(),{x,y},size,mono?0:0.25f,colour);
}
float Art::textWidth(const std::string& s,float size,bool mono) const {
    return MeasureTextEx(mono?telemetry:interface,s.c_str(),size,mono?0:.25f).x;
}
void Art::centred(const std::string& s,float x,float y,float size,Color c,bool mono) const { text(s,x-textWidth(s,size,mono)/2,y,size,c,mono); }
void Art::wrapped(const std::string& value,Rectangle area,float size,Color colour) const {
    std::istringstream words(value);
    std::string word,line;
    float y=area.y;
    while(words>>word) {
        const std::string next=line.empty()?word:line+" "+word;
        if(!line.empty() && textWidth(next,size)>area.width) { text(line,area.x,y,size,colour); y+=size*1.45f; line=word; }
        else line=next;
    }
    if(!line.empty()) text(line,area.x,y,size,colour);
}
} // namespace sling
