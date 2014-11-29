stamp
=====

                    ____    _____      _      __  __   ____
                   / ___|  |_   _|    / \    |  \/  | |  _ \
                   \___ \    | |     / _ \   | |\/| | | |_) |
                    ___) |   | |    / ___ \  | |  | | |  __/
                   |____/    |_|   /_/   \_\ |_|  |_| |_|

            Stanford Transactional Applications for Multi-Processing

                           http://stamp.stanford.edu
               Announce List: stamp-announce@lists.stanford.edu
                  General List: stamp-users@lists.stanford.edu
                 Contact: stamp-users-owner@lists.stanford.edu


___Introduction___

The Stanford Transactional Applications for Multi-Processors (STAMP) is a
collection of applications well suited for transactional memory research. For
each application, STAMP includes sequential code, parallel code that uses
coarse-grain transactions, and reference data sets. We provide transactional
code for both HTM and STM systems, and provide an STM system based on TL2 [3]. A
characterization of the applications is given in [1] and [2].

We are currently working on additional STAMP applications and welcome your
feedback, corrections, and suggestions. If you make some improvements to STAMP,
we would appreciate receiving a copy that we can include in the next release.

If you use STAMP in your work, please cite [1]. Thanks for using STAMP!


___Distribution Contents___

This directory contains the following items:

    AUTHORS ----- A list of people who have contributed to STAMP
    LICENSE ----- BSD-style license; if you use STAMP, please let us know
    README ------ This file
    VERSIONS ---- Revision history
    bayes/ ------ Bayesian network structure learning benchmark  
    common/ ----- Common Makefile variables and rules
    labyrinth/ -- Maze routing benchmark
    lib/ -------- Common libraries (data structures, etc.)
    genome/ ----- Gene sequencing benchmark
    intruder/ --- Network intrusion detectino benchmark
    kmeans/ ----- K-means clustering benchmark
    ssca2/ ------ Graph kernel benchmark
    vacation/ --- Travel reservation system benchmark
    yada/ ------- Delaunay mesh refinement benchmark

Each of the benchmarks contains a README file that has a description of the
program and inputs and instructions for compilation and running. There are
different Makefiles to build different flavors (e.g., sequential, stm, etc.).

To adapt the benchmarks for a particular TM system, change lib/tm*. These files
contain documentation on the purpose and usage of each of the macros.

For general compilation changes (e.g., choice of compiler), edit common/*.


___Platforms___

STAMP has been tested on Ubuntu 6, Ubuntu 7, Fedora Core 5, Fedora Core 6,
CentOS 4, and CentOS 5, on both 32-bit i386 and 64-bit x86_64 architectures.


___References___

[1] C. Cao Minh, J. Chung, C. Kozyrakis, and K. Olukotun. STAMP: Stanford 
    Transactional Applications for Multi-processing. In IISWC '08: Proceedings
    of The IEEE International Symposium on Workload Characterization,
    September 2008. 

[2] C. Cao Minh, M. Trautmann, J. Chung, A. McDonald, N. Bronson, J. Casper,
    C. Kozyrakis, and K. Olukotun. An Effective Hybrid Transactional Memory
    System with Strong Isolation Guarantees. In Proceedings of the 34th Annual
    International Symposium on Computer Architecture, 2007.

[2] D. Dice, O. Shalev, and N. Shavit. Transactional Locking II. In
    Proceedings of the 20th International Symposium on Distributed Computing
    (DISC), 2006.
