


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"
#include "imageHandler.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "CameraMoves.h"
#include <vector>
#define  PI 3.14592

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

struct Metal{
    float roughness;
};
struct Dielectric
{
    float ref_idx;
};

struct Material {
    alignas(16) float col[3]; // color
    int type; // metal, or lamb
    Metal metal;
    Dielectric dielectric;

};

struct Sphere
{
    alignas(16) float center_pos[3];
    float R;
    int material_id;

};



int main()
{
    Sphere s0;
    s0.center_pos[0] = -0.51;
    s0.center_pos[1] = 0.5;
    s0.center_pos[2] = -10.0;
    s0.R = 0.5;
    s0.material_id = 0;
    // floor
    Sphere s1;
    s1.center_pos[0] = 0.0;
    s1.center_pos[1] = -1000.5;
    s1.center_pos[2] = -1.0;
    s1.R = 1000;
    s1.material_id = 1;

    Sphere s2;
    s2.center_pos[0] = -0.51;
    s2.center_pos[1] = 0.5;
    s2.center_pos[2] = -3.0;
    s2.R = 0.5;
    s2.material_id = 2;

// METAL
    Material m0;
    m0.col[0] = 0.5;
    m0.col[1] = 0.4;
    m0.col[2] = 0.7;
    m0.type = 1;
    m0.metal.roughness = 0.02;

//floor
    Material m1;
    m1.col[0] = 1.0;
    m1.col[1] = 0.0;
    m1.col[2] = 0.0;
    m1.type = 0; // 0 is lambert

    Material m2;
    m2.col[0] = 0.2;
    m2.col[1] = 0.7;
    m2.col[2] = 0.5;
    m2.type = 1;
    m2.metal.roughness = 0.01;


    Sphere spheres[3] = {s0,s1,s2};
    Material materials[3] = {m0,m1,m2};
    std::vector<Sphere> vSpheres(2);
    vSpheres[0] = s1;
    vSpheres[1] = s0;


    std::cout << "size of sphere " << sizeof(s0) << std::endl;

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyRayTracer", NULL, NULL);
    MouseMovement::MouseHandler mouseController(600,800);

    glfwSetCursorPosCallback(window,MouseMovement::CursorPositionCallback);
    glfwSetMouseButtonCallback(window, MouseMovement::mouseButtonCallback);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    float vertices1[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    Shader GraphShader("../res/Shaders/basic.shader");


     //--------------
     //----- Creating empty texture we are going to write into?
     //dimensions of the image
    int tex_w = 600, tex_h = 400;
    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
        NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    //--------------------

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
        work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    int work_grp_size[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
        work_grp_size[0], work_grp_size[1], work_grp_size[2]);

    int work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);



    ComputeShader CompShader("../res/Shaders/mycompute.glsl");

    

    // IMGUI
    const char* glsl_version = "#version 330";
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // check for linking errors and validate program as per normal here

    // drawing loop
    const double limitFPS = 1.0 / 60.0;
    double lastTime = glfwGetTime();
    double timer = lastTime;
    double deltaTime = 0, nowTime = 0;
    int frames = 0, updates = 0;




    int r_int = 0;
    // SSBO SPHERES
    GLuint ssbo_spheres;
    glGenBuffers(1, &ssbo_spheres);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof (spheres), &spheres, GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    // binding it
    GLuint ssbo_binding_point_index_spheres = 1;
    GLuint block_index_spheres = glGetProgramResourceIndex(CompShader.getID(), GL_SHADER_STORAGE_BLOCK, "Spheres_data");
    glShaderStorageBlockBinding(CompShader.getID(), block_index_spheres, ssbo_binding_point_index_spheres);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding_point_index_spheres, ssbo_spheres);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    std::cout << "Block Index Spheres: " << block_index_spheres << std:: endl;

    //SSBO MATERIALS

    GLuint ssbo_materials;
    glGenBuffers(1, &ssbo_materials);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_materials);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(materials), &materials, GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    ///
    GLuint ssbo_binding_point_index_materials = 4;
    GLuint block_index_materials = glGetProgramResourceIndex(CompShader.getID(), GL_SHADER_STORAGE_BLOCK, "Materials_data");
    glShaderStorageBlockBinding(CompShader.getID(), block_index_materials, ssbo_binding_point_index_materials);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding_point_index_materials, ssbo_materials);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind


    // CAMERA
    // UBO
    MouseMovement::CamInfo camera;
    GLuint ubo = 0;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(camera), &camera, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint binding_point_index = 2;
    unsigned int uboIdx = glGetUniformBlockIndex(CompShader.getID(), "camInfo");
    glUniformBlockBinding(CompShader.getID(),    uboIdx , binding_point_index);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);
    std::cout << "UBO Idx: " << uboIdx << std:: endl;


    // Our state
    bool show_demo_window = false;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float tmp_x, tmp_z;



    while (!glfwWindowShouldClose(window)) {
        bool camera_moved = false;


        float timeValue = glfwGetTime();
        float xValue = 5*(sin(timeValue) / 2.0f) + 0.5f;
        float zValue = 5*(sin(timeValue + PI/2.0) / 2.0f) + 0.5f;

        spheres[0].center_pos[0] = xValue;
        spheres[0].center_pos[2] = zValue;
        camera_moved = true;
//        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
//        GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
//        memcpy(p, &camera, sizeof(camera));
//        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
        GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        memcpy(p, &spheres, sizeof(spheres));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

//        glBindBuffer(GL_UNIFORM_BUFFER, ssbo_spheres);
//        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(spheres), &spheres);
//        glBindBuffer(GL_UNIFORM_BUFFER, 0);

//        glBindBuffer(GL_UNIFORM_BUFFER, ssbo_materials);
//        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(materials), &materials);
//        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) / limitFPS;
        lastTime = nowTime;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

//        ImGui::ShowUserGuide();
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
//        {
//            static float f = 0.0f;
//            static int counter = 0;
//
//            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
//            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
//            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//            ImGui::Checkbox("Another Window", &show_another_window);
//            ImGui::SliderFloat("X", &camera.directionView[0], -1.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
//            ImGui::SliderFloat("Z", &camera.directionView[1], -1.0f, 1.0f);
//            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
//            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//                counter++;
//            ImGui::SameLine();
//            ImGui::Text("counter = %d", counter);
//            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//            ImGui::End();
//        }
        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Control info", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Use WASD to move");
            ImGui::Text("Use Left mouse button to rotate camera");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }



        //while (deltaTime > 1) {
            glfwPollEvents();
            if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window, 1);
            }
        UpdateCamera(window,camera,camera_moved);

            deltaTime--;


            //////////////
            { // launch compute shaders!
                
                CompShader.use();
//                CompShader.setVec3(camera_pos[0], camera_pos[1], camera_pos[2], "camera_origin");

                glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

//                CompShader.setVec3(direction_view[0] + camera_pos[0], direction_view[1]+ camera_pos[1],
//                                   direction_view[2]+camera_pos[2], "direction_view");

                if (camera_moved)
                {
                    r_int = 0;
                }
                
                    r_int++;
                //int r_int = camera_moved ?  rand()% 100 : 0;

                CompShader.setInt(r_int,"state");
                CompShader.setFloat(float(r_int) / float(r_int + 1),"u_Accum");
            }

            // make sure writing to image has finished before read
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            // normal drawing pass
            glClear(GL_COLOR_BUFFER_BIT);

            glBindVertexArray(VAO);
            GraphShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_output);

            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);


            frames++;
        //}

        if (glfwGetTime() - timer > 1.0) {
            timer++;
            std::cout << "FPS: " << frames << std::endl;
            std::cout << "X pos: " <<MouseMovement::xPos << std::endl;
            std::cout << "Last X pos: " <<MouseMovement::last_xPos << std::endl;

            std:: cout << "is LKM pressed " << MouseMovement::is_LKM_pressed << std::endl;
            frames = 0;

        }

        
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}