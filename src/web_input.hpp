#pragma once

namespace sling {
struct WebPointer {
    float x,y;
    bool down,pressed,released,cancelled;
};
void installWebInput();
WebPointer pollWebPointer();
}
