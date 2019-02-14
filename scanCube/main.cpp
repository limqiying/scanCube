#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <math.h>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/vvector.h>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

// ----------------------------------------------------------
// Global Variables
// ----------------------------------------------------------
// For rotate cube (degree)
Vector2f rotate_m(0, 0);

// For moving probe
double step = 0;
Vector3f height(1, 0, 0);

// For dragging the view
static double c=M_PI/180.0f;
static int du=90,oldmy=-1,oldmx=-1;
static double r=1.5f,h=0.0f;

// For drawing a cone
Vector3f probe(0, 0, 0);
Vector3f ending(-2, 0, 0);
Vector3f randomPoint(0, 0, 0);
Vector3f orientation(-1, 0, 0);

double spreadness = 25;
double resolution = 1;

Vector3f getRotated(Vector3f original, double angle, Vector3f axis){
    // rotates the original point about the specified axis
    double rad = angle / 180 * 3.1416;
    AngleAxis<float> rot(rad, axis);
    return rot * original;
}

bool rayPlaneIntersection(Vector3f& intersection, Vector3f ray, Vector3f probe,
                            Vector3f normal, Vector3f planePoint) {
    const double temp = normal.dot(ray);
    
    //check if the ray is parallel to the plane
    if (temp == 0) { return false;}
    
    // compute the intersection point
    double t = ((planePoint - probe).dot(normal)) / temp;
    intersection = probe + ray * t;
    return true;
}

