#include "algo/FortunesAlgorithm.h"
#include "types/Point.h";
#include "utils/Conversion.h"
#include "utils/PriorityQueue.h"

#include <cstdlib>
#include <direct.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <time.h>

void drawBeachLine(const std::vector<BL::Arc*>& beachLine, const double h);
void drawParabola(const Point& point, const double lh, double x1, double x2);
void drawLine(const Point& p1, const Point& p2);
void drawPoints(const std::vector<Point>& points);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

FortunesAlgorithm* algorithm;
VoronoiDiagram* voronoi;
bool showTri = false, pause = true;
double speed = 5.0;

char currentPath[FILENAME_MAX];

int main(int argc, char* argv[])
{
    if (argc < 2)
        return -1;
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* DataIntialization */
    if (!_getcwd(currentPath, sizeof(currentPath)))
        return -1;

    std::string cPath = currentPath;

    std::vector<Point> points; 
    for (int i = 0; i < 1000; i++)
    {
        points.push_back(Point({(double)(rand() % 1000), (double)i }));
    }

    voronoi = new VoronoiDiagram(cPath + "\\" + argv[1]);
    //voronoi = new VoronoiDiagram(points);
    algorithm = new FortunesAlgorithm(*voronoi);
    FortunesAlgorithm& fA = *algorithm;

    SetPlaneBounds(-5.0 + voronoi->MinX, 5.0 + voronoi->MaxX, -5.0 + voronoi->MinY, 5.0 + voronoi->MaxY);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    double x, y, pastTime = 0, totalTime = 0, time;
    bool printed = false;
    while (!glfwWindowShouldClose(window))
    {

        glClear(GL_COLOR_BUFFER_BIT);
        double time = fA.GetHeight();

        if (!pause)
        {
            totalTime += (speed) * (glfwGetTime() - pastTime);
        }
        pastTime = glfwGetTime();
        time = PlaneBounds::GetInstance()->Upper - (totalTime);

        /* Render here */
        // Drawing Voronoi BeachLine and Sites
        glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
        drawPoints(voronoi->Points);

        if (fA.IsComplete())
        {
            //SetPlaneBounds(fA.MinX, fA.MaxX, fA.MinY, fA.MaxY);
            for (const BL::Edge* edge : fA.GetInfiniteEdges())
            {
                drawLine(*edge->Start, *edge->End);
            }

            if(showTri) 
            {
                glColor3f(0.0f, 1.0f, 0.0f); 
                for (const DCEL::HalfEdge* edge : voronoi->TriangulationHalfEdges)
                {
                    drawLine(edge->origin->point, edge->dest->point);
                }
            }

            if (!printed)
                voronoi->PrintToFile(cPath + "\\voronoi.txt");

            time = fA.GetHeight();
        }
        else
        {
            fA.Continues(time);
            drawBeachLine(fA.InOrder(), time);
            if (time < PlaneBounds::GetInstance()->Lower)
                fA.Run();
        }

        //Drawing Sweep Line
        glColor3f(1.0f, 0.5f, 0.5f); // Set color to white
        drawLine(Point({ PlaneBounds::GetInstance()->Left, time }), Point(PlaneBounds::GetInstance()->Right, time));

        glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
        for (const BL::Edge* edge : fA.GetCompletedEdges())
        {
            drawLine(*edge->Start, *edge->End);
        }

        // Drawing Vertices

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        glfwSetKeyCallback(window, keyCallback);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    delete algorithm;
    delete voronoi;

    return 0;
}

// Key callback function
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == 257 && !algorithm->IsComplete())
        {
            algorithm->Run();
        }
        else if (key == 32)
        {
            pause = !pause;
            //algorithm->Next();
        }
        else if (key == 87) {
            speed *= 1.5;
        }
        else if (key == 83) {
            if(speed >= 0) speed /= 1.5;
        }
        else if (key == 48)
        {
            voronoi->PrintVoronoiDCEL(std::cout);
        }
        else if (key == 57)
        {
            voronoi->PrintDelaunayTriangulation(std::cout);
        }
        else if (key == 84)
        {
            showTri = !showTri;
        }
        else if (key == 49)
        {
            std::cout << algorithm->MinX << ", " << algorithm->MaxX << std::endl;
            PlaneBounds::GetInstance()->SetBounds(algorithm->MinX - 5.0, algorithm->MaxX + 5.0, algorithm->MinY - 5.0, algorithm->MaxY + 5.0);
        }
        std::cout << "Key pressed: " << key << std::endl;
    }
}

void drawBeachLine(const std::vector<BL::Arc*>& beachLine, const double h)
{
    double lastBreakPointX = PlaneBounds::GetInstance()->Left;
    double nextBreakPoint = PlaneBounds::GetInstance()->Right;
    if (0 == beachLine.size())
        return;

    if (1 == beachLine.size())
    {
        drawParabola(beachLine[0]->Site->point, h, lastBreakPointX, nextBreakPoint);
        return;
    }

    for (int i = 1; i < beachLine.size(); i += 2)
    {
        if (nullptr == beachLine[i]->Edge)
            continue;
        if (beachLine[i]->Edge->IsVertical)
            nextBreakPoint = (beachLine[i]->Edge->Left->point.x + beachLine[i]->Edge->Right->point.x) / 2.0;
        else
            nextBreakPoint = IntersectionX(beachLine[i]->Edge->Left->point, beachLine[i]->Edge->Right->point, h);
        drawParabola(beachLine[i - 1]->Site->point, h, lastBreakPointX, nextBreakPoint);
        drawLine(*beachLine[i]->Edge->Start,
            Point({ nextBreakPoint, CalculateParabolaY(nextBreakPoint, h, beachLine[i - 1]->Site->point) }));
        lastBreakPointX = nextBreakPoint;
    }

    drawParabola(beachLine[beachLine.size() - 1]->Site->point, h, lastBreakPointX, PlaneBounds::GetInstance()->Right);
}

void drawParabola(const Point& point, const double lh, double x1, double x2) {
    glBegin(GL_LINE_STRIP);

    double xo, yo;
    for (double x = x1; x <= x2 && x <= PlaneBounds::GetInstance()->Right; x += 0.2) {
        PointToScreen(Point(x, CalculateParabolaY(x, lh, point)), xo, yo);
        glVertex2f(GLfloat(xo), GLfloat(yo));
    }
    glEnd();
}

void drawLine(const Point& p1, const Point& p2)
{
    double x, y;

    glBegin(GL_LINES);
    PointToScreen(p1, x, y);
    glVertex2f(GLfloat(x), GLfloat(y));
    PointToScreen(p2, x, y);
    glVertex2f(GLfloat(x), GLfloat(y));
    glEnd();
}

void drawPoints(const std::vector<Point>& points)
{
    double x, y;

    glEnable(GL_POINT_SIZE);
    glPointSize(3);

    glBegin(GL_POINTS);
    for (const Point& p : points)
    {
        PointToScreen(p, x, y);
        glVertex2f(GLfloat(x), GLfloat(y));
    }
    glEnd();
}