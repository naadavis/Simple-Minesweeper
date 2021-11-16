//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include "Minefield.h"

Minefield::Minefield( const size_t& h, const size_t& w, const size_t& bombs )
{
	seed_val = time(NULL);
	srand(seed_val);
	reformat( h, w, bombs );
}

void Minefield::reformat( const size_t& h, const size_t& w, const size_t& b )
{
	board.resize( h*w );

	width = w;
	height = h;
	num_bombs = b;
	init = false;
}

void Minefield::reset()
{
	init = false;
	for( auto &i : board ) { i = 0; }
}

int8_t Minefield::flip( const size_t& y, const size_t& x )
{
	if( !init )		// need to populate board with mines
	{
		auto reserves = num_bombs;
		size_t skipped = y*width + x;

		// place mines down
		int pos = 0;
		while( reserves > 0 )
		{
			if( pos == skipped ) 
			{
				++pos;
				continue;
			}
			board[pos++] = -1;
			--reserves;
		}

		// shuffle positions
		for( int i = 0; i < width*height-1; i++ )
		{
			if( i == skipped ) continue;
			auto target = rand() % ( width*height - 1 - i );
			target += ( target >= skipped ) + i;
			auto temp = board[i];
			board[i] = board[target];
			board[target] = temp;
		}
		// now populate board with numbers
		countBoard();
		init = true;
	}
	// now return the value of the flip
	return board[ y*width + x ];
}

void Minefield::seed( const unsigned int& s )
{
	seed_val = s;
	srand(seed_val);
}

void Minefield::printSeed()
{
	std::cout << "Seed: " << seed_val;
}

void Minefield::countBoard()
{
	for( int i = 0; i < board.size(); ++i )
	{
		if( board[i] == -1 ) // is a bomb
		{
			int x = i % width;
			int y = i / width;
			for( int k = x-1; k < x+2; ++k )
			{
				// check bounds
				if( k < 0 || k > width - 1 ) continue;
				for( int j = y-1; j < y+2; ++j )
				{
					if( j < 0 || j > height -1 ) continue;
					// in bounds and not a bomb, increment
					if( board[ j*width + k ]  != -1 )
					{
						++board[ j*width + k ];
					}
				}
			}
		}
	}
}
