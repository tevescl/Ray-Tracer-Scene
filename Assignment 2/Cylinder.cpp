/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cylinder class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/
#include "Cylinder.h"
#include <math.h>


/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) {
    glm::vec3 d = p0 - center;
    float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * (dir.x * d.x + dir.z * d.z);
    float c = (d.x * d.x) + (d.z * d.z) - (radius * radius);

    float delta = b * b - 4 * a * c;

    if (delta < 0.0) return -1.0;

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    if (t1 < 0.01) t1 = -1;
    if (t2 < 0.01) t2 = -1;

    float tA = (t1 > t2) ? t2 : t1;
    float tB = (t1 > t2) ? t1 : t2;

    float ypos = p0.y + dir.y * tA;
    if (ypos >= center.y && ypos <= center.y + height) return tA;

    ypos = p0.y + dir.y * tB;
    if (ypos >= center.y && ypos <= center.y + height) return tB;

    return -1.0;
}


/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
	glm::vec3 d = p - center;
	glm::vec3 n = glm::vec3(d.x, 0, d.z);
	n = glm::normalize(n); 
	return n;
}