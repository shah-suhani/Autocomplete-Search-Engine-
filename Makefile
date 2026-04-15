CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic
TARGET   := autocomplete

SRCS := main.cpp \
        core/TrieNode.cpp \
        core/RadixTrie.cpp \
        core/TopKCache.cpp \
        ranking/UserProfile.cpp \
        ranking/Ranker.cpp \
        fuzzy/Levenshtein.cpp \
        fuzzy/BKTree.cpp \
        fuzzy/FuzzyMatcher.cpp \
        expansion/AbbreviationEngine.cpp \
        io/DictionaryLoader.cpp \
        io/Persistence.cpp \
        ui/TermiosController.cpp \
        ui/KeyHandler.cpp \
        ui/Renderer.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find . -name "*.o" -delete
	rm -f $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
