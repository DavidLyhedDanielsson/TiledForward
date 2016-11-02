#include <stdexcept>
#include "glUniform.h"

template<>
void GLUniform<GLfloat>::UploadData() { glUniform1f(location, data); }

template<>
void GLUniform<glm::vec2>::UploadData() { glUniform2f(location, data.x, data.y); }

template<>
void GLUniform<glm::vec3>::UploadData() { glUniform3f(location, data.x, data.y, data.z); }

template<>
void GLUniform<glm::vec4>::UploadData() { glUniform4f(location, data.x, data.y, data.z, data.w); }

template<>
void GLUniform<GLint>::UploadData() { glUniform1i(location, data); }

template<>
void GLUniform<glm::ivec2>::UploadData() { glUniform2i(location, data.x, data.y); }

template<>
void GLUniform<glm::ivec3>::UploadData() { glUniform3i(location, data.x, data.y, data.z); }

template<>
void GLUniform<glm::ivec4>::UploadData() { glUniform4i(location, data.x, data.y, data.z, data.w); }

template<>
void GLUniform<GLuint>::UploadData() { glUniform1ui(location, data); }

template<>
void GLUniform<glm::uvec2>::UploadData() { glUniform2ui(location, data.x, data.y); }

template<>
void GLUniform<glm::uvec3>::UploadData() { glUniform3ui(location, data.x, data.y, data.z); }

template<>
void GLUniform<glm::uvec4>::UploadData() { glUniform4ui(location, data.x, data.y, data.z, data.w); }

template<>
void GLUniformArray<GLfloat*>::UploadData() { glUniform1fv(location, count, data); }

template<>
void GLUniformArray<glm::vec2*>::UploadData() { glUniform2fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::vec3*>::UploadData() { glUniform3fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::vec4*>::UploadData() { glUniform4fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<GLint*>::UploadData() { glUniform1iv(location, count, data); }

template<>
void GLUniformArray<glm::ivec2*>::UploadData() { glUniform2iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
void GLUniformArray<glm::ivec3*>::UploadData() { glUniform3iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
void GLUniformArray<glm::ivec4*>::UploadData() { glUniform4iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
void GLUniformArray<GLuint*>::UploadData() { glUniform1uiv(location, count, data); }

template<>
void GLUniformArray<glm::uvec2*>::UploadData() { glUniform2uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
void GLUniformArray<glm::uvec3*>::UploadData() { glUniform3uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
void GLUniformArray<glm::uvec4*>::UploadData() { glUniform4uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
void GLUniformArray<glm::mat2x2*>::UploadData() { glUniformMatrix2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat3x3*>::UploadData() { glUniformMatrix3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat4x4*>::UploadData() { glUniformMatrix4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat2x3*>::UploadData() { glUniformMatrix2x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat3x2*>::UploadData() { glUniformMatrix3x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat2x4*>::UploadData() { glUniformMatrix2x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat4x2*>::UploadData() { glUniformMatrix4x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat3x4*>::UploadData() { glUniformMatrix3x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
void GLUniformArray<glm::mat4x3*>::UploadData() { glUniformMatrix4x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
bool GLUniform<GLfloat>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT; }

template<>
bool GLUniform<glm::vec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC2; }

template<>
bool GLUniform<glm::vec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC3; }

template<>
bool GLUniform<glm::vec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC4; }

template<>
bool GLUniform<GLint>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
bool GLUniform<glm::ivec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
bool GLUniform<glm::ivec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
bool GLUniform<glm::ivec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
bool GLUniform<GLuint>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
bool GLUniform<glm::uvec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
bool GLUniform<glm::uvec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
bool GLUniform<glm::uvec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
bool GLUniform<GLfloat*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT; }

template<>
bool GLUniform<glm::vec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC2; }

template<>
bool GLUniform<glm::vec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC3; }

template<>
bool GLUniform<glm::vec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC4; }

template<>
bool GLUniform<GLint*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
bool GLUniform<glm::ivec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
bool GLUniform<glm::ivec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
bool GLUniform<glm::ivec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
bool GLUniform<GLuint*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
bool GLUniform<glm::uvec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
bool GLUniform<glm::uvec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
bool GLUniform<glm::uvec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
bool GLUniform<glm::mat2x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2; }

template<>
bool GLUniform<glm::mat3x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3; }

template<>
bool GLUniform<glm::mat4x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4; }

template<>
bool GLUniform<glm::mat2x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X3; }

template<>
bool GLUniform<glm::mat3x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X2; }

template<>
bool GLUniform<glm::mat2x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X4; }

template<>
bool GLUniform<glm::mat4x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X2; }

template<>
bool GLUniform<glm::mat3x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X4; }

template<>
bool GLUniform<glm::mat4x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X3; }
