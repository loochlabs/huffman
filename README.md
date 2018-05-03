# Huffman Encoder & Decoder

These two programs are implementations of a lossless Huffman encoding algorithm. The encoded files are stateless as a huffman tree is encoded into .huf files and used during decoding.  
  
https://en.wikipedia.org/wiki/Huffman_coding

## Compilation  
From cli:  
> g++ henc.cpp -o henc  
> g++ hdec.cpp -o hdec  

## Usage

Encoding  
> ./henc filename

Decoding  
> ./hdec filename.huf
