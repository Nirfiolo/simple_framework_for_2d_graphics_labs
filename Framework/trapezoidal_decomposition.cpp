#include "trapezoidal_decomposition.h"

#include <set>
#include <cassert>
#include <algorithm>
#include <random>
#include <iostream>


namespace frm
{
    void output_trapezoid(std::ostream & os, TrapezoidData const & trapezoid_data) noexcept
    {
        for (size_t i = 0; i < trapezoid_data.trapezoids.size(); ++i)
        {
            Trapezoid const & current_trapezoid = trapezoid_data.trapezoids[i];
            os << i << " : ";
            os << current_trapezoid.top_line_segment_index << " ";
            os << current_trapezoid.bottom_line_segment_index << " | ";

            os << current_trapezoid.left_end_index << " ";
            os << current_trapezoid.right_end_index << " | ";

            os << current_trapezoid.top_left_neighbor_index << " ";
            os << current_trapezoid.bottom_left_neighbor_index << " ";
            os << current_trapezoid.top_right_neighbor_index << " ";
            os << current_trapezoid.bottom_right_neighbor_index << " | ";

            os << static_cast<uint16_t>(current_trapezoid.trapezoid_node->type) << " " << current_trapezoid.trapezoid_node->index_by_type << '\n';
        }
    }

    void output_tree(std::ostream & os, std::shared_ptr<GraphNode> node, std::string offset) noexcept
    {
        os << offset << static_cast<uint16_t>(node->type) << " " << node->index_by_type << '\n';
        if (node->left_child)
        {
            output_tree(os, node->left_child, offset + "|   ");
        }
        if (node->right_child)
        {
            output_tree(os, node->right_child, offset + "|   ");
        }
    }

    std::shared_ptr<GraphNode> get_trapezoid_index(
        TrapezoidData const & trapezoid_data,
        std::shared_ptr<GraphNode> const & current,
        frm::Point point
    ) noexcept(!IS_DEBUG)
    {
        if (current->type == GraphNode::Type::Leaf)
        {
            return current;
        }

        if (current->type == GraphNode::Type::XUnit)
        {
            frm::Point const current_point = trapezoid_data.ends_of_line_segment[current->index_by_type];

            if (point.x - current_point.x > frm::epsilon)
            {
                return get_trapezoid_index(trapezoid_data, current->right_child, point);
            }
            return get_trapezoid_index(trapezoid_data, current->left_child, point);
        }

        if (current->type == GraphNode::Type::YUnit)
        {
            LineSegment const line_segment_indices = trapezoid_data.line_segments[current->index_by_type];

            frm::Point const begin_point = trapezoid_data.ends_of_line_segment[line_segment_indices.begin_index];
            frm::Point const end_point = trapezoid_data.ends_of_line_segment[line_segment_indices.end_index];

            float const k = (end_point.y - begin_point.y) / (end_point.x - begin_point.x);
            float const c = end_point.y - k * end_point.x;

            bool const is_point_over_line = frm::is_point_over_line(point, { k, c });

            if (is_point_over_line)
            {
                return get_trapezoid_index(trapezoid_data, current->left_child, point);
            }
            return get_trapezoid_index(trapezoid_data, current->right_child, point);
        }

        assert("Undefined graph node type" && false);
        return 0;
    }

    size_t get_free_trapezoid_index(TrapezoidData & trapezoid_data) noexcept(!IS_DEBUG)
    {
        size_t const index = trapezoid_data.trapezoids.size();
        trapezoid_data.trapezoids.push_back({});
        return index;
    }

    void update_famous_neighbors(TrapezoidData & trapezoid_data, size_t trapezoid_index) noexcept(!IS_DEBUG)
    {
        Trapezoid const & trapezoid = trapezoid_data.trapezoids[trapezoid_index];

        if (trapezoid.top_left_neighbor_index != std::numeric_limits<size_t>::max())
        {
            Trapezoid & neighbor_trapezoid = trapezoid_data.trapezoids[trapezoid.top_left_neighbor_index];
            neighbor_trapezoid.top_right_neighbor_index = trapezoid_index;
        }
        if (trapezoid.bottom_left_neighbor_index != std::numeric_limits<size_t>::max())
        {
            Trapezoid & neighbor_trapezoid = trapezoid_data.trapezoids[trapezoid.bottom_left_neighbor_index];
            neighbor_trapezoid.bottom_right_neighbor_index = trapezoid_index;
        }
        if (trapezoid.top_right_neighbor_index != std::numeric_limits<size_t>::max())
        {
            Trapezoid & neighbor_trapezoid = trapezoid_data.trapezoids[trapezoid.top_right_neighbor_index];
            neighbor_trapezoid.top_left_neighbor_index = trapezoid_index;
        }
        if (trapezoid.bottom_right_neighbor_index != std::numeric_limits<size_t>::max())
        {
            Trapezoid & neighbor_trapezoid = trapezoid_data.trapezoids[trapezoid.bottom_right_neighbor_index];
            neighbor_trapezoid.bottom_left_neighbor_index = trapezoid_index;
        }
    }


