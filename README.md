# Seldon - A code for opinion dynamics simulations

![Logo](res/logo_text.png "Seldon Logo")

## About 

Seldon is a performant code for performing various types of opinion dynamics simulations (under development), written primarily in `C++`. 

## Installation 

We use [`pixi`](https://github.com/prefix-dev/pixi) as the package manager. 

```bash
pixi shell # to activate the env
```

### Compilation

We use `meson` to compile and build Seldon. 

```bash
meson setup build
meson compile -C build
```

### Quick Start 

Run the executable, and provide the input configuration TOML file (as the first
positional argument), and an optional output directory parent location. If the output location is not specified, it is set to the parent directory in which the config file is present

```bash
cd build
./seldon /path/to/config -o /path/to/parent/output/dir
``` 
