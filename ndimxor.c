// Sam Siewert
//
// Basic tests for:
//
// 2-dim XOR, non-MDS, capable of recovering from any 3+ erasures as long as not forming square subset
//     XOR includes column and row = DIM + DIM
//     E.g. 2x2, 04 XOR, 04 DATA, 08 TOTAL = 50.00% efficient
//     E.g. 3x3, 06 XOR, 09 DATA, 15 TOTAL = 60.00% efficient
//     E.g. 4x4, 08 XOR, 16 DATA, 24 TOTAL = 66.67% efficient
//     E.g. 6x6, 12 XOR, 36 DATA, 48 TOTAL = 75.00% efficient + 16 spares per rack -- INTERESTING
//     E.g. 7x6, 13 XOR, 42 DATA, 55 TOTAL = 76.36% efficient + 09 spares per rack -- INTERESTING
//     E.g. 7x7, 14 XOR, 49 DATA, 63 TOTAL = 77.78% efficient + 01 spare  per rack -- INTERESTING
//     E.g. 8x8, 16 XOR, 64 DATA, 80 TOTAL = 80.00% efficient
//
// 3-dim XOR, non-MDS, capable of recovering from any 4+ erasures as long as no two planes contain square subset
//    XOR at each height for column and row = [DIM x (DIM + DIM)] + (DIM x DIM)
//    E.g. 2x2x2, 012 XOR, 008 DATA, 020 TOTAL = 40% efficient
//    E.g. 4x4x4, 048 XOR, 064 DATA, 112 TOTAL = 57% efficient
//    E.g. 8x8x8, 192 XOR, 512 DATA, 704 TOTAL = 72.72% efficient
//
// 
// 4-dim XOR, involves connecting the stacks of nodes in a rack together - not clear this has benefit other than sharing spares
//
//
// Testing is to determine how feasible this type of ordered XOR encoding and recovery is for a real system
// composed of 16 disks per node (4x4) with 4 nodes per rack and 4 total racks interconnected in a toroidal fashion
// which is 256 total storage devices.
//
// This forms a simple baesline for the sequenced XOR to compare to various LDPC, Galois Field, Flat XOR, etc. codes, which range from
// MDS codes where the code chunks = the erasure tolerance to codes such as this dimensional XOR and the extreme, flat XOR.
//

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

#define DIM (4)

#define PRINT_DATA				\
  for(r=0; r < DIM; r++)			\
    for(c=0; c < DIM; c++)			\
      for(h=0; h < DIM; h++)			\
        for(rack=0; rack < DIM; rack++)		\
          printf("%d ", data[r][c][h][rack]);	\
  printf("\n");  				\


// row x col x height x rack-length
unsigned char data[DIM][DIM][DIM][DIM];
unsigned char check_data[DIM][DIM][DIM][DIM];

// 2D XORs = (DIM + DIM), DIM x (DIM + DIM) for 3D
unsigned char row_xor[DIM][DIM];
unsigned char col_xor[DIM][DIM];


// 3D XORs = [DIM x (DIM + DIM)] + (DIM x DIM)
unsigned char height_xor[DIM][DIM];

// 4D XORs = ??
unsigned char rackxor[DIM][DIM][DIM];

