#include "physics.hpp"

#include <algorithm>
#include <limits>

namespace sling {

Vec acceleration(Vec p, std::span<const Body> bodies) {
    Vec a{};
    for (const auto& body:bodies) {
        const Vec d=body.position-p;
        const double r2=dot(d,d)+softening*softening;
        a+=d*(body.mu/(r2*std::sqrt(r2)));
    }
    return a;
}

double potential(Vec p, std::span<const Body> bodies) {
    double value=0;
    for (const auto& body:bodies) {
        const Vec d=body.position-p;
        value-=body.mu/std::sqrt(dot(d,d)+softening*softening);
    }
    return value;
}

double segmentCircle(Vec a, Vec b, Vec centre, double radius) {
    const Vec f=a-centre, d=b-a;
    const double c=dot(f,f)-radius*radius;
    if (c<=0) return 0;
    const double aa=dot(d,d);
    if (aa<1e-20) return std::numeric_limits<double>::infinity();
    const double bb=dot(f,d);
    const double discriminant=bb*bb-aa*c;
    if (discriminant<0) return std::numeric_limits<double>::infinity();
    const double t=(-bb-std::sqrt(discriminant))/aa;
    return t>=0 && t<=1 ? t : std::numeric_limits<double>::infinity();
}

void advance(Flight& s, const Level& level, double dt) {
    if (s.outcome!=Outcome::Flying) return;
    if (!finite(s.position) || !finite(s.velocity) || !std::isfinite(s.time) ||
        !std::isfinite(dt) || dt<=0 || dt>0.1 || s.time<0 || length(s.velocity)>5000) {
        s.outcome=Outcome::Invalid;
        return;
    }
    if(s.position.x<0 || s.position.x>worldWidth || s.position.y<0 || s.position.y>worldHeight) {
        s.outcome=Outcome::Escaped;
        return;
    }
    const Vec a=acceleration(s.position,level.bodies);
    const Vec next=s.position+s.velocity*dt+a*(0.5*dt*dt);
    const Vec v=s.velocity+(a+acceleration(next,level.bodies))*(0.5*dt);
    if (!finite(next) || !finite(v)) { s.outcome=Outcome::Invalid; return; }

    double first=segmentCircle(s.position,next,level.gate,level.gateRadius);
    Outcome result=std::isfinite(first) ? Outcome::Arrived : Outcome::Flying;
    int hit=-1;
    for (int i=0;i<static_cast<int>(level.bodies.size());++i) {
        const auto& b=level.bodies[i];
        const double t=segmentCircle(s.position,next,b.position,b.radius+shipRadius);
        if (t<=first && std::isfinite(t)) { first=t; result=Outcome::Impact; hit=i; }
    }
    // Swept boundaries prevent fast ships from jumping beyond the chart.
    const Vec delta=next-s.position;
    double exit=std::numeric_limits<double>::infinity();
    if (next.x<0 && delta.x<0) exit=std::min(exit,-s.position.x/delta.x);
    if (next.x>worldWidth && delta.x>0) exit=std::min(exit,(worldWidth-s.position.x)/delta.x);
    if (next.y<0 && delta.y<0) exit=std::min(exit,-s.position.y/delta.y);
    if (next.y>worldHeight && delta.y>0) exit=std::min(exit,(worldHeight-s.position.y)/delta.y);
    if (exit<first && exit>=0) { first=exit; result=Outcome::Escaped; hit=-1; }
    if (result!=Outcome::Flying) {
        s.position=s.position+delta*first;
        s.velocity=s.velocity+(v-s.velocity)*first;
        s.time+=dt*first;
        s.outcome=result;
        s.hitBody=hit;
        return;
    }
    s.position=next;
    s.velocity=v;
    s.time+=dt;
    if (s.time>=timeLimit) s.outcome=Outcome::Expired;
}

bool correct(Flight& s, Vec impulse) {
    if (s.outcome!=Outcome::Flying || s.corrected || !finite(impulse) || length(impulse)<1) return false;
    s.velocity+=limited(impulse,maxCorrection);
    s.corrected=true;
    return true;
}

Trace predict(Flight ship, const Level& level, int stride) {
    Trace result;
    stride=std::max(stride,1);
    result.points.reserve(2800/stride+2);
    result.points.push_back(ship.position);
    for (int i=0;i<static_cast<int>(timeLimit/stepSeconds)+2 && ship.outcome==Outcome::Flying;++i) {
        advance(ship,level);
        if (i%stride==0 || ship.outcome!=Outcome::Flying) result.points.push_back(ship.position);
    }
    result.end=ship;
    return result;
}

int starsFor(const Flight& s, const Level& level) {
    if (s.outcome!=Outcome::Arrived) return 0;
    return 1+(!s.corrected ? 1:0)+(s.time<=level.par ? 1:0);
}
} // namespace sling
