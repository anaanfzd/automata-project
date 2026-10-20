# AutomataViz — Interactive Regular Expression to DFA Visualizer

> **A Complete Theory of Computation Course Capstone Project**  
> *Deterministic Finite Automata (DFA), Non-Deterministic Finite Automata (NFA), ε-NFA, Regular Expressions, Equivalence of NFA and DFA, DFA Minimization, and String Simulation.*

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![CMake](https://img.shields.io/badge/CMake-3.16+-brightgreen.svg)](https://cmake.org/)
[![SFML](https://img.shields.io/badge/SFML-2.6.1-green.svg)](https://www.sfml-dev.org/)
[![Tests](https://img.shields.io/badge/Tests-68%20Passed%20(100%25)-success.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

---

## 1. Executive Summary & Problem Statement

### 1.1 Problem Statement
In undergraduate Computer Science curricula, **Theory of Computation (TOC) / Formal Languages & Automata Theory (FLAT)** is often perceived as abstract and mathematically daunting. Students struggle to build mental models of dynamic automaton transformations, specifically:
1. How infix regular expressions convert into postfix notation via precedence-aware tokenization.
2. How Thompson's construction recursively synthesizes ε-NFA fragments without introducing spurious cycles.
3. How the Subset Construction algorithm resolves non-determinism by computing powerset subsets via ε-closures and move operations.
4. How partition refinement (Hopcroft/Moore) detects behavioral indistinguishability to merge redundant DFA states.
5. How a minimized DFA traces input strings character-by-character to decide language membership.

Commercial tools and command-line utilities (like Lex/Flex or grep) perform these transformations as a "black box", obscuring the intermediate mathematical states.

### 1.2 Objectives
**AutomataViz** is an interactive, pedagogical finite automata learning workbench engineered from scratch in modern **C++17** and **SFML**. It dynamically executes and animates every phase of the classic automata pipeline:

$$\text{Regular Expression} \xrightarrow[\text{Shunting-Yard}]{\text{AST / Postfix}} \varepsilon\text{-NFA} \xrightarrow[\text{Subset Construction}]{\text{Powerset}} \text{DFA} \xrightarrow[\text{Partition Refinement}]{\text{Hopcroft / Moore}} \text{Minimized DFA} \xrightarrow{\text{Execution}} \text{String Decision}$$

---

## 2. Theoretical Foundations (Unit 1 TOC Syllabus)

### 2.1 Stage 1: Regular Expressions & Infix-to-Postfix Parsing
* **Definition**: An algebraic representation of a regular language over an alphabet $\Sigma$.
* **Grammar & Operators**:
  * Atom: Symbol $a \in \Sigma$, Epsilon $\varepsilon$, or Empty Set $\emptyset$.
  * Union / Alternation ($a | b$): $L(a|b) = L(a) \cup L(b)$ (Lowest precedence, binary infix).
  * Concatenation ($a \cdot b$): $L(a \cdot b) = L(a) L(b)$ (Intermediate precedence, binary infix).
  * Kleene Star ($a^*$): $L(a^*) = \bigcup_{i=0}^\infty L(a)^i$ (Highest precedence, unary postfix).
  * Extended operators: Plus ($a^+$ = $a \cdot a^*$), Optional ($a?$ = $a | \varepsilon$).
* **Conversion Algorithm**: Shunting-Yard Algorithm adapted for explicit concatenation insertion.

### 2.2 Stage 2: Thompson's Construction (RE $\to$ $\varepsilon$-NFA)
* **What it is**: An inductive syntax-directed algorithm that constructs an $\varepsilon$-NFA accepting $L(R)$.
* **Invariants**:
  1. Exactly one initial state with in-degree 0.
  2. Exactly one final/accepting state with out-degree 0.
  3. Every state has at most two outgoing transitions.
  4. Total states created is bounded by $2 \times |Tokens|$.
* **Fragment Construction Rules**:
  * **Symbol $a$**: $s \xrightarrow{a} f$ (2 states, 1 transition).
  * **Concatenation ($A \cdot B$)**: Connect exit of $A$ to entry of $B$ via $\varepsilon$: $f_A \xrightarrow{\varepsilon} s_B$.
  * **Union ($A | B$)**: Create new start $s_{new}$ and exit $f_{new}$. $s_{new} \xrightarrow{\varepsilon} s_A$, $s_{new} \xrightarrow{\varepsilon} s_B$, $f_A \xrightarrow{\varepsilon} f_{new}$, $f_B \xrightarrow{\varepsilon} f_{new}$.
  * **Kleene Star ($A^*$)**: Create new $s_{new}$ and $f_{new}$. $s_{new} \xrightarrow{\varepsilon} s_A$, $s_{new} \xrightarrow{\varepsilon} f_{new}$ (bypass), $f_A \xrightarrow{\varepsilon} s_A$ (loopback), $f_A \xrightarrow{\varepsilon} f_{new}$.

### 2.3 Stage 3: Equivalence of NFA and DFA & Subset Construction
* **Theorem**: For every NFA $N = (Q_N, \Sigma, \Delta, q_0, F_N)$, there exists a DFA $D = (Q_D, \Sigma, \delta, q_{0,D}, F_D)$ such that $L(D) = L(N)$.
* **Subset Construction Operations**:
  * $\varepsilon\text{-closure}(T)$: Set of NFA states reachable from $T \subseteq Q_N$ using only $\varepsilon$-transitions (computed via BFS/DFS).
  * $\text{move}(T, a)$: $\{ q \in Q_N \mid \exists p \in T \text{ such that } p \xrightarrow{a} q \}$.
  * Next state: $\delta_D(S, a) = \varepsilon\text{-closure}(\text{move}(S, a))$.
  * Accepting condition: $S \in F_D \iff S \cap F_N \neq \emptyset$.

### 2.4 Stage 4: DFA Minimization (Partition Refinement)
* **Myhill-Nerode Theorem**: Every regular language has a unique minimal DFA up to isomorphism.
* **Algorithm**:
  1. Remove all unreachable states from $q_{0,D}$.
  2. Initial 0-equivalence partition: $P_0 = \{ Q_D \setminus F_D, F_D \}$.
  3. Refinement: States $p, q$ in partition group $G$ remain in the same group in $P_{k+1}$ iff $\forall a \in \Sigma$, $\delta(p, a)$ and $\delta(q, a)$ belong to the same group in $P_k$.
  4. Fixed point: When $P_{k+1} = P_k$, merge states in each group into a single minimal state.

---

## 3. System Architecture & Modular Design

```
AutomataViz/
├── CMakeLists.txt              # Standard cross-platform CMake build configuration
├── Makefile                    # Standalone GNU Make / MinGW build system
├── build.bat                   # One-click Windows build script
├── run.bat                     # Windows launch runner
├── README.md                   # Complete academic documentation & viva guide
│
├── include/
│   ├── Regex/
│   │   ├── Token.h             # Token definitions & operator precedence
│   │   ├── RegexValidator.h    # Comprehensive syntax & parenthesis validation
│   │   └── RegexParser.h       # Shunting-yard parser with explicit concatenation
│   │
│   ├── Automata/
│   │   ├── State.h             # State model, IDs, labels, and NFA subsets
│   │   ├── Transition.h        # Directed edge model with symbol & epsilon handling
│   │   ├── NFA.h               # Formal NFA (Q, Σ, Δ, q0, F) with ε-closure & move
│   │   └── DFA.h               # Formal DFA (Q, Σ, δ, q0, F) with simulation & tables
│   │
│   ├── Algorithms/
│   │   ├── ThompsonConstruction.h  # Dynamic RE -> ε-NFA construction
│   │   ├── SubsetConstruction.h    # Powerset construction with intermediate steps
│   │   ├── DFAMinimization.h       # Partition refinement minimization & group merging
│   │   └── StringSimulator.h       # Interactive string execution trace engine
│   │
│   ├── Visualization/
│   │   ├── Node.h              # Visual node geometry, outlines, double rings, labels
│   │   ├── Edge.h              # Curved bidirectional edges, self-loops, arrowheads
│   │   ├── GraphLayout.h       # Topological layered DAG + Force-Directed relaxation
│   │   ├── GraphRenderer.h     # High-precision anti-aliased SFML graph renderer
│   │   └── Animation.h         # Automated step timer & pulse controller
│   │
│   └── UI/
│       ├── UITheme.h           # Curated modern academic dark color palette
│       ├── Controls.h          # Button, TextInput, and TabBar widgets
│       ├── InfoPanel.h         # Scrollable step-by-step mathematical explanation panel
│       ├── TableView.h         # Formatted DFA transition matrix grid view
│       └── MainWindow.h        # Application lifecycle, layout, and event dispatcher
│
├── src/
│   ├── Regex/                  # Regex parsing & validation implementations
│   ├── Automata/               # NFA & DFA mathematical implementations
│   ├── Algorithms/             # Thompson, Subset, Minimization & Simulator engines
│   ├── Visualization/          # Layout algorithms & SFML rendering
│   ├── UI/                     # UI controls, side panels, and window loop
│   └── main.cpp                # Desktop application entry point
│
└── tests/
    └── test_runner.cpp         # 68 automated verification tests (100% passing)
```

---

## 4. Complexity Analysis

| Algorithm | Stage | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| **Regex Validation** | Preprocessing | $O(N)$ | $O(N)$ |
| **Shunting-Yard Parser** | Infix $\to$ Postfix | $O(N)$ | $O(N)$ |
| **Thompson's Construction** | RE $\to$ $\varepsilon$-NFA | $O(N)$ | $O(N)$ states, $O(N)$ transitions |
| **Epsilon Closure** | Subset Construction | $O(\|Q_{NFA}\| + \|\Delta_{NFA}\|)$ | $O(\|Q_{NFA}\|)$ |
| **Subset Construction** | $\varepsilon$-NFA $\to$ DFA | $O(2^{\|Q_{NFA}\|} \cdot \|\Sigma\| \cdot \|Q_{NFA}\|^2)$ | $O(2^{\|Q_{NFA}\|})$ |
| **DFA Minimization** | Hopcroft/Moore | $O(\|\Sigma\| \cdot \|Q_{DFA}\|^2)$ | $O(\|Q_{DFA}\|)$ |
| **DFA String Simulation** | Verification | $O(M)$ for string length $M$ | $O(1)$ |
| **Force-Directed Layout** | Visualization | $O(I \cdot \|Q\|^2)$ for $I$ iterations | $O(\|Q\|)$ |

---

## 5. Build & Installation Guide

AutomataViz is engineered to build seamlessly on Windows (MSYS2, MinGW, Visual Studio), Linux, and macOS.

### Prerequisites
1. **C++17 Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
2. **SFML 2.5 or 2.6**: Simple and Fast Multimedia Library (`sfml-graphics`, `sfml-window`, `sfml-system`)
3. **CMake 3.16+** (or use the included `build.bat` / `Makefile`)

### Option A: Native Windows Scripts (`build.ps1` / `build.bat`)
In PowerShell (recommended):
```powershell
.\build.ps1     # Builds both the main application and test suite
.\run.ps1       # Launches the high-performance AutomataViz visualizer
```
Or in Command Prompt:
```cmd
build.bat
run.bat
```

### Option B: CMake Build (Cross-Platform)
```bash
# 1. Create build directory
mkdir build
cd build

# 2. Configure project
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. Compile
cmake --build .

# 4. Run automated tests
ctest --output-on-failure

# 5. Launch application
./AutomataViz
```

### Option C: Standalone Makefile
```bash
make -j4
./bin/AutomataViz_Tests.exe
./bin/AutomataViz.exe
```

---

## 6. Verification & Automated Test Results

AutomataViz includes a headless verification test harness in `tests/test_runner.cpp` with **68 automated assertions** covering every syllabus topic:

```
========================================================
  AutomataViz Automated Algorithm & Verification Suite
========================================================

=== 1. Regex Validation Tests ===
  [PASS] Valid single symbol 'a'
  [PASS] Valid union 'a|b'
  [PASS] Valid concatenation 'ab'
  [PASS] Valid Kleene star 'a*'
  [PASS] Valid parenthesized star '(a|b)*'
  [PASS] Valid complex regex '(a|b)*abb'
  [PASS] Valid nested parens '((a|b)*c)+'
  [PASS] Reject empty string
  [PASS] Reject unmatched opening '('
  [PASS] Reject unmatched closing ')'
  [PASS] Reject empty parentheses '()'
  [PASS] Reject leading union '|a'
  [PASS] Reject trailing union 'a|'
  [PASS] Reject consecutive union 'a||b'
  [PASS] Reject leading Kleene star '*a'
  [PASS] Reject subexpression starting with '|'

=== 2. Tokenization & Postfix Conversion Tests ===
  [PASS] Infix 'ab' -> Postfix 'ab.'
  [PASS] Infix 'a|b' -> Postfix 'ab|'
  [PASS] Infix 'a*' -> Postfix 'a*'
  [PASS] Infix '(a|b)*' -> Postfix 'ab|*'
  [PASS] Infix '(a|b)*abb' -> Postfix 'ab|*a.b.b.'

=== 3. Thompson Construction Tests ===
  [PASS] Thompson NFA generated for 'a'
  [PASS] Valid start state for 'a'
  [PASS] Non-empty final states for 'a'
  [PASS] State count >= 2 for 'a'
  [PASS] Thompson NFA generated for 'a|b'
  [PASS] Valid start state for 'a|b'
  [PASS] Non-empty final states for 'a|b'
  [PASS] State count >= 2 for 'a|b'
  [PASS] Thompson NFA generated for 'ab'
  [PASS] Valid start state for 'ab'
  [PASS] Non-empty final states for 'ab'
  [PASS] State count >= 2 for 'ab'
  [PASS] Thompson NFA generated for 'a*'
  [PASS] Valid start state for 'a*'
  [PASS] Non-empty final states for 'a*'
  [PASS] State count >= 2 for 'a*'
  [PASS] Thompson NFA generated for '(a|b)*'
  [PASS] Valid start state for '(a|b)*'
  [PASS] Non-empty final states for '(a|b)*'
  [PASS] State count >= 2 for '(a|b)*'
  [PASS] Thompson NFA generated for '(a|b)*abb'
  [PASS] Valid start state for '(a|b)*abb'
  [PASS] Non-empty final states for '(a|b)*abb'
  [PASS] State count >= 2 for '(a|b)*abb'

=== 4. Subset Construction & DFA Minimization Tests ===
  [PASS] Subset construction succeeded for '(a|b)*abb'
  [PASS] DFA has states
  [PASS] DFA has start state
  [PASS] DFA has accepting states
  [PASS] DFA Minimization succeeded for '(a|b)*abb'
  [PASS] Minimized states <= Original states
  [PASS] Canonical minimized state count for '(a|b)*abb' is 4

=== 5. String Acceptance Simulation Tests ===
  [PASS] Accept 'abb'
  [PASS] Accept 'aabb'
  [PASS] Accept 'ababb'
  [PASS] Accept 'babb'
  [PASS] Accept 'ababbabb'
  [PASS] Accept 'bbabb'
  [PASS] Reject 'ab' (missing final 'b')
  [PASS] Reject 'aaa'
  [PASS] Reject empty string ''
  [PASS] Reject 'b'
  [PASS] Reject 'abba'
  [PASS] a* accepts empty string ''
  [PASS] a* accepts 'a'
  [PASS] a* accepts 'aaaa'
  [PASS] a* rejects 'b'
  [PASS] a* rejects 'ab'

========================================================
  Test Results: 68 / 68 Passed (100%)
========================================================
```

---

## 7. User Interface & Interactive Features

1. **Top Navigation Bar**:
   * Text Input for custom regular expressions with instant validation.
   * `[Generate ▶]` button compiles the entire pipeline.
   * Preset Example Buttons: `(a|b)*abb`, `a*`, `(a|b)*`, `(a|b)(a|b)`, `a(b|c)*d`.
2. **Pipeline Tab Switcher**:
   * **Tab 1: Postfix / AST**: Token stream, operator precedence table, and step-by-step Shunting-Yard trace.
   * **Tab 2: ε-NFA (Thompson)**: Step-by-step animation of NFA fragments with newly created states/edges highlighted.
   * **Tab 3: Subset Construction**: Powerset conversion with intermediate $\varepsilon\text{-closure}$ and $\text{move}$ steps. Includes **Split View (Side-by-Side NFA & DFA)** highlighting active NFA state subsets in real time.
   * **Tab 4: DFA (Graph/Table)**: Complete DFA state graph with one-click toggle to a formatted transition table.
   * **Tab 5: Minimized DFA**: Partition refinement rounds, split explanations, merged state mappings, and reduction statistics ($N_{orig} \to N_{min}$, $T_{orig} \to T_{min}$).
   * **Tab 6: String Simulation**: Character-by-character tape simulation with active state glow, pointer head, and green `ACCEPTED ✓` / red `REJECTED ✗` badges.
   * **Tab 7: TOC Course Guide**: Comprehensive in-app textbook reference covering Unit 1 theory.
3. **Step & Playback Controls**:
   * `[◀ Prev Step]`, `[Next Step ▶]`, `[▶ Auto Play / ⏸ Pause]`, `[↺ Reset]`.
   * `[Speed: 1x / 2x / 4x / 0.5x]`: Adjustable animation playback speed.
   * `[Split View: ON / OFF]`: Side-by-side NFA & DFA comparison mode during Subset Construction.
4. **Canvas Navigation & Direct Node Manipulation**:
   * **Interactive Node Dragging**: Left-click and drag ANY state circle to move it freely and untangle complex graphs in real time.
   * **Node Hover Glow**: Hovering over states provides visual feedback and focus indicators.
   * **Smooth Zoom & Pan**: Mouse wheel zoom centered on cursor; Left/Middle click drag on background canvas to pan.
   * `[Fit View]`: Instantly centers and scales the automaton to fit the viewport.
   * **Zero-Lag Batched Rendering**: Single draw-call point-grid, zero heap reallocations during animation, and hardware VSync with up to 120+ FPS.

---

## 8. Academic Viva Voce Question Bank (For Examinations)

### Q1: Why is Thompson's Construction preferred over McNaughton-Yamada-Glushkov for regular expression compilation?
**Answer**: Thompson's construction runs in linear time $O(|R|)$ and produces an $\varepsilon$-NFA with at most $2|R|$ states and $4|R|$ transitions, where every state has an out-degree of at most 2. This strict structural regularity simplifies modular fragment composition. Glushkov's algorithm constructs an $\varepsilon$-free NFA directly but requires computing First, Last, and Follow sets with $O(|R|^2)$ transition complexity.

### Q2: What is the significance of the $\varepsilon$-closure operation in Subset Construction?
**Answer**: The $\varepsilon$-closure of a state set $T$ computes all states reachable from $T$ without reading any input symbol ($\varepsilon$-transitions). In a DFA, reading a symbol must correspond to a single deterministic state change; thus, every DFA state represents an entire equivalence set of NFA states that the non-deterministic machine could simultaneously occupy.

### Q3: Why does DFA minimization guarantee a unique minimum state machine?
**Answer**: By the **Myhill-Nerode Theorem**, the number of states in a minimal DFA equals the index (number of equivalence classes) of the right-invariant equivalence relation $R_L$ defined by $x \, R_L \, y \iff (\forall z \in \Sigma^*, xz \in L \iff yz \in L)$. Because the equivalence classes of $R_L$ depend strictly on the language $L$ and not on any specific automaton representation, the minimal DFA is unique up to state relabeling (isomorphism).

### Q4: What is the worst-case space complexity of Subset Construction, and when does it occur?
**Answer**: The theoretical worst-case is $O(2^n)$ states, where $n$ is the number of NFA states. This occurs for languages such as $L_k = (a|b)^* a (a|b)^{k-1}$ (strings where the $k$-th symbol from the end is 'a'). While the NFA requires only $k+1$ states, any equivalent DFA must remember the last $k$ symbols, necessitating exactly $2^k$ states.

---

## 9. Future Scope
* Extension to **Glushkov's Algorithm** (Position Automaton) for direct RE $\to$ NFA without $\varepsilon$.
* **Brzozowski's Algorithm** for DFA minimization via reversal and double powerset construction.
* Export automata graphs directly to **Graphviz DOT** and **SVG/PNG** formats.
* Support for Pushdown Automata (PDA) for Context-Free Grammars in Unit 2.

---

## 10. License & Academic Attribution
Developed as a Theory of Computation (TOC) College Capstone Project.  
Released under the **MIT License**. Free for educational, classroom, and academic demonstration use.
