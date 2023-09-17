#pragma once
#include "model.hpp"

namespace Seldon {

class DeGrootModel : Model
{
    private:
    std::vector<Agent> agent_current_copy;

    public:
    DeGrootModel(State & state);

    void run() override;
};

}