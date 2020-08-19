#include <gtest/gtest.h>
#include <indoors/pipeline/buffer.h>
#include <indoors/pipeline/message.h>

using namespace indoors::pipeline;

TEST(buffer, create) { Buffer<Event<Void>> buffer; }
