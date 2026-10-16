#include "physics.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <string_view>

using namespace sling;
int checks=0, failures=0;
void check(bool ok, std::string_view message) {
    ++checks;
    if (!ok) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; }
}
int main() {
    Level empty{"test","","","",{100,100},{1100,450},20,10,{}, {}};
    const std::vector<Body> one{{{600,270},50,100000,0}};
    check(length(acceleration({600,270},one))==0,"finite force at a body centre");
    check(finite(acceleration({600+1e-10,270},one)),"near-centre force remains finite");
    check(acceleration({700,270},one).x<0,"attraction points toward mass");
    const auto near=length(acceleration({700,270},one));
    const auto far=length(acceleration({800,270},one));
    check(near/far>3.8 && near/far<4.0,"softened inverse-square falloff");
    const std::vector<Body> pair{{{400,270},30,100000},{{800,270},30,100000}};
    check(length(acceleration({600,270},pair))<1e-12,"superposition and symmetry");
    Flight s{{100,100},{80,30}};
    for(int i=0;i<120;++i) advance(s,empty);
    check(length(s.position-Vec{180,130})<1e-9,"zero-force constant velocity");
    check(std::abs(s.time-1)<1e-12,"fixed clock");
    check(std::abs(segmentCircle({0,0},{100,0},{50,0},10)-0.4)<1e-12,"swept collision");
    check(std::abs(segmentCircle({0,10},{100,10},{50,0},10)-0.5)<1e-12,"tangent collision");
    check(segmentCircle({50,0},{50,0},{50,0},10)==0,"stationary initial overlap");
    check(!std::isfinite(segmentCircle({0,0},{0,0},{50,0},10)),"stationary outside");
    Level tunnel=empty;
    tunnel.bodies={{{130,100},1,0}};
    Flight fast{{100,100},{4000,0}};
    advance(fast,tunnel);
    check(fast.outcome==Outcome::Impact && fast.position.x<130,"fast craft cannot tunnel");
    tunnel.gate={115,100}; tunnel.gateRadius=2;
    fast={{100,100},{4000,0}}; advance(fast,tunnel);
    check(fast.outcome==Outcome::Arrived,"earlier gate beats later obstacle");
    tunnel.gate={150,100}; fast={{100,100},{4000,0}}; advance(fast,tunnel);
    check(fast.outcome==Outcome::Impact,"earlier obstacle beats later gate");
    Flight invalid{{std::numeric_limits<double>::quiet_NaN(),0},{1,0}};
    advance(invalid,empty); check(invalid.outcome==Outcome::Invalid,"NaN is contained");
    invalid={{100,100},{1,0}}; advance(invalid,empty,0);
    check(invalid.outcome==Outcome::Invalid,"invalid timestep rejected");
    Flight escape{{1199,100},{400,0}}; advance(escape,empty);
    check(escape.outcome==Outcome::Escaped && std::abs(escape.position.x-1200)<1e-9,"swept world boundary");
    Flight expired{{100,100},{0,0},timeLimit}; advance(expired,empty);
    check(expired.outcome==Outcome::Expired,"flight timeout");
    Flight burn{{100,100},{0,0}};
    check(!correct(burn,{0,0}) && !burn.corrected,"empty burn costs nothing");
    check(correct(burn,{1000,0}) && std::abs(length(burn.velocity)-maxCorrection)<1e-12,"burn clamps impulse");
    check(!correct(burn,{10,10}),"only one midflight burn");
    burn.outcome=Outcome::Arrived; burn.time=1;
    check(starsFor(burn,empty)==2,"burn scoring"); burn.corrected=false;
    check(starsFor(burn,empty)==3,"clean fast scoring"); burn.time=11;
    check(starsFor(burn,empty)==2,"slow clean scoring"); burn.corrected=true;
    check(starsFor(burn,empty)==1,"arrival always earns a star");

    // A softened circular orbit should retain its energy and return near its start.
    const double r=120, mu=250000;
    const double speed=std::sqrt(mu*r*r/std::pow(r*r+softening*softening,1.5));
    Level orbit=empty; orbit.bodies={{{600,270},30,mu}};
    Flight satellite{{600+r,270},{0,speed}};
    const double energy0=dot(satellite.velocity,satellite.velocity)/2+potential(satellite.position,orbit.bodies);
    const int count=static_cast<int>(2*3.141592653589793*r/speed/stepSeconds);
    double drift=0;
    for(int i=0;i<count;++i) {
        advance(satellite,orbit);
        drift=std::max(drift,std::abs(dot(satellite.velocity,satellite.velocity)/2+potential(satellite.position,orbit.bodies)-energy0));
    }
    check(satellite.outcome==Outcome::Flying,"circular orbit survives");
    check(drift/std::abs(energy0)<1e-5,"Verlet orbital energy drift");
    check(length(satellite.position-Vec{720,270})<1,"orbital period closure");
    const Vec relative=satellite.position-Vec{600,270};
    check(std::abs((relative.x*satellite.velocity.y-relative.y*satellite.velocity.x)-r*speed)<1e-7,"central-force angular momentum");
    Flight outside{{-10,100},{0,0}}; advance(outside,empty);
    check(outside.outcome==Outcome::Escaped,"initial state outside chart");
    const auto stopped=outside; advance(outside,empty);
    check(outside.position.x==stopped.position.x && outside.time==stopped.time,"terminal flights cannot advance");

    for(const auto& level:levels()) {
        Flight a{level.launch,level.solution}, b=a;
        const Trace predicted=predict(a,level);
        while(a.outcome==Outcome::Flying) advance(a,level);
        while(b.outcome==Outcome::Flying) advance(b,level);
        check(a.position.x==b.position.x && a.position.y==b.position.y && a.time==b.time,"deterministic replay");
        check(a.position.x==predicted.end.position.x && a.position.y==predicted.end.position.y && a.outcome==predicted.end.outcome,"preview matches actual integration");
        check(length(level.solution)<=maxLaunchSpeed,"solution within launch budget");
        check(a.outcome==Outcome::Arrived,level.name);
        check(starsFor(a,level)==3,"every level has a three-star route");
        int forgiving=0;
        for(double dx:{-1.0,0.0,1.0}) for(double dy:{-1.0,0.0,1.0})
            forgiving+=predict({level.launch,level.solution+Vec{dx,dy}},level).end.outcome==Outcome::Arrived;
        check(forgiving>=6,"verified routes tolerate small aiming errors");
        std::cout<<level.name<<": "<<static_cast<int>(a.outcome)<<" in "<<a.time<<" s; launch "<<level.solution.x<<", "<<level.solution.y<<'\n';
    }
    std::cout<<checks<<" checks, "<<failures<<" failures\n";
    return failures ? 1:0;
}
