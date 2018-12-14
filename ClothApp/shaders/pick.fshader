#version 130

in vec2 vTexCoord;
out vec4 fragColor;

uniform int uTessFact;

void main(){
    float vx = floor(vTexCoord.x * uTessFact + 0.5);
    float vy = floor(vTexCoord.y * uTessFact + 0.5);
    fragColor = vec4(vx / uTessFact, vy / uTessFact, 0.2, 1.0);
}