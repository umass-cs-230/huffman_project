# COMPSCI 230 Computer Systems Principles
# Huffman Coding

[starter code]: https://github.com/umass-cs-230/project-huffman-s18/archive/master.zip

## Overview

This project assignment will exercise your understanding of bit-level
manipulation in C, allocations and manipulation of array and
pointer-linked data structures in C, and working with larger C
programs. Make sure you follow the instructions carefully.  The actual
code you write is fairly short.  However, the details are quite
precise.  Programming errors often result from slight differences that
are hard to detect -- so be careful and understand exactly what the
exercises are asking you to do.

The goal of this project is to complete the implementation of two
programs using the [Huffman coding algorithm][huffman] for file
compression and decompression.  In particular, the first program
`huffc` will take as input a text file in ASCII format and generate a
compressed version of that file as output:

```bash
$ ./huffc books/holmes.txt holmes.he
```

The program will not produce any output to the console, but it will
create a new file called `holmes.he` that is a compressed form of the
original input file `books/holmes.txt`. In our implementation this
results in the following file sizes:

```bash
$ ls -lh books/holmes.txt
-rw-r--r-- 1 vagrant vagrant 14M Oct 10 17:45 books/holmes.txt
$ ls -lh holmes.he
-rw-r--r-- 1 vagrant vagrant 7.2M Oct 14 18:51 holmes.he
```

As you can see, the `huffc` program was able to compress the original input
text file by almost 50%.  Since compression is not really interesting unless we
can decompress a compressed file, we can then run the decompression program
`huffd` to convert the file back to its original form:

```bash
$ ./huffd holmes.he holmes.txt
```

Note, that we did not overwrite the original text file in the books
directory!  Be careful that you do not do this!  We can then see if
our decompressed file is the same size as the original text file:

```bash
$ ls -lh books/holmes.txt
-rw-r--r-- 1 vagrant vagrant 14M Oct 10 17:45 books/holmes.txt
$ ls -lh holmes.txt
-rw-r--r-- 1 vagrant vagrant 14M Oct 14 18:55 holmes.txt
```

Although this compares the size, it is even more important that they
contain the **exact same** bytes.  This is an easy test using the
`diff` tool:

```bash
$ diff books/holmes.txt holmes.txt
```

The `diff` tool should return without printing anything to the
console.  If it shows differences, you know something went wrong!
(Note: if you ever want to compare two *binary* files, take a look at
the `cmp` program.)

In this project you will complete the implementation of these two
programs to compress and decompress ASCII text files using Huffman
coding.

## Suggested Reading

Although the goal of this assignment is not really to learn about
Huffman coding in particular, you will need to read up on how it works
in order to understand the provided [starter code] and how to
implement the parts indicated below.  A good starting point is the
description of [Huffman coding][huffman] on Wikipedia.

You should also watch the [video][overview-video] that we provide that
covers the Huffman algorithm and highlights the important files in the
implementation.

Note, you should read the documentation for this assignment *multiple
times* to get a better understanding of what you must do, how Huffman
coding works, and how the implementation is constructed, so that you
will be successful.  Each reading will provide deeper understanding as
you come to grasp all the parts and how they work together.

## Part 0: Project Startup

Download and extract the contents of the [starter code] into your
Workspace folder. To do this, download the [starter code] and place
the zip archive file in your Workspace folder (see the first
assignment documentation if you forget where this is). After you place
the zip file in the Workspace folder, right-click on the zip archive
file and select "Extract Here". This will extract the contents of the
zip file and you will see a directory for this project. You can then
delete the zip archive file that you downloaded.

## Part 1: Understand The Code

The code tree is organized as follows, under the main project
directory:

