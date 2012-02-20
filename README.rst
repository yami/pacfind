Introduction
============

Pacfind is a simple bash command-not-found handler for ArchLinux.

This version still does not work, because it uses /tmp/pacfind as its working directory, which is a bad idea. If you managed to create a /tmp/pacfind directory first, and run 'pacfind -u' manually. Then 'pacman -q <binary name>' would work.


Why
===

Why another command-not-found handler (we have pactools alreay)? 
The reason is I think pactools is slow, because it always parses compressed archive files. Pacfind uses a constant database (Tiny-CDB) to store parsed content, so that we have a much faster lookup later.

Another reason is pactools depends on Python heavily, however I'd like a pure C solution.
