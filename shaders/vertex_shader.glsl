#version 450

layout(binding = 0) uniform UBO {
    float time;
    mat4 model; // Model matrix now includes translation
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPosition;

void main() {
    int speedFactor = 5;
    // Calculate rotation angle based on time
    float angle = radians(45.0) * ubo.time * speedFactor;

    // Rotation matrix around the Y-axis
    mat4 rotationMatrix = mat4(cos(angle), 0.0, sin(angle), 0.0, 0.0, 1.0, 0.0, 0.0, -sin(angle), 0.0, cos(angle), 0.0, 0.0, 0.0, 0.0, 1.0);

    // Apply rotation and then model (translation) matrix
    mat4 modelMatrix = ubo.model * rotationMatrix;
    vec4 worldPosition = modelMatrix * vec4(inPosition, 1.0);

    // Apply view and projection transformations
    vec4 viewPosition = ubo.view * worldPosition;
    vec4 projectedPosition = ubo.projection * viewPosition;

    fragPosition = worldPosition.xyz;
    gl_Position = projectedPosition;
}
