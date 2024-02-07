CC = clang
CFLAGS = -std=c11 -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lvulkan -lMoltenVK -lglfw -lpthread -framework Cocoa -framework IOKit -framework CoreVideo   
APPFLAGS = -L./ships -rpath @executable_path/ships -lvulkan -lglfw -lMoltenVK -framework Cocoa -framework IOKit -framework CoreVideo -framework IOSurface -framework Metal -framework QuartzCore


#app is dynamically linked with libaries in ./ships
vulkanapp: vulkanapp.c
	$(CC) $(CFLAGS) -o vulkanapp vulkanapp.c $(APPFLAGS)

#app is dynamically linked through the system (brew for me) and vulkan sdk
vulkan: vulkanapp.c
	$(CC) $(CFLAGS) -o vulkantest vulkanapp.c $(LDFLAGS)

#tests
test: tests/test.c
	$(CC) $(CFLAGS) -o test ./tests/test.c $(LDFLAGS)
test2: tests/test2.c
	$(CC) $(CFLAGS) -o test2 ./tests/test2.c -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo


# Shader compilation
shaders: shaders/vertex_shader.spv shaders/fragment_shader.spv

shaders/vertex_shader.spv: shaders/vertex_shader.glsl
	glslangValidator -V -S vert -o shaders/vertex_shader.spv shaders/vertex_shader.glsl

shaders/fragment_shader.spv: shaders/fragment_shader.glsl
	glslangValidator -V -S frag -o shaders/fragment_shader.spv shaders/fragment_shader.glsl

.PHONY: shaders

clean:
	rm -f main
cleanall:
	rm -f main test vulkanapp
