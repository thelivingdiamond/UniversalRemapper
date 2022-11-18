#include "ModMain.h"
#include "RemapperKeyEventListener.h"
#include <Prey/CryGame/IGameFramework.h>
#include <Prey/CryAction/ActionMapManager.h>
#include <Prey/CryAction/ActionFilter.h>
#include <pugixml.hpp>

ModMain* gMod = nullptr;

#ifdef EXAMPLE
// You can define PreyDll.dll function using PreyFunction
// auto fnFuncName = PreyFunction<void(T funcArgs)>(functionOffset);
// Many functions are defined as static members with F prefix:
//  ArkPlayer.h
//     static inline auto FHasAbility = PreyFunction<bool(ArkPlayer* _this, uint64_t _abilityID)>(0x1550330);
static auto fn_ArkPlayer_HasAbility = PreyFunction<bool(ArkPlayer* _this, uint64_t _abilityID)>(0x1550330);

// Use MakeHook() method of PreyFunction to create a hook.
// See ModMain::InitHooks for more.
static auto s_hook_ArkPlayer_HasAbility = fn_ArkPlayer_HasAbility.MakeHook();

// This function will be called in place of ArkPlayer::HasAbility.
// See ModMain::InitHooks for more.
static bool ArkPlayer_HasAbility_Hook(ArkPlayer* _this, uint64_t _abilityID)
{
	// NOTE: This particular method of ability unlocking is unreliable.
	// NOTE: It's only used as an example of function hooking.

	if (_abilityID % 2 == 0)
	{
		// Player always has abilities whose ID is even.
		return true;
	}

	// Use InvokeOrig of PreyFunctionHook to call the original (non-hooked) function.
	return s_hook_ArkPlayer_HasAbility.InvokeOrig(_this, _abilityID);
}

#endif

void ModMain::FillModInfo(ModDllInfo& info)
{
	info.thisStructSize = sizeof(ModDllInfo);
	info.modName = "thelivingdiamond.UniversalRemapper"; // CHANGE ME
	info.supportsHotReload = true; // TODO: Add comment/wiki link
}

void ModMain::InitHooks()
{
#ifdef EXAMPLE
	// Functions hooks are intalled early into mod loading process,
	// before any engine subsystem is initialized.
	// But for hook to succeed PreyFunctionHook needs to know the hook function
	// (that will be called in place of original one)
	// That's why ModMain::InitHooks exists.
	// Call SetHookFunc from here.
	s_hook_ArkPlayer_HasAbility.SetHookFunc(&ArkPlayer_HasAbility_Hook);
#endif
}
static RemapperKeyEventListener listener;
void ModMain::InitSystem(const ModInitInfo& initInfo, ModDllInfo& dllInfo)
{
	BaseClass::InitSystem(initInfo, dllInfo);
	// Your code goes here
}

void ModMain::InitGame(bool isHotReloading)
{
	BaseClass::InitGame(isHotReloading);
    gEnv->pInput->AddEventListener(&listener);
	// Your code goes here
}

void dumpActionMaps() {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("ActionMaps");
    for(auto& actionMap : ActionMapManager->m_actionMaps){
//        auto actionMapNode = root.append_child("ActionMap");
//        actionMapNode.append_attribute("name") = actionMap.first.c_str();
        auto iActionMap = (IActionMap*)actionMap.second;
        for(int i = 0; i < iActionMap->GetActionsCount(); i++){
            auto action = iActionMap->GetAction(i);
            auto actionNode = root.append_child("Action");
            actionNode.append_attribute("ActionMap") = actionMap.first.c_str();
            actionNode.append_attribute("ActionID") = action->GetActionId().c_str();
//            for(int j = 0; j < action->GetNumActionInputs(); j++){
//                auto actionInput = action->GetActionInput(j);
//                auto actionInputNode = actionNode.append_child("ActionInput");
//                actionInputNode.append_attribute("Input") = actionInput->input.c_str();
//                actionInputNode.append_attribute("InputDevice") = actionInput->inputDevice;
//                actionInputNode.append_attribute("DefaultInput") = actionInput->defaultInput;
//            }
        }
    }
    doc.save_file("ActionDumpNoBinds.xml");
};

