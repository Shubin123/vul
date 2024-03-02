CC = clang
CFLAGS = -std=c11 -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lvulkan -lMoltenVK -lglfw -lpthread -framework Cocoa -framework IOKit -framework CoreVideo   
APPFLAGS = -L./ships -rpath @executable_path/ships -lpthread -lvulkan -lglfw -lMoltenVK -framework Cocoa -framework IOKit -framework CoreVideo -framework IOSurface -framework Metal -framework QuartzCore
WARNINGS = -Wall -Wextra -Wpedantic -Werror
WINFLAGS = -I./include -I./windowsInclude/vulkanSDK/1.3.275.0/Include -I./windowsInclude/glfw-3.4.bin.WIN64/include  -I./windowsInclude/cglm-0.9.2/include -l./windowsInclude/vulkanSDK/1.3.275.0/Lib/vulkan-1.lib -l./windowsInclude/glfw-3.4.bin.WIN64/lib-static-ucrt/glfw3dll


#app is dynamically linked with libaries in ./ships
vulkanapp: ./src/vulkanapp.c ./src/helpers.c ./src/myvulkan.c ./src/mymath.c ./src/mygltf.c ./src/myglfw.c
	$(CC) $(CFLAGS) -o vulkanapp ./src/vulkanapp.c $(WARNINGS) $(APPFLAGS)

winapp: ./src/vulkanapp.c ./src/helpers.c ./src/myvulkan.c ./src/mymath.c ./src/mygltf.c ./src/myglfw.c
	$(CC) -o vulkanapp.exe ./src/vulkanapp.c $(WINFLAGS)


#tests
test: tests/test.c #vulkan test
	$(CC) $(CFLAGS) -o test ./tests/test.c $(LDFLAGS)
test2: tests/test2.c #glfw test
	$(CC) $(CFLAGS) -o test2 ./tests/test2.c -lglfw -framework Cocoa -framework IOKit -framework CoreVideo
test3: tests/test3.c #cglm test
	$(CC) -o test3 ./tests/test3.c
test4: tests/test4.c #cgltf test
	$(CC) -o test4 ./tests/test4.c $(WARNINGS) 

testwin: tests/test.c
	$(CC) -o test ./tests/test.c $(WINFLAGS)
testwin2: tests/test2.c
	$(CC) -o test2 ./tests/test2.c $(WINFLAGS)
testwin3: tests/test3.c
	$(CC) -o test3 ./tests/test3.c $(WINFLAGS)
testwin4: tests/test4.c
	$(CC) -o test4 ./tests/test4.c $(WINFLAGS)

# Shader compilation
shaders: shaders/vertex_shader.spv shaders/fragment_shader.spv

shaders/vertex_shader.spv: shaders/vertex_shader.glsl
	glslangValidator -V -S vert -o shaders/vertex_shader.spv shaders/vertex_shader.glsl

shaders/fragment_shader.spv: shaders/fragment_shader.glsl
	glslangValidator -V -S frag -o shaders/fragment_shader.spv shaders/fragment_shader.glsl

.PHONY: shaders

clean:
	rm -f vulkanapp vulkantest test test2 test3 test4
