#version 150 compatibility

layout(triangles) in;
layout(line_strip, max_vertices=50) out;

uniform float length;
uniform int segment;
uniform vec3 gravity;
uniform vec4 color_in;

out vec4 color_;

in Vertex{
    vec3 normal;
}vertex[];

void main(){
    
    for(int i = 0; i < gl_in.length(); i++){

		color_ = vec4(0.0f,0.0f,0.0f,0.0f);
		vec4 tmp =  gl_in[i].gl_Position;
		vec3 tmp_normal = vertex[i].normal;
		vec4 output_color = color_in;

		for(int j = 0 ;j < segment ; j++){

			//start point
			gl_Position = gl_ProjectionMatrix * tmp;
		  EmitVertex();

			tmp_normal+= gravity;
			tmp_normal = normalize(tmp_normal);

		   //end point
		  gl_Position = gl_ProjectionMatrix * (tmp + vec4(tmp_normal,0.0f) * length);
		  EmitVertex();

		  tmp = tmp + vec4(tmp_normal,0.0f) * length;

		  output_color.x = color_in.x + j*0.3/segment;
		  output_color.y = color_in.y + j*0.3/segment;
		  output_color.z = color_in.z + j*0.7/segment;
		  
		  color_ = output_color;
		}
		
        EndPrimitive();
    }
}
