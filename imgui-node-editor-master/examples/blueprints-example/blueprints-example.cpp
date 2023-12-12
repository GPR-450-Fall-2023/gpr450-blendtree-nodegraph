#define IMGUI_DEFINE_MATH_OPERATORS
#include <application.h>
#include "utilities/builders.h"
#include "utilities/widgets.h"

#include <imgui_node_editor.h>
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <unordered_set>

//#include <json/json.h>
//#include <json/value.h>
//#include <json/writer.h>
#include <fstream>
#include <iostream> // remove later - just for debug purposes
#include <windows.h> // remove later - just for debug purposes
#include <consoleapi.h> // remove later - just for debug purposes


static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;

using ax::Widgets::IconType;

static ed::EditorContext* m_Editor = nullptr;

//extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int vkey);
//extern "C" bool Debug_KeyPress(int vkey)
//{
//    static std::map<int, bool> state;
//    auto lastState = state[vkey];
//    state[vkey] = (GetAsyncKeyState(vkey) & 0x8000) != 0;
//    if (state[vkey] && !lastState)
//        return true;
//    else
//        return false;
//}

enum class BlendEditorWindow
{
    editor = 0,
    save,
    load
};

//Hardcoded for proof of concpet, don't need to go too deep
const std::vector<std::string> BONE_NAMES =
{
    "mixamorig:Hips",
    "mixamorig:Spine",
    "mixamorig:Spine1",
    "mixamorig:Spine2",
    "mixamorig:Neck",
    "mixamorig:Head",
    "mixamorig:HeadTop_End",
    "mixamorig:LeftEye",
    "mixamorig:RightEye",
    "mixamorig:LeftShoulder",
    "mixamorig:LeftArm",
    "mixamorig:LeftForeArm",
    "mixamorig:LeftHand",
    "mixamorig:LeftHandThumb1",
    "mixamorig:LeftHandThumb2",
    "mixamorig:LeftHandThumb3",
    "mixamorig:LeftHandThumb4",
    "mixamorig:LeftHandIndex1",
    "mixamorig:LeftHandIndex2",
    "mixamorig:LeftHandIndex3",
    "mixamorig:LeftHandIndex4",
    "mixamorig:LeftHandMiddle1",
    "mixamorig:LeftHandMiddle2",
    "mixamorig:LeftHandMiddle3",
    "mixamorig:LeftHandMiddle4",
    "mixamorig:LeftHandRing1",
    "mixamorig:LeftHandRing2",
    "mixamorig:LeftHandRing3",
    "mixamorig:LeftHandRing4",
    "mixamorig:LeftHandPinky1",
    "mixamorig:LeftHandPinky2",
    "mixamorig:LeftHandPinky3",
    "mixamorig:LeftHandPinky4",
    "mixamorig:RightShoulder",
    "mixamorig:RightArm",
    "mixamorig:RightForeArm",
    "mixamorig:RightHand",
    "mixamorig:RightHandPinky1",
    "mixamorig:RightHandPinky2",
    "mixamorig:RightHandPinky3",
    "mixamorig:RightHandPinky4",
    "mixamorig:RightHandRing1",
    "mixamorig:RightHandRing2",
    "mixamorig:RightHandRing3",
    "mixamorig:RightHandRing4",
    "mixamorig:RightHandMiddle1",
    "mixamorig:RightHandMiddle2",
    "mixamorig:RightHandMiddle3",
    "mixamorig:RightHandMiddle4",
    "mixamorig:RightHandIndex1",
    "mixamorig:RightHandIndex2",
    "mixamorig:RightHandIndex3",
    "mixamorig:RightHandIndex4",
    "mixamorig:RightHandThumb1",
    "mixamorig:RightHandThumb2",
    "mixamorig:RightHandThumb3",
    "mixamorig:RightHandThumb4",
    "mixamorig:LeftUpLeg",
    "mixamorig:LeftLeg",
    "mixamorig:LeftFoot",
    "mixamorig:LeftToeBase",
    "mixamorig:LeftToe_End",
    "mixamorig:RightUpLeg",
    "mixamorig:RightLeg",
    "mixamorig:RightFoot",
    "mixamorig:RightToeBase",
    "mixamorig:RightToe_End"
};

enum class AnimalVar
{
    animal_var_hierarchyPoseGroup_skel,

    animal_var_jumpClipCtrl,
    animal_var_idleClipCtrl,
    animal_var_walkClipCtrl,
    animal_var_runClipCtrl,
    animal_var_idlePistolClipCtrl,

    animal_var_ctrlVelocityMagnitude,
    animal_var_idleBlendThreshold,
    animal_var_walkBlendThreshold,
    animal_var_runBlendThreshold,

    animal_var_jumpLerpParam,
    animal_var_jumpDuration,
    animal_var_jumpHeight,
    animal_var_jumpFadeInTime,
    animal_var_jumpFadeOutTime,
    animal_var_timeSinceJump,
    animal_var_isJumping,
    animal_var_ctrlNode,

    animal_var_max
};

const std::string ANIMAL_VAR_NAMES[] =
{
    "hierarchyPoseGroup_skel",
    "jumpClipCtrl",
    "idleClipCtrl",
    "walkClipCtrl",
    "runClipCtrl",
    "idlePistolClipCtrl",
    "ctrlVelocityMagnitude",
    "idleBlendThreshold",
    "walkBlendThreshold",
    "runBlendThreshold",
    "jumpLerpParam",
    "jumpDuration",
    "jumpHeight",
    "jumpFadeInTime",
    "jumpFadeOutTime",
    "timeSinceJump",
    "isJumping",
    "ctrlNode"
};

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
    Dropdown
};

enum class PinKind
{
    Output,
    Input
};

enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini
};

enum class DataGroup
{
    Spatial,
    Param,
    Misc,
    ID
};

struct Node;

struct Pin
{
    ed::PinId   ID;
    ::Node* Node;
    std::string Name;
    PinType     Type; // Flow, float, etc.
    PinKind     Kind; // Input, output
    std::string data; //Dirty, used to store name of animal variable if it is a dropdown type pin or the string value if it is a string type pin

    DataGroup dataGroup; // How this data will be interpreted in Animal3D (spatial, param, misc)

    Pin(int id, const char* name, PinType type, DataGroup group) :
        ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input), dataGroup(group)
    {
    }
};

struct Node
{
    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;

    std::string blendOp;

    std::string State;
    std::string SavedState;

    Node(int id, const char* name, std::string blendOperation, ImColor color = ImColor(255, 255, 255)) :
        ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0), blendOp(blendOperation)
    {
    }
};

struct Link
{
    ed::LinkId ID;

    ed::PinId StartPinID;
    ed::PinId EndPinID;

    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) :
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

struct NodeIdLess
{
    bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};

struct SaveData
{
    std::string path;
};

struct LoadData
{
    std::string path;
};

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

struct BlendEditor
{
    std::vector<Node>    m_Nodes;
    std::vector<Link>    m_Links;

    // Connection data maps
    std::unordered_map<int, Link*> inputPinToLink; // Pin to link connected to it (only works input to output)
    std::unordered_map<int, Pin*> pinIDToPin; // Pin id to pin reference

    std::unordered_map<std::string, bool> affectedBones;


    Pin* GetOutputConnectedPin(Pin* inputPin)
    {
        std::unordered_map<int, Link*>::iterator it = inputPinToLink.find(inputPin->ID.Get());

        if (it == inputPinToLink.end()) return nullptr; // Pin not connected to any link

        Link* link = inputPinToLink[inputPin->ID.Get()];
        return pinIDToPin[link->StartPinID.Get()];
    }


    void InitializeConnectionDataMaps()
    {
        inputPinToLink.clear();
        pinIDToPin.clear();

        // Loop through each node and update pinIDToPin with correct mappings
        for (int i = 0; i < m_Nodes.size(); i++)
        {
            Node* node = &(m_Nodes[i]);
            
            for (int j = 0; j < node->Inputs.size(); j++)
            {
                pinIDToPin.insert(std::make_pair(node->Inputs[j].ID.Get(), &(node->Inputs[j])));
            }

            for (int j = 0; j < node->Outputs.size(); j++)
            {
                pinIDToPin.insert(std::make_pair(node->Outputs[j].ID.Get(), &(node->Outputs[j])));
            }
        }

        // Loop through each link and map ending to inputPinToLink
        for (int i = 0; i < m_Links.size(); i++)
        {
            Link* link = &(m_Links[i]);

            inputPinToLink.insert(std::make_pair(link->EndPinID.Get(), link));
        }
    }
};

