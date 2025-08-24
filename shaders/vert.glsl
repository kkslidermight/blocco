#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;

layout(set=0, binding=0) uniform CameraUBO {
  mat4 view;
  mat4 proj;
} cameraUBO;
layout(set=0, binding=1) uniform ModelUBO {
  mat4 model;
} modelUBO;

layout(location=0) out vec3 vNormal;
layout(location=1) out vec2 vUV;

void main(){
  gl_Position = cameraUBO.proj * cameraUBO.view * modelUBO.model * vec4(inPos,1.0);
  vNormal = mat3(modelUBO.model) * inNormal;
  vUV = inUV;
}
