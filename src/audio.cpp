#include "audio.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace sling {
static Sound synth(int kind) {
    constexpr int rate=22050;
    const float duration=kind==5?8.f:kind==4?1.2f:kind==3?.6f:.27f;
    const auto frames=static_cast<unsigned int>(duration*rate);
    std::vector<std::int16_t> pcm(frames);
    std::uint32_t noise=731;
    double phase=0;
    for(unsigned int i=0;i<frames;++i) {
        const float t=i/static_cast<float>(rate),u=t/duration;
        noise=noise*1664525u+1013904223u;
        const float n=(noise>>8)/8388608.f-1;
        float value=0;
        if(kind==5) {
            const float env=std::pow(std::sin(3.14159265f*u),2.f);
            value=(std::sin(t*6.2831853f*110)+.6f*std::sin(t*6.2831853f*164.8138f)+.4f*std::sin(t*6.2831853f*220))*.07f*env;
        } else if(kind==3) {
            value=(n*.7f+std::sin(t*6.2831853f*54)*.3f)*std::pow(1-u,3.f)*.6f;
        } else if(kind==4) {
            const float f=t<.23f?440:t<.46f?554.365f:659.255f;
            phase+=6.2831853*f/rate;
            value=(std::sin(phase)+std::sin(phase*2)*.2f)*std::min(1.f,t*30)*std::pow(1-u,1.8f)*.25f;
        } else {
            const float f=kind==0?740-400*u:kind==1?150+950*u:350+300*u;
            phase+=6.2831853*f/rate;
            value=(std::sin(phase)*.55f+n*(kind==0?.03f:.22f))*std::sin(3.14159265f*u)*std::pow(1-u,1.2f)*.4f;
        }
        pcm[i]=static_cast<std::int16_t>(std::clamp(value,-1.f,1.f)*32760);
    }
    Wave wave{frames,rate,16,1,pcm.data()};
    return LoadSoundFromWave(wave);
}
void Audio::init() {
    if(ready) return;
    InitAudioDevice();
    ready=IsAudioDeviceReady();
    if(!ready) return;
    for(int i=0;i<5;++i) sounds[i]=synth(i);
    bed=synth(5);
}
void Audio::update(float volume,bool ambience) {
    if(!ready) return;
    SetMasterVolume(std::clamp(volume,0.f,1.f));
    if(ambience && volume>0) { if(!IsSoundPlaying(bed)) PlaySound(bed); }
    else StopSound(bed);
}
void Audio::play(Cue cue) { if(ready) PlaySound(sounds[static_cast<int>(cue)]); }
void Audio::close() {
    if(!ready) return;
    for(auto sound:sounds) UnloadSound(sound);
    UnloadSound(bed); CloseAudioDevice(); ready=false;
}
}
