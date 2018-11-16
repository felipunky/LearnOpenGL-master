#version 330 core
layout( location = 0 ) in vec3 aPos; // the position variable has attribute position 0
//layout( location = 2 ) in vec2 aTexCoord; // the position variable has attribute position 0

out vec4 vertexColour; // specify a color output to the fragment shader

uniform float iTime;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform vec2 iVel;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

const float siz = 0.2;

void main()
{

	vec3 transformedPos = aPos;

	transformedPos.xy = transformedPos.xy * 0.5 + 0.5;

	vec2 uv = transformedPos.xy;

	vec2 fld = texture2D( iChannel1, uv ).yz;

	/*
	uv.x *= iResolution.x / iResolution.y;

	vec2 mou = iMouse.xy / iResolution;

	vec2 col = vec2( 0 );

	if( iMouse.z > 0.5 && smoothstep( siz, siz - 0.05, length( uv - mou ) ) > 0.0 )
	{
	
		col = fld * iVel;
	
	}
	*/

	vertexColour = vec4( fld, 0, 1 );

	gl_PointSize = 1.0;

	vec2 pos = fld + aPos.xy;

	gl_Position = vec4( pos, 0, 1 );

}