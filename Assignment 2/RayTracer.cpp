/*==================================================================================
* COSC 363  Computer Graphics
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* RayTracing Assignment 
* 
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Cylinder.h"
#include "Sphere.h"
#include "Cone.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "TextureBMP.h"
#include <glm/gtc/matrix_transform.hpp>


#define M_PI 3.14159265358979323846

using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
bool fogSet = false;

TextureBMP texture;
TextureBMP texture2;
vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(-15., 10., -10.);					//Light's position
	glm::vec3 color(0);
	glm::vec3 ambientLight(0.2f);
	glm::vec3 specular(0);
	SceneObject* obj;
	float f = 20.0;



	ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
	if (ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found


	color = obj->lighting(lightPos, -ray.dir, ray.hit);		//Object's colour using PHONG = doesn't apply to diff colors 
	glm::vec3 normalVec = sceneObjects[ray.index]->normal(ray.hit);

	glm::vec3 lightVec = lightPos - ray.hit;
	float lightDist = glm::length(lightVec);
	lightVec = glm::normalize(lightVec);


	//---------------------Checkered Floor---------------------------------------
	if (ray.index == 0)
	{
		int stripeWidth = 5;
		int iz = (-ray.hit.z+20.f) / stripeWidth;
		int ix = (-ray.hit.x + 20.f) / stripeWidth;
		int k = iz % 2;								// 2 colors 
		int l = ix % 2;
		if ((!k && !l) || (k && l)) {

			color = glm::vec3(0.85, 0.73, 1.0);
		}
		else {
			color =  glm::vec3(1.0, 0.95, 0.6);

		}

	}
	//---------------------Earth Texture---------------------------------------
	if (ray.index == 8)
	{
		glm::vec3 center(-2.5, -3.0, -70.);
		glm::vec3 d = glm::normalize(ray.hit - center);
		float u = 0.5f + ((atan2(d.z, d.x)) / (2 * M_PI));
		float v = 0.5f + (asin(d.y) / M_PI);
		color = texture.getColorAt(u, v);
	}
	//---------------------Cylinder Texture---------------------------------------
	if (ray.index == 6)
	{
		glm::vec3 center(-13., -15., -68.);
		glm::vec3 d = glm::normalize(ray.hit - center);
		float u = 0.5f + ((atan2(d.z, d.x)) / (2 * M_PI));
		float v = d.y;
		color = texture2.getColorAt(u, v);
	}


	//---------------------lab Reflectivity---------------------------------------
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	//---------------------Transparent---------------------------------------
	if (obj->isTransparent() && step < MAX_STEPS)
	{
		float coeff = obj->getTransparencyCoeff();
		Ray transRay(ray.hit, ray.dir);
		glm::vec3 tracedColor = trace(transRay, step + 1);
		color = (color * (1 - coeff)) + (coeff * tracedColor);
	}


	//---------------------Refraction---------------------------------------
	if (obj->isRefractive() && step < MAX_STEPS)
	{
		float eta = 1.0f / obj->getRefractiveIndex();
		float coeff = obj->getRefractionCoeff();

		glm::vec3 g = glm::refract(ray.dir, normalVec,eta);
		Ray refrRay(ray.hit, g);
		refrRay.closestPt(sceneObjects);
		glm::vec3 m = obj->normal(refrRay.hit);
		glm::vec3 h	 = glm::refract(g, -m, eta);
		Ray refrRay2(refrRay.hit, h);
		glm::vec3 refrColor = trace(refrRay2, step + 1);
		color = (color * (1 - coeff)) + (coeff * refrColor);
	}

	

	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	SceneObject* shadowObj = sceneObjects[shadowRay.index];
	if ((shadowRay.index > -1) && (shadowRay.dist < lightDist))
	{
		if (shadowObj->isTransparent() || shadowObj->isRefractive())
		{
			color = 0.6f * color;				
		}
		else {
			color = 0.2f * color;				


		}
	}
	//---------------------Fog---------------------------------------
	if (fogSet == true  && step < MAX_STEPS)
	{
		float z1 = -60.0f;
		float z2 = -131.0f;
		float lam = fmod(((ray.hit.z) - z1) / (z2 - z1), 1.0);
		color = ((1 - lam) * color) + (lam * (glm::vec3(1.0, 1.0, 1.0)));

	}
	return color;
}

glm::vec3 antiAliasing(float xp, float yp, glm::vec3 eye, float pixelSize)
{
	float quarter = pixelSize * 0.25f;
	float threeQuarter = pixelSize * 0.75f;
	glm::vec3 colorSum(0);
	glm::vec3 avg(0.25);

	glm::vec2 quarters[4]{
		glm::vec2(quarter, quarter),
		glm::vec2(quarter, threeQuarter),
		glm::vec2(threeQuarter, quarter),
		glm::vec2(threeQuarter, threeQuarter),
	};
	for (int i = 0; i < 4; i++) {
		Ray ray = Ray(eye, glm::vec3(xp + quarters[i].x, yp + quarters[i].y, -EDIST));
		ray.normalize();
		colorSum += trace(ray, 1);
	}
	colorSum *= avg;

	return colorSum;
}


//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j * cellY;

			glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

			Ray ray = Ray(eye, dir);

		
			ray.normalize();				
			glm::vec3 col = antiAliasing(xp, yp, eye, cellX);

			
			/*glm::vec3 col = trace(ray, 1); */

			

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}
void drawBox()
{
	//-- add box 

	glm::vec3 A(-20., -15., 10.);
	glm::vec3 B(20., -15., 10.);
	glm::vec3 C(20., -15., -130.);
	glm::vec3 D(-20., -15., -130.);

	glm::vec3 E(-20., 25., 10.);
	glm::vec3 F(20., 25., 10.);
	glm::vec3 G(20., 25., -130.);
	glm::vec3 H(-20., 25., -130.);

	Plane* floor = new Plane(A, B, C, D); // checkered floor
	Plane* left = new Plane(A, D, H, E);
	Plane* right = new Plane(C, B, F, G);
	Plane* top = new Plane(F, E, H, G);
	Plane* back = new Plane(D, C, G, H);
	Plane* front = new Plane(B, A, E, F);


	//--sets wall color
	left->setColor(glm::vec3(1.0, 0., 0.));
	right->setColor(glm::vec3(1.0, 1.0, 0.));
	back->setColor(glm::vec3(0, 1., 0.));
	front->setColor(glm::vec3(1, 0, 1.));
	top->setColor(glm::vec3(0, 1.0, 1.));
	//floor->setColor(glm::vec3(1.0, 1.0, 0.));

	//--turn off specularity for all 
	left->setSpecularity(false);
	right->setSpecularity(false);
	back->setSpecularity(false);
	front->setSpecularity(false);
	top->setSpecularity(false);

	sceneObjects.push_back(floor);	//0 
	sceneObjects.push_back(left);	//1
	sceneObjects.push_back(right);	//2	
	sceneObjects.push_back(back);	//3
	sceneObjects.push_back(front);	//4
	sceneObjects.push_back(top);	//5

}
void tableLegs()
{
	Cylinder* cylinder = new Cylinder(glm::vec3(-8., -15., -65.), 0.5, 10.);
	Cylinder* cylinder2 = new Cylinder(glm::vec3(8., -15., -65.), 0.5, 10.);
	Cylinder* cylinder3 = new Cylinder(glm::vec3(8., -15., -75.), 0.5, 10.);
	Cylinder* cylinder4 = new Cylinder(glm::vec3(-8., -15., -75.), 0.5, 10.);
	cylinder->setColor(glm::vec3(1, 0, 1)); //11
	cylinder2->setColor(glm::vec3(1, 0, 1)); //12
	cylinder3->setColor(glm::vec3(1, 0, 1)); //13
	cylinder4->setColor(glm::vec3(1, 0, 1)); //14


	sceneObjects.push_back(cylinder);
	sceneObjects.push_back(cylinder2);
	sceneObjects.push_back(cylinder3);
	sceneObjects.push_back(cylinder4);

}


