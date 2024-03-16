#include "config_parser.hpp"
#include "fmt/core.h"
#include "util/tomlplusplus.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <optional>
#include <stdexcept>
#include <string>
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

    options.rng_seed = tbl["simulation"]["rng_seed"].value_or( int( options.rng_seed ) );

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
        // Reluctances
        model_settings.use_reluctances   = tbl["ActivityDriven"]["reluctances"].value_or<bool>( false );
        model_settings.reluctance_mean   = tbl["ActivityDriven"]["reluctance_mean"].value_or<double>( 1.0 );
        model_settings.reluctance_sigma  = tbl["ActivityDriven"]["reluctance_sigma"].value_or<double>( 0.25 );
        model_settings.reluctance_eps    = tbl["ActivityDriven"]["reluctance_eps"].value_or<double>( 0.01 );
        model_settings.covariance_factor = tbl["ActivityDriven"]["covariance_factor"].value_or<double>( 0.0 );

        model_settings.max_iterations = tbl["model"]["max_iterations"].value<int>();

        // bot
        model_settings.n_bots = tbl["ActivityDriven"]["n_bots"].value_or<size_t>( 0 );

        auto push_back_bot_array = [&]( auto toml_node, auto & options_array, auto default_value )
        {
            if( toml_node.is_array() )
            {
                toml::array * toml_arr = toml_node.as_array();

                toml_arr->for_each(
                    [&]( auto && elem )
                    {
                        if( elem.is_integer() )
                        {
                            options_array.push_back( elem.as_integer()->get() );
                        }
                        else if( elem.is_floating_point() )
                        {
                            options_array.push_back( elem.as_floating_point()->get() );
                        }
                    } );
            }
            else
            {
                options_array = std::vector<decltype( default_value )>( model_settings.n_bots, default_value );
            }
        };

        auto bot_opinion   = tbl["ActivityDriven"]["bot_opinion"];
        auto bot_m         = tbl["ActivityDriven"]["bot_m"];
        auto bot_activity  = tbl["ActivityDriven"]["bot_activity"];
        auto bot_homophily = tbl["ActivityDriven"]["bot_homophily"];

        push_back_bot_array( bot_m, model_settings.bot_m, model_settings.m );
        push_back_bot_array( bot_opinion, model_settings.bot_opinion, 0.0 );
        push_back_bot_array( bot_activity, model_settings.bot_activity, 1.0 );
        push_back_bot_array( bot_homophily, model_settings.bot_homophily, model_settings.homophily );

        options.model_settings = model_settings;
    }

    // Parse settings for the generation of the initial network
    options.network_settings               = InitialNetworkSettings();
    options.network_settings.n_agents      = tbl["network"]["number_of_agents"].value_or( 0 );
    options.network_settings.n_connections = tbl["network"]["connections_per_agent"].value_or( 0 );

    return options;
}

// This macro expands the variable x to: "x", x and is used together with the check function
#define name_and_var( x ) #x, x

// Helper function to check variables, depending on a condition function with an optional explanation
void check(
    const std::string & variable_name, auto variable, auto condition,
    const std::optional<std::string> & explanation = std::nullopt )
{
    if( !condition( variable ) )
    {
        std::string msg = fmt::format( "The value {} is not valid for {}", variable, variable_name );
        if( explanation.has_value() )
        {
            msg += "\n";
            msg += explanation.value();
        }
        throw std::runtime_error( msg );
    }
}

void validate_settings( const SimulationOptions & options )
{
    auto g_zero   = []( auto x ) { return x > 0; };
    auto geq_zero = []( auto x ) { return x >= 0; };

    if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = std::get<ActivityDrivenSettings>( options.model_settings );

        check( name_and_var( model_settings.dt ), g_zero );
        check( name_and_var( model_settings.m ), geq_zero );
        check( name_and_var( model_settings.eps ), g_zero );
        check( name_and_var( model_settings.gamma ), []( auto x ) { return x > 2.0; } );
        check( name_and_var( model_settings.alpha ), geq_zero );
        // check( name_and_var( model_settings.homophily ), geq_zero );
        check( name_and_var( model_settings.reciprocity ), geq_zero );
        // Reluctance options
        check( name_and_var( model_settings.reluctance_mean ), g_zero );
        check( name_and_var( model_settings.reluctance_sigma ), g_zero );
        check( name_and_var( model_settings.reluctance_eps ), g_zero );
        check( name_and_var( model_settings.covariance_factor ), geq_zero );
        // Bot options
        size_t n_bots             = model_settings.n_bots;
        auto check_bot_size       = [&]( auto x ) { return x.size() >= n_bots; };
        const std::string bot_msg = "Length needs to be >= n_bots";
        check( name_and_var( model_settings.bot_m ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_activity ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_opinion ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_homophily ), check_bot_size, bot_msg );
    }
    else if( options.model == Model::DeGroot )
    {
        auto model_settings = std::get<DeGrootSettings>( options.model_settings );
        check( name_and_var( model_settings.convergence_tol ), geq_zero );
    }
}

void print_settings( const SimulationOptions & options )
{
    fmt::print( "INFO: Seeding with seed {}\n", options.rng_seed );
    fmt::print( "Model type: {}\n", options.model_string );
    fmt::print( "Network has {} agents\n", options.network_settings.n_agents );
    // @TODO: Optionally print *all* settings to the console, including defaults that were set
    if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = std::get<ActivityDrivenSettings>( options.model_settings );
        if( model_settings.n_bots > 0 )
        {
            fmt::print( "n_bots           {}\n", model_settings.n_bots );
            fmt::print( "Bot opinions     {}\n", model_settings.bot_opinion );
            fmt::print( "Bot m            {}\n", model_settings.bot_m );
            fmt::print( "Bot activity(s)  {}\n", model_settings.bot_activity );
            fmt::print( "Bot homophily(s) {}\n", model_settings.bot_homophily );
        }
    }
}

} // namespace Seldon::Config