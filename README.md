# Seldon - A code for opinion dynamics simulations

![Logo](res/logo_text.png "Seldon Logo")

## About 

Seldon is a performant code for performing various types of opinion dynamics simulations (under development), written primarily in `C++`. 

## Installation 

We use [`pixi`](https://github.com/prefix-dev/pixi) as the package manager. 

```bash
pixi shell # to activate the env
```

If you want to use [`micromamba`](https://mamba.readthedocs.io/en/latest/user_guide/micromamba.html) as the package manager, create and activate 
the environment.

```bash
micromamba create -f environment.yml
micromamba activate seldonenv
```

### Compilation

We use `meson` to compile and build Seldon. 

```bash
meson setup build
meson compile -C build
```

### Quick Start 

Run the executable, and provide the input configuration TOML file (as the first
positional argument), and an optional output directory location. If the output location is not specified, it is set to the parent directory in which the config file is present

```bash
cd build
./seldon /path/to/config -o /path/to/output/dir
``` 

### Running Tests

To run the tests, go into the build directory and run the following: 

```bash
meson test
```