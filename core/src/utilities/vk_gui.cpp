#include "engine/utilities/vk_gui.h"

namespace vke
{
    void Panel::render()
    {
        ImGui::SetNextWindowPos(m_position);
        ImGui::SetNextWindowSize(m_extent);
        ImGui::SetNextWindowCollapsed(m_collapsed, ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m_rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_padding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, m_minExtent);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_borderSize);
        if (m_color.x > -1)
            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, m_color);

        if (ImGui::Begin(m_title, NULL, (ImGuiWindowFlags)m_flags | ImGuiWindowFlags_NoMove))
        {

            for (auto widget : m_children)
                widget->render();
        }

        ImGui::End();
        if (m_color.x > -1)
            ImGui::PopStyleColor();
        ImGui::PopStyleVar(4);
    }
    void TextLine::render()
    {
        switch (m_type)
        {
        case SIMPLE:
            ImGui::Text(m_text);
            break;
        case WARPED:
            ImGui::TextWrapped(m_text);
            break;
        case COLORIZED:
            ImGui::TextColored(m_color, m_text);
            break;
        case BULLET:
            ImGui::BulletText(m_text);
            break;
        }
    }
    void SceneExplorer::render()
    {
        ImGui::Spacing();
        ImGui::SeparatorText("SCENE EXPLORER");

        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody /*| ImGuiTableFlags_BordersH*/;

        if (ImGui::BeginTable("2ways", 2, flags, ImVec2(m_parent->get_extent().x, m_parent->get_extent().y * 0.3f)))
        {
            ImGui::TableSetupColumn(" Name", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 3 * 18.0f);
            ImGui::TableHeadersRow();

            // Simple storage to output a dummy file-system.
            struct MyTreeNode
            {
                Object3D *obj;
                const char *Name;
                const char *Type;
                int ChildIdx;
                int ChildCount;
                bool selected;
                static void DisplayNode(MyTreeNode *node, std::vector<MyTreeNode> all_nodes)
                {

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    const bool is_folder = (node->ChildCount > 0);
                    if (is_folder)
                    {
                        bool open = ImGui::TreeNodeEx(node->Name, ImGuiTreeNodeFlags_SpanFullWidth);
                        ImGui::TableNextColumn();
                        ImGui::TextDisabled("--");
                        if (open)
                        {
                            for (int child_n = 0; child_n < node->ChildCount; child_n++)
                                DisplayNode(&all_nodes[node->ChildIdx + child_n], all_nodes);
                            ImGui::TreePop();
                        }
                    }
                    else
                    {

                        if (ImGui::TreeNodeEx(node->obj->get_name().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
                        {
                            /*	node->selected = ImGui::IsItemClicked();
                                if (ImGui::IsItemClicked()) {
                                    UIManager::setSelectedObj(node->obj, true);
                                }*/
                        };

                        ImGui::TableNextColumn();

                        if (ImGui::SmallButton(node->obj->is_active() ? "true" : "false"))
                        {
                            node->obj->set_active(!node->obj->is_active());
                        }
                    }
                }
            };

            auto objs = m_scene->get_children();
            std::vector<MyTreeNode> nodes;
            int counter = 0;
            for (auto obj : objs)
            {
                auto tpe = obj->get_type();

                ImGui::PushID(counter);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string name = obj->get_name();

                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth))
                {
                    // node->selected = ImGui::IsItemClicked();
                    // if (ImGui::IsItemClicked())
                    // {
                    //     if (UIManager::m_SelectedObject)
                    //         UIManager::m_SelectedObject->selected = false;
                    //     UIManager::m_SelectedObject = obj.second;
                    //     obj->selected = true;
                    // }
                };

                ImGui::TableNextColumn();
                if (ImGui::Button(obj->is_active() ? "true" : "false"))
                {
                    obj->set_active(!obj->is_active());
                }
                ImGui::PopID();

                counter++;
            }

            ImGui::EndTable();
        }
        ImGui::Spacing();
        ImGui::Separator();
    }
    void Profiler::render()
    {
        ImGui::Text(" %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    void Space::render()
    {
        ImGui::Spacing();
    }
    void Separator::render()
    {
        !m_text ? ImGui::Separator() : ImGui::SeparatorText(m_text);
    }

}