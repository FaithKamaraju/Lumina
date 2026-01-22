//
// Created by saiva on 08-10-2025.
//

#include "EventManager.h"
#include "Core/Logger.h"


void LE::EventManager::Shutdown()
{
    m_EventQueue.clear();
    m_HandlerTable.clear();
}

void LE::EventManager::Subscribe(EventType eventType, std::unique_ptr<IEventHandlerWrapper> &&handler) {

    auto subscribers = m_HandlerTable.find(eventType);
    if (subscribers != m_HandlerTable.end()) {
        auto& handlers = subscribers->second;
        for (const auto& it : handlers) {
            if (it->GetType() == handler->GetType()) {
                LE_CORE_ERROR("ATTEMPTING TO DOUBLE REGISTER CALLBACK!");
                return;
            }
        }
        handlers.emplace_back(std::move(handler));
    }
    else {
        m_HandlerTable[eventType].emplace_back(std::move(handler));
    }

}

void LE::EventManager::Unsubscribe(EventType eventType, const std::string &handlerName) {

    auto& handlers = m_HandlerTable[eventType];
    for (auto it = handlers.begin(); it != handlers.end(); ++it) {
        if (it->get()->GetType() == handlerName) {
            it = handlers.erase(it);
            return;
        }
    }
}

void LE::EventManager::TriggerEvent(const Event &event) {

    for (const auto& handler : m_HandlerTable[event.GetEventType()]) {
        handler->Exec(event);
    }

}

void LE::EventManager::QueueEvent(std::unique_ptr<Event> &&event) {
    m_EventQueue.emplace_back(std::move(event));
}

void LE::EventManager::DispatchEvents() {
    for (auto it = m_EventQueue.begin(); it != m_EventQueue.end();) {
        if (!it->get()->bHandled) {
            TriggerEvent(*it->get());
            it = m_EventQueue.erase(it);
        }
        else {
            ++it;
        }

    }
}
