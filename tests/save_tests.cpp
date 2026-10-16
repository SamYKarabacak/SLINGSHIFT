#include "save.hpp"
#include <iostream>
#include <string>

int main() {
    int failures=0,checks=0;
    auto check=[&](bool ok,const char* message) { ++checks; if(!ok) { ++failures; std::cerr<<"FAIL: "<<message<<'\n'; } };
    sling::Progress p;
    check(p.unlocked()==0 && p.totalStars()==0,"new profile starts at first signal");
    p.stars[0]=3; p.stars[1]=1; p.best[0]=3.14; p.volume=.3f; p.motion=false; p.tutorialDone=true;
    sling::Progress restored;
    check(restored.decode(p.encode()),"saved profile round trip");
    check(restored.stars==p.stars && restored.best==p.best && restored.volume==p.volume && !restored.motion && restored.tutorialDone,"round trip retains scores and preferences");
    check(restored.unlocked()==2 && restored.totalStars()==4,"progression follows completed signals");
    const std::string original=restored.encode();
    check(!restored.decode("SLINGSHIFT 2\n"),"unknown save version rejected");
    check(!restored.decode("SLINGSHIFT 1\n3 2"),"truncated save rejected");
    auto corrupt=p.encode(); corrupt.replace(corrupt.find('\n')+1,1,"9");
    check(!restored.decode(corrupt),"out-of-range score rejected");
    check(restored.encode()==original,"failed load preserves existing profile");
    p.stars.fill(3); check(p.unlocked()==11 && p.totalStars()==36,"complete campaign remains in range");
    std::cout<<checks<<" save checks, "<<failures<<" failures\n";
    return failures?1:0;
}
