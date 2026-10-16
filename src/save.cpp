#include "save.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS(char*, readSave, (), {
    try { return stringToNewUTF8(localStorage.getItem('slingshift.save.v1')||""); }
    catch(e) { return stringToNewUTF8(""); }
});
EM_JS(int, writeSave, (const char* text), {
    try { localStorage.setItem('slingshift.save.v1',UTF8ToString(text)); return 1; }
    catch(e) { return 0; }
});
#endif

namespace sling {
int Progress::unlocked() const {
    int n=0;
    while(n<11 && stars[n]>0) ++n;
    return n;
}
int Progress::totalStars() const { return std::accumulate(stars.begin(),stars.end(),0); }
std::string Progress::encode() const {
    std::ostringstream out;
    out<<"SLINGSHIFT 1\n"<<std::setprecision(10);
    for(int s:stars) out<<s<<' ';
    out<<'\n'; for(double t:best) out<<t<<' ';
    out<<'\n'<<volume<<' '<<ambience<<' '<<motion<<' '<<glow<<' '<<tutorialDone<<'\n';
    return out.str();
}
bool Progress::decode(const std::string& text) {
    std::istringstream in(text);
    std::string magic; int version=0;
    Progress candidate;
    if(!(in>>magic>>version) || magic!="SLINGSHIFT" || version!=1) return false;
    for(auto& s:candidate.stars) if(!(in>>s) || s<0 || s>3) return false;
    for(auto& t:candidate.best) if(!(in>>t) || !std::isfinite(t) || t<0 || t>30) return false;
    if(!(in>>candidate.volume>>candidate.ambience>>candidate.motion>>candidate.glow>>candidate.tutorialDone) ||
       !std::isfinite(candidate.volume) || candidate.volume<0 || candidate.volume>1) return false;
    *this=candidate;
    return true;
}
#ifndef __EMSCRIPTEN__
static std::filesystem::path savePath() {
    #ifdef _WIN32
    const char* base=std::getenv("LOCALAPPDATA");
    return base ? std::filesystem::path(base)/"Slingshift"/"progress.txt":std::filesystem::path("slingshift-progress.txt");
    #else
    const char* xdg=std::getenv("XDG_DATA_HOME");
    if(xdg && *xdg) return std::filesystem::path(xdg)/"slingshift"/"progress.txt";
    const char* home=std::getenv("HOME");
    return home ? std::filesystem::path(home)/".local"/"share"/"slingshift"/"progress.txt":std::filesystem::path("slingshift-progress.txt");
    #endif
}
#endif
void Progress::load() {
    #ifdef __EMSCRIPTEN__
    char* data=readSave(); if(data) { decode(data); std::free(data); }
    #else
    std::ifstream file(savePath());
    if(file) { std::ostringstream text; text<<file.rdbuf(); if(decode(text.str())) return; }
    auto backup=savePath(); backup+=".bak";
    std::ifstream old(backup);
    if(old) { std::ostringstream text; text<<old.rdbuf(); decode(text.str()); }
    #endif
}
bool Progress::save() const {
    #ifdef __EMSCRIPTEN__
    return writeSave(encode().c_str())!=0;
    #else
    const auto path=savePath();
    std::error_code ec;
    if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(),ec);
    if(ec) return false;
    auto temporary=path; temporary+=".tmp";
    { std::ofstream out(temporary,std::ios::trunc); out<<encode(); if(!out) return false; }
    // Windows rename does not replace existing files. Keep the last good file as a backup.
    auto backup=path; backup+=".bak";
    if(std::filesystem::exists(path,ec)) {
        std::filesystem::remove(backup,ec); ec.clear();
        std::filesystem::rename(path,backup,ec);
        if(ec) return false;
    }
    std::filesystem::rename(temporary,path,ec);
    if(ec) { std::error_code ignored; std::filesystem::rename(backup,path,ignored); return false; }
    return true;
    #endif
}
}
