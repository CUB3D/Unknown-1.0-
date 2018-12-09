//
// Created by cub3d on 03/11/18.
//

#ifndef UNKNOWN_DEVELOPMENT_TOOL_ENGINECONFIG_H
#define UNKNOWN_DEVELOPMENT_TOOL_ENGINECONFIG_H

#include <string>
#include "rttr/registration"

#include <Types/Dimension.h>

namespace Unknown {
    struct EngineConfig {

        EngineConfig();

        Dimension<int> targetSize;

        std::string title;
        int targetUPS;

        // Render Settings
        int rendererMode;
        bool MSAA;
        int vsync;
        bool textureFallback;
    };
}


#endif //UNKNOWN_DEVELOPMENT_TOOL_ENGINECONFIG_H
