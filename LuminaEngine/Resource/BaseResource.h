//
// Created by Faith Kamaraju on 2026-03-01.
//

#pragma once

namespace LE {
    struct BaseResource {

        void MarkHasChanged() {hasChanged = true;}

        bool hasChanged = false;
    };
}