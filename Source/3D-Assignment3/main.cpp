// B-Spline editor
// Author: UUSim
// February 2011

#include <stdio.h>
#include <stdlib.h>
#include <GLUT/GLUT.h> // For macOS
//#include <GL/glut.h> // For Windows
#include <string>
#include <vector>
using namespace std;

struct controlpoint { float x, y; };
vector<controlpoint> p; // Create the control point vector
vector<int> t; // Create the knot vector
vector<controlpoint> pi; // Curve points vector
int d =3, n, m; // Default degree is 3, quadratic
bool uniform = true;
int psize = 10; // display size of the control points
int pselected = -1; // Stores the index pointer of the selected Control point, has no mathematical purpose
bool pmoving;
bool calculated = false; // Is true if the knot vector has been calculated.
bool drawLines=true;
int screenwidth = 600;
int screenheight = 500;


// GLUT popup menu item definitions
#define OPTION_UNIFORM 1
#define OPTION_DEGREE2 12
#define OPTION_DEGREE3 13
#define OPTION_DEGREE4 14
#define OPTION_DEGREE5 15
#define OPTION_DEGREE6 16
#define OPTION_DRAWLINES 3
#define OPTION_REMOVEX 4
#define OPTION_QUIT 27


void displayText(float x, float y, std::string text) {
    // Outputs text using GLUT, at the specyfied location.
    
    char c[100];
    memset(c,0,sizeof(c));
    strcpy(c, text.data());
    
    glRasterPos2f(x, y);
    for (int i=0; i < (int)strlen(c); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c[i]);
    }
}


void showControlPoints() {
    // Places small boxes (size defined in psize) to indicate Control points
    // Also displays guidelines between the control points
    
    for(unsigned int i = 0; i< p.size(); i++){
        if (i==pselected) {
            glColor3f(1,1,0);
        }
        else {
            glColor3f(0,1,0);
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f( p.at(i).x - psize, p.at(i).y - psize);
        glVertex2f( p.at(i).x + psize, p.at(i).y - psize);
        glVertex2f( p.at(i).x + psize, p.at(i).y + psize);
        glVertex2f( p.at(i).x - psize, p.at(i).y + psize);
        glEnd();
        if (i!=0){
            glColor3f(1,0,0);
            glBegin(GL_LINES);
            glVertex2f(p.at(i-1).x, p.at(i-1).y);
            glVertex2f(p.at(i).x, p.at(i).y);
            glEnd();
        }
    }
}


float divide(float a, float b) {
    // This function is to prevent almost 0 floating point values screwing up the results.
    if (b==0) {
        // Therefore, return a proper 0
        return 0;
    }
    else {
        return a/b;
    }
}


float basisFunction(int bj, int bd, float bu) {
    if (bd==1){
        if (t.at(bj) <= bu && bu < t.at(bj+1)) {
            return 1.0;
        }
        else {
            return 0.0;
        }
    }
    else {
        float base=	divide( (float)(bu - t.at(bj)),(float)(t.at(bj+bd-1)-t.at(bj)) ) * basisFunction (bj,	bd-1,	bu) +
        divide((float)(t.at(bj+bd)-bu),(float)(t.at(bj+bd)-t.at(bj+1)))	* basisFunction (bj+1,	bd-1,	bu);
        
        return base;
    }
}


void calculateInterpolatedPoints() {
    if ((int)p.size()>=d) {
        // The range is Td-1 >= u <= Tn+1 (n+1 is equal to p.size(), thanks Robby!!)
        float uMin = (float)t.at(d-1);
        float uMax = (float)t.at(n+1);
        float uStep = 0.05;
        float tempb = 0; // variable for results of the basis funtion
        
        pi.clear(); //Empty the Curve vector
        
        for (float u = uMin; u <= uMax; u+=uStep) {
            controlpoint pin = {0,0};
            for (int j=0; j < (int)p.size(); j++) {
                // Compute the basis function for Point j
                tempb = basisFunction(j, d, u);
                
                // Update the curve point x & y
                pin.x += p.at(j).x * tempb;
                pin.y += p.at(j).y * tempb;
            }
            // The Curve point is calculated, add it to the curve vector(stack)
            pi.push_back(pin);
        }
        
        // The position of the last control point to the curve point vector (only form non-unform)
        if (!uniform) pi.push_back(p.at(p.size()-1));
    }
    else {
        // Not enough control points to calculate a line with this degree.
        glColor3f(1,0,0);
        displayText(4, 4, "Not enough control points defined for this degree.");
    }
    
    
}


void drawInterpolatedPoints(){
    if ((int)p.size()>=d) {
        // Draw lines or points for all the Curve Q(u) points
        for (int i=0;i<(int)pi.size();i++){
            if (i!=0){
                glColor3f(0,1,1);
                if (drawLines) {
                    glBegin(GL_LINES);
                }
                else {
                    glPointSize(3);
                    glBegin(GL_POINTS);
                }
                glVertex2f(pi.at(i-1).x, pi.at(i-1).y);
                glVertex2f(pi.at(i).x, pi.at(i).y);
                glEnd();
            }
        }
    }
    else {
        // Not enough control points to draw a line with this degree.
    }
}


void calculateKnotVector(){
    // (Step 0) Determine general properties of the Spline
    n = (int)p.size() - 1; // the number of the control points, minus 1! (Again, credits to Robby)
    m = n + d + 1;
    
    // Compute the Knot vector, if nessecary
    if (!calculated) {
        t.clear();
        
        if (uniform) {
            // Create a uniform knot vector
            for (int i=0;i!=m;i++) {
                t.push_back(i);		// Fill the knot vector
            }
        }
        else {
            // Create a non-uniform knot vector
            for (int j=0;j!=m;j++) {
                if (j<d) {
                    t.push_back(0);		// Fill the knot vector
                }
                else if (j <= n) {
                    t.push_back(j-d+1);
                }
                else {
                    t.push_back(n-d+2);
                }
            }
            
        }
        calculated=true;
        
        // Output the knot vector to the console
        printf ("Knot vector: ");
        for (int i=0;i!=t.size();i++) {
            printf ("%i ",t.at(i));
        }
        printf ("\n");
    }
}


char* modeName(int dName) {
    // Convert degree number into name
    // Used for displaying purposes only
    
    switch (dName){
        case 2:
            return "Linear";
            break;
        case 3:
            return "Quadratic";
            break;
        case 4:
            return "Cubic";
            break;
        case 5:
            return "Quartic";
            break;
        case 6:
            return "Quintic";
            break;
    }
    return "Unkown";
}


void mainWindow (void) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0, screenwidth, 0.0, screenheight, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    
    // Display the current Polinomial degree on the screen
    glColor3f(0.8,0.8,0.8);
    string info = "Mode: ";
    info += modeName(d);
    displayText(4, screenheight - 14, info);
    
    // Calculate & show the curve, with its control points
    calculateKnotVector();
    showControlPoints();
    calculateInterpolatedPoints();
    drawInterpolatedPoints();
    
    // Output everything correctly to the screen
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glutSwapBuffers();
}