struct Example :
    public Application
{
    using Application::Application;

    int GetNextId()
    {
        return m_NextId++;
    }

    //ed::NodeId GetNextNodeId()
    //{
    //    return ed::NodeId(GetNextId());
    //}

    ed::LinkId GetNextLinkId()
    {
        return ed::LinkId(GetNextId());
    }

    void TouchNode(ed::NodeId id)
    {
        m_NodeTouchTime[id] = m_TouchTime;
    }

    float GetTouchProgress(ed::NodeId id)
    {
        auto it = m_NodeTouchTime.find(id);
        if (it != m_NodeTouchTime.end() && it->second > 0.0f)
            return (m_TouchTime - it->second) / m_TouchTime;
        else
            return 0.0f;
    }

    void UpdateTouch()
    {
        const auto deltaTime = ImGui::GetIO().DeltaTime;
        for (auto& entry : m_NodeTouchTime)
        {
            if (entry.second > 0.0f)
                entry.second -= deltaTime;
        }
    }

    Node* FindNode(ed::NodeId id)
    {
        for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
            if (node.ID == id)
                return &node;

        return nullptr;
    }

    Link* FindLink(ed::LinkId id)
    {
        for (auto& link : blendEditors[currentEditorIndex].m_Links)
            if (link.ID == id)
                return &link;

        return nullptr;
    }

    Pin* FindPin(ed::PinId id)
    {
        if (!id)
            return nullptr;

        for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
        {
            for (auto& pin : node.Inputs)
                if (pin.ID == id)
                    return &pin;

            for (auto& pin : node.Outputs)
                if (pin.ID == id)
                    return &pin;
        }

        return nullptr;
    }

    bool IsPinLinked(ed::PinId id)
    {
        if (!id)
            return false;

        for (auto& link : blendEditors[currentEditorIndex].m_Links)
            if (link.StartPinID == id || link.EndPinID == id)
                return true;

        return false;
    }

    bool CanCreateLink(Pin* a, Pin* b)
    {
        if (!a || !b || a->Type == PinType::Dropdown || b->Type == PinType::Dropdown || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
            return false;

        return true;
    }

    //void DrawItemRect(ImColor color, float expand = 0.0f)
    //{
    //    ImGui::GetWindowDrawList()->AddRect(
    //        ImGui::GetItemRectMin() - ImVec2(expand, expand),
    //        ImGui::GetItemRectMax() + ImVec2(expand, expand),
    //        color);
    //};

    //void FillItemRect(ImColor color, float expand = 0.0f, float rounding = 0.0f)
    //{
    //    ImGui::GetWindowDrawList()->AddRectFilled(
    //        ImGui::GetItemRectMin() - ImVec2(expand, expand),
    //        ImGui::GetItemRectMax() + ImVec2(expand, expand),
    //        color, rounding);
    //};

    void BuildNode(Node* node)
    {
        for (auto& input : node->Inputs)
        {
            input.Node = node;
            input.Kind = PinKind::Input;
        }

        for (auto& output : node->Outputs)
        {
            output.Node = node;
            output.Kind = PinKind::Output;
        }
    }

    /////////////// Custom Nodes /////////////////

    
    /// <summary>
    /// Node to evaluate a clip controller
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnClipCtrlNode(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Evaluate Clip Controller", "blendop_evaluate_clip_controller");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "ID", PinType::String, DataGroup::ID);
        /*m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input Clip Ctrl", PinType::Object);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input Hierarchy Pose Group", PinType::Object);*/
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input Clip Ctrl", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input Hierarchy Pose Group", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Output Pose", PinType::Flow, DataGroup::Spatial);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    /// <summary>
    /// Node to blend three poses on a 1D axis
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnBlend3Node(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Blend 3", "blendop_blend_3");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "ID", PinType::String, DataGroup::ID);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Blend Param", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Threshold A", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Threshold B", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Threshold C", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input A", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input B", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input C", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Output Pose", PinType::Flow, DataGroup::Spatial);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    /// <summary>
    /// Node to lerp between two poses
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnLerpNode(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Lerp", "blendop_lerp");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "ID", PinType::String, DataGroup::ID);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Magnitude", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input A", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input B", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Output Pose", PinType::Flow, DataGroup::Spatial);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    /// <summary>
    /// Node to handle jump logic
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnHandleJumpNode(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Handle Jump", "blendop_handle_jump");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "ID", PinType::String, DataGroup::ID);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Duration", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Height", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Fade In Time", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Fade Out Time", PinType::Dropdown, DataGroup::Param);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Time Since Jump", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Lerp Param", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Is Jumping", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Control Node", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Input 1", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Output Pose", PinType::Flow, DataGroup::Spatial);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    /// <summary>
    /// Node to branch based on a condition
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnBranchNode(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Branch", "blendop_bool_branch");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "ID", PinType::String, DataGroup::ID);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Dropdown, DataGroup::Misc);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "True", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "False", PinType::Flow, DataGroup::Spatial);
        editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Output Pose", PinType::Flow, DataGroup::Spatial);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    /// <summary>
    /// Root node at which the tree ends
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    Node* SpawnRootNode(BlendEditor* editor)
    {
        editor->m_Nodes.emplace_back(GetNextId(), "Root", "");
        editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Root Pose", PinType::Flow, DataGroup::ID);

        BuildNode(&editor->m_Nodes.back());

        return &editor->m_Nodes.back();
    }

    //////////////////////////////////////////////

    //Node* SpawnInputActionNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "InputAction Fire", ImColor(255, 128, 128));
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    ///*Node* SpawnBranchNode()
    //{
    //    m_Nodes.emplace_back(GetNextId(), "Branch");
    //    m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    m_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
    //    m_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
    //    m_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

    //    BuildNode(&m_Nodes.back());

    //    return &m_Nodes.back();
    //}*/

    //Node* SpawnDoNNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Do N");
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "N", PinType::Int);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnOutputActionNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "OutputAction");
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnPrintStringNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Print String");
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "In String", PinType::String);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnStringNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "", ImColor(128, 195, 248));
    //    editor->m_Nodes.back().Type = NodeType::Simple;
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "String", PinType::String);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnSetTimerNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Set Timer", ImColor(128, 195, 248));
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnLessNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "<", ImColor(128, 195, 248));
    //    editor->m_Nodes.back().Type = NodeType::Simple;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnWeirdNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "o.O", ImColor(128, 195, 248));
    //    editor->m_Nodes.back().Type = NodeType::Simple;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnTraceByChannelNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Start", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "End", PinType::Int);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Channel", PinType::Float);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Complex", PinType::Bool);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Actors to Ignore", PinType::Int);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Draw Debug Type", PinType::Bool);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "Ignore Self", PinType::Bool);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Out Hit", PinType::Float);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "Return Value", PinType::Bool);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnTreeSequenceNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Sequence");
    //    editor->m_Nodes.back().Type = NodeType::Tree;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnTreeTaskNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Move To");
    //    editor->m_Nodes.back().Type = NodeType::Tree;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    
    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnTreeTask2Node(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Random Wait");
    //    editor->m_Nodes.back().Type = NodeType::Tree;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnComment(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Test Comment");
    //    editor->m_Nodes.back().Type = NodeType::Comment;
    //    editor->m_Nodes.back().Size = ImVec2(300, 200);

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnHoudiniTransformNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Transform");
    //    editor->m_Nodes.back().Type = NodeType::Houdini;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    
    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    //Node* SpawnHoudiniGroupNode(BlendEditor* editor)
    //{
    //    editor->m_Nodes.emplace_back(GetNextId(), "Group");
    //    editor->m_Nodes.back().Type = NodeType::Houdini;
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    //    editor->m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    //    BuildNode(&editor->m_Nodes.back());

    //    return &editor->m_Nodes.back();
    //}

    void BuildNodes()
    {
        for (int i = 0; i < blendEditors.size(); i++)
        {
            for (int j = 0; j < blendEditors[i].m_Nodes.size(); j++)
                BuildNode(&blendEditors[i].m_Nodes[j]);
        }
        
    }

    /// <summary>
    /// Initialize affected bones with defaults
    /// </summary>
    /// <param name="editor">Current blend node editor window (blend tree)</param>
    /// <returns>Created node</returns>
    void InitAffectedBones(BlendEditor* editor)
    {
        editor->affectedBones.clear();
        editor->affectedBones =
        {
            {"mixamorig:Hips", true},
            {"mixamorig:Spine", true},
            {"mixamorig:Spine1", true},
            {"mixamorig:Spine2", true},
            {"mixamorig:Neck", true},
            {"mixamorig:Head", true},
            {"mixamorig:HeadTop_End", true},
            {"mixamorig:LeftEye", true},
            {"mixamorig:RightEye", true},
            {"mixamorig:LeftShoulder", true},
            {"mixamorig:LeftArm", true},
            {"mixamorig:LeftForeArm", true},
            {"mixamorig:LeftHand", true},
            {"mixamorig:LeftHandThumb1", true},
            {"mixamorig:LeftHandThumb2", true},
            {"mixamorig:LeftHandThumb3", true},
            {"mixamorig:LeftHandThumb4", true},
            {"mixamorig:LeftHandIndex1", true},
            {"mixamorig:LeftHandIndex2", true},
            {"mixamorig:LeftHandIndex3", true},
            {"mixamorig:LeftHandIndex4", true},
            {"mixamorig:LeftHandMiddle1", true},
            {"mixamorig:LeftHandMiddle2", true},
            {"mixamorig:LeftHandMiddle3", true},
            {"mixamorig:LeftHandMiddle4", true},
            {"mixamorig:LeftHandRing1", true},
            {"mixamorig:LeftHandRing2", true},
            {"mixamorig:LeftHandRing3", true},
            {"mixamorig:LeftHandRing4", true},
            {"mixamorig:LeftHandPinky1",  true},
            {"mixamorig:LeftHandPinky2",  true},
            {"mixamorig:LeftHandPinky3",  true},
            {"mixamorig:LeftHandPinky4",  true},
            {"mixamorig:RightShoulder", true},
            {"mixamorig:RightArm", true},
            {"mixamorig:RightForeArm", true},
            {"mixamorig:RightHand", true},
            {"mixamorig:RightHandPinky1",  true},
            {"mixamorig:RightHandPinky2",  true},
            {"mixamorig:RightHandPinky3",  true},
            {"mixamorig:RightHandPinky4",  true},
            {"mixamorig:RightHandRing1", true},
            {"mixamorig:RightHandRing2", true},
            {"mixamorig:RightHandRing3", true},
            {"mixamorig:RightHandRing4", true},
            {"mixamorig:RightHandMiddle1", true},
            {"mixamorig:RightHandMiddle2", true},
            {"mixamorig:RightHandMiddle3", true},
            {"mixamorig:RightHandMiddle4", true},
            {"mixamorig:RightHandIndex1", true},
            {"mixamorig:RightHandIndex2", true},
            {"mixamorig:RightHandIndex3", true},
            {"mixamorig:RightHandIndex4", true},
            {"mixamorig:RightHandThumb1", true},
            {"mixamorig:RightHandThumb2", true},
            {"mixamorig:RightHandThumb3", true},
            {"mixamorig:RightHandThumb4", true},
            {"mixamorig:LeftUpLeg", true},
            {"mixamorig:LeftLeg", true},
            {"mixamorig:LeftFoot", true},
            {"mixamorig:LeftToeBase", true},
            {"mixamorig:LeftToe_End", true},
            {"mixamorig:RightUpLeg", true},
            {"mixamorig:RightLeg", true},
            {"mixamorig:RightFoot", true},
            {"mixamorig:RightToeBase", true},
            {"mixamorig:RightToe_End", true}
        };
    }

    /// <summary>
    /// Contains ImGui logic for displaying the current blend tree editor window
    /// </summary>
    /// <param name="io"></param>
    void ShowEditorWindow(ImGuiIO& io)
    {
        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

        ed::SetCurrentEditor(m_Editor);

        //auto& style = ImGui::GetStyle();

# if 0
        {
            for (auto x = -io.DisplaySize.y; x < io.DisplaySize.x; x += 10.0f)
            {
                ImGui::GetWindowDrawList()->AddLine(ImVec2(x, 0), ImVec2(x + io.DisplaySize.y, io.DisplaySize.y),
                    IM_COL32(255, 255, 0, 255));
            }
        }
# endif

        static ed::NodeId contextNodeId = 0;
        static ed::LinkId contextLinkId = 0;
        static ed::PinId  contextPinId = 0;
        static bool createNewNode = false;
        static Pin* newNodeLinkPin = nullptr;
        static Pin* newLinkPin = nullptr;

        static float leftPaneWidth = 400.0f;
        static float rightPaneWidth = 800.0f;
        Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

        ShowLeftPane(leftPaneWidth - 4.0f);

        ImGui::SameLine(0.0f, 12.0f);

        ed::Begin("Node editor");
        {
            auto cursorTopLeft = ImGui::GetCursorScreenPos();

            util::BlueprintNodeBuilder builder(m_HeaderBackground, GetTextureWidth(m_HeaderBackground), GetTextureHeight(m_HeaderBackground));

            for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
            {
                if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
                    continue;

                const auto isSimple = node.Type == NodeType::Simple;

                bool hasOutputDelegates = false;
                for (auto& output : node.Outputs)
                    if (output.Type == PinType::Delegate)
                        hasOutputDelegates = true;

                builder.Begin(node.ID);
                if (!isSimple)
                {
                    builder.Header(node.Color);
                    ImGui::Spring(0);
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::Spring(1);
                    ImGui::Dummy(ImVec2(0, 28));
                    if (hasOutputDelegates)
                    {
                        ImGui::BeginVertical("delegates", ImVec2(0, 28));
                        ImGui::Spring(1, 0);
                        for (auto& output : node.Outputs)
                        {
                            if (output.Type != PinType::Delegate)
                                continue;

                            auto alpha = ImGui::GetStyle().Alpha;
                            if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                                alpha = alpha * (48.0f / 255.0f);

                            ed::BeginPin(output.ID, ed::PinKind::Output);
                            ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
                            ed::PinPivotSize(ImVec2(0, 0));
                            ImGui::BeginHorizontal(output.ID.AsPointer());
                            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                            if (!output.Name.empty())
                            {
                                ImGui::TextUnformatted(output.Name.c_str());
                                ImGui::Spring(0);
                            }
                            if (output.Type != PinType::Dropdown)
                                DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                            ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                            ImGui::EndHorizontal();
                            ImGui::PopStyleVar();
                            ed::EndPin();

                            //DrawItemRect(ImColor(255, 0, 0));
                        }
                        ImGui::Spring(1, 0);
                        ImGui::EndVertical();
                        ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                    }
                    else
                        ImGui::Spring(0);
                    builder.EndHeader();
                }

                for (auto& input : node.Inputs)
                {
                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    builder.Input(input.ID);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

                    /////////////////////// Dropdown Menus and other custom changes //////////////////////////////////
                    //Dillon Drummond

                    //Dropdown pin, don't draw icon
                    if (input.Type == PinType::Dropdown)
                    {
                        //Draw dropdown menu on node, not working
                        // 
                        //std::string name = node.Name + " (" + input.Name + ", " + std::to_string((int)&(node.ID)) + ")";

                        //// Find the longest animal var name
                        //std::string longest = "";
                        //for (int i = 0; i < (int)AnimalVar::animal_var_max; i++)
                        //{
                        //    if (longest.length() < ANIMAL_VAR_NAMES[i].length())
                        //    {
                        //        longest = ANIMAL_VAR_NAMES[i];
                        //    }
                        //}

                        ////Set width of combo box to longest animal var length
                        //ImGuiStyle& style = ImGui::GetStyle();
                        //float w = ImGui::CalcTextSize(longest.c_str()).x + style.FramePadding.x * 2.0f;
                        //float spacing = style.ItemInnerSpacing.x;
                        //const float combo_width = w;
                        //ImGui::SetNextItemWidth(combo_width);

                        ////Create combo box
                        //if (ImGui::BeginCombo(name.c_str(), input.data.c_str()))
                        //{
                        //    for (int i = 0; i < IM_ARRAYSIZE(ANIMAL_VAR_NAMES); i++)
                        //    {
                        //        bool is_selected = !strcmp(input.data.c_str(), ANIMAL_VAR_NAMES[i].c_str());
                        //        if (ImGui::Selectable(ANIMAL_VAR_NAMES[i].c_str(), is_selected))
                        //            input.data = ANIMAL_VAR_NAMES[i];
                        //        if (is_selected)
                        //            ImGui::SetItemDefaultFocus();
                        //    }

                        //    ImGui::EndCombo();
                        //}
                    }
                    else //Other pin
                    {
                        DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
                    }

                    //String pin, add textbox after icon
                    if (input.Type == PinType::String && !IsPinLinked(input.ID))
                    {
                        //Adapted from output pin in the section below
                        bool wasActive = false;

                        ImGui::PushItemWidth(100.0f);
                        char* cstr = new char[input.data.length() + 1];
                        strcpy(cstr, input.data.c_str());
                        ImGui::InputText("##edit", cstr, 127);
                        input.data = cstr;
                        ImGui::PopItemWidth();
                        if (ImGui::IsItemActive() && !wasActive)
                        {
                            ed::EnableShortcuts(false);
                            wasActive = true;
                        }
                        else if (!ImGui::IsItemActive() && wasActive)
                        {
                            ed::EnableShortcuts(true);
                            wasActive = false;
                        }
                        ImGui::Spring(0);
                    }
                    //////////////////////////////////////////////////////
                    ImGui::Spring(0);
                    if (!input.Name.empty())
                    {


                        /////
                        // Dillon Drummond
                        //Display animal var name next to dropdown pin name
                        if (input.Type == PinType::Dropdown)
                        {
                            std::string text = input.Name + ": " + input.data;
                            ImGui::TextUnformatted(text.c_str());
                        }
                        else
                        {
                            ImGui::TextUnformatted(input.Name.c_str());
                        }
                        /////

                        ImGui::Spring(0);
                    }
                    if (input.Type == PinType::Bool)
                    {
                        ImGui::Button("Hello");
                        ImGui::Spring(0);
                    }
                    ImGui::PopStyleVar();
                    builder.EndInput();
                }

                if (isSimple)
                {
                    builder.Middle();

                    ImGui::Spring(1, 0);
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::Spring(1, 0);
                }

                for (auto& output : node.Outputs)
                {
                    if (!isSimple && output.Type == PinType::Delegate)
                        continue;

                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    builder.Output(output.ID);
                    if (output.Type == PinType::String)
                    {
                        static char buffer[128] = "Edit Me\nMultiline!";
                        static bool wasActive = false;

                        ImGui::PushItemWidth(100.0f);
                        ImGui::InputText("##edit", buffer, 127);
                        ImGui::PopItemWidth();
                        if (ImGui::IsItemActive() && !wasActive)
                        {
                            ed::EnableShortcuts(false);
                            wasActive = true;
                        }
                        else if (!ImGui::IsItemActive() && wasActive)
                        {
                            ed::EnableShortcuts(true);
                            wasActive = false;
                        }
                        ImGui::Spring(0);
                    }
                    if (!output.Name.empty())
                    {
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(output.Name.c_str());
                    }
                    ImGui::Spring(0);
                    DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                    ImGui::PopStyleVar();
                    builder.EndOutput();
                }

                builder.End();
            }

            for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
            {
                if (node.Type != NodeType::Tree)
                    continue;

                const float rounding = 5.0f;
                const float padding = 12.0f;

                const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(128, 128, 128, 200));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(32, 32, 32, 200));
                ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

                ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
                ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
                ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
                ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
                ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
                ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
                ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
                ed::BeginNode(node.ID);

                ImGui::BeginVertical(node.ID.AsPointer());
                ImGui::BeginHorizontal("inputs");
                ImGui::Spring(0, padding * 2);

                ImRect inputsRect;
                int inputAlpha = 200;
                if (!node.Inputs.empty())
                {
                    auto& pin = node.Inputs[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    inputsRect = ImGui_GetItemRect();

                    ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
#if IMGUI_VERSION_NUM > 18101
                    ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersBottom);
#else
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
#endif
                    ed::BeginPin(pin.ID, ed::PinKind::Input);
                    ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar(3);

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
                else
                    ImGui::Dummy(ImVec2(0, padding));

                ImGui::Spring(0, padding * 2);
                ImGui::EndHorizontal();

                ImGui::BeginHorizontal("content_frame");
                ImGui::Spring(1, padding);

                ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
                ImGui::Dummy(ImVec2(160, 0));
                ImGui::Spring(1);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1);
                ImGui::EndVertical();
                auto contentRect = ImGui_GetItemRect();

                ImGui::Spring(1, padding);
                ImGui::EndHorizontal();

                ImGui::BeginHorizontal("outputs");
                ImGui::Spring(0, padding * 2);

                ImRect outputsRect;
                int outputAlpha = 200;
                if (!node.Outputs.empty())
                {
                    auto& pin = node.Outputs[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    outputsRect = ImGui_GetItemRect();

#if IMGUI_VERSION_NUM > 18101
                    ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_RoundCornersTop);
#else
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
#endif
                    ed::BeginPin(pin.ID, ed::PinKind::Output);
                    ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
                    ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar();

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }
                else
                    ImGui::Dummy(ImVec2(0, padding));

                ImGui::Spring(0, padding * 2);
                ImGui::EndHorizontal();

                ImGui::EndVertical();

                ed::EndNode();
                ed::PopStyleVar(7);
                ed::PopStyleColor(4);

                auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

                //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
                //const auto unitSize    = 1.0f / fringeScale;

                //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
                //{
                //    if ((col >> 24) == 0)
                //        return;
                //    drawList->PathRect(a, b, rounding, rounding_corners);
                //    drawList->PathStroke(col, true, thickness);
                //};

#if IMGUI_VERSION_NUM > 18101
                const auto    topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
                const auto bottomRoundCornersFlags = ImDrawFlags_RoundCornersBottom;
#else
                const auto    topRoundCornersFlags = 1 | 2;
                const auto bottomRoundCornersFlags = 4 | 8;
#endif

                drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, bottomRoundCornersFlags);
                //ImGui::PopStyleVar();
                drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, topRoundCornersFlags);
                //ImGui::PopStyleVar();
                drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                drawList->AddRect(
                    contentRect.GetTL(),
                    contentRect.GetBR(),
                    IM_COL32(48, 128, 255, 100), 0.0f);
                //ImGui::PopStyleVar();
            }

            for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
            {
                if (node.Type != NodeType::Houdini)
                    continue;

                const float rounding = 10.0f;
                const float padding = 12.0f;


                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(229, 229, 229, 200));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(125, 125, 125, 200));
                ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(229, 229, 229, 60));
                ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(125, 125, 125, 60));

                const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

                ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
                ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
                ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
                ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
                ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
                ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
                ed::PushStyleVar(ed::StyleVar_PinRadius, 6.0f);
                ed::BeginNode(node.ID);

                ImGui::BeginVertical(node.ID.AsPointer());
                if (!node.Inputs.empty())
                {
                    ImGui::BeginHorizontal("inputs");
                    ImGui::Spring(1, 0);

                    ImRect inputsRect;
                    int inputAlpha = 200;
                    for (auto& pin : node.Inputs)
                    {
                        ImGui::Dummy(ImVec2(padding, padding));
                        inputsRect = ImGui_GetItemRect();
                        ImGui::Spring(1, 0);
                        inputsRect.Min.y -= padding;
                        inputsRect.Max.y -= padding;

#if IMGUI_VERSION_NUM > 18101
                        const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
#else
                        const auto allRoundCornersFlags = 15;
#endif
                        //ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                        //ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                        ed::PushStyleVar(ed::StyleVar_PinCorners, allRoundCornersFlags);

                        ed::BeginPin(pin.ID, ed::PinKind::Input);
                        ed::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
                        ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                        ed::EndPin();
                        //ed::PopStyleVar(3);
                        ed::PopStyleVar(1);

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(inputsRect.GetTL(), inputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);
                        drawList->AddRect(inputsRect.GetTL(), inputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, allRoundCornersFlags);

                        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                            inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                    }

                    //ImGui::Spring(1, 0);
                    ImGui::EndHorizontal();
                }

                ImGui::BeginHorizontal("content_frame");
                ImGui::Spring(1, padding);

                ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
                ImGui::Dummy(ImVec2(160, 0));
                ImGui::Spring(1);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::PopStyleColor();
                ImGui::Spring(1);
                ImGui::EndVertical();
                auto contentRect = ImGui_GetItemRect();

                ImGui::Spring(1, padding);
                ImGui::EndHorizontal();

                if (!node.Outputs.empty())
                {
                    ImGui::BeginHorizontal("outputs");
                    ImGui::Spring(1, 0);

                    ImRect outputsRect;
                    int outputAlpha = 200;
                    for (auto& pin : node.Outputs)
                    {
                        ImGui::Dummy(ImVec2(padding, padding));
                        outputsRect = ImGui_GetItemRect();
                        ImGui::Spring(1, 0);
                        outputsRect.Min.y += padding;
                        outputsRect.Max.y += padding;

#if IMGUI_VERSION_NUM > 18101
                        const auto allRoundCornersFlags = ImDrawFlags_RoundCornersAll;
                        const auto topRoundCornersFlags = ImDrawFlags_RoundCornersTop;
#else
                        const auto allRoundCornersFlags = 15;
                        const auto topRoundCornersFlags = 3;
#endif

                        ed::PushStyleVar(ed::StyleVar_PinCorners, topRoundCornersFlags);
                        ed::BeginPin(pin.ID, ed::PinKind::Output);
                        ed::PinPivotRect(outputsRect.GetCenter(), outputsRect.GetCenter());
                        ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                        ed::EndPin();
                        ed::PopStyleVar();


                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);
                        drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR(),
                            IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, allRoundCornersFlags);


                        if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                            outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                    }

                    ImGui::EndHorizontal();
                }

                ImGui::EndVertical();

                ed::EndNode();
                ed::PopStyleVar(7);
                ed::PopStyleColor(4);

                // auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

                //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
                //const auto unitSize    = 1.0f / fringeScale;

                //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
                //{
                //    if ((col >> 24) == 0)
                //        return;
                //    drawList->PathRect(a, b, rounding, rounding_corners);
                //    drawList->PathStroke(col, true, thickness);
                //};

                //drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
                //ImGui::PopStyleVar();
                //drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                ////ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
                ////ImGui::PopStyleVar();
                //drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
                //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
                //drawList->AddRect(
                //    contentRect.GetTL(),
                //    contentRect.GetBR(),
                //    IM_COL32(48, 128, 255, 100), 0.0f);
                //ImGui::PopStyleVar();
            }

            for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
            {
                if (node.Type != NodeType::Comment)
                    continue;

                const float commentAlpha = 0.75f;

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
                ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
                ed::BeginNode(node.ID);
                ImGui::PushID(node.ID.AsPointer());
                ImGui::BeginVertical("content");
                ImGui::BeginHorizontal("horizontal");
                ImGui::Spring(1);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1);
                ImGui::EndHorizontal();
                ed::Group(node.Size);
                ImGui::EndVertical();
                ImGui::PopID();
                ed::EndNode();
                ed::PopStyleColor(2);
                ImGui::PopStyleVar();

                if (ed::BeginGroupHint(node.ID))
                {
                    //auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
                    auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

                    //ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

                    auto min = ed::GetGroupMin();
                    //auto max = ed::GetGroupMax();

                    ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                    ImGui::BeginGroup();
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::EndGroup();

                    auto drawList = ed::GetHintBackgroundDrawList();

                    auto hintBounds = ImGui_GetItemRect();
                    auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

                    drawList->AddRectFilled(
                        hintFrameBounds.GetTL(),
                        hintFrameBounds.GetBR(),
                        IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

                    drawList->AddRect(
                        hintFrameBounds.GetTL(),
                        hintFrameBounds.GetBR(),
                        IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

                    //ImGui::PopStyleVar();
                }
                ed::EndGroupHint();
            }

            for (auto& link : blendEditors[currentEditorIndex].m_Links)
                ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

            if (!createNewNode)
            {
                if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
                {
                    auto showLabel = [](const char* label, ImColor color)
                    {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                        auto size = ImGui::CalcTextSize(label);

                        auto padding = ImGui::GetStyle().FramePadding;
                        auto spacing = ImGui::GetStyle().ItemSpacing;

                        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                        auto rectMin = ImGui::GetCursorScreenPos() - padding;
                        auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                        auto drawList = ImGui::GetWindowDrawList();
                        drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                        ImGui::TextUnformatted(label);
                    };

                    ed::PinId startPinId = 0, endPinId = 0;
                    if (ed::QueryNewLink(&startPinId, &endPinId))
                    {
                        auto startPin = FindPin(startPinId);
                        auto endPin = FindPin(endPinId);

                        newLinkPin = startPin ? startPin : endPin;

                        if (startPin->Kind == PinKind::Input)
                        {
                            std::swap(startPin, endPin);
                            std::swap(startPinId, endPinId);
                        }

                        if (startPin && endPin)
                        {
                            if (endPin == startPin)
                            {
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            else if (endPin->Kind == startPin->Kind)
                            {
                                showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                            //else if (endPin->Node == startPin->Node)
                            //{
                            //    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                            //    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                            //}
                            else if (endPin->Type != startPin->Type)
                            {
                                showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                                ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                            }
                            else
                            {
                                showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                                if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                                {
                                    blendEditors[currentEditorIndex].m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                                    blendEditors[currentEditorIndex].m_Links.back().Color = GetIconColor(startPin->Type);
                                }
                            }
                        }
                    }

                    ed::PinId pinId = 0;
                    if (ed::QueryNewNode(&pinId))
                    {
                        newLinkPin = FindPin(pinId);
                        if (newLinkPin)
                            showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                        if (ed::AcceptNewItem())
                        {
                            createNewNode = true;
                            newNodeLinkPin = FindPin(pinId);
                            newLinkPin = nullptr;
                            ed::Suspend();
                            ImGui::OpenPopup("Create New Node");
                            ed::Resume();
                        }
                    }
                }
                else
                    newLinkPin = nullptr;

                ed::EndCreate();

                if (ed::BeginDelete())
                {
                    ed::NodeId nodeId = 0;
                    while (ed::QueryDeletedNode(&nodeId))
                    {
                        if (ed::AcceptDeletedItem())
                        {
                            auto id = std::find_if(blendEditors[currentEditorIndex].m_Nodes.begin(), blendEditors[currentEditorIndex].m_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                            if (id != blendEditors[currentEditorIndex].m_Nodes.end())
                                blendEditors[currentEditorIndex].m_Nodes.erase(id);
                        }
                    }

                    ed::LinkId linkId = 0;
                    while (ed::QueryDeletedLink(&linkId))
                    {
                        if (ed::AcceptDeletedItem())
                        {
                            auto id = std::find_if(blendEditors[currentEditorIndex].m_Links.begin(), blendEditors[currentEditorIndex].m_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                            if (id != blendEditors[currentEditorIndex].m_Links.end())
                                blendEditors[currentEditorIndex].m_Links.erase(id);
                        }
                    }
                }
                ed::EndDelete();
            }

            ImGui::SetCursorScreenPos(cursorTopLeft);
        }

# if 1
        auto openPopupPosition = ImGui::GetMousePos();
        ed::Suspend();
        if (ed::ShowNodeContextMenu(&contextNodeId))
            ImGui::OpenPopup("Node Context Menu");
        else if (ed::ShowPinContextMenu(&contextPinId))
            ImGui::OpenPopup("Pin Context Menu");
        else if (ed::ShowLinkContextMenu(&contextLinkId))
            ImGui::OpenPopup("Link Context Menu");
        else if (ed::ShowBackgroundContextMenu())
        {
            ImGui::OpenPopup("Create New Node");
            newNodeLinkPin = nullptr;
        }
        ed::Resume();

        ed::Suspend();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("Node Context Menu"))
        {
            auto node = FindNode(contextNodeId);

            ImGui::TextUnformatted("Node Context Menu");
            ImGui::Separator();
            if (node)
            {
                ImGui::Text("ID: %p", node->ID.AsPointer());
                ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
                ImGui::Text("Inputs: %d", (int)node->Inputs.size());
                ImGui::Text("Outputs: %d", (int)node->Outputs.size());
            }
            else
                ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteNode(contextNodeId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Pin Context Menu"))
        {
            auto pin = FindPin(contextPinId);

            ImGui::TextUnformatted("Pin Context Menu");
            ImGui::Separator();
            if (pin)
            {
                ImGui::Text("ID: %p", pin->ID.AsPointer());
                if (pin->Node)
                    ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
                else
                    ImGui::Text("Node: %s", "<none>");
            }
            else
                ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Link Context Menu"))
        {
            auto link = FindLink(contextLinkId);

            ImGui::TextUnformatted("Link Context Menu");
            ImGui::Separator();
            if (link)
            {
                ImGui::Text("ID: %p", link->ID.AsPointer());
                ImGui::Text("From: %p", link->StartPinID.AsPointer());
                ImGui::Text("To: %p", link->EndPinID.AsPointer());
            }
            else
                ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
                ed::DeleteLink(contextLinkId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Create New Node"))
        {
            auto newNodePostion = openPopupPosition;
            //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

            //auto drawList = ImGui::GetWindowDrawList();
            //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

            Node* node = nullptr;
            if (ImGui::MenuItem("Root Node"))
                node = SpawnRootNode(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Evaluate Clip Ctrl Node"))
                node = SpawnClipCtrlNode(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Blend 3 Node"))
                node = SpawnBlend3Node(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Lerp Node"))
                node = SpawnLerpNode(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Handle Jump Node"))
                node = SpawnHandleJumpNode(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Branch Node"))
                node = SpawnBranchNode(&blendEditors[currentEditorIndex]);
            if (ImGui::MenuItem("Root Node"))
                node = SpawnRootNode(&blendEditors[currentEditorIndex]);
            ImGui::Separator();
            /*if (ImGui::MenuItem("Input Action"))
                node = SpawnInputActionNode();
            if (ImGui::MenuItem("Output Action"))
                node = SpawnOutputActionNode();
            if (ImGui::MenuItem("Branch"))
                node = SpawnBranchNode();
            if (ImGui::MenuItem("Do N"))
                node = SpawnDoNNode();
            if (ImGui::MenuItem("Set Timer"))
                node = SpawnSetTimerNode();
            if (ImGui::MenuItem("Less"))
                node = SpawnLessNode();
            if (ImGui::MenuItem("Weird"))
                node = SpawnWeirdNode();
            if (ImGui::MenuItem("Trace by Channel"))
                node = SpawnTraceByChannelNode();
            if (ImGui::MenuItem("Print String"))
                node = SpawnPrintStringNode();
            ImGui::Separator();
            if (ImGui::MenuItem("Comment"))
                node = SpawnComment();
            ImGui::Separator();
            if (ImGui::MenuItem("Sequence"))
                node = SpawnTreeSequenceNode();
            if (ImGui::MenuItem("Move To"))
                node = SpawnTreeTaskNode();
            if (ImGui::MenuItem("Random Wait"))
                node = SpawnTreeTask2Node();
            ImGui::Separator();*/
            /*if (ImGui::MenuItem("String"))
                node = SpawnStringNode(&blendEditors[currentEditorIndex]);*/
            /*ImGui::Separator();
            if (ImGui::MenuItem("Transform"))
                node = SpawnHoudiniTransformNode();
            if (ImGui::MenuItem("Group"))
                node = SpawnHoudiniGroupNode();*/

            if (node)
            {
                BuildNodes();

                createNewNode = false;

                ed::SetNodePosition(node->ID, newNodePostion);

                if (auto startPin = newNodeLinkPin)
                {
                    auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

                    for (auto& pin : pins)
                    {
                        if (CanCreateLink(startPin, &pin))
                        {
                            auto endPin = &pin;
                            if (startPin->Kind == PinKind::Input)
                                std::swap(startPin, endPin);

                            blendEditors[currentEditorIndex].m_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
                            blendEditors[currentEditorIndex].m_Links.back().Color = GetIconColor(startPin->Type);

                            break;
                        }
                    }
                }
            }

            ImGui::EndPopup();
        }
        else
            createNewNode = false;
        ImGui::PopStyleVar();
        ed::Resume();
# endif


        /*
            cubic_bezier_t c;
            c.p0 = pointf(100, 600);
            c.p1 = pointf(300, 1200);
            c.p2 = pointf(500, 100);
            c.p3 = pointf(900, 600);

            auto drawList = ImGui::GetWindowDrawList();
            auto offset_radius = 15.0f;
            auto acceptPoint = [drawList, offset_radius](const bezier_subdivide_result_t& r)
            {
                drawList->AddCircle(to_imvec(r.point), 4.0f, IM_COL32(255, 0, 255, 255));

                auto nt = r.tangent.normalized();
                nt = pointf(-nt.y, nt.x);

                drawList->AddLine(to_imvec(r.point), to_imvec(r.point + nt * offset_radius), IM_COL32(255, 0, 0, 255), 1.0f);
            };

            drawList->AddBezierCurve(to_imvec(c.p0), to_imvec(c.p1), to_imvec(c.p2), to_imvec(c.p3), IM_COL32(255, 255, 255, 255), 1.0f);
            cubic_bezier_subdivide(acceptPoint, c);
        */

        ed::End();

        auto editorMin = ImGui::GetItemRectMin();
        auto editorMax = ImGui::GetItemRectMax();

        if (m_ShowOrdinals)
        {
            int nodeCount = ed::GetNodeCount();
            std::vector<ed::NodeId> orderedNodeIds;
            orderedNodeIds.resize(static_cast<size_t>(nodeCount));
            ed::GetOrderedNodeIds(orderedNodeIds.data(), nodeCount);


            auto drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(editorMin, editorMax);

            int ordinal = 0;
            for (auto& nodeId : orderedNodeIds)
            {
                auto p0 = ed::GetNodePosition(nodeId);
                auto p1 = p0 + ed::GetNodeSize(nodeId);
                p0 = ed::CanvasToScreen(p0);
                p1 = ed::CanvasToScreen(p1);


                ImGuiTextBuffer builder;
                builder.appendf("#%d", ordinal++);

                auto textSize = ImGui::CalcTextSize(builder.c_str());
                auto padding = ImVec2(2.0f, 2.0f);
                auto widgetSize = textSize + padding * 2;

                auto widgetPosition = ImVec2(p1.x, p0.y) + ImVec2(0.0f, -widgetSize.y);

                drawList->AddRectFilled(widgetPosition, widgetPosition + widgetSize, IM_COL32(100, 80, 80, 190), 3.0f, ImDrawFlags_RoundCornersAll);
                drawList->AddRect(widgetPosition, widgetPosition + widgetSize, IM_COL32(200, 160, 160, 190), 3.0f, ImDrawFlags_RoundCornersAll);
                drawList->AddText(widgetPosition + padding, IM_COL32(255, 255, 255, 255), builder.c_str());
            }

            drawList->PopClipRect();
        }


        //ImGui::ShowTestWindow();
        //ImGui::ShowMetricsWindow();
    }

    //////////// Blend Tree JSON I/O //////////////
    // aster

    void ShowSaveWindow()
    {
        // toggles a window that displays the following:
        // 1. file save location
        // 2. file name
        // 3. save button (bind to SaveBlendTreeToJSON())
        // 4. show user feedback of fail/success
        //char* cstr = new char[saveData.path.length() + 1];
        //strcpy(cstr, saveData.path.c_str());
        //ImGui::InputText("Save Path", cstr, 127);
        //saveData.path = cstr;

        if (ImGui::Button("Save File"))
        {
            SaveBlendTreeToJSON();
        }
    }


	// Getting rid of loading due to scope
    void ShowLoadWindow()
    {
        // toggles a window that displays the following:
        // 1. file select
        // 2. load button (bind to LoadBlendTreeFromJSON())
        // 3. show user feedback of fail/success

        char* cstr = new char[loadData.path.length() + 1];
        strcpy(cstr, loadData.path.c_str());
        ImGui::InputText("Load Path", cstr, 127);
        loadData.path = cstr;

        if (ImGui::Button("Load File"))
        {
            LoadBlendTreeFromJSON();
        }
    }

    std::string GetTreeString(Node* node, BlendEditor blendGraph, std::string existingTreeStr, 
        std::unordered_set<Node*>& visitedNodes)
    {
        if (visitedNodes.find(node) != visitedNodes.end()) return existingTreeStr; // Already visited node, skip

        visitedNodes.insert(node);

        for (int i = 0; i < node->Inputs.size(); i++)
        {
            Pin* connectedPin = blendGraph.GetOutputConnectedPin(&(node->Inputs[i]));

            if (connectedPin == nullptr) continue;

            existingTreeStr = GetTreeString(connectedPin->Node, blendGraph, existingTreeStr, visitedNodes);
        }

        bool lastDataEntry = node->Inputs.size() <= 1;
        bool isRoot = false;

        std::string blendOp = node->blendOp;

        // Root will have no id, replace with correct json key "root"
        std::string id = node->Inputs[0].data;
        if (node->Inputs[0].data == "")
        {
            id = "root";
            lastDataEntry = true;
            isRoot = true;
            blendOp = blendGraph.GetOutputConnectedPin(&(node->Inputs[0]))->Node->Inputs[0].data; // Get name of connected node
        }

        existingTreeStr += "\t\t\"" + id + "\": {\n";
        existingTreeStr += "\t\t\t\"blendOp\": \"" + blendOp + "\"";

        if (!lastDataEntry)
        {
            existingTreeStr += ",";
        }

        existingTreeStr += "\n";

        int numSpatial = 0;
        int numParam = 0;
        int numMisc = 0;

        for (int i = 1; i < node->Inputs.size(); i++)
        {
            //existingTreeStr += node->Inputs[i].data

            if (i + 1 >= node->Inputs.size()) lastDataEntry = true;

            existingTreeStr += "\t\t\t\"";

            switch (node->Inputs[i].dataGroup)
            {
                case DataGroup::Spatial:
                {
                    existingTreeStr += "spatialDataNodes" + std::to_string(numSpatial);
                    numSpatial++;
                    break;
                }

                case DataGroup::Param:
                {
                    existingTreeStr += "paramData" + std::to_string(numParam);
                    numParam++;
                    break;
                }

                case DataGroup::Misc:
                {
                    existingTreeStr += "miscData" + std::to_string(numMisc);
                    numMisc++;
                    break;
                }
            }

            // Add animal_var_....
            existingTreeStr += "\": ";

            Pin* connectedPin = blendGraph.GetOutputConnectedPin(&(node->Inputs[i]));

            if (connectedPin == nullptr) // Not connected to anything
            {
                existingTreeStr += "\"animal_var_" + node->Inputs[i].data + "\"";
            }
            else
            {
                existingTreeStr += "\"" + connectedPin->Node->Inputs[0].data + "\"";
            }

            if (!lastDataEntry)
            {
                existingTreeStr += ",";
            }

            existingTreeStr += "\n";
        }

        existingTreeStr += "\t\t}";

        if (!isRoot)
        {
            existingTreeStr += ",";
        }

        existingTreeStr += "\n\n";

        return existingTreeStr;
    }

    std::string GetTreeString(Node* node, BlendEditor blendGraph)
    {
        std::unordered_set<Node*> visitedNodes;
        return GetTreeString(node, blendGraph, "", visitedNodes);
    }

    bool SaveBlendTreeToJSON()
    {
        std::ofstream fout;

        CreateDirectory("BlendGraphOutput", NULL);
        fout.open("BlendGraphOutput/blendtree.json");

        //Failed to save
        if (!fout || fout.fail())
        {
            return false;
        }

        fout << "{\n";

        for (int treeIndex = 0; treeIndex < blendEditors.size(); treeIndex++)
        {
            blendEditors[treeIndex].InitializeConnectionDataMaps();

            //Print blend tree name
            fout << "\t\"blendTree" + std::to_string(treeIndex) + "\": {\n\n";

            //Print target joint array
            fout << "\t\t\"target_joints\": [" << std::endl;

            for (std::unordered_map<std::string, bool>::iterator it = blendEditors[treeIndex].affectedBones.begin(); it != blendEditors[treeIndex].affectedBones.end(); it++)
            {
                if (it->second == true)
                {
                    fout << "\t\t\t\"" + it->first + "\"";

                    if (std::next(it) != blendEditors[treeIndex].affectedBones.end())
                    {
                        fout << ",";
                    }

                    fout << std::endl;
                }
            }

            fout << "\t\t]," << std::endl;

            //Print number of nodes
            fout << std::endl << "\t\t\"node_num\": " + std::to_string(blendEditors[treeIndex].m_Nodes.size()) + "," << std::endl << std::endl;

            std::string a = GetTreeString(&(blendEditors[treeIndex].m_Nodes[0]), blendEditors[treeIndex]);
            fout << a;

            //add deepest node (root);
            fout << "\t}";

			if (treeIndex < blendEditors.size() - 1)
			{
				fout << ",";
			}

			fout << "\n";
        }

        fout << "}";

        fout.close();

        return true;

        // get nodes from m_Nodes

        // 1. find LAST node (root)
        // 2. get input from root node
        // 3. handle node based on its blendOp:
        // 
        //      blendop_bool_branch
        //          param, input1, input2
        // 
        //      blendop_handle_jump
        //          duration, height, fadeintime, fadeouttime, timesincejump, lerp param, isjumping, ctrlnode, output pose
        // 
        //      blendop_lerp
        //          input 1, output pose, param
        // 
        //      blendop_blend_3
        //          Inputs: Magnitude, Threshold 1, Threshold 2, Threshold 3, Input 1, Input 2, Input 3
        //          Outputs: Output Pose
        // 
        //      blendop_evaluate_clip_controller
        //          Inputs: input clip ctrl, input hierarchy pose
        //          Outputs: Output Pose

        /*
        m_Nodes.back().Inputs
        m_Nodes.back().Outputs
        */



        // write to json
        // example from https://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
        
        //Json::Value event;
        //Json::Value vec(Json::arrayValue);
        //vec.append(Json::Value(1));
        //vec.append(Json::Value(2));
        //vec.append(Json::Value(3));

        //event["competitors"]["home"]["name"] = "Liverpool";
        //event["competitors"]["away"]["code"] = 89223;
        //event["competitors"]["away"]["name"] = "Aston Villa";
        //event["competitors"]["away"]["code"]=vec;

        ////std::cout << event << std::endl;

        //std::ofstream myfile;
        //myfile.open ("../../../../blueprints-example/example.json");
        //myfile << event;
        //myfile.close();

        // output:
        /*
        {
                "competitors" :
                {
                        "away" :
                        {
                                "code" : [ 1, 2, 3 ],
                                "name" : "Aston Villa"
                        },
                        "home" :
                        {
                                "name" : "Liverpool"
                        }
                }
        }
        */
    }

    void LoadBlendTreeFromJSON()
    {
       // // load json file
       // // example from https://stackoverflow.com/questions/32205981/reading-json-files-in-c

       // // FOR DEBUG ONLY!!! REMOVE LATER!!!
       // // opens a console window and binds cout/cin to print to it (since cout is not supported by visual studio)
       // AllocConsole();
       // freopen("CONOUT$", "w", stdout);
       // freopen("CONOUT$", "w", stderr);
       // 
       // // using a placeholder for testing - needs to be swapped out by user's selected file
       // std::ifstream blendtreeFile("../../../../blueprints-example/blendtree.json", std::ifstream::binary);
       // Json::Value blendtree;
       // blendtreeFile >> blendtree; // convert file into json "object"

       // std::cout << blendtree << std::endl; //This will print the entire json object.

       // // spawn nodes depending on blendOp
       // // modified from https://stackoverflow.com/questions/4800605/iterating-through-objects-in-jsoncpp

       // for (Json::Value::const_iterator itr = blendtree.begin(); itr != blendtree.end(); itr++) {

       //     PrintJSONValue(itr.key());
       //     printf("\n");

       ////     printf(itr.key()["blendOp"].asString().c_str());
       //     /*for (Json::Value::const_iterator itrChild = header.begin(); itrChild != header.end(); itrChild++) {
       //         PrintJSONValue(itrChild.key());
       //         printf("\n");
       //     }*/
       // }
       // 

       // // example:
       // // 
       // // Node* node;
       // // node = SpawnDoNNode();              
       // // ed::SetNodePosition(node->ID, ImVec2(-238, 504));
       //// Node* node;
    }

    //from https://stackoverflow.com/questions/4800605/iterating-through-objects-in-jsoncpp
    // for testing only - remove later
    /*void PrintJSONValue(const Json::Value& val)
    {
        if (val.isString()) {
            printf("string(%s)", val.asString().c_str());
        }
        else if (val.isBool()) {
            printf("bool(%d)", val.asBool());
        }
        else if (val.isInt()) {
            printf("int(%d)", val.asInt());
        }
        else if (val.isUInt()) {
            printf("uint(%u)", val.asUInt());
        }
        else if (val.isDouble()) {
            printf("double(%f)", val.asDouble());
        }
        else
        {
            printf("unknown type=[%d]", val.type());
        }
    }*/

    //// from https://stackoverflow.com/questions/4800605/iterating-through-objects-in-jsoncpp
    //// for testing only - remove later
    //bool PrintJSONTree(const Json::Value& root, unsigned short depth /* = 0 */)
    //{
    //    depth += 1;
    //    printf(" {type=[%d], size=%d}", root.type(), root.size());

    //    if (root.size() > 0) {
    //        printf("\n");
    //        for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++) {
    //            // Print depth. 
    //            for (int tab = 0; tab < depth; tab++) {
    //                printf("-");
    //            }
    //            printf(" subvalue(");
    //            PrintJSONValue(itr.key());
    //            printf(") -");
    //            PrintJSONTree(*itr, depth);
    //        }
    //        return true;
    //    }
    //    else {
    //        printf(" ");
    //        PrintJSONValue(root);
    //        printf("\n");
    //    }
    //    return true;
    //}

    //////////////////////////////////////////////

    void OnStart() override
    {
        ed::Config config;

        config.SettingsFile = "Blueprints.json";

        config.UserPointer = this;

        config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
        {
            auto self = static_cast<Example*>(userPointer);

            auto node = self->FindNode(nodeId);
            if (!node)
                return 0;

            if (data != nullptr)
                memcpy(data, node->State.data(), node->State.size());
            return node->State.size();
        };

        config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
        {
            auto self = static_cast<Example*>(userPointer);

            auto node = self->FindNode(nodeId);
            if (!node)
                return false;

            node->State.assign(data, size);

            self->TouchNode(nodeId);

            return true;
        };

        m_Editor = ed::CreateEditor(&config);
        ed::SetCurrentEditor(m_Editor);

        Node* node;

        //Create legs/torso blend tree
        node = SpawnRootNode(&blendEditors[0]);         ed::SetNodePosition(node->ID, ImVec2(0, 500));          //0

        node = SpawnBranchNode(&blendEditors[0]);       ed::SetNodePosition(node->ID, ImVec2(-500, 500));       //1
        node->Inputs[0].data = "jumpBranchNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_isJumping];

        node = SpawnHandleJumpNode(&blendEditors[0]);   ed::SetNodePosition(node->ID, ImVec2(-1000, 250));      //2
        node->Inputs[0].data = "handleJumpNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpDuration];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpHeight];
        node->Inputs[3].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpFadeInTime];
        node->Inputs[4].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpFadeOutTime];
        node->Inputs[5].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_timeSinceJump];
        node->Inputs[6].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpLerpParam];
        node->Inputs[7].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_isJumping];
        node->Inputs[8].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_ctrlNode];

        node = SpawnLerpNode(&blendEditors[0]);         ed::SetNodePosition(node->ID, ImVec2(-1500, 500));      //3
        node->Inputs[0].data = "jumpGroundLerpNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpLerpParam];

        node = SpawnBlend3Node(&blendEditors[0]);       ed::SetNodePosition(node->ID, ImVec2(-2000, 750));      //4
        node->Inputs[0].data = "blendGroundPoseNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_ctrlVelocityMagnitude];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_idleBlendThreshold];
        node->Inputs[3].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_walkBlendThreshold];
        node->Inputs[4].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_runBlendThreshold];

        node = SpawnClipCtrlNode(&blendEditors[0]);     ed::SetNodePosition(node->ID, ImVec2(-2200, 500));      //5
        node->Inputs[0].data = "jumpCCNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_jumpClipCtrl];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_hierarchyPoseGroup_skel];

        node = SpawnClipCtrlNode(&blendEditors[0]);     ed::SetNodePosition(node->ID, ImVec2(-2700, 700));      //6
        node->Inputs[0].data = "idleCCNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_idleClipCtrl];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_hierarchyPoseGroup_skel];

        node = SpawnClipCtrlNode(&blendEditors[0]);     ed::SetNodePosition(node->ID, ImVec2(-2700, 850));      //7
        node->Inputs[0].data = "walkCCNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_walkClipCtrl];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_hierarchyPoseGroup_skel];

        node = SpawnClipCtrlNode(&blendEditors[0]);     ed::SetNodePosition(node->ID, ImVec2(-2700, 1000));     //8
        node->Inputs[0].data = "runCCNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_runClipCtrl];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_hierarchyPoseGroup_skel];

        //Disable arm/hand bones
        for (int i = 9; i < 57; i++)
        {
            blendEditors[0].affectedBones[BONE_NAMES[i]] = false;
        }


        //Create arms blend tree
        node = SpawnRootNode(&blendEditors[1]);         ed::SetNodePosition(node->ID, ImVec2(0, 500));          //0

        node = SpawnClipCtrlNode(&blendEditors[1]);     ed::SetNodePosition(node->ID, ImVec2(-1000, 500));      //5
        node->Inputs[0].data = "upperIdlePistolNode";
        node->Inputs[1].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_idlePistolClipCtrl];
        node->Inputs[2].data = ANIMAL_VAR_NAMES[(int)AnimalVar::animal_var_hierarchyPoseGroup_skel];

        //Disable any bones that aren't in the arms/hands
        for (int i = 0; i < 9; i++)
        {
            blendEditors[1].affectedBones[BONE_NAMES[i]] = false;
        }
        for (int i = 57; i < BONE_NAMES.size(); i++)
        {
            blendEditors[1].affectedBones[BONE_NAMES[i]] = false;
        }

        ed::NavigateToContent();

        BuildNodes();

        std::vector<Link>* links = &blendEditors[0].m_Links;

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[1].Outputs[0].ID, blendEditors[0].m_Nodes[0].Inputs[0].ID));

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[2].Outputs[0].ID, blendEditors[0].m_Nodes[1].Inputs[2].ID));

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[3].Outputs[0].ID, blendEditors[0].m_Nodes[2].Inputs[9].ID));

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[4].Outputs[0].ID, blendEditors[0].m_Nodes[3].Inputs[3].ID));
        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[4].Outputs[0].ID, blendEditors[0].m_Nodes[1].Inputs[3].ID));

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[5].Outputs[0].ID, blendEditors[0].m_Nodes[3].Inputs[2].ID));

        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[6].Outputs[0].ID, blendEditors[0].m_Nodes[4].Inputs[5].ID));
        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[7].Outputs[0].ID, blendEditors[0].m_Nodes[4].Inputs[6].ID));
        links->push_back(Link(GetNextLinkId(), blendEditors[0].m_Nodes[8].Outputs[0].ID, blendEditors[0].m_Nodes[4].Inputs[7].ID));



        links = &blendEditors[1].m_Links;

        links->push_back(Link(GetNextLinkId(), blendEditors[1].m_Nodes[1].Outputs[0].ID, blendEditors[1].m_Nodes[0].Inputs[0].ID));

        /*node = SpawnInputActionNode();      ed::SetNodePosition(node->ID, ImVec2(-252, 220));
        node = SpawnBranchNode();           ed::SetNodePosition(node->ID, ImVec2(-300, 351));
        node = SpawnDoNNode();              ed::SetNodePosition(node->ID, ImVec2(-238, 504));
        node = SpawnOutputActionNode();     ed::SetNodePosition(node->ID, ImVec2(71, 80));
        node = SpawnSetTimerNode();         ed::SetNodePosition(node->ID, ImVec2(168, 316));

        node = SpawnTreeSequenceNode();     ed::SetNodePosition(node->ID, ImVec2(1028, 329));
        node = SpawnTreeTaskNode();         ed::SetNodePosition(node->ID, ImVec2(1204, 458));
        node = SpawnTreeTask2Node();        ed::SetNodePosition(node->ID, ImVec2(868, 538));

        node = SpawnComment();              ed::SetNodePosition(node->ID, ImVec2(112, 576)); ed::SetGroupSize(node->ID, ImVec2(384, 154));
        node = SpawnComment();              ed::SetNodePosition(node->ID, ImVec2(800, 224)); ed::SetGroupSize(node->ID, ImVec2(640, 400));

        node = SpawnLessNode();             ed::SetNodePosition(node->ID, ImVec2(366, 652));
        node = SpawnWeirdNode();            ed::SetNodePosition(node->ID, ImVec2(144, 652));
        node = SpawnMessageNode();          ed::SetNodePosition(node->ID, ImVec2(-348, 698));
        node = SpawnPrintStringNode();      ed::SetNodePosition(node->ID, ImVec2(-69, 652));

        node = SpawnHoudiniTransformNode(); ed::SetNodePosition(node->ID, ImVec2(500, -70));
        node = SpawnHoudiniGroupNode();     ed::SetNodePosition(node->ID, ImVec2(500, 42));

        ed::NavigateToContent();

        BuildNodes();

        m_Links.push_back(Link(GetNextLinkId(), m_Nodes[5].Outputs[0].ID, m_Nodes[6].Inputs[0].ID));
        m_Links.push_back(Link(GetNextLinkId(), m_Nodes[5].Outputs[0].ID, m_Nodes[7].Inputs[0].ID));

        m_Links.push_back(Link(GetNextLinkId(), m_Nodes[14].Outputs[0].ID, m_Nodes[15].Inputs[0].ID));*/

        m_HeaderBackground = LoadTexture("data/BlueprintBackground.png");
        m_SaveIcon = LoadTexture("data/ic_save_white_24dp.png");
        m_RestoreIcon = LoadTexture("data/ic_restore_white_24dp.png");


        //auto& io = ImGui::GetIO();
    }

    void OnStop() override
    {
        auto releaseTexture = [this](ImTextureID& id)
        {
            if (id)
            {
                DestroyTexture(id);
                id = nullptr;
            }
        };

        releaseTexture(m_RestoreIcon);
        releaseTexture(m_SaveIcon);
        releaseTexture(m_HeaderBackground);

        if (m_Editor)
        {
            ed::DestroyEditor(m_Editor);
            m_Editor = nullptr;
        }
    }

    ImColor GetIconColor(PinType type)
    {
        switch (type)
        {
        default:
        case PinType::Flow:     return ImColor(255, 255, 255);
        case PinType::Bool:     return ImColor(220, 48, 48);
        case PinType::Int:      return ImColor(68, 201, 156);
        case PinType::Float:    return ImColor(147, 226, 74);
        case PinType::String:   return ImColor(124, 21, 153);
        case PinType::Object:   return ImColor(51, 150, 215);
        case PinType::Function: return ImColor(218, 0, 183);
        case PinType::Delegate: return ImColor(255, 48, 48);
        case PinType::Dropdown: return ImColor(100, 100, 100);
        }
    };

    void DrawPinIcon(const Pin& pin, bool connected, int alpha)
    {
        IconType iconType;
        ImColor  color = GetIconColor(pin.Type);
        color.Value.w = alpha / 255.0f;
        switch (pin.Type)
        {
        case PinType::Flow:     iconType = IconType::Flow;   break;
        case PinType::Bool:     iconType = IconType::Circle; break;
        case PinType::Int:      iconType = IconType::Circle; break;
        case PinType::Float:    iconType = IconType::Circle; break;
        case PinType::String:   iconType = IconType::Circle; break;
        case PinType::Object:   iconType = IconType::Circle; break;
        case PinType::Function: iconType = IconType::Circle; break;
        case PinType::Delegate: iconType = IconType::Square; break;
        default:
            return;
        }

        ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
    };

    void ShowStyleEditor(bool* show = nullptr)
    {
        if (!ImGui::Begin("Style", show))
        {
            ImGui::End();
            return;
        }

        auto paneWidth = ImGui::GetContentRegionAvail().x;

        auto& editorStyle = ed::GetStyle();
        ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Values");
        ImGui::Spring();
        if (ImGui::Button("Reset to defaults"))
            editorStyle = ed::Style();
        ImGui::EndHorizontal();
        ImGui::Spacing();
        ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Hovered Node Border Offset", &editorStyle.HoverNodeBorderOffset, 0.1f, -40.0f, 40.0f);
        ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Selected Node Border Offset", &editorStyle.SelectedNodeBorderOffset, 0.1f, -40.0f, 40.0f);
        ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
        //ImVec2  SourceDirection;
        //ImVec2  TargetDirection;
        ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
        ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
        ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
        ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
        //ImVec2  PivotAlignment;
        //ImVec2  PivotSize;
        //ImVec2  PivotScale;
        //float   PinCorners;
        //float   PinRadius;
        //float   PinArrowSize;
        //float   PinArrowWidth;
        ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

        ImGui::Separator();

        static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
        ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Filter Colors");
        ImGui::Spring();
        ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_DisplayRGB);
        ImGui::Spring(0);
        ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_DisplayHSV);
        ImGui::Spring(0);
        ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_DisplayHex);
        ImGui::EndHorizontal();

        static ImGuiTextFilter filter;
        filter.Draw("##filter", paneWidth);

        ImGui::Spacing();

        ImGui::PushItemWidth(-160);
        for (int i = 0; i < ed::StyleColor_Count; ++i)
        {
            auto name = ed::GetStyleColorName((ed::StyleColor)i);
            if (!filter.PassFilter(name))
                continue;

            ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
        }
        ImGui::PopItemWidth();

        ImGui::End();
    }

    void ShowLeftPane(float paneWidth)
    {
        auto& io = ImGui::GetIO();

        ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

        paneWidth = ImGui::GetContentRegionAvail().x;

        static bool showStyleEditor = false;
        ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
        ImGui::Spring(0.0f, 0.0f);
        if (ImGui::Button("Zoom to Content"))
            ed::NavigateToContent();
        ImGui::Spring(0.0f);
        if (ImGui::Button("Show Flow"))
        {
            for (auto& link : blendEditors[currentEditorIndex].m_Links)
                ed::Flow(link.ID);
        }
        ImGui::EndHorizontal();
        ImGui::BeginHorizontal("Blend Files", ImVec2(paneWidth, 0));
        /////////// FOR JSON I/O /////////////
        /*if (ImGui::Button("Save Blend File"))
            ShowSaveWindow();
        if (ImGui::Button("Load Blend File"))
            ShowLoadWindow();*/
        //////////////////////////////////////
        ImGui::Spring();
        if (ImGui::Button("Edit Style"))
            showStyleEditor = true;
        ImGui::EndHorizontal();
        ImGui::Checkbox("Show Ordinals", &m_ShowOrdinals);

        if (showStyleEditor)
            ShowStyleEditor(&showStyleEditor);

        std::vector<ed::NodeId> selectedNodes;
        std::vector<ed::LinkId> selectedLinks;
        selectedNodes.resize(ed::GetSelectedObjectCount());
        selectedLinks.resize(ed::GetSelectedObjectCount());

        int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
        int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

        selectedNodes.resize(nodeCount);
        selectedLinks.resize(linkCount);

        int saveIconWidth = GetTextureWidth(m_SaveIcon);
        int saveIconHeight = GetTextureWidth(m_SaveIcon);
        int restoreIconWidth = GetTextureWidth(m_RestoreIcon);
        int restoreIconHeight = GetTextureWidth(m_RestoreIcon);
        

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
            ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::TextUnformatted("Nodes");
        ImGui::Indent();
        for (auto& node : blendEditors[currentEditorIndex].m_Nodes)
        {
            ImGui::PushID(node.ID.AsPointer());
            auto start = ImGui::GetCursorScreenPos();

            if (const auto progress = GetTouchProgress(node.ID))
            {
                ImGui::GetWindowDrawList()->AddLine(
                    start + ImVec2(-8, 0),
                    start + ImVec2(-8, ImGui::GetTextLineHeight()),
                    IM_COL32(255, 0, 0, 255 - (int)(255 * progress)), 4.0f);
            }

            bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), node.ID) != selectedNodes.end();
