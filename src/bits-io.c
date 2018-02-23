/********************************************************************

 The bits-io module is used to encode bits into an output file, and decode
 them from an input file. The bits-io module buffers output/input bits into an
 internal byte-sized buffer using a counter to know when to write or read a
 full byte to the output or input file. Here are the details:

 WRITING BITS

   Because we are using the stdio function fputc to write a byte to an output
   file we are required by its interface to deal with bytes, not bits, when
   writing.  That is, it is not possible to write a single bit to an output
   file in Unix - so, we must buffer the bits that we ultimately want to write
   to the output file into a byte. When the byte is full we can then write it
   to the output file.

   To do this correctly, we count the number of bits that can still be
   filled.  We insert new bits, working from the most significant to least
   significant bit.  When a byte is full, we write it and start over with
   an empty (zero) byte.

 READING BITS

   Because we are using the stdio function fgetc to read bytes from an input
   file, we are required by its interface to deal with bytes and not bits when
   reading (as for writing). Thus, we also count how many bits we have left
   to return from a given byte, and read the next byte only when necessary.
   Because at a higher level we track the number of uncompressed items we
   wrote to the file originally, and on reading we count that down, we
   should never encounter end of file when reading data back - we should
   read exactly the right number of bytes.  However, you should *check*
   that you do not read past the end of the file.  For this reason, fgetc
   return an int, not a char, and you should check for a -1 return value
   from fgetc, which indicates EOF (end of file).

 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bits-io.h"
#include "tree.h"

/**
 * This structure is used to maintain the writing/reading of a
 * compressed file.
 */
struct BitsIOFile {
  FILE *fp;            // The output/input file
  char mode;           // The mode 'w' for write and 'r' for read
  char bitsLeft;       // Number of bits left to read/write in this byte
  unsigned char byte;  // The byte buffer to hold the bits we are
		       // reading/writing
  int numBytes;        // Number of bytes written to / read from
                       // the encoded file
};

/**
 * Opens a new BitsIOFile. Returns NULL if there is a failure.
 *
 * The `name` is the name of the file.
 * The `mode` is "w" for write and "r" for read.
 */
BitsIOFile *bits_io_open (const char *name, const char *mode) {

  FILE *fp = fopen(name, mode);
  if (fp == NULL) {
    return NULL;
  }

  char mode_letter = mode[0];

  BitsIOFile *bfile = (BitsIOFile *)(malloc(sizeof(BitsIOFile)));
  bfile->fp       = fp;
  bfile->mode     = mode_letter;
  bfile->bitsLeft = (mode_letter == 'w' ? 8 : 0);
                    // This forces reading the first byte when reading,
                    // and waiting for the byte to filel when writing.
  bfile->byte     = 0;
  bfile->numBytes = 0;
  return bfile;
}


/**
 * Close the BitsIOFile. Returns EOF if there was an error.
 */
int bits_io_close (BitsIOFile *bfile) {

  assert(bfile != NULL);

  // Write the current (last) byte if we are in 'w' mode:
  if (bfile->mode == 'w' && bfile->bitsLeft != 8) {
    int result = fputc(bfile->byte, bfile->fp);

    // Check to see if there was a problem:
    if (result == EOF) {
      // We will follow the fputc return value convention
      // which is to return EOF:
      return EOF;
    }
    ++(bfile->numBytes);
  }

  fflush(bfile->fp);
  fclose(bfile->fp);
  free(bfile);

  return 0;
}

/**
 * Number of bytes written to / read from the encoded file
 */
int bits_io_num_bytes (BitsIOFile *bfile) {
  return bfile->numBytes;
}

/**
 * Read a bit from the BitsIOFile.
 * Returns 0 or 1 for a bit read,
 * or EOF (-1) if there are no more bits to read
 */
int bits_io_read_bit (BitsIOFile *bfile) {

  assert(bfile != NULL);

  // TODO:

  // First, if there are no bits left to read, we need to try to
  // read another byte from the file.  It that fails (returns -1),
  // we should return -1.  Otherwise, we set up for allowing the
  // next 8 bits to be read.
  //
  // Second, we obtain a bit from the high order bit of the byte,
  // shift the byte left, and decrement the bits left.  We return
  // the bits that we obtained.


  return EOF;
}

/**
 * Writes the given bit (1 or 0) to the BitsIOFile.
 * Returns the bit written, or -1 if there was an error.
 */
int bits_io_write_bit (BitsIOFile *bfile, int bit) {
  assert(bfile != NULL);
  assert((bit & 1) == bit);

  // First, check if the byte is full.  If so, write the byte,
  // zero the byte, and set the bits left to 8.
  //
  // Next, decrement the number of bits left and OR in the new
  // bit shifted by the number of bits left.

  if (bfile->bitsLeft <= 0) {
    int result = fputc(bfile->byte, bfile->fp);
    if (result < 0) {
      return -1;
    }
    ++(bfile->numBytes);
    bfile->byte     = 0;
    bfile->bitsLeft = 8;
  }
  --(bfile->bitsLeft);
  bfile->byte |= (bit << bfile->bitsLeft);
  return bit;
}


/**
 * Writes the Huffman tree to the BitsIOFile.
 *
 * We need to write the tree to the file so that we can use it when we
 * decode the compressed file.
 */
int bits_io_write_tree (BitsIOFile *bfile, TreeNode *tree) {
  // If the mode is not for writing we return -1.
  if (bfile->mode != 'w') {
    return -1;
  }

  tree_serialize(tree, bfile->fp);
  return tree_size(tree);
}


/**
 * Reads the huffman tree from the BitsIOFile.
 *
 * We need to do this first so we have a tree that will be used to
 * decode the rest of the input.
 */
TreeNode *bits_io_read_tree (BitsIOFile *bfile) {
  // If the mode is not for reading we return NULL.
  if (bfile->mode != 'r') {
    return NULL;
  }

  return tree_deserialize(bfile->fp);
}


/**
 * Writes the original file's length to the output
 *
 * We need to write the length to know when to stop decoding
 * when reading data back.  Returns 0 if ok, -1 if error.
 */
int bits_io_write_length (BitsIOFile *bfile, int length) {
  // If the mode is not for writing we return -1.
  if (bfile->mode != 'w') {
    return -1;
  }

  for (int i = 4; --i >= 0; ) {
    if (fputc((unsigned char)(length & 0xFF), bfile->fp) < 0) {
      return -1;
    }
    length >>= 8;
  }
  return 0;
}


/**
 * Reads the original file's length
 *
 * We need to do this so that we know when to stop reading during
 * decoding.  Returns -1 on error.
 */
int bits_io_read_length (BitsIOFile *bfile) {
  // If the mode is not for reading we return -1.
  if (bfile->mode != 'r') {
    return -1;
  }
  unsigned int length = 0;
  for (int i = 4; --i >= 0; ) {
    int ch;
    if ((ch = fgetc(bfile->fp)) < 0) {
      return -1;
    }
    length >>= 8;
    length |= ((unsigned int)(ch & 0xFF) << (32 - 8));
  }
  return length;
}

