#pragma once
#include <ChairLoader/ModSDK/ChairloaderModBase.h>

class SActionInput;

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
    void RemapKey(CCryName ActionID, std::string ActionMap, std::string keyName);
    bool m_bWaitingForInput = false;
    IActionMap* m_pActionMapToRemap = nullptr;
    IActionMapAction* m_pActionToRemap = nullptr;
    SActionInput* m_pActionInputToRemap = nullptr;
};

extern ModMain* gMod;
