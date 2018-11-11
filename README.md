# nestake

[![CircleCI](https://circleci.com/gh/mathetake/nestake.svg?style=shield)](https://circleci.com/gh/mathetake/nestake)

a NES emulator written in c++


# build && test

```$bash
git clone https://github.com/mathetake/nestake
cd nestake
mkdir -p cmake/DownloadProject && git clone https://github.com/Crascit/DownloadProject cmake/DownloadProject
mkdir build && cd build
cmake ..
make
make test
```