#version 130

in vec3 vNormal;
out vec4 fragColor;

void main(){
    vec3 normal = normalize(vNormal);
    if(!gl_FrontFacing) normal = -normal;

    vec3 toLight = normalize(vec3(0, 0, 0.5));
    float diffuse = max(0, dot(toLight, normal));

    toLight = normalize(vec3(-3, -1, 0.5));
    diffuse += max(0, dot(toLight, normal));
    fragColor = diffuse * vec4(0, 0.3, 0.7, 1.0);
}