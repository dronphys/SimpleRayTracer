#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#define  PI 3.14592
namespace MouseMovement{
    static bool mouseMoved = false;
    static double xPos;
    static double last_xPos;

    static double yPos;
    static double last_yPos;

    static bool is_LKM_pressed;


struct CamInfo{
    float cameraPosition[4] = { 0.0,  0.0, 3.0 };
    float directionView[4] = {0.0, 0.0, -1.0};
    float rightPerp[4] = {1.0 ,0.0, 0.0 };
};

void UpdateCamera(GLFWwindow* window,CamInfo& cam, bool& camera_moved){
    float tmp_x;
    float tmp_z;
    float dMove  = 0.1;

    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) {
        camera_moved = true;
        cam.cameraPosition[0] += dMove*cam.directionView[0];
        cam.cameraPosition[2] += dMove*cam.directionView[2];

    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) {
        camera_moved = true;
        cam.cameraPosition[0] -= dMove*cam.directionView[0];
        cam.cameraPosition[2] -= dMove*cam.directionView[2];
    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) {
        camera_moved = true;
        cam.cameraPosition[0] += cam.rightPerp[0] *dMove;
        cam.cameraPosition[2] += cam.rightPerp[2]*dMove;
    }
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) {
        camera_moved = true;
        cam.cameraPosition[0] += -cam.rightPerp[0]*dMove;
        cam.cameraPosition[2] += -cam.rightPerp[2]*dMove;
    }

    if (is_LKM_pressed && !ImGui::GetIO().WantCaptureMouse){
        if (mouseMoved){
        double angle_change = (xPos - last_xPos)*0.016;
        camera_moved = true;
        tmp_x = cam.directionView[0];
        tmp_z = cam.directionView[2];
        cam.directionView[0] = tmp_x*cos(angle_change) - tmp_z* sin(angle_change) ;
        cam.directionView[2] = tmp_x*sin(angle_change) + tmp_z * cos(angle_change) ;
        tmp_x = cam.rightPerp[0];
        tmp_z = cam.rightPerp[2];
        cam.rightPerp[0] = tmp_x*cos(angle_change) - tmp_z* sin(angle_change) ;
        cam.rightPerp[2] = tmp_x*sin(angle_change) + tmp_z * cos(angle_change) ;
        }
        mouseMoved =false;
    }

}



void CursorPositionCallback(GLFWwindow *window, double xPos_, double yPos_){
        mouseMoved= true;
        last_xPos = xPos;
        last_yPos = yPos;
        xPos = xPos_;
        yPos = yPos_;

}
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mode)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        is_LKM_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        is_LKM_pressed = false;
    }
}



class MouseHandler{
public:
    MouseHandler(int width_,int height_)
    :width(width_), height(height_)
    {
        LKM_pressed = false;
    }
    void CursorPositionCallback(GLFWwindow *window, double xPos_, double yPos_){
        xPos = xPos_;
        yPos = yPos_;
        std::cout << xPos << std::endl;
    }

    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mode)
    {

    }

    double getX() const
    {
        return xPos;
    }
    double getY() const
    {
        return yPos;
    }
private:
    int width;
    int height;
    double xPos;
    double yPos;
    bool LKM_pressed;

};

}
