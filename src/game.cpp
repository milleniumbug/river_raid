#include "stdafx.hpp"
#include "game.hpp"

Game::Game() :
	current_level(0),
	level_reset_requested(false),
	full_reset_requested(false)
{
	reset_all_commit();
}

void Game::reset_level_commit()
{
	level_reset_requested = false;
	entities.clear();
	entities_to_activate.clear();
	entities_to_add.clear();
	actions.clear();
	auto ilosc_zyc = gracz ? gracz->life_count() : 3;
	auto pl = std::make_shared<Samolot>(Zespolona(320, 0), ilosc_zyc);
	gracz = pl;
	entities.push_back(pl);
	load_from_file();
}

void Game::reset_level()
{
	level_reset_requested = true;
}

void Game::reset_all_commit()
{
	points = 0;
	current_level = 0;
	gst = on;
	full_reset_requested = false;
	gracz.reset();
	reset_level_commit();
}

void Game::reset_all()
{
	full_reset_requested = true;
}

void Game::load_from_file()
{
	std::stringstream ss;
	ss << current_level << ".lev";
	std::ifstream plik(ss.str());
	std::string name_and_args;
	while(std::getline(plik, name_and_args))
	{
		auto pos = name_and_args.find_first_of(' ');
		if(!name_and_args.empty() && name_and_args.substr(0, pos) != "#")
			push_to_container(make_entity(name_and_args.substr(0, pos), name_and_args.substr(pos)));
	}
}

bool Game::is_it_reachable(const AxisAlignedBoundingBox& box) const
{
	return detect_collision(AxisAlignedBoundingBox(Zespolona(0, std::numeric_limits<double>::infinity()),
	                                               Zespolona(level_width(), pozycja_gracza().imag() - 400)),
	                        box);
}

void Game::push_to_container(std::shared_ptr<Entity> e)
{
	if(!is_it_reachable(e->hitbox()))
		return;
	else if(detect_collision(current_game_area_hitbox(), e->hitbox()))
		entities.push_back(e);
	else
		entities_to_activate.insert(e);
}

void Game::schedule_action_after_n_ticks(int delay, std::function<void()> f)
{
	actions.push_back(std::make_pair(delay, f));
}

void Game::create_entity(std::shared_ptr<DynamicEntity> e)
{
	//poniewa? nie chcemy uniewa?nia?terator?	//dodajemy do osobnej listy, a potem dopiero
	//przenosimy do listy entek
	entities_to_add.push_back(e);
}

void Game::refresh()
{
	if(level_reset_requested)
		reset_level_commit();
	if(full_reset_requested)
		reset_all_commit();
	for(auto it = actions.begin(); it != actions.end(); )
	{
		if(--it->first == 0)
		{
			it->second();
			actions.erase(it++);
		}
		else
			++it;
	}
	for(auto it1 = entities.begin(); it1 != std::prev(entities.end()); ++it1)
	{
		for(auto it2 = it1; it2 != entities.end(); ++it2)
		{
			if(detect_collision((*it1)->hitbox(), (*it2)->hitbox()))
				(*it1)->collide_with(**it2);
		}
	}
	for(auto& entity : entities)
		if(entity)
		{
			bool should_destroy = entity->refresh(*this);
			if(should_destroy || !detect_collision(entity->hitbox(), current_game_area_hitbox()))
				entity.reset();
		}
	for(auto it = entities_to_activate.begin(); it != entities_to_activate.end();)
	{
		if(!is_it_reachable((*it)->hitbox()))
		{
			auto erased_entity_iterator = it++;
			entities_to_activate.erase(erased_entity_iterator);
		}
		else if(detect_collision((*it)->hitbox(), current_game_area_hitbox()))
		{
			auto moved_entity_iterator = it++;
			entities.push_back(*moved_entity_iterator);
			entities_to_activate.erase(moved_entity_iterator);
		}
		else
			break;

	}
	for(auto& entity : entities_to_add)
		push_to_container(entity);
	entities.erase(std::remove(entities.begin(), entities.end(), nullptr), entities.end());
	entities_to_add.clear();
}

Zespolona Game::pozycja_gracza() const
{
	return gracz->pozycja();
}

int Game::paliwo_gracza_w_procentach() const
{
	return gracz->paliwo();
}

void Game::player_change_direction(Zespolona kierunek)
{
	gracz->skieruj_sie(kierunek);
}

void Game::player_shoot()
{
	gracz->shoot();
}

AxisAlignedBoundingBox Game::current_game_area_hitbox() const
{
	return AxisAlignedBoundingBox(Zespolona(0, pozycja_gracza().imag() + 800), Zespolona(level_width(), pozycja_gracza().imag() - 400));
}

Entity::~Entity()
{

}

std::unique_ptr<Entity> make_entity(const std::string& name, const std::string& params)
{
#define REGISTER_DATATYPE(...) do {\
	if(#__VA_ARGS__ == name)\
		return __VA_ARGS__::from_string(params);\
} while(0)
	REGISTER_DATATYPE(Bridge);
	REGISTER_DATATYPE(DynamicTank);
	REGISTER_DATATYPE(DynamicBoat);
	REGISTER_DATATYPE(Fighter);
	REGISTER_DATATYPE(FuelStation);
	REGISTER_DATATYPE(Heli);
	REGISTER_DATATYPE(House);
	REGISTER_DATATYPE(ImportantBridge);
	REGISTER_DATATYPE(NormalLand);
	REGISTER_DATATYPE(StaticBoat);
	REGISTER_DATATYPE(StaticTank);
	REGISTER_DATATYPE(Tree);
	assert(0);
	return nullptr;
}

bool Samolot::refresh(Game& game)
{
	if(destroyed())
	{
		game.create_entity(std::make_shared<LargeExplosion>(poz, 360));
		if(--lives != 0)
			game.schedule_action_after_n_ticks(400, [&](){ game.reset_level(); });
		else
			game.schedule_action_after_n_ticks(500, [&](){ game.set_game_state(Game::game_over); });
	}
	else
	{
		poz += predkosc * 0.2;
		if(std::abs(predkosc) >= 12.0)
			predkosc = std::polar(12.0, std::arg(predkosc));
		if(predkosc.imag() < 4.5) predkosc.imag(4.5);
		if(bullet_reload_time == bullet_timeout)
		{
			game.create_entity(std::make_shared<Bullet>(poz + Zespolona(0, 50), Zespolona(0.0, 10.0), 45));
		}
		if(bullet_reload_time != 0)
			--bullet_reload_time;
		fuel -= 2;
		if(fuel <= 0)
			hurt();
	}
	return destroyed();
}

int Game::lives() const
{
	return gracz->life_count();
}

bool entity_position_comparer::operator()(const std::shared_ptr<Entity>& lhs, const std::shared_ptr<Entity>& rhs) const
{
	return lhs->pozycja().imag() < rhs->pozycja().imag();
}

bool is_land_at(Game& game, Zespolona pozycja)
{
	for(auto& entity : game.active_entities())
	{
		if(dynamic_cast<Land*>(entity.get()))
		{
			bool land = detect_collision(entity->hitbox(), AxisAlignedBoundingBox(pozycja - Zespolona(10, 10), pozycja + Zespolona(10, 10)));
			if(land) return true;
		}
	}
	return false;
}