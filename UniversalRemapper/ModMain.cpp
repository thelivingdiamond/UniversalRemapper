#include "ModMain.h"
#include "RemapperKeyEventListener.h"
#include <Prey/CryGame/IGameFramework.h>
#include <Prey/CryAction/ActionMapManager.h>
#include <Prey/CryAction/ActionFilter.h>
#include <pugixml.hpp>
#include <Chairloader.ImGui/imgui.h>
#include <Prey/CryAction/ActionMap.h>
#include <Prey/CryGame/Game.h>
#include <Prey/GameDll/ark/ArkGame.h>
#include <Prey/GameDll/ark/ui/ArkOptionMenu.h>

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

#define MOD_NAME "thelivingdiamond.UniversalRemapper"

static bool firstTime = true;
static auto s_hook_ArkOptionMenu_UpdateInputPrompts = ArkOptionMenu::FUpdateInputPrompts.MakeHook();
static void ArkOptionMenu_UpdateInputPrompts_Hook(ArkOptionMenu* _this){
//    // do nothing, see what happens
//    auto something = _this->m_pCurrentSubPage;
//    auto label = something->m_Label;
//    if(something!= nullptr) {
//        auto something2 = _this->m_pCurrentSubPage->m_Label;
//        label.clear();
//    }
    if(!firstTime)
        return s_hook_ArkOptionMenu_UpdateInputPrompts.InvokeOrig(_this);
    firstTime = false;
    for(auto& page: _this->m_optionLayout.m_Pages){
        if(page.m_Label == "@ui_ControlsSettingsTitle"){
            for(auto &subPage: page.m_SubPages){
                if(subPage.m_Label == "@ui_KeyboardMapping"){
                    for(auto & attribute: subPage.m_Attributes){
//                        CryLog("Attribute: {}", attribute.m_Label);
                        attribute.m_Hidden = true;
                    }
                    subPage.m_Warning = "Warning, this menu will not function properly while Universal Remapper is installed. Please use Universal Remapper instead.";
                }
            }
        }
    }
}



void ModMain::FillModInfo(ModDllInfoEx& info)
{
    info.modName = "thelivingdiamond.UniversalRemapper"; // CHANGE ME
    info.logTag = "UniversalRemapper"; // CHANGE ME
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
    s_hook_ArkOptionMenu_UpdateInputPrompts.SetHookFunc(&ArkOptionMenu_UpdateInputPrompts_Hook);
}
static RemapperKeyEventListener listener;
void ModMain::InitSystem(const ModInitInfo& initInfo, ModDllInfo& dllInfo)
{
	BaseClass::InitSystem(initInfo, dllInfo);
	// Your code goes here
    auto success = m_actionCategoryDoc.load_file("Mods/" MOD_NAME "/ActionsByCategories.xml");
    if (!success)
    {
        CryError("Failed to load ActionsByCategories.xml");
        return;
    }
    m_actionCategoryNode = m_actionCategoryDoc.first_child();
}

void *ModMain::QueryInterface(const char *ifaceName) {
#ifdef EXAMPLE
    // this is used to return an interface for your mod, if available.
    // Your mod class should inherit from the interface class. i.e: class ModMain : public ChairloaderModBase, public IExampleMod {
    // Then you can return the interface pointer here.
    if (!strcmp(ifaceName, "ExampleMod"))
        return static_cast<IExampleMod*>(this);
    // If you have multiple interfaces, you can return as many as you want for even potentially different objects.
    // if you don't have an interface, just return nullptr.
#endif
    if (!strcmp(ifaceName, "IUniversalRemapper"))
        return static_cast<IUniversalRemapper*>(this);
    return nullptr;
}

void ModMain::Connect(const std::vector<IChairloaderMod *> &mods) {
#ifdef EXAMPLE
    // Example of how to get a mod interface from the list of mods
    IOtherMod* otherMod = nullptr;
    for (auto & mod: mods) {
        otherMod = mod->QueryInterface("IOtherMod001"); // the interface name is defined in the other mod
        if (otherMod) {
            break;
        }
    }

    // do something with otherMod
#endif
    IUniversalRemapper* remapper = nullptr;
    CryLog("Connecting!");
    for(auto & mod: mods){
        remapper = static_cast<IUniversalRemapper*>(mod->QueryInterface("IUniversalRemapper"));
        if(remapper){
            break;
        }
    }
    if(remapper){
        CryLog("Found remapper!");
        remapper->DoNothing();
    }
}


void ModMain::InitGame(bool isHotReloading)
{
	BaseClass::InitGame(isHotReloading);
    gEnv->pInput->AddEventListener(&listener);
    REGISTER_CVAR2("universal_remapper_show_detailed_info", &m_bShowDetailedInfo, m_bShowDetailedInfo, VF_DUMPTODISK, "Show detailed info in the remapper");
    loadActionMapsFromXML();
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
        }
    }
    doc.save_file("ActionDumpNoBinds.xml");
};


bool ModMain::RebindKeybindListen(std::string actionID, std::string actionMap, std::string currentBinding){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap {} not found", actionMap.c_str());
        return false;
    }
//    auto action = actionMapPtr->GetAction(CCryName(actionID.c_str()));
//    if(action == nullptr){
//        CryError("Action %s not found in ActionMap %s", actionID.c_str(), actionMap.c_str());
//        return false;
//    }
    if(currentBinding.empty()){
        m_bListeningForInput = true;
        m_actionMap = actionMap;
        m_actionID = actionID;
        m_currentBinding = currentBinding;
        m_keybindAction = KeybindAction::Add;
        listener.startListening();
    } else {
        m_bListeningForInput = true;
        m_actionMap = actionMap;
        m_actionID = actionID;
        m_currentBinding = currentBinding;
        m_keybindAction = KeybindAction::Rebind;
        listener.startListening();
    }
    return true;
}

bool ModMain::AddKeybindListen(std::string actionID, std::string actionMap){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap {} not found", actionMap.c_str());
        return false;
    }
    m_bListeningForInput = true;
    m_actionMap = actionMap;
    m_actionID = actionID;
    m_currentBinding.clear();
    m_keybindAction = KeybindAction::Add;
    listener.startListening();
    return true;
}
bool ModMain::RemoveKeybindListen(std::string actionID, std::string actionMap, std::string currentBinding){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap {} not found", actionMap.c_str());
        return false;
    }
   /* auto action = actionMapPtr->GetAction(CCryName(actionID.c_str()));
    if(action == nullptr){
        CryError("Action %s not found in ActionMap %s", actionID.c_str(), actionMap.c_str());
        return false;
    }*/
//   actionMapPtr->RemoveActionInput(CCryName(actionID.c_str()), currentBinding.c_str());
        m_bListeningForInput = true;
        m_actionMap = actionMap;
        m_actionID = actionID;
        m_currentBinding = currentBinding;
        m_keybindAction = KeybindAction::Remove;
    return true;
}

static auto windowFlags = ImGuiWindowFlags_NoNavInputs;


