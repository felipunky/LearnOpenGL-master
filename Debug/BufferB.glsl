#version 330 core
out vec4 fragColor;

uniform int iFrame;
uniform float iTime;
uniform float iTimeDelta;
uniform vec2 iResolution;
uniform vec2 iVel;
uniform vec3 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

const float dx = 0.5;
const float dt = dx * dx * 0.5;
const float siz = 0.2;
const float di = 0.25;
const float alp = ( dx * dx ) / dt;
const float rbe = 1.0 / ( 4.0 + alp );
const float vo = 12.0;

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
    
    uv -= dt * vo * cur( uv ) * dir;
    
    return uv;
    
}

vec2 dif( vec2 uv )
{

    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    vec2 cen = texture( iChannel0, uv ).xy;
    vec2 top = texture( iChannel0, vec2( x, y + ypi ) ).xy;
    vec2 lef = texture( iChannel0, vec2( x - xpi, y ) ).xy;
    vec2 rig = texture( iChannel0, vec2( x + xpi, y ) ).xy;
    vec2 dow = texture( iChannel0, vec2( x, y - ypi ) ).xy;
    
    return ( di * rbe ) * ( top + lef + rig + dow + alp * cen ) * rbe;
    
}

float dis( vec2 uv, vec2 mou )
{

    return length( uv - mou );

}

float cir( vec2 uv, vec2 mou, float r )
{

    float o = smoothstep( r, r - 0.05, dis( uv, mou ) );
    
    return o;

}

vec2 adv( vec2 uv, vec2 p, vec2 mou )
{
    
	vec2 vel = iVel / iResolution;

    // Eulerian.
    vec2 pre = texture( iChannel1, vor( uv ) ).xy;
	if( iMouse.z > 0.5 && cir( p, mou, siz ) > 0.0 ) 
	pre = 3.0 * vel;
    pre = iTimeDelta * dt * pre;
    
    uv -= pre;
    
    return uv;
    
}

vec4 forc( vec2 uv, vec2 p, vec2 mou, sampler2D tex )
{

    vec4 col = vec4( 0 ); 

    if( iMouse.z > 0.5 )
	col += 0.1 * cir( p, mou, siz );
    
    return col;

}

vec2 div( vec2 uv )
{

    float xpi = 1.0 / iResolution.x;
    float ypi = 1.0 / iResolution.y;
    
    float x = uv.x;
    float y = uv.y;
    
    float cen = texture( iChannel0, uv ).a;
    float top = texture( iChannel0, vec2( x, y + ypi ) ).r;
    float lef = texture( iChannel0, vec2( x - xpi, y ) ).r;
    float rig = texture( iChannel0, vec2( x + xpi, y ) ).r;
    float dow = texture( iChannel0, vec2( x, y - ypi ) ).r;
    
    float dX = ( rig - lef ) * 0.5;
    float dY = ( top - dow ) * 0.5;
    
    return vec2( dX, dY );

}

vec2 pre( vec2 uv )
{

    vec2 pre = uv;
    
    uv -= ( di * dx * dx ) * div( uv );
    
    return uv;

}

vec2 vel( vec2 uv )
{
    
    vec2 pr = pre( uv );
    vec2 die = div( uv );
    
    uv += dt * die - pr;
   
    return uv;
    
}

vec4 fin( vec2 uv, vec2 p, vec2 mou )
{

    vec4 col = vec4( 0.0 ); float dam = 1.0; vec4 colO = vec4( 0 ); vec2 pre = uv;
    
    uv = adv( uv, p, mou );
    uv -= dt * iTimeDelta * ( vel( uv ) * dif( uv ) );
    col += forc( uv, p, mou, iChannel0 );
    colO = texture( iChannel0, uv ) + col; 
    dam *= 0.99;
    colO *= dam;
    
    if( pre.y < 0.00 || pre.x < 0.00 || pre.x > 1.0 || pre.y > 1.0 ) colO *= 0.0;
    
    return colO;

}

void main( )
{
    
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 p = gl_FragCoord.xy / iResolution.y;

    vec2 mou = iMouse.xy / iResolution.y;
    
    vec4 colO = fin( uv, p, mou );
    
    fragColor = colO;
    
}