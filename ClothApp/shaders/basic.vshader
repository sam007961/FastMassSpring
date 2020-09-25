#version 120

uniform mat4 uModelViewMatrix;
// uniform mat4 uNormalMatrix;
uniform mat4 uProjectionMatrix;

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;

varying vec3 vNormal;
varying vec2 vTexCoord;

void main(){
    vNormal = aNormal;
    vTexCoord = aTexCoord;
    vec4 position = vec4(aPosition, 1.0);
    gl_Position = uProjectionMatrix * uModelViewMatrix * position;
}