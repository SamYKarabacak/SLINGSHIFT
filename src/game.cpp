#include "game.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#ifdef __EMSCRIPTEN__
#include "web_input.hpp"
#endif

namespace sling {
static std::string number(double value,int digits=1) {
    char text[48]; std::snprintf(text,sizeof(text),"%.*f",digits,value); return text;
}
void Game::init() {
    progress.load(); art.load(); loadLevel(0); screen=Screen::Home;
    #ifdef __EMSCRIPTEN__
    installWebInput();
    #endif
}
void Game::close() { audio.close(); art.unload(); }
void Game::store() { saveFailed=!progress.save(); }
void Game::layout() {
    portrait=GetScreenWidth()<GetScreenHeight()*.92f;
    compact=!portrait && GetScreenHeight()<540 && GetScreenWidth()<1100;
    uiScale=std::min(GetScreenWidth()/(portrait?480.f:compact?1000.f:1280.f),GetScreenHeight()/(portrait?960.f:compact?460.f:800.f));
    width=GetScreenWidth()/uiScale;
    height=GetScreenHeight()/uiScale;
    boardScale=std::min((width-(portrait?32:80))/1200.f,(height-(compact?160:285))/540.f);
    boardScale=std::max(boardScale,.15f);
    boardOrigin={(width-1200*boardScale)/2,portrait?220.f:compact?70.f:132.f};
}
Vector2 Game::map(Vec p) const { return {boardOrigin.x+static_cast<float>(p.x)*boardScale,boardOrigin.y+static_cast<float>(p.y)*boardScale}; }
Vec Game::unmap(Vector2 p) const { return {(p.x-boardOrigin.x)/boardScale,(p.y-boardOrigin.y)/boardScale}; }
void Game::loadLevel(int index) {
    levelIndex=std::clamp(index,0,11); screen=Screen::Play; mode=Mode::Aiming;
    const auto& level=levels()[levelIndex];
    ship={level.launch,{}}; aim={285,-30};
    trail.clear(); particles.clear(); dragging=false; paused=false; showHint=false; fineAiming=false;
    accumulator=0; resultTime=0; correction={}; lastPreview=-1; shake=0; flash=0; focus=0;
    tutorial=levelIndex==0 && !progress.tutorialDone; tutorialStep=0;
    hintRoute=predict({level.launch,level.solution},level);
}
void Game::launch() {
    if(mode!=Mode::Aiming || length(aim)<35) return;
    ship={levels()[levelIndex].launch,limited(aim,maxLaunchSpeed)};
    mode=Mode::Flying; dragging=false; accumulator=0; trail.clear(); ++attempts; focus=0;
    if(tutorial) tutorialStep=2;
    audio.play(Cue::Launch); burst(ship.position,mint,14);
}
void Game::burst(Vec at,Color colour,int count) {
    for(int i=0;i<count;++i) {
        const double a=GetRandomValue(0,6283)/1000.0,speed=GetRandomValue(20,170);
        const float life=GetRandomValue(30,105)/100.f;
        particles.push_back({at,{std::cos(a)*speed,std::sin(a)*speed},life,life,colour});
    }
}
void Game::finish() {
    resultTime=0; dragging=false; accumulator=0;
    if(ship.outcome==Outcome::Arrived) {
        mode=Mode::Won; audio.play(Cue::Arrival); burst(ship.position,mint,80); flash=.25f;
        progress.stars[levelIndex]=std::max(progress.stars[levelIndex],starsFor(ship,levels()[levelIndex]));
        if(progress.best[levelIndex]==0 || ship.time<progress.best[levelIndex]) progress.best[levelIndex]=ship.time;
        if(levelIndex==0) progress.tutorialDone=true;
        store();
    } else {
        mode=Mode::Lost; audio.play(Cue::Impact); burst(ship.position,danger,56); shake=7; flash=.12f;
    }
}
void Game::update(float dt) {
    clock+=dt;
    shake=std::max(0.f,shake-dt*23); flash=std::max(0.f,flash-dt);
    for(auto& p:particles) { p.position+=p.velocity*dt; p.velocity=p.velocity*std::exp(-dt*2); p.life-=dt; }
    std::erase_if(particles,[](const Particle& p){return p.life<=0;});
    if(screen!=Screen::Play) {
        if(IsKeyPressed(KEY_ESCAPE)) {
            if(screen==Screen::Settings) { screen=settingsReturn; focus=0; }
            else { screen=Screen::Home; focus=0; }
        }
        return;
    }
    if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) { paused=!paused; dragging=false; accumulator=0; }
    if(IsKeyPressed(KEY_R)) { loadLevel(levelIndex); return; }
    if(!IsWindowFocused() && mode==Mode::Flying) { paused=true; dragging=false; }
    if(paused) return;
    if(mode==Mode::Won || mode==Mode::Lost) { resultTime+=dt; return; }
    const auto& level=levels()[levelIndex];
    if(mode==Mode::Aiming) {
        double angle=std::atan2(aim.y,aim.x),speed=length(aim);
        const double precision=IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)?.15:1.0;
        if(IsKeyDown(KEY_LEFT)) angle-=dt*.85*precision;
        if(IsKeyDown(KEY_RIGHT)) angle+=dt*.85*precision;
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD)) speed+=dt*100*precision;
        if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) speed-=dt*100*precision;
        speed=std::clamp(speed,40.0,maxLaunchSpeed);
        aim={std::cos(angle)*speed,std::sin(angle)*speed};
        const auto origin=map(level.launch);
        if(pointerPressed && CheckCollisionPointRec(pointer,{boardOrigin.x,boardOrigin.y,1200*boardScale,540*boardScale})) {
            dragging=true;
            fineAiming=!CheckCollisionPointCircle(pointer,origin,std::max(portrait?34.f:40.f,24/uiScale));
            dragAnchor=fineAiming?unmap(pointer):level.launch;
            aimBeforeDrag=aim;
        }
        if(dragging) {
            aim=limited(fineAiming?aimBeforeDrag+(dragAnchor-unmap(pointer))*(boardScale*.45):(dragAnchor-unmap(pointer))*(portrait?3.0:2.15),maxLaunchSpeed);
            if(tutorial && length(aim)>100) tutorialStep=1;
            if(pointerReleased) { dragging=false; if(!fineAiming && length(aim)>35) launch(); fineAiming=false; }
        }
        if(IsKeyPressed(KEY_SPACE)) launch();
    } else if(mode==Mode::Flying) {
        const Rectangle board{boardOrigin.x,boardOrigin.y,1200*boardScale,540*boardScale};
        if(!ship.corrected && pointerPressed && CheckCollisionPointRec(pointer,board)) {
            dragging=true; dragAnchor=unmap(pointer); correction={};
        }
        if(dragging) {
            correction=limited((dragAnchor-unmap(pointer))*.7,maxCorrection);
            if(pointerReleased) {
                dragging=false;
                if(correct(ship,correction)) { audio.play(Cue::Burn); burst(ship.position,gold,25); }
                correction={};
            }
        } else if(!ship.corrected) {
            Vec direction{static_cast<double>(IsKeyDown(KEY_RIGHT)-IsKeyDown(KEY_LEFT)),static_cast<double>(IsKeyDown(KEY_DOWN)-IsKeyDown(KEY_UP))};
            if(length(direction)>0) correction=limited(direction*maxCorrection,maxCorrection);
            if(IsKeyPressed(KEY_SPACE) && correct(ship,correction)) { audio.play(Cue::Burn); burst(ship.position,gold,25); correction={}; }
        }
        accumulator+=dt*(dragging?.12f:1.f);
        int steps=0;
        while(accumulator>=stepSeconds && steps++<32) {
            trail.push_back(ship.position); if(trail.size()>520) trail.pop_front();
            advance(ship,level); accumulator-=static_cast<float>(stepSeconds);
            if(ship.outcome!=Outcome::Flying) { finish(); break; }
        }
    }
    if(clock-lastPreview>.035 || lastPreview<0) {
        Flight future=mode==Mode::Aiming?Flight{level.launch,aim}:ship;
        if(mode==Mode::Flying && length(correction)>0 && !ship.corrected) correct(future,correction);
        preview=predict(future,level,9); lastPreview=clock;
    }
}
void Game::card(Rectangle r,Color border) {
    DrawRectangleRounded(r,.12f,8,panel);
    DrawRectangleRoundedLinesEx(r,.12f,8,1,border);
}
bool Game::button(const std::string& label,Rectangle r,bool primary,bool enabled) {
    const bool blocked=screen==Screen::Play && (paused || mode==Mode::Won || mode==Mode::Lost) && !modalLayer;
    const bool active=enabled && !blocked;
    const int index=active?uiCount++:-1;
    bool hover=CheckCollisionPointRec(pointer,r);
    const bool focused=active && focus==index && keyboardNavigation;
    if(hover && active) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    Color fill=primary ? mint : panel;
    if(hover && active) fill=primary?Color{151,255,225,255}:Color{27,42,60,255};
    if(!enabled) fill=alpha(panel,.6f);
    DrawRectangleRounded(r,.18f,8,fill);
    DrawRectangleRoundedLinesEx(r,.18f,8,1,focused?gold:alpha(primary?mint:muted,enabled?.3f:.1f));
    art.centred(label,r.x+r.width/2,r.y+(r.height-(portrait?17:18))/2-1,portrait?17:18,enabled?(primary?ink:paper):alpha(muted,.4f));
    if(active && ((hover && pointerPressed) || (focus==index && keyboardActivate))) { audio.play(Cue::Tap); keyboardActivate=false; focus=0; return true; }
    return false;
}
void Game::drawTrajectory(const Trace& trace,Color colour,bool ghost) {
    for(std::size_t i=2;i<trace.points.size();i+=2) {
        const auto p=map(trace.points[i]);
        const float fade=1-static_cast<float>(i)/static_cast<float>(trace.points.size())*.65f;
        DrawCircleV(p,std::max(ghost?1.7f:1.8f,1.1f/uiScale),alpha(colour,fade*(ghost?.75f:.8f)));
    }
    if(!trace.points.empty() && !ghost) {
        const auto p=map(trace.end.position);
        if(trace.end.outcome==Outcome::Arrived) { DrawCircleLinesV(p,8,mint); }
        else { DrawLineEx({p.x-4,p.y-4},{p.x+4,p.y+4},1.5f,alpha(colour,.6f)); DrawLineEx({p.x-4,p.y+4},{p.x+4,p.y-4},1.5f,alpha(colour,.6f)); }
    }
}
void Game::drawPlay() {
    const auto& level=levels()[levelIndex];
    art.text("SLINGSHIFT",portrait?20:compact?24:40,compact?18:24,portrait?21:compact?23:25,paper);
    art.text(level.region,compact?235:portrait?20:40,compact?14:75,compact?9:12,mint,true);
    art.text(level.name,compact?235:portrait?20:40,compact?32:portrait?98:95,compact?22:portrait?29:28);
    if(button("Retry",{width-(portrait?166:compact?204:250),compact?12.f:20.f,portrait?70.f:compact?84.f:100.f,compact?46.f:42.f})) { loadLevel(levelIndex); return; }
    if(button("Pause",{width-(portrait?86:compact?108:140),compact?12.f:20.f,portrait?66.f:compact?84.f:100.f,compact?46.f:42.f})) { paused=true; dragging=false; }
    if(portrait) art.wrapped(level.brief,{20,145,width-40,70},16);
    else if(!compact) art.text(level.brief,400,101,17,muted);
    const Vector2 stableOrigin=boardOrigin;
    if(progress.motion && shake>0) { boardOrigin.x+=std::sin(clock*91)*shake; boardOrigin.y+=std::cos(clock*113)*shake*.6f; }
    const Rectangle board{boardOrigin.x,boardOrigin.y,1200*boardScale,540*boardScale};
    DrawRectangleLinesEx(board,1,alpha(muted,.12f));
    if(showField) for(double y=45;y<540;y+=60) for(double x=45;x<1200;x+=60) {
        const Vec a=acceleration({x,y},level.bodies);
        const double n=length(a); if(n<.5) continue;
        bool inside=false; for(const auto& b:level.bodies) if(length(Vec{x,y}-b.position)<b.radius+18) inside=true;
        if(inside) continue;
        const Vec d=a/std::max(n,1.0)*std::min(15.0,n*.4);
        DrawLineEx(map({x,y}),map(Vec{x,y}+d),1,alpha(violet,.22f));
    }
    for(const auto& body:level.bodies) {
        art.planet(map(body.position),static_cast<float>(body.radius)*boardScale,body.palette,body.kind,progress.motion?clock:0,progress.glow);
    }
    art.gate(map(level.gate),static_cast<float>(level.gateRadius)*boardScale,progress.motion?clock:0,progress.glow);
    const auto start=map(level.launch),gate=map(level.gate);
    art.centred("DEPARTURE",start.x,start.y+31,portrait?10:11,muted,true);
    art.centred("ARRIVAL",gate.x,gate.y+static_cast<float>(level.gateRadius)*boardScale+14,portrait?10:11,mint,true);
    DrawCircleLinesV(start,18,alpha(mint,.25f));
    if(showHint && mode==Mode::Aiming) drawTrajectory(hintRoute,gold,true);
    if(mode==Mode::Aiming || mode==Mode::Flying) drawTrajectory(preview,mode==Mode::Flying && length(correction)>0?gold:mint);
    for(std::size_t i=1;i<trail.size();i+=2) DrawLineEx(map(trail[i-1]),map(trail[i]),2,alpha(mint,static_cast<float>(i)/trail.size()*.5f));
    if(mode==Mode::Aiming) {
        art.ship(start,aim,std::max(portrait?8.f:10.f,7/uiScale),dragging?.25f:0.f,clock);
        const Vec visual=limited(aim/(portrait?3.0:2.15),200);
        const auto tail=map(level.launch-visual);
        DrawLineEx(start,tail,1.5f,alpha(mint,.5f)); DrawCircleLinesV(tail,6,alpha(mint,.7f));
        if(!dragging) DrawCircleLinesV(start,26+(progress.motion?std::sin(clock*2)*3:0),alpha(mint,.25f));
    } else if(mode==Mode::Flying) {
        const float blend=std::clamp(accumulator/static_cast<float>(stepSeconds),0.f,1.f);
        const Vec visible=trail.empty()?ship.position:trail.back()*(1-blend)+ship.position*blend;
        art.ship(map(visible),ship.velocity,std::max(portrait?9.f:10.f,7/uiScale),.6f,clock);
        if(length(correction)>0 && !ship.corrected) DrawLineEx(map(ship.position),map(ship.position+correction),2,gold);
    }
    for(const auto& p:particles) DrawCircleV(map(p.position),std::max(1.f,3*p.life/p.maxLife),alpha(p.colour,p.life/p.maxLife));
    if(progress.motion && flash>0) DrawRectangleRec(board,alpha(mode==Mode::Won?mint:danger,flash*.25f));
    boardOrigin=stableOrigin;
    const float bottom=board.y+board.height+(compact?12:24);
    art.text(mode==Mode::Aiming?"PULL BACK. FIND YOUR LINE.":ship.corrected?"BURN SPENT. TRUST THE ORBIT.":dragging?"TIME DILATED / RELEASE TO BURN":"ONE BURN. MAKE IT COUNT.",portrait?20:40,bottom,portrait?13:15,mode==Mode::Aiming?mint:gold,true);
    if(!compact) art.wrapped(mode==Mode::Aiming?"Pull from the ship to launch. Drag empty space to refine your aim, then tap Launch.":"Drag anywhere in space for one correction. Time slows while you aim.",{portrait?20.f:40.f,bottom+29,portrait?width-40:740.f,70},portrait?16:17);
    if(fineAiming && dragging) art.centred("FINE AIM / RELEASE TO HOLD YOUR LINE",width/2,board.y+board.height-28,portrait?10:12,gold,true);
    const float telemetryX=compact?40.f:portrait?20.f:width-430;
    const float telemetryY=compact?bottom+28:portrait?bottom+77:bottom+60;
    art.text(mode==Mode::Aiming?"POWER "+number(length(aim)/maxLaunchSpeed*100,0)+"%":number(ship.time,2)+" s  /  BURN "+(ship.corrected?"0":"1"),telemetryX,telemetryY,12,muted,true);
    if(portrait && mode==Mode::Aiming) {
        if(button("-",{width-128,bottom+66,48,36})) { const double speed=std::max(40.0,length(aim)-20); aim=aim/std::max(1.0,length(aim))*speed; lastPreview=-1; }
        if(button("+",{width-70,bottom+66,48,36})) { const double speed=std::min(maxLaunchSpeed,length(aim)+20); aim=aim/std::max(1.0,length(aim))*speed; lastPreview=-1; }
    }
    if(mode==Mode::Aiming && button("Launch",{portrait?20.f:compact?width-165:width-230,bottom+(portrait?112:0),portrait?width-40:compact?140.f:190.f,compact?52.f:48.f},true)) launch();
    if(button(showHint?"Hide route":"Route hint",{compact?width-477:portrait?20.f:40.f,bottom+(portrait?174:compact?0:86),portrait?136.f:compact?140.f:145.f,compact?52.f:42.f})) showHint=!showHint;
    if(button(showField?"Hide gravity":"Gravity field",{compact?width-325:portrait?166.f:198.f,bottom+(portrait?174:compact?0:86),portrait?145.f:compact?148.f:160.f,compact?52.f:42.f})) showField=!showField;
    if(showHint && mode==Mode::Aiming && !compact) art.wrapped(level.hint,{portrait?20.f:390.f,bottom+(portrait?234:85),portrait?width-40:650.f,65},15,gold);
    if(!showHint && !compact) art.text("3 STARS: ARRIVE / NO BURN / UNDER "+number(level.par)+" s",portrait?20:390,bottom+(portrait?234:97),portrait?11:12,muted,true);
    if(tutorial && mode!=Mode::Won && mode!=Mode::Lost && !paused) {
        const float tw=portrait?width-40:compact?350:430,tx=portrait?20:compact?width-375:width-470,ty=portrait?bottom+282:board.y+24;
        card({tx,ty,tw,portrait?120.f:110.f},alpha(mint,.4f));
        art.text(tutorialStep==0?"01 / TAKE THE SLING":tutorialStep==1?"02 / READ THE FUTURE":"03 / YOUR EMERGENCY BURN",tx+18,ty+14,12,mint,true);
        art.wrapped(tutorialStep==0?"Touch the ship. Pull backward, away from the gate. The farther you pull, the faster you fly.":tutorialStep==1?"The dots are your future. Bend them into the green gate, then release to launch.":"Need a nudge? Drag anywhere in space for one burn. A clean arrival earns an extra star.",{tx+18,ty+38,tw-36,80},15);
    }
    if((mode==Mode::Won || mode==Mode::Lost) && !paused && resultTime>.4f) drawResult();
    if(paused) drawPause();
}
void Game::drawResult() {
    modalLayer=true;
    DrawRectangle(0,0,static_cast<int>(width),static_cast<int>(height),alpha(ink,.8f));
    const float w=portrait?width-40:500,x=(width-w)/2,y=std::max(85.f,(height-330)/2);
    card({x,y,w,330},alpha(mode==Mode::Won?mint:danger,.35f));
    const bool won=mode==Mode::Won;
    art.centred(won?"SIGNAL RECEIVED":"SIGNAL LOST",width/2,y+26,12,won?mint:danger,true);
    art.centred(won?"Beautifully flown.":"A different line.",width/2,y+55,portrait?29:33);
    if(won) {
        const bool criteria[3]={true,!ship.corrected,ship.time<=levels()[levelIndex].par};
        for(int i=0;i<3;++i) {
            const float pop=std::clamp((resultTime-.45f-i*.11f)*7.f,0.f,1.f);
            art.star({width/2+(i-1)*(portrait?112.f:135.f),y+125},17*pop,criteria[i]?gold:alpha(muted,.2f));
            art.centred(i==0?"ARRIVED":i==1?"NO BURN":"ON TIME",width/2+(i-1)*(portrait?112.f:135.f),y+153,10,criteria[i]?gold:muted,true);
        }
        art.centred(number(ship.time,2)+" s  /  best "+number(progress.best[levelIndex],2)+" s",width/2,y+180,14,muted,true);
    } else {
        std::string message=ship.outcome==Outcome::Impact?"That world has a surface. Give it more room.":ship.outcome==Outcome::Expired?"Orbit is lovely. The gate is waiting.":ship.outcome==Outcome::Invalid?"Flight computer reset. Retry to launch again.":"You left the chart. Try a gentler angle.";
        if(ship.outcome==Outcome::Impact && ship.hitBody>=0 && levels()[levelIndex].bodies[ship.hitBody].kind==BodyKind::Star) message="Too close to the heat. Start wider and let the star pull you in.";
        art.wrapped(message,{x+30,y+115,w-60,75},17);
    }
    if(button(won?(levelIndex==11?"The journey home":"Next signal"):"Try again",{x+30,y+216,w-60,46},true,resultTime>.35f)) {
        if(won && levelIndex==11) { screen=Screen::Complete; focus=0; }
        else loadLevel(won?levelIndex+1:levelIndex);
        return;
    }
    if(button(won?"Fly again":"Choose a signal",{x+30,y+274,w-60,38},false,resultTime>.35f)) {
        if(won) loadLevel(levelIndex); else { screen=Screen::Chart; focus=0; }
    }
}
void Game::drawPause() {
    modalLayer=true;
    DrawRectangle(0,0,static_cast<int>(width),static_cast<int>(height),alpha(ink,.85f));
    const float w=portrait?width-40:400,x=(width-w)/2,y=std::max(70.f,(height-390)/2);
    card({x,y,w,390}); art.centred("Holding position",width/2,y+30,29);
    if(button("Resume",{x+30,y+93,w-60,48},true)) paused=false;
    if(button("Restart",{x+30,y+153,w-60,48})) loadLevel(levelIndex);
    if(button("Settings",{x+30,y+213,w-60,48})) { settingsReturn=Screen::Play; screen=Screen::Settings; focus=0; }
    if(button("Signal chart",{x+30,y+273,w-60,48})) { screen=Screen::Chart; focus=0; }
    art.centred("R / INSTANT RETRY    ESC / RESUME",width/2,y+349,11,muted,true);
}
void Game::drawHome() {
    const float left=portrait?25.f:compact?40.f:80.f;
    art.text("AN ORBITAL PUZZLE GAME",left,portrait?36:compact?25:70,12,mint,true);
    art.text("SLINGSHIFT",left,portrait?71:compact?56:111,portrait?53:compact?65:91,paper);
    art.wrapped("A little thrust. A lot of gravity. Find your way through twelve quiet corners of the universe.",{left,portrait?145.f:compact?145.f:228.f,portrait?width-50:compact?450.f:500.f,110},portrait?19:compact?19:23,muted);
    const Vector2 p{portrait?width*.71f:width*.76f,portrait?345.f:height*.47f};
    const float r=portrait?89.f:compact?105.f:151.f;
    for(int i=0;i<170;++i) {
        float a=i/170.f*6.283185f,b=(i+1)/170.f*6.283185f;
        auto orbit=[&](float t){const float x=std::cos(t)*r*2.1f,y=std::sin(t)*r*.77f; return Vector2{p.x+x*.93f-y*.36f,p.y+x*.36f+y*.93f};};
        DrawLineEx(orbit(a),orbit(b),1,alpha(mint,.13f));
    }
    art.planet(p,r,0,BodyKind::Planet,clock,progress.glow);
    art.planet({p.x-r*1.46f,p.y+r*.61f},r*.21f,2,BodyKind::Planet,clock,progress.glow);
    const float t=progress.motion?clock*.08f:0;
    const Vector2 courier{p.x+std::cos(t+2.7f)*r*1.95f,p.y+std::sin(t+2.7f)*r*.85f};
    art.ship(courier,{1,-.55},portrait?10:13,.7f,clock);
    art.gate({p.x+r*1.44f,p.y-r*.9f},portrait?13:23,progress.motion?clock:0,progress.glow);
    const float by=portrait?std::max(475.f,height-330):compact?235.f:370.f;
    const float bw=portrait?width-50:305.f;
    if(button(progress.totalStars()?"Continue / signal "+number(progress.unlocked()+1,0):"Begin the journey",{left,by,bw,54},true)) { loadLevel(progress.unlocked()); return; }
    if(button("Signal chart",{left,by+68,bw,48})) { screen=Screen::Chart; focus=0; return; }
    if(button("Settings",{left,by+130,(bw-12)/2,46})) { settingsReturn=Screen::Home; screen=Screen::Settings; focus=0; return; }
    if(button("About",{left+(bw+12)/2,by+130,(bw-12)/2,46})) { screen=Screen::Credits; focus=0; return; }
    if(!compact) art.text(number(progress.totalStars(),0)+" / 36 STARS",left,by+203,12,gold,true);
    art.text(portrait?"DRAG TO AIM / RELEASE TO FLY":"DRAG TO AIM / RELEASE TO FLY / ONE BURN TO CORRECT",left,height-(compact?25:45),11,muted,true);
    if(!portrait && !compact) art.text("v1.0 / NO SIGNAL REQUIRED",width-315,height-45,11,muted,true);
}
void Game::drawChart() {
    art.text("THE SIGNAL CHART",portrait?20:60,compact?20:40,12,mint,true);
    art.text("Twelve ways home.",portrait?20:60,compact?46:72,portrait?31:compact?32:43);
    art.text(number(progress.totalStars(),0)+" / 36 STARS",portrait?20:60,compact?89:portrait?123:133,12,gold,true);
    if(button("Home",{width-(portrait?100:170),34,portrait?80.f:110.f,44})) { screen=Screen::Home; focus=0; return; }
    const int columns=portrait?2:4;
    const float margin=portrait?20:60,gap=portrait?12:compact?10:18,cw=(width-2*margin-(columns-1)*gap)/columns;
    const float ch=portrait?112:compact?88:145,top=portrait?177:compact?125:210;
    for(int i=0;i<12;++i) {
        const float x=margin+(i%columns)*(cw+gap),y=top+(i/columns)*(ch+gap);
        const bool unlocked=i<=progress.unlocked();
        card({x,y,cw,ch},alpha(unlocked?mint:muted,unlocked?.2f:.1f));
        art.text((i<9?"0":"")+std::to_string(i+1)+(unlocked?" / SIGNAL":" / LOCKED"),x+15,y+13,10,unlocked?mint:muted,true);
        const auto& level=levels()[i];
        art.text(level.name,x+15,y+(compact?31:37),portrait?17:compact?16:21,unlocked?paper:muted);
        if(!portrait && !compact) {
            for(std::size_t j=0;j<level.bodies.size();++j) DrawCircleV({x+cw-23-static_cast<float>(j)*15,y+19},3,palette(level.bodies[j].palette).glow);
        }
        for(int j=0;j<3;++j) art.star({x+21+j*20,y+(compact?69:portrait?74:84)},6,j<progress.stars[i]?gold:alpha(muted,.2f));
        const bool selected=button(unlocked?(progress.stars[i]?"Replay":"Fly"):"Locked",{x+cw-(portrait?86:compact?82:100),y+ch-(compact?34:43),portrait?72.f:compact?70.f:86.f,compact?28.f:32.f},false,unlocked);
        if(selected || (unlocked && pointerPressed && CheckCollisionPointRec(pointer,{x,y,cw,ch}))) { loadLevel(i); return; }
        if(!portrait && !compact && progress.best[i]>0) art.text(number(progress.best[i],2)+" s",x+15,y+ch-31,11,muted,true);
    }
    art.text("REACH A GATE TO UNLOCK THE NEXT SIGNAL.",margin,top+(12/columns)*(ch+gap)+13,portrait?10:12,muted,true);
}
void Game::drawSettings() {
    const float w=portrait?width-40:600,x=(width-w)/2;
    art.text("FLIGHT PREFERENCES",x,compact?20:45,12,mint,true);
    art.text("Make space yours.",x,compact?44:79,portrait?32:compact?32:43);
    const float top=portrait?160:compact?100:180,row=compact?58:79;
    card({x,top,w,row*4+25});
    art.text("Volume",x+22,top+(compact?19:24),19);
    if(button("-",{x+w-196,top+(compact?7.f:15.f),44,44})) { progress.volume=std::max(0.f,progress.volume-.1f); store(); }
    art.centred(number(progress.volume*100,0)+"%",x+w-111,top+(compact?21:28),16,muted,true);
    if(button("+",{x+w-70,top+(compact?7.f:15.f),44,44})) { progress.volume=std::min(1.f,progress.volume+.1f); store(); }
    const char* names[]={"Ambient sound","Background motion","Atmosphere glow"};
    bool* values[]={&progress.ambience,&progress.motion,&progress.glow};
    for(int i=0;i<3;++i) {
        const float y=top+row*(i+1);
        DrawLineEx({x+22,y},{x+w-22,y},1,alpha(muted,.12f));
        art.text(names[i],x+22,y+(compact?19:24),portrait?17:19);
        if(button(*values[i]?"On":"Off",{x+w-110,y+(compact?7.f:15.f),84,44})) { *values[i]=!*values[i]; store(); }
    }
    if(!compact) art.wrapped("Motion off removes starfield drift, camera shake and flashes. Audio starts after your first interaction.",{x+6,top+row*4+45,w-12,70},15);
    if(button("Fullscreen / F11",{x,top+row*4+(compact?44:124),compact?(w-14)/2:w,45})) ToggleFullscreen();
    if(button("Back",{compact?x+(w+14)/2:x,top+row*4+(compact?44:183),compact?(w-14)/2:w,48},true)) { screen=settingsReturn; focus=0; }
}
void Game::drawCredits() {
    const float w=portrait?width-40:700,x=(width-w)/2;
    if(compact) {
        art.text("SLINGSHIFT",35,24,40);
        art.text("FLIGHT MANUAL",35,86,12,mint,true);
        const float column=(width-105)/2;
        art.wrapped("Pull backward from your ship and release to launch. Drag empty space to refine the projected route, then tap Launch. In flight, drag anywhere for one emergency burn. Time slows while aiming.",{35,121,column,180},18);
        art.wrapped("Keyboard: arrows aim; Up / Down change power; Shift refines; Space launches. In flight, arrows choose a burn direction and Space fires. R retries. Escape pauses. Tab and Enter navigate. F11 toggles fullscreen.",{70+column,121,column,180},18);
        art.wrapped("Created by Sam Y. Karabacak. A small game about finding a way. Simplified fixed-body gravity; not a spaceflight simulator. Progress stays on this device.",{35,322,width-70,75},15);
        if(button("Back to space",{width/2-180,height-62,360,46},true)) { screen=Screen::Home; focus=0; }
        return;
    }
    art.text("SLINGSHIFT",x,55,portrait?40:59);
    art.text("A SMALL GAME ABOUT FINDING A WAY.",x,portrait?119:139,portrait?11:13,mint,true);
    art.wrapped("Twelve orbital puzzles. One spacecraft. No fuel to waste. Pull backward to set your launch, then use gravity to reach the green gate.",{x,190,w,100},20,paper);
    art.text("FLIGHT MANUAL",x,310,12,mint,true);
    art.wrapped("Mouse / touch: pull back from the ship and release. Drag empty space to refine your aim, then tap Launch. In flight, pull anywhere for one correction. Time slows while you aim.",{x,341,w,110},18);
    art.wrapped("Keyboard: Left / Right rotate. Up / Down or + / - change power. Hold Shift for precision. Space launches. In flight, arrows choose a burn direction and Space fires. R retries. Escape pauses. Tab and Enter navigate menus. F11 toggles fullscreen.",{x,450,w,150},17);
    art.wrapped("Created by Sam Y. Karabacak. A small game about finding a way. This is a simplified, fixed-body gravity puzzle, not a spaceflight simulator. Progress stays on this device.",{x,portrait?612.f:582.f,w,105},15);
    if(button("Back to space",{x,portrait?780.f:711.f,w,48},true)) { screen=Screen::Home; focus=0; }
}
void Game::drawComplete() {
    if(compact) {
        art.gate({width*.78f,height*.43f},45,progress.motion?clock:0,progress.glow);
        art.text("ALL SIGNALS FOUND",40,60,12,mint,true);
        art.text("You made it home.",40,92,43);
        art.text(number(progress.totalStars(),0)+" / 36 STARS",40,166,20,gold,true);
        art.wrapped("Some paths looked impossible from the launch pad. You found a line through every one of them.",{40,220,500,95},20);
        if(button(progress.totalStars()==36?"Fly your favorite signals":"Chase the remaining stars",{40,329,430,50},true)) { screen=Screen::Chart; focus=0; }
        if(button("Return to the quiet",{40,392,430,46})) { screen=Screen::Home; focus=0; }
        art.text("Created by Sam Y. Karabacak",width*.58f,365,14,muted);
        return;
    }
    const float centre=width/2;
    art.gate({centre,portrait?120.f:160.f},42,progress.motion?clock:0,progress.glow);
    art.centred("ALL SIGNALS FOUND",centre,portrait?213:260,12,mint,true);
    art.centred("You made it home.",centre,portrait?245:296,portrait?35:57);
    art.centred(number(progress.totalStars(),0)+" / 36 STARS",centre,portrait?315:380,20,gold,true);
    art.wrapped("Some paths looked impossible from the launch pad. You found a line through every one of them.",{portrait?30.f:centre-260,portrait?367.f:435.f,portrait?width-60:520.f,100},20);
    const float x=portrait?30:centre-220,w=portrait?width-60:440,y=portrait?500:550;
    if(button(progress.totalStars()==36?"Fly your favorite signals":"Chase the remaining stars",{x,y,w,52},true)) { screen=Screen::Chart; focus=0; }
    if(button("Return to the quiet",{x,y+68,w,46})) { screen=Screen::Home; focus=0; }
    art.centred("Created by Sam Y. Karabacak",centre,y+132,14,muted);
}
void Game::frame() {
    layout();
    #ifdef __EMSCRIPTEN__
    const auto input=pollWebPointer();
    pointer={input.x/uiScale,input.y/uiScale};
    pointerPressed=input.pressed; pointerReleased=input.released; pointerDown=input.down;
    if(input.cancelled) { dragging=false; fineAiming=false; correction={}; }
    #else
    pointer={GetMousePosition().x/uiScale,GetMousePosition().y/uiScale};
    pointerPressed=IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    pointerReleased=IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    pointerDown=IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    // raylib's gesture queue retains very short taps that begin and end between frames.
    if(!dragging && !pointerDown && !pointerReleased &&
       (IsGestureDetected(GESTURE_TAP) || IsGestureDetected(GESTURE_DOUBLETAP))) {
        pointerPressed=true; pointerReleased=true;
    }
    #endif
    if(pointerPressed || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) audio.init();
    audio.update(progress.volume,progress.ambience);
    keyboardActivate=IsKeyPressed(KEY_ENTER);
    if(pointerPressed) keyboardNavigation=false;
    if(IsKeyPressed(KEY_TAB)) {
        if(keyboardNavigation) focus=(focus+(IsKeyDown(KEY_LEFT_SHIFT)?-1:1)+std::max(previousUiCount,1))%std::max(previousUiCount,1);
        else focus=0;
        keyboardNavigation=true;
    }
    #ifndef __EMSCRIPTEN__
    if(IsKeyPressed(KEY_F11)) ToggleFullscreen();
    #endif
    const float dt=std::min(GetFrameTime(),.1f);
    update(dt); uiCount=0; modalLayer=false;
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    BeginDrawing(); ClearBackground(ink);
    Camera2D ui{{0,0},{0,0},0,uiScale}; BeginMode2D(ui);
    art.background(width,height,clock,progress.motion);
    switch(screen) {
        case Screen::Home: drawHome(); break;
        case Screen::Chart: drawChart(); break;
        case Screen::Play: drawPlay(); break;
        case Screen::Settings: drawSettings(); break;
        case Screen::Credits: drawCredits(); break;
        case Screen::Complete: drawComplete(); break;
    }
    if(screen!=previousScreen) { transition=.18f; previousScreen=screen; }
    if(transition>0) { DrawRectangle(0,0,static_cast<int>(width),static_cast<int>(height),alpha(ink,transition/.18f*.7f)); transition=std::max(0.f,transition-dt); }
    if(saveFailed) art.centred("Progress could not be saved on this device.",width/2,height-26,14,danger);
    EndMode2D(); EndDrawing(); previousUiCount=uiCount;
}
} // namespace sling
