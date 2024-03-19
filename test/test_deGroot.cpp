#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "config_parser.hpp"
#include "models/DeGroot.hpp"
#include "network.hpp"
#include <random>

TEST_CASE( "Test the DeGroot Model Symmetric", "[DeGroot]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;
    using Network = Network<DeGrootModel::AgentT>;

    size_t n_agents     = 2;
    auto neighbour_list = std::vector<std::vector<size_t>>{
        { 1, 0 },
        { 0, 1 },
    };

    auto weight_list = std::vector<std::vector<double>>{
        { 0.2, 0.8 },
        { 0.2, 0.8 },
    };

    auto network = Network( std::move( neighbour_list ), std::move( weight_list ), Network::EdgeDirection::Incoming );
    auto model   = DeGrootModel( network );

    model.convergence_tol          = 1e-6;
    model.max_iterations           = 100;
    network.agents[0].data.opinion = 0.0;
    network.agents[1].data.opinion = 1.0;

    while( !model.finished() )
    {
        model.iteration();
    }

    INFO( fmt::format( "N_iterations = {} (with convergence_tol {})\n", model.n_iterations(), model.convergence_tol ) );
    for( size_t i = 0; i < n_agents; i++ )
    {
        INFO( fmt::format( "Opinion {} = {}\n", i, network.agents[i].data.opinion ) );
        REQUIRE_THAT( network.agents[i].data.opinion, WithinAbs( 0.5, model.convergence_tol * 10.0 ) );
    }
}