#version 120

attribute vec4 aPos;
attribute vec2 aTex;
attribute vec4 tInd;
uniform mat4 P;
uniform mat4 MV;
uniform vec2 cps[25];
varying vec2 vTex;
varying vec2 cp00;
varying vec2 cp01;
varying vec2 cp10;
varying vec2 cp11;
uniform float nVert;

void main()
{
    
    
    
	gl_Position = P * MV * aPos;
	vTex = aTex;
}
