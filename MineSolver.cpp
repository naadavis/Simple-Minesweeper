#include "MineSolver.h"
#include <unordered_set>

MineSolver::MineSolver(  Minefield* const &mboard )
{
	debug = false;
	setMinefield( mboard );
}

void MineSolver::setMinefield( Minefield* const &mboard )
{
	field = mboard;
	num_revealed = 0;
	if( field != nullptr )
	{
		remaining_mines = mboard->gBombs();
		guide.resize( mboard->gHeight()*mboard->gWidth() );
	}
}

void MineSolver::group::print( std::ostream &os )
{
	os << "{[" << min_mines << "," << max_mines << "] ";
	for( auto &i : elements ) { os << i << " "; }
	os << "}";
}

bool MineSolver::solve()
{
	while( true )
	{
		if( num_revealed == field->gWidth()*field->gHeight() - field->gBombs() )
		{
			return true;
		}
		else
		{
			if( stepSolve() == false ) return false;
		}
	}
}

bool MineSolver::stepSolve()
{
	// Win Condition, Reveal all non bombs
	if( num_revealed == field->gWidth()*field->gHeight() - field->gBombs() )
	{
		std::cout << "SOLVED!" << std::endl;
		return false;	// no further moves
	}

	// Reveal all remaining unreveled
	if( remaining_mines == 0 )	// all remaining are not mines
	{
		for( int i = 0; i < guide.size(); ++i )
		{
			if( !guide[i].known )
			{
				if( !reveal(i) ) return false;
			}
		}
		return true;
	}

	// making a move with no information
	// is todo queue ever empty after starting??
	// probably
	if( todo.empty() )
	{
		//std::cerr << "Empty Todo Queue. Straight Guess." << std::endl;
		for( int i = 0; i < guide.size(); ++i )
		{
			if( !guide[i].known )
			{
				return reveal(i);
			}
		}
	}

	// go through the to do list
	// Single Unit Reasoning --------------------------------------------------------------------
	//std::cerr << "Single Unit Reason Begin - " << std::endl;
	auto it = todo.begin();
	while( it != todo.end() )
	{
		auto i = *it;
		auto nbors = getNeighbors( i );

		// ensure there is work to do, as other units may have covered this one already
		std::list<int> loc_unflip;
		for( auto &j : nbors ) { if( !guide[j].known ) loc_unflip.push_back(j); }
		if( loc_unflip.empty() )
		{
			auto temp = it++;
			todo.erase(temp);
			continue;
		}

		// get number of flagged mines around this unit
		int loc_mines = 0;
		for( auto &j : nbors ) { if( guide[j].known && guide[j].val == -1 ) ++loc_mines; }

		if( guide[i].val == loc_mines )		// reveal all unflipped units, everything flagged
		{
			while( !loc_unflip.empty() )
			{
				// flip unit
				if( !reveal(loc_unflip.front()) ) { return false; }
				// remove flipped
				loc_unflip.pop_front();

			}
			todo.erase(it);	// done solving this unit
			return true;
		}
		else if( guide[i].val == loc_mines + loc_unflip.size() )	// flag everything
		{
			for( auto &j : loc_unflip )
			{
				flag(j);
			}
			todo.erase(it);	// done with this unit
			return true;
		}
		++it;
	}

	//std::cerr << "Intersective Reason Begin - " << std::endl;
	// Intersective reasoning
	switch( IntersectiveReason( todo, guide ) )
	{
		case Yes:
			return true;
		case No:
			return false;
		default:	// no move found, keep going
			break;
	}

	//std::cerr << "Guessing - " << std::endl;
	switch( Guess() )
	{
		case Yes:
			return true;
		case No:
			return false;
		default:	// no move found, keep going
			break;
	}

	std::cout << "Stumped. " << remaining_mines << " mines remaining." << std::endl;
	return false;
}

