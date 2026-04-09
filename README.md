# Huffman compression

## Project
A compression tool for text files written in C, using Huffman compression.
I used binary trees and queues for this project.

## Results
The ratio `compressed_size / original_size` gets smaller as the original size gets bigger.
It appears that for a very big file, the **compressed file will be about 60% of the original size**.
I tried to optimize the algorythm as much as I could:
- The compressor takes about **1s** to compress a 250Ko file into a 150Ko file *(not optimized yet)*
- The decompressor takes about **30ms** to decompress a 150Ko file into a 250Ko file
