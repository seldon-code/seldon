#include "state.hpp"
#include <toml++/toml.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <set>

Seldon::State::State(std::string config_file)
{
    std::set<std::string> allowed_models = {
        "DeGroot"
    };

    toml::table tbl;

    tbl = toml::parse_file(config_file);

    std::string model = tbl["model"].value_or("");

    if (!allowed_models.count(model))
    {
        throw std::runtime_error(fmt::format("Unknown model type: '{}'!", model));
    }
}