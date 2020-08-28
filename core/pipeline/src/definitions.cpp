#include <indoors/pipeline/definitions.h>

namespace indoors::pipeline {

StandardAnnotated::StandardAnnotated(std::string annotation)
    : m_annotation{std::move(annotation)} {}

const std::string &StandardAnnotated::annotation() const {
  return m_annotation;
}

} // namespace indoors::pipeline
