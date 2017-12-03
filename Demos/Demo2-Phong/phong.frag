{
   vec4 fCol = gl_Color;
   vec4 fTex = gl_TexCoord;
   vec4 eyeNorm;
   vec4 coeff;
   vec4 shade;
   const vec4 lVec = env1; 
   const vec4 lHalf = gl_Light_Half; 
   const vec4 red = vec4(1.0,0.0,0.0,1.0);
   eyeNorm = fTex;
   eyeNorm[3] = dp3(eyeNorm,eyeNorm);
   eyeNorm[3] = rsq(eyeNorm[3]);
   eyeNorm = eyeNorm * eyeNorm[3];
   shade = gl_Light_Ambient * fCol;
   coeff[0] = dp3(lVec,eyeNorm);
   coeff[1] = dp3(lHalf,eyeNorm); 
   coeff[3] = gl_Material_Shininess[0];
   coeff = lit(coeff);
   shade = shade + coeff[1] * fCol;
   shade = shade + coeff[2] * fCol;
   gl_FragColor = shade;

}



