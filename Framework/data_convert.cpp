#include "data_convert.h"

#include <cassert>


namespace frm
{
    vvve::VVVE dcel_to_vvve(dcel::DCEL const & dcel) noexcept
    {
        vvve::VVVE vvve{};

        assert(dcel.free_vertices.size() == 0);

        for (size_t i = 0; i < dcel.vertices.size(); ++i)
        {
            vvve::add_vertex(vvve, dcel.vertices[i].coordinate);
        }

        return vvve;
    }
}
