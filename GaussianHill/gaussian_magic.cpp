//This file is intended to try to use limiters on velocity field to address stable simulations with d1q3
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <limits>

//Domain size
const int NX=50;

//Time steps
int N=20;
int NOUTPUT=1;


int xInit = 25;
int sigma=3;

//Fields and populations
double g[NX][3], g2[NX][3];
double phase[NX];

//Diffusion parameters
double ux_main=0.5;

//Underlying lattice parameters
double weights[]={2.0/3.0,1.0/6.0,1.0/6.0};
int cx[]={0,1,-1};
int compliment[]={0,2,1};

double magic=1.0/6.0;
double omega=0.3;


void writephase(std::string const & fName)
{
	std::string fullName = "tmp_magic6/" + fName+ ".dat";
	std::ofstream fout(fullName.c_str());
	fout.precision(10);

	for (int iX=0; iX<NX; ++iX)
		fout<<phase[iX]<<" ";
	fout<<"\n";
}


void init()
{

	//Phase initialization prior any equilibrium functions calculations
	for(int iX=0;iX<NX;iX++)
	{
	    if (abs(iX-xInit) <= 3*sigma)
	        phase[iX]=exp(-0.5*double(iX-xInit)*double(iX-xInit)/(double(sigma)*double(sigma)));
	    else
	        phase[iX]=0.0;
	}

	//Bulk nodes initialization
	for(int iX=0;iX<NX;iX++)
	{
		//Initialization of the macroscopic fields
		for (int k=0; k<3; k++)
			g[iX][k] = weights[k]*phase[iX]*(1.0 + 3.0 * cx[k]*ux_main + 4.5 * (cx[k]*cx[k]-1.0/3.0)*ux_main*ux_main); 
		
	}

}


void collide_bulk(int counter)
{
	//Construction of phase field prior all other calculations
	for(int iX=0;iX<NX;iX++)
	{
		phase[iX]=0.0;
		for(int iPop=0;iPop<3;iPop++)
			phase[iX]+=g[iX][iPop];	
	}

    for(int iX=0;iX<NX;iX++)
 	{
        double g_plus[3],g_minus[3];
		double geq[3],geq_plus[3],geq_minus[3];        

        for (int k=0; k<3; k++)
        {
            g_plus[k]=0.5*(g[iX][k]+g[iX][compliment[k]]);
            g_minus[k]=0.5*(g[iX][k]-g[iX][compliment[k]]);
        }

        //The equilibrium populations are taken from Irina's bb_pressure and bb_examples
		for (int k=0; k<3; k++)
		{
			geq[k]=weights[k]*phase[iX]*(1.0+3.0*cx[k]*ux_main+4.5*(cx[k]*cx[k]-1.0/3.0)*ux_main*ux_main);			
		}


        for (int k=0; k<3; k++)
        {
            geq_plus[k]=0.5*(geq[k]+geq[compliment[k]]);
            geq_minus[k]=0.5*(geq[k]-geq[compliment[k]]);
        }

        //Determine which one are related to which one
        //double omega_minus_phase=omega;
        double omega_minus_phase=1.0/(magic/(1.0/omega-0.5)+0.5);
        
        //double omega_plus_phase=1.0/(magic/(1.0/omega-0.5)+0.5);
        double omega_plus_phase=omega;
 
		for(int k=0; k < 3; k++)
		{
			g2[iX][k]=g[iX][k]-omega_plus_phase*(g_plus[k]-geq_plus[k])-omega_minus_phase*(g_minus[k]-geq_minus[k]);
		}

		//for(int k=0; k < 3; k++)
		//{
		//	g2[iX][k]=g[iX][k]-omega_phase*(g[iX][k]-geq[k]);
		//	f2[iX][k]=f[iX][k]-omega_temp*(f[iX][k]-feq[k])+3.0*force*cx[k];			
		//}

	}
	
}


int main(int argc, char* argv[])
{
    init();
	for(int counter=0;counter<=N;counter++)
	{

        collide_bulk(counter);

		//Streaming
		for(int iX=0;iX<NX;iX++)
			for(int iPop=0;iPop<3;iPop++)
			{
				int iX2=(iX-cx[iPop]+NX)%NX;
	            g[iX][iPop]=g2[iX2][iPop];
			}

		//Writing files
		if (counter%NOUTPUT==0)
		{
			std::cout<<counter<<"\n";
 			std::stringstream filewritephase;
 			std::stringstream counterconvert;
 			counterconvert<<counter;
			filewritephase<<std::fixed;

            filewritephase<<"phase"<<std::string(6-counterconvert.str().size(),'0')<<counter;

            writephase(filewritephase.str());

		}


	}

	return 0;
}
