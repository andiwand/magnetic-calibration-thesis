#ifndef INDOORS_MAGNETICS_GL_UTIL_H
#define INDOORS_MAGNETICS_GL_UTIL_H

#include <GLES2/gl2.h>
#include <string>

namespace indoors::magnetics {

class GLUtil final {
public:
  static GLint load_shader(GLint type, const std::string &source);
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_GL_UTIL_H
