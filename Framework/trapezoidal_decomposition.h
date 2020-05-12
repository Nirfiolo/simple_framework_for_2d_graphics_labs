#pragma once


#include "dcel.h"


namespace frm
{
    struct GraphNode
    {
        enum class Type : uint8_t
        {
            // trapezoid index
            Leaf,
            // ends of line segment index
            XUnit,
            // line segments index
            YUnit
        };

        Type type;
        size_t index_by_type;

        std::shared_ptr<GraphNode> left_child{ nullptr };
        std::shared_ptr<GraphNode> right_child{ nullptr };
    };

    struct LineSegment
    {
        size_t begin_index;
        size_t end_index;
        size_t face_over_line;
        size_t face_under_line;
    };

    struct Trapezoid
    {
        size_t top_line_segment_index{ std::numeric_limits<size_t>::max() };
        size_t bottom_line_segment_index{ std::numeric_limits<size_t>::max() };

        size_t left_end_index{ std::numeric_limits<size_t>::max() };
        size_t right_end_index{ std::numeric_limits<size_t>::max() };

        //*neighbor_index == std::numeric_limits<size_t>::max() => neighbor does not exist
        size_t top_left_neighbor_index{ std::numeric_limits<size_t>::max() };
        size_t bottom_left_neighbor_index{ std::numeric_limits<size_t>::max() };
        size_t top_right_neighbor_index{ std::numeric_limits<size_t>::max() };
        size_t bottom_right_neighbor_index{ std::numeric_limits<size_t>::max() };

        std::shared_ptr<GraphNode> trapezoid_node;
    };

    struct TrapezoidData
    {
        std::vector<frm::Point> ends_of_line_segment;
        std::vector<LineSegment> line_segments;
        std::vector<Trapezoid> trapezoids;
    };


    // first parameter is outside face
    using trapezoid_data_and_graph_root_t = std::pair<size_t, std::pair<TrapezoidData, std::shared_ptr<GraphNode>>>;

    // O(nlog(n))
    trapezoid_data_and_graph_root_t generate_trapezoid_data_and_graph_root(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG);


    // O(log(n))
    size_t get_face_index(trapezoid_data_and_graph_root_t const & trapezoid_data_and_graph_root, frm::Point point) noexcept(!IS_DEBUG);
}