#version 330 core

// const vec3 skyColor = vec3(0.81f, 0.1f, 0.1f);

uniform vec3 lineColor;

// in vec3 vertNormal;
// in float visibility;

out vec4 fColor;

void main() {
  
  fColor = vec4(lineColor, 1.0F);
  // fColor = mix(vec4(skyColor, 1.0), fColor, visibility);
}
