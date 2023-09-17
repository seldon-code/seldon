#include <iostream>
#include <fmt/core.h>

#include "state.hpp"

#include "models/DeGroot.hpp"

int main()
{
    auto state = Seldon::State("conf.toml");
    auto model = Seldon::DeGrootModel(state);
    model.run();
}