MineSolver::Tribool MineSolver::IntersectiveReason( const std::list<int> &un, const std::vector<unit> &fguide )
{
	// have an add queue and a set of added groups and add one at a time pushing new
	std::list<group> intersections;
	std::list<group> intersecQueue;

	// get initial set of groups by checking un ( revealed units that have unrevealed neighbors )
	for( auto &i : un )
	{
		group g;
		g.max_mines = fguide[i].val;
		for( auto &j : getNeighbors(i) )
		{
			if( !fguide[j].known ) g.elements.push_back( j );
			g.max_mines -= fguide[j].known && fguide[j].val == -1;
		}
		g.min_mines = g.max_mines;
		intersecQueue.push_back( g );
	}

	// work through intersecQueue
	while( !intersecQueue.empty() )
	{
		std::list<group> discoveredInters;
		auto a = intersecQueue.front();
		bool repeated = false;

		// check against all existing groups, adding new intersections to queue
		for( auto &b : intersections )
		{
			/*std::cout << "Check: ";
			a.print();
			std::cout << " , ";
			b.print();
			std::cout << std::endl; */
			group intersection;
			intersection.elements = intersect( a.elements, b.elements );
			if( intersection.elements.empty() ) continue;
			if( intersection.elements.size() == a.elements.size() && a.elements.size() == b.elements.size() )
			{
				discoveredInters.clear();
				intersecQueue.pop_front();
				repeated = true;
				break;
			}

			/*std::cout << "Found Intersect: ";
			a.print();
			std::cout << " , ";
			b.print();
			std::cout << std::endl;
			*/
			// get groupA - intersect
			group adi;
			adi.elements = difference(a.elements,intersection.elements);
			// and groupB - intersect
			group bdi;
			bdi.elements = difference(b.elements,intersection.elements);

			// find minimum/maximum number of mines for intersect from both
			int mina = a.min_mines - adi.elements.size();
			int minb = b.min_mines - bdi.elements.size();
			intersection.min_mines = ( mina < minb ) ? minb : mina;

			// probably set intersection to zero at minimum?!?!
			if( intersection.min_mines < 0 ) intersection.min_mines = 0;

			// maximums
			int maxa = a.max_mines > intersection.elements.size() ? intersection.elements.size() : a.max_mines;
			int maxb = b.max_mines > intersection.elements.size() ? intersection.elements.size() : b.max_mines;
			intersection.max_mines = ( maxa > maxb ) ? maxb : maxa;

			// other subgroup values
			adi.min_mines = a.min_mines - intersection.max_mines;
			if( adi.min_mines < 0 ) adi.min_mines = 0;
			adi.max_mines = a.max_mines - intersection.min_mines;
			bdi.min_mines = b.min_mines - intersection.max_mines;
			if( bdi.min_mines < 0 ) bdi.min_mines = 0;
			bdi.max_mines = b.max_mines - intersection.min_mines;

			if( debug )
			{
				std::cerr << "Intersecting: ";
				a.print(std::cerr);
				b.print(std::cerr);
				std::cerr << "\n\t";
				intersection.print(std::cerr);
				std::cerr << "\n\t";
				adi.print(std::cerr);
				std::cerr << "\n\t";
				bdi.print(std::cerr);
				std::cerr << std::endl;
			}

			// find guaranteed moves
			if( intersection.min_mines >= intersection.elements.size() )	// flag all
			{
				for( auto &i : intersection.elements )
				{
					flag(i);
					return Yes;
				}
			}
			if( intersection.max_mines == 0 )	// reveal all
			{
				for( auto &i : intersection.elements )
				{
					bool coll_r = true;
					coll_r = reveal(i) && coll_r;
					return coll_r ? Yes : No;
				}
			}
			if( adi.min_mines >= adi.elements.size() )	// flag all
			{
				for( auto &i : adi.elements )
				{
					flag(i);
					return Yes;
				}
			}
			if( adi.max_mines == 0 )	// reveal all
			{
				for( auto &i : adi.elements )
				{
					bool coll_r = true;
					coll_r = reveal(i) && coll_r;
					return coll_r ? Yes : No;
				}
			}
			if( bdi.min_mines >= bdi.elements.size() )	// flag all
			{
				for( auto &i : bdi.elements )
				{
					flag(i);
					return Yes;
				}
			}
			if( bdi.max_mines == 0 )	// reveal all
			{
				for( auto &i : bdi.elements )
				{
					bool coll_r = true;
					coll_r = reveal(i) && coll_r;
					return coll_r ? Yes : No;
				}
			}

			// stage subgroups
			discoveredInters.push_back( intersection );
			if( !adi.elements.empty() ) { discoveredInters.push_back(adi); }
			if( !bdi.elements.empty() ) { discoveredInters.push_back(bdi); }
		}

		if( !repeated )
		{
			// add this group to intersections
			intersections.push_back(intersecQueue.front());
			// PRINT DEBUG
			/*
			for( auto &k : intersecQueue.front().elements )
			{
				std::cout << k << " ";
			}
			std::cout << std::endl;
			*/
			// remove it from the queue
			intersecQueue.pop_front();
			// add new ones to queue
			intersecQueue.insert( intersecQueue.end(), discoveredInters.begin(), discoveredInters.end() );
		} 
		else
		{
			repeated = false;
		}

	}
	return None;
}

