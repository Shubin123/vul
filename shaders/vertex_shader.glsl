#version 450
layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UBO {
    float time;
} ubo;

void main() {
    // Example: Simple horizontal movement
    float xMovement = sin(ubo.time);
    vec3 newPosition = inPosition + vec3(xMovement, 0.0, 0.0);
    gl_Position = vec4(newPosition, 1.0);
}
