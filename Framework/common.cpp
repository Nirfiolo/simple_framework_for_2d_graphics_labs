#include "common.h"

#include <string>


namespace frm
{
    std::ostream & operator<<(std::ostream & os, Point const & point) noexcept
    {
        os << "[ " << point.x << " , " << point.y << " ] ";

        return os;
    }

    std::istream & operator>>(std::istream & is, Point & point) noexcept
    {
        std::string additional_symbols;

        is >> additional_symbols >> point.x >> additional_symbols >> point.y >> additional_symbols;

        return is;
    }
}