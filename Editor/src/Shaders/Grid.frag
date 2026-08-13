#version 460 core

layout(location = 0) out vec4 FragColor;

// --- camera / screen reconstruction ---
layout (std140, binding = 0) uniform Scene {
    // Camera
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
    vec3 u_CameraPosition;

    vec2 u_ViewportSize;

    // Frame info
    double u_Time;
};

// If your engine uses reversed/zero-to-one depth (glClipControl(..., GL_ZERO_TO_ONE)),
// set this to 1. Default OpenGL clip volume (GL_NEGATIVE_ONE_TO_ONE) is 0.
#define ZERO_TO_ONE_DEPTH 0

// --- grid parameters ---
uniform float uMajorGridDiv;
uniform float uAxisLineWidth;
uniform float uMajorLineWidth;
uniform float uMinorLineWidth;

uniform vec4 uMajorLineColor;
uniform vec4 uMinorLineColor;
uniform vec4 uBaseColor; // set alpha to 0 for a transparent base — that's the fix for issue #1

uniform vec4 uXAxisColor;
uniform vec4 uXAxisDashColor;
uniform vec4 uYAxisColor;
uniform vec4 uYAxisDashColor;
uniform vec4 uZAxisColor;
uniform vec4 uZAxisDashColor;
uniform float uAxisDashScale;
uniform vec4 uCenterColor;

uniform int uAxis; // 0 = X, 1 = Y (default), 2 = Z

vec2 axisComponents(vec3 v)
{
    if (uAxis == 0)      return v.yz;
    else if (uAxis == 2) return v.xy;
    else                  return v.xz;
}

vec3 planeNormalForAxis()
{
    if (uAxis == 0)      return vec3(1.0, 0.0, 0.0);
    else if (uAxis == 2) return vec3(0.0, 0.0, 1.0);
    else                  return vec3(0.0, 1.0, 0.0);
}

