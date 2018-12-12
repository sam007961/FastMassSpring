#version 130

uniform mat4 uModelViewMatrix;
// uniform mat4 NormalMatrix;
uniform mat4 uProjectionMatrix;

in vec3 aPosition;
// in vec3 aNormal;

// out vec3 vNormal;

void main(){
    // vNormal = NormalMatrix * aNormal;
    vec4 position = vec4(aPosition, 1.0);
    gl_Position = uProjectionMatrix * uModelViewMatrix * position;
    // gl_Position = position;
}