```
.
|-- books
|-- compile.sh
|-- docs
|   `-- instructions.md
|-- huffcs
|-- huffds
|-- Makefile
|-- README.md
|-- src
|   |-- bits-io.c
|   |-- bits-io.h
|   |-- decoder.c
|   |-- decoder.h
|   |-- encoder.c
|   |-- encoder.h
|   |-- huffc.c
|   |-- huffd.c
|   |-- huffman.c
|   |-- huffman.h
|   |-- hzip.h
|   |-- pqueue.c
|   |-- pqueue.h
|   |-- table.c
|   |-- tableg.c
|   |-- table.h
|   |-- tree.c
|   |-- treeg.c
|   `-- tree.h
|-- tablegs
|-- test
|   `-- all_tests.c
|-- test.sh
`-- treegs
```

We provide you with [starter code] for this assignment.  Your first
task is to **read** through each of the provided source files in
detail, so that you understand the structure of the code.  This is
particularly important for this assignment because of the number of
source files involved.  The `huffc` and `huffd` programs are
themselves quite simple, but they depend on a number of "modules" for
their implementation.  The following is a description of each of these
modules and their corresponding source files.

### BitsIO Module

The BitsIO module consists of the following source files:

* `bits-io.h`
* `bits-io.c`

The BitsIO module provides support for writing to and reading from a
compressed file.  A Huffman compressed file is stored in the following
format:

```
  [LENGTH][SERIALIZED HUFFMAN CODING TREE] [HUFFMAN ENCODED (COMPRESSED) DATA]
   0----7  8----------------------------s   h-------------------------------n
```

The eight byte length gives the number of original bytes that were
encoded into the compressed file.  The `SERIALIZED HUFFMAN CODING
TREE` part of the file starts at byte *8* and ends at byte *s*.  It
contains a string-based representation of the Huffman coding tree that
was used to encode the input ASCII text file.  We need to include the
tree as part of the compressed file so we know how to decompress the
encoded ASCII in the `HUFFMAN ENCODED DATA` part of the file.  This
part of the file starts at byte *h* (one after *s*) and continues to
the end of the file, with byte *n*.  It contains the bit encoding of
each of the characters found in the original input text file.

This module includes functions for opening and closing a compressed
file, writing and reading bits, writing and reading the Huffman tree,
and writing and reading the eight byte length.  The two most important
functions are `bits_io_write_bit` and `bits_io_read_bit`.  Both of
these functions depend on counting the number of bits processed
related to each byte of the compressed file.  The details of the
formatting are elaborated in the `bits-io.c` file -- which you should
read completely!  This module relies on the stdio `fputc` and `fgetc`
functions for writing and reading a byte respectively.  It is not
possible to write or read individual *bits* to and from a file, so we
must buffer in an individual byte the bits that we write and read.
When we are writing bits to a file, we write bits to that one-byte
buffer until it is full before writing the byte to the actual file.
Likewise, when we read in individual bits we must first read a byte
from the compressed file into the byte buffer, and then read
individual bits from the byte buffer.

To do this correctly we must have a well defined format for the bytes
we are writing and reading.  In particular we use this bit ordering:

```
 12345678
```

That is, the most significant bit is the one that was first asked to
be written, the next most significant bit was requested second, and so
on.  The main structure we use for representing a bit stream connected
to a file (with a byte buffer) is the `BitsIOFile` struct found in
`bits-io.c`.  You will notice the definition of the byte buffer as a
field of this structure called `byte`.  You can also look at the
implementation of the `bits_io_write_bit` function to see how it is
used.  Reading bits is straightforward as well -- just remember to
read them from the same bits positions in the same order as they are
written.  Both writing and reading use a counter, ``bitsLeft``, that
counts down from 8.  The comments in `bits-io.c` include more details
of the scheme.

Concerning EOF, it is problematic to read using `fgetc` into a *byte*
and then try to check for EOF.  The reason is that EOF is the int
value -1.  When stored into a byte we cannot distinguish it from a
true byte we read that is all ones.  So make sure to use an ``int``
for the result of ``fgetc``.


### Tree Module

The Tree module consists of the following source files:

* `tree.h`
* `tree.c`