    void handle_inside_one_trapezoid(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        size_t begin_index,
        size_t end_index,
        size_t line_index
    ) noexcept(!IS_DEBUG)
    {
        bool const begin_is_existing_end = trapezoid_data.trapezoids[trapezoid->index_by_type].left_end_index == begin_index;
        bool const end_is_existing_end = trapezoid_data.trapezoids[trapezoid->index_by_type].right_end_index == end_index;

        if (begin_is_existing_end && end_is_existing_end)
        {
            size_t const old_trapezoid_index = trapezoid->index_by_type;
            Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

            size_t const top_trapezoid_index = old_trapezoid_index;
            size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = begin_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
            top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = begin_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            update_famous_neighbors(trapezoid_data, top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            std::shared_ptr<GraphNode> line_node = trapezoid;
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            line_node->left_child = top_trapezoid_node;
            line_node->right_child = bottom_trapezoid_node;
        }

        if (begin_is_existing_end && !end_is_existing_end)
        {
            size_t const old_trapezoid_index = trapezoid->index_by_type;
            Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

            size_t const right_trapezoid_index = old_trapezoid_index;
            size_t const top_trapezoid_index = get_free_trapezoid_index(trapezoid_data);
            size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

            Trapezoid & right_trapezoid = trapezoid_data.trapezoids[right_trapezoid_index];
            right_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            right_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            right_trapezoid.left_end_index = end_index;
            right_trapezoid.right_end_index = old_trapezoid.right_end_index;
            right_trapezoid.top_left_neighbor_index = top_trapezoid_index;
            right_trapezoid.bottom_left_neighbor_index = bottom_trapezoid_index;
            right_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            right_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = begin_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
            top_trapezoid.top_right_neighbor_index = right_trapezoid_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = begin_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = right_trapezoid_index;

            update_famous_neighbors(trapezoid_data, right_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> right_trapezoid_node = std::make_shared<GraphNode>();
            right_trapezoid_node->type = GraphNode::Type::Leaf;
            right_trapezoid_node->index_by_type = right_trapezoid_index;
            right_trapezoid.trapezoid_node = right_trapezoid_node;

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            std::shared_ptr<GraphNode> end_node = trapezoid;
            end_node->type = GraphNode::Type::XUnit;
            end_node->index_by_type = end_index;

            std::shared_ptr<GraphNode> line_node = std::make_shared<GraphNode>();
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            end_node->left_child = line_node;
            end_node->right_child = right_trapezoid_node;

            line_node->left_child = top_trapezoid_node;
            line_node->right_child = bottom_trapezoid_node;
        }

        if (!begin_is_existing_end && end_is_existing_end)
        {
            size_t const old_trapezoid_index = trapezoid->index_by_type;
            Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

            size_t const left_trapezoid_index = old_trapezoid_index;
            size_t const top_trapezoid_index = get_free_trapezoid_index(trapezoid_data);
            size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

            Trapezoid & left_trapezoid = trapezoid_data.trapezoids[left_trapezoid_index];
            left_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            left_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            left_trapezoid.left_end_index = old_trapezoid.left_end_index;
            left_trapezoid.right_end_index = begin_index;
            left_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            left_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            left_trapezoid.top_right_neighbor_index = top_trapezoid_index;
            left_trapezoid.bottom_right_neighbor_index = bottom_trapezoid_index;

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = begin_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = left_trapezoid_index;
            top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
            top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = begin_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_left_neighbor_index = left_trapezoid_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            update_famous_neighbors(trapezoid_data, left_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> left_trapezoid_node = std::make_shared<GraphNode>();
            left_trapezoid_node->type = GraphNode::Type::Leaf;
            left_trapezoid_node->index_by_type = left_trapezoid_index;
            left_trapezoid.trapezoid_node = left_trapezoid_node;

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            std::shared_ptr<GraphNode> begin_node = trapezoid;
            begin_node->type = GraphNode::Type::XUnit;
            begin_node->index_by_type = begin_index;

            std::shared_ptr<GraphNode> line_node = std::make_shared<GraphNode>();
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            begin_node->left_child = left_trapezoid_node;
            begin_node->right_child = line_node;

            line_node->left_child = top_trapezoid_node;
            line_node->right_child = bottom_trapezoid_node;
        }

        if (!begin_is_existing_end && !end_is_existing_end)
        {
            size_t const old_trapezoid_index = trapezoid->index_by_type;
            Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

            size_t const left_trapezoid_index = old_trapezoid_index;
            size_t const right_trapezoid_index = get_free_trapezoid_index(trapezoid_data);
            size_t const top_trapezoid_index = get_free_trapezoid_index(trapezoid_data);
            size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

            Trapezoid & left_trapezoid = trapezoid_data.trapezoids[left_trapezoid_index];
            left_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            left_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            left_trapezoid.left_end_index = old_trapezoid.left_end_index;
            left_trapezoid.right_end_index = begin_index;
            left_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            left_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            left_trapezoid.top_right_neighbor_index = top_trapezoid_index;
            left_trapezoid.bottom_right_neighbor_index = bottom_trapezoid_index;

            Trapezoid & right_trapezoid = trapezoid_data.trapezoids[right_trapezoid_index];
            right_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            right_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            right_trapezoid.left_end_index = end_index;
            right_trapezoid.right_end_index = old_trapezoid.right_end_index;
            right_trapezoid.top_left_neighbor_index = top_trapezoid_index;
            right_trapezoid.bottom_left_neighbor_index = bottom_trapezoid_index;
            right_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            right_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = begin_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = left_trapezoid_index;
            top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
            top_trapezoid.top_right_neighbor_index = right_trapezoid_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = begin_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_left_neighbor_index = left_trapezoid_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = right_trapezoid_index;

            update_famous_neighbors(trapezoid_data, left_trapezoid_index);
            update_famous_neighbors(trapezoid_data, right_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> left_trapezoid_node = std::make_shared<GraphNode>();
            left_trapezoid_node->type = GraphNode::Type::Leaf;
            left_trapezoid_node->index_by_type = left_trapezoid_index;
            left_trapezoid.trapezoid_node = left_trapezoid_node;

            std::shared_ptr<GraphNode> right_trapezoid_node = std::make_shared<GraphNode>();
            right_trapezoid_node->type = GraphNode::Type::Leaf;
            right_trapezoid_node->index_by_type = right_trapezoid_index;
            right_trapezoid.trapezoid_node = right_trapezoid_node;

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            std::shared_ptr<GraphNode> begin_node = trapezoid;
            begin_node->type = GraphNode::Type::XUnit;
            begin_node->index_by_type = begin_index;

            std::shared_ptr<GraphNode> end_node = std::make_shared<GraphNode>();
            end_node->type = GraphNode::Type::XUnit;
            end_node->index_by_type = end_index;

            std::shared_ptr<GraphNode> line_node = std::make_shared<GraphNode>();
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            begin_node->left_child = left_trapezoid_node;
            begin_node->right_child = end_node;

            end_node->left_child = line_node;
            end_node->right_child = right_trapezoid_node;

            line_node->left_child = top_trapezoid_node;
            line_node->right_child = bottom_trapezoid_node;
        }
    }

    void handle_first_trapezoid_with_existing_vertex(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        std::shared_ptr<GraphNode> & last_top_node,
        std::shared_ptr<GraphNode> & last_bottom_node,
        size_t begin_index,
        size_t end_index,
        size_t line_index,
        bool is_right_end_of_begin_trapezoid_over_line
    ) noexcept(!IS_DEBUG)
    {
        size_t const old_trapezoid_index = trapezoid->index_by_type;
        Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

        size_t const top_trapezoid_index = old_trapezoid_index;
        size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

        Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
        top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
        top_trapezoid.bottom_line_segment_index = line_index;
        top_trapezoid.left_end_index = begin_index;
        if (is_right_end_of_begin_trapezoid_over_line)
        {
            top_trapezoid.right_end_index = old_trapezoid.right_end_index;
        }
        top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
        top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
        top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
        top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

        Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
        bottom_trapezoid.top_line_segment_index = line_index;
        bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
        bottom_trapezoid.left_end_index = begin_index;
        if (!is_right_end_of_begin_trapezoid_over_line)
        {
            bottom_trapezoid.right_end_index = old_trapezoid.right_end_index;
        }
        bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
        bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

        update_famous_neighbors(trapezoid_data, top_trapezoid_index);
        update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

        std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
        top_trapezoid_node->type = GraphNode::Type::Leaf;
        top_trapezoid_node->index_by_type = top_trapezoid_index;
        top_trapezoid.trapezoid_node = top_trapezoid_node;

        std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
        bottom_trapezoid_node->type = GraphNode::Type::Leaf;
        bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
        bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

        std::shared_ptr<GraphNode> line_node = trapezoid;
        line_node->type = GraphNode::Type::YUnit;
        line_node->index_by_type = line_index;

        line_node->left_child = top_trapezoid_node;
        line_node->right_child = bottom_trapezoid_node;

        last_top_node = top_trapezoid_node;
        last_bottom_node = bottom_trapezoid_node;
    }

    void handle_first_trapezoid_without_existing_vertex(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        std::shared_ptr<GraphNode> & last_top_node,
        std::shared_ptr<GraphNode> & last_bottom_node,
        size_t begin_index,
        size_t end_index,
        size_t line_index,
        bool is_right_end_of_begin_trapezoid_over_line
    ) noexcept(!IS_DEBUG)
    {
        size_t const old_trapezoid_index = trapezoid->index_by_type;
        Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

        size_t const left_trapezoid_index = old_trapezoid_index;
        size_t const top_trapezoid_index = get_free_trapezoid_index(trapezoid_data);
        size_t const bottom_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

        Trapezoid & left_trapezoid = trapezoid_data.trapezoids[left_trapezoid_index];
        left_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
        left_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
        left_trapezoid.left_end_index = old_trapezoid.left_end_index;
        left_trapezoid.right_end_index = begin_index;
        left_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
        left_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
        left_trapezoid.top_right_neighbor_index = top_trapezoid_index;
        left_trapezoid.bottom_right_neighbor_index = bottom_trapezoid_index;

        Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
        top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
        top_trapezoid.bottom_line_segment_index = line_index;
        top_trapezoid.left_end_index = begin_index;
        if (is_right_end_of_begin_trapezoid_over_line)
        {
            top_trapezoid.right_end_index = old_trapezoid.right_end_index;
        }
        top_trapezoid.top_left_neighbor_index = left_trapezoid_index;
        top_trapezoid.bottom_left_neighbor_index = std::numeric_limits<size_t>::max();
        top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
        top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

        Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
        bottom_trapezoid.top_line_segment_index = line_index;
        bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
        bottom_trapezoid.left_end_index = begin_index;
        if (!is_right_end_of_begin_trapezoid_over_line)
        {
            bottom_trapezoid.right_end_index = old_trapezoid.right_end_index;
        }
        bottom_trapezoid.top_left_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_left_neighbor_index = left_trapezoid_index;
        bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

        update_famous_neighbors(trapezoid_data, left_trapezoid_index);
        update_famous_neighbors(trapezoid_data, top_trapezoid_index);
        update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

        std::shared_ptr<GraphNode> left_trapezoid_node = std::make_shared<GraphNode>();
        left_trapezoid_node->type = GraphNode::Type::Leaf;
        left_trapezoid_node->index_by_type = left_trapezoid_index;
        left_trapezoid.trapezoid_node = left_trapezoid_node;

        std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
        top_trapezoid_node->type = GraphNode::Type::Leaf;
        top_trapezoid_node->index_by_type = top_trapezoid_index;
        top_trapezoid.trapezoid_node = top_trapezoid_node;

        std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
        bottom_trapezoid_node->type = GraphNode::Type::Leaf;
        bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
        bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

        std::shared_ptr<GraphNode> begin_node = trapezoid;
        begin_node->type = GraphNode::Type::XUnit;
        begin_node->index_by_type = begin_index;

        std::shared_ptr<GraphNode> line_node = std::make_shared<GraphNode>();
        line_node->type = GraphNode::Type::YUnit;
        line_node->index_by_type = line_index;

        begin_node->left_child = left_trapezoid_node;
        begin_node->right_child = line_node;

        line_node->left_child = top_trapezoid_node;
        line_node->right_child = bottom_trapezoid_node;

        last_top_node = top_trapezoid_node;
        last_bottom_node = bottom_trapezoid_node;
    }


    void handle_middle_trapezoid(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        std::shared_ptr<GraphNode> & last_top_node,
        std::shared_ptr<GraphNode> & last_bottom_node,
        size_t begin_index,
        size_t end_index,
        size_t line_index,
        bool is_last_point_over_line,
        bool is_current_point_over_line
    ) noexcept(!IS_DEBUG)
    {
        size_t const old_trapezoid_index = trapezoid->index_by_type;
        Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

        size_t const last_top_trapezoid_index = last_top_node->index_by_type;
        size_t const last_bottom_trapezoid_index = last_bottom_node->index_by_type;

        Trapezoid & last_top_trapezoid = trapezoid_data.trapezoids[last_top_trapezoid_index];
        Trapezoid & last_bottom_trapezoid = trapezoid_data.trapezoids[last_bottom_trapezoid_index];

        if (is_last_point_over_line)
        {
            size_t const top_trapezoid_index = old_trapezoid_index;

            last_top_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_top_trapezoid.bottom_right_neighbor_index = top_trapezoid_index;

            if (!is_current_point_over_line)
            {
                last_bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;
            }

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = old_trapezoid.left_end_index;
            if (is_current_point_over_line)
            {
                top_trapezoid.right_end_index = old_trapezoid.right_end_index;
            }
            top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            top_trapezoid.bottom_left_neighbor_index = last_top_trapezoid_index;
            top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            update_famous_neighbors(trapezoid_data, last_top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            std::shared_ptr<GraphNode> line_node = trapezoid;
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            line_node->left_child = top_trapezoid_node;
            line_node->right_child = last_bottom_node;

            last_top_node = top_trapezoid_node;
        }
        else
        {
            size_t const bottom_trapezoid_index = old_trapezoid_index;

            last_bottom_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_bottom_trapezoid.top_right_neighbor_index = bottom_trapezoid_index;

            if (is_current_point_over_line)
            {
                last_top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            }

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = old_trapezoid.left_end_index;
            if (!is_current_point_over_line)
            {
                bottom_trapezoid.right_end_index = old_trapezoid.right_end_index;
            }
            bottom_trapezoid.top_left_neighbor_index = last_bottom_trapezoid_index;
            bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            update_famous_neighbors(trapezoid_data, last_bottom_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            std::shared_ptr<GraphNode> line_node = trapezoid;
            line_node->type = GraphNode::Type::YUnit;
            line_node->index_by_type = line_index;

            line_node->left_child = last_top_node;
            line_node->right_child = bottom_trapezoid_node;

            last_bottom_node = bottom_trapezoid_node;
        }
    }

    void handle_last_trapezoid_with_existing_vertex(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        std::shared_ptr<GraphNode> & last_top_node,
        std::shared_ptr<GraphNode> & last_bottom_node,
        size_t begin_index,
        size_t end_index,
        size_t line_index,
        bool is_last_point_over_line
    ) noexcept(!IS_DEBUG)
    {
        size_t const old_trapezoid_index = trapezoid->index_by_type;
        Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

        if (is_last_point_over_line)
        {
            size_t const last_top_trapezoid_index = last_top_node->index_by_type;
            size_t const top_trapezoid_index = old_trapezoid_index;

            Trapezoid & last_top_trapezoid = trapezoid_data.trapezoids[last_top_trapezoid_index];
            last_top_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_top_trapezoid.bottom_right_neighbor_index = top_trapezoid_index;

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = old_trapezoid.left_end_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            top_trapezoid.bottom_left_neighbor_index = last_top_trapezoid_index;
            top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            update_famous_neighbors(trapezoid_data, last_top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            last_top_node = top_trapezoid_node;
        }
        else
        {
            size_t const last_bottom_trapezoid_index = last_bottom_node->index_by_type;
            size_t const bottom_trapezoid_index = old_trapezoid_index;

            Trapezoid & last_bottom_trapezoid = trapezoid_data.trapezoids[last_bottom_trapezoid_index];
            last_bottom_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_bottom_trapezoid.top_right_neighbor_index = bottom_trapezoid_index;

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = old_trapezoid.left_end_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = last_bottom_trapezoid_index;
            bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            update_famous_neighbors(trapezoid_data, last_bottom_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            last_bottom_node = bottom_trapezoid_node;
        }

        size_t const top_trapezoid_index = last_top_node->index_by_type;
        size_t const bottom_trapezoid_index = last_bottom_node->index_by_type;

        Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
        top_trapezoid.right_end_index = end_index;
        top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
        top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

        Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
        bottom_trapezoid.right_end_index = end_index;
        bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

        update_famous_neighbors(trapezoid_data, top_trapezoid_index);
        update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

        std::shared_ptr<GraphNode> line_node = trapezoid;
        line_node->type = GraphNode::Type::YUnit;
        line_node->index_by_type = line_index;

        line_node->left_child = last_top_node;
        line_node->right_child = last_bottom_node;
    }

    void handle_last_trapezoid_without_existing_vertex(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> trapezoid,
        std::shared_ptr<GraphNode> & last_top_node,
        std::shared_ptr<GraphNode> & last_bottom_node,
        size_t begin_index,
        size_t end_index,
        size_t line_index,
        bool is_last_point_over_line
    ) noexcept(!IS_DEBUG)
    {
        size_t const old_trapezoid_index = trapezoid->index_by_type;
        Trapezoid const old_trapezoid = trapezoid_data.trapezoids[old_trapezoid_index];

        if (is_last_point_over_line)
        {
            size_t const last_top_trapezoid_index = last_top_node->index_by_type;
            size_t const top_trapezoid_index = old_trapezoid_index;

            Trapezoid & last_top_trapezoid = trapezoid_data.trapezoids[last_top_trapezoid_index];
            last_top_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_top_trapezoid.bottom_right_neighbor_index = top_trapezoid_index;

            Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
            top_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
            top_trapezoid.bottom_line_segment_index = line_index;
            top_trapezoid.left_end_index = old_trapezoid.left_end_index;
            top_trapezoid.right_end_index = end_index;
            top_trapezoid.top_left_neighbor_index = old_trapezoid.top_left_neighbor_index;
            top_trapezoid.bottom_left_neighbor_index = last_top_trapezoid_index;
            top_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
            top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

            update_famous_neighbors(trapezoid_data, last_top_trapezoid_index);
            update_famous_neighbors(trapezoid_data, top_trapezoid_index);

            std::shared_ptr<GraphNode> top_trapezoid_node = std::make_shared<GraphNode>();
            top_trapezoid_node->type = GraphNode::Type::Leaf;
            top_trapezoid_node->index_by_type = top_trapezoid_index;
            top_trapezoid.trapezoid_node = top_trapezoid_node;

            last_top_node = top_trapezoid_node;
        }
        else
        {
            size_t const last_bottom_trapezoid_index = last_bottom_node->index_by_type;
            size_t const bottom_trapezoid_index = old_trapezoid_index;

            Trapezoid & last_bottom_trapezoid = trapezoid_data.trapezoids[last_bottom_trapezoid_index];
            last_bottom_trapezoid.right_end_index = old_trapezoid.left_end_index;
            last_bottom_trapezoid.top_right_neighbor_index = bottom_trapezoid_index;

            Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
            bottom_trapezoid.top_line_segment_index = line_index;
            bottom_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
            bottom_trapezoid.left_end_index = old_trapezoid.left_end_index;
            bottom_trapezoid.right_end_index = end_index;
            bottom_trapezoid.top_left_neighbor_index = last_bottom_trapezoid_index;
            bottom_trapezoid.bottom_left_neighbor_index = old_trapezoid.bottom_left_neighbor_index;
            bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
            bottom_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

            update_famous_neighbors(trapezoid_data, last_bottom_trapezoid_index);
            update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

            std::shared_ptr<GraphNode> bottom_trapezoid_node = std::make_shared<GraphNode>();
            bottom_trapezoid_node->type = GraphNode::Type::Leaf;
            bottom_trapezoid_node->index_by_type = bottom_trapezoid_index;
            bottom_trapezoid.trapezoid_node = bottom_trapezoid_node;

            last_bottom_node = bottom_trapezoid_node;
        }

        size_t const top_trapezoid_index = last_top_node->index_by_type;
        size_t const bottom_trapezoid_index = last_bottom_node->index_by_type;
        size_t const right_trapezoid_index = get_free_trapezoid_index(trapezoid_data);

        Trapezoid & right_trapezoid = trapezoid_data.trapezoids[right_trapezoid_index];
        right_trapezoid.top_line_segment_index = old_trapezoid.top_line_segment_index;
        right_trapezoid.bottom_line_segment_index = old_trapezoid.bottom_line_segment_index;
        right_trapezoid.left_end_index = end_index;
        right_trapezoid.right_end_index = old_trapezoid.right_end_index;
        right_trapezoid.top_left_neighbor_index = top_trapezoid_index;
        right_trapezoid.bottom_left_neighbor_index = bottom_trapezoid_index;
        right_trapezoid.top_right_neighbor_index = old_trapezoid.top_right_neighbor_index;
        right_trapezoid.bottom_right_neighbor_index = old_trapezoid.bottom_right_neighbor_index;

        Trapezoid & top_trapezoid = trapezoid_data.trapezoids[top_trapezoid_index];
        top_trapezoid.right_end_index = end_index;
        top_trapezoid.top_right_neighbor_index = right_trapezoid_index;
        top_trapezoid.bottom_right_neighbor_index = std::numeric_limits<size_t>::max();

        Trapezoid & bottom_trapezoid = trapezoid_data.trapezoids[bottom_trapezoid_index];
        bottom_trapezoid.right_end_index = end_index;
        bottom_trapezoid.top_right_neighbor_index = std::numeric_limits<size_t>::max();
        bottom_trapezoid.bottom_right_neighbor_index = right_trapezoid_index;

        update_famous_neighbors(trapezoid_data, right_trapezoid_index);
        update_famous_neighbors(trapezoid_data, top_trapezoid_index);
        update_famous_neighbors(trapezoid_data, bottom_trapezoid_index);

        std::shared_ptr<GraphNode> right_trapezoid_node = std::make_shared<GraphNode>();
        right_trapezoid_node->type = GraphNode::Type::Leaf;
        right_trapezoid_node->index_by_type = right_trapezoid_index;
        right_trapezoid.trapezoid_node = right_trapezoid_node;

        std::shared_ptr<GraphNode> end_node = trapezoid;
        end_node->type = GraphNode::Type::XUnit;
        end_node->index_by_type = end_index;

        std::shared_ptr<GraphNode> line_node = std::make_shared<GraphNode>();
        line_node->type = GraphNode::Type::YUnit;
        line_node->index_by_type = line_index;

        end_node->left_child = line_node;
        end_node->right_child = right_trapezoid_node;

        line_node->left_child = last_top_node;
        line_node->right_child = last_bottom_node;
    }

    void handle_between_trapezoids(
        TrapezoidData & trapezoid_data,
        std::shared_ptr<GraphNode> begin_trapezoid,
        std::shared_ptr<GraphNode> end_trapezoid,
        size_t begin_index,
        size_t end_index,
        size_t line_index
    ) noexcept(!IS_DEBUG)
    {
        std::shared_ptr<GraphNode> last_top_node;
        std::shared_ptr<GraphNode> last_bottom_node;

        frm::Point const begin_point = trapezoid_data.ends_of_line_segment[begin_index];
        frm::Point const end_point = trapezoid_data.ends_of_line_segment[end_index];

        float const k = (end_point.y - begin_point.y) / (end_point.x - begin_point.x);
        float const c = end_point.y - k * end_point.x;

        bool const is_right_end_of_begin_trapezoid_over_line = frm::is_point_over_line(
            trapezoid_data.ends_of_line_segment[trapezoid_data.trapezoids[begin_trapezoid->index_by_type].right_end_index],
            { k, c }
        );

        std::shared_ptr<GraphNode> next_node;

        if (is_right_end_of_begin_trapezoid_over_line)
        {
            size_t const bottom_right_neighbor_index = trapezoid_data.trapezoids[begin_trapezoid->index_by_type].bottom_right_neighbor_index;
            next_node = trapezoid_data.trapezoids[bottom_right_neighbor_index].trapezoid_node;
        }
        else
        {
            size_t const top_right_neighbor_index = trapezoid_data.trapezoids[begin_trapezoid->index_by_type].top_right_neighbor_index;
            next_node = trapezoid_data.trapezoids[top_right_neighbor_index].trapezoid_node;
        }

        // connect to existing
        if (trapezoid_data.trapezoids[begin_trapezoid->index_by_type].left_end_index == begin_index)
        {
            handle_first_trapezoid_with_existing_vertex(
                trapezoid_data,
                begin_trapezoid,
                last_top_node,
                last_bottom_node,
                begin_index,
                end_index,
                line_index,
                is_right_end_of_begin_trapezoid_over_line
            );
        }
        else
        {
            handle_first_trapezoid_without_existing_vertex(
                trapezoid_data, begin_trapezoid,
                last_top_node, last_bottom_node,
                begin_index, end_index,
                line_index,
                is_right_end_of_begin_trapezoid_over_line
            );
        }

        bool is_last_point_over_line = is_right_end_of_begin_trapezoid_over_line;

        while (next_node != end_trapezoid)
        {
            std::shared_ptr<GraphNode> current_trapezoid = next_node;
            bool const is_current_point_over_line = frm::is_point_over_line(
                trapezoid_data.ends_of_line_segment[trapezoid_data.trapezoids[current_trapezoid->index_by_type].right_end_index],
                { k, c }
            );

            if (is_current_point_over_line)
            {
                size_t const bottom_right_neighbor_index = trapezoid_data.trapezoids[current_trapezoid->index_by_type].bottom_right_neighbor_index;
                next_node = trapezoid_data.trapezoids[bottom_right_neighbor_index].trapezoid_node;
            }
            else
            {
                size_t const top_right_neighbor_index = trapezoid_data.trapezoids[current_trapezoid->index_by_type].top_right_neighbor_index;
                next_node = trapezoid_data.trapezoids[top_right_neighbor_index].trapezoid_node;
            }

            handle_middle_trapezoid(
                trapezoid_data,
                current_trapezoid,
                last_top_node,
                last_bottom_node,
                begin_index,
                end_index,
                line_index,
                is_last_point_over_line,
                is_current_point_over_line
            );
            is_last_point_over_line = is_current_point_over_line;
        }

        // connect to existing
        if (trapezoid_data.trapezoids[end_trapezoid->index_by_type].right_end_index == end_index)
        {
            handle_last_trapezoid_with_existing_vertex(
                trapezoid_data,
                end_trapezoid,
                last_top_node,
                last_bottom_node,
                begin_index,
                end_index,
                line_index,
                is_last_point_over_line
            );
        }
        else
        {
            handle_last_trapezoid_without_existing_vertex(
                trapezoid_data,
                end_trapezoid,
                last_top_node,
                last_bottom_node,
                begin_index,
                end_index,
                line_index,
                is_last_point_over_line
            );
        }
    }

    trapezoid_data_and_graph_root_t generate_trapezoid_data_and_graph_root(frm::dcel::DCEL const & dcel) noexcept(!IS_DEBUG)
    {
        TrapezoidData trapezoid_data{};
        std::shared_ptr<GraphNode> root;

        size_t const outside_face_index = get_outside_face_index(dcel);

        // convert dcel to trapezoid data
        {
            trapezoid_data.ends_of_line_segment.resize(dcel.vertices.size());
            for (size_t i = 0; i < dcel.vertices.size(); ++i)
            {
                trapezoid_data.ends_of_line_segment[i] = dcel.vertices[i].coordinate;
            }

            std::vector<LineSegment> double_edges(dcel.edges.size());

            for (size_t i = 0; i < double_edges.size(); ++i)
            {
                size_t first_vertex = dcel.edges[i].origin_vertex;
                size_t second_vertex = dcel.edges[dcel.edges[i].twin_edge].origin_vertex;

                frm::Point const begin = dcel.vertices[first_vertex].coordinate;
                frm::Point const end = dcel.vertices[second_vertex].coordinate;

                size_t face_over_line;
                size_t face_under_line;

                if (end.x - begin.x < frm::epsilon)
                {
                    face_under_line = dcel.edges[i].incident_face;
                    face_over_line = dcel.edges[dcel.edges[i].twin_edge].incident_face;
                }
                else
                {
                    face_over_line = dcel.edges[i].incident_face;
                    face_under_line = dcel.edges[dcel.edges[i].twin_edge].incident_face;
                }

                if (second_vertex > first_vertex)
                {
                    std::swap(first_vertex, second_vertex);
                }

                double_edges[i] = { first_vertex, second_vertex, face_over_line, face_under_line };
            }

            std::sort(double_edges.begin(), double_edges.end(), [&dcel](LineSegment a, LineSegment b) noexcept -> bool
                {
                    if (a.begin_index == b.begin_index)
                    {
                        return a.end_index < b.end_index;
                    }
                    return a.begin_index < b.begin_index;
                });

            trapezoid_data.line_segments.resize(double_edges.size() / 2);

            for (size_t i = 0; i < double_edges.size(); i += 2)
            {
                trapezoid_data.line_segments[i / 2] = double_edges[i];
            }

            for (size_t i = 0; i < trapezoid_data.line_segments.size(); ++i)
            {
                size_t const begin_index = trapezoid_data.line_segments[i].begin_index;
                size_t const end_index = trapezoid_data.line_segments[i].end_index;
                frm::Point const begin = trapezoid_data.ends_of_line_segment[begin_index];
                frm::Point const end = trapezoid_data.ends_of_line_segment[end_index];

                if (begin.x - end.x > frm::epsilon)
                {
                    trapezoid_data.line_segments[i] = {
                        end_index,
                        begin_index,
                        trapezoid_data.line_segments[i].face_over_line,
                        trapezoid_data.line_segments[i].face_under_line
                    };
                }
            }

            std::shuffle(trapezoid_data.line_segments.begin(), trapezoid_data.line_segments.end(), std::default_random_engine{});
        }

        // init outside rectangle
        {
            float top = dcel.vertices[0].coordinate.y;
            float bottom = dcel.vertices[0].coordinate.y;
            float right = dcel.vertices[0].coordinate.x;
            float left = dcel.vertices[0].coordinate.x;

            for (size_t i = 0; i < dcel.vertices.size(); ++i)
            {
                top = std::max(top, dcel.vertices[i].coordinate.y);
                bottom = std::min(bottom, dcel.vertices[i].coordinate.y);
                right = std::max(right, dcel.vertices[i].coordinate.x);
                left = std::min(left, dcel.vertices[i].coordinate.x);
            }

            float const offset_to_side = 100.f * frm::epsilon;

            size_t const left_top_end = trapezoid_data.ends_of_line_segment.size();
            trapezoid_data.ends_of_line_segment.emplace_back(frm::Point{ left - offset_to_side, top + offset_to_side });
            size_t const right_top_end = trapezoid_data.ends_of_line_segment.size();
            trapezoid_data.ends_of_line_segment.emplace_back(frm::Point{ right + offset_to_side, top + offset_to_side });
            size_t const left_bottom_end = trapezoid_data.ends_of_line_segment.size();
            trapezoid_data.ends_of_line_segment.emplace_back(frm::Point{ left - offset_to_side, bottom - offset_to_side });
            size_t const right_bottom_end = trapezoid_data.ends_of_line_segment.size();
            trapezoid_data.ends_of_line_segment.emplace_back(frm::Point{ right + offset_to_side, bottom - offset_to_side });

            size_t const top_line_segment_index = trapezoid_data.line_segments.size();
            trapezoid_data.line_segments.emplace_back(LineSegment{ left_top_end, right_top_end, outside_face_index, outside_face_index });
            size_t const bottom_line_segment_index = trapezoid_data.line_segments.size();
            trapezoid_data.line_segments.emplace_back(LineSegment{ left_bottom_end, right_bottom_end, outside_face_index, outside_face_index });

            size_t const outside_rectangle_index = trapezoid_data.trapezoids.size();
            trapezoid_data.trapezoids.push_back({});
            Trapezoid & outside_rectangle = trapezoid_data.trapezoids[outside_rectangle_index];

            outside_rectangle.top_line_segment_index = top_line_segment_index;
            outside_rectangle.bottom_line_segment_index = bottom_line_segment_index;

            outside_rectangle.left_end_index = left_top_end;
            outside_rectangle.right_end_index = right_top_end;

            root = std::make_shared<GraphNode>();
            root->type = GraphNode::Type::Leaf;
            root->index_by_type = outside_rectangle_index;
            outside_rectangle.trapezoid_node = root;
        }

        for (size_t i = 0; i < trapezoid_data.line_segments.size() - 2; ++i)
        {
            size_t const begin_index = trapezoid_data.line_segments[i].begin_index;
            size_t const end_index = trapezoid_data.line_segments[i].end_index;

            frm::Point const begin = trapezoid_data.ends_of_line_segment[begin_index];
            frm::Point const end = trapezoid_data.ends_of_line_segment[end_index];

            if (abs(begin.x - end.x) > frm::epsilon)
            {
                frm::Point const begin_offseted = frm::lerp(begin, end, frm::epsilon * 10.f);
                frm::Point const end_offseted = frm::lerp(end, begin, frm::epsilon * 10.f);

                std::shared_ptr<GraphNode> begin_trapezoid = get_trapezoid_index(trapezoid_data, root, begin_offseted);
                std::shared_ptr<GraphNode> end_trapezoid = get_trapezoid_index(trapezoid_data, root, end_offseted);

                if (begin_trapezoid == end_trapezoid)
                {
                    handle_inside_one_trapezoid(trapezoid_data, begin_trapezoid, begin_index, end_index, i);
                }
                if (begin_trapezoid != end_trapezoid)
                {
                    handle_between_trapezoids(trapezoid_data, begin_trapezoid, end_trapezoid, begin_index, end_index, i);
                }
            }
        }

        trapezoid_data_and_graph_root_t trapezoid_data_and_graph_root{};
        trapezoid_data_and_graph_root.first = outside_face_index;
        trapezoid_data_and_graph_root.second.first = std::move(trapezoid_data);
        trapezoid_data_and_graph_root.second.second = root;

        return trapezoid_data_and_graph_root;
    }

    size_t get_face_index(trapezoid_data_and_graph_root_t const & trapezoid_data_and_graph_root, frm::Point point) noexcept(!IS_DEBUG)
    {
        std::shared_ptr<GraphNode> trapezoid_node = get_trapezoid_index(trapezoid_data_and_graph_root.second.first, trapezoid_data_and_graph_root.second.second, point);

        size_t const trapezoid_index = trapezoid_node->index_by_type;
        Trapezoid const trapezoid = trapezoid_data_and_graph_root.second.first.trapezoids[trapezoid_index];

        LineSegment top_line_segment = trapezoid_data_and_graph_root.second.first.line_segments[trapezoid.top_line_segment_index];
        LineSegment bottom_line_segment = trapezoid_data_and_graph_root.second.first.line_segments[trapezoid.bottom_line_segment_index];

        assert(top_line_segment.face_under_line == bottom_line_segment.face_over_line);

        return bottom_line_segment.face_over_line;
    }
}