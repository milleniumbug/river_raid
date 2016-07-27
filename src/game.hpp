#pragma once
#include "stdafx.hpp"
#include "utilities.hpp"
class DynamicEntity;
class Entity;
class Samolot;
typedef std::vector<std::shared_ptr<Entity>> dynamic_entity_container_type;
static_assert(std::is_convertible<std::iterator_traits<dynamic_entity_container_type::iterator>::iterator_category,
	std::bidirectional_iterator_tag>::value, "dynamic_entity_container_type's iterator must be at least bidirectional");

struct entity_position_comparer
{
	bool operator()(const std::shared_ptr<Entity>& lhs, const std::shared_ptr<Entity>& rhs) const;
};

class Game
{
public:
	enum GameState { off, on, game_over, victory };
private:
	int points;
	std::shared_ptr<Samolot> gracz;
	GameState gst;
	bool level_reset_requested, full_reset_requested;
	int current_level;
	dynamic_entity_container_type entities;
	std::multiset<std::shared_ptr<Entity>, entity_position_comparer> entities_to_activate;
	dynamic_entity_container_type entities_to_add;
	std::list<std::pair<int, std::function<void()>>> actions;

	void push_to_container(std::shared_ptr<Entity> e);
	AxisAlignedBoundingBox current_game_area_hitbox() const;
	bool is_it_reachable(const AxisAlignedBoundingBox& box) const;
	void reset_all_commit();
	void reset_level_commit();
public:
	int increase_score(unsigned bonus) { return points += bonus; }
	int score() const { return points; }
	int lives() const;
	GameState game_state() const { return gst; }
	GameState set_game_state(GameState s) { return gst = s; }
	void reset_all();
	void reset_level();
	void next_level() { ++current_level; reset_level(); }
	void schedule_action_after_n_ticks(int delay, std::function<void()> f);
	Range<dynamic_entity_container_type::const_iterator> active_entities() const
	{
		return Range<dynamic_entity_container_type::const_iterator>(entities.begin(), entities.end());
	}
	double level_width() const
	{
		return 640;
	}
	void create_entity(std::shared_ptr<DynamicEntity> e);
	void player_change_direction(Zespolona kierunek);
	int paliwo_gracza_w_procentach() const;
	void player_shoot();
	void refresh();
	void load_from_file();
	Zespolona pozycja_gracza() const;
	Game();
};

bool is_land_at(Game& game, Zespolona pozycja);

class Entity
{
protected:
	Zespolona poz;
	virtual void collide_with_impl(Entity& other)
	{
		
	}

public:
	virtual bool refresh(Game& game) { return false; }
	void collide_with(Entity& other)
	{
		other.collide_with_impl(*this);
		this->collide_with_impl(other);
	}
	Zespolona pozycja() { return poz; }
	virtual AxisAlignedBoundingBox hitbox() = 0;
	virtual ~Entity();
	Entity(Zespolona pozycja) :
		poz(pozycja)
	{

	}
};

class DynamicEntity : public Entity
{
public:
	virtual bool refresh(Game& game) { return false; }
	DynamicEntity(Zespolona pozycja) :
		Entity(pozycja)
	{

	}
};

class Destroyable
{
protected:
	const int hit_endurance;
	int hits_taken;
public:
	void hurt() { ++hits_taken; }
	bool destroyed() { return hits_taken >= hit_endurance; }
	virtual ~Destroyable() {};
	Destroyable(int endurance = 1) :
		hit_endurance(endurance),
		hits_taken(0)
	{

	}
};

class Enemy
{
public:
	virtual ~Enemy() {};
};

class CannotWorkAboveWater
{
public:
	virtual ~CannotWorkAboveWater() {};
};

class CannotWorkOnLand
{
public:
	virtual ~CannotWorkOnLand() {};
};

class EntityWithDirection
{
protected:
	Zespolona kierunek;
public:
	Zespolona direction() { return kierunek; }

	EntityWithDirection(Zespolona k = Zespolona(0.0, 0.0)) :
		kierunek(k)
	{

	}
};

