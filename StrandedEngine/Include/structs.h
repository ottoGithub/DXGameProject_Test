/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#ifndef _UGP_STRUCTS_H_
#define _UGP_STRUCTS_H_


// A structure for our custom vertex type.
struct stGUIVertex
{
    float x, y, z, rhw;
    unsigned long color;
    float tu, tv;
};

#endif