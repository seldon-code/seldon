#pragma once

#include "config_parser.hpp"
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

private:
    std::mt19937 gen;

public:

    std::unique_ptr<ModelBase> model;
    std::unique_ptr<Network> network;
    Config::OutputSettings output_settings;
    Simulation(
        const Config::SimulationOptions & options, const std::optional<std::string> & cli_network_file,
        const std::optional<std::string> & cli_agent_file );
};

} // namespace Seldon