#version 450

layout(binding = 0) uniform UBO {
    float time;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inModelRow0;
layout(location = 2) in vec4 inModelRow1;
layout(location = 3) in vec4 inModelRow2;
layout(location = 4) in vec4 inModelRow3;

layout(location = 0) out vec3 fragPosition;

void main() {
    int speedFactor = 5;
    float angle = radians(45.0) * ubo.time * speedFactor;

    // Rotation matrix around the Y-axis
    mat4 rotationMatrix = mat4(cos(angle), 0.0, sin(angle), 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               -sin(angle), 0.0, cos(angle), 0.0,
                               0.0, 0.0, 0.0, 1.0);

    // Reconstruct the model matrix from instance data
    mat4 modelMatrix = mat4(inModelRow0, inModelRow1, inModelRow2, inModelRow3);

    // Apply rotation to the model matrix
    modelMatrix = modelMatrix * rotationMatrix;

    // Transform the vertex position
    vec4 worldPosition = modelMatrix * vec4(inPosition, 1.0);
    vec4 viewPosition = ubo.view * worldPosition;
    vec4 projectedPosition = ubo.projection * viewPosition;

    fragPosition = worldPosition.xyz;
    gl_Position = projectedPosition;
}
