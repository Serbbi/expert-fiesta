#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the inputs to the fragment shader
// These must have the same type and name!
in vec3 vertNormal;
in vec4 coordinates;
in vec2 textureCoordinates;

// Specify the Uniforms of the fragment shaders
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec4 materialCoeffecients;
uniform sampler2D samplerUniform; 

// Specify the constants
const vec3 materialColor = vec3(1.0F, 1.0F, 1.0F);

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

void main() {
  vec4 textureColor = texture(samplerUniform, textureCoordinates);
  if(textureColor.a < 0.5) {
      discard;
  }

  vec3 V = vec3(coordinates);
  vec3 normNormal = normalize(vertNormal);

  vec3 Ia = textureColor.rgb * materialCoeffecients.x;
  vec3 L = normalize(lightPosition - V);
  vec3 Id = max(0.0, dot(L, normNormal)) * textureColor.rgb * lightColor * materialCoeffecients.y;
  vec3 R = reflect(-L, normNormal);

  vec3 normV = normalize(-V);
  vec3 Is = pow(max(0.0, dot(R, normV)), materialCoeffecients.w) * lightColor * materialCoeffecients.z;
  vec3 phongColor = Ia + Id + Is;
  fColor = vec4(textureColor.rgb, 1.0F);
}
