#pragma once

#include "model_base.hpp"
#include "network.hpp"
#include <memory>
#include <optional>
#include <random>
#include <string>

namespace Seldon
{

class Simulation
{
    struct OutputSettings
    {
        // Write out the agents/network every n iterations, nullopt means never
        std::optional<size_t> n_output_agents  = 1;
        std::optional<size_t> n_output_network = std::nullopt;
        bool print_progress                    = true; // Print the iteration time, by default always prints
    };

private:
    std::mt19937 gen;

public:
    int n_agents;
    std::unique_ptr<ModelBase> model;
    std::unique_ptr<Network> network;
    OutputSettings output_settings;
    Simulation(
        const std::string & config_file, const std::optional<std::string> & cli_network_file,
        const std::optional<std::string> & cli_agent_file );
};

} // namespace Seldon