bool checkPointInTriangle(Vector3f point, Vector3f p1, Vector3f p2, Vector3f p3){
    Vector3f v0 = p3 - p1;
    Vector3f v1 = p2 - p1;
    Vector3f vp = point - p1;
    
    Matrix2f A;
    A << v0.dot(v0), v0.dot(v1),
        v1.dot(v0), v1.dot(v1);
    
    Vector2f b(vp.dot(v0), vp.dot(v1));
    
    // Use Cramer's Rule to solve Aw = b
    // solve for w1
    Matrix2f A1 = A;
    A1.block<2, 1>(0, 0) = b;   // replaces the first column of A1 by b
    double u = A1.determinant() / A.determinant();
    if (u < 0 || u > 1) { return false ;}
    
    // solve for w2
    Matrix2f A2 = A;
    A2.block<2, 1>(0, 1) = b;   // replaces the second column of A2 by b
    double v = A2.determinant() / A.determinant();
    if (v < 0 || v > 1) { return false ;}
    
    return u + v <= 1 ;
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
    gluLookAt(r*cos(c*du), h, r*sin(c*du), 0, 0, 0, 0, 1, 0);
    
    glPushMatrix();
    
    // Rotate when user changes rotate_x and rotate_y
    glRotatef(rotate_m[0], 1.0, 0.0, 0.0 );
    glRotatef(rotate_m[1], 0.0, 1.0, 0.0 );
    
    // FRONT
    glBegin(GL_POLYGON);
    glColor3f(   0.6,  0.6,  0.6 );
    glVertex3f(  0.3, -0.3, -0.3 );
    glVertex3f(  0.3,  0.3, -0.3 );
    glVertex3f( -0.3,  0.3, -0.3 );
    glVertex3f( -0.3, -0.3, -0.3 );
    glEnd();
    
    // BACK
    glBegin(GL_POLYGON);
    glColor3f(   0.5,  0.5, 0.5 );
    glVertex3f(  0.3, -0.3, 0.3 );
    glVertex3f(  0.3,  0.3, 0.3 );
    glVertex3f( -0.3,  0.3, 0.3 );
    glVertex3f( -0.3, -0.3, 0.3 );
    glEnd();
    
    // RIGHT
    glBegin(GL_POLYGON);
    glColor3f(   0.7,  0.7, 0.7 );
    glVertex3f( 0.3, -0.3, -0.3 );
    glVertex3f( 0.3,  0.3, -0.3 );
    glVertex3f( 0.3,  0.3,  0.3 );
    glVertex3f( 0.3, -0.3,  0.3 );
    glEnd();
    
    // LEFT
    glBegin(GL_POLYGON);
    glColor3f(    0.8,  0.8, 0.8 );
    glVertex3f( -0.3, -0.3,  0.3 );
    glVertex3f( -0.3,  0.3,  0.3 );
    glVertex3f( -0.3,  0.3, -0.3 );
    glVertex3f( -0.3, -0.3, -0.3 );
    glEnd();
    
    // TOP
    glBegin(GL_POLYGON);
    glColor3f(    0.4,  0.4, 0.4 );
    glVertex3f(  0.3,  0.3,  0.3 );
    glVertex3f(  0.3,  0.3, -0.3 );
    glVertex3f( -0.3,  0.3, -0.3 );
    glVertex3f( -0.3,  0.3,  0.3 );
    glEnd();
    
    // BOTTOM
    glBegin(GL_POLYGON);
    glColor3f(    0.3,  0.3, 0.3 );
    glVertex3f(  0.3, -0.3, -0.3 );
    glVertex3f(  0.3, -0.3,  0.3 );
    glVertex3f( -0.3, -0.3,  0.3 );
    glVertex3f( -0.3, -0.3, -0.3 );
    glEnd();
    
    glPopMatrix();
    
    glPushMatrix();
    
    // Define the cube
    Vector3f normals[3];
    Vector3f e0(0, 0, 1);
    Vector3f e1(1, 0, 0);
    Vector3f e2(0, 1, 0);
    normals[0] = e0;
    normals[1] = e1;
    normals[2] = e2;
    
    
    // FRONT: 0, BACK: 1, RIGHT: 2, LEFT: 3, TOP: 4, BOTTOM: 5
    /*
     
     3-------0
     |       |
     |       |
     |       |
     2-------1
     
     */
    
    // 7 vertices defining the cube
    Vector3f p0(0.3, 0.3, 0.3);
    Vector3f p1(0.3, -0.3, 0.3);
    Vector3f p2(-0.3, -0.3, 0.3);
    Vector3f p3(-0.3, 0.3, 0.3);
    Vector3f p4(-0.3, 0.3, -0.3);
    Vector3f p5(0.3, 0.3, -0.3);
    Vector3f p6(0.3, -0.3, -0.3);
    Vector3f p7(-0.3, -0.3, -0.3);
    
    
    //    vector<double> sides[6][4];
    Vector3f sides[6][4];
    
    sides[0][0] = p0 - height;
    sides[0][1] = p1 - height;
    sides[0][2] = p2 - height;
    sides[0][3] = p3 - height;
    
    sides[1][0] = p4 - height;
    sides[1][1] = p7 - height;
    sides[1][2] = p6 - height;
    sides[1][3] = p5 - height;
    
    sides[2][0] = p5 - height;
    sides[2][1] = p6 - height;
    sides[2][2] = p1 - height;
    sides[2][3] = p0 - height;
    
    sides[3][0] = p3 - height;
    sides[3][1] = p2 - height;
    sides[3][2] = p7 - height;
    sides[3][3] = p4 - height;
    
    sides[4][0] = p5 - height;
    sides[4][1] = p0 - height;
    sides[4][2] = p3 - height;
    sides[4][3] = p4 - height;
    
    sides[5][0] = p1 - height;
    sides[5][1] = p6 - height;
    sides[5][2] = p7 - height;
    sides[5][3] = p2 - height;
    
    
    // Define the probes
    //    glTranslatef(height_x, height_y, height_z);
    glTranslatef(height[0], height[1], height[2]);
    
    // draw prob and axis
    glPointSize(7.0f);//set point size to 10 pixels
    glColor3f(1.0,1.0,0.0); //red color
    glBegin(GL_POINTS); //starts drawing of points
    glVertex3f(probe[0],probe[1],probe[2]);
    glEnd();
    glBegin(GL_LINES);
    glLineWidth(4.0);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(probe[0], probe[1], probe[2]);
    glVertex3f(ending[0], ending[1], ending[2]);
    glEnd();
    
    // pick a random normal in 3D space
    Vector3f vec1 = randomPoint - probe;
    Vector3f normal = orientation.cross(vec1);
    normal.normalize();
    
    for(int i = 0; i < 360/resolution; i++){
        
        // draw one line on the cone
        Vector3f conePoint = getRotated(ending, spreadness, normal);
        
        glBegin(GL_LINES);
        glLineWidth(3.0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(probe[0], probe[1], probe[2]);
        glVertex3f(conePoint[0], conePoint[1], conePoint[2]);
        glEnd();
        
        // find intersections
        Vector3f ray = conePoint - probe;
        Vector3f intersection;
        
        // Draw intersection points
        glPointSize(7.0f);//set point size to 10 pixels
        glColor3f(1.0f,0.0f,0.0f); //red color
        int count = 0;
        for (int i = 0; i < 6 && count < 2; i++){
            if(!rayPlaneIntersection(intersection, ray, probe, normals[i/2], sides[i][0])){
                continue;
            }
            if(checkPointInTriangle(intersection, sides[i][0], sides[i][1], sides[i][2])){
                glBegin(GL_POINTS); //starts drawing of points
                glVertex3f(intersection[0],intersection[1],intersection[2]);
                glEnd();//end drawing of points
                float distance = (probe - intersection).norm();
                cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << " " << distance << endl;
                
                count++;
                continue;
            }
            if(checkPointInTriangle(intersection, sides[i][2], sides[i][3], sides[i][0])){
                glBegin(GL_POINTS); //starts drawing of points
                glVertex3f(intersection[0],intersection[1],intersection[2]);
                glEnd();//end drawing of points
                float distance = (probe - intersection).norm();
                cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << " " << distance << endl;
                
                count++;
                continue;
            }
        }
        normal = getRotated(normal, resolution, orientation);
        
    }
    
    glPopMatrix();
    
    glFlush();
    
    glutSwapBuffers();
    
}

// Mouse click action: record old coordinate when click
void Mouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN){
        oldmx = x;
        oldmy = y;
    }
}

