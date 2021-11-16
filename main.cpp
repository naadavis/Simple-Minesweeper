// Nicholas Davis
// 10/16/2020

#include <iostream>
#include <iomanip>
#include <cstring>
#include "stdio.h"
#include "MineSolver.h"
using namespace std;

int main( int argc, char *argv[] )
{
	int batch = -1;
	Minefield mf;
	MineSolver msolve( &mf );

	if( argc > 1 )
	{
		bool badflag = false;
		for( int i = 1; i < argc; ++i )
		{
			if( badflag ) break; 
			if( strcmp( argv[i], "-s" ) == 0 )	// set seed
			{
				if( argc > ++i )
				{
					unsigned int s;
					if( sscanf( argv[i], "%u", &s ) == 1 )
					{
						mf.seed( s );
					}
					else
					{
						badflag = true;
					}
				} 
				else 
				{
					badflag = true;
				}
			}
			else if( strcmp( argv[i], "-d" ) == 0 )	// print debugging info, basically enable cerr printing
			{
				msolve.sDebug( true );
			}
			else if( strcmp( argv[i], "-b" ) == 0 )
			{
				if( argc > ++i )
				{	
					if( !(sscanf( argv[i], "%d", &batch ) == 1 ) )
					{
						badflag = true;
					}
				} 
				else 
				{
					badflag = true;
				}
			}
			else
			{
				badflag = true;
			}
		}
		if( badflag )
		{
			std::cout << "Bad Arguments" << std::endl;
			return -1;
		}
	}

	if( batch > 1 )
	{
		mf.printSeed();
		std::cout << std::endl;
		int wins = 0;
		for( int i = 0; i < batch; ++i )
		{
			MineSolver batchsolver( &mf );
			if( batchsolver.solve() )
			{
				std::cout << "Solved!" << std::endl;
				++wins;
			}
			else
			{
				std::cout << "failed." << std::endl;
			}
			mf.reset();
		}
		std::cout << wins << " wins out of " << batch << " games. " << 100*wins/(float)batch << " Winrate." << std::endl;
		return 0;
	}

	mf.printSeed();
	std::cout << std::endl;

	while( msolve.stepSolve() )
	{
	//	msolve.printGuide();
	//	cout << endl;
	}

	msolve.printGuide();
	cout << endl;

	int w = mf.gWidth();
	int h = mf.gHeight();
	for( int i = 0; i < h; ++i )
	{
		for( int j = 0; j < w; ++j )
		{
			cout << setw(5) << (int)mf.flip(i,j) << flush;
		}
		cout << endl;
	}
	return 0;
};
