#version 450

layout(binding = 0) uniform UBO {
    float time;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPosition;

void main() {
    vec3 center = vec3(0.0, 0.0, 0.0);
    float angle = radians(45.0) * ubo.time;

    // Rotate around the Y-axis
    mat4 rotationMatrix = mat4(
        cos(angle), 0.0, sin(angle), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(angle), 0.0, cos(angle), 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    mat4 model = rotationMatrix;
    vec4 worldPosition = model * vec4(inPosition, 1.0);
    vec4 viewPosition = ubo.view * worldPosition;
    vec4 projectedPosition = ubo.projection * viewPosition;

    fragPosition = worldPosition.xyz;
    gl_Position = projectedPosition;
}