MineSolver::Tribool MineSolver::Guess()
{
	// get all unknownTouched units, in an unorderedSet
	std::map<int,int> guesses;
	for( auto &k : todo )
	{
		for( auto &j : getNeighbors(k) )
		{
			if( !guide[j].known ) { guesses[j] = 0; }
		}
	}

	// Make the guesses
	int perms = findPermutations( guesses, guesses.begin(), guide );

	// Log
	/*
	std::cout << "Total Possibilities: " << perms << ". Guess List { ";
	for( auto& k : guesses )
	{
		std::cout << k.first << ":" << k.second << " ";
	}
	std::cout << std::endl;
	*/

	// Check for guaranteed moves & track min
	int min = guesses.begin()->second;
	int min_i = guesses.begin()->first;
	int sumMines = 0;
	Tribool guaranteed = None;
	for( auto &k : guesses )
	{
		if( k.second == perms )
		{
			if( guaranteed == None ) { guaranteed = Yes; }
			flag(k.first);
		}
		else if ( k.second == 0 )
		{
			guaranteed = reveal( k.first ) ? Yes : No;
		}
		if( k.second < min )
		{
			min = k.second;
			min_i = k.first;
		}
		sumMines += k.second;
	}
	if( guaranteed != None ) { return guaranteed; }

	// Guess min or an undiscovered
	float bestTouched = min/(float)perms;
	float avgMines = sumMines/(float)perms;
	int untouched = field->gWidth()*field->gHeight() - num_revealed - guesses.size() - ( field->gBombs() - remaining_mines );
	float bestUntouched = untouched > 0 ? ( remaining_mines - avgMines ) / untouched : 2;

	if( bestTouched <= bestUntouched )
	{
		std::cout << "Guessing " << min_i << " "<< min_i/field->gWidth() << "," << min_i%field->gWidth() << " with p(mine) = " << bestTouched << std::endl;
		return reveal( min_i ) ? Yes : No;
	}
	else		// unveil an untouched
	{
		for( int i = 0; i < guide.size(); ++i )
		{
			if( !guide[i].known && guesses.find(i) == guesses.end() )
			{
				std::cout << "Guessing " << i << " " << i/field->gWidth() << "," << i%field->gWidth() << " with p(mine) = " << bestUntouched << std::endl;
				return reveal( i ) ? Yes : No;
			}
		}
	}

	return None;

}

int MineSolver::findPermutations( std::map<int,int> &g, std::map<int,int>::iterator it, std::vector<unit> fguide )
{
	// base case:
	if( it == g.end() ) return 1;

	// see if there is a guaranteed move??? -- dont think so

	int rVal = 0;
	// take first element check legality of flagged and non-flagged variant
	// if legal pass on recursively
	int move = it->first;
	++it;
	/*
	std::cout << "FindPerm on: " << move << " { ";
	for( auto temi = it; temi != g.end(); ++temi ) { std::cout << it->first << " "; }
	std::cout << std::endl;
	*/

	// Flag move & recurse
	fguide[move].known = true;
	fguide[move].val = -1;
	if( isLegal( fguide ) )
	{
		//std::cout << "\tFlagLegal" << std::endl;
		rVal = findPermutations( g, it, fguide );
		g[move] += rVal;
	}
	// Mark non-mine and recurse
	fguide[move].val = 10;
	if( isLegal( fguide ) )
	{
		//std::cout << "\tEmptyLegal" << std::endl;
		rVal += findPermutations( g, it, fguide );
	}

	return rVal;

}

bool MineSolver::isLegal( const std::vector<unit> &fguide )
{
	// check all squares are properly counted ( higher than legal counts are not mines but unknown )
	int mineCount = 0;
	int unknownCount = 0;
	for( auto i = 0; i < fguide.size(); ++i )
	{
		if( fguide[i].known )
		{
			if( fguide[i].val == -1 ) 
			{
				++mineCount;
			}
			else
			{
				// check number of mines matches is possible
				if( fguide[i].val >= 0 && fguide[i].val < 9 )
				{
					int mineTracker = fguide[i].val;
					int unknowns = 0;
					for( auto &k : getNeighbors(i) )
					{
						if( fguide[k].known && fguide[k].val == -1  ) { --mineTracker; }
						if( !fguide[k].known ) { ++unknowns; }
					}
					if( mineTracker < 0 || mineTracker - unknowns > 0 ) { return false; }
				}
			}
		} 
		else
		{
			++unknownCount;
		}
	}
	// Check Overall mine count still possible
	if( mineCount > field->gBombs() || mineCount + unknownCount < field->gBombs() ) { return false; }

	// passed checks
	return true;
}

