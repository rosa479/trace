CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
CC       = gcc
CFLAGS   = -g -O0 -no-pie

TARGET = trace
SRCS   = src/main.cpp src/debugger.cpp src/breakpoint.cpp src/registers.cpp
OBJS   = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test-targets: test/hello test/loop

test/hello: test/hello.c
	$(CC) $(CFLAGS) -o $@ $<

test/loop: test/loop.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) test/hello test/loop

.PHONY: all clean test-targets
