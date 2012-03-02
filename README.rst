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