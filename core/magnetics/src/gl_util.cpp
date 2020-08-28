#include <GLES3/gl3.h>
#include <indoors/magnetics/gl_util.h>
#include <iostream>

namespace indoors::magnetics {

int GLUtil::load_shader(GLint type, const std::string &source) {
  int shader = glCreateShader(type);
  if (shader == 0)
    return 0;

  const GLchar *source_cstr = source.c_str();
  glShaderSource(shader, 1, &source_cstr, nullptr);
  glCompileShader(shader);

  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == 0) {
    glDeleteShader(shader);

    // TODO
    char buffer[100000];
    int length;
    glGetShaderInfoLog(shader, 100000, &length, buffer);
    std::cout << std::string(buffer, length) << std::endl;

    throw; // TODO
  }

  return shader;
}

} // namespace indoors::magnetics
