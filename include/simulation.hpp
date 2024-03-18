#pragma once

#include "config_parser.hpp"
#include "network.hpp"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <agent_generation.hpp>
#include <filesystem>
#include <memory>
#include <models/ActivityDrivenModel.hpp>
#include <models/DeGroot.hpp>
#include <network_generation.hpp>
#include <optional>
#include <string>
#include <util/io.hpp>
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

    Simulation(
        const Config::SimulationOptions & options, const std::optional<std::string> & cli_network_file,
        const std::optional<std::string> & cli_agent_file )
            : output_settings( options.output_settings )
    {
        // Initialize the rng
        gen = std::mt19937( options.rng_seed );

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

            //@TODO figure this out
            if( options.model == Config::Model::ActivityDrivenModel )
            {
                auto model_settings = std::get<Config::ActivityDrivenSettings>( options.model_settings );
                if( model_settings.mean_weights )
                {
                    network = NetworkGeneration::generate_fully_connected<AgentType>( n_agents );
                }
                else
                {
                    network
                        = NetworkGeneration::generate_n_connections<AgentType>( n_agents, n_connections, true, gen );
                }
            }
            else
            {
                network = NetworkGeneration::generate_n_connections<AgentType>( n_agents, n_connections, true, gen );
            }
        }

        if constexpr( std::is_same_v<AgentType, DeGrootModel::AgentT> )
        {
            auto degroot_settings = std::get<Config::DeGrootSettings>( options.model_settings );

            // DeGroot specific parameters
            model = [&]()
            {
                auto model             = std::make_unique<DeGrootModel>( network );
                model->max_iterations  = degroot_settings.max_iterations;
                model->convergence_tol = degroot_settings.convergence_tol;
                return model;
            }();

            if( cli_agent_file.has_value() )
            {
                network.agents = AgentGeneration::generate_from_file<DeGrootModel::AgentT>( cli_agent_file.value() );
            }
        }
        else if constexpr( std::is_same_v<AgentType, ActivityDrivenModel::AgentT> )
        {
            auto activitydriven_settings = std::get<Config::ActivityDrivenSettings>( options.model_settings );

            model = [&]()
            {
                auto model             = std::make_unique<ActivityDrivenModel>( network, gen );
                model->dt              = activitydriven_settings.dt;
                model->m               = activitydriven_settings.m;
                model->eps             = activitydriven_settings.eps;
                model->gamma           = activitydriven_settings.gamma;
                model->homophily       = activitydriven_settings.homophily;
                model->reciprocity     = activitydriven_settings.reciprocity;
                model->alpha           = activitydriven_settings.alpha;
                model->K               = activitydriven_settings.K;
                model->mean_activities = activitydriven_settings.mean_activities;
                model->mean_weights    = activitydriven_settings.mean_weights;
                model->max_iterations  = activitydriven_settings.max_iterations;
                // Reluctance
                model->use_reluctances  = activitydriven_settings.use_reluctances;
                model->reluctance_mean  = activitydriven_settings.reluctance_mean;
                model->reluctance_sigma = activitydriven_settings.reluctance_sigma;
                model->reluctance_eps   = activitydriven_settings.reluctance_eps;
                // bot
                model->n_bots        = activitydriven_settings.n_bots;
                model->bot_opinion   = activitydriven_settings.bot_opinion;
                model->bot_m         = activitydriven_settings.bot_m;
                model->bot_homophily = activitydriven_settings.bot_homophily;
                model->bot_activity  = activitydriven_settings.bot_activity;
                model->get_agents_from_power_law();
                return model;
            }();

            if( cli_agent_file.has_value() )
            {
                network.agents
                    = AgentGeneration::generate_from_file<ActivityDrivenModel::AgentT>( cli_agent_file.value() );
            }
        }
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
            Seldon::IO::network_to_file( network, ( output_dir_path / fs::path( "network_0.txt" ) ).string() );
            auto filename = fmt::format( "opinions_{}.txt", 0 );
            Seldon::IO::opinions_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
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
                Seldon::IO::opinions_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
            }

            // Write out the network?
            if( n_output_network.has_value() && ( this->model->n_iterations() >= start_output )
                && ( this->model->n_iterations() % n_output_network.value() == 0 ) )
            {
                auto filename = fmt::format( "network_{}.txt", this->model->n_iterations() );
                Seldon::IO::network_to_file( network, ( output_dir_path / fs::path( filename ) ).string() );
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
};

} // namespace Seldon