#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#include <math.h>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/vvector.h>
#include <vector>
#include <iostream>

using namespace std;

// ----------------------------------------------------------
// Global Variables
// ----------------------------------------------------------
// For rotate cube (degree)
double rotate_y = 0;
double rotate_x = 0;

// For moving probe
double step = 0;
double height_x = 1.0;
double height_y = 0.0;
double height_z = 0.0;

// For dragging the view
static double c=M_PI/180.0f;
static int du=90,oldmy=-1,oldmx=-1;
//du - angle wrt y axis, y is up in OpenGL
static double r=1.5f,h=0.0f;

// For drawing a cone
vector<double> probe = {0.0, 0.0, 0.0};
vector<double> ending = {-2.0, 0.0, 0.0};
vector<double> randomPoint(3,0);
vector<double> orientation = {-1.0, 0.0, 0.0}; // unit vector
double spreadness = 25;
double resolution = 1;

//// random rays
//double lines[100][6];


void getRotated(vector<double>& rotated, vector<double> original, double angle, double axis_x, double axis_y, double axis_z){
    double rad = angle / 180 * 3.1416;
    double c = cosf(rad);
    double s = sinf(rad);
    double rotationMat[3][3] =
    { {pow(axis_x,2)*(1-c)+c, axis_x*axis_y*(1-c)+axis_z*s, axis_x*axis_z*(1-c)-axis_y*s},
        {axis_x*axis_y*(1-c)-axis_z*s, pow(axis_y,2)*(1-c)+c, axis_y*axis_z*(1-c)+axis_x*s},
        {axis_z*axis_x*(1-c)+axis_y*s, axis_y*axis_z*(1-c)-axis_x*s, pow(axis_z,2)*(1-c)+c}
    };
    MAT_DOT_VEC_3X3(rotated, rotationMat, original);
}


bool rayPlaneIntersection(vector<double>& intersection, vector<double> ray, vector<double> probe,
                           vector<double> normal, vector<double> planePoint) {
    double temp;
    VEC_DOT_PRODUCT(temp,normal,ray);
    
    //check if the ray is parallel to the plane
    if (temp == 0) {
        return false;
    }
    
    double t = ((planePoint[0] - probe[0]) * normal[0] + (planePoint[1] - probe[1]) * normal[1] + (planePoint[2] - probe[2]) * normal[2]) / temp;
    intersection[0] = probe[0] + ray[0] * t;
    intersection[1] = probe[1] + ray[1] * t;
    intersection[2] = probe[2] + ray[2] * t;
    
    return true;
}


