//modified by: Christian Schmiedel
//date:
//
//original author: Gordon Griesel
//date:            Fall 2024
//purpose:         OpenGL sample program
//
//This program needs some refactoring.
//We will do this in class together.
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

//some structures

class Global {
public:
    int xres, yres;
    float w;
    float vel;
    float pos[2];
    float color[3]; //adds color int
    Global() {
        xres = 400;
        yres = 200;
        w = 20.0f;
        vel = 30.0f;
        pos[0] = 0.0f + w;
        pos[1] = yres / 2.0f;
        color[0] = yres / 2.0f; //starting color blue
        color[1] = 0.0f;
        color[2] = 1.0f;
    }
} g;

class X11_wrapper {
private:
    Display *dpy;
    Window win;
    GLXContext glc;
public:
    ~X11_wrapper();
    X11_wrapper();
    void set_title();
    bool getXPending();
    XEvent getXNextEvent();
    void swapBuffers();
    void reshape_window(int width, int height);
    void check_resize(XEvent *e);
    void check_mouse(XEvent *e);
    int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);


int main()
{
    init_opengl();
    int done = 0;
    //main game loop
    while (!done) {
        //look for external events such as keyboard, mouse.
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            x11.check_mouse(&e);
            done = x11.check_keys(&e);
        }
        physics();
        render();
        x11.swapBuffers();
        usleep(200);
    }
    return 0;
}


X11_wrapper::~X11_wrapper()
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w = g.xres, h = g.yres;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        cout << "\n\tcannot connect to X server\n" << endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        cout << "\n\tno appropriate visual found\n" << endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 Lab-1");
}

bool X11_wrapper::getXPending()
{
    //See if there are pending events.
    return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
    //Get a pending event.
    XEvent e;
    XNextEvent(dpy, &e);
    return e;
}

void X11_wrapper::swapBuffers()
{
    glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
    //Window has been resized.
    g.xres = width;
    g.yres = height;
    //
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != g.xres || xce.height != g.yres) {
        //Window size did change.
        reshape_window(xce.width, xce.height);
    }
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    //Weed out non-mouse events
    if (e->type != ButtonRelease &&
        e->type != ButtonPress &&
        e->type != MotionNotify) {
        //This is not a mouse event that we care about.
        return;
    }
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed.
            //int y = g.yres - e->xbutton.y;
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed.
            return;
        }
    }
    if (e->type == MotionNotify) {
        //The mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
            //Code placed here will execute whenever the mouse moves.


        }
    }
}

int X11_wrapper::check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_a:
                //the 'a' key was pressed
                break;
            case XK_Escape:
                //Escape key was pressed
                return 1;
        }
    }
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //
    //initialize the model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //Set the screen background color
    //set when screen is clear
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics()
{
    static int bounceCount = 0;
    static int frameCount = 0;

    if (g.xres < g.w * 2) {
        return; //box dissapear if window too small
}

    g.pos[0] += g.vel;

    if (g.pos[0] >= (g.xres - g.w) || g.pos[0] <= g.w) {
        g.vel = -g.vel;
        bounceCount++;
    }

    frameCount++;

    if (frameCount % 60 == 0) { // Adjust color every 60 frames
        float rate = (float)bounceCount / frameCount;
        if (rate > 0.05f) { // Higher rate -> red
            g.color[0] = 1.0f;
            g.color[1] = 0.0f;
            g.color[2] = 0.0f;
        } else { // Lower rate -> blue
            g.color[0] = 0.0f;
            g.color[1] = 0.0f;
            g.color[2] = 1.0f;
        }
        bounceCount = 0;
        frameCount = 0;
    }
}

void render()
{
    //
    glClear(GL_COLOR_BUFFER_BIT);
    if (g.xres < g.w *2) {
        return; //dont render if too small
}

    glPushMatrix();
    glColor3f(g.color[0], g.color[1], g.color[2]);
    glTranslatef(g.pos[0], g.pos[1], 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-g.w, -g.w);
        glVertex2f(-g.w,  g.w);
        glVertex2f( g.w,  g.w);
        glVertex2f( g.w, -g.w);
    glEnd();
    glPopMatrix();
}

~                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
~                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
~                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
~                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
~                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
~                                                                                  
