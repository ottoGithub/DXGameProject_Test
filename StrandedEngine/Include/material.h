/*
   Demo Name:  Game Project 4
      Author:  Allen Sherrod
     Chapter:  Chapter 5
*/


#ifndef _UGP_MATERIAL_H_
#define _UGP_MATERIAL_H_

struct stMaterial
{
   stMaterial()
   {
      emissiveR = emissiveG = emissiveB = emissiveA = 0;
      ambientR = ambientG = ambientB = ambientA = 1;
      diffuseR = diffuseG = diffuseB = diffuseA = 1;
      specularR = specularG = specularB = specularA = 0;
      power = 0;
   }

   float emissiveR, emissiveG, emissiveB, emissiveA;
   float ambientR, ambientG, ambientB, ambientA;
   float diffuseR, diffuseG, diffuseB, diffuseA;
   float specularR, specularG, specularB, specularA;
   float power;
};

#endif