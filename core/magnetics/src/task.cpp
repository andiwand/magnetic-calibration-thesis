#include <indoors/magnetics/task.h>

namespace indoors::magnetics {

StandardTask::StandardTask(std::string annotation)
    : m_annotation{std::move(annotation)} {}

const std::string &StandardTask::annotation() { return m_annotation; }

void StandardTask::start() {}

void StandardTask::stop() {}

} // namespace indoors::magnetics
