//
// Created by saiva on 19-10-2025.
//

#pragma once

#include "Event.h"

// WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,

// None
// EventCategoryApplication
// EventCategoryInput
// EventCategoryKeyboard
// EventCategoryMouse
// EventCategoryMouseButton

namespace LE {

    class WindowCloseEvent : public Event {
    public:
        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

        WindowCloseEvent() {};

    };

    class WindowResizeEvent : public Event {

    public:
        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

        WindowResizeEvent(int new_width, int new_height) : width(new_width), height(new_height) {}

        int width, height;
    };

    class WindowFramebufferResizeEvent : public Event {

    public:
        EVENT_CLASS_TYPE(WindowFramebufferResize)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

        WindowFramebufferResizeEvent(int new_width, int new_height) : width(new_width), height(new_height) {}

        int width, height;
    };

    class WindowFocusEvent : public Event {

    public:
        EVENT_CLASS_TYPE(WindowFocus)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

        WindowFocusEvent() {};
    };

    class WindowLostFocusEvent : public Event {

    public:
        EVENT_CLASS_TYPE(WindowLostFocus)
        EVENT_CLASS_CATEGORY(EventCategoryApplication)

        WindowLostFocusEvent() {};
    };


}

