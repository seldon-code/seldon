#include "config_parser.hpp"
#include "model.hpp"
#include "models/ActivityDrivenModel.hpp"
#include "models/DeGroot.hpp"
#include "models/DeffuantModel.hpp"
#include "network.hpp"
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>

namespace Seldon::ModelFactory
{

using ModelVariantT = Config::SimulationOptions::ModelVariantT;

template<typename AgentT, typename ModelT, typename FuncT>
auto check_agent_type( FuncT func )
{
    if constexpr( std::is_same_v<AgentT, typename ModelT::AgentT> )
    {
        return func();
    }
    else
    {
        throw std::runtime_error( "Incompatible agent and model type!" );
        return std::unique_ptr<Model<AgentT>>{};
    }
}

template<typename AgentT>
inline auto create_model_degroot( Network<AgentT> & network, const ModelVariantT & model_settings )
{
    if constexpr( std::is_same_v<AgentT, DeGrootModel::AgentT> )
    {
        auto degroot_settings = std::get<Config::DeGrootSettings>( model_settings );
        auto model            = std::make_unique<DeGrootModel>( degroot_settings, network );
        return model;
    }
    else
    {
        throw std::runtime_error( "Incompatible agent and model type!" );
        return std::unique_ptr<Model<AgentT>>{};
    }
}

template<typename AgentT>
inline auto
create_model_activity_driven( Network<AgentT> & network, const ModelVariantT & model_settings, std::mt19937 & gen )
{
    if constexpr( std::is_same_v<AgentT, ActivityDrivenModel::AgentT> )
    {
        auto activitydriven_settings = std::get<Config::ActivityDrivenSettings>( model_settings );
        auto model                   = std::make_unique<ActivityDrivenModel>( activitydriven_settings, network, gen );
        return model;
    }
    else
    {
        throw std::runtime_error( "Incompatible agent and model type!" );
        return std::unique_ptr<Model<AgentT>>{};
    }
}

template<typename AgentT>
inline auto create_model_deffuant( Network<AgentT> & network, const ModelVariantT & model_settings, std::mt19937 & gen )
{
    if constexpr( std::is_same_v<AgentT, DeffuantModel::AgentT> )
    {
        auto deffuant_settings = std::get<Config::DeffuantSettings>( model_settings );
        auto model             = std::make_unique<DeffuantModel>( deffuant_settings, network, gen );
        return model;
    }
    else
    {
        throw std::runtime_error( "Incompatible agent and model type!" );
        return std::unique_ptr<Model<AgentT>>{};
    }
}

template<typename AgentT>
inline auto
create_model_deffuant_vector( Network<AgentT> & network, const ModelVariantT & model_settings, std::mt19937 & gen )
{
    if constexpr( std::is_same_v<AgentT, DeffuantModelVector::AgentT> )
    {
        auto deffuant_settings = std::get<Config::DeffuantSettings>( model_settings );
        auto model             = std::make_unique<DeffuantModelVector>( deffuant_settings, network, gen );
        model.initialize_agents();
        return model;
    }
    else
    {
        throw std::runtime_error( "Incompatible agent and model type!" );
        return std::unique_ptr<Model<AgentT>>{};
    }
}

} // namespace Seldon::ModelFactory