# Tau Programming Language

Tau is a modern, JavaScript-inspired programming language, implemented in C++. It features block scoping, classes and inheritance, closures, rich expressions, built-in error handling, customizable standard library, and a flexible interpreter architecture. This documentation provides a comprehensive guide for Tau, including language syntax, semantics, and usage, with detailed examples for every concept.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Language Overview](#language-overview)
    - [Variables](#variables)
    - [Data Types & Literals](#data-types--literals)
    - [Expressions and Operators](#expressions-and-operators)
    - [Functions](#functions)
    - [Control Flow](#control-flow)
    - [Classes & Inheritance](#classes--inheritance)
    - [Error Handling](#error-handling)
    - [Modules & Standard Library](#modules--standard-library)
    - [Arrays & Objects](#arrays--objects)
3. [Reference](#reference)
    - [Keywords](#keywords)
    - [Operators](#operators)
    - [Statements](#statements)
    - [Special Values](#special-values)
4. [Complete Example](#complete-example)
5. [Extending Tau](#extending-tau)
6. [Frequently Asked Questions](#frequently-asked-questions)

---

## Getting Started

### Installation

Tau is a C++ project. Build it with CMake or your preferred C++ toolchain.

```sh
git clone https://github.com/yourusername/tau-programming-lang.git
cd tau-programming-lang
mkdir build && cd build
cmake ..
make