# if IMGUI_VERSION_NUM >= 18967
            ImGui::SetNextItemAllowOverlap();
# endif
            if (ImGui::Selectable((node.Name + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer()))).c_str(), &isSelected))
            {
                if (io.KeyCtrl)
                {
                    if (isSelected)
                        ed::SelectNode(node.ID, true);
                    else
                        ed::DeselectNode(node.ID);
                }
                else
                    ed::SelectNode(node.ID, false);

                ed::NavigateToSelection();
            }
            if (ImGui::IsItemHovered() && !node.State.empty())
                ImGui::SetTooltip("State: %s", node.State.c_str());

            auto id = std::string("(") + std::to_string(reinterpret_cast<uintptr_t>(node.ID.AsPointer())) + ")";
            auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);
            auto iconPanelPos = start + ImVec2(
                paneWidth - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().IndentSpacing - saveIconWidth - restoreIconWidth - ImGui::GetStyle().ItemInnerSpacing.x * 1,
                (ImGui::GetTextLineHeight() - saveIconHeight) / 2);
            ImGui::GetWindowDrawList()->AddText(
                ImVec2(iconPanelPos.x - textSize.x - ImGui::GetStyle().ItemInnerSpacing.x, start.y),
                IM_COL32(255, 255, 255, 255), id.c_str(), nullptr);

            auto drawList = ImGui::GetWindowDrawList();
            ImGui::SetCursorScreenPos(iconPanelPos);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# else
            ImGui::SetNextItemAllowOverlap();
