#version 450 core

in vec2 vUv;
out vec4 out_color;

uniform vec3 uColorA;
uniform vec3 uColorB;
uniform vec3 uColorC;

void main() {
  vec3 temp = mix(uColorA, uColorB, vUv.x);
  vec3 blendedColor = mix(temp, uColorC, vUv.y * 0.95);
  out_color = vec4(blendedColor, 1.0);
}
