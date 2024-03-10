#include "config_parser.hpp"
#include "util/tomlplusplus.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <stdexcept>
#include <string_view>

namespace Seldon::Config
{

Model model_string_to_enum( std::string_view model_string )
{
    if( model_string == "DeGroot" )
    {
        return Model::DeGroot;
    }
    else if( model_string == "ActivityDriven" )
    {
        return Model::ActivityDrivenModel;
    }
    throw std::runtime_error( fmt::format( "Invalid model string {}", model_string ) );
}

SimulationOptions parse_config_file( std::string_view config_file_path )
{
    SimulationOptions options;

    toml::table tbl;
    tbl = toml::parse_file( config_file_path );

    options.rng_seed = tbl["simulation"]["rng_seed"].value<int>();

    // Parse output settings
    options.output_settings.n_output_network = tbl["io"]["n_output_network"].value<size_t>();
    options.output_settings.n_output_agents  = tbl["io"]["n_output_agents"].value<size_t>();
    options.output_settings.print_progress
        = tbl["io"]["print_progress"].value_or<bool>( bool( options.output_settings.print_progress ) );

    // Check if the 'model' keyword exists
    std::optional<std::string> model_string = tbl["simulation"]["model"].value<std::string>();
    if( !model_string.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'simulation.model'!" ) );

    options.model_string = model_string.value();
    options.model        = model_string_to_enum( model_string.value() );

    if( options.model == Model::DeGroot )
    {
        auto model_settings            = DeGrootSettings();
        model_settings.max_iterations  = tbl["model"]["max_iterations"].value<int>();
        model_settings.convergence_tol = tbl["DeGroot"]["convergence"].value_or( model_settings.convergence_tol );
        options.model_settings         = model_settings;
    }
    else if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = ActivityDrivenSettings();

        //@TODO: the following should not hardcode the defaults, take from the settings class
        model_settings.dt              = tbl["ActivityDriven"]["dt"].value_or<double>( 0.01 );
        model_settings.m               = tbl["ActivityDriven"]["m"].value_or<size_t>( 10 );
        model_settings.eps             = tbl["ActivityDriven"]["eps"].value_or<double>( 0.01 );
        model_settings.gamma           = tbl["ActivityDriven"]["gamma"].value_or<double>( 2.1 );
        model_settings.homophily       = tbl["ActivityDriven"]["homophily"].value_or<double>( 0.5 );
        model_settings.reciprocity     = tbl["ActivityDriven"]["reciprocity"].value_or<double>( 0.5 );
        model_settings.alpha           = tbl["ActivityDriven"]["alpha"].value_or<double>( 3.0 );
        model_settings.K               = tbl["ActivityDriven"]["K"].value_or<double>( 3.0 );
        model_settings.mean_activities = tbl["ActivityDriven"]["mean_activities"].value_or<bool>( false );
        model_settings.mean_weights    = tbl["ActivityDriven"]["mean_weights"].value_or<bool>( false );

        model_settings.max_iterations = tbl["model"]["max_iterations"].value<int>();

        // bot
        model_settings.n_bots = tbl["ActivityDriven"]["n_bots"].value_or<size_t>( 0 );

        auto bot_opinion   = tbl["ActivityDriven"]["bot_opinion"];
        auto bot_m         = tbl["ActivityDriven"]["bot_m"];
        auto bot_activity  = tbl["ActivityDriven"]["bot_activity"];
        auto bot_homophily = tbl["ActivityDriven"]["bot_homophily"];
        for( size_t i = 0; i < model_settings.n_bots; i++ )
        {
            model_settings.bot_opinion.push_back( bot_opinion[i].value_or<double>( 0.0 ) );
            model_settings.bot_m.push_back( bot_m[i].value_or<size_t>( size_t( model_settings.m ) ) );
            model_settings.bot_activity.push_back( bot_activity[i].value_or<double>( 0.0 ) );
            model_settings.bot_homophily.push_back(
                bot_homophily[i].value_or<double>( double( model_settings.homophily ) ) );
        }
        options.model_settings = model_settings;
    }

    // Parse settings for the generation of the initial network
    options.network_settings               = InitialNetworkSettings();
    options.network_settings.n_agents      = tbl["network"]["number_of_agents"].value_or( 0 );
    options.network_settings.n_connections = tbl["network"]["connections_per_agent"].value_or( 0 );

    return options;
}

} // namespace Seldon::Config