The Tree module provides support for creating a Huffman binary tree.
A Huffman binary tree consists of *internal* and *leaf* tree nodes.
Both node types are represented by the structure `TreeNode`; they are
distinguished by a `type` field that can either be `INTERNAL` or
`LEAF` (values of an enumeration type).  An internal tree node is a
node that has at least one child node.  Its frequency field is always
the sum of the frequencies of its children (initially 0 when it has no
children yet), and its character is always the null character `'\0'`
(that field is not really used for internal nodes).  A leaf tree node
is a node that has no children and its frequency field has a value
that is greater than 0 and a character encountered from the input text
file.  A `TreeNode` also has a unique id that is used to identify it
when it is serialized in the compressed file (as mentioned in the
BitsIO module section).  In addition, a `TreeNode` has an additional
pointer called `next` that points to another `TreeNode`.  This field
is used during the deserialization process to form a linked list of
tree nodes.

The module provides functions for creating new `TreeNode` objects,
freeing them, getting the size of the tree, and printing a tree.  You
may consider using the `tree_print` function during testing and
debugging to see what your tree looks like, to make sure it is
something sensible.  In addition, this module provides the
`tree_serialize` and `tree_deserialize` functions that convert a tree
of `TreeNode` objects into a string that is written to a file, and
conversely converts a string representation of a tree into `TreeNode`
objects.  This is used by the BitsIO module to write the `SERIALIZED
TREE` part of its formatted file.  You do not need to implement
anything in this file.  However, it is important that you review the
implementation and read the comments in the code to understand how it
works.

### Priority Queue Module

The Priority Queue module consists of the following source files:

* `pqueue.h`
* `pqueue.c`

The Priority Queue module provides functionality for creating and
using a [priority queue], to assist in building the Huffman tree.
This implementation is simple in that it uses an array and a `sort`
function rather than a more efficient implementation such as a [heap]
data structure.  The module provides functions for creating a priority
queue, freeing a priority queue, enqueue and dequeue, size, and
printing.

The implementation consists of a `PriorityQueue` structure containing
an array of `TreeNode` pointers and an integer field `count`
indicating the number of used slots in the priority queue.  The array
has a max size of 256, which will never be exceeded because we will
only need it large enough to hold any byte value that can occur in a
file.  Note that the implementation is customized for this application
and thus does not provide a generic implementation of a priority queue
data structure.

We include a utility function called `sort` that uses the `qsort`
function defined in `stdlib.h`.  You can use `man` to read up more
details on the `qsort` function.  In short, it will sort an array
given a *comparator* function.  We have not discussed function
pointers in C.  However, this is an example of one.  The comparator
function is used to sort the priority queue by priority in ascending
order (lowest priority first).  The `sort` function is called after a
`TreeNode` object is enqueued in the queue.

You will complete the implementation of the `pqueue_enqueue` and
`pqueue_dequeue` functions.  You will see that these are labeled with
a `TODO` and additional instructions on how to complete the
implementation.

### Table Module

The Table module consists of the following source files:

* `table.h`
* `table.c`

The table module is used to create an *encoding table* that maps
characters to their bit encoding.  The table is constructed using a
Huffman tree built from a specific input text file.  The module
provides the `table_build` function that takes as its parameter a
`TreeNode*` and returns an `EncodeTable*`.  The returned encoding
table can be used with the `table_bit_encode` function to return a
character string (`char *`) of '1' and '0' digits representing the
encoding.  The encoding string is terminated by the usual null
character.

Although it is possible to use the original Huffman tree to determine
the encoding for a character it would require a search over the entire
tree to find the corresponding encoding.  For this reason we create a
simple lookup table that "remembers" what the encoding is.  This table
is built from visiting all the nodes and paths in the Huffman tree and
recording each path to each leaf node (each character).

This module also provides a `table_print` function that can be used
during testing and debugging to ensure that your implementation is
working properly.  You do not need to make any changes to this module.

### Huffman Module

The Huffman module consists of the following source files:

* `huffman.h`
* `huffman.c`

