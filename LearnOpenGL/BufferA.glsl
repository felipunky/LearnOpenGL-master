#version 330 core
out vec4 fragColor;

uniform float iTime;
uniform float iTimeDelta;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform vec2 iVel;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

const float dx = 0.5;
const float dt = dx * dx * 0.5;
const int ITER = 1;
const float siz = 0.2;
const float di = 0.5;
const float alp = ( dx * dx ) / dt;
const float rbe = 1.0 / ( 4.0 + alp );
const float vo = 102.0;
const int FIELD = 1;
const float vf = 0.005;//0.005;
const float mul = 20.0;
const float e = 0.0025;//1e-3;//0.005;//1e-2;//0.005;
const float pres = 0.01;

//2D Vector field visualizer by nmz (twitter: @stormoid)

/*
	There is already a shader here on shadertoy for 2d vector field viz, 
	but I found it to be hard to use so I decided to write my own.
	Heavily modified by me to make it work as an interactive vector field 
	for my fluid sim.
*/

const float arrow_density = 0.2;
const float arrow_length = 0.95;

const vec3 luma = vec3(0.2126, 0.7152, 0.0722);

float segm(in vec2 p, in vec2 a, in vec2 b) //from iq
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa,ba)/dot(ba,ba), 0., 1.);
	return length(pa - ba*h)*20.*arrow_density;
}

float cur( vec2 uv )
{
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    float top = texture( iChannel0, vec2( x, y + ypi ) ).r;
    float lef = texture( iChannel0, vec2( x - xpi, y ) ).r;
    float rig = texture( iChannel0, vec2( x + xpi, y ) ).r;
    float dow = texture( iChannel0, vec2( x, y - ypi ) ).r;
    
    float dY = ( top - dow ) * 0.5;
    float dX = ( rig - lef ) * 0.5;
    
    return dX * dY;
}

vec2 vor( vec2 uv )
{
    
    vec2 pre = uv;
    
    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;

    vec2 dir = vec2( 0 );
    dir.y = ( cur( vec2( x, y + ypi ) ) ) - ( cur( vec2( x, y - ypi ) ) );
    dir.x = ( cur( vec2( x + xpi, y ) ) ) - ( cur( vec2( x - xpi, y ) ) );
    
    dir = normalize( dir );
    
    if( length( dir ) > 0.0 )
    
    uv -= dt * iTimeDelta * vo * cur( uv ) * dir;
    
    return uv;
    
}

void main()
{
	vec2 p = gl_FragCoord.xy / iResolution.y;
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 mou = iMouse.xy / iResolution.y;
    p *= mul;
    mou *= mul;
    
    float fO = 0.0;
    fO += texture( iChannel1, vor( uv ) ).r + texture( iChannel1, vor( uv ) ).g + texture( iChannel1, vor( uv ) ).b;
   	fO *= 0.333;
	vec2 vel = iVel / iResolution.xy;
    
    vec2 ep = vec2( e, 0 );
    vec2 rz= vec2( 0 );
	//vel *= 0.01;
	for( int i = 0; i < FIELD; i++ )
	{
        float t0 = 0.0, t1 = 0.0, t2 = 0.0;
		//if( length( uv - mou ) < siz )
		t0 += texture( iChannel0, uv ).a * dt * vf;
		t1 += texture( iChannel0, uv + ep.xy ).a * dt * vf;
		t2 += texture( iChannel0, uv + ep.yx ).a * dt * vf;
        vec2 g = vec2( ( t1 - t0 ), ( t2 - t0 ) ) / ep.xx;
		vec2 t = vec2( -g.y, g.x );
        
        p += 0.9 * t + g * 0.3;
        rz += t;
	}
    
    vec2 fld = rz;
    
    vec3 col = sin(vec3(-.3,0.1,0.5)+fld.x-fld.y)*0.65+0.35;
    col = mix(col,vec3(fld.x,-fld.x,fld.y),smoothstep(0.75,1.,sin(0.4)))*0.85;
    float fviz = 0.0;
    
    vec2 ip = floor(p*arrow_density)/arrow_density + .5/arrow_density;   
    vec2 t = rz;
    float m = pow(length(t),0.5)*(arrow_length/arrow_density);
    vec2 b = normalize(t)*m;
    float r = segm(p, ip, ip+b);
    vec2 prp = (vec2(-b.y,b.x));
    r = min(r,segm(p, ip+b, ip+b*0.65+prp*0.3));
    r = clamp(min(r,segm(p, ip+b, ip+b*0.65-prp*0.3)),0.,1.);
    fviz = r;
    
    col = max(vec3(0.0), 1.-fviz*vec3(1.));    
    
    float o = 0.0;
    
    o = texture( iChannel0, uv ).a * 0.99;
	fO += o;

	float ra = siz * mul;

	if( iMouse.z > 0.5 && smoothstep( ra, ra - 0.05, length( p - mou ) ) > 0.0 ) 
	fld = 3.0 * vel;

    if( uv.y < 0.00 || uv.x < 0.00 || uv.x > 1.0 || uv.y > 1.0 ) o *= 0.0;
    
    fragColor = vec4( 0, fld, fO );
    
}