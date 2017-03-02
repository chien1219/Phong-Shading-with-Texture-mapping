#version 150 compatibility

out vec3 vertexEyeSpace;
out vec3 normal;

void main(){
	  gl_TexCoord[0] = gl_MultiTexCoord0;
    vertexEyeSpace = vec3(gl_ModelViewMatrix * gl_Vertex);
    normal = normalize(gl_NormalMatrix * gl_Normal);
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