// Mouse move action
void onMouseMove(int x,int y)
{
    du += x - oldmx;
    // move left-right
    h +=0.03f*(y-oldmy);
    // move up-down
    if(h>1.0f)
        h=1.0f;
    // limit watch point's y axis
    else
        if(h<-1.0f) h=-1.0f;
    oldmx=x;
    oldmy=y;
    
    // Render to display
    glutPostRedisplay();
}

void init()
{
    glEnable(GL_DEPTH_TEST);
}

void reshape(int w,int h)
{
    glViewport( 0, 0, w, h );
    
    glMatrixMode( GL_PROJECTION );
    
    glLoadIdentity();
    
    gluPerspective(75.0f, (double)w/h, 0.5f, 1000.0f);
    
    glMatrixMode( GL_MODELVIEW );
    
}

void selfMoving() {
    Vector3f next;
    Vector3f A(1.0, 0.5, -0.5);
    Vector3f B(1.0, 0.3, 0.5);
    Vector3f C(1.0, -0.3, -0.5);
    Vector3f D(1.0, -0.5, 0.6);

    next = pow(1-step, 3) * A + 3*step*pow(1-step, 2) * B + 3*pow(step, 2)*(1-step) * C + pow(step, 3) * D;

    step += 0.01;
    
    // Move probe
    height = next;

    // Render to display
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    
    randomPoint = {(double)(rand() % 100 - 50)/50, (double)(rand() % 100 - 50)/50, (double)(rand() % 100 - 50)/50};
    
    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    
    glutInitWindowPosition(100, 100);
    
    glutInitWindowSize(400, 400);
    
    glutCreateWindow("ScanCube");
    
    init();
    
    glutReshapeFunc( reshape );
    
    glutDisplayFunc(display);
    
    glutIdleFunc(selfMoving);
    
    glutMouseFunc(Mouse);
    
    glutMotionFunc(onMouseMove);
    
    glutMainLoop();
    
    return 0;
    
}
