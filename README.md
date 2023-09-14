# Author

Created by Kirill Rodriguez on 07/2018.

# About

The purpose of this project is to animate automata in order to provide intuition for understanding complexity, and is to evolve into a more efficient framework for investigating algorithms and topologies in the languages of various automata.

# Demonstration

This is a random **Day and night** simulation:

[![day_and_night](./images/day_and_night.gif)](./images/day_and_night.mp4)

# Tools

* c++20, clang++
* opengl 3/4, libepoxy, glfw
* [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)

# Special features

* GPU-powered updates
* Ising model
* Multi-state automata

# Implementation

* Renderer
    * GLSL compute shaders (only B/S/C automata, when compute shaders are supported)
    * OpenMP-powered updates on CPU otherwise
* Storage mode
    * Textures (B/S/C automata)
    * CPU memory
        * Single buffer on CPU for automata where individual cells are updated (e.g. Ising model)
        * Double-buffer on CPU for update-all cellular automata
        * Extra buffer for case when buffer is larger than screen (for averaging)
* Access mode
    * Bounded
    * Toroid (looped)
* Topology
    * Grid

# Compiling

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cd ..
make -C build
# running
./build/automaton
```

# Potential roadmap

* Loading specific patterns
* More kinds of initializations
* Triangular/Hexagonal topologies
* More kinds of rules
* More stochastic automata
* Continuous automata
* Training a model to learn evolution of a stable CA

# References

* http://www.conwaylife.com/wiki/Main_Page
* http://www.conwaylife.com/forums/viewtopic.php?t=3303
* https://en.wikipedia.org/wiki/Elementary_cellular_automaton
* https://en.wikipedia.org/wiki/Life-like_cellular_automaton
* https://codegolf.stackexchange.com/questions/88783/build-a-digital-clock-in-conways-game-of-life/
* https://codegolf.stackexchange.com/questions/11880/build-a-working-game-of-tetris-in-conways-game-of-life
* http://play.starmaninnovations.com/qftasm/
* https://www.youtube.com/watch?v=_eC14GonZnU
* http://uncomp.uwe.ac.uk/genaro/rule110/glidersRule110.html
* https://neerc.ifmo.ru/wiki/index.php?title=%D0%9A%D0%BE%D0%BB%D0%BC%D0%BE%D0%B3%D0%BE%D1%80%D0%BE%D0%B2%D1%81%D0%BA%D0%B0%D1%8F_%D1%81%D0%BB%D0%BE%D0%B6%D0%BD%D0%BE%D1%81%D1%82%D1%8C
* http://www.chaos-math.org/en
* http://www.mirekw.com/ca/ca_rules.html