The Huffman module provides the core functionality for the Huffman
coding algorithm.  The Huffman coding algorithm consists of three
phases.  The first phase computes the frequencies of characters found
in the input text file.  After we have found the frequencies of the
characters, the second phase creates a new `TreeNode` for each of the
characters that were found (frequency greater than 0) and adds them to
a priority queue.  The priority queue will arrange the `TreeNode`
objects in ascending order based on their frequency.  The third phase
iterates over the priority while it has more than one item, building
the Huffman tree.  The details of each of these phases are elaborate
in `huffman.c`.  You must implement each of the phases in order to
successfully construct a Huffman tree.

### Encoder and Decoder Module

The Encoder and Decoder modules consists of the following source
files:

* `encoder.h`
* `encoder.c`
* `decoder.h`
* `decoder.c`

The Encoder module uses the modules described above to carefully
encode an ASCII text file and generate a Huffman compressed output
file.  The Decoder module uses the modules described above to decode a
Huffman compressed file and generate the ASCII text file.  The API to
both of these modules are simple.  For encoding, you create a new
`Encoder` object, encode the file, and free the `Encoder`.  The
Decoder works in a similar fashion.  You do not need to modify these
modules.

### The huffc and huffd Programs

The `huffc` and `huffd` programs are built from the `huffc.c` and
`huffd.c` source files respectively.  You should read the code
contained in these files - it is self-explanatory.

## Part 2: Compiling and Testing the Code

To compile your project code you must run the following script from
the terminal inside the project's root directory (either using the
Linux terminal program or control-` (type the control and backtick
character at the same time) for VSCode's integrated terminal):

```bash
$ ./compile
```

You will notice that after you run this script it will create a obj
directory as well as a `huffman_submit.zip` file. The later is the zip
archive file you should submit to Gradescope. In addition, this will
produce 4 executable files:

* `huffc`
* `huffd`
* `treeg`
* `tableg`

The `huffc` and `huffd` executables perform Huffman compression and
decompression respectively. The `treeg` program will read in an ASCII
text file and print a text representation of the Huffman tree. You can
use this as part of your testing and debugging to see if you have done
this properly. You can use our provided solution executable `treegs`
to see what our solution prints out.  To see if your output is the
same as ours you can redirect the output of both programs to files and
compare them using the `diff` command. (Note: We are not talking about
comparing the programs - your versions will almost certainly be
different from ours because your source code is different. It is the
output of the programs that you should compare.)

Likewise, the `tableg` executable will print out the table created
from the Huffman tree.  We have provided our solution executable
`tablegs` so that you can compare the output against our solution.

To run the provided tests you can run the following from the terminal:

```bash
$ ./test.sh
```

This will compile your project and run the tests (take a peek in this
file to see how this is done). You will see this output the first time
you run this script:

```
[==========] Running 4 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 4 tests from ProjectTests
[ RUN      ] ProjectTests.test_bits_io_open
[       OK ] ProjectTests.test_bits_io_open (0 ms)
[ RUN      ] ProjectTests.test_bits_io_close
[       OK ] ProjectTests.test_bits_io_close (0 ms)
[ RUN      ] ProjectTests.test_bits_io_read_bit
[       OK ] ProjectTests.test_bits_io_read_bit (6 ms)
[ RUN      ] ProjectTests.test_bits_io_write_bit
test/all_tests.c:97: Failure
      Expected: bit
      Which is: '\xFF' (255)
To be equal to: 1
bits_io_read_bit() returned wrong bit.
[  FAILED  ] ProjectTests.test_bits_io_write_bit (1 ms)
[----------] 4 tests from ProjectTests (7 ms total)

[----------] Global test environment tear-down
[==========] 4 tests from 1 test case ran. (8 ms total)
[  PASSED  ] 3 tests.
[  FAILED  ] 1 test, listed below:
[  FAILED  ] ProjectTests.test_bits_io_write_bit

 1 FAILED TEST
