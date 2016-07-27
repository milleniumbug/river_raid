#pragma once
#include "stdafx.hpp"

typedef std::shared_ptr<SDL_Surface> Sprite;
typedef std::complex<double> Zespolona;

//quit with error message
void die(const char* message, int errcode = -1);
SDL_Surface* raw_get_sprite(const std::string& name);
Sprite get_sprite(const std::string& name);

static const Zespolona left = Zespolona(-1, 0);
static const Zespolona right = Zespolona(1, 0);
static const Zespolona up = Zespolona(0, 1);
static const Zespolona down = Zespolona(0, -1);
static const double pi = 3.1415927;

template<typename Iterator>
class Range
{
	Iterator b;
	Iterator e;
public:
	Range(Iterator b_, Iterator e_) :
		b(b_),
		e(e_)
	{

	}

	Iterator begin() { return b; }
	Iterator end() { return e; }
};

template<typename Iterator>
Range<Iterator> range(Iterator b, Iterator e)
{
	return Range<Iterator>(b, e);
}

uint32_t timer_callback(uint32_t interval, void* data);

class AxisAlignedBoundingBox
{
	double left, right, top, bottom;
public:
	friend bool detect_collision(const AxisAlignedBoundingBox& first, const AxisAlignedBoundingBox& second)
	{
		return !(second.left > first.right
        || second.right < first.left
        || second.top < first.bottom
        || second.bottom > first.top);
	}

	AxisAlignedBoundingBox(Zespolona p1, Zespolona p2)
	{
		left = std::min(p1.real(), p2.real());
		top = std::max(p1.imag(), p2.imag());
		right = std::max(p1.real(), p2.real());
		bottom = std::min(p1.imag(), p2.imag());
	}
};