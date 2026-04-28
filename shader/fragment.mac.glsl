#version 410 core

in vec2 vUv;
out vec4 out_color;

uniform vec3 uColorA;
uniform vec3 uColorB;

void main() {
  vec3 blendedColor = mix(uColorA, uColorB, vUv.x);
  out_color = vec4(blendedColor, 1.0);
}