void windowResizer (int width, int height) {
    // Adjust the global screen size variables,
    // otherwise points will be added in unexpected positions
    screenwidth = width;
    screenheight = height;
    
    glViewport( 0, 0, (GLsizei) width, (GLsizei) height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity ();
    gluPerspective( 60, (GLfloat) width /(GLfloat) height , 0.10, 1000.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}


void editPoint(int pindex, int x, int y) {
    p.at(pindex).x = x;
    p.at(pindex).y = y;
}


void addPoint(float x,float y) {
    controlpoint pn;
    pn.x = x;
    pn.y = y;
    
    // Add the point to the control point list
    p.push_back(pn);
    
    // Visually select the new point
    pselected = (int)p.size() - 1;
    
    // Make sure the knot vector gets updated
    calculated = false;
}


void removePoint (int pindex) {
    p.erase(p.begin() + pindex);
    pmoving = false;
    
    // Select a previous point, or select none if it was the last one.
    pselected = (int)p.size()-1;
    calculated = false;
}


int checkForPoint(int x, int y) {
    // Funtion checks if the given x and y coords match an existing control point
    // returns the control point vector index if a match is found,
    // otherwise return -1
    for(unsigned int i = 0 ; i< p.size();i++){
        if( (x>=p.at(i).x - psize) && (x<=p.at(i).x + psize) && (y>=p.at(i).y - psize) && (y<=p.at(i).y + psize)) {
            return (int)i;
        }
    }
    return -1;
}


void toggleUniform() {
    // Switch between Uniform & Non-uniform B-Spline
    uniform = !uniform;
    calculated = false;
    if (uniform) printf ("Uniform B-Spline\n");
    else printf ("Non-Uniform B-Spline\n");
}


void toggleDrawLines() {
    // Display the curve either as lines, or as the curve points
    
    drawLines = !drawLines;
    if (drawLines) printf ("Curve displaying as line\n");
    else printf ("Curve displaying as points\n");
}


void changeDegree(int dNew){
    // Choose the polynomial degree
    // Only update if it is different
    if (dNew!=d){
        d=dNew;
        printf("Mode: %s\n", modeName(d));
        calculated=false;
    }
}


void keyInputHandler(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
        case 'q':
            // Exit application on Escape or 'q' key
            exit(0);
            break;
            
        case 'd':
            // Toggle between polynomial degrees
            switch(d){
                case 6:
                    changeDegree(2);
                    break;
                default:
                    changeDegree(d+1);
                    break;
            }
            break;
            
        case 'u':
            // Switch between uniform and Non-uniform
            toggleUniform();
            break;
            
        case 'l':
            // Toggle between curve points / lines
            toggleDrawLines();
            break;
            
        case 'x':
            // The X key removes a selected point
            if (pselected!=-1) {
                removePoint(pselected);
            }
            break;
    }
}


void mouseInputHandler(int button, int state, int mx, int my) {
    // Calculate the y axis according to the world location
    my = screenheight - my;
    pmoving = false;
    
    // Create or update a control point
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        pselected = checkForPoint(mx,my);
        
        if (pselected<0) {
            // New point
            addPoint(mx,my);
        }
        else {
            // Edit the point
            editPoint(pselected, mx ,my);
            pmoving = true;
        }
    }
    glutPostRedisplay();
}


