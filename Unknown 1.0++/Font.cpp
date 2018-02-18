#include "stdafx.h"
#include "Font.h"

#include <iostream>
#include <SDL_ttf.h>

Unknown::Graphics::Font::Font(Image* fontSheet, const std::string layout, const int charSize) : fontSheet(fontSheet), layout(layout), charSize(charSize) {}

void Unknown::Graphics::Font::drawChar(const char character, const int x, const int y)
{
	fontSheet->textureRect.w = this->charSize;
	fontSheet->textureRect.h = this->charSize;

	int position = layout.find(character);

	SDL_Rect clip;
	clip.x = position * this->charSize;
	clip.y = 0;
	clip.w = this->charSize;
	clip.h = this->charSize;

	this->fontSheet->render(x, y, 0, &clip);
}

void Unknown::Graphics::Font::drawString(const std::string string, const int x, const int y)
{
	for (int i = 0; i < string.size(); i++)
	{
		this->drawChar(string.c_str()[i], x + (i * this->charSize), y);
	}
}

int Unknown::Graphics::Font::getStringWidth(const std::string str) const
{
	return str.size() * this->charSize;
}

int Unknown::Graphics::Font::getStringHeight(const std::string str) const
{
    return this->charSize;
}

//TTFont

Unknown::Graphics::TTFont::TTFont(std::string name, const int size, Colour colour) : Font(NULL, "", 0)
{
    this->font = TTF_OpenFont(name.c_str(), size);
    this->color = colour.toSDLColour();
}

void Unknown::Graphics::TTFont::drawString(const std::string string, const int x, const int y)
{
	SDL_Surface* textSurface = TTF_RenderText_Blended(font, string.c_str(), this->color);
	auto texture = SDL_CreateTextureFromSurface(getUnknown()->windowRenderer, textSurface);
	SDL_FreeSurface(textSurface);
	SDL_Rect quad = {x, y, textSurface->w, textSurface->h};
	SDL_RenderCopy( getUnknown()->windowRenderer, texture, NULL, &quad );
    SDL_DestroyTexture(texture);
}

int Unknown::Graphics::TTFont::getStringWidth(const std::string str) const
{
    int width = 0;
    TTF_SizeText(this->font, str.c_str(), &width, NULL);
    return width;
}

int Unknown::Graphics::TTFont::getStringHeight(const std::string str) const
{
    int height = 0;
    TTF_SizeText(this->font, str.c_str(), NULL, &height);
    return height;
}

