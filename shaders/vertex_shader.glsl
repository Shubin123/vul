#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPosition; // Output to fragment shader

layout(binding = 0) uniform UBO {
    float time;
} ubo;

void main() {
    float xMovement = sin(ubo.time);
    vec3 newPosition = inPosition + vec3(xMovement, 0.0, 0.0);
    fragPosition = newPosition; // Pass newPosition as fragPosition
    gl_Position = vec4(newPosition, 1.0);
}