void ModMain::Draw()
{
    if(ImGui::BeginMainMenuBar()){
        if(ImGui::BeginMenu("Universal Remapper")){
            ImGui::MenuItem("Show Remapper", nullptr, &m_bDraw);
#ifdef  DEBUG_BUILD
            if(ImGui::MenuItem("Dump ActionMaps")){
                dumpActionMaps();
            }
#endif
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    if(m_bDraw) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        if (ImGui::Begin("Universal Remapper", &m_bDraw, windowFlags)) {
            auto ActionMapManager = (CActionMapManager*) gCL->cl->GetFramework()->GetIActionMapManager();
            static const char* deviceNames[] = {"Unknown", "Keyboard/Mouse", "Xbox Controller", "PS4 Controller", "Steam Controller", "All"};
            std::map<std::string, EActionInputDevice> deviceMap = {
                    {"Unknown", eAID_Unknown},
                    {"Keyboard/Mouse", eAID_KeyboardMouse},
                    {"Xbox Controller", eAID_XboxPad},
                    {"PS4 Controller", eAID_PS4Pad},
                    {"Steam Controller", eAID_SteamController},
                    {"All", eAID_All}
            };
            static int* currentCombo = new int(EActionInputDevice::eAID_KeyboardMouse);
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
            ImGui::Combo("Device", currentCombo, deviceNames, IM_ARRAYSIZE(deviceNames));
            ImGui::SameLine();
            if(ImGui::Button("Save Keybinds")){
                saveActionMapsToXML();
            }
            ImGui::SameLine();
            if(ImGui::Button("Load Keybinds")){
                resetActionMapsToDefaults();
                loadActionMapsFromXML();
            }
            ImGui::SameLine();
            if(ImGui::Button("Reset To Defaults")){
                resetActionMapsToDefaults();
            }
            ImGui::Checkbox("Show Detailed Info", (bool*)&m_bShowDetailedInfo);
            m_device = deviceMap[deviceNames[*currentCombo]];
            if(m_device == EActionInputDevice::eAID_XboxPad || m_device == EActionInputDevice::eAID_PS4Pad || m_device == EActionInputDevice::eAID_SteamController){
                // push a nice yellow style color
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
                ImGui::Text("Controller support is experimental and may require additional manual editing.");
                ImGui::PopStyleColor();
            }
            if (ImGui::BeginTabBar("Input Methods Tab Bar")) {
                static float minimumColumnWidth = 300;
                if(ImGui::BeginTabItem("Actions")) {
                    // calculate the number of columns that fit in the current window width
                    int columns = (int) (ImGui::GetWindowWidth() / minimumColumnWidth);
                    if (columns < 1) columns = 1;
                    // distribute the categories evenly across the columns
                    ImGui::Text("Number of actionMaps: %llu", ActionMapManager->m_actionMaps.size());
                    static ImGuiTextFilter filter;
                    filter.Draw();
                    if(ImGui::BeginTable("Category Table", columns, ImGuiTableFlags_ScrollY, ImVec2(0, 0), 0)) {
                        ImGui::TableNextColumn();
                        int j = 0;
                        for (int i = 0; i < columns; i++) {
                            j = 0;
                            ImGui::TableSetColumnIndex(i);
                            // every nth category is in the same column
                            for (auto category : m_actionCategoryNode.children()) {
                                auto column = j % columns;
                                if (column == i) {
                                    bool showCategory = false;
                                    for(auto actions : category){
                                        if(filter.PassFilter(actions.attribute("ActionID").as_string()) || filter.PassFilter(actions.attribute("displayName").as_string()))
                                            showCategory = true;
                                    }
                                    if(showCategory) {
                                        std::string categoryName = category.attribute("name").as_string();
                                        ImGui::Separator();
                                        if(ImGui::CollapsingHeader(categoryName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                                            if (ImGui::BeginTable(categoryName.c_str(), 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
                                                ImGui::TableSetupColumn("Action");
                                                ImGui::TableSetupColumn("Binding");
                                                ImGui::TableSetupColumn("##Buttons", ImGuiTableColumnFlags_WidthFixed, 18.0f);
                                                ImGui::TableHeadersRow();
                                                for (auto actions: category) {
                                                    if (filter.PassFilter(actions.attribute("ActionID").as_string()) || filter.PassFilter(actions.attribute("displayName").as_string())) {
                                                        // Groups
                                                        if (actions.name() == std::string("ActionGroup")) {
                                                            std::vector<std::string> actionMaps;
                                                            for(auto & action: actions) {
                                                                actionMaps.emplace_back(action.attribute("ActionMap").as_string());
                                                            }
                                                            std::string baseActionMap = actions.attribute("baseActionMap").as_string();
                                                            ImGui::TableNextRow();
                                                            ImGui::TableNextColumn();
                                                            ImGui::TextWrapped("%s", actions.attribute("displayName").as_string());
                                                            if(ImGui::IsItemHovered()){
                                                                ImGui::BeginTooltip();
                                                                ImGui::Text("%s", actions.attribute("ActionID").as_string());
                                                                ImGui::Separator();
                                                                for(auto & actionMap: actionMaps){
                                                                    if(actionMap == baseActionMap)
                                                                        ImGui::Text("%s *", actionMap.c_str());
                                                                    else
                                                                        ImGui::Text("%s", actionMap.c_str());
                                                                }
                                                                ImGui::EndTooltip();
                                                            }
                                                            ImGui::TableNextColumn();
                                                            drawActionMapActionGroup(actions.attribute("ActionID").as_string(), actionMaps, baseActionMap);
                                                        }
                                                        // Actions
                                                        else {
                                                            ImGui::TableNextRow();
                                                            ImGui::TableNextColumn();
                                                            ImGui::TextWrapped("%s", /*actions.attribute("ActionMap").as_string(),*/ actions.attribute("displayName").as_string());
                                                            if(ImGui::IsItemHovered()/* && GImGui->HoveredIdTimer > 0.5f*/){
                                                                ImGui::BeginTooltip();
                                                                ImGui::Text("%s", actions.attribute("ActionID").as_string());
                                                                ImGui::Separator();
                                                                ImGui::Text("%s", actions.attribute("ActionMap").as_string());
                                                                ImGui::EndTooltip();
                                                            }
                                                            ImGui::TableNextColumn();
                                                            auto map = (CActionMap*)ActionMapManager->m_actionMaps[actions.attribute("ActionMap").as_string()];
                                                            if (map != nullptr) {
                                                                auto actionMap = map;
                                                                auto mappedAction = (CActionMapAction*)actionMap->GetAction(CCryName(actions.attribute("ActionID").as_string()));
                                                                drawActionMapAction(mappedAction, actions.attribute("ActionID").as_string(), actions.attribute("ActionMap").as_string());
                                                            } else {
                                                                ImGui::Text("ActionMap not found");
                                                            }
                                                        }
                                                    }
                                                }
                                                ImGui::EndTable();
                                            }
                                        }
                                    }
                                }
                                j++;
                            }
                        }
                        ImGui::EndTable();
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
                // advanced editor
                if(ImGui::BeginTabItem("Advanced")){
                    static std::string selectedActionMap;
                    // create a layout with a sidebar
                    if(ImGui::BeginChild("Action Maps", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 0), true)){
                        for(auto& actionMap: ActionMapManager->m_actionMaps){
                            if(ImGui::Selectable(actionMap.first.c_str(), selectedActionMap.c_str() == actionMap.first)){
                                selectedActionMap = actionMap.first;
                            }
                        }
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();
                    if(ImGui::BeginChild("Action Map", ImVec2(0, 0), true)){
                        if(!selectedActionMap.empty()){
                            auto actionMap = ActionMapManager->m_actionMaps[selectedActionMap.c_str()];
                            if(actionMap != nullptr){
                                ImGui::Text("Name: %s", selectedActionMap.c_str());
                                ImGui::Text("Enabled: %d", actionMap->m_enabled);
                                ImGui::SameLine();
                                if(ImGui::Button("Toggle Enabled")){
                                    actionMap->m_enabled = !actionMap->m_enabled;
                                }
                                ImGui::Text("Action Map Name: %s", actionMap->m_name.c_str());
                                static ImGuiTextFilter filter;
                                filter.Draw();
                                if(ImGui::BeginTable("Actions", 3, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders)){
                                    ImGui::TableSetupColumn("Action");
                                    ImGui::TableSetupColumn("Bindings");
                                    ImGui::TableSetupColumn("##AddButton", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, 20);
                                    ImGui::TableSetupScrollFreeze(0, 1);
                                    ImGui::TableHeadersRow();
                                    for(auto & action: actionMap->m_actions){
                                        if(filter.PassFilter(action.first.c_str())) {
                                            ImGui::TableNextRow();
                                            ImGui::TableNextColumn();
                                            ImGui::Text("%s", action.first.c_str());
                                            ImGui::TableNextColumn();
                                            auto mappedAction = actionMap->GetAction(action.first);
                                            if (mappedAction != nullptr)
                                                drawActionMapAction((CActionMapAction *) mappedAction, action.first.c_str(), selectedActionMap);
                                        }
                                    }
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Text("Add Action");
                                    ImGui::TableNextColumn();
                                    static char actionName[32];
                                    ImGui::InputText("##NewActionName", actionName, 32);
                                    ImGui::SameLine();
                                    if(ImGui::SmallButton("Add")){
                                        actionMap->CreateAction(CCryName(actionName));
                                    }
                                    ImGui::EndTable();
                                }
                            }
                        }
                    }
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
#ifdef DEBUG_BUILD
                if(ImGui::BeginTabItem("DEBUG")){
                    ImGui::Text("Is listening for input: %d", listener.isListening());
                    ImGui::Text("KeybindAction: %d", m_keybindAction);
                    ImGui::Text("ListeningForInput: %d", m_bListeningForInput);
                    ImGui::Text("Group: %d", m_bGroup);
                    ImGui::Text("ActionID: %s", m_actionID.c_str());
                    ImGui::Text("ActionMap: %s", m_actionMap.c_str());
                    ImGui::Text("CurrentBinding: %s", m_currentBinding.c_str());
                    ImGui::Text("NewBinding: %s", m_newBinding.c_str());
                    ImGui::Text("ActionMaps: ");
                    for(auto& actionMap: m_actionMaps){
                        ImGui::Text("%s", actionMap.c_str());
                    }
                    if(ImGui::CollapsingHeader("ActionMapManager")) {
                        for (auto &actionMapEventListener: ActionMapManager->m_actionMaps) {
                            ImGui::Text("ActionMap: %s", actionMapEventListener.first.c_str());
                        }
                    }
                    if(ImGui::Button("Save Profile")){
                        ((CGame*)gEnv->pGame)->m_pArkGame->SaveProfile();
                    }
                    ImGui::Text("Bound Action Maps: ");
                    ImGui::Separator();
                    for(auto& map: m_boundActionMaps){
                        ImGui::Text("%s", map.c_str());
                    }
                    ImGui::EndTabItem();
                }
#endif
                ImGui::EndTabBar();
            }
        }
        drawActionMapContextMenu();
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

void ModMain::MainUpdate(unsigned updateFlags)
{
    listenForInput();
}

void ModMain::ShutdownGame(bool isHotUnloading)
{
	// Your code goes here
    gEnv->pConsole->UnregisterVariable("universal_remapper_show_detailed_info");
    saveActionMapsToXML();
    BaseClass::ShutdownGame(isHotUnloading);
    gEnv->pInput->RemoveEventListener(&listener);
}

void ModMain::ShutdownSystem(bool isHotUnloading)
{
	// Your code goes here
	BaseClass::ShutdownSystem(isHotUnloading);
}

void dumpActionInput(SActionInput& actionInput){
    // print out everything in actionInput
    // EActionInputDevice            inputDevice;
    //	TActionInputString            input;
    //	TActionInputString            defaultInput;
    //	SActionInputBlockData         inputBlockData;
    //	uint32                        inputCRC;
    //	float                         fPressedTime;
    //	float                         fPressTriggerDelay;
    //	float                         fPressTriggerDelayRepeatOverride;
    //	float                         fLastRepeatTime;
    //	float                         fAnalogCompareVal;
    //	float                         fHoldTriggerDelay;
    //	float                         fCurrentHoldValue;   // A normalized amount for the current hold before triggering at the hold delay. Is 1 when hold is hit, & it does not reset when repeating
    //	float                         fReleaseTriggerThreshold;
    //	float                         fHoldRepeatDelay;
    //	float                         fHoldTriggerDelayRepeatOverride;
    //	int                           activationMode;
    //	int                           modifiers;
    //	int                           iPressDelayPriority;   // If priority is higher than the current
    //	EInputState                   currentState;
    //	EActionAnalogCompareOperation analogCompareOp;
    //	bool                          bHoldTriggerFired;
    //	bool                          bAnalogConditionFulfilled;
    CryLog("inputDevice: {}", actionInput.inputDevice);
    CryLog("input: {}", actionInput.input.c_str());
    CryLog("defaultInput: {}", actionInput.defaultInput.c_str());
//    CryLog("inputBlockData: {}", actionInput.inputBlockData);
    CryLog("inputCRC: {}", actionInput.inputCRC);
    CryLog("fPressedTime: {}", actionInput.fPressedTime);
    CryLog("fPressTriggerDelay: {}", actionInput.fPressTriggerDelay);
    CryLog("fPressTriggerDelayRepeatOverride: {}", actionInput.fPressTriggerDelayRepeatOverride);
    CryLog("fLastRepeatTime: {}", actionInput.fLastRepeatTime);
    CryLog("fAnalogCompareVal: {}", actionInput.fAnalogCompareVal);
    CryLog("fHoldTriggerDelay: {}", actionInput.fHoldTriggerDelay);
    CryLog("fCurrentHoldValue: {}", actionInput.fCurrentHoldValue);
    CryLog("fReleaseTriggerThreshold: {}", actionInput.fReleaseTriggerThreshold);
    CryLog("fHoldRepeatDelay: {}", actionInput.fHoldRepeatDelay);
    CryLog("fHoldTriggerDelayRepeatOverride: {}", actionInput.fHoldTriggerDelayRepeatOverride);
    // activation mode
    //  eAAM_Invalid   = 0,
    //	eAAM_OnPress   = BIT(0),        // Used when the action key is pressed
    //	eAAM_OnRelease = BIT(1),        // Used when the action key is released
    //	eAAM_OnHold    = BIT(2),        // Used when the action key is held
    //	eAAM_Always    = BIT(3),
    //
    //	// Special modifiers.
    //	eAAM_Retriggerable = BIT(4),
    //	eAAM_NoModifiers   = BIT(5),
    //	eAAM_ConsoleCmd    = BIT(6),
    //	eAAM_AnalogCompare = BIT(7),    // Used when analog compare op succeeds
    std::string activationMode;
    if(actionInput.activationMode & 0){
        activationMode += "eAAM_Invalid";
    }
    if(actionInput.activationMode & BIT(0)){
        activationMode += "eAAM_OnPress";
    }
    if(actionInput.activationMode & BIT(1)){
        activationMode += "eAAM_OnRelease";
    }
    if(actionInput.activationMode & BIT(2)){
        activationMode += "eAAM_OnHold";
    }
    if(actionInput.activationMode & BIT(3)){
        activationMode += "eAAM_Always";
    }
    if(actionInput.activationMode & BIT(4)){
        activationMode += "eAAM_Retriggerable";
    }
    if(actionInput.activationMode & BIT(5)){
        activationMode += "eAAM_NoModifiers";
    }
    if(actionInput.activationMode & BIT(6)){
        activationMode += "eAAM_ConsoleCmd";
    }
    if(actionInput.activationMode & BIT(7)){
        activationMode += "eAAM_AnalogCompare";
    }
    CryLog("activationMode: {}", activationMode);
    CryLog("modifiers: {}", actionInput.modifiers);
    CryLog("iPressDelayPriority: {}", actionInput.iPressDelayPriority);
    CryLog("currentState: {}", actionInput.currentState);
    CryLog("analogCompareOp: {}", actionInput.analogCompareOp);
    CryLog("bHoldTriggerFired: {}", actionInput.bHoldTriggerFired);
    CryLog("bAnalogConditionFulfilled: {}", actionInput.bAnalogConditionFulfilled);
}

void ModMain::listenForInput() {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    if(m_bListeningForInput){
        // time to remap
        if(listener.isEventValid() || m_keybindAction == KeybindAction::Remove){
            auto event = listener.getLastEvent();
            // filter out escape, commit, maxis x y and z
            if((event.keyId == EKeyId::eKI_Escape || event.keyId == EKeyId::eKI_SYS_Commit || event.keyId == EKeyId::eKI_MouseX || event.keyId == EKeyId::eKI_MouseY || event.keyId == EKeyId::eKI_MouseZ) && m_keybindAction != KeybindAction::Remove){
                // abort this operation
                clearInput();
                return;
            }
            if(event.keyName.key != nullptr) {
                if (event.keyName.key == m_currentBinding && m_keybindAction != KeybindAction::Remove) {
                    OverlayError("Keybind already exists");
                    // abort this operation
                    clearInput();
                    return;
                }
            }
            bool success = false;
            SActionInput input;
            if (event.deviceType == eIDT_Mouse || event.deviceType == eIDT_Keyboard) {
                input = getDefaultActionInput(m_actionID, m_actionMap, EActionInputDevice::eAID_KeyboardMouse);
            } else {
                input = getDefaultActionInput(m_actionID, m_actionMap, m_device);
            }
            input.input = event.keyName.key;
            input.inputCRC = CCrc32::ComputeLowercase(input.input.c_str());
            if(!m_bGroup) {
                CryLog("Individual");
                auto ActionMap = ActionMapManager->m_actionMaps[m_actionMap.c_str()];
                if (ActionMap == nullptr) {
                    CryError("Action map {} not found", m_actionMap.c_str());
                    clearInput();
                    return;
                }
                auto action = (CActionMapAction*)ActionMap->GetAction(CCryName(m_actionID.c_str()));
                switch (m_keybindAction) {
                    case KeybindAction::None:
                        break;
                    case KeybindAction::Add:
                        CryLog("Adding");

                        success = AddKeybind(m_actionID, m_actionMap, input);
                        if(!success)
                            CryError("Failed to add keybind");
                        else
                            ActionMap->m_iNumRebindedInputs++;
                        break;
                    case KeybindAction::Remove:
                        CryLog("Removing");
                        success = RemoveKeybind(m_actionID, m_actionMap, m_currentBinding);
                        break;
                    case KeybindAction::Rebind:
                        CryLog("Rebinding");
                        success = RebindKeybind(m_actionID, m_actionMap, m_currentBinding, event.keyName.key);
//                        success = ActionMap->ReBindActionInput(CCryName(m_actionID.c_str()), m_currentBinding.c_str(), event.keyName.key);
                        break;
                }
                if (success) {
                    CryLog("remapped successfully");
                } else {
                    CryError("remapping failed");
                }
            }
            //! Groups
            else {
                CryLog("Group");
                for(auto& actionMap : m_actionMaps){
                    auto mapFind = ActionMapManager->m_actionMaps.find(actionMap.c_str());
                    if(mapFind == ActionMapManager->m_actionMaps.end()){
                        CryWarning("Action map {} not found", actionMap.c_str());
                        continue;
                    }
                    auto ActionMap = mapFind->second;
                    switch (m_keybindAction) {
                        case KeybindAction::None:
                            break;
                        case KeybindAction::Add:
                            CryLog("Adding");
                            success = AddKeybind(m_actionID, actionMap, input);
                            if(!success){
                                CryError("Failed to add keybind to {}", actionMap.c_str());
                            } else {
                                ActionMap->m_iNumRebindedInputs++;
                            }
                            break;
                        case KeybindAction::Remove:
//                            CryLog("Removing");
                            success = RemoveKeybind(m_actionID, actionMap, m_currentBinding);
                            if(!success){
                                CryError("Failed to remove keybind from {}", actionMap.c_str());
                            }
                            break;
                        case KeybindAction::Rebind:
//                            CryLog("Rebinding");
                            success = RebindKeybind(m_actionID, actionMap, m_currentBinding, event.keyName.key);
                            if(!success){
                                CryError("Failed to rebind keybind in {}", actionMap.c_str());
                            }
                            break;
                    }
                }
                if(success){
                    switch(m_keybindAction){
                        case KeybindAction::Rebind:
                            OverlayLog("Remapped Successfully");
                            break;
                        case KeybindAction::Add:
                            OverlayLog("Added Successfully");
                            break;
                        case KeybindAction::Remove:
                            OverlayLog("Removed Successfully");
                            break;
                        default:
                            OverlayError("This is a bug, please report me! (NONE keybind action passed to success)");
                            break;
                    }

                }
                else{
                    switch(m_keybindAction){
                        case KeybindAction::Rebind:
                            OverlayLog("Remapping Failed");
                            break;
                        case KeybindAction::Add:
                            OverlayLog("Adding Failed");
                            break;
                        case KeybindAction::Remove:
                            OverlayLog("Removal Failed");
                            break;
                        default:
                            OverlayError("This is a bug, please report me! (NONE keybind action passed to !success)");
                            break;
                    }
                }
            }
            clearInput();
        }
    }

}

bool ModMain::rebindKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding, std::string primaryMap) {
    if(m_bListeningForInput){
        CryError("Already listening for input");
        return false;
    }
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_currentBinding = currentBinding;
    m_actionMap = primaryMap;
    m_keybindAction = KeybindAction::Rebind;
    listener.startListening();
    return true;
}

bool ModMain::addKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string primaryMap) {
    if(m_bListeningForInput){
        CryError("Already listening for input");
        return false;
    }
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_actionMap = primaryMap;
    m_keybindAction = KeybindAction::Add;
    listener.startListening();
    return true;
}

bool ModMain::removeKeybindGroupListen(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding) {
    CryLog("Removing keybind group");
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_currentBinding = currentBinding;
    m_keybindAction = KeybindAction::Remove;
    return true;
}

void ModMain::drawActionMapAction(CActionMapAction *mappedAction, std::string actionID, std::string actionMap) {
    if (mappedAction != nullptr) {
        auto action = mappedAction;
        bool isBound = false;
        for (auto &actionInput: action->m_actionInputs) {
            if (actionInput->inputDevice == m_device || m_device == EActionInputDevice::eAID_All) {
                isBound = true;
                bool selectedForListening = m_bListeningForInput
                                            && m_actionID == actionID
                                            && m_actionMap == actionMap
                                            && (m_currentBinding == actionInput->input.c_str() || m_keybindAction == KeybindAction::Add);
                if(ImGui::Selectable(actionInput->input.c_str(), selectedForListening)  && ImGui::IsItemHovered()){
                    RebindKeybindListen(actionID, actionMap, actionInput->input.c_str());
                }
                if(ImGui::IsItemClicked(ImGuiMouseButton_Right)){
                    setActionMapContextMenu(true, actionID, actionMap, actionInput->input.c_str());
//                    RemoveKeybindListen(actionID, actionMap, actionInput->input.c_str());
                }
            }
        }
        if (action->m_actionInputs.empty() || !isBound) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            bool selectedForListening = m_bListeningForInput
                                        && m_actionID == actionID
                                        && m_actionMap == actionMap
                                        && m_currentBinding.empty();
            if(ImGui::Selectable(("None##" + actionMap + actionID).c_str(), selectedForListening)){
                AddKeybindListen(actionID, actionMap);
            }
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered()){
                setActionMapContextMenu(true, actionID, actionMap, "");
            }
            ImGui::PopStyleColor();
        } else if (!action->m_actionInputs.empty()) {
            ImGui::TableNextColumn();
            if(ImGui::SmallButton((std::string("+##") + actionMap + actionID).c_str())){
                AddKeybindListen(actionID, actionMap);
            }
        }
    } else {
        ImGui::TextDisabled("Action Not Found");
    }

}

void ModMain::drawActionMapActionGroup(std::string actionID, std::vector<std::string> actionMaps, std::string primaryActionMap) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto mapFind = ActionMapManager->m_actionMaps.find(primaryActionMap.c_str());
    if(mapFind == ActionMapManager->m_actionMaps.end()){
        ImGui::TextDisabled("Primary Action Map Not Found");
        return;
    }
    auto primaryMap = mapFind->second;
    std::string primaryMapName = mapFind->first.c_str();
    if(primaryMap != nullptr){
        auto action = (CActionMapAction*)primaryMap->GetAction(CCryName(actionID.c_str()));
        if(action == nullptr){
            ImGui::TextDisabled("Action Not Found");
            return;
        }
        bool isBound = false;
        for (auto &actionInput: action->m_actionInputs) {
            if (actionInput->inputDevice == m_device || m_device == EActionInputDevice::eAID_All) {
                isBound = true;
                bool selectedForListening = m_bListeningForInput
                                            && m_actionID == actionID
                                            && m_bGroup
                                            &&( m_currentBinding == actionInput->input.c_str() || m_keybindAction == KeybindAction::Add);
                if(ImGui::Selectable(actionInput->input.c_str(), selectedForListening)){
//                    RebindKeybindListen(actionID, actionMap, actionInput->input.c_str());
                    rebindKeybindGroupListen(actionID, actionMaps, actionInput->input.c_str(), primaryMapName);
                }
                if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered()){
//                    removeKeybindGroupListen(actionID, actionMaps, actionInput->input.c_str());
                    setActionMapContextMenu(true, actionID, primaryActionMap, actionInput->input.c_str(), true, actionMaps);
                }
            }
        }
        if (action->m_actionInputs.empty() || !isBound) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            bool selectedForListening = m_bListeningForInput
                                        && m_actionID == actionID
                                        && m_bGroup
                                        && m_currentBinding.empty();
            if(ImGui::Selectable("None", selectedForListening)){
                addKeybindGroupListen(actionID, actionMaps, primaryMapName);
            }
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered()){
                setActionMapContextMenu(true, actionID, primaryActionMap, "", true, actionMaps);
            }
            ImGui::PopStyleColor();
        } else if (!action->m_actionInputs.empty()) {
            ImGui::TableNextColumn();
            if(ImGui::SmallButton((std::string("+##") + primaryActionMap + actionID).c_str())){
                addKeybindGroupListen(actionID, actionMaps, primaryMapName);
            }
        }
    } else {
        ImGui::TextDisabled("Action Not Found");
    }
}

