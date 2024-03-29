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