class Explosion : public DynamicEntity
{
	int ttl;
	int initial_ttl;
public:
	bool refresh(Game& game) override
	{
		if(ttl-- == 0)
			return true;
		return false;
	}

	int time_to_live()
	{
		return ttl;
	}

	int initial_time_to_live()
	{
		return initial_ttl;
	}

	Explosion(Zespolona poz, int time) :
		DynamicEntity(poz),
		ttl(time),
		initial_ttl(time)
	{
		
	}
};

class SmallExplosion : public Explosion
{
	static const int radius = 12;
public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-radius, -radius), poz + Zespolona(+radius, +radius));
	}

	SmallExplosion(Zespolona poz, int time) :
		Explosion(poz, time)
	{
		
	}
};

class MediumExplosion : public Explosion
{
	static const int radius = 25;
public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-radius, -radius), poz + Zespolona(+radius, +radius));
	}

	MediumExplosion(Zespolona poz, int time) :
		Explosion(poz, time)
	{
		
	}
};

class LargeExplosion : public Explosion
{
	static const int radius = 50;
public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-radius, -radius), poz + Zespolona(+radius, +radius));
	}
	LargeExplosion(Zespolona poz, int time) :
		Explosion(poz, time)
	{
		
	}
};

class Projectile : public DynamicEntity, public Destroyable, public EntityWithDirection
{
public:
	bool refresh(Game& game)
	{
		poz += kierunek;
		return destroyed();
	}

	Projectile(Zespolona poz, Zespolona k) :
		DynamicEntity(poz),
		EntityWithDirection(k)
	{
		
	}
};

class Bullet : public Projectile
{
	int time_to_live;
	static const int radius = 10;
public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-radius, -radius), poz + Zespolona(+radius, +radius));
	}
	Bullet(Zespolona poz, Zespolona k, int ttl = 40) :
		Projectile(poz, k),
		time_to_live(ttl)
	{
		
	}

	bool refresh(Game& game) override
	{
		if(--time_to_live == 0)
		{
			game.create_entity(std::make_shared<SmallExplosion>(poz, 40));
			return true;
		}
		return Projectile::refresh(game);
	}
};

//pseudo-entity that holds water properties
class Water : public Entity
{
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<CannotWorkAboveWater*>(&en))
			if(auto e = dynamic_cast<Destroyable*>(&en))
				e->hurt();
	}
public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-3, -3), poz + Zespolona(+3, +3));
	}

	Water(Zespolona pozycja) :
		Entity(pozycja)
	{

	}
};

class Land : public DynamicEntity
{
protected:
	bool refresh(Game& game) override
	{
		return false;
	}

	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<CannotWorkOnLand*>(&en))
			if(auto e = dynamic_cast<Destroyable*>(&en))
				e->hurt();
	}

public:
	Land(Zespolona poz) :
		DynamicEntity(poz)
	{
		
	}
};

class NormalLand : public Land
{
	std::array<Zespolona, 4> boundaries;
public:
	std::array<Zespolona, 4> granice()
	{
		return boundaries;
	}

	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(boundaries[0], boundaries[3]);
	}

	NormalLand(Zespolona p0, Zespolona p3) :
		Land(p0)
	{
		//TODO
		boundaries[0] = p0;
		boundaries[1] = Zespolona(0, 0);
		boundaries[2] = Zespolona(0, 0);
		boundaries[3] = p3;
	}

	static std::unique_ptr<NormalLand> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona p0, p3;
		ss >> p0 >> p3;
		return std::unique_ptr<NormalLand>(new NormalLand(p0, p3));
	}
};

class Bridge : public Land, public Destroyable
{
protected:
	void collide_with_impl(Entity& en) override
	{
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
		Land::collide_with_impl(en);
	}
public:
	bool refresh(Game& game) override
	{
		if(destroyed())
		{
			game.create_entity(std::make_shared<LargeExplosion>(poz - Zespolona(-70, 0), 40));
			game.create_entity(std::make_shared<LargeExplosion>(poz - Zespolona(0, 0), 40));
			game.create_entity(std::make_shared<LargeExplosion>(poz - Zespolona(+70, 0), 40));
		}
		return destroyed();
	}

	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-192, -75), poz + Zespolona(+192, +75));
	}

	Bridge(Zespolona poz) :
		Land(poz),
		Destroyable(5)
	{

	}

	static std::unique_ptr<Bridge> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<Bridge>(new Bridge(pozycja));
	}
};

