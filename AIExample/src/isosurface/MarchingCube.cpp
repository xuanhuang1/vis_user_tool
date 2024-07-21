#include "MarchingCube.h"
#include <vector>
#include <iostream>
#include <string>
#include <array>
#include <cmath>
//#include <ObjectArray.h>
using namespace std;


// Edge and triangle tables for the cases of marching cubes
// From http://paulbourke.net/geometry/polygonise/ and
// https://graphics.stanford.edu/~mdfisher/MarchingCubes.html
int triTable[256][16]{
	
	{-1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  9,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  1,  9,  8,  3,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 2, 10,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3,  1,  2, 10, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 9,  2, 10,  9,  0,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3,  2, 10,  3, 10,  8,  8, 10,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  3, 11, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{11,  0,  8, 11,  2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  9,  0,  2,  3, 11, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 2,  1,  9,  2,  9, 11, 11,  9,  8, -1,  0,  0,  0,  0,  0,  0},

	{ 3, 10,  1,  3, 11, 10, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  0,  8,  1,  8, 10, 10,  8, 11, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  3, 11,  0, 11,  9,  9, 11, 10, -1,  0,  0,  0,  0,  0,  0},

	{11, 10,  9, 11,  9,  8, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  7,  8, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  3,  0,  4,  7,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  7,  8,  9,  0,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 9,  4,  7,  9,  7,  1,  1,  7,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  7,  8,  1,  2, 10, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  3,  0,  4,  7,  3,  2, 10,  1, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  9,  0,  2, 10,  9,  4,  7,  8, -1,  0,  0,  0,  0,  0,  0},

	{ 3,  2,  7,  7,  9,  4,  7,  2,  9,  9,  2, 10, -1,  0,  0,  0},

	{ 8,  4,  7,  3, 11,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 7, 11,  2,  7,  2,  4,  4,  2,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  3, 11,  1,  9,  0,  8,  4,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  1,  9,  2,  9,  4,  2,  4, 11, 11,  4,  7, -1,  0,  0,  0},

	{10,  3, 11, 10,  1,  3,  8,  4,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  7,  0,  0, 10,  1,  7, 10,  0,  7, 11, 10, -1,  0,  0,  0},

	{ 8,  4,  7,  0,  3, 11,  0, 11,  9,  9, 11, 10, -1,  0,  0,  0},

	{ 7,  9,  4,  7, 11,  9,  9, 11, 10, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  9,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  3,  0,  4,  9,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  5,  4,  0,  1,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  8,  3,  4,  3,  5,  5,  3,  1, -1,  0,  0,  0,  0,  0,  0},

	{ 1,  2, 10,  9,  5,  4, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  9,  5,  8,  3,  0,  1,  2, 10, -1,  0,  0,  0,  0,  0,  0},

	{10,  5,  4, 10,  4,  2,  2,  4,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  8,  3,  4,  3,  2,  4,  2,  5,  5,  2, 10, -1,  0,  0,  0},

	{ 2,  3, 11,  5,  4,  9, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{11,  0,  8, 11,  2,  0,  9,  5,  4, -1,  0,  0,  0,  0,  0,  0},

	{ 5,  0,  1,  5,  4,  0,  3, 11,  2, -1,  0,  0,  0,  0,  0,  0},

	{11,  2,  8,  8,  5,  4,  2,  5,  8,  2,  1,  5, -1,  0,  0,  0},

	{ 3, 10,  1,  3, 11, 10,  5,  4,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 9,  5,  4,  1,  0,  8,  1,  8, 10, 10,  8, 11, -1,  0,  0,  0},

	{10,  5, 11, 11,  0,  3, 11,  5,  0,  0,  5,  4, -1,  0,  0,  0},

	{ 4, 10,  5,  4,  8, 10, 10,  8, 11, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  9,  5,  7,  8,  9, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  9,  5,  0,  5,  3,  3,  5,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  0,  1,  8,  1,  7,  7,  1,  5, -1,  0,  0,  0,  0,  0,  0},

	{ 3,  1,  5,  3,  5,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 7,  9,  5,  7,  8,  9,  1,  2, 10, -1,  0,  0,  0,  0,  0,  0},

	{ 1,  2, 10,  0,  9,  5,  0,  5,  3,  3,  5,  7, -1,  0,  0,  0},

	{ 7,  8,  5,  5,  2, 10,  8,  2,  5,  8,  0,  2, -1,  0,  0,  0},

	{10,  3,  2, 10,  5,  3,  3,  5,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 9,  7,  8,  9,  5,  7, 11,  2,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  9,  2,  2,  7, 11,  2,  9,  7,  7,  9,  5, -1,  0,  0,  0},

	{ 3, 11,  2,  8,  0,  1,  8,  1,  7,  7,  1,  5, -1,  0,  0,  0},

	{ 2,  7, 11,  2,  1,  7,  7,  1,  5, -1,  0,  0,  0,  0,  0,  0},

	{11,  1,  3, 11, 10,  1,  7,  8,  9,  7,  9,  5, -1,  0,  0,  0},

	{11, 10,  1, 11,  1,  7,  7,  1,  0,  7,  0,  9,  7,  9,  5, -1},

	{ 5,  7,  8,  5,  8, 10, 10,  8,  0, 10,  0,  3, 10,  3, 11, -1},

	{11, 10,  5, 11,  5,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{10,  6,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3, 10,  6,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 9,  0,  1,  5, 10,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  1,  9,  8,  3,  1, 10,  6,  5, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  1,  2,  6,  5,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 6,  1,  2,  6,  5,  1,  0,  8,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 5,  9,  0,  5,  0,  6,  6,  0,  2, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  5,  2,  2,  8,  3,  5,  8,  2,  5,  9,  8, -1,  0,  0,  0},

	{ 2,  3, 11, 10,  6,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0, 11,  2,  0,  8, 11,  6,  5, 10, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  1,  9,  3, 11,  2, 10,  6,  5, -1,  0,  0,  0,  0,  0,  0},

	{10,  6,  5,  2,  1,  9,  2,  9, 11, 11,  9,  8, -1,  0,  0,  0},

	{11,  6,  5, 11,  5,  3,  3,  5,  1, -1,  0,  0,  0,  0,  0,  0},

	{11,  6,  8,  8,  1,  0,  8,  6,  1,  1,  6,  5, -1,  0,  0,  0},

	{ 0,  3, 11,  0, 11,  6,  0,  6,  9,  9,  6,  5, -1,  0,  0,  0},

	{ 5, 11,  6,  5,  9, 11, 11,  9,  8, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  8,  4,  6,  5, 10, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3,  4,  7,  3,  0,  4,  5, 10,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  5, 10,  7,  8,  4,  9,  0,  1, -1,  0,  0,  0,  0,  0,  0},

	{ 5, 10,  6,  9,  4,  7,  9,  7,  1,  1,  7,  3, -1,  0,  0,  0},

	{ 1,  6,  5,  1,  2,  6,  7,  8,  4, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  0,  4,  7,  3,  0,  6,  5,  1,  6,  1,  2, -1,  0,  0,  0},

	{ 4,  7,  8,  5,  9,  0,  5,  0,  6,  6,  0,  2, -1,  0,  0,  0},

	{ 2,  6,  5,  2,  5,  3,  3,  5,  9,  3,  9,  4,  3,  4,  7, -1},

	{ 4,  7,  8,  5, 10,  6, 11,  2,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  5, 10,  7, 11,  2,  7,  2,  4,  4,  2,  0, -1,  0,  0,  0},

	{ 4,  7,  8,  9,  0,  1,  6,  5, 10,  3, 11,  2, -1,  0,  0,  0},

	{ 6,  5, 10, 11,  4,  7, 11,  2,  4,  4,  2,  9,  9,  2,  1, -1},

	{ 7,  8,  4, 11,  6,  5, 11,  5,  3,  3,  5,  1, -1,  0,  0,  0},

	{ 0,  4,  7,  0,  7,  1,  1,  7, 11,  1, 11,  6,  1,  6,  5, -1},

	{ 4,  7,  8,  9,  6,  5,  9,  0,  6,  6,  0, 11, 11,  0,  3, -1},

	{ 7, 11,  4, 11,  9,  4, 11,  5,  9, 11,  6,  5, -1,  0,  0,  0},

	{10,  4,  9, 10,  6,  4, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{10,  4,  9, 10,  6,  4,  8,  3,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 1, 10,  6,  1,  6,  0,  0,  6,  4, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  8,  6,  6,  1, 10,  6,  8,  1,  1,  8,  3, -1,  0,  0,  0},

	{ 9,  1,  2,  9,  2,  4,  4,  2,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3,  9,  1,  2,  9,  2,  4,  4,  2,  6, -1,  0,  0,  0},

	{ 0,  2,  6,  0,  6,  4, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3,  4,  8,  3,  2,  4,  4,  2,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 4, 10,  6,  4,  9, 10,  2,  3, 11, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  2,  0,  8, 11,  2,  4,  9, 10,  4, 10,  6, -1,  0,  0,  0},

	{ 2,  3, 11,  1, 10,  6,  1,  6,  0,  0,  6,  4, -1,  0,  0,  0},

	{ 8, 11,  2,  8,  2,  4,  4,  2,  1,  4,  1, 10,  4, 10,  6, -1},

	{ 3, 11,  1,  1,  4,  9, 11,  4,  1, 11,  6,  4, -1,  0,  0,  0},

	{ 6,  4,  9,  6,  9, 11, 11,  9,  1, 11,  1,  0, 11,  0,  8, -1},

	{11,  0,  3, 11,  6,  0,  0,  6,  4, -1,  0,  0,  0,  0,  0,  0},

	{ 8, 11,  6,  8,  6,  4, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 6,  7,  8,  6,  8, 10, 10,  8,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 3,  0,  7,  7, 10,  6,  0, 10,  7,  0,  9, 10, -1,  0,  0,  0},

	{ 1, 10,  6,  1,  6,  7,  1,  7,  0,  0,  7,  8, -1,  0,  0,  0},

	{ 6,  1, 10,  6,  7,  1,  1,  7,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 9,  1,  8,  8,  6,  7,  8,  1,  6,  6,  1,  2, -1,  0,  0,  0},

	{ 7,  3,  0,  7,  0,  6,  6,  0,  9,  6,  9,  1,  6,  1,  2, -1},

	{ 8,  6,  7,  8,  0,  6,  6,  0,  2, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  6,  7,  2,  7,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{11,  2,  3,  6,  7,  8,  6,  8, 10, 10,  8,  9, -1,  0,  0,  0},

	{ 9, 10,  6,  9,  6,  0,  0,  6,  7,  0,  7, 11,  0, 11,  2, -1},

	{ 3, 11,  2,  0,  7,  8,  0,  1,  7,  7,  1,  6,  6,  1, 10, -1},

	{ 6,  7, 10,  7,  1, 10,  7,  2,  1,  7, 11,  2, -1,  0,  0,  0},

	{ 1,  3, 11,  1, 11,  9,  9, 11,  6,  9,  6,  7,  9,  7,  8, -1},

	{ 6,  7, 11,  9,  1,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  0,  7,  0,  6,  7,  0, 11,  6,  0,  3, 11, -1,  0,  0,  0},

	{ 6,  7, 11, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 6, 11,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3,  0,  8, 11,  7,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 6, 11,  7,  9,  0,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  8,  3,  1,  9,  8,  7,  6, 11, -1,  0,  0,  0,  0,  0,  0},

	{11,  7,  6,  2, 10,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  2, 10,  0,  8,  3, 11,  7,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 9,  2, 10,  9,  0,  2, 11,  7,  6, -1,  0,  0,  0,  0,  0,  0},

	{11,  7,  6,  3,  2, 10,  3, 10,  8,  8, 10,  9, -1,  0,  0,  0},

	{ 2,  7,  6,  2,  3,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  7,  6,  8,  6,  0,  0,  6,  2, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  2,  3,  7,  6,  2,  1,  9,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  7,  9,  9,  2,  1,  9,  7,  2,  2,  7,  6, -1,  0,  0,  0},

	{ 6, 10,  1,  6,  1,  7,  7,  1,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 6, 10,  1,  6,  1,  0,  6,  0,  7,  7,  0,  8, -1,  0,  0,  0},

	{ 7,  6,  3,  3,  9,  0,  6,  9,  3,  6, 10,  9, -1,  0,  0,  0},

	{ 6,  8,  7,  6, 10,  8,  8, 10,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  6, 11,  8,  4,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{11,  3,  0, 11,  0,  6,  6,  0,  4, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  8,  4,  6, 11,  8,  0,  1,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 1,  9,  3,  3,  6, 11,  9,  6,  3,  9,  4,  6, -1,  0,  0,  0},

	{ 8,  6, 11,  8,  4,  6, 10,  1,  2, -1,  0,  0,  0,  0,  0,  0},

	{ 2, 10,  1, 11,  3,  0, 11,  0,  6,  6,  0,  4, -1,  0,  0,  0},

	{11,  4,  6, 11,  8,  4,  2, 10,  9,  2,  9,  0, -1,  0,  0,  0},

	{ 4,  6, 11,  4, 11,  9,  9, 11,  3,  9,  3,  2,  9,  2, 10, -1},

	{ 3,  8,  4,  3,  4,  2,  2,  4,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 2,  0,  4,  2,  4,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  1,  9,  3,  8,  4,  3,  4,  2,  2,  4,  6, -1,  0,  0,  0},

	{ 9,  2,  1,  9,  4,  2,  2,  4,  6, -1,  0,  0,  0,  0,  0,  0},

	{ 6, 10,  4,  4,  3,  8,  4, 10,  3,  3, 10,  1, -1,  0,  0,  0},

	{ 1,  6, 10,  1,  0,  6,  6,  0,  4, -1,  0,  0,  0,  0,  0,  0},

	{10,  9,  0, 10,  0,  6,  6,  0,  3,  6,  3,  8,  6,  8,  4, -1},

	{10,  9,  4, 10,  4,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 6, 11,  7,  5,  4,  9, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3,  9,  5,  4,  7,  6, 11, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  5,  4,  0,  1,  5,  6, 11,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  6, 11,  4,  8,  3,  4,  3,  5,  5,  3,  1, -1,  0,  0,  0},

	{ 2, 10,  1, 11,  7,  6,  5,  4,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  8,  3,  1,  2, 10,  4,  9,  5, 11,  7,  6, -1,  0,  0,  0},

	{ 6, 11,  7, 10,  5,  4, 10,  4,  2,  2,  4,  0, -1,  0,  0,  0},

	{ 6, 11,  7,  5,  2, 10,  5,  4,  2,  2,  4,  3,  3,  4,  8, -1},

	{ 2,  7,  6,  2,  3,  7,  4,  9,  5, -1,  0,  0,  0,  0,  0,  0},

	{ 4,  9,  5,  8,  7,  6,  8,  6,  0,  0,  6,  2, -1,  0,  0,  0},

	{ 3,  6,  2,  3,  7,  6,  0,  1,  5,  0,  5,  4, -1,  0,  0,  0},

	{ 1,  5,  4,  1,  4,  2,  2,  4,  8,  2,  8,  7,  2,  7,  6, -1},

	{ 5,  4,  9,  6, 10,  1,  6,  1,  7,  7,  1,  3, -1,  0,  0,  0},

	{ 4,  9,  5,  7,  0,  8,  7,  6,  0,  0,  6,  1,  1,  6, 10, -1},

	{ 3,  7,  6,  3,  6,  0,  0,  6, 10,  0, 10,  5,  0,  5,  4, -1},

	{ 4,  8,  5,  8, 10,  5,  8,  6, 10,  8,  7,  6, -1,  0,  0,  0},

	{ 5,  6, 11,  5, 11,  9,  9, 11,  8, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  9,  5,  0,  5,  6,  0,  6,  3,  3,  6, 11, -1,  0,  0,  0},

	{ 8,  0, 11, 11,  5,  6, 11,  0,  5,  5,  0,  1, -1,  0,  0,  0},

	{11,  5,  6, 11,  3,  5,  5,  3,  1, -1,  0,  0,  0,  0,  0,  0},

	{10,  1,  2,  5,  6, 11,  5, 11,  9,  9, 11,  8, -1,  0,  0,  0},

	{ 2, 10,  1,  3,  6, 11,  3,  0,  6,  6,  0,  5,  5,  0,  9, -1},

	{ 0,  2, 10,  0, 10,  8,  8, 10,  5,  8,  5,  6,  8,  6, 11, -1},

	{11,  3,  6,  3,  5,  6,  3, 10,  5,  3,  2, 10, -1,  0,  0,  0},

	{ 2,  3,  6,  6,  9,  5,  3,  9,  6,  3,  8,  9, -1,  0,  0,  0},

	{ 5,  0,  9,  5,  6,  0,  0,  6,  2, -1,  0,  0,  0,  0,  0,  0},

	{ 6,  2,  3,  6,  3,  5,  5,  3,  8,  5,  8,  0,  5,  0,  1, -1},

	{ 6,  2,  1,  6,  1,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  9,  5,  8,  5,  3,  3,  5,  6,  3,  6, 10,  3, 10,  1, -1},

	{ 1,  0, 10,  0,  6, 10,  0,  5,  6,  0,  9,  5, -1,  0,  0,  0},

	{ 0,  3,  8, 10,  5,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{10,  5,  6, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{11,  5, 10, 11,  7,  5, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 5, 11,  7,  5, 10, 11,  3,  0,  8, -1,  0,  0,  0,  0,  0,  0},

	{11,  5, 10, 11,  7,  5,  9,  0,  1, -1,  0,  0,  0,  0,  0,  0},

	{ 9,  3,  1,  9,  8,  3,  5, 10, 11,  5, 11,  7, -1,  0,  0,  0},

	{ 2, 11,  7,  2,  7,  1,  1,  7,  5, -1,  0,  0,  0,  0,  0,  0},

	{ 3,  0,  8,  2, 11,  7,  2,  7,  1,  1,  7,  5, -1,  0,  0,  0},

	{ 2, 11,  0,  0,  5,  9,  0, 11,  5,  5, 11,  7, -1,  0,  0,  0},

	{ 9,  8,  3,  9,  3,  5,  5,  3,  2,  5,  2, 11,  5, 11,  7, -1},

	{10,  2,  3, 10,  3,  5,  5,  3,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 5, 10,  7,  7,  0,  8, 10,  0,  7, 10,  2,  0, -1,  0,  0,  0},

	{ 1,  9,  0, 10,  2,  3, 10,  3,  5,  5,  3,  7, -1,  0,  0,  0},

	{ 7,  5, 10,  7, 10,  8,  8, 10,  2,  8,  2,  1,  8,  1,  9, -1},

	{ 7,  5,  1,  7,  1,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  1,  0,  8,  7,  1,  1,  7,  5, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  5,  9,  0,  3,  5,  5,  3,  7, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  5,  9,  7,  9,  8, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  5, 10,  4, 10,  8,  8, 10, 11, -1,  0,  0,  0,  0,  0,  0},

	{11,  3, 10, 10,  4,  5, 10,  3,  4,  4,  3,  0, -1,  0,  0,  0},

	{ 9,  0,  1,  4,  5, 10,  4, 10,  8,  8, 10, 11, -1,  0,  0,  0},

	{ 3,  1,  9,  3,  9, 11, 11,  9,  4, 11,  4,  5, 11,  5, 10, -1},

	{ 8,  4, 11, 11,  1,  2,  4,  1, 11,  4,  5,  1, -1,  0,  0,  0},

	{ 5,  1,  2,  5,  2,  4,  4,  2, 11,  4, 11,  3,  4,  3,  0, -1},

	{11,  8,  4, 11,  4,  2,  2,  4,  5,  2,  5,  9,  2,  9,  0, -1},

	{ 2, 11,  3,  5,  9,  4, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  5, 10,  4, 10,  2,  4,  2,  8,  8,  2,  3, -1,  0,  0,  0},

	{10,  4,  5, 10,  2,  4,  4,  2,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  1,  9,  8,  2,  3,  8,  4,  2,  2,  4, 10, 10,  4,  5, -1},

	{10,  2,  5,  2,  4,  5,  2,  9,  4,  2,  1,  9, -1,  0,  0,  0},

	{ 4,  3,  8,  4,  5,  3,  3,  5,  1, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  4,  5,  0,  5,  1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  3,  9,  3,  5,  9,  3,  4,  5,  3,  8,  4, -1,  0,  0,  0},

	{ 4,  5,  9, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 7,  4,  9,  7,  9, 11, 11,  9, 10, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  3,  0,  7,  4,  9,  7,  9, 11, 11,  9, 10, -1,  0,  0,  0},

	{ 0,  1,  4,  4, 11,  7,  1, 11,  4,  1, 10, 11, -1,  0,  0,  0},

	{10, 11,  7, 10,  7,  1,  1,  7,  4,  1,  4,  8,  1,  8,  3, -1},

	{ 2, 11,  7,  2,  7,  4,  2,  4,  1,  1,  4,  9, -1,  0,  0,  0},

	{ 0,  8,  3,  1,  4,  9,  1,  2,  4,  4,  2,  7,  7,  2, 11, -1},

	{ 7,  2, 11,  7,  4,  2,  2,  4,  0, -1,  0,  0,  0,  0,  0,  0},

	{ 7,  4, 11,  4,  2, 11,  4,  3,  2,  4,  8,  3, -1,  0,  0,  0},

	{ 7,  4,  3,  3, 10,  2,  3,  4, 10, 10,  4,  9, -1,  0,  0,  0},

	{ 2,  0,  8,  2,  8, 10, 10,  8,  7, 10,  7,  4, 10,  4,  9, -1},

	{ 4,  0,  1,  4,  1,  7,  7,  1, 10,  7, 10,  2,  7,  2,  3, -1},

	{ 4,  8,  7,  1, 10,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 9,  7,  4,  9,  1,  7,  7,  1,  3, -1,  0,  0,  0,  0,  0,  0},

	{ 8,  7,  0,  7,  1,  0,  7,  9,  1,  7,  4,  9, -1,  0,  0,  0},

	{ 4,  0,  3,  4,  3,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 4,  8,  7, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  9, 10,  8, 10, 11, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0, 11,  3,  0,  9, 11, 11,  9, 10, -1,  0,  0,  0,  0,  0,  0},

	{ 1,  8,  0,  1, 10,  8,  8, 10, 11, -1,  0,  0,  0,  0,  0,  0},

	{ 3,  1, 10,  3, 10, 11, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 2,  9,  1,  2, 11,  9,  9, 11,  8, -1,  0,  0,  0,  0,  0,  0},

	{ 0,  9,  3,  9, 11,  3,  9,  2, 11,  9,  1,  2, -1,  0,  0,  0},

	{11,  8,  0, 11,  0,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 2, 11,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3, 10,  2,  3,  8, 10, 10,  8,  9, -1,  0,  0,  0,  0,  0,  0},

	{ 9, 10,  2,  9,  2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 3,  8,  2,  8, 10,  2,  8,  1, 10,  8,  0,  1, -1,  0,  0,  0},

	{ 2,  1, 10, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 8,  9,  1,  8,  1,  3, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 1,  0,  9, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{ 0,  3,  8, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

	{-1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},

};



int edge_vertices[12][2]{

	{0, 1},

	{1, 2},

	{2, 3},

	{3, 0},

	{4, 5},

	{6, 5},

	{6, 7},

	{7, 4},

	{0, 4},

	{1, 5},

	{2, 6},

	{3, 7},

};



int index_to_vertex[8][3]{

	{0, 0, 0},

	{1, 0, 0},

	{1, 1, 0},

	{0, 1, 0},

	{0, 0, 1},

	{1, 0, 1},

	{1, 1, 1},

	{0, 1, 1},

};




// form a front-facing triangle per non zero vertex
// should give a vague frog shape
void simpleSurface(const std::vector<char>& input,
	const int dim[3], // x y z dimension of the data
	std::vector<float>& output_vertices,
	std::vector<float>& output_normals);


// the marching cube algorithm
// 
void MarchingCube(const std::vector<char>& input,
	const int dim[3], // x y z dimension of the data
	std::vector<float>& output_vertices,
	std::vector<float>& output_normals, char threshold) {

	// fill the outputs with results applying Marching cube on input

	/*

  A diagram for convenience while planning.

  z
  ^    _  y
  |    /|
  |  /
  |/______> X
	   ________
	 /|       /|
   /__|____ /  |
  |   |	 |   |
  |   |____|___|
  |  /     |  /
  |/_______|/

	*/

	float vert[3]{ 0,0,0 };

	int whichVertex = 0;
		
    for (int currentZ = 0; currentZ < dim[2] - 1; currentZ++)
		for (int currentY = 0; currentY < dim[1] - 1; currentY++)
			for (int currentX = 0; currentX < dim[0] - 1; currentX++)
			{
				/*
					Some assumptions I am making about the array shape.
					--------> x direction
					|
					|
					|
					V y direction
				*/
				char cornerLowerLeftFront = input[currentX + currentY * dim[0] + currentZ * dim[0] * dim[1]];
				char cornerLowerRightFront = input[currentX + 1 + currentY * dim[0] + currentZ * dim[0] * dim[1]];
				char cornerLowerLeftBack = input[currentX + (currentY + 1) * dim[0] + currentZ * dim[0] * dim[1]];
				char cornerLowerRightBack = input[currentX + 1 + (currentY + 1) * dim[0] + currentZ * dim[0] * dim[1]];
				char cornerUpperLeftFront = input[currentX + currentY * dim[0] + (currentZ + 1) * dim[0] * dim[1]];
				char cornerUpperRightFront = input[currentX + 1 + currentY * dim[0] + (currentZ + 1) * dim[0] * dim[1]];
				char cornerUpperLeftBack = input[currentX + (currentY + 1) * dim[0] + (currentZ + 1) * dim[0] * dim[1]];
				char cornerUpperRightBack = input[currentX + 1 + (currentY + 1) * dim[0] + (currentZ + 1) * dim[0] * dim[1]];
				bool cornerLowerLeftFrontAboveThreshold = cornerLowerLeftFront > threshold;
				bool cornerLowerRightFrontAboveThreshold = cornerLowerRightFront > threshold;
				bool cornerLowerLeftBackAboveThreshold = cornerLowerLeftBack > threshold;
				bool cornerLowerRightBacktAboveThreshold = cornerLowerRightBack > threshold;
				bool cornerUpperLeftFrontAboveThreshold = cornerUpperLeftFront > threshold;
				bool cornerUpperRightFrontAboveThreshold = cornerUpperRightFront > threshold;
				bool cornerUpperLeftBackAboveThreshold = cornerUpperLeftBack > threshold;
				bool cornerUpperRightBackAboveThreshold = cornerUpperRightBack > threshold;
				int howManyCornersAbove = 0;
				if (cornerLowerLeftFrontAboveThreshold)
					howManyCornersAbove++;
				if (cornerLowerRightFrontAboveThreshold)
					howManyCornersAbove++;
				if (cornerLowerLeftBackAboveThreshold)
					howManyCornersAbove++;
				if (cornerLowerRightBacktAboveThreshold)
					howManyCornersAbove++;
				if (cornerUpperLeftFrontAboveThreshold)
					howManyCornersAbove++;
				if (cornerUpperRightFrontAboveThreshold)
					howManyCornersAbove++;
				if (cornerUpperLeftBackAboveThreshold)
					howManyCornersAbove++;
				if (cornerUpperRightBackAboveThreshold)
					howManyCornersAbove++;

				bool vertexValues[8]{ cornerLowerLeftFrontAboveThreshold, cornerLowerRightFrontAboveThreshold, cornerLowerRightBacktAboveThreshold, cornerLowerLeftBackAboveThreshold, cornerUpperLeftFrontAboveThreshold, cornerUpperRightFrontAboveThreshold, cornerUpperRightBackAboveThreshold,cornerUpperLeftBackAboveThreshold };

				// The triangle table gives us the mapping from index to actual

				// triangles to return for this configuration

				int index = 0;
				

				for (int v = 0; v < 8; ++v) {

					if (!vertexValues[v]) {

						index |= 1 << v;

					}

				}

				for (int t = 0; triTable[index][t] != -1; ++t) {

					int v0 = edge_vertices[triTable[index][t]][0];

					int v1 = edge_vertices[triTable[index][t]][1];


					

					int va[3]{ index_to_vertex[v0][0], index_to_vertex[v0][1], index_to_vertex[v0][2] };

					int vb[3]{ index_to_vertex[v1][0], index_to_vertex[v1][1], index_to_vertex[v1][2] };

					int fa = vertexValues[v0];

					int fb = vertexValues[v1];

					int isoval = threshold;

					float t2 = 0.0;


					/*
					if (abs(fa - fb) < 0.0001) {

						t2 = 0.0;

					}
					else {

						//t2 = (isoval - fa) / (fb - fa);

					}
					*/

					

					vert[0] = va[0] + t2;// *(vb[0] - va[0]);

					vert[1] = va[1] + t2;// *(vb[1] - va[1]);

					vert[2] = va[2] + t2;//  *(vb[2] - va[2]);

					
					

					// Note: The vertex positions need to be placed on the dual grid,

					// since that's where the isosurface is computed and defined.

					vert[0] += currentX + 0.5;

					vert[1] += currentY + 0.5;

					vert[2] += currentZ + 0.5;

					output_vertices.push_back(float(vert[0]) / float(dim[0])- 0.5);
					output_vertices.push_back(float(vert[1]) / float(dim[0]) - 0.5);
					output_vertices.push_back(float(vert[2]) / float(dim[0]));

					whichVertex++;

					if (whichVertex >= 3)
					{
						whichVertex = 0;

						float p1x = output_vertices.at(output_vertices.size() - 9);
						float p1y = output_vertices.at(output_vertices.size() - 8);
						float p1z = output_vertices.at(output_vertices.size() - 7);
						float p2x = output_vertices.at(output_vertices.size() - 6);
						float p2y = output_vertices.at(output_vertices.size() - 5);
						float p2z = output_vertices.at(output_vertices.size() - 4);
						float p3x = output_vertices.at(output_vertices.size() - 3);
						float p3y = output_vertices.at(output_vertices.size() - 2);
						float p3z = output_vertices.at(output_vertices.size() - 1);
						float firstVector[3]{ p2x - p1x, p2y - p1y, p2z - p1z };
						float secondVector[3]{ p3x - p1x, p3y - p1y, p3z - p1z };
						float normalX = firstVector[1] * secondVector[2] - firstVector[2] * secondVector[1];
						float normalY = firstVector[2] * secondVector[0] - firstVector[0] * secondVector[2];
						float normalZ = firstVector[0] * secondVector[1] - firstVector[1] * secondVector[0];
						output_normals.push_back(normalX / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalY / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalZ / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalX / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalY / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalZ / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalX / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalY / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
						output_normals.push_back(normalZ / (sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)));
					}


				//The contents below were a prior implimentation. Ignore for all future purposes.
				/*
				if (howManyCornersAbove != 0 && howManyCornersAbove != 8)
				{
					/* These were some cases. They are currently not being used.
					if (howManyCornersAbove == 7)
					{
						//Case 1
						if (!cornerLowerLeftFrontAboveThreshold)
						{
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

						}
						if (!cornerLowerRightFrontAboveThreshold)
						{
							currentX += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentX -= 1;

						}
						if (!cornerLowerLeftBackAboveThreshold)
						{
							currentY += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentY -= 1;
						}
						if (!cornerLowerRightBacktAboveThreshold)
						{
							currentX += 1;
							currentY += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentX -= 1;
							currentY -= 1;
						}
						if (!cornerUpperLeftFrontAboveThreshold)
						{
							currentZ += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentZ -= 1;
						}
						if (!cornerUpperRightFrontAboveThreshold)
						{
							currentX += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentX -= 1;
							currentZ -= 1;
						}
						if (!cornerUpperLeftBackAboveThreshold)
						{
							currentY += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentY -= 1;
							currentZ -= 1;
						}
						if (!cornerUpperRightBackAboveThreshold)
						{
							currentX += 1;
							currentY += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentX -= 1;
							currentY -= 1;
							currentZ -= 1;
						}

					}

					if (howManyCornersAbove == 1)
					{
						//Case 1
						if (cornerLowerLeftFrontAboveThreshold)
						{
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

						}
						if (cornerLowerRightFrontAboveThreshold)
						{
							currentX += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentX -= 1;

						}
						if (cornerLowerLeftBackAboveThreshold)
						{
							currentY += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentY -= 1;
						}
						if (cornerLowerRightBacktAboveThreshold)
						{
							currentX += 1;
							currentY += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
							currentX -= 1;
							currentY -= 1;
						}
						if (cornerUpperLeftFrontAboveThreshold)
						{
							currentZ += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentZ -= 1;
						}
						if (cornerUpperRightFrontAboveThreshold)
						{
							currentX += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentX -= 1;
							currentZ -= 1;
						}
						if (cornerUpperLeftBackAboveThreshold)
						{
							currentY += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentY -= 1;
							currentZ -= 1;
						}
						if (cornerUpperRightBackAboveThreshold)
						{
							currentX += 1;
							currentY += 1;
							currentZ += 1;
							output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
							output_vertices.push_back(float(currentZ) / float(dim[0]));

							output_vertices.push_back(float(currentX) / float(dim[0]));
							output_vertices.push_back(float(currentY) / float(dim[0]));
							output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
							currentX -= 1;
							currentY -= 1;
							currentZ -= 1;
						}

					}

					//Case 2
					if (howManyCornersAbove == 6)
					{
						if (!cornerLowerLeftFrontAboveThreshold)
						{
							if (!cornerLowerRightFrontAboveThreshold)
							{
								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
							}
							else if (!cornerLowerLeftBackAboveThreshold)
							{
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
							}
							else if (!cornerUpperLeftFrontAboveThreshold)
							{
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
							}
							else
							{
								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));




								if (!cornerLowerRightBacktAboveThreshold)
								{
									currentX += 1;
									currentY += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
									currentX -= 1;
									currentY -= 1;
								}

								if (!cornerUpperRightFrontAboveThreshold)
								{
									currentX += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentX -= 1;
									currentZ -= 1;
								}
								if (!cornerUpperLeftBackAboveThreshold)
								{
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentY -= 1;
									currentZ -= 1;
								}
								if (!cornerUpperRightBackAboveThreshold)
								{
									currentX += 1;
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentX -= 1;
									currentY -= 1;
									currentZ -= 1;
								}
							}
						}
						else if (!cornerLowerRightFrontAboveThreshold)
						{
							if (!cornerLowerRightBacktAboveThreshold)
							{
								currentX += 1;
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
								currentX -= 1;
							}
							else if (!cornerUpperRightFrontAboveThreshold)
							{
								currentX += 1;
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
								currentX -= 1;
							}
							else
							{
								currentX += 1;
								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
								currentX -= 1;



								if (!cornerLowerLeftBackAboveThreshold)
								{
									currentY += 1;
									output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
									currentY -= 1;
								}

								if (!cornerUpperLeftFrontAboveThreshold)
								{
									currentZ += 1;
									output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentZ -= 1;
								}

								if (!cornerUpperLeftBackAboveThreshold)
								{
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentY -= 1;
									currentZ -= 1;
								}
								if (!cornerUpperRightBackAboveThreshold)
								{
									currentX += 1;
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentX -= 1;
									currentY -= 1;
									currentZ -= 1;
								}
							}
						}
						else if (!cornerUpperLeftFrontAboveThreshold)
						{
							if (!cornerUpperRightFrontAboveThreshold)
							{
								currentZ += 1;
								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
								currentZ -= 1;
							}
							else if (!cornerLowerLeftBackAboveThreshold)
							{
								currentZ += 1;
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 1.0) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
								currentZ -= 1;
							}
							else if (!cornerUpperLeftFrontAboveThreshold)
							{
								currentZ += 1;
								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ - 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 1.0) / float(dim[0]));

								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));
								currentZ -= 1;
							}
							else
							{
								currentZ += 1;
								output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
								output_vertices.push_back(float(currentZ) / float(dim[0]));

								output_vertices.push_back(float(currentX) / float(dim[0]));
								output_vertices.push_back(float(currentY) / float(dim[0]));
								output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
								currentZ -= 1;

								if (!cornerLowerRightBacktAboveThreshold)
								{
									currentX += 1;
									currentY += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ + 0.5) / float(dim[0]));
									currentX -= 1;
									currentY -= 1;
								}

								if (!cornerUpperRightFrontAboveThreshold)
								{
									currentX += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentX -= 1;
									currentZ -= 1;
								}
								if (!cornerUpperLeftBackAboveThreshold)
								{
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX + 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentY -= 1;
									currentZ -= 1;
								}
								if (!cornerUpperRightBackAboveThreshold)
								{
									currentX += 1;
									currentY += 1;
									currentZ += 1;
									output_vertices.push_back(float(currentX - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY - 0.5) / float(dim[0]));
									output_vertices.push_back(float(currentZ) / float(dim[0]));

									output_vertices.push_back(float(currentX) / float(dim[0]));
									output_vertices.push_back(float(currentY) / float(dim[0]));
									output_vertices.push_back(float(currentZ - 0.5) / float(dim[0]));
									currentX -= 1;
									currentY -= 1;
									currentZ -= 1;
								}
							}
						}
					}
					if (howManyCornersAbove == 2)
					{

					}

					
					//For now, just draws a cube..


					///*
	   ________
	 /|       /|
  z/__|____ /  |
  |   |	   |   |
  |   |y___|___|
  |  /     |  /
  |/_______|/ x


					


					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));


					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));



					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));


					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));



					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));


					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));




					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));


					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));




					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));


					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));




					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));


					output_vertices.push_back(float(currentX) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ) / float(dim[0]));

					output_vertices.push_back(float(currentX + 1) / float(dim[0]));
					output_vertices.push_back(float(currentY + 1) / float(dim[0]));
					output_vertices.push_back(float(currentZ + 1) / float(dim[0]));

				}
				*/
				}
			}

	// this is a debug thing to show the frog contour
	//simpleSurface(input, dim, output_vertices, output_normals);

}











// form a front-facing triangle per non zero vertex
// should give a vague frog shape

void simpleSurface(const std::vector<char>& input,
	const int dim[3], // x y z dimension of the data
	std::vector<float>& output_vertices,
	std::vector<float>& output_normals) {

	for (int i = 0; i < dim[0] * dim[1] * dim[2]; ++i)
	{

		if (input[i]) {
			int position[3];
			position[0] = i % dim[0];
			position[1] = i / dim[0] % dim[1];
			position[2] = i / (dim[0] * dim[1]);

			float position_unit[3];
			for (int j = 0; j < 3; ++j)
				position_unit[j] = float(position[j]) / dim[0] - 0.5f; // x is the largest

			float triangle_size = 0.01;

			output_vertices.push_back(position_unit[0]);
			output_vertices.push_back(position_unit[1] + triangle_size);
			output_vertices.push_back(position_unit[2]);

			output_vertices.push_back(position_unit[0] + triangle_size);
			output_vertices.push_back(position_unit[1] - triangle_size);
			output_vertices.push_back(position_unit[2]);


			output_vertices.push_back(position_unit[0] - triangle_size);
			output_vertices.push_back(position_unit[1] - triangle_size);
			output_vertices.push_back(position_unit[2]);
		}
	}

}