MineSolver::Tribool MineSolver::NeighborReason( const int &i, const int &nmines, const std::list<int> &un)
{
	// build list of intersections and their mine numbers
	/* for each unflipped, find all flipped adjacent and see if they offer any help */
	std::unordered_set<int> interloc;
	for( auto &k : un )
	{
		for( auto &j : getNeighbors(k) )
		{
			if( guide[j].known && guide[j].val != -1 && j != i )
				interloc.insert(j);
		}
	}

	// have all possible interlocutors
	std::list<group> intersections;
	for( auto &k : interloc )
	{
		/* get neighbors, count mines, find intersection, see if there are mines there
		/ if so make a group, add it to a list of groups of intersections */
		int loc_mines = 0;
		std::list<int> tempunflip;
		for( auto &j : getNeighbors(k) )
		{
			if( guide[j].known && guide[j].val == -1 ) ++loc_mines;
			if( !guide[j].known ) tempunflip.push_back(j);
		}

		group intersection;
		intersection.elements = intersect( tempunflip, un );

		// find mines in intersection
		intersection.max_mines = (int)guide[k].val - loc_mines;
		if( intersection.max_mines > intersection.elements.size() ) intersection.max_mines = intersection.elements.size();
		intersection.min_mines = intersection.max_mines - ( tempunflip.size() - intersection.elements.size() );
		if( intersection.min_mines > 0 ) intersections.push_back( intersection );
	}

	int y = i/field->gWidth();
	int x = i % field->gWidth();
	//std::cout << "Second Order around " << y << "," << x << std::endl;
	if( i == -30*4 + 14)
	{
		std::cout << intersections.size() << " intersecting groups." << std::endl;
		for( auto &k : intersections )
		{
			std::cout << "[" << k.min_mines << "," << k.max_mines << "} mines: ";
			for( auto &a : k.elements ) std::cout << "("<<a/field->gWidth()<<","<<a%field->gWidth() << ") ";
			std::cout << std::endl;
		}
		std::cout << guide[i].val - nmines << " mines to place in locations: ";
		for( auto &a : un ) std::cout << a << " ";
		std::cout << std::endl;
	}


	// likely a recursive function here to simplify the search through all subsets of intersections to see if there is a move to make
	return checkIntersections( un, guide[i].val - nmines, guide[i].val - nmines, intersections.begin(), intersections.end() );
}

// Recursive Function to check combinations of intersections to find a move
MineSolver::Tribool MineSolver::checkIntersections( const std::list<int>&unflip, const int& nminemax, const int& nminemin,
							std::list<group>::iterator start, const std::list<group>::iterator &end )
{
	// base case
	if( start == end ) return None;
	// find next possible intersection that could exist
	for( ;start != end; ++start )
	{
		auto inter = intersect( unflip, start->elements );
		if( inter.size() < start->elements.size() ) continue;		// must be a proper subset
		if( !inter.empty() )
		{
			// difference out the new intersect
			std::list<int> newun;
			auto lit1 = unflip.begin();
			auto lit2 = start->elements.begin();
			while( lit1 != unflip.end() )
			{
				if( *lit1 != *lit2 ) 
				{
					newun.push_back( *lit1 );
				}
				else
				{
					++lit2;
				}
				++lit1;
			}
			// Ensure that newun isn't empty!
			if( newun.empty() ) continue;
			// subtract the mines from that intersect
			// check if no mines left or exactly the same number as open spots left
			int newMax = nminemax - start->min_mines;
			int newMin = nminemin - start->max_mines;
			if( newMax == 0 )		// no mines, reveal all unflipped
			{
				bool collectiveReturn = true;
				for( auto &i : newun )
				{
					collectiveReturn = collectiveReturn && reveal( i );
				}
				return collectiveReturn ? Yes : No;
			}
			else if( newMin == newun.size() )	// all remaining spots are mines
			{
				for( auto &i : newun )
				{
					//std::cout << "Second Order Flag: " << i << std::endl;
					guide[i].known = true;
					guide[i].val = -1;
					--remaining_mines;
				}
				return Yes;
			}
			else	// make the recursive call and if the value is not none, return
			{
				auto next = start;
				auto r = checkIntersections( newun, newMax, newMin, ++next, end );
				if( r != None ) return r;
			}
		}
	}
	return None;	// no solutions found with intersections here
}

