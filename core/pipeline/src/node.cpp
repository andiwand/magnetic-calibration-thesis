#include <indoors/pipeline/node.h>

namespace indoors::pipeline {

StandardNode::StandardNode(std::string annotation) noexcept
    : m_annotation{std::move(annotation)} {}

const std::string &StandardNode::annotation() const noexcept {
  return m_annotation;
}

StandardInterface::StandardInterface(std::string annotation, Node *node) noexcept
    : m_annotation{std::move(annotation)}, m_node{node} {}

const std::string &StandardInterface::annotation() const noexcept {
  return m_annotation;
}

Node *StandardInterface::node() const noexcept { return m_node; }

bool StandardInterface::open() const noexcept { return m_open; }

double StandardInterface::time() const noexcept { return m_time; }

std::size_t StandardInterface::message_count() const noexcept {
  return m_message_count;
}

} // namespace indoors::pipeline
