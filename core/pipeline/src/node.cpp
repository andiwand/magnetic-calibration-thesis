#include <indoors/pipeline/node.h>

namespace indoors::pipeline {

StandardNode::StandardNode(std::string annotation)
    : m_annotation{std::move(annotation)} {}

const std::string &StandardNode::annotation() const {
  return m_annotation;
}

StandardInterface::StandardInterface(std::string annotation, Node *node)
    : m_annotation{std::move(annotation)}, m_node{node} {}

const std::string &StandardInterface::annotation() const {
  return m_annotation;
}

Node *StandardInterface::node() const { return m_node; }

bool StandardInterface::open() const { return m_open; }

double StandardInterface::time() const { return m_time; }

std::size_t StandardInterface::message_count() const {
  return m_message_count;
}

} // namespace indoors::pipeline
