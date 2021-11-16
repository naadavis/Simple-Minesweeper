// A minefield class to allow a solver to work with

#include <vector>
#include <stdint.h>
#include <cstddef>

class Minefield
{
	public:
		Minefield( const size_t& h = 16, const size_t& w = 30, const size_t& bombs = 99 );
		void reformat( const size_t& h, const size_t& w, const size_t& b );
		int8_t flip( const size_t& y, const size_t& x );
		void reset();
		void seed( const unsigned int& s );
		void printSeed();
		const size_t gWidth() { return width; }
		const size_t gHeight() { return height; }
		const size_t gBombs() { return num_bombs; }
	private:
		std::vector<int8_t> board;
		size_t width;
		size_t height;
		size_t num_bombs;
		bool init;
		bool seeded;
		unsigned int seed_val;
		void countBoard();
};
