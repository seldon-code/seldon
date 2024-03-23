#pragma once
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <cstddef>
#include <optional>
#include <random>
#include <string_view>
#include <variant>
#include <vector>

namespace Seldon::Config
{

/*
    As a convention: When a setting has type std::optional,
    nullopt signifies to not perform the operation associated
    with that setting. Also, the default value of any optional should
    be nullopt.
    If you feel tempted to make an optional setting with a different default,
    just make the setting not optional.
*/

enum class Model
{
    DeGroot,
    ActivityDrivenModel,
    InertialModel,
    DeffuantModel
};

struct OutputSettings
{
    // Write out the agents/network every n iterations, nullopt means never
    std::optional<size_t> n_output_agents  = std::nullopt;
    std::optional<size_t> n_output_network = std::nullopt;
    bool print_progress                    = false; // Print the iteration time, by default does not print
    bool output_initial                    = true;  // Output initial opinions and network, by default always outputs.
    size_t start_output = 1; // Start printing opinion and/or network files from this iteration number
};

struct DeGrootSettings
{
    std::optional<int> max_iterations = std::nullopt;
    double convergence_tol;
};

struct DeffuantSettings
{
    std::optional<int> max_iterations = std::nullopt;
    double homophily_threshold
        = 0.2;                      // d in the paper; agents interact if difference in opinion is less than this value
    double mu              = 0.5;   // convergence parameter; similar to social interaction strength K (0,0.5]
    bool use_network       = false; // For using a square lattice network
    bool use_binary_vector = false; // For the multi-dimensional DeffuantModelVector; by default set to false
    size_t dim
        = 1; // The size of the opinions vector. This is used for the multi-dimensional DeffuantModelVector model.
};

struct ActivityDrivenSettings
{
    std::optional<int> max_iterations = std::nullopt;
    double dt                         = 0.01;
    int m                             = 10;
    double eps                        = 0.01;
    double gamma                      = 2.1;
    double alpha                      = 3.0;
    double homophily                  = 0.5;
    double reciprocity                = 0.5;
    double K                          = 3.0;
    bool mean_activities              = false;
    bool mean_weights                 = false;
    size_t n_bots                     = 0; //@TODO why is this here?
    std::vector<int> bot_m            = std::vector<int>( 0 );
    std::vector<double> bot_activity  = std::vector<double>( 0 );
    std::vector<double> bot_opinion   = std::vector<double>( 0 );
    std::vector<double> bot_homophily = std::vector<double>( 0 );
    bool use_reluctances              = false;
    double reluctance_mean            = 1.0;
    double reluctance_sigma           = 0.25;
    double reluctance_eps             = 0.01;
    double covariance_factor          = 0.0;
};

struct InertialSettings : public ActivityDrivenSettings
{
    double friction_coefficient = 1.0;
};

struct InitialNetworkSettings
{
    std::optional<std::string> file;
    size_t n_agents      = 200;
    size_t n_connections = 10;
};

struct SimulationOptions
{
    using ModelVariantT = std::variant<DeGrootSettings, ActivityDrivenSettings, DeffuantSettings>;
    Model model;
    std::string model_string;
    int rng_seed = std::random_device()();
    OutputSettings output_settings;
    ModelVariantT model_settings;
    InitialNetworkSettings network_settings;
};

SimulationOptions parse_config_file( std::string_view config_file_path );
void validate_settings( const SimulationOptions & options );
void print_settings( const SimulationOptions & options );

} // namespace Seldon::Config