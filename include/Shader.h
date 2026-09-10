#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>

#include <mach-o/dyld.h>

struct Shader
{
private:
    std::string Code;
    std::ifstream ShaderFile;
    std::filesystem::path executableDirectory() const
    {

        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) == 0)
        {
            buffer.resize(std::strlen(buffer.c_str()));
            return std::filesystem::path(buffer).parent_path();
        }

        return std::filesystem::current_path();
    }

public:
    Shader() {}

    const char *GetShader(const std::string &filepath)
    {
        ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::filesystem::path shaderPath(filepath);
        if (shaderPath.is_relative())
        {
            const std::filesystem::path executablePath = executableDirectory();
            const std::filesystem::path candidatePath = executablePath / shaderPath;

            if (std::filesystem::exists(candidatePath))
            {
                shaderPath = candidatePath;
            }
        }

        ShaderFile.open(shaderPath);

        std::stringstream vShaderStream;

        vShaderStream << ShaderFile.rdbuf();
        ShaderFile.close();

        Code = vShaderStream.str();

        return Code.c_str();
    }
};