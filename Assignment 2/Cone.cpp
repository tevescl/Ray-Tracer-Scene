/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cone class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/
#include "Cone.h"
#include <math.h>

float Cone::intersect(glm::vec3 pos, glm::vec3 dir)
{
    glm::vec3 d = pos - center;
    float yd = height - pos.y + center.y;
    float tan = (radius / height) * (radius / height);
    float a = (dir.x * dir.x) + (dir.z * dir.z) - (tan * (dir.y * dir.y));
    float b = 2 * (d.x * dir.x + d.z * dir.z + tan * yd * dir.y);
    float c = (d.x * d.x) + (d.z * d.z) - (tan * (yd * yd));
    float delta = (b * b) - 4 * (a * c);

    if (fabs(delta) < 0.001 || delta < 0.0)
        return -1.0;

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float tA = (t1 < 0.01) ? -1 : t1;
    float tB = (t2 < 0.01) ? -1 : t2;

    if (t1 > t2) {
        float temp = tA;
        tA = tB;
        tB = temp;
    }

    float yposA = pos.y + dir.y * tA;
    float yposB = pos.y + dir.y * tB;

    if (yposA >= center.y && yposA <= center.y + height)
        return tA;

    if (yposB >= center.y && yposB <= center.y + height)
        return tB;

    return -1.0;
}



/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cone.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
	glm::vec3 d = p - center;
	float r = sqrt(d.x * d.x + d.z * d.z);
	glm::vec3 n = glm::vec3(d.x, r * (radius / height), d.z);
	n = glm::normalize(n);
	return n;
}