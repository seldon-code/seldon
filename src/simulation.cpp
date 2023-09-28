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

    // Check if the 'model' keyword exists
    std::optional<std::string> model_opt = tbl["model"].value<std::string>();
    if( !model_opt.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'model'!" ) );

    // Check if 'model' is one of the allowed values
    auto model_string = model_opt.value();
    if( !allowed_models.count( model_string ) )
    {
        throw std::runtime_error( fmt::format( "Unknown model type: '{}'!", model_string ) );
    }

    // Construct the network
    n_agents          = tbl["network"]["number_of_agents"].value_or( 0 );
    int n_connections = tbl["network"]["connections_per_agent"].value_or( 0 );
    network           = Network( n_agents, n_connections );

    // Construct the model object
    ModelType model_type;
    if( model_string == "DeGroot" )
    {
        // int n_agents = tbl["DeGroot"]["number_of_agents"].value_or( 0 );
        this->model = std::make_unique<DeGrootModel>( n_agents, network );
        model_type  = ModelType::DeGroot;
    }
}