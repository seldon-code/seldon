#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "config_parser.hpp"
#include "models/FriedkinJohnsen.hpp"
#include "network.hpp"

namespace
{

using Network = Seldon::Network<Seldon::FriedkinJohnsenModel::AgentT>;

// Two agents who listen to each other and to themselves, with the same
// weights the DeGroot test uses, so the two models are compared on one network
// rather than on two.
Network two_agents()
{
    auto neighbour_list = std::vector<std::vector<size_t>>{
        { 1, 0 },
        { 0, 1 },
    };
    auto weight_list = std::vector<std::vector<double>>{
        { 0.2, 0.8 },
        { 0.2, 0.8 },
    };
    return Network( std::move( neighbour_list ), std::move( weight_list ), Network::EdgeDirection::Incoming );
}

Seldon::Config::FriedkinJohnsenSettings settings( std::optional<double> susceptibility )
{
    auto settings            = Seldon::Config::FriedkinJohnsenSettings();
    settings.convergence_tol = 1e-8;
    settings.max_iterations  = 10000;
    settings.susceptibility  = susceptibility;
    return settings;
}

} // namespace

// Susceptibility 1 is DeGroot. This is the claim the model documentation
// makes, and it is the reason to add this model rather than replace the other:
// it has to generalize what is already there, exactly, or it is a second
// model that merely resembles it.
TEST_CASE( "A fully susceptible group is DeGroot", "[FriedkinJohnsen]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto network                   = two_agents();
    network.agents[0].data.opinion = 0.0;
    network.agents[1].data.opinion = 1.0;

    auto model = FriedkinJohnsenModel( settings( 1.0 ), network );
    while( !model.finished() )
    {
        model.iteration();
    }

    // The same 0.5 the DeGroot test asserts, from the same weights.
    for( size_t i = 0; i < network.agents.size(); i++ )
    {
        INFO( fmt::format( "Opinion {} = {}\n", i, network.agents[i].data.opinion ) );
        REQUIRE_THAT( network.agents[i].data.opinion, WithinAbs( 0.5, 1e-6 ) );
    }
    REQUIRE_THAT( model.opinion_spread(), WithinAbs( 0.0, 1e-6 ) );
}

// The property DeGroot cannot express. A group that is stubborn settles, and
// what it settles on is not agreement. Reporting consensus here would be
// reporting the update rule rather than the group.
TEST_CASE( "A stubborn group settles without agreeing", "[FriedkinJohnsen]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto network                   = two_agents();
    network.agents[0].data.opinion = 0.0;
    network.agents[1].data.opinion = 1.0;

    auto model = FriedkinJohnsenModel( settings( 0.5 ), network );
    while( !model.finished() )
    {
        model.iteration();
    }

    // It converged: the run ended on the tolerance rather than on the cap.
    REQUIRE( model.n_iterations() < 10000 );
    // And it did not converge to one number.
    REQUIRE( model.opinion_spread() > 0.1 );
    REQUIRE( network.agents[0].data.opinion < network.agents[1].data.opinion );
    // Each agent stayed on its own side of the average it would have reached
    // under DeGroot, which is what the anchor does.
    REQUIRE( network.agents[0].data.opinion < 0.5 );
    REQUIRE( network.agents[1].data.opinion > 0.5 );
}

// Nobody listening is nobody moving, which is the other end of the parameter
// and the sanity check that the anchor is the initial opinion rather than a
// running one.
TEST_CASE( "A group that listens to nothing does not move", "[FriedkinJohnsen]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto network                   = two_agents();
    network.agents[0].data.opinion = 0.2;
    network.agents[1].data.opinion = 0.9;

    auto model = FriedkinJohnsenModel( settings( 0.0 ), network );
    for( int step = 0; step < 50; step++ )
    {
        model.iteration();
    }

    REQUIRE_THAT( network.agents[0].data.opinion, WithinAbs( 0.2, 1e-12 ) );
    REQUIRE_THAT( network.agents[1].data.opinion, WithinAbs( 0.9, 1e-12 ) );
}

// The anchor comes from the agent, not from the model, so a run can give
// different agents different stubbornness. A model that overwrote either would
// be a model with one parameter pretending to have n.
TEST_CASE( "Susceptibility is per agent when the config names none", "[FriedkinJohnsen]" )
{
    using namespace Seldon;
    using namespace Catch::Matchers;

    auto network                          = two_agents();
    network.agents[0].data.opinion        = 0.0;
    network.agents[0].data.susceptibility = 0.0; // immovable
    network.agents[1].data.opinion        = 1.0;
    network.agents[1].data.susceptibility = 1.0; // fully persuadable

    auto model = FriedkinJohnsenModel( settings( std::nullopt ), network );
    while( !model.finished() )
    {
        model.iteration();
    }

    // The immovable one holds, and the persuadable one is dragged to the
    // weighted average of what it hears, which is now pinned by the other.
    REQUIRE_THAT( network.agents[0].data.opinion, WithinAbs( 0.0, 1e-6 ) );
    REQUIRE_THAT( network.agents[1].data.opinion, WithinAbs( 0.0, 1e-6 ) );
}
