/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of SceneObject, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Plane.h"
#include <math.h>
#include <iostream>     // For std::cout, std::cin

/**
* Plane's intersection method.  The input is a ray (p0, dir).
* See slide Lec09-Slide 31
*/
float Plane::intersect(glm::vec3 p0, glm::vec3 dir)
{
	glm::vec3 n = normal(p0);
	glm::vec3 vdif = a_ - p0;
	float d_dot_n = glm::dot(dir, n);
	if(fabs(d_dot_n) < 1.e-4) return -1;   //Ray parallel to the plane

    float t = glm::dot(vdif, n)/d_dot_n;
	if(t < 0) return -1;

	//lab 2 
	glm::vec3 q = p0 + dir * t;				//Point of intersection
	if (isInside(q)) return t;				//Inside the plane
	else return -1;							//Outside
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the plane.
*/
glm::vec3 Plane::normal(glm::vec3 p)
{
	glm::vec3 v1 = c_-b_;
	glm::vec3 v2 = a_-b_;
	glm::vec3 n = glm::cross(v1, v2);
	n = glm::normalize(n);
	//std::cout << "Normal: " << n.x << ", " << n.y << ", " << n.z << std::endl; // Debug normal

    return n;
}

/**
* 
* Checks if a point q is inside the current polygon
* See slide Lec09-Slide 33
*/
bool Plane::isInside(glm::vec3 q)
{
	glm::vec3 n = normal(q);     //Normal vector at the point of intersection
	glm::vec3 ua = b_ - a_, ub = c_ - b_, uc = d_ - c_, ud = a_ - d_;
	glm::vec3 va = q - a_, vb = q - b_, vc = q - c_, vd = q - d_;
	if (nverts_ == 3) uc = a_ - c_;
	float ka = glm::dot(glm::cross(ua, va), n);
	float kb = glm::dot(glm::cross(ub, vb), n);
	float kc = glm::dot(glm::cross(uc, vc), n);
	float kd;
	if (nverts_ == 4)
		kd = glm::dot(glm::cross(ud, vd), n);
	else
		kd = ka;
	if (ka > 0 && kb > 0 && kc > 0 && kd > 0) return true;
	if (ka < 0 && kb < 0 && kc < 0 && kd < 0) return true;
	else return false;
}


//Getter function for number of vertices
int  Plane::getNumVerts()
{
	return nverts_;
}