//Important bridge is the bridge that entails level change
class ImportantBridge : public Bridge
{
public:
	bool refresh(Game& game) override
	{
		if(destroyed())
			game.schedule_action_after_n_ticks(60, [&](){ game.next_level(); });
		return Bridge::refresh(game);
	}

	ImportantBridge(Zespolona poz) :
		Bridge(poz)
	{

	}

	static std::unique_ptr<ImportantBridge> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<ImportantBridge>(new ImportantBridge(pozycja));
	}
};

class TankShell : public Projectile
{
	int time_to_live;
	static const int radius = 4;
protected:
	void collide_with_impl(Entity& en)
	{
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			this->hurt();
			b->hurt();
		}
	}

public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-radius, -radius), poz + Zespolona(+radius, +radius));
	}
	bool refresh(Game& game)
	{
		if(--time_to_live == 0)
		{
			game.create_entity(std::make_shared<MediumExplosion>(poz, 90));
			return true;
		}
		if(destroyed())
			game.increase_score(5);
		return Projectile::refresh(game);
	}

	TankShell(Zespolona poz, Zespolona k, int ttl) :
		Projectile(poz, k),
		time_to_live(ttl)
	{
		
	}
};

class Samolot : public DynamicEntity, public Destroyable, public CannotWorkOnLand
{
	static const int bullet_timeout = 30;
	static const int max_fuel = 12000;
	Zespolona predkosc;
	double fuel;
	int bullet_reload_time;
	int lives;
protected:
	void collide_with_impl(Entity& en)
	{
		if(auto o = dynamic_cast<Enemy*>(&en))
			this->hurt();
		if(auto e = dynamic_cast<Explosion*>(&en))
			this->hurt();
		if(auto e = dynamic_cast<Bullet*>(&en))
			this->hurt();
	}

public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-18, -21), poz + Zespolona(+18, +21));
	}
	bool refresh(Game& game);
	int paliwo()
	{
		return static_cast<int>(100.0 * fuel / max_fuel);
	}
	void refuel()
	{
		if(fuel <= max_fuel)
			fuel += 30;
	}
	int life_count() const
	{
		return lives;
	}
	void skieruj_sie(Zespolona kierunek)
	{
		if(!destroyed())
		{
			poz += kierunek.real() * 3;
			predkosc += Zespolona(0, kierunek.imag());
		}
	}
	void shoot()
	{
		if(bullet_reload_time == 0)
			bullet_reload_time = bullet_timeout;
	}

	Samolot(Zespolona poz, int l = 3) :
		DynamicEntity(poz),
		predkosc(0, 1.0),
		fuel(max_fuel),
		bullet_reload_time(0),
		lives(l)
	{
		
	}
};

class Boat : public DynamicEntity, public Destroyable, public Enemy, public EntityWithDirection
{
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<Explosion*>(&en))
			hurt();
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
	}

public:
	bool refresh(Game& game) override
	{
		if(destroyed())
		{
			game.create_entity(std::make_shared<MediumExplosion>(poz, 30));
			game.increase_score(20);
		}
		return destroyed();
	}

	AxisAlignedBoundingBox hitbox()
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-30, -10), poz + Zespolona(30, 10));
	}

	Boat(Zespolona poz, Zespolona k = Zespolona(1, 0)) :
		DynamicEntity(poz),
		EntityWithDirection(k)
	{

	}
};

class StaticBoat : public Boat
{
public:
	StaticBoat(Zespolona poz, Zespolona k = Zespolona(1, 0)) :
		Boat(poz, k)
	{
		
	}

	static std::unique_ptr<StaticBoat> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja, kierunek;
		ss >> pozycja;
		if(!(ss >> kierunek))
			kierunek = Zespolona(1, 0);
		return std::unique_ptr<StaticBoat>(new StaticBoat(pozycja, kierunek));
	}
};

