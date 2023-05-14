# Parallel-File-Processing
Processes input files in parallel (written in C). This implementation simply counts the number of words in each text file, but the format could be extended to more meaningful work.

This program uses pipes for parallel computation - one process for each file. Using processes for parallel computation eats up more system resources (memory, OS overhead, etc.) than threads, but processes offer more safety via isolation between tasks.

#### For an example of parallel computation using multithreading, see my multithreaded http server: https://github.com/JacksonKary/Multithreaded-HTTP-Server

## This project uses a number of important systems programming topics:

- Text file input and parsing
- Creating pipes with the pipe() system call
- Using pipe file descriptors in combination with read() and write() to coordinate among several processes

## What is in this directory?
<ul>
  <li>  <code>par_word_lengths.c</code> : Implementation of parallel word counting program.
  <li>  <code>Makefile</code> : Build file to compile and run test cases.
  <li>  <code>test_cases</code> Folder, which contains:
  <ul>
    <li>  <code>input</code> : Input files used in automated testing cases.
    <li>  <code>resources</code> : More input files.
    <li>  <code>output</code> : Expected output.
  </ul>
  <li>  <code>testius</code> : Python script that runs the tests.
</ul>

## Running Tests

A Makefile is provided as part of this project. This file supports the following commands:

<ul>
  <li>  <code>make</code> : Compile all code, produce an executable program.
  <li>  <code>make clean</code> : Remove all compiled items. Useful if you want to recompile everything from scratch.
  <li>  <code>make clean-tests</code> : Remove all files produced during execution of the tests.
  <li>  <code>make zip</code> : Create a zip file for submission to Gradescope.
  <li>  <code>make test</code> : Run all test cases.
  <li>  <code>make test testnum=5</code> : Run test case #5 only.
</ul>
