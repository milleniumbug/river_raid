#pragma once
#include "stdafx.hpp"
#include "utilities.hpp"
#include "game.hpp"

class Display
{
	static const int frames_per_second = 120;
	int game_display_window_width;
	int game_display_window_height;
	int current_frame;
	double current_view;
	SDL_Surface* screen;
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> czcionka;
	typedef std::map<std::type_index, std::function<void(Display&, Entity&)>> draw_action_type;
	draw_action_type draw_action;
	// Graficzny układ współrzędnych, powszechnie przyjęty w grafice
	// wygląda tak:
	// ----------------------------------------------------->
	// |
	// |
	// |
	// |
	// v
	// (tj. im niżej tym większa wartość osi y)
	// Kłóci się to zupełnie z kartezjańskim układem współrzędnych,
	// który jest używany w modelu gry. Generalnie układ kartezjański
	// jest miliard razy lepszy, gdyż mogę traktować liczby zespolone jako współrzędne
	// i wektory w 2D. Tym samym mogę używać liczb zespolonych i 
	// całej potęgi działu matematyki jakim jest geometria analityczna
	// i wszystkich funkcji matematycznych, które operują na układzie współrzędnych.
	// Funkcja poniżej konwertuje na graficzne osie
	Zespolona mathematical_axis_to_graphical_axis(Zespolona z) const
	{
		return Zespolona(z.real(), game_display_window_height-z.imag());
	}
	Zespolona logical_position_to_graphical_position(Zespolona z) const
	{
		return z + Zespolona(0, current_view);
	}
	void put_sprite_on_screen(SDL_Surface* sprite, Zespolona pozycja, unsigned frame = 1, unsigned current_frame = 0)
	{
		Zespolona graficzne_wsp = mathematical_axis_to_graphical_axis(pozycja);
		Sint16 fw = sprite->w, fh = sprite->h / frame;
		auto x = static_cast<Sint16>(graficzne_wsp.real() - fw/2);
		auto y = static_cast<Sint16>(graficzne_wsp.imag() - fh/2);
		SDL_Rect dest = { x, y };
		SDL_Rect src = {
			0,
			static_cast<Sint16>(fh * current_frame),
			static_cast<Uint16>(fw),
			static_cast<Uint16>(fh) };
		SDL_BlitSurface(sprite, &src, screen, &dest);
	}
	void put_text_on_screen(const std::string& s, SDL_Rect pozycja);
	void put_text_on_screen(const std::string& s, SDL_Rect pozycja, SDL_Surface* font);

	void put_pixel_on_screen_unlocked_unchecked(int x, int y, uint8_t r, uint8_t g, uint8_t b)
	{
		int bpp = screen->format->BytesPerPixel;
		uint8_t *p = (uint8_t*)screen->pixels + y * screen->pitch + x * bpp;
		*(uint32_t*)p = SDL_MapRGB(screen->format, r, g, b);
	}
	void put_line_on_screen_unlocked_unchecked(int x, int y, int l, bool vertical, uint8_t r, uint8_t g, uint8_t b)
	{
		for(int i = 0; i < l; i++)
		{
			put_pixel_on_screen_unlocked_unchecked(x, y, r, g, b);
			if(!vertical)
				x++;
			else
				y++;
		}
	}
	void put_line_on_screen(int x, int y, int l, bool vertical, uint8_t r, uint8_t g, uint8_t b)
	{
		SDL_LockSurface(screen);
		put_line_on_screen_unlocked_unchecked(x, y, l, vertical, r, g, b);
		SDL_UnlockSurface(screen);
	}
	void put_rectangle_on_screen(SDL_Rect rect, uint8_t r, uint8_t g, uint8_t b)
	{
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, r, g, b));
	}

public:
	SDL_TimerID graphical_timer;
	Display(int w, int h);
	~Display();
	void render(const Game& game);
};