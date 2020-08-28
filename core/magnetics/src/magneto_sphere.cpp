#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <indoors/magnetics/gl_util.h>
#include <indoors/magnetics/magneto_sphere.h>
#include <iostream>

namespace indoors::magnetics {

namespace {
const char *VERTEX_SHADER = R"(
#version 330 core

uniform mat4 MVP;

in vec3 position;
in vec4 color;

out vec4 fragmentColor;

void main() {
  gl_Position = MVP * vec4(position, 1);
  gl_PointSize = 100;
  fragmentColor = color;
}
)";

const char *FRAGMENT_SHADER = R"(
#version 330 core

in vec4 fragmentColor;

void main(){
  gl_FragColor = vec4(1, 0, 0, 1);
}
)";
} // namespace

struct MagnetoSphere::GLState {
  GLint position_handle;
  GLint color_handle;
  GLint mvp_handle;
  glm::mat4 projection;
};

MagnetoSphere::MagnetoSphere() : m_gl_state{std::make_unique<GLState>()} {}

MagnetoSphere::~MagnetoSphere() = default;

std::shared_ptr<pipeline::Input<pipeline::Event<pipeline::Vector3>>>
MagnetoSphere::create_channel(const pipeline::Vector3 &rgb) {
  auto &channel = m_channels.emplace_back(
      rgb, std::make_shared<
               pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>>(
               "magneto channel", nullptr));
  return channel.buffer;
}

void MagnetoSphere::surface_created() {
  glClearColor(1, 1, 1, 1);

  GLint program = glCreateProgram();
  if (program == 0)
    throw; // TODO

  GLint vertex_shader = GLUtil::load_shader(GL_VERTEX_SHADER, VERTEX_SHADER);
  GLint fragment_shader =
      GLUtil::load_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked == 0) {
    glDeleteProgram(program);

    // TODO
    char buffer[100000];
    int length;
    glGetProgramInfoLog(program, 100000, &length, buffer);
    std::cout << std::string(buffer, length) << std::endl;

    throw; // TODO
  }

  glUseProgram(program);

  m_gl_state->position_handle = glGetAttribLocation(program, "position");
  m_gl_state->color_handle = glGetAttribLocation(program, "color");
  m_gl_state->mvp_handle = glGetUniformLocation(program, "MVP");

  glEnable(GL_DEPTH_TEST);

  glDepthFunc(GL_LESS);
}

void MagnetoSphere::surface_changed(const std::uint32_t width,
                                    const std::uint32_t height) {
  glViewport(0, 0, width, height);
  const float aspect = 1.0f * width / height;

  m_gl_state->projection = glm::perspective(53.13f, aspect, 0.1f, 100.0f);
}

void MagnetoSphere::draw_frame() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view =
      glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  auto view_projection = m_gl_state->projection * view;

  glm::mat4 model(1.0f);

  auto model_view_projection = view_projection * model;
  auto test = model_view_projection * glm::vec4(1, 1, 0, 1);

  glUniformMatrix4fv(m_gl_state->mvp_handle, 1, false,
                     &model_view_projection[0][0]);

  const float points[] = {
      0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, -1, 0, 0, 0, -1, 0, 0, 0, -1, 1, 1, 1,
  };

  static const GLfloat cube[] = {
      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
      1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
      1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,
      1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,
      1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  -1.0f, 1.0f};

  glEnableVertexAttribArray(m_gl_state->position_handle);
  // glVertexAttribPointer(m_gl_state->position_handle, 3, GL_FLOAT, false, 0,
  // points);
  glVertexAttribPointer(m_gl_state->position_handle, 3, GL_FLOAT, false, 0,
                        cube);

  glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

  glDisableVertexAttribArray(m_gl_state->position_handle);

  glFlush();
}

MagnetoSphere::Channel::Channel(
    const pipeline::Vector3 &rgb,
    std::shared_ptr<pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>>
        buffer)
    : rgb{rgb}, buffer{std::move(buffer)} {}

} // namespace indoors::magnetics
