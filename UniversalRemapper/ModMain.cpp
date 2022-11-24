#include "ModMain.h"
#include "RemapperKeyEventListener.h"
#include <Prey/CryGame/IGameFramework.h>
#include <Prey/CryAction/ActionMapManager.h>
#include <Prey/CryAction/ActionFilter.h>
#include <pugixml.hpp>
#include "ImGui/imgui_internal.h"
#include <Prey/CryAction/ActionMap.h>

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
    auto success = m_actionCategoryDoc.load_file("Mods/" MOD_NAME "/ActionsByCategories.xml");
    if (!success)
    {
        CryError("Failed to load ActionsByCategories.xml");
        return;
    }
    m_actionCategoryNode = m_actionCategoryDoc.first_child();
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
        }
    }
    doc.save_file("ActionDumpNoBinds.xml");
};


bool ModMain::rebindKeybind(std::string actionID, std::string actionMap, std::string currentBinding){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap %s not found", actionMap.c_str());
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

bool ModMain::addKeybind(std::string actionID, std::string actionMap){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap %s not found", actionMap.c_str());
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
bool ModMain::removeKeybind(std::string actionID, std::string actionMap, std::string currentBinding){
    auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
    auto actionMapPtr = ActionMapManager->m_actionMaps[actionMap.c_str()];
    if(actionMapPtr == nullptr){
        CryError("ActionMap %s not found", actionMap.c_str());
        return false;
    }
   /* auto action = actionMapPtr->GetAction(CCryName(actionID.c_str()));
    if(action == nullptr){
        CryError("Action %s not found in ActionMap %s", actionID.c_str(), actionMap.c_str());
        return false;
    }*/
   actionMapPtr->RemoveActionInput(CCryName(actionID.c_str()), currentBinding.c_str());
//    m_bListeningForInput = true;
//    m_actionMap = actionMap;
//    m_actionID = actionID;
//    m_currentBinding = currentBinding;
//    m_keybindAction = KeybindAction::Remove;
    return true;
}


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
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
        if (ImGui::Begin("Universal Remapper")) {
            auto ActionMapManager = (CActionMapManager*) gCL->cl->GetFramework()->GetIActionMapManager();
//            	eAID_Unknown       = 0,
//	            eAID_KeyboardMouse = BIT(0),
//	            eAID_XboxPad       = BIT(1),
//	            eAID_PS4Pad        = BIT(2),
//	            eAID_OculusTouch   = BIT(3),
            static const char* deviceNames[] = {"Unknown", "Keyboard/Mouse", "Xbox Controller", "PS4 Controller", "Oculus Touch"};
            static int* currentCombo = new int(EActionInputDevice::eAID_KeyboardMouse);
            ImGui::Combo("Device", currentCombo, deviceNames, IM_ARRAYSIZE(deviceNames));
            m_device = (EActionInputDevice) *currentCombo;
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
                                        if(filter.PassFilter(actions.attribute("ActionID").as_string()))
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
                                                    if (filter.PassFilter(actions.attribute("ActionID").as_string())) {
                                                        // Groups
                                                        if (actions.name() == std::string("ActionGroup")) {
                                                            ImGui::TableNextRow();
                                                            ImGui::TableNextColumn();
                                                            ImGui::Text("%s", actions.attribute("ActionID").as_string());
                                                            ImGui::TableNextColumn();
                                                            // in order to remap we need to know the current binding, actionID, and all actionMaps that contain the action.
                                                            std::map<std::string, std::vector<std::pair<std::string, SActionInput*>>> actionInputToActionMaps;
                                                            for(auto &action: actions){
                                                                auto actionMap = ActionMapManager->m_actionMaps[action.attribute("ActionMap").as_string()];
                                                                if(actionMap != nullptr){
                                                                    auto actionPtr = (CActionMapAction*)actionMap->GetAction(CCryName(action.attribute("ActionID").as_string()));
                                                                    if(actionPtr != nullptr){
                                                                        bool hasBind = false;
                                                                        for(auto& actionInput: actionPtr->m_actionInputs){
                                                                            if(actionInput->inputDevice == m_device || m_device == EActionInputDevice::eAID_Unknown) {
                                                                                hasBind = true;
                                                                                if(actionInputToActionMaps.find(actionInput->input.c_str()) == actionInputToActionMaps.end()){
                                                                                    actionInputToActionMaps[actionInput->input.c_str()] = {};
                                                                                }
                                                                                actionInputToActionMaps[actionInput->input.c_str()].push_back({action.attribute("ActionMap").as_string(), actionInput});
                                                                            }
                                                                        }
                                                                        if(!hasBind){
                                                                            if(actionInputToActionMaps.find("No Bind") == actionInputToActionMaps.end()){
                                                                                actionInputToActionMaps["No Bind"] = {};
                                                                            }
                                                                            actionInputToActionMaps["No Bind"].push_back({action.attribute("ActionMap").as_string(), nullptr});
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                            if(!actionInputToActionMaps.empty()) {
                                                                static std::string popupMenuActionInput;
                                                                for (auto &actionInput: actionInputToActionMaps) {
                                                                    if(actionInput.first == "No Bind") {
                                                                        ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                                                                        if(ImGui::Selectable("None")){
                                                                        }
                                                                        ImGui::PopStyleColor();
                                                                    } else {
                                                                        if (ImGui::Selectable((actionInput.first + "##" +  actions.attribute("ActionID").as_string()).c_str())) {
                                                                            if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
                                                                                popupMenuActionInput = actionInput.first;
                                                                                ImGui::OpenPopup(("GroupRemapPopup##" + popupMenuActionInput + actions.attribute("ActionID").as_string()).c_str());
                                                                            } else {
                                                                                // TODO: implement
                                                                            }
                                                                        }
                                                                    }
//                                                                    if(ImGui::IsItemClicked(ImGui::IsItemClicked(ImGuiMouseButton_Right))){
//                                                                        popupMenuActionInput = actionInput.first;
//                                                                        ImGui::OpenPopup(("GroupRemapPopup##" + popupMenuActionInput + actions.attribute("ActionID").as_string()).c_str());
//                                                                    }
                                                                    if (ImGui::IsItemHovered()) {
                                                                        ImGui::BeginTooltip();
                                                                        for (auto &actionMap: actionInput.second) {
                                                                            ImGui::Text("%s", actionMap.first.c_str());
                                                                        }
                                                                        ImGui::EndTooltip();
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        // Actions
                                                        else {
                                                            ImGui::TableNextRow();
                                                            ImGui::TableNextColumn();
                                                            ImGui::Text("%s", /*actions.attribute("ActionMap").as_string(),*/ actions.attribute("ActionID").as_string());
                                                            if(ImGui::IsItemHovered()/* && GImGui->HoveredIdTimer > 0.5f*/){
                                                                ImGui::BeginTooltip();
                                                                ImGui::Text("%s", actions.attribute("ActionID").as_string());
                                                                ImGui::EndTooltip();
                                                            }
                                                            ImGui::TableNextColumn();
                                                            auto map = (CActionMap*)ActionMapManager->m_actionMaps[actions.attribute("ActionMap").as_string()];
                                                            if (map != nullptr) {
                                                                auto actionMap = map;
                                                                auto mappedAction = (CActionMapAction*)actionMap->GetAction(CCryName(actions.attribute("ActionID").as_string()));
                                                                if (mappedAction != nullptr) {
                                                                    auto action = mappedAction;
                                                                    bool isBound = false;
                                                                    for (auto &actionInput: action->m_actionInputs) {
                                                                        if (actionInput->inputDevice == m_device || m_device == EActionInputDevice::eAID_Unknown) {
                                                                            isBound = true;
                                                                            bool selectedForListening = m_bListeningForInput
                                                                                    && m_actionID == actions.attribute("ActionID").as_string()
                                                                                    && m_actionMap == actions.attribute("ActionMap").as_string()
                                                                                    && m_currentBinding == actionInput->input.c_str();
                                                                            if(ImGui::Selectable(actionInput->input.c_str(), selectedForListening)){
                                                                                rebindKeybind(actions.attribute("ActionID").as_string(), actions.attribute("ActionMap").as_string(), actionInput->input.c_str());
                                                                            }
                                                                            if(ImGui::IsItemClicked(ImGuiMouseButton_Right)){
                                                                                removeKeybind(actions.attribute("ActionID").as_string(), actions.attribute("ActionMap").as_string(), actionInput->input.c_str());
                                                                            }
                                                                        }
                                                                    }
                                                                    if (action->m_actionInputs.empty() || !isBound) {
                                                                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                                                                        bool selectedForListening = m_bListeningForInput
                                                                                                    && m_actionID == actions.attribute("ActionID").as_string()
                                                                                                    && m_actionMap == actions.attribute("ActionMap").as_string()
                                                                                                    && m_currentBinding == "";
                                                                        if(ImGui::Selectable("None", selectedForListening)){
                                                                            addKeybind(actions.attribute("ActionID").as_string(), actions.attribute("ActionMap").as_string());
                                                                        }
                                                                        ImGui::PopStyleColor();
                                                                    } else if (!action->m_actionInputs.empty()) {
                                                                        ImGui::TableNextColumn();
                                                                        if(ImGui::SmallButton((std::string("+##") + actions.attribute("ActionMap").as_string() + actions.attribute("ActionID").as_string()).c_str())){
                                                                            addKeybind(actions.attribute("ActionID").as_string(), actions.attribute("ActionMap").as_string());
                                                                        }
                                                                    }
                                                                } else {
                                                                    ImGui::TextDisabled("Action Not Found");
                                                                }
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
               /* if (ImGui::BeginTabItem("Keyboard/Mouse")) {
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
                }*/
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
        ImGui::PopStyleVar();
    }
}

void ModMain::PreUpdate()
{
	// Your code goes here
}

void ModMain::PostUpdate()
{
    // Your code goes here
    listenForInput();
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

void ModMain::listenForInput() {
    if(m_bListeningForInput){
        // time to remap
        if(listener.isEventValid()){
            auto event = listener.getLastEvent();
            // filter out escape, commit, maxis x y and z
            if(event.keyId == EKeyId::eKI_Escape || event.keyId == EKeyId::eKI_SYS_Commit || event.keyId == EKeyId::eKI_MouseX || event.keyId == EKeyId::eKI_MouseY || event.keyId == EKeyId::eKI_MouseZ){
                // abort this operation
                m_bListeningForInput = false;
                m_actionID.clear();
                m_actionMap.clear();
                m_currentBinding.clear();
                m_keybindAction = KeybindAction::None;
                return;
            }
            auto ActionMapManager = (CActionMapManager *) gCL->cl->GetFramework()->GetIActionMapManager();
            bool success = false;
            SActionInput input;
            if(!m_bGroup) {
                auto ActionMap = ActionMapManager->m_actionMaps[m_actionMap.c_str()];
                if (ActionMap == nullptr) {
                    CryError("Action map %s not found", m_actionMap.c_str());
                    m_bListeningForInput = false;
                    m_actionID.clear();
                    m_actionMap.clear();
                    m_currentBinding.clear();
                    m_keybindAction = KeybindAction::None;
                    return;
                }
                switch (m_keybindAction) {
                    case KeybindAction::None:
                        break;
                    case KeybindAction::Add:
                        CryLog("Adding");
                        input.input = event.keyName.key;
                        if (event.deviceType == eIDT_Mouse || event.deviceType == eIDT_Keyboard) {
                            input.inputDevice = EActionInputDevice::eAID_KeyboardMouse;
                        } else if (event.deviceType == eIDT_Gamepad) {
                            input.inputDevice = EActionInputDevice::eAID_XboxPad;
                        }
//                        input.inputDevice = EActionInputDevice::eAID_KeyboardMouse;
                        input.inputCRC = CCrc32::ComputeLowercase(input.input.c_str());
                        success = ActionMap->AddAndBindActionInput(CCryName(m_actionID.c_str()), input);
                        break;
                    case KeybindAction::Remove:
                        CryLog("Removing");
                        success = ActionMap->RemoveActionInput(CCryName(m_actionID.c_str()), m_currentBinding.c_str());
                        break;
                    case KeybindAction::Rebind:
                        CryLog("Rebinding");
                        success = ActionMap->ReBindActionInput(CCryName(m_actionID.c_str()), m_currentBinding.c_str(), event.keyName.key);
                        break;
                }
                if (success) {
                    CryLog("remapped successfully");
                } else {
                    CryError("remapping failed");
                }
            }
            else {
                for(auto& actionMap : m_actionMaps){
                    auto ActionMap = ActionMapManager->m_actionMaps[actionMap.c_str()];
                    switch (m_keybindAction) {
                        case KeybindAction::None:
                            break;
                        case KeybindAction::Add:
                            CryLog("Adding");
                            input.input = event.keyName.key;
                            if (event.deviceType == eIDT_Mouse || event.deviceType == eIDT_Keyboard) {
                                input.inputDevice = EActionInputDevice::eAID_KeyboardMouse;
                            } else if (event.deviceType == eIDT_Gamepad) {
                                input.inputDevice = EActionInputDevice::eAID_XboxPad;
                            }
                            input.inputDevice = EActionInputDevice::eAID_KeyboardMouse;
                            input.inputCRC = CCrc32::ComputeLowercase(input.input.c_str());
                            success = ActionMap->AddAndBindActionInput(CCryName(m_actionID.c_str()), input);
                            break;
                        case KeybindAction::Remove:
                            CryLog("Removing");
                            success = ActionMap->RemoveActionInput(CCryName(m_actionID.c_str()), m_currentBinding.c_str());
                            break;
                        case KeybindAction::Rebind:
                            CryLog("Rebinding");
                            success = ActionMap->ReBindActionInput(CCryName(m_actionID.c_str()), m_currentBinding.c_str(), event.keyName.key);
                            break;
                    }
                }
            }
            m_bListeningForInput = false;
            m_actionID.clear();
            m_actionMap.clear();
            m_currentBinding.clear();
            m_keybindAction = KeybindAction::None;
        }
    }

}

bool ModMain::rebindKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding) {
    if(m_bListeningForInput){
        CryError("Already listening for input");
        return false;
    }
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_currentBinding = currentBinding;
    m_keybindAction = KeybindAction::Rebind;
    listener.startListening();
    return true;
}

bool ModMain::addKeybindGroup(std::string actionID, std::vector<std::string> actionMaps) {
    if(m_bListeningForInput){
        CryError("Already listening for input");
        return false;
    }
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_keybindAction = KeybindAction::Add;
    listener.startListening();
    return true;
}

bool ModMain::removeKeybindGroup(std::string actionID, std::vector<std::string> actionMaps, std::string currentBinding) {
    if(m_bListeningForInput){
        CryError("Already listening for input");
        return false;
    }
    m_bListeningForInput = true;
    m_bGroup = true;
    m_actionID = actionID;
    m_actionMaps = actionMaps;
    m_currentBinding = currentBinding;
    m_keybindAction = KeybindAction::Remove;
    return true;
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
