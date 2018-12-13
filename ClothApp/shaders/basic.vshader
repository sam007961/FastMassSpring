#version 130

uniform mat4 uModelViewMatrix;
// uniform mat4 NormalMatrix;
uniform mat4 uProjectionMatrix;

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 vNormal;
out vec2 vTexCoord;

void main(){
    vNormal = aNormal;
    vTexCoord = aTexCoord;
    vec4 position = vec4(aPosition, 1.0);
    gl_Position = uProjectionMatrix * uModelViewMatrix * position;
}