bool checkPointInTriangle(vector<double> point, vector<double> v1, vector<double> v2, vector<double> v3){
    vector<double> vec0 = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
    vector<double> vec1 = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
    vector<double> vec2 = {point[0] - v1[0], point[1] - v1[1], point[2] - v1[2]};
    
    double dot00, dot01, dot02, dot11, dot12;
    VEC_DOT_PRODUCT(dot00, vec0, vec0);
    VEC_DOT_PRODUCT(dot01, vec0, vec1);
    VEC_DOT_PRODUCT(dot02, vec0, vec2);
    VEC_DOT_PRODUCT(dot11, vec1, vec1);
    VEC_DOT_PRODUCT(dot12, vec1, vec2);
    
    double inverDeno = 1 / (dot00 * dot11 - dot01 * dot01) ;
    
    double u = (dot11 * dot02 - dot01 * dot12) * inverDeno ;
    if (u < 0 || u > 1) // if u out of range, return directly
    {
        return false ;
    }
    
    double v = (dot00 * dot12 - dot01 * dot02) * inverDeno ;
    if (v < 0 || v > 1) // if v out of range, return directly
    {
        return false ;
    }
    
    return u + v <= 1 ;
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    
    gluLookAt(r*cos(c*du), h, r*sin(c*du), 0, 0, 0, 0, 1, 0);
    
    glPushMatrix();
    
    // Rotate when user changes rotate_x and rotate_y
    glRotatef( rotate_x, 1.0, 0.0, 0.0 );
    glRotatef( rotate_y, 0.0, 1.0, 0.0 );
    
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
    vector<double> normals[3];
    normals[0] = {0, 0, 1};
    normals[1] = {1, 0, 0};
    normals[2] = {0, 1, 0};
    
    // FRONT: 0, BACK: 1, RIGHT: 2, LEFT: 3, TOP: 4, BOTTOM: 5
    /*
     
     3-------0
     |       |
     |       |
     |       |
     2-------1
     
    */
    vector<double> sides[6][4];
    
    sides[0][0] = {0.3-height_x, 0.3-height_y, 0.3-height_z};
    sides[0][1] = {0.3-height_x, -0.3-height_y, 0.3-height_z};
    sides[0][2] = {-0.3-height_x, -0.3-height_y, 0.3-height_z};
    sides[0][3] = {-0.3-height_x, 0.3-height_y, 0.3-height_z};
    
    sides[1][0] = {-0.3-height_x, 0.3-height_y, -0.3-height_z};
    sides[1][1] = {-0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[1][2] = {0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[1][3] = {0.3-height_x, 0.3-height_y, -0.3-height_z};
    
    sides[2][0] = {0.3-height_x, 0.3-height_y, -0.3-height_z};
    sides[2][1] = {0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[2][2] = {0.3-height_x, -0.3-height_y, 0.3-height_z};
    sides[2][3] = {0.3-height_x, 0.3-height_y, 0.3-height_z};
    
    sides[3][0] = {-0.3-height_x, 0.3-height_y, 0.3-height_z};
    sides[3][1] = {-0.3-height_x,-0.3-height_y, 0.3-height_z};
    sides[3][2] = {-0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[3][3] = {-0.3-height_x, 0.3-height_y, -0.3-height_z};
    
    sides[4][0] = {0.3-height_x, 0.3-height_y, -0.3-height_z};
    sides[4][1] = {0.3-height_x, 0.3-height_y, 0.3-height_z};
    sides[4][2] = {-0.3-height_x, 0.3-height_y, 0.3-height_z};
    sides[4][3] = {-0.3-height_x, 0.3-height_y, -0.3-height_z};
    
    sides[5][0] = {0.3-height_x, -0.3-height_y, 0.3-height_z};
    sides[5][1] = {0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[5][2] = {-0.3-height_x, -0.3-height_y, -0.3-height_z};
    sides[5][3] = {-0.3-height_x, -0.3-height_y, 0.3-height_z};
    
    
    // Define the probes
    glTranslatef(height_x, height_y, height_z);
    
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
    vector<double> vec1(3, 0);
    VEC_DIFF(vec1, randomPoint, probe)
    vector<double> normal(3, 0);
    VEC_CROSS_PRODUCT(normal, orientation, vec1);
    VEC_NORMALIZE(normal);
    
    for(int i = 0; i < 360/resolution; i++){
        
        // draw one line on the cone
        vector<double> conePoint(3, 0);
        getRotated(conePoint, ending, spreadness, normal[0], normal[1], normal[2]);
        glBegin(GL_LINES);
        glLineWidth(3.0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(probe[0], probe[1], probe[2]);
        glVertex3f(conePoint[0], conePoint[1], conePoint[2]);
        glEnd();
        
        // find intersections
        vector<double> intersection(3, 0);
        vector<double> ray = {conePoint[0] - probe[0], conePoint[1] - probe[1], conePoint[2] - probe[2]};
        
        // Draw intersection points
        glPointSize(7.0f);//set point size to 10 pixels
        glColor3f(1.0f,0.0f,0.0f); //red color
        int count = 0;
        for(int i = 0; i < 6 && count < 2; i++){
            if(!rayPlaneIntersection(intersection, ray, probe, normals[i/2], sides[i][0])){
                continue;
            }
            if(checkPointInTriangle(intersection, sides[i][0], sides[i][1], sides[i][2])){
                glBegin(GL_POINTS); //starts drawing of points
                glVertex3f(intersection[0],intersection[1],intersection[2]);
                glEnd();//end drawing of points
                cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << endl;
            
                count++;
                continue;
            }
            if(checkPointInTriangle(intersection, sides[i][2], sides[i][3], sides[i][0])){
                glBegin(GL_POINTS); //starts drawing of points
                glVertex3f(intersection[0],intersection[1],intersection[2]);
                glEnd();//end drawing of points
                cout << intersection[0] << " " << intersection[1] << " " << intersection[2] << endl;

                count++;
                continue;
            }
        }
        getRotated(normal, normal, resolution, orientation[0], orientation[1], orientation[2]);
        
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
    //    // Rotate cube
    //    rotate_y = (int)(rotate_y + 1) % 360;
    //    //rotate_x = (int)(rotate_x + 1) % 360;
    //
    //    //rotate_y = 0;
    //    rotate_x = 0;
    //
    
    //    spline equation (Bezier)
    //    p(t) =
    //    (1-t)^3 * A +
    //    (3t*(1-t)^2)* B +
    //    (3*t*t*(1-t))* C +
    //    t*t*t*D
    
    vector<double> next(3,0);
    
    vector<double> A = {1.0, 0.5, -0.5};
    vector<double> B = {1.0, 0.3, 0.5};
    vector<double> C = {1.0, -0.3, -0.5};
    vector<double> D = {1.0, -0.5, 0.6};
    
    vector<double> mult(3, 0);
    VEC_SCALE(mult, pow(1-step, 3), A);
    VEC_SUM(next, next, mult);
    VEC_SCALE(mult, 3*step*pow(1-step, 2), B);
    VEC_SUM(next, next, mult);
    VEC_SCALE(mult, 3*pow(step, 2)*(1-step), C);
    VEC_SUM(next, next, mult);
    VEC_SCALE(mult, pow(step, 3), D);
    VEC_SUM(next, next, mult);
    
    step += 0.01;

    // Move probe
    height_x = next[0];
    height_y = next[1];
    height_z = next[2];

//    if(height_y < -0.5)
//        step = 0;
    
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
