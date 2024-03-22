#include "types/Point.h";
#include "utils/Conversion.h"

#include <cstdlib>
#include <GLFW/glfw3.h>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* DataIntialization */
    int size = 10;
    Point* pointData = nullptr;
    if (!(pointData = (Point*)malloc(sizeof(Point) * size)))
    {
        glfwTerminate();
        return -1;
    }
    for (int i = 0; i < size; i++)
    {
        pointData[i].x = (float)i - 5.f;
        pointData[i].y = (float)i - 5.f;
    }
    SetPlaneBounds(-10.f, 10.f, -10.f, 10.f);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    int i;
    float x, y;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_POINT_SIZE);
        glPointSize(5);
        glBegin(GL_POINTS);
        for (i = 0; i < size; i++)
        {
            PointToScreen(pointData[i], x, y);
            glVertex2f(x, y);

        }
        glEnd();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    free(pointData);
    glfwTerminate();
    return 0;
}