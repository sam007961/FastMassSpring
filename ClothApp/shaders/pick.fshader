#version 120

in vec2 vTexCoord;

uniform int uTessFact;

void main(){
    float vx = floor(vTexCoord.x * (uTessFact - 1));
    float vy = floor(vTexCoord.y * (uTessFact - 1));
    gl_FragColor = vec4(vx / (uTessFact - 1), vy / (uTessFact - 1), 0.2, 1.0);
}