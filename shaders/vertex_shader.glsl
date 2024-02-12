#version 450

layout(binding = 0) uniform UBO {
    float time;
    mat4 view;
    mat4 projection;
} ubo;

// This uniform will receive the dynamic model matrices
layout(binding = 1) uniform DynModel {
    mat4 models[100]; // Example size, adjust as needed
} dynModel;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPosition;

void main() {
    int speedFactor = 5;
    // Calculate rotation angle based on time
    float angle = radians(45.0) * ubo.time * speedFactor;

    // Rotation matrix around the Y-axis
    mat4 rotationMatrix = mat4(cos(angle), 0.0, sin(angle), 0.0, 0.0, 1.0, 0.0, 0.0, -sin(angle), 0.0, cos(angle), 0.0, 0.0, 0.0, 0.0, 1.0);

    // Get the correct model matrix using gl_InstanceIndex and apply rotation
    mat4 modelMatrix = dynModel.models[gl_InstanceIndex] * rotationMatrix;
    vec4 worldPosition = modelMatrix * vec4(inPosition, 1.0);

    // Apply view and projection transformations
    vec4 viewPosition = ubo.view * worldPosition;
    vec4 projectedPosition = ubo.projection * viewPosition;

    fragPosition = worldPosition.xyz;
    gl_Position = projectedPosition;
}
