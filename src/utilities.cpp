#include "stdafx.hpp"
#include <sstream>
#include "utilities.hpp"

void die(const char* message, int errcode)
{
	std::cerr << message << "\n";
	std::exit(errcode);
}

SDL_Surface* raw_get_sprite(const std::string& name)
{
	SDL_Surface* bitmap = SDL_LoadBMP(name.c_str());
	SDL_SetColorKey(bitmap, SDL_SRCCOLORKEY, SDL_MapRGB(bitmap->format, 0xff, 0x00, 0xff));
	return bitmap;
}

Sprite get_sprite(const std::string& name)
{
	return Sprite(raw_get_sprite(name), SDL_FreeSurface);
}

uint32_t timer_callback(uint32_t interval, void* data)
{
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = data;
	userevent.data2 = NULL;
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return interval;
}