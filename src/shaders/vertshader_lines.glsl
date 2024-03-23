#version 330 core

// const float density = 0.007;
// const float gradient = 1;

// Specify the input locations of attributes
layout(location = 0) in vec3 vertCoordinates_in;
// layout(location = 1) in vec3 vertNormal_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;
// uniform mat3 normalMatrix;

// Specify the output of the vertex stage
// out vec3 vertNormal;
// out float visibility;

void main() {
  // gl_Position is the output (a vec4) of the vertex shader
  vec4 worldPosition = modelViewTransform * vec4(vertCoordinates_in, 1.0F);
  gl_Position = projectionTransform * worldPosition;
  // vertNormal = normalize(normalMatrix * vertNormal_in);

  // float distance = length(worldPosition);
  // visibility = exp(-pow((distance * density), gradient));
  // visibility = clamp(visibility, 0.0, 1.0);
}
