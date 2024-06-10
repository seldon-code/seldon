#include "config_parser.hpp"
#include "models/DeGroot.hpp"
#include "models/DeffuantModel.hpp"
#include "models/InertialModel.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "simulation.hpp"
#include <optional>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void run_model(const fs::path config_file_path, std::optional<std::string> agent_file,std::optional<std::string> network_file, const fs::path output_dir_path)
{
    auto simulation_options = Seldon::Config::parse_config_file( config_file_path.string() );
    Seldon::Config::validate_settings( simulation_options );
    Seldon::Config::print_settings( simulation_options );

    if( network_file.has_value() )
    {
        fmt::print( "Reading network from file {}\n", network_file.value() );
    }
    if( agent_file.has_value() )
    {
        fmt::print( "Reading agents from file {}\n", agent_file.value() );
    }

    std::unique_ptr<Seldon::SimulationInterface> simulation;

    if( simulation_options.model == Seldon::Config::Model::DeGroot )
    {
        simulation = std::make_unique<Seldon::Simulation<Seldon::DeGrootModel::AgentT>>(
            simulation_options, network_file, agent_file );
    }
    else if( simulation_options.model == Seldon::Config::Model::ActivityDrivenModel )
    {
        simulation = std::make_unique<Seldon::Simulation<Seldon::ActivityDrivenModel::AgentT>>(
            simulation_options, network_file, agent_file );
    }
    else if( simulation_options.model == Seldon::Config::Model::ActivityDrivenInertial )
    {
        simulation = std::make_unique<Seldon::Simulation<Seldon::InertialModel::AgentT>>(
            simulation_options, network_file, agent_file );
    }
    else if( simulation_options.model == Seldon::Config::Model::DeffuantModel )
    {
        auto model_settings = std::get<Seldon::Config::DeffuantSettings>( simulation_options.model_settings );
        if( model_settings.use_binary_vector )
        {
            simulation = std::make_unique<Seldon::Simulation<Seldon::DeffuantModelVector::AgentT>>(
                simulation_options, network_file, agent_file );
        }
        else
        {
            simulation = std::make_unique<Seldon::Simulation<Seldon::DeffuantModel::AgentT>>(
                simulation_options, network_file, agent_file );
        }
    }
    else
    {
        throw std::runtime_error( "Model has not been created" );
    }

    simulation->run( output_dir_path );
    return;
}