#include <cxxtest/TestSuite.h>

#include "common/list.h"

class ListTestSuite : public CxxTest::TestSuite
{
	public:
	void test_empty_clear( void )
	{
		Common::List<int> container;
		TS_ASSERT( container.empty() );
		container.push_back(17);
		container.push_back(33);
		TS_ASSERT( !container.empty() );
		container.clear();
		TS_ASSERT( container.empty() );
	}

	public:
	void test_size( void )
	{
		Common::List<int> container;
		TS_ASSERT( container.size() == 0 );
		container.push_back(17);
		TS_ASSERT( container.size() == 1 );
		container.push_back(33);
		TS_ASSERT( container.size() == 2 );
		container.clear();
		TS_ASSERT( container.size() == 0 );
	}

	void test_iterator_begin_end( void )
	{
		Common::List<int> container;

		// The container is initially empty ...
		TS_ASSERT( container.begin() == container.end() );

		// ... then non-empty ...
		container.push_back(33);
		TS_ASSERT( container.begin() != container.end() );

		// ... and again empty.
		container.clear();
		TS_ASSERT( container.begin() == container.end() );
	}

	void test_iterator( void )
	{
		Common::List<int> container;
		Common::List<int>::iterator iter;
		Common::List<int>::const_iterator cIter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate over the container and verify that we encounter the elements in
		// the order we expect them to be.

		iter = container.begin();
		cIter = container.begin();

		TS_ASSERT( iter == cIter );

		TS_ASSERT( *iter == 17 );
		++iter;
		++cIter;
		TS_ASSERT( iter != container.end() );
		TS_ASSERT( cIter != container.end() );
		TS_ASSERT( iter == cIter );

		TS_ASSERT( *iter == 33 );
		++iter;
		++cIter;
		TS_ASSERT( iter != container.end() );
		TS_ASSERT( cIter != container.end() );
		TS_ASSERT( iter == cIter );

		// Also test the postinc
		TS_ASSERT( *iter == -11 );
		iter++;
		cIter++;
		TS_ASSERT( iter == container.end() );
		TS_ASSERT( cIter == container.end() );
		TS_ASSERT( iter == cIter );

		cIter = iter;
		TS_ASSERT( iter == cIter );
	}

	void test_insert( void )
	{
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		// Iterate to after the second element
		iter = container.begin();
		++iter;
		++iter;

		// Now insert some values here
		container.insert(iter, 42);
		container.insert(iter, 43);

		iter = container.begin();

		TS_ASSERT( *iter == 17 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 33 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 42 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 43 );
		++iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == -11 );
		++iter;
		TS_ASSERT( iter == container.end() );
	}

	void test_reverse( void )
	{
		Common::List<int> container;
		Common::List<int>::iterator iter;

		// Fill the container with some random data
		container.push_back(17);
		container.push_back(33);
		container.push_back(-11);

		iter = container.reverse_begin();
		TS_ASSERT( iter != container.end() );


		TS_ASSERT( *iter == -11 );
		--iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 33 );
		--iter;
		TS_ASSERT( iter != container.end() );

		TS_ASSERT( *iter == 17 );
		--iter;
		TS_ASSERT( iter == container.end() );

		iter = container.reverse_begin();

		iter = container.reverse_erase(iter);
		TS_ASSERT( iter != container.end() );
		TS_ASSERT( *iter == 33 );

		iter = container.reverse_erase(iter);
		TS_ASSERT( iter != container.end() );
		TS_ASSERT( *iter == 17 );

		iter = container.reverse_erase(iter);
		TS_ASSERT( iter == container.end() );

		TS_ASSERT( container.begin() == container.end() );
		TS_ASSERT( container.reverse_begin() == container.end() );
	}
};