//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

	//--texture
	texture = TextureBMP("Earth.bmp");
	texture2 = TextureBMP("Butterfly.bmp");



	glClearColor(0, 0, 0, 1);

	//obj to 5
	drawBox();//floor is 0 
	

	//////--cylinder texturing 
	Cylinder* cylinderTex = new Cylinder(glm::vec3(-13., -15., -68.), 3.0, 5.0);
	cylinderTex->setColor(glm::vec3(0, 0, 1)); 

	sceneObjects.push_back(cylinderTex); //6


	//-----spheres------- 
	Sphere* sphere1 = new Sphere(glm::vec3(-7.5, -3.0, -65.), 2.0); //transparent  //7
	Sphere* sphere2 = new Sphere(glm::vec3(-2.5, -3.0, -70.), 2.0); //8
	Sphere* sphere3 = new Sphere(glm::vec3(2.5, -3.0, -70.), 2.0); //9
	Sphere* sphere4 = new Sphere(glm::vec3(7.0, -3.0, -65.), 2.0); //10

	sphere1->setColor(glm::vec3(0, 1, 1));
	sphere2->setColor(glm::vec3(0, 0, 0));
	sphere3->setColor(glm::vec3(0, 0, 0));
	sphere4->setColor(glm::vec3(0, 0, 0));

	//--sphere attributes
	sphere1->setTransparency(true, 0.8f);
	sphere3->setRefractivity(true, 0.65f, 1.5f); 
	sphere4->setReflectivity(true, 0.8f);


	sceneObjects.push_back(sphere1);	
	sceneObjects.push_back(sphere2);	
	sceneObjects.push_back(sphere3);	
	sceneObjects.push_back(sphere4); 

	//table == 9 
	Plane* table = new Plane(glm::vec3(-12., -5.0, -60.),
		glm::vec3(12., -5.0, -60.),
		glm::vec3(12., -5.0, -80.),
		glm::vec3(-12., -5.0, -80.));
	table->setColor(glm::vec3(1, 1, 0));

	sceneObjects.push_back(table); //9 

	//-- mirror
	Plane *mirror = new Plane(glm::vec3(-10., -5.0, -85.),
		glm::vec3(10., -5.0, -85.),
		glm::vec3(10., 15., -80.),
		glm::vec3(-10., 15., -80.));
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 0.8f);
	sceneObjects.push_back(mirror); //10

	//--blue cone
	Cone* cone = new Cone(glm::vec3(3.0, -15., -65.), 2.0, 5.0);
	cone->setColor(glm::vec3(0, 1, 1));
	sceneObjects.push_back(cone);  // 6 


	tableLegs();

}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(20, 20);
	glutCreateWindow("Raytracing");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}
