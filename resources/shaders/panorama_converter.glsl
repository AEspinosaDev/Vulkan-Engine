#shader vertex
#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 texCoord;

void main() 
{
//    float x = float((gl_VertexIndex & 1) << 2);
//    float y = float((gl_VertexIndex & 2) << 1);
//    texCoord.x = x * 0.5;
//    texCoord.y = y * 0.5;
//    gl_Position = vec4(x - 1.0, y - 1.0, 0, 1);
 	gl_Position = vec4(pos, 1.0);
    texCoord = uv;
}

#shader geometry
#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out; //6*3

layout(location = 0) in vec2 texCoord[];

layout(location = 0) out vec2 otexCoord;

void main() {
    for(int i = 0; i < 6; i++) {


        gl_Layer = i;

		
        for (int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position; 
            otexCoord = texCoord[i];
            EmitVertex(); 
        }
        EndPrimitive(); // End the first triangle

        
        // for (int i = 0; i < 3; i++) {
        //     gl_Position = gl_in[i + 1].gl_Position; 
        //     oTexCoord = texCoord[i + 1]; 
        //     EmitVertex(); 
        // }

        // gl_Position = gl_in[0].gl_Position; 
		// otexCoord = texCoord[0];
        // EmitVertex();
        
        // gl_Position = gl_in[1].gl_Position; 
		// otexCoord = texCoord[1];
        // EmitVertex();
        
        // gl_Position = gl_in[2].gl_Position; 
		// otexCoord = texCoord[2];
        // EmitVertex();

        EndPrimitive();
      
    }
}


#shader fragment
#version 460 core

#define MATH_PI 3.1415926535897932384626433832795
#define MATH_INV_PI (1.0 / MATH_PI)

precision highp float;

layout(location = 0) in vec2 otexCoord;

layout(location = 0) out vec4 fragmentColor;

layout(set = 0, binding = 0) uniform sampler2D u_panorama;

vec3 uvToXYZ(int face, vec2 uv)
{
	if(face == 0)
		return vec3(     1.f,   uv.y,    -uv.x);

	else if(face == 1)
		return vec3(    -1.f,   uv.y,     uv.x);

	else if(face == 2)
		return vec3(   +uv.x,   -1.f,    +uv.y);

	else if(face == 3)
		return vec3(   +uv.x,    1.f,    -uv.y);

	else if(face == 4)
		return vec3(   +uv.x,   uv.y,      1.f);

	else //(face == 5)
	{	return vec3(    -uv.x,  +uv.y,     -1.f);}
}

vec2 dirToUV(vec3 dir)
{
	return vec2(
		0.5f + 0.5f * atan(dir.z, dir.x) / MATH_PI,
		1.f - acos(dir.y) / MATH_PI);
}

vec3 panoramaToCubeMap(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew);
	vec3 direction = normalize(scan);
	vec2 src = dirToUV(direction);

	return  texture(u_panorama, src).rgb;
}


void main(void)
{
    fragmentColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 color =panoramaToCubeMap(gl_Layer, otexCoord);
    color = color / (color + vec3(1.0));
    const float GAMMA = 2.2;
    color = pow(color, vec3(1.0 / GAMMA));

	fragmentColor.rgb = color;
}
