#version 450

// Uniform buffer object (UBO) for time
layout(binding = 0) uniform UBO {
    float time;
} ubo;

// Input vertex position
layout(location = 0) in vec3 inPosition;

// Output vertex position
layout(location = 0) out vec3 fragPosition;

void main() {
    // Determine the center point of the cube
    vec3 center = vec3(0.0, 0.0, 0.0); // Assuming the cube is centered at the origin

    // Calculate rotation angle based on time (you can modify this based on your needs)
    float angle = radians(45.0) * ubo.time; // Rotate at 45 degrees per second

    // Apply rotation transformation around the center point
    mat4 rotationMatrix = mat4(
        cos(angle), -sin(angle), 0.0, 0.0,
        sin(angle), cos(angle), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Calculate the transformed position
    vec4 newPosition = rotationMatrix * vec4(inPosition - center, 1.0) + vec4(center, 0.0);

    // Output the transformed position
    fragPosition = newPosition.xyz;

    // Output the position for vertex shader
    gl_Position = vec4(newPosition.xyz, 1.0);
}
