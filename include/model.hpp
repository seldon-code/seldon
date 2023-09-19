#pragma once
#include "state.hpp"
#include <cstddef>

namespace Seldon
{

class Model
{

protected:
    State & state;

public:
    Model(State & state);
    virtual void run() = 0;
};

}