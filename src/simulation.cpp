#include "simulation.hpp"
#include "models/DeGroot.hpp"
#include "util/tomlplusplus.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <optional>
#include <set>

enum class ModelType : unsigned int
{
    DeGroot
};

Seldon::Simulation::Simulation( std::string config_file )
{
    std::set<std::string> allowed_models = { "DeGroot" };

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

    // Check if the 'model' keyword exists
    std::optional<std::string> model_opt = tbl["simulation"]["model"].value<std::string>();
    if( !model_opt.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'simulation.model'!" ) );

    // Check if 'model' is one of the allowed values
    auto model_string = model_opt.value();
    if( !allowed_models.count( model_string ) )
    {
        throw std::runtime_error( fmt::format( "Unknown model type: '{}'!", model_string ) );
    }

    // Construct the network
    n_agents          = tbl["network"]["number_of_agents"].value_or( 0 );
    int n_connections = tbl["network"]["connections_per_agent"].value_or( 0 );
    network           = std::make_unique<Network>( n_agents, n_connections, gen );

    // Construct the model object
    // Generic model parameters
    std::optional<int> max_iterations = tbl["model"]["max_iterations"].value<int>();

    ModelType model_type;
    if( model_string == "DeGroot" )
    {
        // DeGroot specific parameters
        double convergence = tbl["DeGroot"]["convergence"].value_or( 1e-6 );

        auto model_DeGroot             = std::make_unique<DeGrootModel>( n_agents, *network );
        model_DeGroot->max_iterations  = max_iterations;
        model_DeGroot->convergence_tol = convergence;

        model      = std::move( model_DeGroot );
        model_type = ModelType::DeGroot;
    }
}