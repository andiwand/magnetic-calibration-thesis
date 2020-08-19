#include <gtest/gtest.h>
#include <indoors/pipeline/csv.h>

using namespace indoors::pipeline;

TEST(csv, hello) {
  std::make_shared<CsvWriter>()->create_input<Event<Void>>("clock")->push({1});

  std::make_shared<CsvReader>()->create_output<Event<Void>>("clock");
}
