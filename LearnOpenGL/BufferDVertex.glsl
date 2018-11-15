#version 330 core
layout( location = 0 ) in vec4 aPos; // the position variable has attribute position 0
//layout( location = 2 ) in vec2 aTexCoord; // the position variable has attribute position 0

out vec4 vertexColour; // specify a color output to the fragment shader

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

void main()
{
    
	/*
	vec2 fragCoord = aTexCoord + 0.5;
	vec2 uv = fragCoord;                 
	//uv += iResolution;
	*/

	vec4 transformedPos = aPos;
	transformedPos.xy /= transformedPos.w;
	transformedPos.xy = transformedPos.xy * 0.5f + 0.5f;
	//transformedPos.xy *= iResolution;

	vec2 fld = texture2D( iChannel1, transformedPos.xy ).yz;
	//gl_Position = vec4( aPos.x * fld.x, aPos.y * fld.y, aPos.z, 1.0 ); // see how we directly give a vec3 to vec4's constructor
	vertexColour = vec4( fld, 0.0, 1.0 ); // set the output variable to a dark-red color

	vec2 mou = iMouse / iResolution;

	gl_PointSize = 5.0;
	//gl_Position = vec4( sin( aPos.x + iTime ), cos( aPos.y + iTime ), aPos.z, 1.0 );
	gl_Position = vec4( fld + aPos.xy, aPos.z, 1 );

}