std::list<int> MineSolver::intersect( const std::list<int>&a, const std::list<int>&b )
{
	std::list<int> r;
	auto lit1 = a.begin();
	auto lit2 = b.begin();
	while( lit1 != a.end() && lit2 != b.end() )
	{
		if( *lit1 > *lit2 )
		{
			++lit2;
		}
		else if( *lit1 < *lit2 )
		{
			++lit1;
		}
		else
		{
			r.push_back( *lit1 );
			++lit1;
			++lit2;
		}
	}
	return r;
}

std::list<int> MineSolver::difference( const std::list<int>&a, const std::list<int>&b )
{
	std::list<int> r;
	auto lit1 = a.begin();
	auto lit2 = b.begin();
	while( lit1 != a.end() && lit2 != b.end() )
	{
		if( *lit1 > *lit2 )
		{
			++lit2;
		}
		else if( *lit1 < *lit2 )
		{
			r.push_back( *lit1 );
			++lit1;
		}
		else
		{
			++lit1;
			++lit2;
		}
	}
	while( lit1 != a.end() )
	{
		r.push_back( *lit1 );
		++lit1;
	}
	return r;
}


bool MineSolver::reveal( const int& i )
{
	// flip this unit on the minefield
	auto r = field->flip( i / field->gWidth(), i % field->gWidth() );
	if( r == -1 ) // bomb
	{
		std::cout << "Stepped on a mine at unit " << i/field->gWidth() << ","
		<< i%field->gWidth() << "!!" << std::endl;
		guide[i].known = true;
		guide[i].val = -9;
		return false;
	}
	else
	{
		// record this value in our record
		if( debug )
		{
			std::cerr << "Revealing " << i/field->gWidth() << ","
			<< i%field->gWidth() << "!!" << std::endl;
		}

		guide[i].known = true;
		guide[i].val = r;
		++num_revealed;
		// if there are adjacent uknowns, add this to todo
		for( auto& k : getNeighbors(i) )
		{
			if( guide[k].known == false )
			{
				todo.push_back( i );
				break;
			}
		}
		return true;
	}
}

void MineSolver::flag( const int& i )
{
	if( debug )
	{
		std::cerr << "Flagging " << i/field->gWidth() << ","
		<< i%field->gWidth() << "!!" << std::endl;
	}
	
	guide[i].known = true;
	guide[i].val = -1;
	--remaining_mines;
}

// needs to be returned in order
std::list<int> MineSolver::getNeighbors( const int& i )
{
	std::list<int> r;
	bool edgeLeft = i % field->gWidth() == 0;
	bool edgeRight = i % field->gWidth() == field->gWidth() - 1;
	bool edgeTop = i < field->gWidth();
	bool edgeBot = i + field->gWidth() >= field->gWidth()*field->gHeight();

	// order matters here. Want sorted lists
	if( ! ( edgeLeft || edgeTop ) ) r.push_back( i - 1 - field->gWidth() );
	if( ! edgeTop ) r.push_back( i - field->gWidth() );
	if( ! ( edgeRight || edgeTop ) ) r.push_back( i + 1 - field->gWidth() );
	if( ! edgeLeft ) r.push_back( i - 1 );
	if( ! edgeRight ) r.push_back( i + 1 );
	if( ! ( edgeLeft || edgeBot ) ) r.push_back( i - 1 + field->gWidth() );
	if( ! edgeBot ) r.push_back( i + field->gWidth() );
	if( ! ( edgeRight || edgeBot ) ) r.push_back( i + 1 + field->gWidth() );

	return r;
}

void MineSolver::printGuide()
{
	for( int i = 0; i < field->gHeight(); ++i )
	{
		for( int j = 0; j < field->gWidth(); ++j )
		{
			int index = i*field->gWidth() + j;
			if( guide[index].known) { std::cout << std::setw(5) << (int)guide[index].val; }
			else { std::cout << std::setw(5) << "_"; }
		}
		std::cout << std::endl;
	}
}
