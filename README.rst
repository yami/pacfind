
Introduction
============

Pacfind is a simple bash command-not-found handler for ArchLinux.
To install it, simply type 'make', then change user to root 'make
install'. You may want to add a cron job to update database timely.

Pacfind uses Tiny-CDB as the database to store binary name to package
mapping. And its configuration file is in JSON format.

NB: Pacfind has a poor Makefile and not all error cases are properly
handled.

Why
===

Why another command-not-found handler, since we already have pactools and
one C++ implementation?

The reason is I think pactools is slow, because it always parses
compressed archive files. Pacfind uses a constant database (Tiny-CDB)
to store parsed content, so that we have a much faster lookup later.

Another reason is pactools depends on Python heavily, however I'd like
a pure C solution.


Some Notes on Source Code
=========================

1. bindb.h/c: the interface to the binary database, i.e. the database storing binary name to package mapping.
2. conf.h/c: read the JSON format configuration file by using jansson library.
3. download.h/c: download the archived files from mirror site by using CURL library.
4. filedb.h/c: the interface to the file database, i.e. the database storing repositor file list. Currently the official format is archive file.
5. pacfind.c: the file where main() locates.
6. slist.h/c: single linked list implementation
7. ttemp.h/c: simple text template implementation. Variables are prefixed with '$', and will be replaced by their values.
8. util.h/c: some utilities.