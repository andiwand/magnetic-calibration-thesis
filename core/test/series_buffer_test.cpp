#include <gtest/gtest.h>
#include <indoors/pipeline/series_buffer.h>
#include <indoors/pipeline/event.h>

using namespace indoors::pipeline;

TEST(series_buffer, create) { SeriesBuffer<Event<Void>> buffer; }
