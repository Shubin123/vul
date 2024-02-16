#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h" //get ready for binary to explode in size

int main() {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "./gltfs/lambo_sto/scene.gltf", &data);

    if (result == cgltf_result_success) {
        printf("Success parsing gltf file\n");
        /* TODO make awesome stuff */
        cgltf_free(data);
    } else {
        // Handle error
        printf("Error parsing gltf file\n");
    }

    return 0;
}
