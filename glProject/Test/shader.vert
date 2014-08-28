#version 410

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in int bone_ids[4];
layout(location = 4) in vec4 bone_weights;

uniform mat4 bones[61];
uniform mat4 MVP;

out vec2 uvf;
out vec3 normalf;
out vec3 vertexf;

out vec4 color;

void main () {
	
	//mat4 BoneTransform = bones[bone_ids[0]] * bone_weights[0];
    //BoneTransform += bones[bone_ids[1]] * bone_weights[1];
    //BoneTransform += bones[bone_ids[2]] * bone_weights[2];
    //BoneTransform += bones[bone_ids[3]] * bone_weights[3];
	//vec4 PosL = BoneTransform * vec4(vertex, 1.0);
	//bones[bone_ids[0]] * bone_weights[0]
	//gl_Position =  MVP * vec4(vertex_position, 1.0);

	uvf = uv;
	normalf = normal;
	vertexf = vertex.xyz;

	color = vec4(normal.xyz, 1.0);
	gl_Position = MVP * vec4(vertex*5.0, 1.0);
}