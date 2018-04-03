# file-system-implementation
A distributed system is designed with three file servers, two clients and a metadata server (M-server) to emulate a distributed
file system. This program is easily extensible for any number of servers and clients.
A file in this file system can be of any size. However, a file is logically divided into chunks,
with each chunk being at most 8192 bytes (8 kilobytes) in size. Chunks of files in your filesystem are actually stored
as Linux files on the three servers. All the chunks of a given file need not be on the same server. In essence, a file in
this filesystem is a concatenation of multiple Linux files. If the total size of a file in this file system is 8x+y kilobytes,
where y < 8, then the file is made up of x + 1 chunks, where the first x chunks are of size 8 kilobytes each, while the
last chunk is of size y kilobytes.

Operations:

1. The code is able to support creation of new files, reading of existing files, and appends to the end of
existing files.
2. If a server goes down, M-server code is able to detect its failure on missing three heartbeat messages
and mark the corresponding chunks as unavailable. While those chunks are available, any attempt to read or
append to them returns an error message.
3. When a failed server recovers, it resumes sending its heartbeat messages, and the M-server repopulates its metadata to indicate the availability fo the recovered server’s chunks.
