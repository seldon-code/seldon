#pragma once
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace Seldon
{

/*
    A class that represents a directed graph using adjacency lists.
    Either incoming or outgoing edges are stored.
    To switch between the different representations (in/out)
    and the network and its transpose (N/N^T), refer to the following table:

              | N (inc)   |  N (out)  |  N^T (inc) | N^T (out) |
    ------------------------------------------------------------
    N (inc)   |     X     |  toggle   |  transpose |   switch  |
    N (out)   |  toggle   |     X     |  switch    | transpose |
    N^T (inc) | transpose |   switch  |      X     |  toggle   |
    N^T (out) |   switch  | transpose |   toggle   |     X     |

    Note: switch is equivalent to toggle + transpose, but much cheaper!
*/
class Network
{
public:
    enum class EdgeDirection
    {
        Incoming,
        Outgoing
    };

    using WeightT = double;

    Network(
        std::vector<std::vector<size_t>> && neighbour_list, std::vector<std::vector<WeightT>> && weight_list,
        EdgeDirection direction );

    /*
    Gives the total number of nodes in the network
    */
    [[nodiscard]] std::size_t n_agents() const;

    /*
    Gives the number of edges going out/coming in at agent_idx, depending on the value of direction().
    If agent_idx is nullopt, gives the total number of edges
    */
    [[nodiscard]] std::size_t n_edges( std::optional<std::size_t> agent_idx = std::nullopt ) const;

    /*
    Returns the current directionality of the adjacency list. That is if incoming or outgoing edges are stored.
    */
    [[nodiscard]] const EdgeDirection & direction() const;

    /*
    Gives the strongly connected components in the graph
    */
    [[nodiscard]] std::vector<std::vector<size_t>> strongly_connected_components() const;

    /*
        Gives a view into the neighbour indices going out/coming in at agent_idx
    */
    [[nodiscard]] std::span<const size_t> get_neighbours( std::size_t agent_idx ) const;
    [[nodiscard]] std::span<size_t> get_neighbours( std::size_t agent_idx );

    /*
    Gives a view into the edge weights going out/coming in at agent_idx
    */
    [[nodiscard]] std::span<const WeightT> get_weights( std::size_t agent_idx ) const;
    [[nodiscard]] std::span<WeightT> get_weights( std::size_t agent_idx );

    /*
    Gives a view into the edge weights going out/coming in at agent_idx
    */
    void set_weights( std::size_t agent_idx, std::span<const WeightT> weights );

    /*
    Sets the neighbour indices and sets the weight to a constant value at agent_idx
    */
    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, const WeightT & weight );

    /*
    Sets the neighbour indices and weights at agent_idx
    */
    void set_neighbours_and_weights(
        std::size_t agent_idx, std::span<const size_t> buffer_neighbours, std::span<const WeightT> buffer_weights );

    /*
    Adds an edge between agent_idx_i and agent_idx_j with weight w
    */
    void push_back_neighbour_and_weight( size_t agent_idx_i, size_t agent_idx_j, WeightT w );

    /*
    Transposes the network, without switching the direction flag (expensive).
    Example: N(inc) -> N(inc)^T
    */
    void transpose();

    /*
    Switches the direction flag *without* transposing the network (expensive)
    Example: N(inc) -> N(out)
    */
    void toggle_incoming_outgoing();

    /*
    Only switches the direction flag. This effectively transposes the network and, simultaneously, changes its
    representation.
    Example: N(inc) -> N^T(out)
    */
    void switch_direction_flag();

private:
    std::vector<std::vector<size_t>> neighbour_list; // Neighbour list for the connections
    std::vector<std::vector<WeightT>> weight_list;   // List for the interaction weights of each connection
    EdgeDirection _direction;
};

} // namespace Seldon