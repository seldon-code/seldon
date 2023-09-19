#pragma once
#include "model.hpp"

namespace Seldon {

class DeGrootModel : public Model
{
    private:
    std::vector<Agent> agent_current_copy;

    public:
    DeGrootModel(State & state);

    void run() override;
};

}