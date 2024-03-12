#pragma once

#include "config_parser.hpp"
#include "model_base.hpp"
#include "network.hpp"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
namespace fs = std::filesystem;

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
    void run( fs::path output_dir_path );
};

} // namespace Seldon