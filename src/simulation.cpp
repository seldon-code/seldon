#include "simulation.hpp"
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
    const std::string & config_file, const std::optional<std::string> & cli_network_file,
    const std::optional<std::string> & cli_agent_file )
{
    std::set<std::string> allowed_models = { "DeGroot", "ActivityDriven" };

    toml::table tbl;
    tbl = toml::parse_file( config_file );

    // Initialize the rng
    std::optional<int> rng_seed = tbl["simulation"]["rng_seed"].value<int>();
    if( rng_seed.has_value() )
    {
        fmt::print( "WARNING: Seeding random number generator with seed {}!\n", rng_seed.value() );
    }
    else
    {
        rng_seed = std::random_device()();
        fmt::print( "INFO: Seeding with seed {}!\n", rng_seed.value() );
    }
    gen = std::mt19937( rng_seed.value() );

    // Parse output settings
    auto n_output_network = tbl["io"]["n_output_network"].value<size_t>();
    if( n_output_network.has_value() )
    {
        output_settings.n_output_network = n_output_network.value();
    }

    auto n_output_agents = tbl["io"]["n_output_agents"].value<size_t>();
    if( n_output_agents.has_value() )
    {
        output_settings.n_output_agents = n_output_agents.value();
    }

    auto print_progress = tbl["io"]["print_progress"].value<bool>();
    if( print_progress.has_value() )
    {
        output_settings.print_progress = print_progress.value();
    }

    // Check if the 'model' keyword exists
    std::optional<std::string> model_opt = tbl["simulation"]["model"].value<std::string>();
    if( !model_opt.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'simulation.model'!" ) );

    // Check if 'model' is one of the allowed values
    auto model_string = model_opt.value();
    if( !allowed_models.contains( model_string ) )
    {
        throw std::runtime_error( fmt::format( "Unknown model type: '{}'!", model_string ) );
    }

    fmt::print( "Model type: {}\n", model_string );

    // Construct the network
    std::optional<std::string> file = cli_network_file;
    if( !file.has_value() ) // Check if toml file should be superceded by cli_network_file
        file = tbl["network"]["file"].value<std::string>();

    if( file.has_value() )
    {
        fmt::print( "Reading netwok from file {}\n", file.value() );
        network = generate_from_file( file.value() );
    }
    else
    {
        fmt::print( "Generating network\n" );
        n_agents          = tbl["network"]["number_of_agents"].value_or( 0 );
        int n_connections = tbl["network"]["connections_per_agent"].value_or( 0 );
        if( model_string == "ActivityDriven" && tbl["ActivityDriven"]["mean_weights"].value_or( false ) )
        {
            // Generate a fully-connected network with weights set to the default 0.0
            network = generate_fully_connected( n_agents );
        }
        else
        {
            network = generate_n_connections( n_agents, n_connections, true, gen );
        }
    }

    n_agents = network->n_agents();
    fmt::print( "Network has {} agents\n", n_agents );

    // Construct the model object
    // Generic model parameters
    std::optional<int> max_iterations = tbl["model"]["max_iterations"].value<int>();

    if( model_string == "DeGroot" )
    {
        // DeGroot specific parameters
        double convergence = tbl["DeGroot"]["convergence"].value_or( 1e-6 );

        auto model_DeGroot             = std::make_unique<DeGrootModel>( n_agents, *network );
        model_DeGroot->max_iterations  = max_iterations;
        model_DeGroot->convergence_tol = convergence;
        model                          = std::move( model_DeGroot );
    }
    else if( model_string == "ActivityDriven" )
    {
        auto model_activityDriven             = std::make_unique<ActivityAgentModel>( n_agents, *network, gen );
        model_activityDriven->dt              = tbl["ActivityDriven"]["dt"].value_or<double>( 0.01 );
        model_activityDriven->m               = tbl["ActivityDriven"]["m"].value_or<size_t>( 10 );
        model_activityDriven->eps             = tbl["ActivityDriven"]["eps"].value_or<double>( 0.01 );
        model_activityDriven->gamma           = tbl["ActivityDriven"]["gamma"].value_or<double>( 2.1 );
        model_activityDriven->homophily       = tbl["ActivityDriven"]["homophily"].value_or<double>( 0.5 );
        model_activityDriven->reciprocity     = tbl["ActivityDriven"]["reciprocity"].value_or<double>( 0.5 );
        model_activityDriven->alpha           = tbl["ActivityDriven"]["alpha"].value_or<double>( 3.0 );
        model_activityDriven->K               = tbl["ActivityDriven"]["K"].value_or<double>( 3.0 );
        model_activityDriven->mean_activities = tbl["ActivityDriven"]["mean_activities"].value_or<bool>( false );
        model_activityDriven->mean_weights    = tbl["ActivityDriven"]["mean_weights"].value_or<bool>( false );

        model_activityDriven->max_iterations = max_iterations;

        // bot
        model_activityDriven->n_bots = tbl["ActivityDriven"]["n_bots"].value_or<size_t>( 0 );
        if( model_activityDriven->bot_present() )
        {
            fmt::print( "Using {} bots\n", model_activityDriven->n_bots );

            auto bot_opinion   = tbl["ActivityDriven"]["bot_opinion"];
            auto bot_m         = tbl["ActivityDriven"]["bot_m"];
            auto bot_activity  = tbl["ActivityDriven"]["bot_activity"];
            auto bot_homophily = tbl["ActivityDriven"]["bot_homophily"];

            auto n_bots = model_activityDriven->n_bots;

            if(
                // clang-format off
                   n_bots > bot_opinion.as_array()->size() 
                || n_bots > bot_m.as_array()->size()
                || n_bots > bot_homophily.as_array()->size() 
                || n_bots > bot_activity.as_array()->size()
                // clang-format on
            )
            {
                throw std::runtime_error( "One of the bot parameter arrays is smaller than n_bots" );
            }

            for( size_t i = 0; i < model_activityDriven->n_bots; i++ )
            {
                model_activityDriven->bot_opinion.push_back( bot_opinion[i].value_or<double>( 0.0 ) );
                model_activityDriven->bot_m.push_back( bot_m[i].value_or<size_t>( size_t( model_activityDriven->m ) ) );
                model_activityDriven->bot_activity.push_back( bot_activity[i].value_or<double>( 0.0 ) );
                model_activityDriven->bot_homophily.push_back(
                    bot_homophily[i].value_or<double>( double( model_activityDriven->homophily ) ) );
            }

            fmt::print( "Bot opinions {}\n", model_activityDriven->bot_opinion );
            fmt::print( "Bot m {}\n", model_activityDriven->bot_m );
            fmt::print( "Bot activity(s) {}\n", model_activityDriven->bot_activity );
            fmt::print( "Bot homophily(s) {}\n", model_activityDriven->bot_homophily );
        }

        model_activityDriven->get_agents_from_power_law();
        model = std::move( model_activityDriven );

        fmt::print( "Finished model setup\n" );
    }

    if( cli_agent_file.has_value() )
    {
        fmt::print( "Reading agents from file {}\n", cli_agent_file.value() );
        model->agents_from_file( cli_agent_file.value() );
    }
}