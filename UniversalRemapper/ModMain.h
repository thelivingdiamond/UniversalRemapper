#pragma once
#include <ChairLoader/ModSDK/ChairloaderModBase.h>

class SActionInput;
class CActionMapAction;

class ModMain : public ChairloaderModBase
{
	using BaseClass = ChairloaderModBase;

	//! Fills in the DLL info during initialization.
	virtual void FillModInfo(ModDllInfo& info) override;

	//! Initializes function hooks before they are installed.
	virtual void InitHooks() override;

	//! Called during CSystem::Init, before any engine modules.
	//! Call order: TODO
	virtual void InitSystem(const ModInitInfo& initInfo, ModDllInfo& dllInfo) override;

	//! Called after CGame::Init
	//! Call order: TODO
	virtual void InitGame(bool isHotReloading) override;

	//! Called before CGame::Update to handle any GUI elements
	virtual void Draw() override;

	//! Before CGame::Update and before any entity updates
	//! Call order: TODO
	virtual void PreUpdate() override;

	//! After CGame::Update, after entities have been updated and after rendering
	//! commands have been issued. 
	//! Call order: TODO
	virtual void PostUpdate() override;

	//! Called before CGame::Shutdown.
	//! Call order: TODO
	virtual void ShutdownGame(bool isHotUnloading) override;

	//! Called before CSystem::Shutdown.
	//! Call order: TODO
	virtual void ShutdownSystem(bool isHotUnloading) override;

private:
    pugi::xml_document m_actionCategoryDoc;
    pugi::xml_node m_actionCategoryNode;
    bool m_bDraw;
    // Your code goes here

    bool rebindKeybind(std::string actionID, std::string actionMap, std::string currentBinding = "");

    bool addKeybind(std::string actionID, std::string actionMap);

    bool removeKeybind(std::string actionID, std::string actionMap, std::string currentBinding);

    bool rebindKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding = "");

    bool addKeybindGroup(std::string actionID, std::vector<std::string> actionMaps);

    bool removeKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding);

    void listenForInput();

    void clearInput();

    void drawActionMapAction(CActionMapAction *mappedAction, std::string actionID, std::string actionMap);
    void drawActionMapActionGroup(std::string actionID, std::vector<std::string> actionMaps, std::string primaryActionMap);

    EActionInputDevice m_device;

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

    bool m_bShowActionMapContextMenu = false;
    void setActionMapContextMenu(bool bShow, std::string actionID, std::string actionMap, std::string currentBinding = "", bool bGroup = false, std::vector<std::string> = {});
    void drawActionMapContextMenu();
};

extern ModMain* gMod;
