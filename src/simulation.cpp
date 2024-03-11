#include "simulation.hpp"
#include "config_parser.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "models/DeGroot.hpp"
#include "network_generation.hpp"
#include "util/tomlplusplus.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <cstddef>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>

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
        network = generate_from_file( file.value() );
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
                network = generate_fully_connected( n_agents );
            }
            else
            {
                network = generate_n_connections( n_agents, n_connections, true, gen );
            }
        }
        else
        {
            network = generate_n_connections( n_agents, n_connections, true, gen );
        }
    }

    if( options.model == Config::Model::DeGroot )
    {
        auto degroot_settings = std::get<Config::DeGrootSettings>( options.model_settings );

        // DeGroot specific parameters
        auto model_DeGroot             = std::make_unique<DeGrootModel>( network->n_agents(), *network );
        model_DeGroot->max_iterations  = degroot_settings.max_iterations;
        model_DeGroot->convergence_tol = degroot_settings.convergence_tol;
        model                          = std::move( model_DeGroot );
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
        model = std::move( model_activityDriven );
    }

    if( cli_agent_file.has_value() )
    {
        model->agents_from_file( cli_agent_file.value() );
    }
}