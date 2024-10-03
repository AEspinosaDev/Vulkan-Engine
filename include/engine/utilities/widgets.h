/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef WIDGETS_H
#define WIDGETS_H

#include <functional>
#include <engine/common.h>
#include <engine/core/scene/scene.h>
#include <engine/core/materials/physically_based.h>
#include <engine/core/materials/hair.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class GUIOverlay;

class Panel;

class Widget
{
protected:
    ImVec2 m_position;
    ImVec2 m_extent;
    ImVec2 m_pixelExtent;
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
    virtual inline Vec2 get_position() const { return {m_position.x, m_position.y}; }
    virtual inline void set_position(Vec2 p) { m_position = {p.x, p.y}; }

    virtual inline Vec2 get_extent() const { return {m_extent.x, m_extent.y}; }
    virtual inline void set_extent(Vec2 p) { m_extent = {p.x, p.y}; }

    inline Vec2 get_pixel_extent() const { return {m_pixelExtent.x, m_pixelExtent.y}; }
};

class Panel : public Widget
{
protected:
    float m_rounding{12.0f};
    ImVec2 m_padding{8.0f, 5.0f};
    ImVec2 m_minExtent;

    ImVec4 m_color{-1.0f, -1.0f, -1.0f, 1.0f};
    float m_borderSize{0.0f};

    bool m_collapsable{true};
    bool m_collapsed{false};
    bool m_menuBar{false};

    bool m_open{true};
    bool m_closable{false};

    bool m_resized{false};

    GUIOverlay *m_parentOverlay{nullptr};

    PanelWidgetFlags m_flags;

    const char *m_title;

    friend class GUIOverlay;

    virtual void render() {}
    virtual void render(ImVec2 extent);

public:
    Panel(const char *title, float posX, float posY, float extentX, float extentY, PanelWidgetFlags flags = PanelWidgetFlags::None, bool collapsed = false, bool closable = false, bool menu = false)
        : Widget(ImVec2(posX, posY), ImVec2(extentX, extentY)), m_title(title),
          m_minExtent(ImVec2(extentX * 0.5f, extentY * 0.5f)), m_flags(flags), m_collapsed(collapsed), m_closable(closable),m_menuBar(menu) {}

    ~Panel()
    {
        for (auto widget : m_children)
            delete widget;
    }

    inline Vec2 get_padding() const { return {m_padding.x, m_padding.y}; }
    inline void set_padding(Vec2 p) { m_padding = {p.x, p.y}; }

    inline Vec2 get_min_extent() const { return {m_minExtent.x, m_minExtent.y}; }
    inline void set_min_extent(Vec2 p) { m_minExtent = {p.x, p.y}; }

    inline float get_rounding() const { return m_rounding; }
    inline void set_rounding(float t) { m_rounding = t; }

    inline float get_border_size() const { return m_borderSize; }
    inline void set_border_size(float t) { m_borderSize = t; }

    inline bool get_is_open() const { return m_open; }
    inline void set_is_open(bool t) { m_open = t; }

    inline void set_is_resized(bool op) { m_resized = op; }
    inline bool get_is_resized() { return m_resized; }
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
protected:
    std::function<void()> callback;
    virtual void render();

public:
    ColorPicker() : Widget(ImVec2(0, 0), ImVec2(0, 0)) {}
    void set_callback(std::function<void()> &&function);
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
    TextLine(char *text, math::vec4 color) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text), m_color({color.r, color.g, color.b, color.a}) {}
    TextLine(char *text, TextWidgetType type) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_text(text), m_type(type) {}

    inline math::vec4 get_color() const { return {m_color.x, m_color.y, m_color.z, m_color.w}; }
    inline void get_color(math::vec4 color) { m_color = {color.r, color.g, color.b, color.a}; }

    inline TextWidgetType get_type() const { return m_type; }
    inline void set_type(TextWidgetType t) { m_type = t; }

    inline char *get_text() const { return m_text; }
    inline void set_text(char *t) { m_text = t; }
};

class SceneExplorerWidget : public Widget
{

protected:
    Scene *m_scene;
    Object3D *m_selectedObject{nullptr};
    virtual void render();

public:
    SceneExplorerWidget(Scene *scene) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_scene(scene) {}

    inline Scene *get_scene() const { return m_scene; }
    inline void set_scene(Scene *sc) { m_scene = sc; }

    inline Object3D *get_selected_object() const { return m_selectedObject; }
};

class ObjectExplorerWidget : public Widget
{
    Object3D *m_object;

protected:
    virtual void render();

public:
    ObjectExplorerWidget(Object3D *obj = nullptr) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_object(obj) {}

    inline Object3D *get_object() const { return m_object; }
    inline void set_object(Object3D *obj) { m_object = obj; }
};

class GlobalSettingsWidget : public Widget
{
    Renderer *m_renderer;

protected:
    virtual void render();

public:
    GlobalSettingsWidget(Renderer *obj = nullptr) : Widget(ImVec2(0, 0), ImVec2(0, 0)), m_renderer(obj) {}

    inline Renderer *get_renderer() const { return m_renderer; }
    inline void set_renderer(Renderer *obj) { m_renderer = obj; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif