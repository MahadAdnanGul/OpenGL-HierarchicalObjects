#pragma once
#include <GL/glut.h>

struct Quaternion
{
    double x, y, z, w;
};

struct Matrix4x4 {
    double m[4][4];
};


Quaternion eulerToQuaternion(double roll, double pitch, double yaw);

GLfloat* quaternionToMatrix(Quaternion q);

GLfloat* eulerToMatrix(double roll, double pitch, double yaw);