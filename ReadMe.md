# Autocomplete Search Engine

> A terminal-based autocomplete search engine built in C++ using a Radix Trie, BK-Tree, and personalized frequency ranking.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Demo](#demo)
- [Project Structure](#project-structure)
- [Installation](#installation)

## Overview

This project implements a complete autocomplete search engine that runs entirely in the terminal.As part of DSA course project, it demonstrates practical application of advanced data structures - Radix Trie, BK-Tree, min-heap, and hash maps - to solve a real-world problem.

As you type, suggestions update in real time. The system corrects typos, expands abbreviations, and personalizes results based on your individual usage history - all in under 0.25 milliseconds per keypress.

## Features

| Feature | Description |
|---------|-------------|
| Real-time suggestions | Updates on every keypress via termios raw mode |
| Typo correction | Handles edit distance ≤ 2 via BK-Tree |
| Abbreviation expansion | `asap` expands to `as soon as possible` |
| Personalization | Frequent selections rank higher for that user |
| Word management | Add, delete, hide words at runtime via menu |
| Persistent profiles | User history saved and loaded across sessions |
| Session score cache | Scores computed once per session, O(1) lookup |

## Demo

```
╔══════════════════════════════════════╗
║     Autocomplete Search Engine       ║
╚══════════════════════════════════════╝

Search: app_

Suggestions:
  > 1. apple
    2. application
    3. apply
    4. approach
    5. appear

  [mode: exact]  [cache: HIT]  [0.02 ms]

  TAB=fill  ↑↓=navigate  ENTER=select  M=menu  ^D=quit
```

## Project Structure

```
autocomplete/
├── core/
│   ├── TrieNode.hpp/.cpp           Radix Trie node - array[26] children, cache, pruning score
│   ├── RadixTrie.hpp/.cpp          Insert, prefix search, cache invalidation, DFS top-K
│   └── TopKCache.hpp/.cpp          Standalone min-heap top-K selection utility
├── ranking/
│   ├── Ranker.hpp/.cpp             Weighted scoring formula with session cache
│   └── UserProfile.hpp/.cpp        Per-user selection history and hidden word set
├── fuzzy/
│   ├── Levenshtein.hpp/.cpp        Bounded DP edit distance with early exit
│   ├── BKTree.hpp/.cpp             Metric space tree for O(log N) average fuzzy search
│   └── FuzzyMatcher.hpp/.cpp       Lazy BK-Tree trigger on low prefix results
├── expansion/
│   └── AbbreviationEngine.hpp/.cpp O(1) abbreviation lookup via hash map
├── io/
│   ├── DictionaryLoader.hpp/.cpp   Read and write word-frequency dictionary file
│   └── Persistence.hpp/.cpp        Serialize and deserialize user profiles
├── ui/
│   ├── TermiosController.hpp/.cpp  RAII raw mode terminal control via termios
│   ├── KeyHandler.hpp/.cpp         Character input parsing and escape sequences
│   └── Renderer.hpp/.cpp           ANSI terminal rendering - search, menu, input modes
├── data/
│   ├── dictionary.txt              500-word everyday dictionary, frequency scale 1-100
│   └── abbreviations.txt           Everyday abbreviations in pipe-delimited format
├── main.cpp                        Application state machine — SEARCH / MENU / INPUT
└── Makefile
```

## Installation

### Prerequisites

- GCC with C++17 support
- Make
- Linux or macOS (termios API required)

Check your versions:
```bash
g++ --version
make --version
```

### Build and Run

```bash
# Clone
git clone https://github.com/shah-suhani/Autocomplete-Search-Engine-.git
cd Autocomplete-Search-Engine-

# Build
make

# Run
./autocomplete
```

### Clean Build

```bash
make clean && make
```

## Usage

On startup, enter a username. Your profile is saved to `data/<username>.profile`.

```
Enter username: suhani
```

Start typing. Suggestions update after every character.

### Word Manager

Press `M` during search to open the word manager menu:

```
  A  →  Add new word to dictionary
  D  →  Delete highlighted suggestion permanently
  B  →  Add abbreviation (two-step: key then expansion)
  H  →  Hide word for your profile only
  ESC → Back to search
```

## Keybindings

| Key | Action |
|-----|--------|
| `a-z` | Append character, update suggestions |
| `↑` / `↓` | Navigate suggestion list |
| `TAB` | Fill query with highlighted suggestion |
| `ENTER` | Select word, record in profile |
| `1` – `5` | Direct select by number |
| `M` | Open word manager |
| `Ctrl+D` | Save profile and exit |

