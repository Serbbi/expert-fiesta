#version 330 core

// Specify the input locations of attributes
layout(location = 0) in vec3 vertCoordinates_in;
layout(location = 1) in vec3 vertNormal_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelViewTransform;
uniform mat4 projectionTransform;
uniform vec3 bottomColor;
uniform vec3 middleColor;
uniform vec3 topColor;

// Specify the constants
const vec3 materialColor = vec3(1.0F, 1.0F, 1.0F);

// Specify the output of the vertex stage
out vec3 color;

void main() {
  // gl_Position is the output (a vec4) of the vertex shader
  gl_Position = projectionTransform * modelViewTransform * vec4(vertCoordinates_in, 1.0F);
  // vertNormal = normalize(normalMatrix * vertNormal_in);

  float vertexHeight = vertCoordinates_in.y;
  // Define the thresholds
    float threshold1 = 0.0;
    float threshold2 = 22.0;

    // Interpolate the colors based on height
    float t = clamp((vertexHeight - threshold1) / (threshold2 - threshold1), 0.0, 1.0);
    color = mix(bottomColor, middleColor, t);
    color = mix(color, topColor, smoothstep(threshold1, threshold2, vertexHeight));
}
