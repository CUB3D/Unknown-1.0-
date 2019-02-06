//
// Created by cub3d on 11/10/18.
//

#ifndef UNKNOWN_DEVELOPMENT_TOOL_FSUTILS_H
#define UNKNOWN_DEVELOPMENT_TOOL_FSUTILS_H

#include <SDL.h>
#include <istream>

Sint64 istreamSeek(SDL_RWops *ops, Sint64 offset, int loc);
size_t istreamRead(SDL_RWops* ops, void* dat, size_t size, size_t max);
int istreamClose(SDL_RWops* ops);

SDL_RWops* getRWopsForStream(std::istream& stream);

#endif //UNKNOWN_DEVELOPMENT_TOOL_FSUTILS_H