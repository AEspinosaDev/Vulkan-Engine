float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; 
    float linearZ = (2.0 * near * far) / (far + near - z * (far - near));
    return (linearZ - near) / (far - near);
}

vec3 shiftTangent(vec3 T, vec3 N, float shift) {
    vec3 shiftedT = T + shift * N;
    return normalize(shiftedT);
}

float getLuminance(vec3 li) {
    return 0.2126 * li.r + 0.7152 * li.g + 0.0722 * li.b;
}


vec3 mapRangeToAnOther(vec3 value, vec3 valueMin, vec3 valueMax, vec3 mapMin, vec3 mapMax)
{
    return (value - valueMin) / (valueMax - valueMin) * (mapMax - mapMin) + mapMin;
}

vec3 mapToZeroOne(vec3 value, vec3 rangeMin, vec3 rangeMax)
{
    return mapRangeToAnOther(value, rangeMin, rangeMax, vec3(0.0), vec3(1.0));
}