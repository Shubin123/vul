#version 450
layout(location = 0) in vec3 fragPosition; // Input from vertex shader
layout(location = 0) out vec4 outColor;   // Output color

void main() {
    vec3 normPos = fragPosition * 0.5 + 0.5; // Normalize the position
    outColor = vec4(normPos, 1.0);           // Output color based on position
}
