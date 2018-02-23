#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
// Include the gtest header file:
#include <gtest/gtest.h>

// Include assignment header file:
#include "../src/hzip.h"

using namespace std;

class ProjectTests : public ::testing::Test
{
  protected:
    ProjectTests() {}          //constructor runs before each test
    virtual ~ProjectTests() {} //destructor cleans up after tests
    virtual void SetUp() {}    //sets up before each test (after constructor)
    virtual void TearDown() {} //clean up after each test, (before destructor)
};

#define BIT_ITERATIONS 100001

// We include this definition here so we can gain access to the struct
// that is being returned from the bits-io module.
struct BitsIOFile {
  FILE *fp;
  char mode;
  char bitsLeft;
  unsigned char byte;
  int numBytes;
};

//  For more details see http://check.sourceforge.net

//////////////////////////////////////////////////////////////////////
///////////// bits-io unit tests
//////////////////////////////////////////////////////////////////////

TEST(ProjectTests, test_bits_io_open)
{
  BitsIOFile *bfile = bits_io_open("books/aladdin.txt", "r");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'books/aladdin.txt'";
  ASSERT_TRUE(bfile->fp != NULL) << "bfile->fp should not be null.";
  ASSERT_TRUE(bfile->mode == 'r') << "bfile->mode should be 'r'.";
}

TEST(ProjectTests, test_bits_io_close)
{
  BitsIOFile *bfile = bits_io_open("books/aladdin.txt", "r");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'books/aladdin.txt'";
  ASSERT_TRUE(bfile->fp != NULL) << "bfile->fp should not be null.";
  ASSERT_TRUE(bfile->mode == 'r') << "bfile->mode should be 'r'.";
  bits_io_close(bfile);
}

TEST(ProjectTests, test_bits_io_read_bit)
{
  // Delete the files if they exist:
  remove("test/test.hf");
  remove("test/test-stress.hf");

  BitsIOFile *bfile = bits_io_open("test/test.hf", "w");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'test/test.hf'";
  unsigned char bit;
  bit = bits_io_write_bit(bfile, 1);
  ASSERT_EQ(bit, 1) << "bits_io_write_bit() returned wrong bit.";
  bit = bits_io_write_bit(bfile, 1);
  ASSERT_EQ(bit, 1) << "bits_io_write_bit() returned wrong bit.";
  bit = bits_io_write_bit(bfile, 0);
  ASSERT_EQ(bit, 0) << "bits_io_write_bit() returned wrong bit.";
  bit = bits_io_write_bit(bfile, 1);
  ASSERT_EQ(bit, 1) << "bits_io_write_bit() returned wrong bit.";
  bit = bits_io_write_bit(bfile, 0);
  ASSERT_EQ(bit, 0) << "bits_io_write_bit() returned wrong bit.";
  int result = bits_io_close(bfile);
  ASSERT_TRUE(result != EOF) << "closing the file should not be EOF.";

  bfile = bits_io_open("test/test-stress.hf", "w");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'test/test-stress.hf'";
  for (int i = 0; i < BIT_ITERATIONS; i++) {
    bit = bits_io_write_bit(bfile, i%2);
    ASSERT_EQ(bit, i%2) << "bits_io_write_bit() returned wrong bit.";
  }
  ASSERT_EQ(bits_io_num_bytes(bfile), ((BIT_ITERATIONS+7)/8) - 1) << "bits_io_num_bytes() returned wrong result.";

  result = bits_io_close(bfile);
  ASSERT_TRUE(result != EOF) << "closing the file should not be EOF.";
}

TEST(ProjectTests, test_bits_io_write_bit)
{
  BitsIOFile *bfile = bits_io_open("test/test.hf", "r");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'test/test.hf'";
  unsigned char bit;
  bit = bits_io_read_bit(bfile);
  ASSERT_EQ(bit, 1) << "bits_io_read_bit() returned wrong bit.";
  bit = bits_io_read_bit(bfile);
  ASSERT_EQ(bit, 1) << "bits_io_read_bit() returned wrong bit.";
  bit = bits_io_read_bit(bfile);
  ASSERT_EQ(bit, 0) << "bits_io_read_bit() returned wrong bit.";
  bit = bits_io_read_bit(bfile);
  ASSERT_EQ(bit, 1) << "bits_io_read_bit() returned wrong bit.";
  bit = bits_io_read_bit(bfile);
  ASSERT_EQ(bit, 0) << "bits_io_read_bit() returned wrong bit.";

  int nbytes = bits_io_num_bytes(bfile);
  ASSERT_EQ(nbytes, 1) << "bits_io_num_bytes() returned incorrect # of biytes.";

  int result = bits_io_close(bfile);
  ASSERT_TRUE(result != EOF) << "closing the file should not be EOF.";

  bfile = bits_io_open("test/test-stress.hf", "r");
  ASSERT_TRUE(bfile != NULL) << "could not open file 'test/test-stress.hf'";
  for (int i = 0; i < BIT_ITERATIONS; i++) {
    bit = bits_io_read_bit(bfile);
    ASSERT_EQ(bit, i%2) << "bits_io_read_bit() returned wrong bit.";
  }
  ASSERT_EQ(bits_io_num_bytes(bfile), (BIT_ITERATIONS+7)/8) << "bits_io_num_bytes() returned incorrect # of biytes.";

  result = bits_io_close(bfile);
  ASSERT_TRUE(result != EOF) << "closing the file should not be EOF.";

  // Delete the files:
  remove("test/test.hf");
  remove("test/test-stress.hf");
}


