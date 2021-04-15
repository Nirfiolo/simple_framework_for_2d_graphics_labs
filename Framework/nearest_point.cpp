#include "nearest_point.h"


namespace frm
{
    size_t nearest_point(dcel::DCEL const & dcel, Point const point) noexcept
    {
        size_t index = 0;
        float sqr_distance = sqr_distance_between_points(point, dcel.vertices[index].coordinate);

        for (size_t i = 1; i < dcel.vertices.size(); ++i)
        {
            if (std::find(dcel.free_vertices.begin(), dcel.free_vertices.end(), i) == std::end(dcel.free_vertices))
            {
                if (sqr_distance > sqr_distance_between_points(point, dcel.vertices[i].coordinate))
                {
                    index = i;
                    sqr_distance = sqr_distance_between_points(point, dcel.vertices[i].coordinate);
                }
            }
        }

        return index;
    }
}
