#include "stdafx.hpp"
#include "utilities.hpp"
#include "display.hpp"

void Display::render(const Game& game)
{
	SDL_Rect upper_rect = {
		0,
		0,
		static_cast<Uint16>(game_display_window_width),
		static_cast<Uint16>(game_display_window_height-80) };
	SDL_Rect lower_rect = {
		0,
		static_cast<Sint16>(game_display_window_height-80),
		static_cast<Uint16>(game_display_window_width),
		80 };
	SDL_FillRect(screen, &upper_rect, SDL_MapRGB(screen->format, 0,0,170));
	current_view = 130 - game.pozycja_gracza().imag();
	for(auto& entity : game.active_entities())
		draw_action.at(typeid(*entity))(*this, *entity);
	SDL_FillRect(screen, &lower_rect, SDL_MapRGB(screen->format, 170,170,170));
	SDL_Rect zycie_tekst_poz = {
		30,
		static_cast<Sint16>(game_display_window_height-60) };
	std::stringstream ilosc_zyc_tekst;
	ilosc_zyc_tekst << "Pozosta\xB3o \xBFyc: " << game.lives();
	put_text_on_screen(ilosc_zyc_tekst.str().c_str(), zycie_tekst_poz);
	SDL_Rect paliwo_tekst_poz = {
		200,
		static_cast<Sint16>(game_display_window_height-60) };
		std::stringstream ilosc_paliwa_tekst;
	ilosc_paliwa_tekst << "Paliwo: " << game.paliwo_gracza_w_procentach() << "%";
	put_text_on_screen(ilosc_paliwa_tekst.str().c_str(), paliwo_tekst_poz);
	SDL_Rect wynik_tekst_poz = {
		30,
		static_cast<Sint16>(game_display_window_height-40) };
	std::stringstream wynik_tekst;
	wynik_tekst << "Ilo\xB6\xE6 punkt\xF3w: " << game.score();
	put_text_on_screen(wynik_tekst.str().c_str(), wynik_tekst_poz);

	SDL_Rect help_text_poz = {
		400,
		static_cast<Sint16>(game_display_window_height-70) };
	put_text_on_screen("River Raid Remake", help_text_poz);
	help_text_poz.y += 10;
	put_text_on_screen("Strzalki - steruj samolotem", help_text_poz);
	help_text_poz.y += 10;
	put_text_on_screen("Spacja - strzel", help_text_poz);
	help_text_poz.y += 10;
	SDL_Rect game_over_tekst_poz = {
		30,
		static_cast<Sint16>(game_display_window_height-20) };
	if(game.game_state() == Game::game_over)
		put_text_on_screen("GAME OVER, naci\xB6nij R by zagra\xE6 od nowa lub naci\xB6nij Esc by wyj\xB6\xE6.", game_over_tekst_poz);
	SDL_Flip(screen);
	current_frame = (current_frame+1)%frames_per_second;
}

