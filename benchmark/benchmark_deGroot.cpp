#include <benchmark/benchmark.h>
#include "config_parser.hpp"
#include "models/DeGroot.hpp"
#include "network.hpp"
#include <random>

using namespace Seldon;

// Benchmark function for deGroot model simulation
static void BM_DeGroot_Model(benchmark::State& state) {
    using Network = Network<DeGrootModel::AgentT>;
    size_t n_agents = 2;
    auto neighbour_list = std::vector<std::vector<size_t>>{
        {1, 0},
        {0, 1},
    };

    auto weight_list = std::vector<std::vector<double>>{
        {0.2, 0.8},
        {0.2, 0.8},
    };

    auto settings = Config::DeGrootSettings(); 
    auto network = Network(std::move(neighbour_list), std::move(weight_list), Network::EdgeDirection::Incoming);

    settings.convergence_tol = 1e-6;
    settings.max_iterations  = 100;    

    auto model = DeGrootModel(settings,network);

    network.agents[0].data.opinion = 0.0;
    network.agents[1].data.opinion = 1.0;

    for (auto _ : state) {
        while (!model.finished()) {
            model.iteration();
        }
    }

}

// Register the benchmark
BENCHMARK(BM_DeGroot_Model);

// Run the benchmark
BENCHMARK_MAIN();
