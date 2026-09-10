#pragma once
#include "agents/friedkin_johnsen_agent.hpp"
#include "config_parser.hpp"
#include "model.hpp"
#include "network.hpp"
#include <optional>
#include <vector>

namespace Seldon
{

/// Friedkin and Johnsen: DeGroot averaging against a fixed anchor.
///
/// DeGroot has a property that is a theorem about the update rule rather than
/// a finding about any group: on a strongly connected, aperiodic network it
/// always converges to one number. Everybody ends up agreeing. A model that
/// cannot represent a group that stays split cannot be used to ask whether one
/// has.
///
/// The fix is one parameter per agent. Each agent keeps weight
/// `1 - susceptibility` on the opinion it started with, so
///
///     x_i(t+1) = lambda_i * sum_j w_ij x_j(t) + (1 - lambda_i) * x_i(0)
///
/// The iteration is still a contraction whenever any agent is stubborn, so it
/// still converges, but to a fixed point that generally is not consensus. The
/// spread of that fixed point is the quantity DeGroot cannot express: how much
/// of the disagreement is structural rather than a matter of more rounds.
///
/// DeGroot is the special case where every lambda_i is 1, and the test suite
/// pins that: this generalizes the model already here rather than replacing
/// it.
///
/// Friedkin, Johnsen, Social influence and opinions, Journal of Mathematical
/// Sociology 15 (1990), doi:10.1080/0022250X.1990.9990069.
class FriedkinJohnsenModel : public Model<FriedkinJohnsenAgent>
{
public:
    using AgentT   = FriedkinJohnsenAgent;
    using NetworkT = Network<AgentT>;

    FriedkinJohnsenModel( Config::FriedkinJohnsenSettings settings, NetworkT & network );

    void iteration() override;
    bool finished() override;

    /// How far apart the opinions are right now.
    ///
    /// Reported rather than derived by a caller because it is the answer the
    /// model exists to give. A converged DeGroot run has a spread of zero by
    /// construction; a converged run of this one does not, and the number is
    /// what says whether the group agreed or merely stopped moving.
    [[nodiscard]] double opinion_spread() const;

private:
    double convergence_tol{};
    std::optional<double> max_opinion_diff = std::nullopt;
    NetworkT & network;
    std::vector<AgentT> agents_current_copy;
};

} // namespace Seldon