# endif
            if (node.SavedState.empty())
            {
                if (ImGui::InvisibleButton("save", ImVec2((float)saveIconWidth, (float)saveIconHeight)))
                    node.SavedState = node.State;

                if (ImGui::IsItemActive())
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
                else if (ImGui::IsItemHovered())
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                else
                    drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
            }
            else
            {
                ImGui::Dummy(ImVec2((float)saveIconWidth, (float)saveIconHeight));
                drawList->AddImage(m_SaveIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
            }

            ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# else
            ImGui::SetNextItemAllowOverlap();
# endif
            if (!node.SavedState.empty())
            {
                if (ImGui::InvisibleButton("restore", ImVec2((float)restoreIconWidth, (float)restoreIconHeight)))
                {
                    node.State = node.SavedState;
                    ed::RestoreNodeState(node.ID);
                    node.SavedState.clear();
                }

                if (ImGui::IsItemActive())
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 96));
                else if (ImGui::IsItemHovered())
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
                else
                    drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 160));
            }
            else
            {
                ImGui::Dummy(ImVec2((float)restoreIconWidth, (float)restoreIconHeight));
                drawList->AddImage(m_RestoreIcon, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 32));
            }

            ImGui::SameLine(0, 0);
# if IMGUI_VERSION_NUM < 18967
            ImGui::SetItemAllowOverlap();
