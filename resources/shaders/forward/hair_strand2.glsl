#shader vertex
#version 460 core
#include object.glsl

//Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 color;

//Output
layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_tangent;

void main() {

    gl_Position = object.model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(object.model))) * tangent);
    v_color = color;

}

#shader geometry
#version 460 core
#include camera.glsl

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec3 v_color[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;
} material;

//Output
layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec3 g_modelPos;
layout(location = 2) out vec3 g_normal;
layout(location = 3) out vec3 g_modelNormal;
layout(location = 4) out vec2 g_uv;
layout(location = 5) out vec3 g_dir;
layout(location = 6) out vec3 g_modelDir;
layout(location = 7) out vec3 g_color;
layout(location = 8) out vec3 g_origin;


void emitQuadPoint(
    vec4 origin,
    vec4 right,
    float offset,
    vec3 forward,
    vec3 normal,
    vec2 uv,
    int id
) {

    vec4 newPos = origin + right * offset; //Model space
    gl_Position = camera.viewProj * newPos;
    g_dir = normalize(mat3(transpose(inverse(camera.view))) * v_tangent[id]);
    g_modelDir = v_tangent[id];
    g_color = v_color[id];
    g_pos = (camera.view * newPos).xyz;
    g_modelPos = newPos.xyz;
    g_uv = uv;
    g_normal = normalize(mat3(transpose(inverse(camera.view))) * normal);
    g_modelNormal = normal;
    g_origin = (camera.view * origin).xyz;

    EmitVertex();
}

void main() {

        //Model space --->>>

    vec4 startPoint = gl_in[0].gl_Position;
    vec4 endPoint = gl_in[1].gl_Position;

    vec4 view0 = vec4(camera.position.xyz, 1.0) - startPoint;
    vec4 view1 = vec4(camera.position.xyz, 1.0) - endPoint;

    vec3 dir0 = v_tangent[0];
    vec3 dir1 = v_tangent[1];

    vec4 right0 = normalize(vec4(cross(dir0.xyz, view0.xyz), 0.0));
    vec4 right1 = normalize(vec4(cross(dir1.xyz, view1.xyz), 0.0));

    vec3 normal0 = normalize(cross(right0.xyz, dir0.xyz));
    vec3 normal1 = normalize(cross(right1.xyz, dir1.xyz));

        //<<<----

    float halfLength = material.thickness * 0.5;

    emitQuadPoint(startPoint, right0, halfLength, dir0, normal0, vec2(1.0, 0.0), 0);
    emitQuadPoint(endPoint, right1, halfLength, dir1, normal1, vec2(1.0, 1.0), 1);
    emitQuadPoint(startPoint, -right0, halfLength, dir0, normal0, vec2(0.0, 0.0), 0);
    emitQuadPoint(endPoint, -right1, halfLength, dir1, normal1, vec2(0.0, 1.0), 1);

}

#shader fragment
#version 460 core
#include light.glsl
#include scene.glsl
#include camera.glsl
#include object.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include reindhart.glsl
#include BRDFs/marschner_LUT_BSDF.glsl

//Input
layout(location = 0) in vec3 g_pos;
layout(location = 1) in vec3 g_modelPos;
layout(location = 2) in vec3 g_normal;
layout(location = 3) in vec3 g_modelNormal;
layout(location = 4) in vec2 g_uv;
layout(location = 5) in vec3 g_dir;
layout(location = 6) in vec3 g_modelDir;
layout(location = 7) in vec3 g_color;
layout(location = 8) in vec3 g_origin;

//Uniforms
layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 4) uniform samplerCube irradianceMap;


layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;

    float Rpower;
    float TTpower;
    float TRTpower; 
    float scatter;

    bool glints;
    bool useScatter;
    bool coloredScatter;
    bool r;

    bool tt;
    bool trt;
    bool occlusion;
} material;

layout(set = 2, binding = 0) uniform sampler2D nTex1;
layout(set = 2, binding = 1) uniform sampler2D nTex2;
layout(set = 2, binding = 2) uniform sampler3D GITex;
// layout(set = 2, binding = 3) uniform sampler2D mGITex;

MarschnerLookupBSDF bsdf;

layout(location = 0) out vec4 fragColor;


// float computeShadow(LightUniform light, int lightId) {

//     vec4 posLightSpace = light.viewProj * vec4(g_modelPos, 1.0);

//     vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

//     projCoords.xy = projCoords.xy * 0.5 + 0.5;

//     if(projCoords.z > 1.0 || projCoords.z < 0.0)
//         return 1.0;

//     vec3 lightDir = normalize(light.position.xyz - g_pos);
//     float bias = max(light.shadowData.x * 5.0 * (1.0 - dot(g_dir, lightDir)), light.shadowData.x);  //Modulate by angle of incidence

//     // return 1.0 - filterPCF(shadowMap,lightId,int(light.pcfKernel),  light.kernelRadius,projCoords, bias);
//   return 1.0 - filterPCF(shadowMap, lightId,int(light.shadowData.w), light.shadowData.y, projCoords, light.shadowData.x);
// }

