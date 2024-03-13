#include "simulation.hpp"
#include "agent_generation.hpp"
#include "config_parser.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "models/DeGroot.hpp"
#include "network_generation.hpp"
#include "util/tomlplusplus.hpp"
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <util/io.hpp>

Seldon::Simulation::Simulation(
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
        network = NetworkGeneration::generate_from_file( file.value() );
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
                network = NetworkGeneration::generate_fully_connected( n_agents );
            }
            else
            {
                network = NetworkGeneration::generate_n_connections( n_agents, n_connections, true, gen );
            }
        }
        else
        {
            network = NetworkGeneration::generate_n_connections( n_agents, n_connections, true, gen );
        }
    }

    if( options.model == Config::Model::DeGroot )
    {
        auto degroot_settings = std::get<Config::DeGrootSettings>( options.model_settings );

        // DeGroot specific parameters
        auto model_DeGroot             = std::make_unique<DeGrootModel>( network->n_agents(), *network );
        model_DeGroot->max_iterations  = degroot_settings.max_iterations;
        model_DeGroot->convergence_tol = degroot_settings.convergence_tol;

        if( cli_agent_file.has_value() )
        {
            model_DeGroot->agents = Agents::generate_from_file<DeGrootModel::AgentT>( cli_agent_file.value() );
        }

        model = std::move( model_DeGroot );
    }
    else if( options.model == Config::Model::ActivityDrivenModel )
    {
        auto activitydriven_settings = std::get<Config::ActivityDrivenSettings>( options.model_settings );

        auto model_activityDriven         = std::make_unique<ActivityAgentModel>( network->n_agents(), *network, gen );
        model_activityDriven->dt          = activitydriven_settings.dt;
        model_activityDriven->m           = activitydriven_settings.m;
        model_activityDriven->eps         = activitydriven_settings.eps;
        model_activityDriven->gamma       = activitydriven_settings.gamma;
        model_activityDriven->homophily   = activitydriven_settings.homophily;
        model_activityDriven->reciprocity = activitydriven_settings.reciprocity;
        model_activityDriven->alpha       = activitydriven_settings.alpha;
        model_activityDriven->K           = activitydriven_settings.K;
        model_activityDriven->mean_activities = activitydriven_settings.mean_activities;
        model_activityDriven->mean_weights    = activitydriven_settings.mean_weights;

        model_activityDriven->max_iterations = activitydriven_settings.max_iterations;

        // bot
        model_activityDriven->n_bots        = activitydriven_settings.n_bots;
        model_activityDriven->bot_opinion   = activitydriven_settings.bot_opinion;
        model_activityDriven->bot_m         = activitydriven_settings.bot_m;
        model_activityDriven->bot_homophily = activitydriven_settings.bot_homophily;
        model_activityDriven->bot_activity  = activitydriven_settings.bot_activity;

        model_activityDriven->get_agents_from_power_law();

        if( cli_agent_file.has_value() )
        {
            model_activityDriven->agents
                = Agents::generate_from_file<ActivityAgentModel::AgentT>( cli_agent_file.value() );
        }

        model = std::move( model_activityDriven );
    }
}

void Seldon::Simulation::run( fs::path output_dir_path )
{
    auto n_output_agents  = this->output_settings.n_output_agents;
    auto n_output_network = this->output_settings.n_output_network;

    fmt::print( "-----------------------------------------------------------------\n" );
    fmt::print( "Starting simulation\n" );
    fmt::print( "-----------------------------------------------------------------\n" );

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
                "Iteration {}   iter_time = {:%Hh %Mm %Ss} \n", this->model->n_iterations,
                std::chrono::floor<ms>( iter_time ) );
        }

        // Write out the opinion?
        if( n_output_agents.has_value() && ( this->model->n_iterations % n_output_agents.value() == 0 ) )
        {
            auto filename = fmt::format( "opinions_{}.txt", this->model->n_iterations );
            Seldon::IO::opinions_to_file( *this, ( output_dir_path / fs::path( filename ) ).string() );
        }

        // Write out the network?
        if( n_output_network.has_value() && ( this->model->n_iterations % n_output_network.value() == 0 ) )
        {
            auto filename = fmt::format( "network_{}.txt", this->model->n_iterations );
            Seldon::IO::network_to_file( *this, ( output_dir_path / fs::path( filename ) ).string() );
        }
    }

    auto t_simulation_end = std::chrono::high_resolution_clock::now();
    auto total_time       = std::chrono::duration_cast<ms>( t_simulation_end - t_simulation_start );

    fmt::print( "-----------------------------------------------------------------\n" );
    fmt::print(
        "Finished after {} iterations, total time = {:%Hh %Mm %Ss}\n", this->model->n_iterations,
        std::chrono::floor<ms>( total_time ) );
    fmt::print( "=================================================================\n" );
}