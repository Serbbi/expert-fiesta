#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout(location = 0) in vec3 vertCoordinates_in;
layout(location = 1) in vec3 vertNormal_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;

// Specify the constants
const vec3 materialColor = vec3(1.0F, 1.0F, 1.0F);

// Specify the output of the vertex stage
// out vec3 vertNormal;
out vec3 color;

void main() {
  // gl_Position is the output (a vec4) of the vertex shader
  gl_Position = projectionTransform * modelViewTransform * vec4(vertCoordinates_in, 1.0F);

  float vertexHeight = vertCoordinates_in.y;

  if(vertexHeight >= 14.0) {
    color = vec3(0.3F, 0.0F, 0.3F);
  } else if(vertexHeight >= 12.0) {
    color = vec3(0.5F, 0.0F, 0.5F);
  } else if(vertexHeight >= 10.0) {
    color = vec3(0.0F, 0.0F, 1.0F);
  } else if(vertexHeight >= 8.0) {
    color = vec3(0.0F, 1.0F, 0.0F);
  } else if(vertexHeight >= 6.0) {
    color = vec3(1.0F, 1.0F, 0.0F);
  } else if(vertexHeight >= 4.0) {
    color = vec3(1.0F, 0.5F, 0.0F);
  } else if(vertexHeight >= 2.0) {
    color = vec3(1.0F, 0.0F, 0.0F);
  } else {
    color = vec3(0.5F, 0.0F, 0.0F);
  }

}
