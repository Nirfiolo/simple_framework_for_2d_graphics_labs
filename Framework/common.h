#pragma once


#include <iostream>


namespace frm
{
	struct Point
	{
		float x;
		float y;
	};

	std::ostream & operator<<(std::ostream & os, Point const & point) noexcept;
	std::istream & operator>>(std::istream & is, Point & point) noexcept;
}