class DynamicBoat : public Boat
{
public:
	bool refresh(Game& game) override
	{
		if(is_land_at(game, poz + kierunek * 10.0))
			kierunek = -kierunek;
		else
			poz += kierunek * 1.0;
		return Boat::refresh(game);
	}

	DynamicBoat(Zespolona poz, Zespolona k = Zespolona(1, 0)) :
		Boat(poz, k)
	{
		
	}

	static std::unique_ptr<DynamicBoat> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja, kierunek;
		ss >> pozycja;
		if(!(ss >> kierunek))
			kierunek = Zespolona(1, 0);
		return std::unique_ptr<DynamicBoat>(new DynamicBoat(pozycja, kierunek));
	}
};

class Heli : public DynamicEntity, public Destroyable, public Enemy, public EntityWithDirection
{
	static const int bullet_timeout = 30;
	int bullet_reload_time;
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<Explosion*>(&en))
			hurt();
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
	}

public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-24, -14), poz + Zespolona(+24, +14));
	}

	bool refresh(Game& game) override
	{
		if(is_land_at(game, poz + kierunek * 10.0))
			kierunek = -kierunek;
		else
			poz += kierunek * 1.0;
		if(bullet_reload_time == bullet_timeout)
		{
			game.create_entity(std::make_shared<Bullet>(poz + Zespolona(50, 0) * kierunek, kierunek * 3.0, 60));
		}
		if(bullet_reload_time != 0)
			--bullet_reload_time;
		shoot();
		if(destroyed())
		{
			game.create_entity(std::make_shared<MediumExplosion>(poz, 30));
			game.increase_score(50);
		}
		return destroyed();
	}

	void shoot()
	{
		if(bullet_reload_time == 0)
			bullet_reload_time = bullet_timeout;
	}

	Heli(Zespolona poz, Zespolona k = Zespolona(1.0, 0)) :
		DynamicEntity(poz),
		EntityWithDirection(k),
		bullet_reload_time(0)
	{
		
	}

	static std::unique_ptr<Heli> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja, kierunek;
		ss >> pozycja;
		if(!(ss >> kierunek))
			kierunek = Zespolona(1, 0);
		return std::unique_ptr<Heli>(new Heli(pozycja, kierunek));
	}
};

class Tank : public DynamicEntity, public Destroyable, public Enemy, public CannotWorkAboveWater
{
	Zespolona turret_facing_direction;
	static const int bullet_timeout = 240;
	int bullet_reload_time;
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<Explosion*>(&en))
			hurt();
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
	}

public:
	AxisAlignedBoundingBox hitbox() override
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-40, -21), poz + Zespolona(+40, +21));
	}

	Zespolona turret_direction() { return turret_facing_direction; }
	bool refresh(Game& game) override
	{
		auto dir = std::arg(game.pozycja_gracza() - pozycja());
		auto p1_dir = dir - pi/36;
		auto p2_dir = dir + pi/36;
		turret_facing_direction = std::polar(1.0, dir);
		if(bullet_reload_time == bullet_timeout)
		{
			game.create_entity(std::make_shared<TankShell>(poz, std::polar(1.0, p1_dir) * 2.0, 145));
			game.create_entity(std::make_shared<TankShell>(poz, std::polar(1.0, p2_dir) * 2.0, 145));
		}
		if(bullet_reload_time != 0)
			--bullet_reload_time;
		shoot();
		if(!is_land_at(game, poz))
			hurt();
		if(destroyed())
		{
			game.create_entity(std::make_shared<MediumExplosion>(poz, 30));
			game.increase_score(250);
		}
		return destroyed();
	}

	void shoot()
	{
		if(bullet_reload_time == 0)
			bullet_reload_time = bullet_timeout;
	}

	Tank(Zespolona poz) :
		DynamicEntity(poz),
		turret_facing_direction(0.0, 1.0),
		bullet_reload_time(0)
	{
		
	}
};

class StaticTank : public Tank
{
public:
	StaticTank(Zespolona poz) :
		Tank(poz)
	{
		
	}

	static std::unique_ptr<StaticTank> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<StaticTank>(new StaticTank(pozycja));
	}
};

