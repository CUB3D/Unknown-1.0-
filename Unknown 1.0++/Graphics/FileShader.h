//
// Created by cub3d on 26/08/18.
//

#ifndef UNKNOWN_DEVELOPMENT_TOOL_FILESHADER_H
#define UNKNOWN_DEVELOPMENT_TOOL_FILESHADER_H

#include "Shader.h"

class FileShader : public Shader
{
public:
    FileShader(const std::string& vert, const std::string& frag);
};


#endif //UNKNOWN_DEVELOPMENT_TOOL_FILESHADER_H