Display::Display(int w, int h) :
	game_display_window_width(w),
	game_display_window_height(h),
	current_view(40),
	czcionka(raw_get_sprite("czcionka.bmp"), SDL_FreeSurface),
	current_frame(0)
{
	screen = SDL_SetVideoMode(game_display_window_width, game_display_window_height, 32, 0);
	SDL_WM_SetCaption("River Raid", nullptr);
	graphical_timer = SDL_AddTimer(1000/frames_per_second, timer_callback, &graphical_timer);
	typedef draw_action_type::value_type action;
	draw_action.insert(action(typeid(Samolot), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("samolot.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	}));
	draw_action.insert(action(typeid(NormalLand), [](Display& disp, Entity& e)
	{
		auto& land = dynamic_cast<NormalLand&>(e);
		auto trapez = land.granice();
		std::transform(trapez.begin(), trapez.end(), trapez.begin(),
			[&disp](Zespolona z){ return disp.logical_position_to_graphical_position(z); });
		SDL_Rect r = { std::min(trapez[0].real(), trapez[3].real()),
		               std::min(disp.game_display_window_height - trapez[0].imag(), disp.game_display_window_height - trapez[3].imag()),
					   std::abs(trapez[0].real() - trapez[3].real()),
		               std::abs(trapez[0].imag() - trapez[3].imag())
		};
		disp.put_rectangle_on_screen(r, 34, 177, 76);
	}));
	draw_action.insert(action(typeid(House), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("dom.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	}));
	draw_action.insert(action(typeid(Tree), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("drzewo.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	}));
	auto draw_boat = [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("boat.bmp");
		auto kierunek = dynamic_cast<EntityWithDirection&>(e).direction();
		disp.put_sprite_on_screen(s.get(), poz, 2, (kierunek.real() < 0 ? 1 : 0));
	};
	draw_action.insert(action(typeid(StaticBoat), draw_boat));
	draw_action.insert(action(typeid(DynamicBoat), draw_boat));
	draw_action.insert(action(typeid(Fighter), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("mysliwiec.bmp");
		auto kierunek = dynamic_cast<EntityWithDirection&>(e).direction();
		disp.put_sprite_on_screen(s.get(), poz, 2, (kierunek.real() > 0 ? 1 : 0));
	}));
	draw_action.insert(action(typeid(Heli), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("helikopter.bmp");
		auto kierunek = dynamic_cast<EntityWithDirection&>(e).direction();
		disp.put_sprite_on_screen(s.get(), poz, 8, (kierunek.real() < 0 ? 4 : 0) + disp.current_frame * 3 / 15 % 4);
	}));
	auto draw_bridge = [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("bridge.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	};
	draw_action.insert(action(typeid(Bridge), draw_bridge));
	draw_action.insert(action(typeid(ImportantBridge), draw_bridge));
	draw_action.insert(action(typeid(Bullet), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto bv = get_sprite("bulletv.bmp");
		static auto bh = get_sprite("bulleth.bmp");
		auto& projectile = dynamic_cast<Projectile&>(e);
		auto k = projectile.direction();
		disp.put_sprite_on_screen(k.imag() == 0 ? bh.get() : bv.get(), poz, 2, k.real() > 0 ? 1 : 0);
	}));
	draw_action.insert(action(typeid(TankShell), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("czolg_shell.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	}));
	draw_action.insert(action(typeid(FuelStation), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("fuel_station.bmp");
		disp.put_sprite_on_screen(s.get(), poz);
	}));
	draw_action.insert(action(typeid(SmallExplosion), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("explosion_small.bmp");
		auto& explosion = dynamic_cast<Explosion&>(e);
		int frame = 4 - explosion.time_to_live() / static_cast<double>(explosion.initial_time_to_live()) * 4;
		disp.put_sprite_on_screen(s.get(), poz, 4, frame);
	}));
	draw_action.insert(action(typeid(MediumExplosion), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("explosion_medium.bmp");
		auto& explosion = dynamic_cast<Explosion&>(e);
		int frame = 4 - explosion.time_to_live() / static_cast<double>(explosion.initial_time_to_live()) * 4;
		disp.put_sprite_on_screen(s.get(), poz, 4, frame);
	}));
	draw_action.insert(action(typeid(LargeExplosion), [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("explosion_large.bmp");
		auto& explosion = dynamic_cast<Explosion&>(e);
		int frame = 4 - explosion.time_to_live() / static_cast<double>(explosion.initial_time_to_live()) * 4;
		disp.put_sprite_on_screen(s.get(), poz, 4, frame);
	}));
	auto draw_tank = [](Display& disp, Entity& e)
	{
		auto poz = disp.logical_position_to_graphical_position(e.pozycja());
		static auto s = get_sprite("czolg_body.bmp");
		static auto t = get_sprite("czolg_turret.bmp");
		auto& tank = dynamic_cast<Tank&>(e);
		disp.put_sprite_on_screen(s.get(), poz);
		double kat = std::arg(tank.turret_direction());
		if(kat < 0) kat += 2*pi;
		disp.put_sprite_on_screen(t.get(), poz, 32, kat / (2*pi) * 32);
	};
	draw_action.insert(action(typeid(StaticTank), draw_tank));
	draw_action.insert(action(typeid(DynamicTank), draw_tank));
}

Display::~Display()
{
	SDL_RemoveTimer(graphical_timer);
}

void Display::put_text_on_screen(const std::string& str, SDL_Rect pozycja)
{
	put_text_on_screen(str, pozycja, czcionka.get());
}

void Display::put_text_on_screen(const std::string& str, SDL_Rect pozycja, SDL_Surface* font)
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	for(auto it = str.begin(); it != str.end(); ++it)
	{
		c = *it & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = pozycja.x;
		d.y = pozycja.y;
		SDL_BlitSurface(czcionka.get(), &s, screen, &d);
		pozycja.x += 8;
	}
}