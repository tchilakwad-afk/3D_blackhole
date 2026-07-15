#version 430 core
layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 camPos;
uniform vec3 camFront;
uniform vec3 camUp;
uniform vec3 camRight;
uniform float tanHalfFov;
uniform float aspect;

// uniform vec3 sphereCenter0; uniform float sphereRadius0; uniform vec3 sphereColor0;
// uniform vec3 sphereCenter1; uniform float sphereRadius1; uniform vec3 sphereColor1;

uniform vec3 BHCenter;
uniform float BHrs;
uniform float BHrFar;
uniform float dLambda;
uniform int maxGeodesicSteps;

uniform float rkfTolerance;
uniform float hMin;
uniform float hMax;

uniform float stepSize;
uniform int maxSteps;
uniform float maxDist;

uniform samplerCube skybox;

bool intersectSphere(vec3 ro, vec3 rd, vec3 center, float radius, out float outT, out vec3 outNormal){
    vec3 oc = center - ro;
    float tClosest = dot(oc, rd);
    vec3 closestPoint = ro + rd*max(tClosest, 0.0);
    float minDist = length(closestPoint - center);
    if(minDist > radius + stepSize){
        return false;
    }
    if(tClosest < 0.0 && length(oc) > radius){
        return false;
    }
    
    float t = 0.0;
    float prevDist = length(ro - center);
    
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

        if(distToCenter > prevDist && distToCenter > radius*3.0)
            break;
        prevDist = distToCenter;

        t += stepSize;
        if(t > maxDist)
            break;
    }
    return false;
}

const int SIZE = 7;

void geodesicRHSArr(float y[SIZE], out float dy[SIZE]){
    float r = y[0], theta = y[1], phi = y[2];
    float dr = y[3], dtheta = y[4], dphi = y[5], dt = y[6];

    float f = 1.0 - BHrs / r;
    float sinTheta = max(sin(theta), 1e-4);
    float cosTheta = cos(theta);

    dy[0] = dr;
    dy[1] = dtheta;
    dy[2] = dphi;
    //d2r
    dy[3] = -(BHrs*f/(2.0*r*r)*dt*dt) + (BHrs/(2.0*r*r*f)*dr*dr) + r*f*dtheta*dtheta + r*f*sinTheta*sinTheta*dphi*dphi;
    //d2theta
    dy[4] = -(2.0/r)*dr*dtheta + sinTheta*cosTheta*dphi*dphi;
    //d2phi
    dy[5] = -(2.0/r)*dr*dphi - 2.0*(cosTheta/sinTheta)*dtheta*dphi;
    //d2t
    dy[6] = -(BHrs / (r*r*f) * dt * dr);
}

// void geodesicRHS(float r, float theta, float dt, float dr, float dtheta, float dphi, out float d2t, out float d2r, out float d2theta, out float d2phi){
//     float f = 1.0 - BHrs / r;
//     float sinTheta = max(sin(theta), 1e-4);
//     float cosTheta = cos(theta);

//     d2t = -(BHrs / (r*r*f) * dt * dr);

//     d2r = -(BHrs*f/(2.0*r*r)*dt*dt) + (BHrs/(2.0*r*r*f)*dr*dr) + r*f*dtheta*dtheta + r*f*sinTheta*sinTheta*dphi*dphi;

//     d2theta = -(2.0/r)*dr*dtheta + sinTheta*cosTheta*dphi*dphi;

//     d2phi = -(2.0/r)*dr*dphi - 2.0*(cosTheta/sinTheta)*dtheta*dphi;
// }