void main()
{
    // --- reconstruct a world-space ray for this pixel ---
    vec2 ndc = (gl_FragCoord.xy / u_ViewportSize) * 2.0 - 1.0;

    vec4 nearH = inverse(u_ViewProjectionMatrix) * vec4(ndc, -1.0, 1.0);
    vec4 farH  = inverse(u_ViewProjectionMatrix) * vec4(ndc,  1.0, 1.0);
    vec3 nearPos = nearH.xyz / nearH.w;
    vec3 farPos  = farH.xyz  / farH.w;

    vec3 ro = u_CameraPosition;
    vec3 rd = normalize(farPos - nearPos);

    // --- intersect with the grid plane (passes through the world origin) ---
    vec3 n = planeNormalForAxis();
    float denom = dot(n, rd);
    if (abs(denom) < 1e-6) discard; // ray parallel to the plane (grazing/horizon)

    float t = -dot(n, ro) / denom;
    if (t <= 0.0) discard; // plane is behind the camera for this pixel

    vec3 worldPos = ro + t * rd;

    // --- write correct depth so this composites with existing opaque geometry ---
    vec4 clip = u_ViewProjectionMatrix * vec4(worldPos, 1.0);
    float ndcZ = clip.z / clip.w;
#if ZERO_TO_ONE_DEPTH
    gl_FragDepth = ndcZ;
#else
    gl_FragDepth = ndcZ * 0.5 + 0.5;
#endif

    // --- everything below here is the same grid math as the mesh-based version ---
    float div = max(2.0, round(uMajorGridDiv));
    vec3 cameraCenteringOffset = floor(u_CameraPosition / div) * div;

    vec2 centered = axisComponents(worldPos - cameraCenteringOffset);
    vec2 raw      = axisComponents(worldPos);
    vec4 vUV = vec4(centered.y, centered.x, raw.y, raw.x);

    vec4 uvDDXY = vec4(dFdx(vUV.xy), dFdy(vUV.xy));
    vec2 uvDeriv = vec2(length(uvDDXY.xz), length(uvDDXY.yw));

    float axisLineWidth = max(uMajorLineWidth, uAxisLineWidth);
    vec2 axisDrawWidth = max(vec2(axisLineWidth), uvDeriv);
    vec2 axisLineAA = uvDeriv * 1.5;
    vec2 axisLines2 = smoothstep(axisDrawWidth + axisLineAA, axisDrawWidth - axisLineAA, abs(vUV.zw * 2.0));
    axisLines2 *= clamp(axisLineWidth / axisDrawWidth, 0.0, 1.0);

    vec2 majorUVDeriv = uvDeriv / div;
    float majorLineWidth = uMajorLineWidth / div;
    vec2 majorDrawWidth = clamp(vec2(majorLineWidth), majorUVDeriv, vec2(0.5));
    vec2 majorLineAA = majorUVDeriv * 1.5;
    vec2 majorGridUV = 1.0 - abs(fract(vUV.xy / div) * 2.0 - 1.0);
    vec2 majorAxisOffset = (1.0 - clamp(abs(vUV.zw / div * 2.0), 0.0, 1.0)) * 2.0;
    majorGridUV += majorAxisOffset;
    vec2 majorGrid2 = smoothstep(majorDrawWidth + majorLineAA, majorDrawWidth - majorLineAA, majorGridUV);
    majorGrid2 *= clamp(majorLineWidth / majorDrawWidth, 0.0, 1.0);
    majorGrid2 = clamp(majorGrid2 - axisLines2, 0.0, 1.0);
    majorGrid2 = mix(majorGrid2, vec2(majorLineWidth), clamp(majorUVDeriv * 2.0 - 1.0, 0.0, 1.0));

    float minorLineWidth = min(uMinorLineWidth, uMajorLineWidth);
    bool minorInvertLine = minorLineWidth > 0.5;
    float minorTargetWidth = minorInvertLine ? 1.0 - minorLineWidth : minorLineWidth;
    vec2 minorDrawWidth = clamp(vec2(minorTargetWidth), uvDeriv, vec2(0.5));
    vec2 minorLineAA = uvDeriv * 1.5;
    vec2 minorGridUV = abs(fract(vUV.xy) * 2.0 - 1.0);
    minorGridUV = minorInvertLine ? minorGridUV : 1.0 - minorGridUV;
    vec2 minorMajorOffset = (1.0 - clamp((1.0 - abs(fract(vUV.zw / div) * 2.0 - 1.0)) * div, 0.0, 1.0)) * 2.0;
    minorGridUV += minorMajorOffset;
    vec2 minorGrid2 = smoothstep(minorDrawWidth + minorLineAA, minorDrawWidth - minorLineAA, minorGridUV);
    minorGrid2 *= clamp(minorTargetWidth / minorDrawWidth, 0.0, 1.0);
    minorGrid2 = clamp(minorGrid2 - axisLines2, 0.0, 1.0);
    minorGrid2 = mix(minorGrid2, vec2(minorTargetWidth), clamp(uvDeriv * 2.0 - 1.0, 0.0, 1.0));
    minorGrid2 = minorInvertLine ? 1.0 - minorGrid2 : minorGrid2;
    minorGrid2 = mix(vec2(0.0), minorGrid2, vec2(greaterThan(abs(vUV.zw), vec2(0.5))));

    float minorGrid = mix(minorGrid2.x, 1.0, minorGrid2.y);
    float majorGrid = mix(majorGrid2.x, 1.0, majorGrid2.y);

    vec2 axisDashUV = abs(fract((vUV.zw + axisLineWidth * 0.5) * uAxisDashScale) * 2.0 - 1.0) - 0.5;
    vec2 axisDashDeriv = uvDeriv * uAxisDashScale * 1.5;
    vec2 axisDash = smoothstep(-axisDashDeriv, axisDashDeriv, axisDashUV);
    axisDash = mix(vec2(1.0), axisDash, vec2(lessThan(vUV.zw, vec2(0.0))));

    vec4 aAxisColor, bAxisColor, aAxisDashColor, bAxisDashColor;
    if (uAxis == 0)
    {
        aAxisColor = uYAxisColor;     bAxisColor = uZAxisColor;
        aAxisDashColor = uYAxisDashColor; bAxisDashColor = uZAxisDashColor;
    }
    else if (uAxis == 2)
    {
        aAxisColor = uXAxisColor;     bAxisColor = uYAxisColor;
        aAxisDashColor = uXAxisDashColor; bAxisDashColor = uYAxisDashColor;
    }
    else
    {
        aAxisColor = uXAxisColor;     bAxisColor = uZAxisColor;
        aAxisDashColor = uXAxisDashColor; bAxisDashColor = uZAxisDashColor;
    }

    aAxisColor = mix(aAxisDashColor, aAxisColor, axisDash.y);
    bAxisColor = mix(bAxisDashColor, bAxisColor, axisDash.x);
    aAxisColor = mix(aAxisColor, uCenterColor, axisLines2.y);

    vec4 axisLines = mix(bAxisColor * axisLines2.y, aAxisColor, axisLines2.x);

    vec4 col = mix(uBaseColor, uMinorLineColor, minorGrid * uMinorLineColor.a);
    col = mix(col, uMajorLineColor, majorGrid * uMajorLineColor.a);
    col = col * (1.0 - axisLines.a) + axisLines;

    if (col.a < 0.001) discard;

    FragColor = col;
}
