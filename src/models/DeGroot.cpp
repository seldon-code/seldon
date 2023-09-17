#include "models/DeGroot.hpp"
#include <iterator>

Seldon::DeGrootModel::DeGrootModel(State & state) : Model(state)
{
    agent_current_copy = std::vector<Agent>(state.agents.size());
}

void Seldon::DeGrootModel::run()
{
    std::vector<std::tuple<std::size_t, double>>  edge_buffer(1);

    for(std::size_t i=0; i<this->state.agents.size(); i++)
    {
        this->state.network.get_edges(i, edge_buffer);

        agent_current_copy[i].opinion = 0.0;

        for(auto & edge : edge_buffer)
        {
            int j;
            double weight;
            std::tie(j, weight) = edge;
            agent_current_copy[i].opinion += weight * state.agents[j].opinion;
        }
    }

    // Update the original agent opinions
    for(std::size_t i=0; i<this->state.agents.size(); i++)
    {
        state.agents[i] = this->agent_current_copy[i];
    }
}