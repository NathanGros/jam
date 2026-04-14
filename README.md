# The Jam compressor
Jam is a lossless text file compression tool written in C, using Huffman coding.

I tried optimizing time performance as much as I could.

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

## Observations

### Compression ratio
The ratio `compressed_size / original_size` gets smaller as the original size gets bigger.

It appears that for a very big file, the compressed file will be **about 60% of the original size** (40% size reduction).

### Time performannce
On my not so recent laptop:
- The compressor takes about **220ms** to compress a 2.9 Mo file into a 1.6 Mo file
- The decompressor takes about **250ms** to decompress a 1.6 Mo file back into a 2.9 Mo file
