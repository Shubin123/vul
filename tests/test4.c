#define CGLTF_IMPLEMENTATION
#include "../include/cgltf.h" //get ready for binary to explode in size

int main()
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, "./gltfs/testScene.gltf", &data);

    if (result == cgltf_result_success)
    {
        printf("Success parsing gltf file\n");
        /* TODO make awesome stuff */
        cgltf_free(data);
    }
    else
    {
        // Handle error
        switch (result)
        {
        case cgltf_result_data_too_short:
            fprintf(stderr, "Error: Data too short\n");
            break;
        case cgltf_result_unknown_format:
            fprintf(stderr, "Error: Unknown format\n");
            break;
        case cgltf_result_invalid_json:
            fprintf(stderr, "Error: Invalid JSON\n");
            break;
        case cgltf_result_invalid_gltf:
            fprintf(stderr, "Error: Invalid GLTF\n");
            break;
        case cgltf_result_out_of_memory:
            fprintf(stderr, "Error: Out of memory\n");
            break;
        case cgltf_result_io_error:
            fprintf(stderr, "Error: IO error\n");
            break;
        // Add cases for other error types as needed
        default:
            fprintf(stderr, "Error: Unknown error\n");
            break;
        }
    }

    return 0;
}
