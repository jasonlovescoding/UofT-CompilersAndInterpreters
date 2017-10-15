{ 
   ivec4 myVec = ivec4(0,+1,-2,3); /* this is 
										a comment */
   fvec4 myVec2 = ivec4(0,+1.0,-2.0,3);	
   bvec2 myVec3 = bvec2(true, !false);
   vec4 temp;
if (true == true){
   temp[0] = gl_Color[0] * gl_FragCoord[0];
   temp[1] = gl_Color[1] * gl_FragCoord[1];
   temp[2] = gl_Color[2];
   temp[3] = gl_Color[3] * gl_FragCoord[0] * gl_FragCoord[1];
}
else{
   temp = gl_Color;
}
  gl_FragColor = temp;

}

