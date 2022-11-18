//
// Created by theli on 11/15/2022.
//

#include "RemapperKeyEventListener.h"

RemapperKeyEventListener::~RemapperKeyEventListener() {

}

bool RemapperKeyEventListener::OnInputEvent(const SInputEvent &event) {
//    CryLog("%s", event.keyName.key);
//TODO: support mouse buttons and scrollwheel
//TODO: support controllers
    if (m_bListening && (event.deviceType == eIDT_Mouse || event.deviceType == eIDT_Keyboard)) {
        // skip mouse movement and mouse1 and mouse2
        //|| event.keyId == eKI_Mouse1 || event.keyId == eKI_Mouse2
        if(event.keyId == eKI_MouseX || event.keyId == eKI_MouseY || event.keyId == eKI_MouseZ || event.keyId == eKI_SYS_Commit) {
            return false;
        }
        m_lastEvent = event;
        m_bListening = false;
        m_bValidEvent = true;
        return true;
    }
    return false;
}

bool RemapperKeyEventListener::OnInputEventUI(const SInputEvent &event) {
//    CryLog("%s: %s", "OnInputEventUI", event.keyName.key);
    return false;
}

int RemapperKeyEventListener::GetPriority() {
    return 0;
}

void RemapperKeyEventListener::startListening() {
    m_bListening = true;
}
