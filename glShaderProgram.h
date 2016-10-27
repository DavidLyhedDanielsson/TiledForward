//#ifndef GLSHADERPROGRAM_H__
//#define GLSHADERPROGRAM_H__
//
//#include "glShader.h"
//
//#include <vector>
//#include <map>
//
//class GLShaderProgram
//{
//    //TODO: Tuples are kind of awful for this, just use a struct instead...
//    //<index, number, type, normalized, stride, offset>
//    typedef std::tuple<GLuint, GLint, GLint, GLboolean, GLint, GLint> InputLayout;
//
//public:
//    GLShaderProgram();
//    ~GLShaderProgram();
//
//    template<typename... T>
//    bool Load(const std::string& vertexShaderPath
//              , const std::string& pixelShaderPath)
//    {
//
//    }
//
//    template<typename... Rest>
//    bool Load(GLShader* vertexShader
//              , GLShader* pixelShader
//              , size_t numberOfValues
//              , GLEnums::DATA_TYPE inputType
//              , bool normalizedInput
//              , Rest... rest)
//    {
//        std::vector<InputLayout> inputLayout;
//
//        GetInputLayoutRec(inputLayout, numberOfValues, inputType, normalizedInput, rest...);
//
//        int totalSize = std::get<5>(inputLayout.back()) + DataTypeSize((GLEnums::DATA_TYPE)std::get<2>(inputLayout.back())) * std::get<1>(inputLayout.back());
//
//        for(auto& layout : inputLayout)
//            std::get<4>(layout) = totalSize;
//
//        Load(inputLayout, vertexShader, pixelShader);
//
//        return true;
//    };
//
//    template<typename... Rest>
//    bool Load(GLShader* vertexShader
//              , GLShader* pixelShader
//              , const std::string& inputName
//              , size_t numberOfValues
//              , GLEnums::DATA_TYPE inputType
//              , bool normalizedInput
//              , int stride
//              , int offset
//              , Rest... rest)
//    {
//        std::vector<InputLayout> inputLayout;
//        GetInputLayoutRec(inputLayout, inputName, numberOfValues, inputType, normalizedInput, stride, offset, rest...);
//
//        Load(inputLayout, vertexShader, pixelShader);
//
//        return true;
//    };
//
//    template<typename... Rest>
//    bool Load(GLShader* vertexShader
//              , GLShader* pixelShader
//              , size_t numberOfValues
//              , GLEnums::DATA_TYPE inputType
//              , bool normalizedInput
//              , int stride
//              , int offset
//              , Rest... rest)
//    {
//        std::vector<InputLayout> inputLayout;
//        GetInputLayoutRec(inputLayout, numberOfValues, inputType, normalizedInput, stride, offset, rest...);
//
//        Load(inputLayout, vertexShader, pixelShader);
//
//        return true;
//    };
//
//    void Bind();
//    void Unbind();
//
//protected:
//private:
//    bool Load(std::vector<InputLayout> inputLayout, GLShader* vertexShader, GLShader* pixelShader);
//
//    int DataTypeSize(GLEnums::DATA_TYPE dataType)
//    {
//        switch(dataType)
//        {
//            case GLEnums::DATA_TYPE::BYTE:
//            case GLEnums::DATA_TYPE::UNSIGNED_BYTE:
//                return 1;
//            case GLEnums::DATA_TYPE::SHORT:
//            case GLEnums::DATA_TYPE::UNSIGNED_SHORT:
//                return 2;
//            case GLEnums::DATA_TYPE::INT:
//            case GLEnums::DATA_TYPE::UNSIGNED_INT:
//            case GLEnums::DATA_TYPE::FLOAT:
//                return 4;
//            case GLEnums::DATA_TYPE::DOUBLE:
//                return 8;
//            default:
//                throw std::runtime_error("Unsupported type");
//        }
//    }
//
//    void AddInputLayout(std::vector<InputLayout>& inputLayout
//                        , size_t numberOfValues
//                        , GLEnums::DATA_TYPE inputType
//                        , bool normalizedInput)
//    {
//        int offset = 0;
//        for(int i = 0; i < inputLayout.size(); ++i)
//        {
//            offset += DataTypeSize(inputType) * std::get<1>(inputLayout[i]);
//        }
//
//        inputLayout.emplace_back(inputLayout.size(), numberOfValues, (GLint)inputType, normalizedInput, -1, offset);
//    }
//
//    void GetInputLayoutRec(std::vector<InputLayout>& inputLayout)
//    { }
//
//    template<typename... Rest>
//    void GetInputLayoutRec(std::vector<InputLayout>& inputLayout
//                           , size_t numberOfValues
//                           , GLEnums::DATA_TYPE inputType
//                           , bool normalizedInput
//                           , Rest... rest)
//    {
//        AddInputLayout(inputLayout, numberOfValues, inputType, normalizedInput);
//
//        GetInputLayoutRec(inputLayout, rest...);
//    }
//
//    template<typename... Rest>
//    void GetInputLayoutRec(std::vector<InputLayout>& inputLayout
//                           , const std::string& inputName
//                           , size_t numberOfValues
//                           , GLEnums::DATA_TYPE inputType
//                           , bool normalizedInput
//                           , int stride
//                           , int offset
//                           , Rest... rest)
//    {
//        glBindAttribLocation(shaderProgram, (GLuint)inputLayout.size(), inputName.c_str());
//
//        inputLayout.emplace_back(inputLayout.size(), numberOfValues, (GLint)inputType, normalizedInput, stride, offset);
//
//        GetInputLayoutRec(inputLayout, rest...);
//    }
//
//    template<typename... Rest>
//    void GetInputLayoutRec(std::vector<InputLayout>& inputLayout
//                           , size_t numberOfValues
//                           , GLEnums::DATA_TYPE inputType
//                           , bool normalizedInput
//                           , int stride
//                           , int offset
//                           , Rest... rest)
//    {
//        inputLayout.emplace_back(inputLayout.size(), numberOfValues, (GLint)inputType, normalizedInput, stride, offset);
//
//        GetInputLayoutRec(inputLayout, rest...);
//    }
//
//    std::vector<std::pair<GLEnums::SHADER_TYPE, GLShader*>> shaders;
//};
//
//#endif // GLSHADERPROGRAM_H__
