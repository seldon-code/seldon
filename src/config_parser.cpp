#include "config_parser.hpp"
#include "fmt/core.h"
#include "util/tomlplusplus.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

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
    else if( model_string == "Deffuant" )
    {
        return Model::DeffuantModel;
    }
    else if( model_string == "ActivityDrivenInertial" )
    {
        return Model::ActivityDrivenInertial;
    }
    throw std::runtime_error( fmt::format( "Invalid model string {}", model_string ) );
}

void set_if_specified( auto & opt, const auto & toml_opt )
{
    using T    = typename std::remove_reference<decltype( opt )>::type;
    auto t_opt = toml_opt.template value<T>();
    if( t_opt.has_value() )
        opt = t_opt.value();
}

// Convenience function to parse activity settings
void parse_activity_settings( auto & model_settings, const auto & toml_model_opt, const auto & tbl )
{
    set_if_specified( model_settings.dt, toml_model_opt["dt"] );
    set_if_specified( model_settings.m, toml_model_opt["m"] );
    set_if_specified( model_settings.eps, toml_model_opt["eps"] );
    set_if_specified( model_settings.gamma, toml_model_opt["gamma"] );
    set_if_specified( model_settings.homophily, toml_model_opt["homophily"] );
    set_if_specified( model_settings.reciprocity, toml_model_opt["reciprocity"] );
    set_if_specified( model_settings.alpha, toml_model_opt["alpha"] );
    set_if_specified( model_settings.K, toml_model_opt["K"] );
    // Mean activity model options
    set_if_specified( model_settings.mean_activities, toml_model_opt["mean_activities"] );
    set_if_specified( model_settings.mean_weights, toml_model_opt["mean_weights"] );
    // Reluctances
    set_if_specified( model_settings.covariance_factor, toml_model_opt["covariance_factor"] );
    set_if_specified( model_settings.use_reluctances, toml_model_opt["reluctances"] );
    set_if_specified( model_settings.reluctance_mean, toml_model_opt["reluctance_mean"] );
    set_if_specified( model_settings.reluctance_sigma, toml_model_opt["reluctance_sigma"] );
    set_if_specified( model_settings.reluctance_eps, toml_model_opt["reluctance_eps"] );

    model_settings.max_iterations = tbl["model"]["max_iterations"].template value<int>();

    // bot
    set_if_specified( model_settings.n_bots, toml_model_opt["n_bots"] );

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

    auto bot_opinion   = toml_model_opt["bot_opinion"];
    auto bot_m         = toml_model_opt["bot_m"];
    auto bot_activity  = toml_model_opt["bot_activity"];
    auto bot_homophily = toml_model_opt["bot_homophily"];

    push_back_bot_array( bot_m, model_settings.bot_m, model_settings.m );
    push_back_bot_array( bot_opinion, model_settings.bot_opinion, 0.0 );
    push_back_bot_array( bot_activity, model_settings.bot_activity, 1.0 );
    push_back_bot_array( bot_homophily, model_settings.bot_homophily, model_settings.homophily );
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
    set_if_specified( options.output_settings.print_progress, tbl["io"]["print_progress"] );
    set_if_specified( options.output_settings.output_initial, tbl["io"]["output_initial"] );
    set_if_specified( options.output_settings.start_output, tbl["io"]["start_output"] );
    set_if_specified( options.output_settings.start_numbering_from, tbl["io"]["start_numbering_from"] );

    // Check if the 'model' keyword exists
    std::optional<std::string> model_string = tbl["simulation"]["model"].value<std::string>();
    if( !model_string.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'simulation.model'!" ) );

    options.model_string = model_string.value();
    options.model        = model_string_to_enum( model_string.value() );

    if( options.model == Model::DeGroot )
    {
        auto model_settings           = DeGrootSettings();
        model_settings.max_iterations = tbl["model"]["max_iterations"].value<int>();
        set_if_specified( model_settings.convergence_tol, tbl[options.model_string]["convergence"] );
        options.model_settings = model_settings;
    }
    else if( options.model == Model::DeffuantModel )
    {
        auto model_settings           = DeffuantSettings();
        model_settings.max_iterations = tbl["model"]["max_iterations"].value<int>();
        set_if_specified( model_settings.homophily_threshold, tbl[options.model_string]["homophily_threshold"] );
        set_if_specified( model_settings.mu, tbl[options.model_string]["mu"] );
        set_if_specified( model_settings.use_network, tbl[options.model_string]["use_network"] );
        // Options for the DeffuantModelVector model
        set_if_specified( model_settings.use_binary_vector, tbl[options.model_string]["binary_vector"] );
        set_if_specified( model_settings.dim, tbl[options.model_string]["dim"] );
        options.model_settings = model_settings;
    }
    else if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = ActivityDrivenSettings();

        parse_activity_settings( model_settings, tbl[options.model_string], tbl );
        options.model_settings = model_settings;
    }
    else if( options.model == Model::ActivityDrivenInertial )
    {
        auto model_settings = ActivityDrivenInertialSettings();

        parse_activity_settings( model_settings, tbl[options.model_string], tbl );
        set_if_specified( model_settings.friction_coefficient, tbl[options.model_string]["friction_coefficient"] );
        options.model_settings = model_settings;
    }

    // Parse settings for the generation of the initial network
    options.network_settings = InitialNetworkSettings();
    set_if_specified( options.network_settings.n_agents, tbl["network"]["number_of_agents"] );
    set_if_specified( options.network_settings.n_connections, tbl["network"]["connections_per_agent"] );

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

    // @TODO: Check that start_output is less than the max_iterations?
    check( name_and_var( options.output_settings.start_output ), g_zero );
    check( name_and_var( options.output_settings.start_numbering_from ), geq_zero );

    auto validate_activity = [&]( const auto & model_settings )
    {
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
        check( name_and_var( model_settings.covariance_factor ), []( auto x ) { return x >= -1.0 && x <= 1.0; } );
        // Bot options
        size_t n_bots             = model_settings.n_bots;
        auto check_bot_size       = [&]( auto x ) { return x.size() >= n_bots; };
        const std::string bot_msg = "Length needs to be >= n_bots";
        check( name_and_var( model_settings.bot_m ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_activity ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_opinion ), check_bot_size, bot_msg );
        check( name_and_var( model_settings.bot_homophily ), check_bot_size, bot_msg );
    };

    if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = std::get<ActivityDrivenSettings>( options.model_settings );

        validate_activity( model_settings );
    }
    else if( options.model == Model::ActivityDrivenInertial )
    {
        auto model_settings = std::get<ActivityDrivenInertialSettings>( options.model_settings );
        check( name_and_var( model_settings.friction_coefficient ), geq_zero );
        validate_activity( model_settings );
    }
    else if( options.model == Model::DeGroot )
    {
        auto model_settings = std::get<DeGrootSettings>( options.model_settings );
        check( name_and_var( model_settings.convergence_tol ), geq_zero );
    }
    else if( options.model == Model::DeffuantModel )
    {
        auto model_settings = std::get<DeffuantSettings>( options.model_settings );
        check( name_and_var( model_settings.homophily_threshold ), g_zero );
        check( name_and_var( model_settings.mu ), []( auto x ) { return x >= 0 && x <= 1; } );
        // DeffuantModelVector settings
        check( name_and_var( model_settings.dim ), g_zero );
        // @TODO: maybe make this check nicer?
        if( !model_settings.use_binary_vector )
        {
            const std::string basic_deff_msg
                = "The basic Deffuant model has not been implemented with multiple dimensions";
            check( name_and_var( model_settings.dim ), []( auto x ) { return x == 1; }, basic_deff_msg );
        }
    }
}

