#pragma once
#include <ChairLoader/ModSDK/ChairloaderModBase.h>
#include "IUniversalRemapper.h"

class SActionInput;
class CActionMapAction;

class ModMain : public ChairloaderModBase, public IUniversalRemapper
{
    using BaseClass = ChairloaderModBase;

    //! Fills in the DLL info during initialization.
    virtual void FillModInfo(ModDllInfoEx& info) override;

public:
    void DoNothing() override;

private:
    //! Retrieves an interface for the mod.
    void* QueryInterface(const char *ifaceName) override;

    //! Initializes function hooks before they are installed.
    virtual void InitHooks() override;

    //! Called during CSystem::Init, before any engine modules.
    //! Call order: TODO
    virtual void InitSystem(const ModInitInfo& initInfo, ModDllInfo& dllInfo) override;

    //! Called after CSystem::Init, after all engine modules and mods have been initialized. Allows your mod to get interfaces from other mods.
    void Connect(const std::vector<IChairloaderMod *> &mods) override;

    //! Called after CGame::Init
    //! Call order: TODO
    virtual void InitGame(bool isHotReloading) override;

    //! Called before CGame::Shutdown.
    //! Call order: TODO
    virtual void ShutdownGame(bool isHotUnloading) override;

    //! Called before CSystem::Shutdown.
    //! Call order: TODO
    virtual void ShutdownSystem(bool isHotUnloading) override;

    //! Called just before MainUpdate to draw GUI. Only called when GUI is visible.
    virtual void Draw() override;

    //! Earliest point of update in a frame, before CGame::Update. The timer still tracks time for the previous frame.
    // virtual void UpdateBeforeSystem(unsigned updateFlags) override;

    //! Called before physics is updated for the new frame, best point for queuing physics jobs.
    //! This is like FixedUpdate() in Unity (but not FPS-independent). Use gEnv->pTimer->GetFrameTime() for time delta.
    // virtual void UpdateBeforePhysics(unsigned updateFlags) override;

    //! Called after entities have been updated but before FlowSystem and FlashUI.
    //! This is the main update where most game logic is expected to occur.
    //! Should be preferred if you don't need any special behavior.
    virtual void MainUpdate(unsigned updateFlags) override;

private:
    pugi::xml_document m_actionCategoryDoc;
    pugi::xml_node m_actionCategoryNode;
    bool m_bDraw;
    // Your code goes here

    bool RebindKeybindListen(std::string actionID, std::string actionMap, std::string currentBinding = "");

    bool AddKeybindListen(std::string actionID, std::string actionMap);

    bool RemoveKeybindListen(std::string actionID, std::string actionMap, std::string currentBinding);

    bool rebindKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding, std::string primaryMap);

    bool addKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string primaryMap);

    bool removeKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding);

    void listenForInput();

    void clearInput();

    void drawActionMapAction(CActionMapAction *mappedAction, std::string actionID, std::string actionMap);
    void drawActionMapActionGroup(std::string actionID, std::vector<std::string> actionMaps, std::string primaryActionMap);

    void saveActionMapsToXML();
    void loadActionMapsFromXML();

    SActionInput getDefaultActionInput(std::string actionID, std::string actionMap, EActionInputDevice device);

    std::set<std::string> m_boundActionMaps;
    std::set<std::string> m_boundActions;

    EActionInputDevice m_device;

    const fs::path m_actionMapFile = "./Mods/config/UniversalRemapper/ActionMaps.xml";

    enum class KeybindAction
    {
        None,
        Add,
        Remove,
        Rebind,
    };
    KeybindAction m_keybindAction = KeybindAction::None;
    bool m_bListeningForInput = false;
    bool m_bGroup = false;
    std::string m_actionID;
    std::string m_actionMap;
    std::vector<std::string> m_actionMaps;
    std::string m_currentBinding;
    std::string m_newBinding;

    bool RebindKeybind(std::string actionID, std::string actionMap, std::string currentBinding, std::string newBinding);

    bool AddKeybind(std::string actionID, std::string actionMap, SActionInput &newInput);

    bool RemoveKeybind(std::string actionID, std::string actionMap, std::string currentBinding);

    bool RebindKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding, std::string newBinding, std::string primaryMap);

    bool AddKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, SActionInput &newInput, std::string primaryMap);

    bool RemoveKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding);

    bool m_bShowActionMapContextMenu = false;
    int m_bShowDetailedInfo = false;
    bool showDetailedInfo(){
        return (bool)m_bShowDetailedInfo;
    }
    void setActionMapContextMenu(bool bShow, std::string actionID, std::string actionMap, std::string currentBinding = "", bool bGroup = false, std::vector<std::string> = {});
    void drawActionMapContextMenu();

    void resetActionMapsToDefaults();
};

extern ModMain* gMod;