bool rkf45Step(inout float y[SIZE], inout float h){
    float k1[SIZE], k2[SIZE], k3[SIZE], k4[SIZE], k5[SIZE], k6[SIZE];
    float ytemp[SIZE];

    geodesicRHSArr(y, k1);

    for(int i = 0; i < SIZE; i++)
        ytemp[i] = y[i] + h*(1.0/4.0)*k1[i];
    
    geodesicRHSArr(ytemp, k2);

    for(int i = 0; i < SIZE; i++)
        ytemp[i] = y[i] + h*(3.0/32.0*k1[i] + 9.0/32.0*k2[i]);

    geodesicRHSArr(ytemp, k3);

    for(int i = 0; i < SIZE; i++)
        ytemp[i] = y[i] + h*(1932.0/2197.0*k1[i] - 7200.0/2197.0*k2[i] + 7296.0/2197.0*k3[i]);

    geodesicRHSArr(ytemp, k4);

    for(int i = 0; i < SIZE; i++)
        ytemp[i] = y[i] + h*(439.0/216.0*k1[i] - 8.0*k2[i] + 3680.0/513.0*k3[i] - 845.0/4104.0*k4[i]);

    geodesicRHSArr(ytemp, k5);

    for(int i = 0; i < SIZE; i++)
        ytemp[i] = y[i] + h*(-8.0/27.0*k1[i] + 2.0*k2[i] - 3544.0/2565.0*k3[i] + 1859.0/4104.0*k4[i] - 11.0/40.0*k5[i]);

    geodesicRHSArr(ytemp, k6);

    float y5[SIZE];
    float errorNorm = 0.0;
    for(int i = 0; i < SIZE; i++){
        float y4_i = y[i] + h*(25.0/216.0*k1[i] + 1408.0/2565.0*k3[i] + 2197.0/4104.0*k4[i] - 1.0/5.0*k5[i]);
        y5[i] = y[i] + h*(16.0/135.0*k1[i] + 6656.0/12825.0*k3[i] + 28561.0/56430.0*k4[i] - 9.0/50.0*k5[i] + 2.0/55.0*k6[i]);
        float scale = max(abs(y[i]), 1.0);
        errorNorm = max(errorNorm, abs(y5[i] - y4_i) / scale);
    }

    float safety = 0.9;
    bool accepted = errorNorm <= rkfTolerance;
    float exponent = accepted ? 0.2 : 0.25;
    float hNew = h * safety * pow(rkfTolerance / max(errorNorm, 1e-12), exponent);
    hNew = clamp(hNew, h*0.1, h*8.0);
    hNew = clamp(hNew, hMin, hMax);

    if(accepted){
        for(int i = 0; i < SIZE; i++)
            y[i] = y5[i];
        h = hNew;
        return true;
    }
    else{
        h = hNew;
        return false;
    }
}

// void rk4Step(inout float r, inout float theta, inout float phi, inout float dr, inout float dtheta, inout float dphi, inout float dt, float h){
//     float r0=r, th0=theta, ph0=phi, dr0=dr, dth0=dtheta, dph0=dphi, dt0=dt;

//     float k1t, k1r, k1th, k1ph;
//     geodesicRHS(r0, th0, dt0, dr0, dth0, dph0, k1t, k1r, k1th, k1ph);

//     float r1=r0+0.5*h*dr0, th1=th0+0.5*h*dth0, ph1=ph0+0.5*h*dph0;
//     float dr1=dr0+0.5*h*k1r, dth1=dth0+0.5*h*k1th, dph1=dph0+0.5*h*k1ph, dt1=dt0+0.5*h*k1t;
//     float k2t,k2r,k2th,k2ph;
//     geodesicRHS(r1, th1, dt1, dr1, dth1, dph1, k2t, k2r, k2th, k2ph);

//     float r2=r0+0.5*h*dr1, th2=th0+0.5*h*dth1, ph2=ph0+0.5*h*dph1;
//     float dr2=dr0+0.5*h*k2r, dth2=dth0+0.5*h*k2th, dph2=dph0+0.5*h*k2ph, dt2=dt0+0.5*h*k2t;
//     float k3t, k3r, k3th, k3ph;
//     geodesicRHS(r2, th2, dt2, dr2, dth2, dph2, k3t, k3r, k3th, k3ph);

//     float r3=r0+h*dr2, th3=th0+h*dth2, ph3=ph0+h*dph2;
//     float dr3=dr0+h*k3r, dth3=dth0+h*k3th, dph3=dph0+h*k3ph, dt3=dt0+h*k3t;
//     float k4t, k4r, k4th, k4ph;
//     geodesicRHS(r3, th3, dt3, dr3, dth3, dph3, k4t, k4r, k4th, k4ph);

//     r = r0 + (h/6.0)*(dr0 + 2.0*dr1 + 2.0*dr2 + dr3);
//     theta = th0 + (h/6.0)*(dth0 + 2.0*dth1 + 2.0*dth2 + dth3);
//     phi = ph0 + (h/6.0)*(dph0 + 2.0*dph1 + 2.0*dph2 + dph3);

//     dr = dr0 + (h/6.0)*(k1r + 2.0*k2r + 2.0*k3r + k4r);
//     dtheta = dth0 + (h/6.0)*(k1th + 2.0*k2th + 2.0*k3th + k4th);
//     dphi = dph0 + (h/6.0)*(k1ph + 2.0*k2ph + 2.0*k3ph + k4ph);
//     dt = dt0 + (h/6.0)*(k1t + 2.0*k2t  + 2.0*k3t  + k4t);
// }

