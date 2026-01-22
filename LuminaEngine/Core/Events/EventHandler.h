//
// Created by saiva on 08-10-2025.
//

#pragma once
#include "Event.h"


namespace LE {



    template<typename EventType>
    using EventHandler = std::function<void(const EventType& e)>;

    class IEventHandlerWrapper {

    public:

        virtual ~IEventHandlerWrapper() = default;

        void Exec(const Event& e) {
            Call(e);
        }

        virtual std::string GetType() const = 0;
        virtual bool IsDestroyOnSuccess() const = 0;

    private:

        virtual void Call(const Event& e) = 0;
    };





    template<typename EventType>
    class EventHandlerWrapper : public IEventHandlerWrapper {

    public:
        explicit EventHandlerWrapper(const EventHandler<EventType>& handler)
            : m_Handler(handler), m_HandlerType(m_Handler.target_type().name()) {};

    private:

        void Call(const Event& e) override {
            if (e.GetEventType() == EventType::GetStaticType()) {
                m_Handler(static_cast<const EventType&>(e));
            }
        }
        [[nodiscard]] std::string GetType() const override { return m_HandlerType; }
        [[nodiscard]] bool IsDestroyOnSuccess() const override { return m_DestroyOnSuccess; }

        EventHandler<EventType> m_Handler;
        const std::string m_HandlerType;
        bool m_DestroyOnSuccess {false};
    };
}