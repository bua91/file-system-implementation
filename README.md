# file-system-implementation
A distributed system is designed with five file servers, two clients and a metadata server (M-server) to emulate a distributed
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
2. If a server goes down, M-server code is able to detect its failure on missing three heartbeat messages.
For each chunk, three replicas are maintained at servers Si, Sj and Sk, where 1 ≤ i, j, k ≤ 5 and i != j != k.
3. When a new chunk is to be created, the M-server selects three chunk servers at random and asks each one of
them to create a copy of the chunk.
4. For each chunk, the M-server maintains information about the chunk servers that host replicas of the chunk.
5. Any append to a chunk is performed to all live replicas of the chunk. Use two-phase commit protocol, with the
appending client as the coordinator and the relevant chunk servers as cohorts, to perform writes. Assume there
is no server failure during an append operation.
6. If multiple appends to the same chunk are submitted concurrently by different clients, the M-server determines
the order in which the appends are performed on all the chunks. Once a client completes an append to a chunk
it informs the M-server. Only then does the M-server permit another client to perform an append to the same
chunk.
7. A read from a chunk can be performed on any one of the current replicas of the chunk.
8. A recovering chunk server may have missed appends to its chunks while it was down. In such situations, the
recovering chunk server, with the help of the M-server, determines which of its chunks are out of date, and
synchronizes those chunks with their latest version by copying the missing appends from one of the current
replicas.
9. Only after all chunks of a recovering chunk server are up-to-date does that chunk server resume participating in
append and read operations.
