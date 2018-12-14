#version 130

in vec2 vTexCoord;
out vec4 fragColor;

uniform int uTessFact;

void main(){
    int vx = int(floor(vTexCoord.x * uTessFact + 0.5));
    int vy = int(floor(vTexCoord.y * uTessFact + 0.5));
    fragColor = vec4(vx *1.0/ uTessFact, vy*1.0 / uTessFact, 0, 1.0);
}