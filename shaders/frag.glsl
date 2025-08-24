#version 450
layout(location=0) in vec3 vNormal;
layout(location=1) in vec2 vUV;
layout(location=0) out vec4 outColor;

layout(set=0, binding=2) uniform SceneUBO {
  vec3 lightDir;
  vec3 baseColor;
} sceneUBO;

void main(){
  float NdotL = max(dot(normalize(vNormal), normalize(-sceneUBO.lightDir)), 0.0);
  vec3 color = sceneUBO.baseColor * (0.2 + 0.8 * NdotL);
  outColor = vec4(color,1.0);
}
