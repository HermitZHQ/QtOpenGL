#version 450 core

layout(local_size_x = 10, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1) buffer calcBuf {
//buffer calcBuf {
    float data[];
}CalcBuf;

layout(binding = 2) buffer outBuf
{
    float data [];
}OutBuf;

void main() {
    //CalcBuf.data[gl_LocalInvocationIndex] = CalcBuf.data[gl_LocalInvocationIndex] * CalcBuf.data[gl_LocalInvocationIndex];
    OutBuf.data[gl_LocalInvocationIndex] = CalcBuf.data[gl_LocalInvocationIndex] * CalcBuf.data[gl_LocalInvocationIndex];
}