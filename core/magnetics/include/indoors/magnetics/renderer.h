#ifndef INDOORS_MAGNETICS_RENDERER_H
#define INDOORS_MAGNETICS_RENDERER_H

#include <cstdint>

namespace indoors::magnetics {

class Renderer {
public:
  virtual ~Renderer() = default;
  virtual void surface_created() = 0;
  virtual void surface_changed(std::uint32_t width, std::uint32_t height) = 0;
  virtual void draw_frame() = 0;
};

} // namespace indoors::magnetics

#endif // INDOORS_MAGNETICS_RENDERER_H
