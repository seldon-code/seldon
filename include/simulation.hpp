#pragma once

#include "config_parser.hpp"
#include "fmt/core.h"
#include "model_factory.hpp"
#include "network.hpp"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <filesystem>
#include <memory>
#include <models/ActivityDrivenModel.hpp>
#include <models/DeGroot.hpp>
#include <models/DeffuantModel.hpp>
#include <network_generation.hpp>
#include <network_io.hpp>
#include <optional>
#include <string>
namespace fs = std::filesystem;

namespace Seldon
{

class SimulationInterface
{
public:
    virtual void run( const fs::path & output_dir_path ) = 0;
    virtual ~SimulationInterface()                       = default;
};

template<typename AgentType>
class Simulation : public SimulationInterface
{

private:
    std::mt19937 gen;

public:
    std::unique_ptr<Model<AgentType>> model;
    Network<AgentType> network;

    Config::OutputSettings output_settings;

    void
    create_network( const Config::SimulationOptions & options, const std::optional<std::string> & cli_network_file )
    {
        // Construct the network
        std::optional<std::string> file = cli_network_file;
        if( !file.has_value() ) // Check if toml file should be superceded by cli_network_file
            file = options.network_settings.file;

        if( file.has_value() )
        {
            network = NetworkGeneration::generate_from_file<AgentType>( file.value() );
        }
        else
        {
            int n_agents       = options.network_settings.n_agents;
            auto n_connections = options.network_settings.n_connections;
            network = NetworkGeneration::generate_n_connections<AgentType>( n_agents, n_connections, true, gen );
        }
    }

    void create_model( const Config::SimulationOptions & options, const std::optional<std::string> & cli_agent_file )
    {
        if( options.model == Config::Model::DeGroot )
        {
            model = ModelFactory::create_model_degroot( network, options.model_settings );
        }
        else if( options.model == Config::Model::ActivityDrivenModel )
        {
            model = ModelFactory::create_model_activity_driven( network, options.model_settings, gen );
        }
        else if( options.model == Config::Model::ActivityDrivenInertial )
        {
            model = ModelFactory::create_model_activity_driven_inertial( network, options.model_settings, gen );
        }
        else if( options.model == Config::Model::DeffuantModel )
        {
            auto deffuant_settings = std::get<Config::DeffuantSettings>( options.model_settings );
            if( deffuant_settings.use_binary_vector )
            {
                model = ModelFactory::create_model_deffuant_vector( network, options.model_settings, gen );
            }
            else
            {
                model = ModelFactory::create_model_deffuant( network, options.model_settings, gen );
            }
        }

        if( cli_agent_file.has_value() )
        {
            network.agents = agents_from_file<AgentType>( cli_agent_file.value() );
        }
    }

    Simulation(
        const Config::SimulationOptions & options, const std::optional<std::string> & cli_network_file,
        const std::optional<std::string> & cli_agent_file )
            : output_settings( options.output_settings )
    {
        // Initialize the rng
        gen = std::mt19937( options.rng_seed );

        create_network( options, cli_network_file );
        create_model( options, cli_agent_file );
    }

    void run( const fs::path & output_dir_path ) override
    {
        auto n_output_agents  = this->output_settings.n_output_agents;
        auto n_output_network = this->output_settings.n_output_network;
        auto start_output     = this->output_settings.start_output;
        auto output_initial   = this->output_settings.output_initial;

        fmt::print( "-----------------------------------------------------------------\n" );
        fmt::print( "Starting simulation\n" );
        fmt::print( "-----------------------------------------------------------------\n" );

        if( output_initial )
        {
            Seldon::network_to_file( network, ( output_dir_path / fs::path( "network_0.txt" ) ).string() );
            auto filename = fmt::format( "opinions_{}.txt", 0 );
            Seldon::agents_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
        }
        this->model->initialize_iterations();

        typedef std::chrono::milliseconds ms;
        auto t_simulation_start = std::chrono::high_resolution_clock::now();
        while( !this->model->finished() )
        {
            auto t_iter_start = std::chrono::high_resolution_clock::now();

            this->model->iteration();

            auto t_iter_end = std::chrono::high_resolution_clock::now();
            auto iter_time  = std::chrono::duration_cast<ms>( t_iter_end - t_iter_start );

            // Print the iteration time?
            if( this->output_settings.print_progress )
            {
                fmt::print(
                    "Iteration {}   iter_time = {:%Hh %Mm %Ss} \n", this->model->n_iterations(),
                    std::chrono::floor<ms>( iter_time ) );
            }

            // Write out the opinion?
            if( n_output_agents.has_value() && ( this->model->n_iterations() >= start_output )
                && ( this->model->n_iterations() % n_output_agents.value() == 0 ) )
            {
                auto filename = fmt::format( "opinions_{}.txt", this->model->n_iterations() );
                Seldon::agents_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
            }

            // Write out the network?
            if( n_output_network.has_value() && ( this->model->n_iterations() >= start_output )
                && ( this->model->n_iterations() % n_output_network.value() == 0 ) )
            {
                auto filename = fmt::format( "network_{}.txt", this->model->n_iterations() );
                Seldon::network_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
            }
        }

        auto t_simulation_end = std::chrono::high_resolution_clock::now();
        auto total_time       = std::chrono::duration_cast<ms>( t_simulation_end - t_simulation_start );

        fmt::print( "-----------------------------------------------------------------\n" );
        fmt::print(
            "Finished after {} iterations, total time = {:%Hh %Mm %Ss}\n", this->model->n_iterations(),
            std::chrono::floor<ms>( total_time ) );
        fmt::print( "=================================================================\n" );
    }
}; // namespace Seldon

} // namespace Seldon