void mouseMotionHandler(int mx, int my) {
    my = screenheight - my;
    if(pmoving) {
        editPoint(pselected,mx,my);
        glutPostRedisplay();
    }
}


void processMenuEvents(int option) {
    // Handle chosen menu options
    switch (option) {
        case OPTION_UNIFORM :
            toggleUniform(); break;
        case OPTION_DRAWLINES :
            toggleDrawLines();
            break;
        case OPTION_DEGREE2:
            changeDegree(2); break;
        case OPTION_DEGREE3:
            changeDegree(3); break;
        case OPTION_DEGREE4:
            changeDegree(4); break;
        case OPTION_DEGREE5:
            changeDegree(5); break;
        case OPTION_DEGREE6:
            changeDegree(6); break;
        case OPTION_REMOVEX:
            if (pselected!=-1) {
                removePoint(pselected);
            }
            break;
        case OPTION_QUIT:
            exit(0); break;
            
    }
}


void createGLUTMenus() {
    
    int menu, submenu;
    // Create a submenu for selecting the Polyniomal degree
    submenu = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Linear",OPTION_DEGREE2);
    glutAddMenuEntry("Quadratic",OPTION_DEGREE3);
    glutAddMenuEntry("Cubic",OPTION_DEGREE4);
    glutAddMenuEntry("Quartic",OPTION_DEGREE5);
    glutAddMenuEntry("Quintic",OPTION_DEGREE6);
    
    menu = glutCreateMenu(processMenuEvents);
    
    // Add the options to the menu
    glutAddMenuEntry("Toggle (Non) Uniform [u]",OPTION_UNIFORM);
    glutAddSubMenu("Polynomial degree -> [d]",submenu);
    glutAddMenuEntry("Toggle curve points/line [l]",OPTION_DRAWLINES);
    glutAddMenuEntry("Remove selected control point [x]", OPTION_REMOVEX);
    glutAddMenuEntry("Quit program [q]", OPTION_QUIT);
    
    // Attach the menu to the right button
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


int main (int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize (screenwidth, screenheight);
    glutInitWindowPosition (200, 100);
    glutCreateWindow ("3D Modelling: Assignment 3, B-Spline");
    glutDisplayFunc(mainWindow);
    glutIdleFunc(mainWindow);
    glutReshapeFunc(windowResizer);
    glutKeyboardFunc(keyInputHandler);
    glutMouseFunc(mouseInputHandler); 
    glutMotionFunc(mouseMotionHandler); 
    createGLUTMenus();
    
    // Add sample points
    addPoint (screenwidth / 2 + -50, screenheight / 2 - 50);
    addPoint (screenwidth / 2 + 20, screenheight / 2 - 50.0);
    addPoint (screenwidth / 2 + 70, screenheight / 2 - -50);
    
    glutMainLoop();
    return 0;
}