void ModMain::Draw()
{
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("Universal Remapper")){
            ImGui::MenuItem("Show Remapper", nullptr, &m_bDraw);
            if(ImGui::MenuItem("Dump ActionMaps")){
                dumpActionMaps();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(m_bDraw) {
        if (ImGui::Begin("Universal Remapper")) {
            auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
            if (ImGui::BeginTabBar("Input Methods Tab Bar")) {
                if (ImGui::BeginTabItem("Keyboard/Mouse")) {
                    if (ImGui::BeginTabBar("Keyboard/Mouse Maps")) {
                        for (auto &ActionMap: ActionMapManager->m_actionMaps) {
                            if (ImGui::BeginTabItem(ActionMap.first.c_str())) {
                                if (ImGui::BeginTable(("Keybinds##" + ActionMap.first).c_str(), 2,
                                                      ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                                                      ImVec2(0, 0))) {
                                    ImGui::TableSetupColumn("Action");
                                    ImGui::TableSetupColumn("Keybind");
                                    ImGui::TableSetupScrollFreeze(0, 1);
                                    ImGui::TableHeadersRow();
                                    auto mapPtr = (IActionMap *) ActionMap.second;
                                    for (int i = 0; i < mapPtr->GetActionsCount(); i++) {
                                        auto action = mapPtr->GetAction(i);
                                        auto input = action->GetActionInput(EActionInputDevice::eAID_KeyboardMouse, 0);
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        std::string actionText;
                                        if(m_pActionToRemap == action &&
                                           mapPtr == m_pActionMapToRemap &&
                                           m_pActionInputToRemap == input &&
                                           m_bWaitingForInput){
                                            actionText = ">   <";
                                        } else {
                                            if(input != nullptr) {
                                                actionText = input->input.c_str();
                                            } else {
                                                actionText = "None";
                                            }
                                        }
                                        if (ImGui::Selectable(action->GetActionId().c_str(),
                                                              m_pActionToRemap == action &&
                                                              mapPtr == m_pActionMapToRemap,
                                                              ImGuiSelectableFlags_SpanAllColumns)) {
                                            listener.startListening();
                                            m_bWaitingForInput = true;
                                            m_pActionToRemap = action;
                                            m_pActionMapToRemap = mapPtr;
                                            m_pActionInputToRemap = (SActionInput *) input;
                                        }
                                        // on right click clear a binding
                                        //TODO: move to a right click context menu with more options
                                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                                            if (m_pActionToRemap != nullptr) {
                                                m_pActionToRemap = nullptr;
                                                m_pActionMapToRemap = nullptr;
                                                m_pActionInputToRemap = nullptr;
                                            } else {
                                                if(input != nullptr) {
                                                    mapPtr->RemoveActionInput(action->GetActionId(),
                                                                              input->input.c_str());
                                                }
                                            }
                                        }
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%s", actionText.c_str());
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::EndTabItem();
                            }
                        }
                        ImGui::EndTabBar();
                    }
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Action Filters")){
                    if(ImGui::BeginTable("Action Filters", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders)){
                        ImGui::TableSetupColumn("Action");
                        ImGui::TableSetupColumn("Filter");
                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableHeadersRow();
                        for(auto& actionFilter: ActionMapManager->m_actionFilters){
                            auto filter = actionFilter.second;
                            if(filter != nullptr) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", actionFilter.first.c_str());
                                ImGui::Text("type: %s", filter->m_type ? "ActionFail" : "ActionPass");
                                ImGui::Text("enabled: %d", filter->m_enabled);
                                ImGui::TableNextColumn();
                                for(auto& filterAction: filter->m_filterActions){
                                    ImGui::Text("%s", filterAction.c_str());
                                }
                            }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}

void ModMain::PreUpdate()
{
	// Your code goes here
}

void ModMain::PostUpdate()
{
    // Your code goes here
    if(m_bWaitingForInput){
        if(listener.isEventValid()){
            auto event = listener.getLastEvent();
            if(event.keyId == EKeyId::eKI_Escape){
                // abort this operation
                m_bWaitingForInput = false;
                m_pActionToRemap = nullptr;
                m_pActionMapToRemap = nullptr;
                m_pActionInputToRemap = nullptr;
                return;
            }
            auto ActionMapManager = (CActionMapManager*)gCL->cl->GetFramework()->GetIActionMapManager();
            bool success = false;
            if(m_pActionInputToRemap != nullptr) {
                CryLog("Rebinding");
                success = m_pActionMapToRemap->ReBindActionInput(m_pActionToRemap->GetActionId(), event.keyName.key,
                                                                 EActionInputDevice::eAID_KeyboardMouse, 0);
            } else {
                CryLog("Adding");
                SActionInput input;
                input.input = event.keyName.key;
                input.inputDevice = EActionInputDevice::eAID_KeyboardMouse;
                input.inputCRC = CCrc32::ComputeLowercase(input.input.c_str());
                success = m_pActionMapToRemap->AddAndBindActionInput(m_pActionToRemap->GetActionId(), input);
            }
            if(success){
                CryLog("remapped successfully");
            } else {
                CryError("remapping failed");
            }
            m_bWaitingForInput = false;
            m_pActionToRemap = nullptr;
            m_pActionMapToRemap = nullptr;
            m_pActionInputToRemap = nullptr;
        }
    }
}

void ModMain::ShutdownGame(bool isHotUnloading)
{
	// Your code goes here
	BaseClass::ShutdownGame(isHotUnloading);
    gEnv->pInput->RemoveEventListener(&listener);
}

void ModMain::ShutdownSystem(bool isHotUnloading)
{
	// Your code goes here
	BaseClass::ShutdownSystem(isHotUnloading);
}


extern "C" DLL_EXPORT IChairloaderMod* ClMod_Initialize()
{
	CRY_ASSERT(!gMod);
	gMod = new ModMain();
	return gMod;
}

extern "C" DLL_EXPORT void ClMod_Shutdown()
{
	CRY_ASSERT(gMod);
	delete gMod;
	gMod = nullptr;
}

// Validate that declarations haven't changed
static_assert(std::is_same_v<decltype(ClMod_Initialize), IChairloaderMod::ProcInitialize>);
static_assert(std::is_same_v<decltype(ClMod_Shutdown), IChairloaderMod::ProcShutdown>);