void ModMain::clearInput() {
    m_keybindAction = KeybindAction::None;
    m_bListeningForInput = false;
    m_actionID.clear();
    m_actionMap.clear();
    m_currentBinding.clear();
    m_bGroup = false;
    m_actionMaps.clear();
}

void ModMain::setActionMapContextMenu(bool bShow, std::string actionID, std::string actionMap, std::string currentBinding, bool bGroup, std::vector<std::string> actionMaps) {
    m_bShowActionMapContextMenu = bShow;
    m_actionID = std::move(actionID);
    m_actionMap = std::move(actionMap);
    m_currentBinding = std::move(currentBinding);
    m_bGroup = bGroup;
    if(bGroup){
        m_actionMaps = std::move(actionMaps);
    } else {
        m_actionMaps.clear();
    }
}

void ModMain::drawActionMapContextMenu() {
    auto actionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    if(m_bShowActionMapContextMenu){
        ImGui::OpenPopup("Action Map Context Menu");
        m_bShowActionMapContextMenu = false;
    }
    static std::string newBinding;
    bool bOpenRebindManually = false, bAddMode = false;
    bool bOpenBindingSettings = false;
    ImGui::SetNextWindowBgAlpha(1.0);
    if (ImGui::BeginPopup("Action Map Context Menu")) {
        ImGui::Text("Action ID: %s", m_actionID.c_str());
        ImGui::Text("Binding: %s", m_currentBinding.c_str());
        if(showDetailedInfo()) {
            ImGui::Separator();
            if (ImGui::BeginTable("Action Maps", 1)) {
                ImGui::TableSetupColumn("Action Maps");
                ImGui::TableHeadersRow();
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s *", m_actionMap.c_str());
                for (auto &actionMap: m_actionMaps) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (actionMap != m_actionMap) {
                        ImGui::Text("%s", actionMap.c_str());
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::Separator();
        if(ImGui::BeginMenu("Bind")){
            if(ImGui::MenuItem("Rebind")){
                bOpenRebindManually = true;
                bAddMode = true;
            }
            if(ImGui::MenuItem("Unbind")){
                if(m_bGroup)
                    removeKeybindGroupListen(m_actionID, m_actionMaps, m_currentBinding);
                else
                    RemoveKeybindListen(m_actionID, m_actionMap, m_currentBinding);
            }
            if (ImGui::MenuItem("Add")) {
                bOpenRebindManually = true;
                bAddMode = true;
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Options")){
            if(ImGui::MenuItem("Copy Action ID")){
                ImGui::SetClipboardText(m_actionID.c_str());
            }
            if(ImGui::MenuItem("Copy Binding")){
                ImGui::SetClipboardText(m_currentBinding.c_str());
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Edit Binding Settings")){
                bOpenBindingSettings = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
    if(bOpenRebindManually){
        ImGui::OpenPopup("Keybinding Menu");
        bOpenRebindManually = false;
    }
    if(ImGui::BeginPopupContextWindow("Keybinding Menu")){
        if (ImGui::Button("Listen for input")) {
            if (m_bGroup) {
                if(!m_currentBinding.empty() || bAddMode)
                    addKeybindGroupListen(m_actionID, m_actionMaps, m_actionMap);
                else
                    rebindKeybindGroupListen(m_actionID, m_actionMaps, m_currentBinding, m_actionMap);
            } else {
                if(!m_currentBinding.empty() || bAddMode)
                    AddKeybindListen(m_actionID, m_actionMap);
                else
                    RebindKeybindListen(m_actionID, m_actionMap, m_currentBinding);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::Text("Enter new binding");
        ImGui::InputText("##newBinding", &newBinding);
        if(!m_currentBinding.empty() || bAddMode) {
            if (ImGui::Button("Rebind")) {
                if (m_bGroup)
                    RebindKeybindGroup(m_actionID, m_actionMaps, m_currentBinding, newBinding, m_actionMap);
                else
                    RebindKeybind(m_actionID, m_actionMap, m_currentBinding, newBinding);
                newBinding.clear();
                ImGui::CloseCurrentPopup();
            }
        } else {
            if(ImGui::Button("Add##button")){
                SActionInput actionInput;
                actionInput = getDefaultActionInput(m_actionID, m_actionMap, m_device);
                actionInput.input = newBinding.c_str();
                actionInput.inputCRC = CCrc32::ComputeLowercase(newBinding.c_str());
                if(m_bGroup)
                    AddKeybindGroup(m_actionID, m_actionMaps, actionInput, m_actionMap);
                else
                    AddKeybind(m_actionID, m_actionMap, actionInput);
                newBinding.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
    static SActionInput* actionInput;
    static bool bValidActionInput = false;
    static bool bLoadActionInput = false;
    if(bOpenBindingSettings){
        actionInput = nullptr;
        auto actionMap = actionMapManager->m_actionMaps[m_actionMap.c_str()];
        if(actionMap != nullptr){
            auto action = &actionMap->m_actions.find(CCryName(m_actionID.c_str()))->second;
            if(action != nullptr){
                for(auto searchInput : action->m_actionInputs){
                    if(searchInput->input.c_str() == m_currentBinding){
                        actionInput = searchInput;
                        bValidActionInput = true;
                        bLoadActionInput = true;
                        break;
                    }
                }
            }
        }
        if(bValidActionInput) {
            ImGui::OpenPopup("Binding Settings");
            bOpenBindingSettings = false;
        }
    }
    bool bOpen = true;
    if(ImGui::BeginPopupModal("Binding Settings", &bOpen, ImGuiWindowFlags_AlwaysAutoResize)){
        // data to store
//              EActionInputDevice            inputDevice; (int)
//	            TActionInputString            input; (string)
//	            TActionInputString            defaultInput; (string)
//	            SActionInputBlockData         inputBlockData;
//                  - EActionInputBlockType blockType;
//	                - TActionInputBlockers  inputs; (array of EKeyId)
//	                - float                 fBlockDuration;
//	                - int                   activationMode;
//	                - uint8                 deviceIndex;        // Device index - controller 1/2 etc
//	                - bool                  bAllDeviceIndices;  // True to block all device indices of deviceID type, otherwise uses deviceIndex
//	            uint32                        inputCRC;
//	            float                         fPressTriggerDelay;
//	            float                         fPressTriggerDelayRepeatOverride;
//	            float                         fAnalogCompareVal;
//	            float                         fHoldTriggerDelay;
//	            float                         fReleaseTriggerThreshold;
//	            float                         fHoldRepeatDelay;
//	            float                         fHoldTriggerDelayRepeatOverride;
//	            int                           activationMode;
//	            int                           modifiers;
//	            int                           iPressDelayPriority;   // If priority is higher than the current
//	            EActionAnalogCompareOperation analogCompareOp;
        if(bValidActionInput && actionInput != nullptr){
            ImGui::Text("ActionID: %s", m_actionID.c_str());
            ImGui::Text("Input: %s", actionInput->input.c_str());
            ImGui::Text("Default Input: %s", actionInput->defaultInput.c_str());
            static const char* deviceNames[] = {"Unknown", "Keyboard/Mouse", "Xbox Controller", "PS4 Controller", "Steam Controller", "All"};
            std::map<std::string, EActionInputDevice> deviceMap = {
                    {"Unknown", eAID_Unknown},
                    {"Keyboard/Mouse", eAID_KeyboardMouse},
                    {"Xbox Controller", eAID_XboxPad},
                    {"PS4 Controller", eAID_PS4Pad},
                    {"Steam Controller", eAID_SteamController},
                    {"All", eAID_All}
            };
            std::map<EActionInputDevice, int> deviceIndexMap = {
                    {eAID_Unknown, 0},
                    {eAID_KeyboardMouse, 1},
                    {eAID_XboxPad, 2},
                    {eAID_PS4Pad, 3},
                    {eAID_SteamController, 4},
                    {eAID_All, 5}
            };
            static int* currentCombo = new int(0);
            *currentCombo = deviceIndexMap[actionInput->inputDevice];
            ImGui::SetNextItemWidth(ImGui::CalcTextSize("Steam Controller").x + 40);
            if(ImGui::BeginCombo("Device", deviceNames[*currentCombo])){
                for(int i = 0; i < IM_ARRAYSIZE(deviceNames); i++){
                    bool isSelected = (i == *currentCombo);
                    if(ImGui::Selectable(deviceNames[i], isSelected)){
                        *currentCombo = i;
                        actionInput->inputDevice = deviceMap[deviceNames[i]];
                    }
                    if(isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Text("Block Data: TODO");
            ImGui::Text("Input CRC: %u", actionInput->inputCRC);
            ImGui::InputFloat("Press Trigger Delay", &actionInput->fPressTriggerDelay);
            ImGui::InputFloat("Press Trigger Delay Repeat Override", &actionInput->fPressTriggerDelayRepeatOverride);
            ImGui::InputFloat("Analog Compare Val", &actionInput->fAnalogCompareVal);
            ImGui::InputFloat("Hold Trigger Delay", &actionInput->fHoldTriggerDelay);
            ImGui::InputFloat("Release Trigger Threshold", &actionInput->fReleaseTriggerThreshold);
            ImGui::InputFloat("Hold Repeat Delay", &actionInput->fHoldRepeatDelay);
            ImGui::InputFloat("Hold Trigger Delay Repeat Override", &actionInput->fHoldTriggerDelayRepeatOverride);
//            eAAM_Invalid   = 0,
//	          eAAM_OnPress   = BIT(0),        // Used when the action key is pressed
//	          eAAM_OnRelease = BIT(1),        // Used when the action key is released
//	          eAAM_OnHold    = BIT(2),        // Used when the action key is held
//	          eAAM_Always    = BIT(3),
//
//	          // Special modifiers.
//	          eAAM_Retriggerable = BIT(4),
//	          eAAM_NoModifiers   = BIT(5),
//	          eAAM_ConsoleCmd    = BIT(6),
//	          eAAM_AnalogCompare = BIT(7),    // Used when analog compare op succeeds
            bool bOnPress = (actionInput->activationMode & eAAM_OnPress) != 0;
            bool bOnRelease = (actionInput->activationMode & eAAM_OnRelease) != 0;
            bool bOnHold = (actionInput->activationMode & eAAM_OnHold) != 0;
            bool bAlways = (actionInput->activationMode & eAAM_Always) != 0;
            bool bRetriggerable = (actionInput->activationMode & eAAM_Retriggerable) != 0;
            bool bNoModifiers = (actionInput->activationMode & eAAM_NoModifiers) != 0;
            bool bConsoleCmd = (actionInput->activationMode & eAAM_ConsoleCmd) != 0;
            bool bAnalogCompare = (actionInput->activationMode & eAAM_AnalogCompare) != 0;
            if(ImGui::Checkbox("On Press", &bOnPress)){
                if(bOnPress)
                    actionInput->activationMode |= eAAM_OnPress;
                else
                    actionInput->activationMode &= ~eAAM_OnPress;
            }
            if(ImGui::Checkbox("On Release", &bOnRelease)){
                if(bOnRelease)
                    actionInput->activationMode |= eAAM_OnRelease;
                else
                    actionInput->activationMode &= ~eAAM_OnRelease;
            }
            if(ImGui::Checkbox("On Hold", &bOnHold)){
                if(bOnHold)
                    actionInput->activationMode |= eAAM_OnHold;
                else
                    actionInput->activationMode &= ~eAAM_OnHold;
            }
            if(ImGui::Checkbox("Always", &bAlways)){
                if(bAlways)
                    actionInput->activationMode |= eAAM_Always;
                else
                    actionInput->activationMode &= ~eAAM_Always;
            }
            if(ImGui::Checkbox("Retriggerable", &bRetriggerable)){
                if(bRetriggerable)
                    actionInput->activationMode |= eAAM_Retriggerable;
                else
                    actionInput->activationMode &= ~eAAM_Retriggerable;
            }
            if(ImGui::Checkbox("No Modifiers", &bNoModifiers)){
                if(bNoModifiers)
                    actionInput->activationMode |= eAAM_NoModifiers;
                else
                    actionInput->activationMode &= ~eAAM_NoModifiers;
            }
            if(ImGui::Checkbox("Console Cmd", &bConsoleCmd)){
                if(bConsoleCmd)
                    actionInput->activationMode |= eAAM_ConsoleCmd;
                else
                    actionInput->activationMode &= ~eAAM_ConsoleCmd;
            }
            if(ImGui::Checkbox("Analog Compare", &bAnalogCompare)){
                if(bAnalogCompare)
                    actionInput->activationMode |= eAAM_AnalogCompare;
                else
                    actionInput->activationMode &= ~eAAM_AnalogCompare;
            }

        }
        ImGui::EndPopup();
    }
}

void ModMain::saveActionMapsToXML() {
    auto actionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
////    ((CGame*)gEnv->pGame)->m_pArkGame.get()->SaveProfile();
    pugi::xml_document doc;
    doc.append_child("ActionMaps");
    for(auto& actionMap: actionMapManager->m_actionMaps){
        auto actionMapNode = doc.child("ActionMaps").append_child("ActionMap");
        actionMapNode.append_attribute("name") = actionMap.first.c_str();
        actionMapNode.append_attribute("remapped").set_value(false);
        bool isActionMapRemapped = false;
        for(auto& action: actionMap.second->m_actions){
//            bool isActionRemapped = false;
//            for(auto& actionInput: action.second.m_actionInputs){
//                if(actionInput->input != actionInput->defaultInput || actionInput->defaultInput.empty()){
//                    actionMapNode.attribute("remapped").set_value(true);
//                    isActionRemapped = true;
//                    isActionMapRemapped = true;
//                    break;
//                }
//            }
//            if(m_boundActionMaps.find(actionMap.first.c_str()) != m_boundActionMaps.end() && m_boundActions.find(action.first.c_str()) != m_boundActions.end()){
//                isActionRemapped = true;
//                isActionMapRemapped = true;
//                actionMapNode.attribute("remapped").set_value(true);
//            }
//            if(!isActionRemapped){
//                continue;
//            }
            auto actionNode = actionMapNode.append_child("Action");
            actionNode.append_attribute("name") = action.first.c_str();
            for(auto& actionInput: action.second.m_actionInputs){
                pugi::xml_node actionInputNode;
                // data to store
//              EActionInputDevice            inputDevice; (int)
//	            TActionInputString            input; (string)
//	            TActionInputString            defaultInput; (string)
//	            SActionInputBlockData         inputBlockData;
//                  - EActionInputBlockType blockType;
//	                - TActionInputBlockers  inputs; (array of EKeyId)
//	                - float                 fBlockDuration;
//	                - int                   activationMode;
//	                - uint8                 deviceIndex;        // Device index - controller 1/2 etc
//	                - bool                  bAllDeviceIndices;  // True to block all device indices of deviceID type, otherwise uses deviceIndex
//	            uint32                        inputCRC;
//	            float                         fPressTriggerDelay;
//	            float                         fPressTriggerDelayRepeatOverride;
//	            float                         fAnalogCompareVal;
//	            float                         fHoldTriggerDelay;
//	            float                         fReleaseTriggerThreshold;
//	            float                         fHoldRepeatDelay;
//	            float                         fHoldTriggerDelayRepeatOverride;
//	            int                           activationMode;
//	            int                           modifiers;
//	            int                           iPressDelayPriority;   // If priority is higher than the current
//	            EActionAnalogCompareOperation analogCompareOp;
                actionInputNode = actionNode.append_child("Binding");
                actionInputNode.append_attribute("device").set_value(actionInput->inputDevice);
                actionInputNode.append_attribute("input").set_value(actionInput->input.c_str());
                actionInputNode.append_attribute("defaultInput").set_value(actionInput->defaultInput.c_str());
                // inputBlockData
                pugi::xml_node inputsNode = actionInputNode.append_child("ActionInputBlockData");
                for(auto& input: actionInput->inputBlockData.inputs){
                    pugi::xml_node inputNode = inputsNode.append_child("Input");
                    inputNode.append_attribute("keyID").set_value(input.keyId);
                }
                inputsNode.append_attribute("blockType").set_value(actionInput->inputBlockData.blockType);
                inputsNode.append_attribute("blockDuration").set_value(actionInput->inputBlockData.fBlockDuration);
                inputsNode.append_attribute("activationMode").set_value(actionInput->inputBlockData.activationMode);
                inputsNode.append_attribute("deviceIndex").set_value(actionInput->inputBlockData.deviceIndex);
                inputsNode.append_attribute("bAllDeviceIndices").set_value(actionInput->inputBlockData.bAllDeviceIndices);

                actionInputNode.append_attribute("inputCRC").set_value(actionInput->inputCRC);
                actionInputNode.append_attribute("fPressTriggerDelay").set_value(actionInput->fPressTriggerDelay);
                actionInputNode.append_attribute("fPressTriggerDelayRepeatOverride").set_value(actionInput->fPressTriggerDelayRepeatOverride);
                actionInputNode.append_attribute("fAnalogCompareVal").set_value(actionInput->fAnalogCompareVal);
                actionInputNode.append_attribute("fHoldTriggerDelay").set_value(actionInput->fHoldTriggerDelay);
                actionInputNode.append_attribute("fReleaseTriggerThreshold").set_value(actionInput->fReleaseTriggerThreshold);
                actionInputNode.append_attribute("fHoldRepeatDelay").set_value(actionInput->fHoldRepeatDelay);
                actionInputNode.append_attribute("fHoldTriggerDelayRepeatOverride").set_value(actionInput->fHoldTriggerDelayRepeatOverride);
                actionInputNode.append_attribute("activationMode").set_value(actionInput->activationMode);
                actionInputNode.append_attribute("modifiers").set_value(actionInput->modifiers);
                actionInputNode.append_attribute("iPressDelayPriority").set_value(actionInput->iPressDelayPriority);
                actionInputNode.append_attribute("analogCompareOp").set_value(actionInput->analogCompareOp);
            }
        }
//        if(!isActionMapRemapped){
//            doc.child("ActionMaps").remove_child(actionMapNode);
//        }
    }
    doc.save_file(m_actionMapFile.c_str());
}
void ModMain::loadActionMapsFromXML() {
    auto actionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(m_actionMapFile.c_str());
    if(!result){
        CryError("Failed to load action maps from file");
        return;
    }
    for(auto& actionMapNode: doc.first_child().children("ActionMap")){
        auto actionMap = actionMapManager->m_actionMaps[actionMapNode.attribute("name").as_string()];
        if(actionMap == nullptr){
            CryError("Failed to find action map {}", actionMapNode.attribute("name").as_string());
            continue;
        }
//        if(actionMapNode.attribute("remapped").as_bool()){
//            m_boundActionMaps.insert(actionMapNode.attribute("name").as_string());
        for(auto& actionNode: actionMapNode.children("Action")){
            std::string actionID = actionNode.attribute("name").as_string();
            auto Action = &actionMap->m_actions.find(CCryName(actionID.c_str()))->second;
            // remove all bindings
            for(auto& actionInput: Action->m_actionInputs){
                actionMap->RemoveActionInput(CCryName(actionID.c_str()), actionInput->input.c_str());
            }
            // Add bindings from xml
            for(auto & rebind: actionNode.children("Binding")){
                SActionInput actionInput;
                actionInput.inputDevice = static_cast<EActionInputDevice>(rebind.attribute("device").as_int());
                actionInput.input = rebind.attribute("input").as_string();
                actionInput.defaultInput = rebind.attribute("defaultInput").as_string();
                // inputBlockData
                auto inputBlockDataNode = rebind.child("ActionInputBlockData");
                for(auto& input: inputBlockDataNode.children("Input")){
                    SActionInputBlocker inputBlocker;
                    inputBlocker.keyId = static_cast<EKeyId>(input.attribute("keyID").as_int());
                    actionInput.inputBlockData.inputs.push_back(inputBlocker);
                }
                actionInput.inputBlockData.blockType = static_cast<EActionInputBlockType>(inputBlockDataNode.attribute("blockType").as_int());
                actionInput.inputBlockData.fBlockDuration = inputBlockDataNode.attribute("blockDuration").as_float();
                actionInput.inputBlockData.activationMode = inputBlockDataNode.attribute("activationMode").as_int();
                actionInput.inputBlockData.deviceIndex = inputBlockDataNode.attribute("deviceIndex").as_int();
                actionInput.inputBlockData.bAllDeviceIndices = inputBlockDataNode.attribute("bAllDeviceIndices").as_bool();

                actionInput.inputCRC = rebind.attribute("inputCRC").as_uint();
                actionInput.fPressTriggerDelay = rebind.attribute("fPressTriggerDelay").as_float();
                actionInput.fPressTriggerDelayRepeatOverride = rebind.attribute("fPressTriggerDelayRepeatOverride").as_float();
                actionInput.fAnalogCompareVal = rebind.attribute("fAnalogCompareVal").as_float();
                actionInput.fHoldTriggerDelay = rebind.attribute("fHoldTriggerDelay").as_float();
                actionInput.fReleaseTriggerThreshold = rebind.attribute("fReleaseTriggerThreshold").as_float();
                actionInput.fHoldRepeatDelay = rebind.attribute("fHoldRepeatDelay").as_float();
                actionInput.fHoldTriggerDelayRepeatOverride = rebind.attribute("fHoldTriggerDelayRepeatOverride").as_float();
                actionInput.activationMode = rebind.attribute("activationMode").as_int();
                actionInput.modifiers = rebind.attribute("modifiers").as_int();
                actionInput.iPressDelayPriority = rebind.attribute("iPressDelayPriority").as_int();
                actionInput.analogCompareOp = (EActionAnalogCompareOperation)rebind.attribute("analogCompareOp").as_int();

                AddKeybind(actionID, actionMapNode.attribute("name").as_string(), actionInput);
            }
        }
//        }
    }
}

bool ModMain::RebindKeybind(std::string actionID, std::string actionMap, std::string currentBinding, std::string newBinding) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        return false;
    }
    auto success = actionMapPtr->ReBindActionInput(CCryName(actionID.c_str()), currentBinding.c_str(), newBinding.c_str());
    m_boundActionMaps.insert(actionMap);
    m_boundActions.insert(actionID);
    return success;

}

bool ModMain::AddKeybind(std::string actionID, std::string actionMap, SActionInput &newInput) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        return false;
    }
    m_boundActionMaps.insert(actionMap);
    m_boundActions.insert(actionID);
    return actionMapPtr->AddAndBindActionInput(CCryName(actionID.c_str()), newInput);
}

bool ModMain::RemoveKeybind(std::string actionID, std::string actionMap, std::string currentBinding) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        return false;
    }
    m_boundActionMaps.insert(actionMap);
    m_boundActions.insert(actionID);
    return actionMapPtr->RemoveActionInput(CCryName(actionID.c_str()), currentBinding.c_str());
}

void ModMain::resetActionMapsToDefaults() {
    auto actionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto nodeRef = gEnv->pSystem->LoadXmlFromFile("Libs/Config/defaultProfile.xml");
    actionMapManager->SetLoadFromXMLPath("Libs/Config/defaultProfile.xml");
    if(nodeRef == nullptr){
        CryError("Failed to load default action maps from file");
        return;
    }
    if(actionMapManager->LoadFromXML(nodeRef)){
        OverlayLog("Action maps reset to default");
    } else {
        OverlayLog("Failed to reset action maps to default");
    }
}

SActionInput ModMain::getDefaultActionInput(std::string actionID, std::string actionMap, EActionInputDevice device) {
//              EActionInputDevice            inputDevice; (int)
//	            TActionInputString            input; (string)
//	            TActionInputString            defaultInput; (string)
//	            SActionInputBlockData         inputBlockData;
//                  - EActionInputBlockType blockType;
//	                - TActionInputBlockers  inputs; (array of EKeyId)
//	                - float                 fBlockDuration;
//	                - int                   activationMode;
//	                - uint8                 deviceIndex;        // Device index - controller 1/2 etc
//	                - bool                  bAllDeviceIndices;  // True to block all device indices of deviceID type, otherwise uses deviceIndex
//	            uint32                        inputCRC;
//	            float                         fPressTriggerDelay;
//	            float                         fPressTriggerDelayRepeatOverride;
//	            float                         fAnalogCompareVal;
//	            float                         fHoldTriggerDelay;
//	            float                         fReleaseTriggerThreshold;
//	            float                         fHoldRepeatDelay;
//	            float                         fHoldTriggerDelayRepeatOverride;
//	            int                           activationMode;
//	            int                           modifiers;
//	            int                           iPressDelayPriority;   // If priority is higher than the current
//	            EActionAnalogCompareOperation analogCompareOp;

    SActionInput defaultInput;
    auto defaultProfileNodeRef = gEnv->pSystem->LoadXmlFromFile("Libs/Config/defaultProfile.xml");
    if(defaultProfileNodeRef == nullptr){
        CryError("Failed to load default action maps from file");
        return  defaultInput;
    }
    XmlNodeRef actionMapNode;
    for(int i = 0; i < defaultProfileNodeRef->getChildCount(); i++){
        auto child = defaultProfileNodeRef->getChild(i);
        if(child->isTag("actionmap") && child->haveAttr("name")){
            if(child->getAttr("name") == actionMap) {
                actionMapNode = child;
                break;
            }
        }
    }
    if(actionMapNode == nullptr){
        CryError("Failed to find action map {} in {}", actionMap.c_str(), defaultProfileNodeRef->getTag());
        return defaultInput;
    }
    XmlNodeRef actionNode;
    for(int i = 0; i < actionMapNode->getChildCount(); i++){
        auto child = actionMapNode->getChild(i);
        if(child->isTag("action") && child->haveAttr("name")){
            if(child->getAttr("name") == actionID) {
                actionNode = child;
                break;
            }
        }
    }
    if(actionNode == nullptr){
        CryError("Failed to find action {} in defaultProfile.xml", actionID.c_str());
        return defaultInput;
    }
    // load all the possible data from the default profile
    defaultInput.inputDevice = device;
    XmlNodeRef inputDataNode;
    bool bHasSubInputData = false;
    switch(device){
        case EActionInputDevice::eAID_KeyboardMouse:
            if(actionNode->haveAttr("keyboard")) {
                defaultInput.defaultInput = actionNode->getAttr("keyboard");
                defaultInput.input = actionNode->getAttr("keyboard");
                defaultInput.inputCRC = CCrc32::ComputeLowercase(defaultInput.input.c_str());
            } else if (actionNode->findChild("keyboard")){
                auto KeyboardNode = actionNode->findChild("keyboard");
                if(KeyboardNode != nullptr){
                    inputDataNode = KeyboardNode->getChild(0);
                    if(inputDataNode != nullptr){
                        bHasSubInputData = true;
                        defaultInput.defaultInput = inputDataNode->getAttr("input");
                        defaultInput.input = inputDataNode->getAttr("input");
                        defaultInput.inputCRC = CCrc32::ComputeLowercase(defaultInput.input.c_str());
                    }
                }
            }
            break;
        case eAID_Unknown:
            // panic
            break;
        case eAID_XboxPad:
            if(actionNode->haveAttr("xboxpad")) {
                defaultInput.defaultInput = actionNode->getAttr("xboxpad");
                defaultInput.input = actionNode->getAttr("xboxpad");
                defaultInput.inputCRC = CCrc32::ComputeLowercase(defaultInput.input.c_str());
            }
            break;
        case eAID_PS4Pad:
            if(actionNode->haveAttr("ps4pad")) {
                defaultInput.defaultInput = actionNode->getAttr("ps4pad");
                defaultInput.input = actionNode->getAttr("ps4pad");
                defaultInput.inputCRC = CCrc32::ComputeLowercase(defaultInput.input.c_str());
            }
            break;
        case EActionInputDevice::eAID_SteamController:
            if(actionNode->haveAttr("steam")) {
                defaultInput.defaultInput = actionNode->getAttr("steam");
                defaultInput.input = actionNode->getAttr("steam");
                defaultInput.inputCRC = CCrc32::ComputeLowercase(defaultInput.input.c_str());
            }
            break;
        case eAID_All:
            // fuck off
            break;
    }
    if(actionNode->haveAttr("pressTriggerDelay")) {
        actionNode->getAttr("pressTriggerDelay", defaultInput.fPressTriggerDelay);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("pressTriggerDelay"))
            inputDataNode->getAttr("pressTriggerDelay", defaultInput.fPressTriggerDelay);
    }
    if(actionNode->haveAttr("pressTriggerDelayRepeatOverride")) {
        actionNode->getAttr("pressTriggerDelayRepeatOverride", defaultInput.fPressTriggerDelayRepeatOverride);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("pressTriggerDelayRepeatOverride"))
            inputDataNode->getAttr("pressTriggerDelayRepeatOverride", defaultInput.fPressTriggerDelayRepeatOverride);
    }
    if(actionNode->haveAttr("analogCompareVal")) {
        actionNode->getAttr("analogCompareVal", defaultInput.fAnalogCompareVal);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("analogCompareVal"))
            inputDataNode->getAttr("analogCompareVal", defaultInput.fAnalogCompareVal);
    }
    if(actionNode->haveAttr("holdTriggerDelay")) {
        actionNode->getAttr("holdTriggerDelay", defaultInput.fHoldTriggerDelay);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("holdTriggerDelay"))
            inputDataNode->getAttr("holdTriggerDelay", defaultInput.fHoldTriggerDelay);
    }
    if(actionNode->haveAttr("releaseTriggerThreshold")) {
        actionNode->getAttr("releaseTriggerThreshold", defaultInput.fReleaseTriggerThreshold);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("releaseTriggerThreshold"))
            inputDataNode->getAttr("releaseTriggerThreshold", defaultInput.fReleaseTriggerThreshold);
    }
    if(actionNode->haveAttr("holdRepeatDelay")) {
        actionNode->getAttr("holdRepeatDelay", defaultInput.fHoldRepeatDelay);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("holdRepeatDelay"))
            inputDataNode->getAttr("holdRepeatDelay", defaultInput.fHoldRepeatDelay);
    }
    if(actionNode->haveAttr("holdTriggerDelayRepeatOverride")) {
        actionNode->getAttr("holdTriggerDelayRepeatOverride", defaultInput.fHoldTriggerDelayRepeatOverride);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("holdTriggerDelayRepeatOverride"))
            inputDataNode->getAttr("holdTriggerDelayRepeatOverride", defaultInput.fHoldTriggerDelayRepeatOverride);
    }
    // activation mode is more complicated
//    eAAM_Invalid   = 0,
//	  eAAM_OnPress   = BIT(0),        // Used when the action key is pressed
//	  eAAM_OnRelease = BIT(1),        // Used when the action key is released
//	  eAAM_OnHold    = BIT(2),        // Used when the action key is held
//	  eAAM_Always    = BIT(3),
//
//	  // Special modifiers.
//	  eAAM_Retriggerable = BIT(4),
//	  eAAM_NoModifiers   = BIT(5),
//	  eAAM_ConsoleCmd    = BIT(6),
//	  eAAM_AnalogCompare = BIT(7),    // Used when analog compare op succeeds
    if(actionNode->haveAttr("onHold")){
        bool onHold;
        actionNode->getAttr("onHold", onHold);
        if(onHold){
            defaultInput.activationMode |= eAAM_OnHold;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("onHold")){
            bool onHold;
            inputDataNode->getAttr("onHold", onHold);
            if(onHold){
                defaultInput.activationMode |= eAAM_OnHold;
            }
        }
    }
    if(actionNode->haveAttr("onPress")){
        bool onPress;
        actionNode->getAttr("onPress", onPress);
        if(onPress){
            defaultInput.activationMode |= eAAM_OnPress;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("onPress")){
            bool onPress;
            inputDataNode->getAttr("onPress", onPress);
            if(onPress){
                defaultInput.activationMode |= eAAM_OnPress;
            }
        }
    }
    if(actionNode->haveAttr("onRelease")){
        bool onRelease;
        actionNode->getAttr("onRelease", onRelease);
        if(onRelease){
            defaultInput.activationMode |= eAAM_OnRelease;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("onRelease")){
            bool onRelease;
            inputDataNode->getAttr("onRelease", onRelease);
            if(onRelease){
                defaultInput.activationMode |= eAAM_OnRelease;
            }
        }
    }
    if(actionNode->haveAttr("always")){
        bool always;
        actionNode->getAttr("always", always);
        if(always){
            defaultInput.activationMode |= eAAM_Always;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("always")){
            bool always;
            inputDataNode->getAttr("always", always);
            if(always){
                defaultInput.activationMode |= eAAM_Always;
            }
        }
    }
    if(actionNode->haveAttr("retriggerable")){
        bool retriggerable;
        actionNode->getAttr("retriggerable", retriggerable);
        if(retriggerable){
            defaultInput.activationMode |= eAAM_Retriggerable;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("retriggerable")){
            bool retriggerable;
            inputDataNode->getAttr("retriggerable", retriggerable);
            if(retriggerable){
                defaultInput.activationMode |= eAAM_Retriggerable;
            }
        }
    }
    if(actionNode->haveAttr("noModifiers")){
        bool noModifiers;
        actionNode->getAttr("noModifiers", noModifiers);
        if(noModifiers){
            defaultInput.activationMode |= eAAM_NoModifiers;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("noModifiers")){
            bool noModifiers;
            inputDataNode->getAttr("noModifiers", noModifiers);
            if(noModifiers){
                defaultInput.activationMode |= eAAM_NoModifiers;
            }
        }
    }
    if(actionNode->haveAttr("consoleCmd")){
        bool consoleCmd;
        actionNode->getAttr("consoleCmd", consoleCmd);
        if(consoleCmd){
            defaultInput.activationMode |= eAAM_ConsoleCmd;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("consoleCmd")){
            bool consoleCmd;
            inputDataNode->getAttr("consoleCmd", consoleCmd);
            if(consoleCmd){
                defaultInput.activationMode |= eAAM_ConsoleCmd;
            }
        }
    }
    if(actionNode->haveAttr("useAnalogCompare")){
        bool analogCompare;
        actionNode->getAttr("useAnalogCompare", analogCompare);
        if(analogCompare){
            defaultInput.activationMode |= eAAM_AnalogCompare;
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("useAnalogCompare")){
            bool analogCompare;
            inputDataNode->getAttr("useAnalogCompare", analogCompare);
            if(analogCompare){
                defaultInput.activationMode |= eAAM_AnalogCompare;
            }
        }
    }
    if(actionNode->haveAttr("modifiers")) {
        actionNode->getAttr("modifiers", defaultInput.modifiers);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("modifiers"))
            inputDataNode->getAttr("modifiers", defaultInput.modifiers);
    } else {
        defaultInput.modifiers = 0;
    }
    if(actionNode->haveAttr("pressDelayPriority")) {
        actionNode->getAttr("pressDelayPriority", defaultInput.iPressDelayPriority);
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("pressDelayPriority"))
            inputDataNode->getAttr("pressDelayPriority", defaultInput.iPressDelayPriority);
    }
    if(actionNode->haveAttr("analogCompareOp")) {
        std::string compareOp = actionNode->getAttr("analogCompareOp");
        if(compareOp == "GREATERTHAN"){
            defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_GreaterThan;
        } else if (compareOp == "LESSTHAN"){
            defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_LessThan;
        } else if (compareOp == "EQUALS"){
            defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_Equals;
        } else if (compareOp == "NOTEQUALS"){
            defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_NotEquals;
        } else {
            defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_None;
            CryError("Unknown analog compare operation {}", compareOp.c_str());
        }
    } else if(bHasSubInputData){
        if(inputDataNode->haveAttr("analogCompareOp")){
            std::string compareOp = inputDataNode->getAttr("analogCompareOp");
            if(compareOp == "GREATERTHAN"){
                defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_GreaterThan;
            } else if (compareOp == "LESSTHAN"){
                defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_LessThan;
            } else if (compareOp == "EQUALS"){
                defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_Equals;
            } else if (compareOp == "NOTEQUALS"){
                defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_NotEquals;
            } else {
                defaultInput.analogCompareOp = EActionAnalogCompareOperation::eAACO_None;
                CryError("Unknown analog compare operation {}", compareOp.c_str());
            }
        }
    }
    return defaultInput;
}

