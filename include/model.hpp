#pragma once
#include <cstddef>

namespace Seldon
{

class Model
{

public:
    Model(){};
    virtual void run() = 0;
};

} // namespace Seldon