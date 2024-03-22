#pragma once
#include <cstddef>
#include <optional>

namespace Seldon
{

/* Model<T> is a base class from which the acutal models would derive. They have efficient access to a vector of AgentT,
 * without any pointer indirections */
template<typename AgentT_>
class Model
{
public:
    using AgentT = AgentT_;

    Model() = default;
    Model( std::optional<size_t> max_iterations ) : max_iterations( max_iterations ){};

    virtual void initialize_iterations()
    {
        _n_iterations = 0;
    }

    virtual void iteration()
    {
        _n_iterations++;
    };

    size_t n_iterations()
    {
        return _n_iterations;
    }

    virtual bool finished()
    {
        if( max_iterations.has_value() )
        {
            return max_iterations.value() <= n_iterations();
        }
        else
        {
            return false;
        }
    };

    virtual ~Model() = default;

private:
    std::optional<size_t> max_iterations = std::nullopt;
    size_t _n_iterations{};
};

} // namespace Seldon