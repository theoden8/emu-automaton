# Author

Created by Kirill Rodriguez on 07/2018.

# About

The purpose of this project is to animate automata in order to provide intuition for understanding complexity, and is to evolve into a more efficient framework for investigating algorithms and topologies in the languages of various automata.

# Demonstration

This is a random **Day and night** simulation:

[![day_and_night](./images/day_and_night.gif)](./images/day_and_night.mp4)

# Tools

* c++17
* opengl 4, glew, glfw3

# Implementations

Currently, the implementation uses a double-buffer of two GL_R8 textures of size proportional to the size of window.

An implementation using GLSL compute shader is attempted, but is not successful yet.

# Compiling

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

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
