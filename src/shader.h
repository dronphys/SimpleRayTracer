#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "fstream"
#include "sstream"

#define ASSERT(x) if (!(x)) __debugbreak();

#define GLCall(f)             \
{                             \
GLClearError();               \
std::cout << __LINE__;         \
f;                            \
ASSERT(GLLogCall);            \
}

static void GLClearError()
{
    while (!glGetError());
}

static bool GLLogCall()
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL error :" << error << "]" << std::endl;
        return false;
    }
    return true;
}

struct ShaderSources { // returning this class in renderer when loading a shader
    std::string vertex() {
        return vertex_shader_source;
    }
    std::string fragment() {
        return fragment_shader_source;
    }

public:
    std::string vertex_shader_source;
    std::string fragment_shader_source;
};


class ShaderLoader {
    enum class ShaderType {
        VERTEX = 0, FRAGMENT = 1
    };

public:
    ShaderLoader(std::string path)
    {
        ShaderType type;
        std::ifstream shader_src_file(path);
        std::string tmp;
        while (std::getline(shader_src_file, tmp))
        {
            if (tmp.find("shader") != std::string::npos)
            {
                (tmp.find("vertex") != std::string::npos ? type = ShaderType::VERTEX
                    : type = ShaderType::FRAGMENT);
            }
            else
            {

                ss[(int)type] << tmp << '\n';
            }

        }
    }
    ShaderSources GetShadersSources() {
        return { ss[0].str(), ss[1].str() };
    }


private:
    std::stringstream ss[2]; // 0 indecs is vertex, 1 is fragment
};



class Shader {
public:
    Shader(std::string path_to_shader) 
    {
        ShaderLoader loader(path_to_shader);
        shaderSrc = loader.GetShadersSources();
        CreateShaderProgram(shaderSrc);
    }

    void use() {
        
        glUseProgram(shader_program);
    }
    void setVec3(float x, float y, float z, std::string name) {
        int vertexColorLocation = glGetUniformLocation(shader_program, name.c_str());
        glUniform3f(vertexColorLocation, x, y, z);
    }
    void setFloat(float value, std::string name) {
        int Location = glGetUniformLocation(shader_program, name.c_str());
       glUniform1f(Location, value);
    }
    void setInt(GLuint value, std::string name) {
        int Location = glGetUniformLocation(shader_program, name.c_str());
        glUniform1i(Location, value);
    }
    GLuint getID() const {
        return shader_program;
    }

private:
    ShaderSources shaderSrc;
    GLuint shader_program;

    GLuint CompileShader(unsigned int type, const char* shader_src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shader_src, nullptr);
        glCompileShader(shader);

        int is_compiled;
        
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled); // checking if shader compiled
        
        if (is_compiled == GL_FALSE)
        {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*) malloc(length * sizeof(char)); // alocating on stack dynamically
            glGetShaderInfoLog(shader, length, &length, message);
            std::cout << "FAILED to compile " << (type == GL_VERTEX_SHADER ? " vertex" : "fragment")
                << "shader" << std::endl;
            std::cout << message << std::endl;
            return -1;
        }

        return shader; // returning compiled shader
    }

    void CreateShaderProgram(ShaderSources src) {
        shader_program = glCreateProgram();
        
        GLuint vs = CompileShader(GL_VERTEX_SHADER, src.vertex().c_str());
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, src.fragment().c_str());
        glAttachShader(shader_program, vs);
        glAttachShader(shader_program, fs);
        glLinkProgram(shader_program);
        checkCompileErrors(shader_program, "PROGRAM");

        glDeleteShader(vs);
        glDeleteShader(fs);
        
    }
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

class SingleShaderLoader {


public:
    SingleShaderLoader(std::string path)
    {
        std::ifstream shader_src_file(path);
        std::string tmp;
        while (std::getline(shader_src_file, tmp))
        {
            ss << tmp << '\n';
        }
    }
    std::string GetShadersSources() {
        return  ss.str();
    }


private:
    std::stringstream ss; // 0 indecs is vertex, 1 is fragment
};

class ComputeShader {
public:
    ComputeShader(std::string path_to_shader)
    {
        SingleShaderLoader loader(path_to_shader);
        shaderSrc = loader.GetShadersSources();
        CreateShaderProgram(shaderSrc);
    }

    void use() {

        glUseProgram(shader_program);
    }
    void setVec3(float x, float y, float z, std::string name) {
        int vertexColorLocation = glGetUniformLocation(shader_program, name.c_str());
        glUniform3f(vertexColorLocation, x, y, z);
    }
    void setFloat(float value, std::string name) {
        int Location = glGetUniformLocation(shader_program, name.c_str());
        glUniform1f(Location, value);
    }
    void setInt(GLuint value, std::string name) {
        int Location = glGetUniformLocation(shader_program, name.c_str());
        glUniform1i(Location, value);
    }
    GLuint getID() const {
        return shader_program;
    }

private:
    std::string shaderSrc;
    GLuint shader_program;

    GLuint CompileShader( const char* shader_src) {
        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &shader_src, nullptr);
        glCompileShader(shader);

        int is_compiled;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled); // checking if shader compiled

        if (is_compiled == GL_FALSE)
        {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)malloc(length * sizeof(char)); // alocating on stack dynamically
            glGetShaderInfoLog(shader, length, &length, message);
            std::cout << "FAILED to compile COMPUTE SHADER" << std::endl;
            std::cout << message << std::endl;
            return -1;
        }

        return shader; // returning compiled shader
    }

    void CreateShaderProgram(std::string src) {
        shader_program = glCreateProgram();

        GLuint shader = CompileShader(src.c_str());
        
        glAttachShader(shader_program, shader);
        glLinkProgram(shader_program);
        checkCompileErrors(shader_program, "PROGRAM");

        glDeleteShader(shader);

    }
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }

};

