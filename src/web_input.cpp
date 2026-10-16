#include "web_input.hpp"
#include <emscripten.h>

// Keep pointer edges until C++ consumes them, including taps shorter than one frame.
// Pointer capture also keeps a slingshot drag alive outside the canvas bounds.
EM_JS(void, installPointerEvents, (), {
    const canvas=Module.canvas;
    const p=window.__slingPointer={x:0,y:0,down:false,pressed:false,released:false,cancelled:false,id:null};
    const position=function(e){const r=canvas.getBoundingClientRect();p.x=e.clientX-r.left;p.y=e.clientY-r.top;};
    canvas.addEventListener('pointerdown',function(e){
        if(p.id!==null && p.id!==e.pointerId)return;
        if(e.pointerType==='mouse' && e.button!==0)return;
        position(e);p.id=e.pointerId;p.down=true;p.pressed=true;
        try{canvas.setPointerCapture(e.pointerId);}catch(error){}
        canvas.focus();e.preventDefault();
    },{passive:false});
    canvas.addEventListener('pointermove',function(e){
        if(p.id!==null && p.id!==e.pointerId)return;
        position(e);
    });
    canvas.addEventListener('pointerup',function(e){
        if(p.id!==e.pointerId)return;
        position(e);p.down=false;p.released=true;p.id=null;e.preventDefault();
    },{passive:false});
    const cancel=function(){if(p.down){p.down=false;p.cancelled=true;p.id=null;}};
    canvas.addEventListener('pointercancel',cancel);
    canvas.addEventListener('lostpointercapture',cancel);
    window.addEventListener('blur',cancel);
});
EM_JS(int, pointerFlags, (), {
    const p=window.__slingPointer;
    const bits=(p.down?1:0)|(p.pressed?2:0)|(p.released?4:0)|(p.cancelled?8:0);
    p.pressed=p.released=p.cancelled=false;return bits;
});
EM_JS(double, pointerX, (), {return window.__slingPointer.x*Module.canvas.width/Module.canvas.clientWidth;});
EM_JS(double, pointerY, (), {return window.__slingPointer.y*Module.canvas.height/Module.canvas.clientHeight;});

namespace sling {
void installWebInput(){installPointerEvents();}
WebPointer pollWebPointer(){const int flags=pointerFlags();return {static_cast<float>(pointerX()),static_cast<float>(pointerY()),(flags&1)!=0,(flags&2)!=0,(flags&4)!=0,(flags&8)!=0};}
}
