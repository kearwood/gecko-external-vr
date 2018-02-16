#ifndef GECKO_VR_H
#define GECKO_VR_H

#include "moz_external_vr.h"

bool gecko_vr_init();
void gecko_vr_shutdown();
void gecko_vr_push_state(const mozilla::gfx::VRDisplayInfo& aDisplayInfo);

#endif // GECKO_VR_H
