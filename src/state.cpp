#include "state.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <toml++/toml.h>
#include <iostream>
#include <optional>
#include <set>

Seldon::State::State( std::string config_file )
{
    std::set<std::string> allowed_models = { "DeGroot" };

    toml::table tbl;
    tbl = toml::parse_file( config_file );

    // Check if the 'model' keyword exists
    std::optional<std::string> model_opt = tbl["model"].value<std::string>();
    if( !model_opt.has_value() )
        throw std::runtime_error( fmt::format( "Configuration file needs to include 'model'!" ) );

    // Check if 'model' is one of the allowed values
    auto model = model_opt.value();
    if( !allowed_models.count( model ) )
    {
        throw std::runtime_error( fmt::format( "Unknown model type: '{}'!", model ) );
    }

    // TODO: construct model object from conf
}