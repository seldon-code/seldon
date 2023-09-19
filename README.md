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
