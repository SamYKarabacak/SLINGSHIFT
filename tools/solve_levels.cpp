#include "physics.hpp"
#include <iomanip>
#include <iostream>
#include <random>
#include <cstdlib>

// Offline design aid. Shipped solutions are constants, never searched at runtime.
int main(int argc,char** argv) {
    std::mt19937 rng(0x51A6);
    std::uniform_real_distribution<double> horizontal(25,420), vertical(-330,330);
    int index=0;
    for (auto level:sling::levels()) {
        if(argc>1 && index++!=std::atoi(argv[1])-1) continue;
        level.gateRadius=2;
        double best=1e9;
        sling::Vec route{};
        int hits=0;
        for (int trial=0;trial<150000;++trial) {
            sling::Vec velocity{horizontal(rng),vertical(rng)};
            if (sling::length(velocity)>sling::maxLaunchSpeed) continue;
            sling::Flight s{level.launch,velocity};
            while(s.outcome==sling::Outcome::Flying) sling::advance(s,level);
            if (s.outcome!=sling::Outcome::Arrived) continue;
            ++hits;
            const double cost=std::abs(sling::length(velocity)-290)+std::abs(velocity.y)*0.06;
            if(cost<best) { best=cost; route=velocity; }
        }
        std::cout<<level.name<<" | "<<std::fixed<<std::setprecision(6)
                 <<route.x<<", "<<route.y<<" | centre routes: "<<hits<<std::endl;
    }
}
