#version 130

in vec3 vNormal;
out vec4 fragColor;

void main(){
    vec3 toLight = normalize(vec3(0, 1, 1));
    float diffuse = max(0, dot(toLight, normalize(vNormal)));
    fragColor = diffuse * vec4(0, 0.3, 0.7, 1.0);
}