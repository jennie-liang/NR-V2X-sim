CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

TARGET   := v2xsim
BUILDDIR := build
SRCS     := $(wildcard src/*.cpp)
OBJS     := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)

.PHONY: all clean run debug sweep

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: src/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)
	@mkdir -p results

debug: CXXFLAGS := $(filter-out -O2,$(CXXFLAGS)) -O0 -g -fsanitize=address,undefined
debug: LDFLAGS  += -fsanitize=address,undefined
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET) --ues 50 --duration 10000

sweep: $(TARGET)
	@bash scripts/sweep.sh

clean:
	@rm -rf $(BUILDDIR) $(TARGET)

-include $(DEPS)
