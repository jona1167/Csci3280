/*

CSCI 3280, Introduction to Multimedia Systems
Spring 2023

Assignment 01 Skeleton

photomosaic.cpp

*/

#include "stdio.h"
#include <iostream>
#include <vector>
#include <string>
#include "malloc.h"
#include "memory.h"
#include "math.h"
#include "bmp.h"		//	Simple .bmp library
#include "list_files.h" //	Simple list file library


#define SAFE_FREE(p)  { if(p){ free(p);  (p)=NULL;} }

int main(int argc, char** argv)
{
	// Parse output and cell shape specified in argv[3]



	// Read source bitmap from argv[1]



	// List .bmp files in argv[2] and do preprocessing



	// Compose the output image as save to argv[4]

	

	return 0;
}