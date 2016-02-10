miniv
=====

An experimental implementation of the minimal part of [Vanadium](https://vanadium.github.io)
necessary to do an RPC in C.

The target systems are those that are too small to run Go, but
big enough to run C and a networking stack.

[![Build Status](https://travis-ci.org/jeffallen/miniv.svg?branch=master)](https://travis-ci.org/jeffallen/miniv)

Go and cgo
----------

If the goal of this project is to write a tiny Vanadium in C,
why are there *.go and *_test.go files? Why is there cgo?

Good question.

Imagine you are a Go programmer. You like to write the least C
possible, because you prefer Go. Writing C is hard. Writing
even more C just to test the first C is maybe not hard, but
certainly annoying.

Enter cgo. If you write your C code like you plan for it to be
used indpendently of Go, but you test it via cgo, you get to write
only the implementation in C, and all your tests in Go.

And if you happened to have a copy of the official v.io
Go code in your GOPATH, then you could even do something really
snazzy: You can use the reference implementation to validate your
new implementation. For an example, see flow/message/message_test.go
where we use [v.io/v23/flow/message](https://godoc.org/v.io/v23/flow/message)
to serialize a message, and then we use our new implementation in C
to deserialize it.

As a general rule, *.[ch] are the C implementation. *.go are the
test files, and nothing you see in them is production code for
the targeted environment.

The current testing environment is MacOS.

Building without Go
-------------------

During normal development, the C is built with "go test".

To use the library outside of the Go/cgo test environment,
build it with CMake:

	$ mkdir obj
	$ cd obj
	$ cmake .. && make
	$ ls -l libminiv.a mvrpc/mvrpc 
        -rwxr-xr-x  1 jra  staff  14296 Feb  6 11:30 mvrpc/mvrpc
	-rw-r--r--  1 jra  staff  5016  Feb  4 23:52 libminiv.a

A bit smaller than the Go Vanadium!

C++?
----

It is 2016. This is a new all C library. What are you smoking?

I'm even worse at C++ than C. That's why I program Go.

But even if I was a C++ hacker, there are lots of embedded systems
that are not ready for C++. This Vanadium implementation is
targeted to them, so it is in C99.