bool ModMain::RebindKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding, std::string newBinding, std::string primaryMap) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    bool success;
    for(auto& actionMap : m_actionMaps) {
        auto mapFind = ActionMapManager->m_actionMaps.find(actionMap.c_str());
        if (mapFind == ActionMapManager->m_actionMaps.end()) {
            CryWarning("Action map {} not found", actionMap.c_str());
            continue;
        }
        auto ActionMap = mapFind->second;
        success = RebindKeybind(m_actionID, actionMap, m_currentBinding, newBinding);
        if (!success) {
            CryError("Failed to rebind keybind in {}", actionMap.c_str());
        }
    }
    return success;
}

bool ModMain::AddKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, SActionInput &newInput, std::string primaryMap) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    bool success;
    for(auto& actionMap : m_actionMaps) {
        auto mapFind = ActionMapManager->m_actionMaps.find(actionMap.c_str());
        if (mapFind == ActionMapManager->m_actionMaps.end()) {
            CryWarning("Action map {} not found", actionMap.c_str());
            continue;
        }
        auto ActionMap = mapFind->second;
        success = AddKeybind(m_actionID, actionMap, newInput);
        if (!success) {
            CryError("Failed to add keybind in {}", actionMap.c_str());
        }
    }
    return false;
}

bool ModMain::RemoveKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding) {
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    bool success;
    for(auto& actionMap : m_actionMaps) {
        auto mapFind = ActionMapManager->m_actionMaps.find(actionMap.c_str());
        if (mapFind == ActionMapManager->m_actionMaps.end()) {
            CryWarning("Action map {} not found", actionMap.c_str());
            continue;
        }
        auto ActionMap = mapFind->second;
        success = RemoveKeybind(m_actionID, actionMap, m_currentBinding);
        if (!success) {
            CryError("Failed to remove keybind in {}", actionMap.c_str());
        }
    }
    return false;
}

void ModMain::DoNothing() {
    // Do nothing
    CryLog("Doing nothing");
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
