#include <indoors/pipeline/synchronizer.h>

namespace indoors::pipeline {

Synchronizer::Synchronizer(const double max_slave_delay)
    : Synchronizer("synchronizer", max_slave_delay) {}

Synchronizer::Synchronizer(std::string annotation, const double max_slave_delay)
    : m_annotation{std::move(annotation)}, m_max_slave_delay{max_slave_delay} {}

void Synchronizer::iterate(std::uint32_t) {
  // TODO
}

} // namespace indoors::pipeline
