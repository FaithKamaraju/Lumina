//
// Created by saiva on 15-10-2025.
//

#pragma once

#include "Event.h"

namespace LE {

    class KeyPressedEvent : public Event {
    public:
        EVENT_CLASS_TYPE(KeyPressed);
        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard);



    };

    class KeyReleasedEvent : public Event {
    public:
        EVENT_CLASS_TYPE(KeyReleased);
        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard);


    };

    class KeyTypedEvent : public Event {
    public:
        EVENT_CLASS_TYPE(KeyTyped);
        EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard);


    };
}