// Minefield solver
#include "Minefield.h"
#include <list>
#include <iostream>
#include <iomanip>
#include <map>

class MineSolver
{
	public:
		MineSolver( Minefield* const &mboard = nullptr );
		void setMinefield( Minefield* const &mboard );
		bool stepSolve();
		bool solve();
		void printGuide();
		void sDebug( const bool &b ) { debug = b; }
	private:
		struct unit
		{
			bool known;
			int8_t val;
			unit() : known(false) {}
		};
		struct group{
			std::list<int> elements;
			int max_mines;
			int min_mines;
			void print( std::ostream& os);
		};
		enum Tribool { Yes = 1, No = 0, None = -1 };
		Minefield *field;
		std::vector<unit> guide;
		std::list<int> todo;
		int remaining_mines;
		int num_revealed;
		bool debug;
		bool reveal( const int& i );
		void flag( const int& i );
		std::list<int> getNeighbors( const int& i );
		MineSolver::Tribool NeighborReason( const int &i, const int &nmines, const std::list<int> &un);
		std::list<int> intersect( const std::list<int>&, const std::list<int>& );
		std::list<int> difference( const std::list<int>&, const std::list<int>& );
		MineSolver::Tribool checkIntersections( const std::list<int>&unflip, const int& nminemax, const int& nminemin,
									std::list<group>::iterator start, const std::list<group>::iterator &end );
		MineSolver::Tribool IntersectiveReason( const std::list<int> &un, const std::vector<unit> &fguide );
		MineSolver::Tribool Guess();
		int findPermutations( std::map<int,int>&, std::map<int,int>::iterator, std::vector<unit> );
		bool isLegal( const std::vector<unit> &fguide );

};