int main(void)
{
  int r, c, h, rack, idx=0, layer=0;

  bzero(data, DIM*DIM*DIM*DIM);
  bzero(row_xor, (DIM*DIM));
  bzero(col_xor, (DIM*DIM));
  bzero(height_xor, (DIM*DIM));
  bzero(rackxor, (DIM*DIM*DIM));


  // Fill 4x4x4x4 racks with known pattern for testing
  for(r=0; r < DIM; r++)
  {
    for(c=0; c < DIM; c++)
    {
      for(h=0; h < DIM; h++)
      {
        for(rack=0; rack < DIM; rack++)
        {
          data[r][c][h][rack]=idx;
          check_data[r][c][h][rack]=idx;
          idx++;
        }
      }
    }
  }


  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);


  // loop through rows and columns to compute row-XOR and height-XOR
  h=0;
  for(r=0; r < DIM; r++)
  {
    for(c=0; c < DIM; c++)
    {
        row_xor[r][layer]=(row_xor[r][layer]) ^ (data[r][c][0][0]);
        
        for(h=0; h < DIM; h++)
        {
          height_xor[r][c]^=data[r][c][h][0];
        }
    }
  }


  // loop through columns and rows to compute column-XOR
  for(c=0; c < DIM; c++)
  {
    for(r=0; r < DIM; r++)
    {
        col_xor[c][layer]=(col_xor[c][layer]) ^ (data[r][c][0][0]);
    }
  }


  // loop through all cubic locations to compute liner rack-XOR
  for(r=0; r < DIM; r++)
  {
    for(c=0; c < DIM; c++)
    {
      for(h=0; h < DIM; h++)
      {
        rackxor[r][c][h]=0;
      }
    }
  }


  // Skips erasing XORs since this is trivial recomputation of XOR


  // SIMPLE SINGLE data ERASURES
  //
  printf("\n********** SINGLE ERASURE TESTS\n");

  // Erase any single data entry in a planar row or column
  //
  // We can in fact erase a whole column or a whole row, but we only test one erasure per column or row
  // at this point

  printf("\nDemonstrate erasing %d, %d and recover by row XOR: ", 3, 3);
  data[3][3][0][0]=0;
  data[3][3][0][0]=row_xor[3][layer] ^ data[0][3][0][0] ^ data[1][3][0][0] ^ data[2][3][0][0];
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");

  // erase first column data in any row
  for(r=0; r < DIM; r++)
  {
    data[r][0][0][0]=0;
    data[r][0][0][0]=row_xor[r][layer] ^ data[(r+1)%DIM][0][0][0] ^ data[(r+2)%DIM][0][0][0] ^ data[(r+3)%DIM][0][0][0];
    printf("ERASE[%d, 0, 0, 0] ", r);
  }
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");

  printf("\nDemonstrate erasing %d, %d and recover by col XOR: ", 3, 3);
  data[3][3][0][0]=0;
  data[3][3][0][0]=col_xor[3][layer] ^ data[3][0][0][0] ^ data[3][1][0][0] ^ data[3][2][0][0];
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");

  // erase first row data in any column
  for(c=0; c < DIM; c++)
  {
    data[0][c][0][0]=0;
    data[0][c][0][0]=col_xor[c][layer] ^ data[0][(c+1)%DIM][0][0] ^ data[0][(c+2)%DIM][0][0] ^ data[0][(c+3)%DIM][0][0];
    printf("ERASE[0, %d, 0, 0] ", c);
  }
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");



  // Erase any single data entry in cube plane or surface (unique height at each row, col location)
  //
  // We can in fact erase a whole plane or surface at a time as long as there is only one erasure
  // per z-column

  printf("\nDemonstrate erasing %d, %d, %d, 0: ", 3, 3, 3);
  data[3][3][3][0]=0;
  data[3][3][3][0]=height_xor[3][3] ^ data[3][3][0][0] ^ data[3][3][1][0] ^ data[3][3][2][0];
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");

  printf("\nDemonstrate erasing %d, %d, %d, 0: ", 0, 0, 0);
  data[0][0][0][0]=0;
  data[0][0][0][0]=height_xor[0][0] ^ data[0][0][1][0] ^ data[0][0][2][0] ^ data[0][0][3][0];
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");


  // all height erasures
  printf("\n");
  for(r=0; r < DIM; r++)
  {
    for(c=0; c < DIM; c++)
    {
      for(h=0; h < DIM; h++)
      {
        data[r][c][h][0]=0;

        // recover entry in cube data using all data and XOR except data erased at location h
        data[r][c][h][0]=height_xor[r][c] ^ data[r][c][(h+1)%DIM][0] ^ data[r][c][(h+2)%DIM][0] ^ data[r][c][(h+3)%DIM][0];

        printf("ERASE[%d, %d, %d, 0] ", r, c, h);
      }
    }
  }

  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");


  // DOUBLE data ERASURES
  //
  printf("\n********** DOUBLE ERASURE TESTS\n");

  // erase first and last column data in any row
  for(r=0; r < DIM; r++)
  {
    data[r][0][0][0]=0;
    data[r][3][0][0]=0;
    printf("DOUBLE ERASE[%d, 0, 0, 0] & [%d, 3, 0, 0] ", r, r);
    data[r][0][0][0]=col_xor[0][layer] ^ data[(r+1)%DIM][1][0][0] ^ data[(r+2)%DIM][2][0][0] ^ data[(r+3)%DIM][3][0][0];
    data[r][3][0][0]=col_xor[3][layer] ^ data[(r+1)%DIM][0][0][0] ^ data[(r+2)%DIM][1][0][0] ^ data[(r+3)%DIM][2][0][0];
  }
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");


  // erase first and last row data in any column
  for(c=0; c < DIM; c++)
  {
    data[0][c][0][0]=0;
    data[3][c][0][0]=0;
    printf("DOUBLE ERASE[0, %d, 0, 0] & [3, %d, 0, 0] ", c, c);
    data[0][c][0][0]=row_xor[0][layer] ^ data[1][(c+1)%DIM][0][0] ^ data[2][(c+2)%DIM][0][0] ^ data[3][(c+3)%DIM][0][0];
    data[3][c][0][0]=row_xor[3][layer] ^ data[0][(c+1)%DIM][0][0] ^ data[1][(c+2)%DIM][0][0] ^ data[2][(c+3)%DIM][0][0];
  }
  assert(memcmp(&data[0][0][0][0], &check_data[0][0][0][0], (DIM*DIM*DIM*DIM)) == 0);
  printf("OK\n");



}