```

As usual, you are not provided all the tests that will cover what is
necessary to truly test your code. We encourage you to look at the
tests that we provide in the `test/all_tests.c` file. You should think
hard about how to test your code and implement some tests to ensure
yourself that you are on the right direction.

## Part 3: Complete Priority Queue

Your first task is to implement the functions:

* `pqueue_enqueue`
* `pqueue_dequeue`

in the `pqueue.c` file.  The description of what you need to do is
outlined in the functions themselves. You should write some additional
tests in `test/all_tests.c` to test your work to see if you are
implementing this part of the project correctly. 

## Part 4: Complete Huffman

Your second task is to implement the functions:

* `compute_freq`
* `create_tree_nodes`
* `build_tree`

in the `huffman.c` file.  The description of what you need to do is
outlined in the functions themselves. You should write some additional
tests in `test/all_tests.c` to test your work to see if you are
implementing this part of the project correctly. In addition, you can
use the `treeg` and `tableg` executables to see what the output is and
compare against our solution executables. If you are getting the
exact output as our implementation you are in great shape.

## Part 5: Complete BitsIO

Your third task is to implement the function:

* `bits_io_read_bit`

in the `bits-io.c` file.  The description of what you need to do is
outlined in the function itself. You should write some additional
tests in `test/all_tests.c` to test your work to see if you are
implementing this part of the project correctly. 

## Part 6: Compression and Decompression Running and Testing

Once you reach this part of the assignment and the provided tests and
your own tests are passing you are in a good position to start running
the actual `huffc` and `huffd` executables.  We have provided the
solution binaries as `huffcs` and `huffds` that you can use to see if
your implementation is working properly.  For example, you can run our
`huffcs` to generate a compressed file and then use your `huffd` to
decompress.  Likewise, use your `huffc` to generate a compressed file
and then use our `huffds` to decompress it.  You should also compare
the size of the compressed files generated by our solution and yours
to see if they match.  You can do this using the following command:

```bash
$ ./huffc books/holmes.txt holmes.he
$ ./huffcs books/holmes.txt holmess.he
$ ls -lh holmes.he holmess.he
```

Another test you can perform is to decompress a compressed file
generated by your programs and compare the decompressed text file
against the original to make sure they are identical:

```bash
$ ./huffc books/holmes.txt holmes.he
$ ./huffd holmes.he holmes.txt
$ diff holmes.txt books/holmes.txt
```

You can also use the `cmp` program to compare binary files, byte for
byte.

### Debugging Hints

C programs involving pointers can be tricky to debug.  You should use
the `valgrind` tool to help identify problems in your code and any
memory leaks (allocating memory without freeing it).  To run
`valgrind` from the command line you do this:

```bash
$ valgrind ./huffc books/holmes.txt holmes.he
$ valgrind ./huffd holmes.he holmes.txt
```

This will report any invalid access to memory and any memory that had
been allocated and not freed before the program terminated.  If your
program tries to access memory in a way it is not allowed to do you
will likely see `segmentation violation (core dump)` as the only
output after running your program.  Make sure you use `valgrind` to
help better understand where things went wrong.  Here is a list of how
to go about debugging your C code:

1. Use `printf` to output debugging information.  Do not underestimate
   the usefulness of this simple method of debugging!
1. Use `valgrind` to narrow the scope of where your problem is (which
   function), then use the `printf` method.
1. Use the vscode debugger if you are really stuck and need to step
   through your program one line at a time.

We will check your programs to make sure that you do not have any
memory leaks.

## Submission Instructions

Once you have past your tests and ours, and have otherwise convinced
yourself that your implementation meets the requirements you should
upload the generated `huffman_submit.zip` file to Gradescope. This
will run the private tests on your implementation and report to you if
you have passed all the tests. If you have passed all the tests you
have mastered this project. If you fail any tests you should use the
provided feedback from the failed tests to debug and improve your
implementation until you pass all the tests. You can re-submit your
project as many times as you wish to Gradescope before the due date of
the assignment.

[huffman]: http://en.wikipedia.org/wiki/Huffman_coding
[priority queue]: http://en.wikipedia.org/wiki/Priority_queue
[heap]: http://en.wikipedia.org/wiki/Heap_(data_structure)

[check]: http://check.sourceforge.net
[macros]: http://www.thegeekstuff.com/2012/05/c-macros/
[overview-video]: https://drive.google.com/file/d/0B5jcIXotvJOzaVpxX0FJOUJLc00/view?usp=sharing