bool traceGeodesicRay(vec3 ro, vec3 rd, out vec3 escapeDir){
    vec3 relPos = ro - BHCenter;
    float r = length(relPos);
    float theta = acos(clamp(relPos.y/r, -1.0, 1.0));
    float phi = atan(relPos.z, relPos.x);
    float sinTheta = max(sin(theta), 1e-4);

    vec3 eR = vec3(sin(theta)*cos(phi), cos(theta), sin(theta)*sin(phi));
    vec3 eTheta = vec3(cos(theta)*cos(phi), -sin(theta), cos(theta)*sin(phi));
    vec3 ePhi = vec3(-sin(phi), 0.0, cos(phi));

    float dr = dot(rd, eR);
    float dtheta = dot(rd, eTheta) / r;
    float dphi = dot(rd, ePhi) / (r*sinTheta);
    float f0 = 1.0 - BHrs / r;
    float dt = sqrt(max((dr*dr/f0 + r*r*dtheta*dtheta + r*r*sinTheta*sinTheta*dphi*dphi) / f0, 0.0));

    float y[SIZE];
    y[0]=r; y[1]=theta; y[2]=phi; y[3]=dr; y[4]=dtheta; y[5]=dphi; y[6]=dt;

    float h = dLambda;

    for(int i = 0; i < maxGeodesicSteps; i++){
        if(y[0] <= BHrs*1.01) return false;
        if(y[0] >= BHrFar){
            float st = sin(y[1]), ct = cos(y[1]), sp = sin(y[2]), cp = cos(y[2]);
            vec3 curEr = vec3(st*cp, ct, st*sp);
            vec3 curEtheta = vec3(ct*cp, -st, ct*sp);
            vec3 curEphi = vec3(-sp, 0.0, cp);
            escapeDir = normalize(y[3]*curEr + y[0]*y[4]*curEtheta + y[0]*st*y[5]*curEphi);
            return true;
        }

        // rk4Step(r, theta, phi, dr, dtheta, dphi, dt, dLambda);

        bool stepOk = false;
        for(int retry = 0; retry < 4; retry++){
            if(rkf45Step(y, h)){
                stepOk = true;
                break;
            }
        }
        if(!stepOk){
            h = hMin;
            rkf45Step(y, h);
        }
    }
    float st = sin(y[1]), ct = cos(y[1]), sp = sin(y[2]), cp = cos(y[2]);
    escapeDir = normalize(y[3]*vec3(st*cp, ct, st*sp) + y[0]*y[4]*vec3(ct*cp, -st, ct*sp) + y[0]*st*y[5]*vec3(-sp, 0.0, cp));
    return false;
}

void main(){
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 texSize = imageSize(imgOutput);

    vec2 ndc = (vec2(texelCoord) + 0.5) / vec2(texSize) * 2.0 - 1.0;
    vec3 rayDir = normalize(camFront + camRight*ndc.x*tanHalfFov*aspect + camUp*(-ndc.y)*tanHalfFov);
    vec3 rayOrigin = camPos;

    // bool didHit = false;
    // float closestT = 1e30;
    // vec3 hitNormal = vec3(0.0);
    // vec3 hitColor = vec3(0.0);

    // float t; vec3 n;

    // if(intersectSphere(rayOrigin, rayDir, sphereCenter0, sphereRadius0, t, n)){
    //     if(t < closestT){ closestT = t; hitNormal = n; hitColor = sphereColor0; didHit = true; }
    // }
    // if(intersectSphere(rayOrigin, rayDir, sphereCenter1, sphereRadius1, t, n)){
    //     if(t < closestT){ closestT = t; hitNormal = n; hitColor = sphereColor1; didHit = true; }
    // }
    

    // vec3 outColor;
    // if(didHit){
    //     vec3 lightDir = normalize(vec3(0.6, 0.8, 0.5));
    //     float diff = max(dot(hitNormal, lightDir), 0.0);
    //     float ambient = 0.15;
    //     outColor = hitColor * (ambient + diff * 0.85);
    // } 
    // else {
    //     outColor = texture(skybox, rayDir).rgb;
    // }

    vec3 escapeDir;
    bool escaped = traceGeodesicRay(rayOrigin, rayDir, escapeDir);
    vec3 outColor = escaped ? texture(skybox, escapeDir).rgb : vec3(0.0);

    imageStore(imgOutput, texelCoord, vec4(outColor, 1.0));

}