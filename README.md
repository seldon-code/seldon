# Seldon - A code for opinion dynamics simulations

![Logo](res/logo_text.png "Seldon Logo")

## About 

Seldon is a performant code for performing various types of opinion dynamics simulations (under development), written primarily in `C++`. 

## Installation 
If you want to use [`micromamba`](https://mamba.readthedocs.io/en/latest/user_guide/micromamba.html) as the package manager, create and activate the environment.

```bash
micromamba create -f environment.yml
micromamba activate seldonenv
```

### Compilation and Installation

We use `meson` to compile and build Seldon. 

```bash
meson setup build
meson compile -C build
```

To install `seldon` to your `conda` environment, run the following:

```bash
meson setup build --prefix $CONDA_PREFIX
meson install -C build
```

### Quick Start 

Run the executable, and provide the input configuration TOML file (as the first
positional argument), and an optional output directory location. If the output location is not specified, it is set to the parent directory in which the config file is present

```bash
cd build
./seldon /path/to/config -o /path/to/output/dir
``` 

If you've installed it, you can simply run `seldon` anywhere.

```bash
seldon /path/to/config -o /path/to/output/dir
```

#### Models

| Model | What it is | Reference |
|---|---|---|
| `DeGroot` | Repeated weighted averaging over the network | doi:10.1080/01621459.1974.10480137 |
| `FriedkinJohnsen` | DeGroot averaging against each agent's initial opinion | doi:10.1080/0022250X.1990.9990069 |
| `Deffuant` | Pairwise interaction within a confidence bound | |
| `ActivityDriven` | Time-varying network from agent activity | |
| `ActivityDrivenInertial` | The same, with inertia | |

On a strongly connected, aperiodic network DeGroot always converges to one
number. That is a property of the update rule, not a finding about the group,
so a run of it cannot answer whether a group stays split.

`FriedkinJohnsen` adds one parameter per agent. Each agent keeps weight
`1 - susceptibility` on the opinion it started with:

```
x_i(t+1) = lambda_i * sum_j w_ij x_j(t) + (1 - lambda_i) * x_i(0)
```

It still converges when any agent is stubborn, but to a fixed point that
generally is not consensus. Setting every `susceptibility` to 1 reproduces
DeGroot exactly, and a test pins that.

Set it for the whole run in the config:

```toml
[FriedkinJohnsen]
convergence = 1e-6
susceptibility = 0.5
```

or leave `susceptibility` out and give each agent its own in an agent file,
whose columns are `opinion, initial_opinion, susceptibility`. An agent file
that gives only an opinion anchors the agent to it.

#### Initial opinions

Opinions come from the run, not from the model. A network read with `-n`, or
an agent file passed with `-a`, is used as given. Only a generated network is
seeded, with opinions spread evenly over `[0, 1)` in agent order.

#### Output files
The file `network.txt` contains information about the network. 
First column is the index of the agent, then the next column is the number of incoming agent connections *including* the agent itself. Subsequent columns are the neighbouring incoming agent indices and weights. In addition, every iteration produces a *double* opinion value for each agent. These are outputted to files named opinions_i.txt.

### Running Tests

To run the tests, go into the build directory and run the following: 

```bash
meson test
```

## Contributing

> All contributions are welcome!!

- We follow a modified [NumPy commit guidelines](https://numpy.org/doc/stable/dev/development_workflow.html#writing-the-commit-message).
  + Each commit is to be prefixed by the area of code it affects
- Please run our linter and tests locally to ensure no errors exist
- [Co-author commits](https://github.blog/2018-01-29-commit-together-with-co-authors/) generously

# License
GNU GPL v3 or later.
