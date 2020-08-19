#include <indoors/pipeline/worker.h>

namespace indoors::pipeline {

Worker::Worker(std::shared_ptr<Runable> runable)
    : m_runable{std::move(runable)}, m_thread{&Runable::run, m_runable.get()} {}

Worker::~Worker() { m_thread.join(); }

} // namespace indoors::pipeline
