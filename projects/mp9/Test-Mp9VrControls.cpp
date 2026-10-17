#include "vr/openxr_wii_remote.h"
#include <cassert>
#include <cstdio>
using namespace mkw::vr::wii_remote;
#ifndef MKW_GAME_MP9
#error Compile this test with the MP9 controller profile
#endif
int main() {
    HandInputs l{}, r{};
    assert(RemoteButtons(l,r)==0);
    l.primary=true; r.secondary=true;
    assert(RemoteButtons(l,r)==(kButtonOne|kButtonTwo));
    l={};r={};r.primary=true;r.trigger=.8f;l.trigger=.8f;
    assert(RemoteButtons(l,r)==(kButtonA|kButtonB|kButtonMinus));
    l={};r={};l.menu=true;assert(RemoteButtons(l,r)==kButtonPlus);
    l={};l.thumbstick_click=true;assert(RemoteButtons(l,r)==kButtonPlus);
    l={};l.squeeze=.8f;assert(RemoteButtons(l,r)==0);
    r.squeeze=.8f;assert(RemoteButtons(l,r)==kButtonHome);
    l={};r={};l.secondary=true;r.thumbstick_click=true;
    assert(RemoteButtons(l,r)==0); // Settings/recenter never leak into game buttons.
    l={};r={};l.stick_x=-.9f;assert(RemoteButtons(l,r)==kButtonLeft);
    l.stick_x=.9f;assert(RemoteButtons(l,r)==kButtonRight);
    l.stick_x=0;l.stick_y=.9f;assert(RemoteButtons(l,r)==kButtonUp);
    l.stick_y=-.9f;assert(RemoteButtons(l,r)==kButtonDown);
    l.stick_x=-.9f;assert(RemoteButtons(l,r)==(kButtonLeft|kButtonDown));
    l={};l.stick_x=.3f;l.stick_y=.3f;assert(RemoteButtons(l,r)==0);
    r.stick_y=.9f;assert(RemoteButtons(l,r)==0); // No old MKW 1/2 stick alias.
    puts("MP9 VR controls: buttons, 1+2 chord, D-pad, dead zone, reserved controls passed");
}