float bilinear( float v[4], vec2 f )
	{
		return mix( mix( v[0], v[1], f.x ), mix( v[2], v[3], f.x ), f.y );
	}

vec3 bilinear( vec3 v[4], vec2 f )
	{
		return mix( mix( v[0], v[1], f.x ), mix( v[2], v[3], f.x ), f.y );
	}    
vec3 hairShadow( out vec3 spread, out float directF, vec3 pShad, int lightId )
	{
        float hairDensity = 1.0;
		ivec2 size = textureSize( shadowMap, 0 ).xy;
		vec2 t = pShad.xy * vec2(size) + 0.5;
		vec2 f = t - floor(t);
		vec2 s = 0.5 / vec2(size);
		
		vec2 tcp[4];
		tcp[0] = pShad.xy + vec2( -s.x, -s.y );
		tcp[1] = pShad.xy + vec2(  s.x, -s.y );
		tcp[2] = pShad.xy + vec2( -s.x,  s.y );
		tcp[3] = pShad.xy + vec2(  s.x,  s.y );

		const float coverage = 0.05;
		const vec3 a_f = vec3( 0.507475266, 0.465571405, 0.394347166 );
		const vec3 w_f = vec3( 0.028135575, 0.027669785, 0.027669785 );
		float dir[4];
		vec3 spr[4], t_d[4];
		for ( int i=0; i<4; ++i ) {
			float z = texture(shadowMap, vec3(tcp[i],lightId)).r;
			//vis[i] = z < pShad.z ? 0.0 : 1.0;
			float h = max( 0.0, pShad.z - z );
			float n = h * hairDensity;
			dir[i] = pow( 1.0 - coverage, n );
			t_d[i] = pow( 1.0 - coverage*(1.0 - a_f), vec3(n,n,n) );
			spr[i] = n * coverage * w_f;
		}
		
		directF = bilinear( dir, f );
		spread  = bilinear( spr, f );
		return    bilinear( t_d, f );
	}
vec3 computeShadow(LightUniform light,int lightId, out vec3 spread, out float directF )
	{
          vec4 posLightSpace = light.viewProj * vec4(g_modelPos, 1.0);
         vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z
        projCoords.xy = projCoords.xy * 0.5 + 0.5;
        // if(projCoords.z > 1.0 || projCoords.z < 0.0)
        //     return 1.0;
		// projCoords.z -= light.shadowData.x;
		vec3 trans_direct = hairShadow( spread, directF, projCoords,lightId );
		directF *= 0.5;
		return trans_direct * 0.5;
	}    


vec3 computeAmbient(vec3 n) {

    vec3 ambient;
    if(scene.useIBL){
        float rad = radians(scene.envRotation);
        float c = cos(rad);
        float s = sin(rad);
        mat3 rotationY = mat3(c, 0.0, -s,
                0.0, 1.0, 0.0,
                s, 0.0, c);
        vec3 rotatedNormal = normalize(rotationY * n);

        ambient = evalMarschnerLookupBSDF(
                rotatedNormal, 
                normalize(-g_pos),
                texture(irradianceMap, rotatedNormal).rgb*scene.ambientIntensity,
                bsdf, 
                nTex1,
                nTex2,
                GITex,
                material.r, 
                false,  //Take oput transmitance
                material.trt);


    }else{
        ambient = (scene.ambientIntensity * scene.ambientColor) *  bsdf.baseColor;
    }
    return ambient;
}

void main() {

    //BSDF setup ............................................................
    bsdf.tangent =  normalize(g_dir);
    bsdf.baseColor = material.baseColor;
    bsdf.Rpower = material.Rpower;
    bsdf.TTpower = material.TTpower;
    bsdf.TRTpower = material.TRTpower;


    //DIRECT LIGHTING .......................................................
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i], g_pos)) {

            vec3 shadow = vec3(1.0);
            if(int(object.otherParams.y) == 1 && scene.lights[i].shadowCast == 1) {
                if(scene.lights[i].shadowType == 0) //Classic
                    shadow = computeShadow(scene.lights[i], i,bsdf.spread, bsdf.directFraction);
                if(scene.lights[i].shadowType == 1) //VSM   
                    shadow.x = computeVarianceShadow(shadowMap,scene.lights[i],i,g_modelPos);
            }
            vec3 lighting = evalMarschnerLookupBSDF(
                normalize(scene.lights[i].position.xyz - g_pos), 
                normalize(-g_pos),
                scene.lights[i].color * scene.lights[i].intensity,
                bsdf, 
                nTex1,
                nTex2,
                GITex,
                material.r, 
                material.tt, 
                material.trt);


            color += lighting * shadow;
        }
    }


    // vec3 n1 = cross(g_modelDir, cross(camera.position.xyz, g_modelDir));
    vec3 fakeNormal = normalize(g_modelPos-object.volumeCenter);
    // vec3 fakeNormal = mix(n1,n2,0.5);

    //AMBIENT COMPONENT ..........................................................

    vec3 ambient = computeAmbient(fakeNormal);
    color += ambient;

    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog(gl_FragCoord.z);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }


    fragColor = vec4(color, 1.0);

}