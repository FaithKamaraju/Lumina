//
// Created by saiva on 20-10-2025.
//

#pragma once
#include "Core/Events/Event.h"

namespace LE {
    class Layer {

    public:
        explicit Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnImGuiRender() = 0;
        virtual void OnEvent(Event& e) = 0;

        [[nodiscard]] const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

} // LE