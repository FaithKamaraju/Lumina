//
// Created by saiva on 08-10-2025.
//

#pragma once

#include "EventHandler.h"
#include "Core/EngineStatics.h"

namespace LE {

    class EventManager {

    public:

        void Shutdown();

        void Subscribe(EventType eventType, std::unique_ptr<IEventHandlerWrapper>&& handler);
        void Unsubscribe(EventType eventType, const std::string& handlerName);
        void TriggerEvent(const Event& event);
        void QueueEvent(std::unique_ptr<Event>&& event);
        void DispatchEvents();

    private:
        std::vector<std::unique_ptr<Event>> m_EventQueue;
        std::unordered_map<EventType, std::vector<std::unique_ptr<IEventHandlerWrapper>>> m_HandlerTable;



    };


    namespace Events {

        template<typename EventType>
        void Subscribe(const EventHandler<EventType>& callback) {
            std::unique_ptr<IEventHandlerWrapper> handler_wrapper = std::make_unique<EventHandlerWrapper<EventType>>(callback);
            Globals::GetEventManager()->Subscribe(EventType::GetStaticType(), std::move(handler_wrapper));
        }

        template<typename EventType>
        void Unsubscribe(const EventHandler<EventType>& callback)
        {
            const std::string handlerName = callback.target_type().name();
            Globals::GetEventManager()->Unsubscribe(EventType::GetStaticType(), handlerName);
        }

        inline void TriggerEvent(const Event& triggeredEvent)
        {
            Globals::GetEventManager()->TriggerEvent(triggeredEvent);
        }

        inline void QueueEvent(std::unique_ptr<Event>&& queuedEvent)
        {
            Globals::GetEventManager()->QueueEvent(std::forward<std::unique_ptr<Event>>(queuedEvent));
        }
    }




}
