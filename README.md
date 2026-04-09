# The jam compressor
Jam is a lossless text file compression tool written in C, using Huffman compression.

## Usage
```sh
$ jam <input> [output]
$ unjam <input> [output]
```

### Examples

`$ jam file.txt` will produce the jammed file `file.txt.jam`.

`$ jam file.txt output` will produce the jammed file `output`.

`$ unjam file.txt.jam` will produce the unjammed file `file.txt`.

`$ unjam file.txt.jam output` will produce the unjammed file `output`.

## Results
The ratio `compressed_size / original_size` gets smaller as the original size gets bigger.
It appears that for a very big file, the **compressed file will be about 60% of the original size**.
I tried to optimize the algorythm as much as I could.
On my machine:
- The compressor takes about **1s** to compress a 250Ko file into a 150Ko file *(not optimized yet)*
- The decompressor takes about **30ms** to decompress a 150Ko file into a 250Ko file
