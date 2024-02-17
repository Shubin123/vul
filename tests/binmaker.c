#include <stdio.h>
#include <stdlib.h>

int main() {
    // Define the vertex positions for a triangle
  float vertices[9] = {
    -0.5f, -0.5f, 0.0f, // Vertex 1
     0.5f, -0.5f, 0.0f, // Vertex 2
     0.0f,  0.5f, 0.0f  // Vertex 3
};
unsigned short indices[3] = {0, 1, 2};

    // Open the file for writing
    FILE *file = fopen("triangle.bin", "wb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Write the vertex data to the file
    size_t written = fwrite(vertices, sizeof(float), 9, file);
    if (written != 9) {
        perror("Error writing vertices to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Write the index data to the file
    written = fwrite(indices, sizeof(unsigned short), 3, file);
    if (written != 3) {
        perror("Error writing indices to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Close the file
    fclose(file);

    printf("Binary file 'triangle.bin' created successfully.\n");

    return EXIT_SUCCESS;
}
