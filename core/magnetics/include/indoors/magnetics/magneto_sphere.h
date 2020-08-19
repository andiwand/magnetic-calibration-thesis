#ifndef INDOORS_MAGNETICS_MAGNETO_SPHERE_H
#define INDOORS_MAGNETICS_MAGNETO_SPHERE_H

#include <indoors/magnetics/renderer.h>
#include <indoors/pipeline/node.h>
#include <indoors/pipeline/message.h>
#include <vector>

namespace indoors::magnetics {

class MagnetoSphere : public Renderer {
public:
  MagnetoSphere();
  ~MagnetoSphere() override;

  std::shared_ptr<pipeline::Input<pipeline::Event<pipeline::Vector3>>>
  create_channel(const pipeline::Vector3 &rgb);

  void surface_created() override;
  void surface_changed(std::uint32_t width, std::uint32_t height) override;
  void draw_frame() override;

private:
  struct GLState;
  struct Channel;

  std::unique_ptr<GLState> m_gl_state;
  std::vector<Channel> m_channels;

  struct Channel {
    pipeline::Vector3 rgb;
    std::shared_ptr<pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>> buffer;

    Channel(const pipeline::Vector3 &rgb,
            std::shared_ptr<pipeline::BufferedInput<pipeline::Event<pipeline::Vector3>>>
                buffer);
  };
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_MAGNETO_SPHERE_H
