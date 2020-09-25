#version 120

in vec3 vNormal;

uniform vec3 uAlbedo;
uniform vec3 uAmbient;
uniform vec3 uLight;
void main(){
    vec3 normal = normalize(vNormal);
	vec3 albedo = uAlbedo;
    if(!gl_FrontFacing) {
		normal = -normal;
		albedo = 1.0 - albedo;
	}

    vec3 toLight = normalize(-uLight);
    float diffuse = max(0, dot(toLight, normal));

    vec3 color = diffuse * albedo + uAmbient * albedo;
    gl_FragColor = vec4(color, 1.0);
}