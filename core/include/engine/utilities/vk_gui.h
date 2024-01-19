#ifndef VK_GUI
#define VK_GUI

#include "../private/vk_core.h"
#include "../scene_objects/vk_scene.h"

// WIP..
// MUCH TO DO HERE, JUST READY FOR A SIMPLE DEMO

namespace vke
{
    class GUIOverlay;

    class Panel;

    class Widget
    {
    protected:
        ImVec2 m_position;
        ImVec2 m_extent;

        std::vector<Widget *> m_children;
        Widget *m_parent;

        virtual void render(){};

        friend class Panel;

    public:
        Widget(ImVec2 pos, ImVec2 extent) : m_position(pos), m_extent(extent) {}

        virtual inline void add_child(Widget *w)
        {
            w->m_parent = this;
            m_children.push_back(w);
        }
        virtual inline glm::vec2 get_position() const { return {m_position.x, m_position.y}; }
        virtual inline void set_position(glm::vec2 p) { m_position = {p.x, p.y}; }

        virtual inline glm::vec2 get_extent() const { return {m_extent.x, m_extent.y}; }
        virtual inline void set_extent(glm::vec2 p) { m_extent = {p.x, p.y}; }
    };

    class Panel : public Widget
    {
    protected:
        float m_rounding{0.0f};
        ImVec2 m_padding{0.0f, 0.0f};
        ImVec2 m_minExtent;
        ImVec4 m_color{-1.0f, -1.0f, -1.0f, 1.0f};
        float m_borderSize{0.0f};

        bool m_collapsed{false};

        PanelWidgetFlags m_flags;

        const char *m_title;

        friend class GUIOverlay;

        virtual void render();

    public:
        Panel(const char *title, float posX, float posY, float extentX, float extentY, PanelWidgetFlags flags = PanelWidgetFlags::None, bool collapsed = false)
            : Widget(ImVec2(posX, posY), ImVec2(extentX, extentY)), m_title(title),
              m_minExtent(ImVec2(extentX * 0.5f, extentY * 0.5f)), m_flags(flags), m_collapsed(collapsed) {}

        ~Panel()
        {
            for (auto widget : m_children)
                delete widget;
        }

        inline glm::vec2 get_padding() const { return {m_padding.x, m_padding.y}; }
        inline void set_padding(glm::vec2 p) { m_padding = {p.x, p.y}; }

        inline glm::vec2 get_min_extent() const { return {m_minExtent.x, m_minExtent.y}; }
        inline void set_min_extent(glm::vec2 p) { m_minExtent = {p.x, p.y}; }

        inline float get_rounding() const { return m_rounding; }
        inline void set_rounding(float t) { m_rounding = t; }

        inline float get_border_size() const { return m_borderSize; }
        inline void set_border_size(float t) { m_borderSize = t; }
    };

    class Button : public Widget
    {
        // TO DO
    };

    class Slider : public Widget
    {
        // TO DO
    };

    class ColorPicker : public Widget
    {
        // TO DO
    };

    class Separator : public Widget
    {
    protected:
        char *m_text{nullptr};
        virtual void render();

    public:
        Separator() : Widget(ImVec2(0, 0), ImVec2(0, 0)) {}
        Separator(char *text) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text) {}
    };

    class Space : public Widget
    {
    protected:
        virtual void render();

    public:
        Space() : Widget(ImVec2(0, 0), ImVec2(0, 0)) {}
    };

    class Profiler : public Widget
    {
    protected:
        virtual void render();

    public:
        Profiler() : Widget(ImVec2(0, 0), ImVec2(0, 0)) {}
    };

    class TextLine : public Widget
    {
    protected:
        char *m_text;
        ImVec4 m_color{1.0f, 1.0f, 1.0f, 1.0f};

        TextWidgetType m_type{WARPED};

        virtual void render();

    public:
        TextLine(char *text) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text) {}
        TextLine(char *text, glm::vec4 color) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text), m_color({color.r, color.g, color.b, color.a}) {}
        TextLine(char *text, TextWidgetType type) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text), m_type(type) {}

        inline glm::vec4 get_color() const { return {m_color.x, m_color.y, m_color.z, m_color.w}; }
        inline void get_color(glm::vec4 color) { m_color = {color.r, color.g, color.b, color.a}; }

        inline TextWidgetType get_type() const { return m_type; }
        inline void set_type(TextWidgetType t) { m_type = t; }

        inline char *get_text() const { return m_text; }
        inline void set_text(char *t) { m_text = t; }
    };

    class SceneExplorer : public Widget
    {

    protected:
        Scene *m_scene;
        virtual void render();

    public:
        SceneExplorer(Scene *scene) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_scene(scene) {}

        inline Scene *get_scene() const { return m_scene; }
        inline void set_scene(Scene *sc) { m_scene = sc; }
    };

    class GUIOverlay
    {

    private:
        // VkDescriptorPoo

        std::vector<Panel *> m_panels;

        inline void render()
        {
            for (auto p : m_panels)
            {
                p->render();
            }
        }

        static void init();
        static void render(GUIOverlay &gui);
        static void upload_draw_data(GUIOverlay &gui);
        static void cleanup(GUIOverlay &gui);

        friend class Renderer;

    public:
        GUIOverlay() {}
        ~GUIOverlay()
        {
            for (auto p : m_panels)
            {
                delete p;
            }
        }
        inline void add_panel(Panel *p)
        {
            m_panels.push_back(p);
        }
    };

}

#endif