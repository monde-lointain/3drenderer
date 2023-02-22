# 3drenderer
Features include loading models via obj files, rotating/scaling/translating models, free look camera, different rendering modes, flat/Gouraud shading and adjustable lighting. Uses multithreading via Visual Studio OpenMP directives.

[![](preview.mp4)](https://user-images.githubusercontent.com/114368550/220672803-867eb62d-414a-45dc-8599-ef64ddf9efd3.mp4)<br>

References used while writing this program include [Gustavo Pezzi](https://github.com/gustavopezzi/3drenderer) and [zauonlok](https://github.com/zauonlok/renderer)'s renderers, as well as the [Mesa3D/Gallium3D](https://github.com/Mesa3D/mesa) OpenGL drivers.

Libraries used:<br>
[SDL2](https://github.com/libsdl-org/SDL)<br>
[glm](https://github.com/g-truc/glm)<br>
[vectorclass](https://github.com/vectorclass/version2)<br>
[imgui](https://github.com/ocornut/imgui)<br>
[fast_obj](https://github.com/thisistherk/fast_obj)<br>
[tracy](https://github.com/wolfpld/tracy) (profiling)
