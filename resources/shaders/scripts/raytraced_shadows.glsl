float computeRaytracedShadow(accelerationStructureEXT TLAS, vec3 origin, vec3 direction, float max){

    rayQueryEXT rayQuery;
    /*rayQueryEXT rayQuery, accelerationStructureEXT topLevel,uint rayFlags, uint cullMask,vec3 origin,float tMin, vec3 direction, float tMax*/
	rayQueryInitializeEXT(rayQuery, TLAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, 0.01, direction, max); 
	// Traverse the acceleration structure and store information about the first intersection (if any)
	rayQueryProceedEXT(rayQuery);
	// If the intersection has hit a triangle, the fragment is shadowed
    float shadow = 1.0;
	if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) {
		shadow = 0.0;
	}

    return shadow;

}
