#  OSINT Username Discovery System

##  Overview

This project aims to build an intelligent **OSINT (Open-Source Intelligence) tool** for discovering and analyzing usernames across platforms.

Instead of relying on exact matches (like traditional tools such as Sherlock), this system focuses on:

* Finding **similar usernames**
* Understanding **naming patterns**
* Generating **probable variations**
* Building **connections between identities**

---

##  Objectives

* Efficiently store and query large sets of usernames
* Provide fast **prefix-based suggestions**
* Enable **fuzzy matching** (future)
* Discover patterns in username construction
* Reduce empty or irrelevant OSINT results

---

##  System Architecture

The system is composed of multiple layers:

### 1. Trie (Core Engine)

A prefix tree is used to store normalized usernames.

#### Responsibilities:

* Fast insertion of usernames
* Prefix lookup (autocomplete)
* Traversal for suggestion generation

#### Features:

* Custom character mapping (`a-z`, `0-9`, `_`, `.`)
* Prefix count tracking (`pfx`)
* Word termination tracking (`end`)

---

### 2. Normalization Layer

All usernames are processed before insertion:

#### Rules:

* Convert to lowercase
* Standardize separators (`_`, `.`, etc.)
* Ignore unsupported characters

#### Purpose:

* Avoid duplication (`Honda` vs `honda`)
* Improve matching consistency

---

### 3. Search & Suggestion Engine

#### Prefix Search:

* Navigate trie to prefix node
* Perform DFS to collect results

#### DFS Traversal:

* Builds usernames dynamically
* Stops after **top-K results** (performance optimization)

---

### 4. Pattern Analysis (Planned)

Instead of random generation, patterns will be learned from data:

Examples:

* Suffixes: `123`, `_yt`, `_dev`
* Prefixes: `real_`, `official_`

#### Goal:

Generate **realistic username variations**, not random guesses

---

### 5. Graph Layer (Optional / Future)

Usernames can be modeled as a graph:

* Nodes → usernames
* Edges → similarity (edit distance, pattern match)

This allows:

* Identity clustering
* Relationship discovery
* Multi-platform linking

Traversal algorithms such as **BFS/DFS** can be used for exploration.

---

## used data structures
### Trie

* Insert usernames
* Retrieve prefix node
* Perform DFS traversal

---

### Graph 

* Adjacency list representation
* BFS / DFS traversal
* Parent & distance tracking

---

##  Features

###  Implemented

* Trie structure
* Username insertion
* Prefix-based traversal foundation
* Character normalization

###  Planned

* Prefix search API
* DFS suggestions with limit
* Pattern-based generation
* Fuzzy matching (edit distance)
* Ranking system (frequency-based)
* Hash table for originals
---

##  Performance Considerations

* Trie enables **O(L)** insert/search
* DFS limited to top-K results
* Memory optimized via fixed-size arrays
* Future upgrade: **compressed trie (radix tree)**

---

## Limitations

* Currently limited character set
* No fuzzy matching yet
* No external data integration
* Memory usage grows with dataset size

---

##  Future Improvements

* Levenshtein distance integration
* BK-tree or fuzzy trie traversal
* Platform availability checks
* Caching frequent queries
* UI / API layer

---

##  Key Insight

The system is built on the idea that:

> Usernames are not random — they follow patterns.

By combining:

* structured storage (Trie)
* intelligent traversal (DFS/A* in future)
* and learned patterns

we move from:

> searching usernames

to:

> understanding digital identity behavior

---

##  References

* Sherlock – baseline OSINT tool
* Trie data structures
* Graph traversal algorithms (BFS, DFS)
* String matching techniques

---

## Conclusion

This project evolves OSINT from simple lookup tools into a **pattern-aware identity discovery system**, capable of uncovering connections that traditional tools miss.

---
