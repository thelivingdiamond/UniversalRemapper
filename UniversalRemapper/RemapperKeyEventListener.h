//
// Created by theli on 11/15/2022.
//

#ifndef UNIVERSALREMAPPER_REMAPPERKEYEVENTLISTENER_H
#define UNIVERSALREMAPPER_REMAPPERKEYEVENTLISTENER_H


class RemapperKeyEventListener : public IInputEventListener {
public:
    ~RemapperKeyEventListener() override;

    bool OnInputEvent(const SInputEvent &event) override;

    bool OnInputEventUI(const SInputEvent &event) override;

    int GetPriority() override;

    void startListening();

    bool isListening() const { return m_bListening; }

    bool isEventValid() const { return m_bValidEvent; }

    SInputEvent& getLastEvent() { m_bValidEvent = false; return m_lastEvent; }
private:
    bool m_bListening = false;
    bool m_bValidEvent = false;
    EInputDeviceType m_listeningDeviceType = static_cast<EInputDeviceType>(EInputDeviceType::eIDT_Keyboard |
                                                                           EInputDeviceType::eIDT_Mouse);
    SInputEvent m_lastEvent;
};


#endif //UNIVERSALREMAPPER_REMAPPERKEYEVENTLISTENER_H
