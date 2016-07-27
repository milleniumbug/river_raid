#include "stdafx.hpp"

#include "utilities.hpp"
#include "game.hpp"
#include "controller.hpp"
#include "display.hpp"

int real_main()
{
	int initerr = SDL_Init(SDL_INIT_EVERYTHING);
	if(initerr) die("Blad: inicjalizacja SDL nieudana");
	
	SDL_TimerID logical_timer = SDL_AddTimer(1000/120, timer_callback, &logical_timer);
	SDL_Event event;

	Game gra;
	Display ekran(640, 600);
	std::unique_ptr<SDLController> kontroler(new SDLKeyboardController());
	bool should_i_quit = false;
	while(SDL_WaitEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYUP: /* uwaga fall-through */
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE)
				should_i_quit = true;
			kontroler->forward_state(&event);
		break;
		case SDL_USEREVENT:
			if(event.user.data1 == &logical_timer)
			{
				kontroler->control(gra);
				gra.refresh();
			}
			if(event.user.data1 == &ekran.graphical_timer)
			{
				ekran.render(gra);
			}
		break;
		case SDL_QUIT:
			should_i_quit = true;
		}

		if(should_i_quit)
			break;
	}
	SDL_RemoveTimer(logical_timer);
	SDL_Quit();
	return 0;
}


#ifndef _WIN32
int main(int argc, char** argv)
{
	return real_main();
}
#else
#include <windows.h>
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
	return real_main();
}
#endif
