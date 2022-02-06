


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

enum  MaterialType {
    LAMBERT = 0,
    METAL = 1,
    GLASS = 2,
    LIGHT = 3
};


int main()
{
    // floor
    Sphere s0;
    s0.center_pos[0] = 0.0;
    s0.center_pos[1] = -1000.5;
    s0.center_pos[2] = -1.0;
    s0.R = 1000;
    s0.material_id = 0;


    Sphere s1;
    s1.center_pos[0] = -0.51;
    s1.center_pos[1] = 0.5;
    s1.center_pos[2] = -10.0;
    s1.R = 0.5;
    s1.material_id = 1;



    Sphere s2;
    s2.center_pos[0] = -0.51;
    s2.center_pos[1] = 0.5;
    s2.center_pos[2] = -3.0;
    s2.R = 0.5;
    s2.material_id = 2;

    //floor
    Material m0;
    m0.col[0] = 1.0;
    m0.col[1] = 0.0;
    m0.col[2] = 0.0;
    m0.type = LAMBERT; // 0 is lambert


// METAL
    Material m1;
    m1.col[0] = 0.5;
    m1.col[1] = 0.4;
    m1.col[2] = 0.7;
    m1.type = METAL;
    m1.metal.roughness = 0.02;

    Material m2;
    m2.col[0] = 0.2;
    m2.col[1] = 0.7;
    m2.col[2] = 0.5;
    m2.type = METAL;
    m2.metal.roughness = 0.01;


    Sphere spheres[3] = {s0,s1,s2};
    Material materials[3] = {m0,m1,m2};
    std::vector<Sphere> vSpheres;
    vSpheres.reserve(10);
    std::vector<Material> vMaterials;
    vMaterials.reserve(10);


    vSpheres.push_back(s0);
    vSpheres.push_back(s1);
    vSpheres.push_back(s2);

    vMaterials.push_back(m0);
    vMaterials.push_back(m1);
    vMaterials.push_back(m2);



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
    glBufferData(GL_SHADER_STORAGE_BUFFER, vSpheres.size()*sizeof (Sphere), vSpheres.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
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
    glBufferData(GL_SHADER_STORAGE_BUFFER, vMaterials.size()*sizeof(Material), vMaterials.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
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




    bool showEditPanel = false;
    int idEdit = 2;
    bool delete_something = false;
    int idxDelete = -1;
    while (!glfwWindowShouldClose(window)) {
        bool camera_moved = false;


        float timeValue = glfwGetTime();
        float xValue = 5*(sin(timeValue) / 2.0f) + 0.5f;
        float yValue = 1*(sin(timeValue + PI/2.0) / 2.0f) + 1.5f;
        float zValue = 1*(sin(timeValue + PI/4.0) / 2.0f);
//
//        vSpheres[0].center_pos[0] = xValue;
//        vSpheres[0].center_pos[1] = yValue;
//        vSpheres[0].center_pos[2] = zValue;
//        camera_moved = true;



        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera), &camera);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);



        glBindBuffer(GL_UNIFORM_BUFFER, ssbo_spheres);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, vSpheres.size()*sizeof(Sphere), vSpheres.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
//        GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
//        memcpy(p, vSpheres.data(), vSpheres.size()*sizeof (Sphere));
//        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);



        glBindBuffer(GL_UNIFORM_BUFFER, ssbo_materials);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, vMaterials.size()*sizeof(Material), vMaterials.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);


        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) / limitFPS;
        lastTime = nowTime;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (showEditPanel){

            float* xPos = &vSpheres[idEdit].center_pos[0];
            float* yPos = &vSpheres[idEdit].center_pos[1];
            float* zPos = &vSpheres[idEdit].center_pos[2];
            float* Radius = &vSpheres[idEdit].R;
            float* col = vMaterials[idEdit].col;
            char* items[] = {"Lambert", "Metal", "Glass", "Light"};
            int* current_type = &vMaterials[idEdit].type;


        {
            ImGui::Begin("Sphere Param Editor", &showEditPanel);
            if (idEdit != 0){ // not allowing to move ground
            if (ImGui::SliderFloat("Slider X", xPos,-5.0f,5.0f))
            {
                camera_moved = true;
            }

            if (ImGui::SliderFloat("Slider Y", yPos,-5.0f,5.0f))
            {
                camera_moved = true;
            }

            if (ImGui::SliderFloat("Slider Z", zPos,-10.0f,10.0f))
            {
                camera_moved = true;
            }
            if (ImGui::SliderFloat("Radius", Radius,0,5.0f) )
            {
                camera_moved = true;
            }
            }
            if (ImGui::ColorEdit3("color", col))
            {
                camera_moved = true;
            }

            if (ImGui::Combo("Type", current_type, items, 4))
            {
                camera_moved = true;
            }
            if (*current_type == METAL)
            {
                if (ImGui::SliderFloat("Rougness", &vMaterials[idEdit].metal.roughness,0.0f,0.2f))
                {
                    camera_moved = true;
                }
            }
            if (*current_type == GLASS)
            {
                if (ImGui::SliderFloat("Refractive Index", &vMaterials[idEdit].dielectric.ref_idx,1.0f,2.0f))
                {
                    camera_moved = true;
                }


            }
            ImGui::End();
        }
        }
        ImGui::Begin("List");
        if (ImGui::BeginTable("List of all spheres", 3))
        {
            for (int i = 0; i < vSpheres.size(); i++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                ImGui::Text("Sphere %d", i);

                ImGui::TableNextColumn();
                std::string title = "Edit###" + std::to_string( i);

                if (ImGui::Button(title.c_str()))
                {
                    idEdit = i;
                    showEditPanel = true;
                    std::cout << "edit something triggerred";
                }

                ImGui::TableNextColumn();
                std::string d_title = "delete###" + std::to_string(vSpheres.size() +i);

                if ( ImGui::Button(d_title.c_str()) )
                {
                    delete_something = true;
                    idxDelete = i;

                }


            }
            ImGui::EndTable();
            if (ImGui::Button("Add New"))
            {
                Sphere s;
                Material m;
                s.center_pos[0] = -0.5;
                s.center_pos[1] = 0.25;
                s.center_pos[2] = 0.0;
                s.R = 0.5;
                s.material_id = vMaterials.size();
                vSpheres.push_back(s);

                m.col[0] = 1.0f; m.col[1] = 1.0f; m.col[2] = 1.0f;
                m.type = LIGHT;
                m.dielectric.ref_idx = 1.2f;
                m.metal.roughness = 0.0f;
                vMaterials.push_back(m);
                std:: cout << "Material size : ";
                std::cout << vMaterials.size() << std::endl;


                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
                glBufferData(GL_SHADER_STORAGE_BUFFER, vSpheres.size()*sizeof (Sphere), vSpheres.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_materials);
                glBufferData(GL_SHADER_STORAGE_BUFFER, vMaterials.size()*sizeof(Material), vMaterials.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

                camera_moved = true;
                idEdit = vSpheres.size()-1;
                showEditPanel = true;
            }

        }
        ImGui::End();


        if (delete_something)
        {
            vSpheres.erase(vSpheres.begin() + idxDelete);
            vMaterials.erase(vMaterials.begin() + idxDelete);
            // update vectors
            for (int i = 0; i < vSpheres.size(); i++)
            {
                vSpheres[i].material_id = i;
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
            glBufferData(GL_SHADER_STORAGE_BUFFER, vSpheres.size()*sizeof (Sphere), vSpheres.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_materials);
            glBufferData(GL_SHADER_STORAGE_BUFFER, vMaterials.size()*sizeof(Material), vMaterials.data(), GL_DYNAMIC_COPY); //sizeof(data) only works for statically sized C/C++ arrays.
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
            delete_something = false;
            camera_moved = true;
        }



        //ImGui::ShowDemoWindow();

            glfwPollEvents();
            if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(window, 1);
            }
        UpdateCamera(window,camera,camera_moved);

            deltaTime--;



            { // launch compute shaders!
                
                CompShader.use();
                glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

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