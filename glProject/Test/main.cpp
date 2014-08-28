/*
 * GLUT Shapes Demo
 *
 * Written by Nigel Stewart November 2003
 *
 * This program is test harness for the sphere, cone
 * and torus shapes in GLUT.
 *
 * Spinning wireframe and smooth shaded shapes are
 * displayed until the ESC or q key is pressed.  The
 * number of geometry stacks and slices can be adjusted
 * using the + and - keys.
 */

#define GLEW_STATIC

#include <stdio.h>
#include <stdlib.h>
#include <gl\glew.h>
#include <gl\glut.h>
#include "BImporter.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtx\transform.hpp>
#include "BCamera.h"

//#pragma comment(lib, "SOIL.lib")

static int slices = 16;
static int stacks = 16;
BGameObject * gameObject;
BCamera * camera;

#define OFFSET(i) (void*)NULL + i

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

static void display(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3d(1,0,0);
	glLoadIdentity();

	if(gameObject != NULL)
	{
		//glPushMatrix();
		//glTranslated(-0.4,0.2,0.0);
		//glScalef(10, 10, 10);
		gameObject->render();
		//glPopMatrix();
	}
	
    glutSwapBuffers();
	//glutPostRedisplay();
}


static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;

        case '+':
            slices++;
            stacks++;
            break;

        case '-':
            if (slices>3 && stacks>3)
            {
                slices--;
                stacks--;
            }
            break;
    }

    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };


void initGlew()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Cant initialize GLEW");
	}
}

int main(int argc, char *argv[])
{
	gameObject = NULL;
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	
    glutCreateWindow("GLUT Shapes");
    //glutReshapeFunc(resize);
    //glutDisplayFunc(display);
    //glutKeyboardFunc(key);
    glutIdleFunc(display);
	
	initGlew();
	
    glClearColor(0,0,0,1);
	/*
	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	//gluLookAt(0,0,-5,0,0,0,0,1,0);
	*/
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CW); // GL_CCW for counter clock-wise

	camera = new BCamera();

	//glm::mat4 ViewMatrix = glm::translate(glm::vec3(-3.0f, 0.0f ,0.0f));
	glm::mat4x4 lProjectionMatrix = glm::perspective<GLfloat>(45.0f, 4.0f/3.0f, 0.1f, 10000.0f);
	camera->setViewMatrix( glm::lookAt<GLfloat>(glm::vec3(4,3,300), glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) );

	BImporter * lImporter = new BImporter();
	lImporter->initImporter();

	gameObject = BGameObject::createRootScene();
	BGameObject * lShuricen = lImporter->loadObject("C://Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/Test/Models/asd.FBX");
	BGameObject * lShuricen1 = lImporter->loadObject("C://Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/Test/Models/ground.FBX");
	BGameObject * lShuricen2 = lImporter->loadObject("C://Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/Test/Models/Wood_House.fbx");
	//BGameObject * lHumanoid = lImporter->loadObject("C://Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/humanoid.fbx");
	gameObject->addChild(lShuricen);
	gameObject->addChild(lShuricen1);
	gameObject->addChild(lShuricen2);
	//gameObject->addChild(lHumanoid);
	//TODO: camera matrix should be passed in another way
	gameObject->setProjectionMatrix(lProjectionMatrix);
	gameObject->setViewMatrix(camera->getViewMatrix());

	//lImporter->freeImporter();

    glutMainLoop();
    return 0;
}