void print_settings( const SimulationOptions & options )
{
    fmt::print( "Random seed: {}\n", options.rng_seed );

    fmt::print( "[Model]\n" );
    fmt::print( "    type {}\n", options.model_string );

    auto print_activity_settings = [&]( auto model_settings )
    {
        fmt::print( "    max_iterations {}\n", model_settings.max_iterations );
        fmt::print( "    dt {} \n", model_settings.dt );
        fmt::print( "    m {} \n", model_settings.m );
        fmt::print( "    eps {} \n", model_settings.eps );
        fmt::print( "    gamma {} \n", model_settings.gamma );
        fmt::print( "    alpha {} \n", model_settings.alpha );
        fmt::print( "    homophily {} \n", model_settings.homophily );
        fmt::print( "    reciprocity {} \n", model_settings.reciprocity );
        fmt::print( "    K {} \n", model_settings.K );
        fmt::print( "    mean_activities {} \n", model_settings.mean_activities );
        fmt::print( "    mean_weights {} \n", model_settings.mean_weights );
        fmt::print( "    n_bots           {}\n", model_settings.n_bots );
        if( model_settings.n_bots > 0 )
        {
            fmt::print( "    Bot opinions     {}\n", model_settings.bot_opinion );
            fmt::print( "    Bot m            {}\n", model_settings.bot_m );
            fmt::print( "    Bot activity(s)  {}\n", model_settings.bot_activity );
            fmt::print( "    Bot homophily(s) {}\n", model_settings.bot_homophily );
        }

        fmt::print( "    use_reluctances {}\n", model_settings.use_reluctances );
        if( model_settings.use_reluctances )
        {
            fmt::print( "    reluctance_mean {}\n", model_settings.reluctance_mean );
            fmt::print( "    reluctance_sigma {}\n", model_settings.reluctance_sigma );
            fmt::print( "    reluctance_eps {}\n", model_settings.reluctance_eps );
            fmt::print( "    covariance_factor {}\n", model_settings.covariance_factor );
        }
    };

    // @TODO: Optionally print *all* settings to the console, including defaults that were set
    if( options.model == Model::ActivityDrivenModel )
    {
        auto model_settings = std::get<ActivityDrivenSettings>( options.model_settings );

        print_activity_settings( model_settings );
    }
    else if( options.model == Model::ActivityDrivenInertial )
    {
        auto model_settings = std::get<ActivityDrivenInertialSettings>( options.model_settings );
        print_activity_settings( model_settings );
        fmt::print( "    friction_coefficient {}\n", model_settings.friction_coefficient );
    }
    else if( options.model == Model::DeGroot )
    {
        auto model_settings = std::get<DeGrootSettings>( options.model_settings );
        fmt::print( "    max_iterations {}\n", model_settings.max_iterations );
        fmt::print( "    convergence_tol {}\n", model_settings.convergence_tol );
    }
    else if( options.model == Model::DeffuantModel )
    {
        auto model_settings = std::get<DeffuantSettings>( options.model_settings );
        fmt::print( "    max_iterations {}\n", model_settings.max_iterations );
        fmt::print( "    homophily_threshold {}\n", model_settings.homophily_threshold );
        fmt::print( "    mu {}\n", model_settings.mu );
        fmt::print( "    use_network {}\n", model_settings.use_network );
        fmt::print( "    use_binary_vector {}\n", model_settings.use_binary_vector );
        fmt::print( "    dim {}\n", model_settings.dim );
    }

    fmt::print( "[Network]\n" );
    fmt::print( "    n_agents {}\n", options.network_settings.n_agents );
    fmt::print( "    n_connections {}\n", options.network_settings.n_connections );

    fmt::print( "[Output]\n" );
    fmt::print( "    n_output_agents  {}\n", options.output_settings.n_output_agents );
    fmt::print( "    n_output_network {}\n", options.output_settings.n_output_network );
    fmt::print( "    print_progress {}\n", options.output_settings.print_progress );
    fmt::print( "    output_initial {}\n", options.output_settings.output_initial );
    fmt::print( "    start_output {}\n", options.output_settings.start_output );
    fmt::print( "    start_numbering_from {}\n", options.output_settings.start_numbering_from );
}

} // namespace Seldon::Config