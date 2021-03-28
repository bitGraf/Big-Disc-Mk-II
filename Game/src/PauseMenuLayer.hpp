#pragma once
#include <Engine.hpp>

class Button {
public:
    Button(float xmin, float ymin, float width, float height)
        : m_xmin(xmin), m_ymin(ymin), m_width(width), m_height(height), m_Name("__button__")
    {}

    Button(float xmin, float ymin, float width, float height, const char* name)
        : m_xmin(xmin), m_ymin(ymin), m_width(width), m_height(height), m_Name(name)
    {}

    std::string m_Name;
    float m_xmin, m_ymin, m_width, m_height;
};

class PauseLayer : public Engine::EngineLayer {
public:
    PauseLayer();
    virtual ~PauseLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Engine::Timestep ts) override;
    virtual void OnEvent(Engine::Event& event) override;
    virtual void OnGuiRender() override;

    bool inBox(float x, float y, const Button& button);

private:
    bool OnKeyPressedEvent(Engine::KeyPressedEvent& e);
    bool OnMouseButtonReleasedEvent(Engine::MouseButtonReleasedEvent& e);
    bool OnMouseButtonPressedEvent(Engine::MouseButtonPressedEvent& e);

private:
    std::vector<Button> m_Buttons;
};