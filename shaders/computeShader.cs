#version 430 core
layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float tanHalfFov;
uniform float aspect;

uniform vec3 sphereCenter0; uniform float sphereRadius0; uniform vec3 sphereColor0;
uniform vec3 sphereCenter1; uniform float sphereRadius1; uniform vec3 sphereColor1;

uniform float stepSize;
uniform int maxSteps;
uniform float maxDist;

bool intersectSphere(vec3 ro, vec3 rd, vec3 center, float radius, out float outT, out vec3 outNormal){
    float t = 0.0;
    
    for(int i = 0; i < maxSteps; i++){
        vec3 p = ro + rd*t;
        float distToCenter = length(p - center);

        if(distToCenter <= radius){
            float tLo = t - stepSize;
            float tHi = t;
            for(int j = 0; j < 8; j++){
                float tMid = 0.5*(tLo + tHi);
                vec3 pMid = ro + rd*tMid;
                if(length(pMid - center) <= radius)
                    tHi = tMid;
                else
                    tLo = tMid;
            }
            outT = tHi;
            outNormal = normalize((ro + rd * outT) - center);
            return true;
        }
        t += stepSize;
        if(t > maxDist)
            break;
    }
    return false;
}

void main(){
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 texSize = imageSize(imgOutput);

    vec2 ndc = (vec2(texelCoord) + 0.5) / vec2(texSize) * 2.0 - 1.0;
    vec3 rayDir = normalize(camFront + camRight*ndc.x*tanHalfFov*aspect + camUp*(-ndc.y)*tanHalfFov);
    vec3 rayOrigin = camPos;

    bool didHit = false;
    float closestT = 1e30;
    vec3 hitNormal = vec3(0.0);
    vec3 hitColor = vec3(0.0);

    float t; vec3 n;

    if(intersectSphere(rayOrigin, rayDir, sphereCenter0, sphereRadius0, t, n)){
        if(t < closestT){ closestT = t; hitNormal = n; hitColor = sphereColor0; didHit = true; }
    }
    if(intersectSphere(rayOrigin, rayDir, sphereCenter1, sphereRadius1, t, n)){
        if(t < closestT){ closestT = t; hitNormal = n; hitColor = sphereColor1; didHit = true; }
    }

    vec3 outColor;
    if(didHit){
        vec3 lightDir = normalize(vec3(0.6, 0.8, 0.5));
        float diff = max(dot(hitNormal, lightDir), 0.0);
        float ambient = 0.15;
        outColor = hitColor * (ambient + diff * 0.85);
    } else {
        float sky = 0.5 * (rayDir.y + 1.0);
        outColor = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), sky);
    }

    imageStore(imgOutput, texelCoord, vec4(outColor, 1.0));
}