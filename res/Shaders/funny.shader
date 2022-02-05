#shader vertex
#version 460
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colors;
uniform vec3 external_param;
out vec3 color;
void main(){
   color = vertex_colors + external_param;
   gl_Position = vec4(vertex_position*external_param*external_param, 1.0f);
}
#shader fragment
#version 460
in vec3 color;
out vec4 frag_color;
void main(){
   frag_color = vec4(color, 1.0f);
}