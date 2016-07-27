#pragma once
#include "stdafx.hpp"
#include "game.hpp"

class AbstractController
{
public:
	virtual void control(Game& game) = 0;
	virtual ~AbstractController() {}
};

class SDLController : public AbstractController
{
public:
	virtual void forward_state(SDL_Event* ev) = 0;
};

class SDLKeyboardController : public SDLController
{
	std::array<bool, SDLK_LAST> key_pressed;
	Zespolona current;
	bool strzel;
public:
	void forward_state(SDL_Event* ev) override
	{
		switch(ev->type)
		{
		case SDL_KEYUP:
			key_pressed.at(ev->key.keysym.sym) = false;
			break;
		case SDL_KEYDOWN:
			if(ev->key.keysym.sym == SDLK_SPACE)
				strzel = true;
			key_pressed.at(ev->key.keysym.sym) = true;
			break;
		}
	}

	void control(Game& game) override
	{
		Zespolona kierunek(0, 0);
		if(key_pressed[SDLK_UP]) kierunek += up;
		if(key_pressed[SDLK_DOWN]) kierunek += down;
		if(key_pressed[SDLK_LEFT]) kierunek += left;
		if(key_pressed[SDLK_RIGHT]) kierunek += right;
		if(strzel)
		{
			game.player_shoot();
			strzel = false;
		}
		game.player_change_direction(kierunek);
		if(key_pressed[SDLK_r]) game.reset_all();
	}

	SDLKeyboardController()
	{
		for(auto& x : key_pressed)
			x = false;
		strzel = false;
	}
};