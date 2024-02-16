#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h" //get ready for binary to explode in size

// borrows from myvulkan.c create index/vertex buffers cause thats where the api is called
int gltfLoad() {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "scene.gltf", &data);

    if (result != cgltf_result_success) {
        printf("Error parsing gltf file\n");
        cgltf_free(data);
    } 

    cgltf_free(data);



    return 0;
}
