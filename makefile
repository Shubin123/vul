CC = clang
CFLAGS = -std=c11 -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lvulkan -lMoltenVK -lglfw -lpthread -framework Cocoa -framework IOKit -framework CoreVideo   
APPFLAGS = -L./ships -rpath @executable_path/ships -lpthread -lvulkan -lglfw -lMoltenVK -framework Cocoa -framework IOKit -framework CoreVideo -framework IOSurface -framework Metal -framework QuartzCore
WARNINGS = -Wall -Wextra -Wpedantic -Werror

#app is dynamically linked with libaries in ./ships
vulkanapp: ./src/vulkanapp.c ./src/helpers.c ./src/myvulkan.c ./src/mymath.c ./src/mygltf.c ./src/myglfw.c
	$(CC) $(CFLAGS) -o vulkanapp ./src/vulkanapp.c $(WARNINGS) $(APPFLAGS)

#app is dynamically linked through the system (brew for me) and vulkan sdk
vulkan: ./src/ulkanapp.c
	$(CC) $(CFLAGS) -o vulkantest ./src/vulkanapp.c $(LDFLAGS)

#tests
test: tests/test.c #vulkan test
	$(CC) $(CFLAGS) -o test ./tests/test.c $(LDFLAGS)
test2: tests/test2.c #glfw test
	$(CC) $(CFLAGS) -o test2 ./tests/test2.c -lglfw -framework Cocoa -framework IOKit -framework CoreVideo
test3: tests/test3.c #cglm test
	$(CC) -o test3 ./tests/test3.c
test4: tests/test4.c #cgltf test
	$(CC) -o test4 ./tests/test4.c 


# Shader compilation
shaders: shaders/vertex_shader.spv shaders/fragment_shader.spv

shaders/vertex_shader.spv: shaders/vertex_shader.glsl
	glslangValidator -V -S vert -o shaders/vertex_shader.spv shaders/vertex_shader.glsl

shaders/fragment_shader.spv: shaders/fragment_shader.glsl
	glslangValidator -V -S frag -o shaders/fragment_shader.spv shaders/fragment_shader.glsl

.PHONY: shaders

clean:
	rm -f vulkanapp vulkantest test test2 test3