class DynamicTank : public Tank, public EntityWithDirection
{
	Zespolona kierunek;
public:
	bool refresh(Game& game) override
	{
		//sprawdzamy czy mo?emy przejecha?o ca?ej szeroko?ci planszy
		for(int i = 0; i < game.level_width(); i += 30)
		{
			if(!is_land_at(game, Zespolona(i, poz.imag())))
			{
				kierunek = Zespolona(0.0, 0.0); //je?eli nie, zatrzymaj czo?g
				break;
			}
		}
		if(!is_land_at(game, poz + kierunek * 55.0))
			kierunek = -kierunek;
		else
			poz += kierunek * 0.5;
		return Tank::refresh(game);
	}

	DynamicTank(Zespolona poz, Zespolona k) :
		Tank(poz),
		kierunek(k)
	{
		
	}

	static std::unique_ptr<DynamicTank> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja, kierunek;
		ss >> pozycja;
		if(!(ss >> kierunek))
			kierunek = Zespolona(1, 0);
		return std::unique_ptr<DynamicTank>(new DynamicTank(pozycja, kierunek));
	}
};

class Fighter : public DynamicEntity, public Destroyable, public Enemy, public EntityWithDirection
{
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<Explosion*>(&en))
			hurt();
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
	}

public:
	AxisAlignedBoundingBox hitbox()
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-25, -16), poz + Zespolona(+25, +16));
	}

	bool refresh(Game& game) override
	{
		poz += direction() * 5.0;
		if(poz.real() > game.level_width())
			poz.real(0);
		if(poz.real() < 0)
			poz.real(game.level_width());
		if(destroyed())
		{
			game.create_entity(std::make_shared<MediumExplosion>(poz, 30));
			game.increase_score(100);
		}
		return destroyed();
	}

	Fighter(Zespolona poz, Zespolona k = Zespolona(1.0, 0.0)) :
		DynamicEntity(poz),
		EntityWithDirection(k)
	{
		
	}

	static std::unique_ptr<Fighter> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja, kierunek;
		ss >> pozycja;
		if(!(ss >> kierunek))
			kierunek = Zespolona(1, 0);
		return std::unique_ptr<Fighter>(new Fighter(pozycja, kierunek));
	}
};

class FuelStation : public DynamicEntity, public Destroyable
{
protected:
	void collide_with_impl(Entity& en)
	{
		if(dynamic_cast<Explosion*>(&en))
			hurt();
		if(auto b = dynamic_cast<Bullet*>(&en))
		{
			hurt();
			b->hurt();
		}
		if(auto p = dynamic_cast<Samolot*>(&en))
			p->refuel();
	}

public:
	AxisAlignedBoundingBox hitbox()
	{
		return AxisAlignedBoundingBox(poz + Zespolona(-28, -25), poz + Zespolona(+28, +25));
	}

	FuelStation(Zespolona poz) :
		DynamicEntity(poz)
	{
		
	}

	bool refresh(Game& game) override
	{
		if(destroyed())
		{
			game.create_entity(std::make_shared<LargeExplosion>(poz, 30));
			game.increase_score(40);
		}
		return destroyed();
	}

	static std::unique_ptr<FuelStation> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<FuelStation>(new FuelStation(pozycja));
	}
};

class Tree : public Entity
{
public:
	AxisAlignedBoundingBox hitbox()
	{
		return AxisAlignedBoundingBox(poz, poz);
	}

	Tree(Zespolona poz) :
		Entity(poz)
	{
		
	}

	static std::unique_ptr<Tree> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<Tree>(new Tree(pozycja));
	}
};

class House : public Entity
{
public:
	AxisAlignedBoundingBox hitbox()
	{
		return AxisAlignedBoundingBox(poz, poz);
	}

	House(Zespolona poz) :
		Entity(poz)
	{
		
	}

	static std::unique_ptr<House> from_string(const std::string& s)
	{
		std::stringstream ss(s);
		Zespolona pozycja;
		ss >> pozycja;
		return std::unique_ptr<House>(new House(pozycja));
	}
};

std::unique_ptr<Entity> make_entity(const std::string& name, const std::string& params);