# endif
            ImGui::Dummy(ImVec2(0, (float)restoreIconHeight));

            ImGui::PopID();
        }
        ImGui::Unindent();

        static int changeCount = 0;

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
            ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::TextUnformatted("Selection");

        ImGui::BeginHorizontal("Selection Stats", ImVec2(paneWidth, 0));
        ImGui::Text("Changed %d time%s", changeCount, changeCount > 1 ? "s" : "");
        ImGui::Spring();
        if (ImGui::Button("Deselect All"))
            ed::ClearSelection();
        ImGui::EndHorizontal();
        ImGui::Indent();
        for (int i = 0; i < nodeCount; ++i)
        {
            //ImGui::Text("Node (%p)", selectedNodes[i].AsPointer());

            ///////////////// Dropdown Menu Implementation ////////////////////
            //Dillon Drummond
            Node* node = FindNode(selectedNodes[i]);

            ImGui::Text("Node Name: %s - %s", node->Name.c_str(), node->Inputs[0].data.c_str());

            for (int pinIndex = 0; pinIndex < node->Inputs.size(); pinIndex++)
            {
                Pin* currentPin = &node->Inputs[pinIndex];
                //Dropdown pin, don't draw icon
                if (currentPin->Type == PinType::Dropdown)
                {
                    // Find the longest animal var name
                    std::string longest = "";
                    for (int i = 0; i < (int)AnimalVar::animal_var_max; i++)
                    {
                        if (longest.length() < ANIMAL_VAR_NAMES[i].length())
                        {
                            longest = ANIMAL_VAR_NAMES[i];
                        }
                    }

                    //Set width of combo box to longest animal var length
                    ImGuiStyle& style = ImGui::GetStyle();
                    float w = ImGui::CalcTextSize(longest.c_str()).x + style.FramePadding.x * 2.0f;
                    float spacing = style.ItemInnerSpacing.x;
                    const float combo_width = w;
                    ImGui::SetNextItemWidth(combo_width);

                    //Unique ID using pointer address, node name, and pin name
                    std::string name = currentPin->Name + " (" + node->Name + ", " + std::to_string((int)&(node->ID)) + ")";

                    //Create combo box
                    if (ImGui::BeginCombo(name.c_str(), currentPin->data.c_str()))
                    {
                        for (int i = 0; i < IM_ARRAYSIZE(ANIMAL_VAR_NAMES); i++)
                        {
                            bool is_selected = !strcmp(currentPin->data.c_str(), ANIMAL_VAR_NAMES[i].c_str());
                            if (ImGui::Selectable(ANIMAL_VAR_NAMES[i].c_str(), is_selected))
                                currentPin->data = ANIMAL_VAR_NAMES[i];
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }

                        ImGui::EndCombo();
                    }
                }
            }

            ImGui::Spacing();
        }

        for (int i = 0; i < linkCount; ++i) ImGui::Text("Link (%p)", selectedLinks[i].AsPointer());
        ImGui::Unindent();
        
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
            for (auto& link : blendEditors[currentEditorIndex].m_Links)
                ed::Flow(link.ID);

        if (ed::HasSelectionChanged())
            ++changeCount;

        //Bone list for animation layering
        ImGui::GetWindowDrawList()->AddRectFilled(
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2(paneWidth, ImGui::GetTextLineHeight()),
            ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), ImGui::GetTextLineHeight() * 0.25f);
        ImGui::Spacing(); ImGui::SameLine();
        ImGui::TextUnformatted("Affected Bones");
        ImGui::Indent();

        for (int i = 0; i < blendEditors[currentEditorIndex].affectedBones.size(); i++)
        {
            bool* checkbox = &blendEditors[currentEditorIndex].affectedBones[BONE_NAMES[i]];
            if (ImGui::Checkbox(BONE_NAMES[i].c_str(), checkbox))
            {

            }
        }

        ImGui::Unindent();

        ImGui::EndChild();
    }

    void OnFrame(float deltaTime) override
    {
        UpdateTouch();

        ImGuiIO& io = ImGui::GetIO();

        //Editor window options
        if (ImGui::Button("Create New Editor Window"))
        {
            blendEditors.push_back(BlendEditor());
            InitAffectedBones(&blendEditors[blendEditors.size() - 1]);
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove Editor Window") &&
            blendEditors.size() > 1)
        {
            blendEditors.pop_back();
        }

        //Tabs
        for (int i = 0; i < blendEditors.size(); i++)
        {
            std::string name = "Editor " + std::to_string(i);
            if (i > 0)
            {
                ImGui::SameLine();
            }

            if (ImGui::Button(name.c_str()))
            {
                ed::ClearSelection();
                m_Current = BlendEditorWindow::editor;
                currentEditorIndex = i;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            m_Current = BlendEditorWindow::save;
        }
        //ImGui::SameLine();
        /*if (ImGui::Button("Load"))
        {
            m_Current = BlendEditorWindow::load;
        }*/
        

        //Show Window
        if (m_Current == BlendEditorWindow::editor)
        {
            ShowEditorWindow(io);
        }
        else if (m_Current == BlendEditorWindow::save)
        {
            ImGui::Text("Save");
            ShowSaveWindow();
        }
        else if (m_Current == BlendEditorWindow::load)
        {
            ImGui::Text("Load");
            ShowLoadWindow();
        }
    }

    std::vector<BlendEditor> blendEditors;
    int currentEditorIndex = 0;
    

    int                  m_NextId = 1;
    const int            m_PinIconSize = 24;
    ImTextureID          m_HeaderBackground = nullptr;
    ImTextureID          m_SaveIcon = nullptr;
    ImTextureID          m_RestoreIcon = nullptr;
    const float          m_TouchTime = 1.0f;
    std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
    bool                 m_ShowOrdinals = false;
    
    BlendEditorWindow    m_Current = BlendEditorWindow::editor;
    SaveData saveData;
    LoadData loadData;
    
};

int Main(int argc, char** argv)
{
    Example example("Blueprints", argc, argv);
    example.blendEditors.push_back(BlendEditor());
    example.blendEditors.push_back(BlendEditor());

    example.InitAffectedBones(&example.blendEditors[0]);
    example.InitAffectedBones(&example.blendEditors[1]);

    if (example.Create())
        return example.Run();

    return 0;
}