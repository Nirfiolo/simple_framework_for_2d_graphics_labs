#include "nearest_line.h"


namespace frm
{
    size_t nearest_line(dcel::DCEL const & dcel, Point const point) noexcept
    {
        size_t index = 0;
        Point line_begin = dcel.vertices[dcel.edges[index].origin_vertex].coordinate;
        Point line_end = dcel.vertices[dcel.edges[dcel.edges[index].twin_edge].origin_vertex].coordinate;

        float distance = distance_between_point_and_line_segment(line_begin, line_end, point);

        for (size_t i = 1; i < dcel.edges.size(); ++i)
        {
            if (std::find(dcel.free_edges.begin(), dcel.free_edges.end(), i) == std::end(dcel.free_edges))
            {
                line_begin = dcel.vertices[dcel.edges[i].origin_vertex].coordinate;
                line_end = dcel.vertices[dcel.edges[dcel.edges[i].twin_edge].origin_vertex].coordinate;
                float current_distance = distance_between_point_and_line_segment(line_begin, line_end, point);

                if (distance > current_distance)
                {
                    index = i;
                    distance = current_distance;
                }
            }
        }

        line_begin = dcel.vertices[dcel.edges[index].origin_vertex].coordinate;
        line_end = dcel.vertices[dcel.edges[dcel.edges[index].twin_edge].origin_vertex].coordinate;

        if (is_point_on_left_side(line_begin, line_end, point))
        {
            return index;
        }

        return dcel.edges[index].twin_edge;
    }
}
