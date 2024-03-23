#include "config_parser.hpp"
#include "models/DeGroot.hpp"
#include "simulation.hpp"
#include <benchmark/benchmark.h>
#include <argparse/argparse.hpp>
#include <filesystem>
#include <memory>
#include <models/ActivityDrivenModel.hpp>
#include <string>

namespace fs = std::filesystem;
using namespace Seldon;
using AgentT = ActivityDrivenModel::AgentT;

// Benchmark function for ActivityDriven model simulation
static void BM_ActivityDriven( benchmark::State & state )
{

    auto proj_root_path = fs::current_path();
    auto input_file = proj_root_path / fs::path( "test/res/activity_probabilistic_conf.toml" );

    auto options = Config::parse_config_file( input_file.string() );
    auto simulation = Simulation<AgentT>( options, std::nullopt, std::nullopt );

    fs::path output_dir_path = proj_root_path / fs::path( "test/output" );
        fs::create_directories( output_dir_path );

    for (auto _ : state) {
        fs::remove_all(output_dir_path);
        fs::create_directories(output_dir_path);

        assert(fs::is_empty(output_dir_path));

        simulation.run(output_dir_path);

        assert(!fs::is_empty(output_dir_path));

        // Cleanup
        fs::remove_all(output_dir_path);
    }
}

// Benchmark function for ActivityDriven model simulation with two agents
static void BM_ActivityDriven_withTwoAgents(benchmark::State& state) {


    auto proj_root_path = std::filesystem::current_path();
    auto input_file = proj_root_path / std::filesystem::path("test/res/2_agents_activity_prob.toml");

    auto options = Config::parse_config_file(input_file.string());
    auto simulation = Simulation<AgentT>(options, std::nullopt, std::nullopt);

    fs::path output_dir_path = proj_root_path / std::filesystem::path("test/output");

    for (auto _ : state) {
        simulation.run(output_dir_path);

        auto model_settings = std::get<Seldon::Config::ActivityDrivenSettings>(options.model_settings);
        auto K = model_settings.K;
        auto alpha = model_settings.alpha;

        assert(std::abs(K - 2.0) < 1e-16);
        assert(std::abs(alpha - 1.01) < 1e-16);

        double analytical_x = 1.9187384098662013;

         for (size_t idx_agent = 0; idx_agent < simulation.network.n_agents(); idx_agent++) {
            auto &agent = simulation.network.agents[idx_agent];
            assert(std::abs(agent.data.opinion - analytical_x) < 1e-4);
        }

        // Cleanup
        fs::remove_all(output_dir_path);
    }
}


// Benchmark function for ActivityDriven model simulation with one bot and one agent
static void BM_ActivityDriven_withOneBotOneAgent(benchmark::State& state) {

    auto proj_root_path = std::filesystem::current_path();
    auto input_file = proj_root_path / std::filesystem::path("test/res/1bot_1agent_activity_prob.toml");

    auto options = Config::parse_config_file(input_file.string());
    Config::print_settings(options);

    auto simulation = Simulation<AgentT>(options, std::nullopt, std::nullopt);

    fs::path output_dir_path = proj_root_path / std::filesystem::path("test/output");

    for (auto _ : state) {
        // Get the bot opinion (which won't change)
        auto bot = simulation.network.agents[0];
        auto x_bot = bot.data.opinion; // Bot opinion

        // Get the initial agent opinion
        auto &agent = simulation.network.agents[1];
        auto x_0 = agent.data.opinion; // Agent opinion

        simulation.run(output_dir_path);

        auto model_settings = std::get<Seldon::Config::ActivityDrivenSettings>(options.model_settings);
        auto K = model_settings.K;
        auto alpha = model_settings.alpha;
        auto iterations = model_settings.max_iterations.value();
        auto dt = model_settings.dt;
        auto time_elapsed = iterations * dt;

        // Final agent and bot opinions after the simulation run
        auto x_t = agent.data.opinion;
        auto x_t_bot = bot.data.opinion;
        auto reluctance = agent.data.reluctance;
        // The bot opinion should not change during the simulation
        assert(std::abs(x_t_bot - x_bot) < 1e-16);

        auto x_t_analytical = (x_0 - K / reluctance * tanh(alpha * x_bot)) * exp(-time_elapsed)
                              + K / reluctance * tanh(alpha * x_bot);

        assert(std::abs(x_t - x_t_analytical) < 1e-5);

        // Cleanup
        fs::remove_all(output_dir_path);
    }
}




// Register the benchmark
BENCHMARK( BM_ActivityDriven );
BENCHMARK( BM_ActivityDriven_withTwoAgents );
BENCHMARK( BM_ActivityDriven_withOneBotOneAgent );

// Run the main
BENCHMARK_MAIN();
