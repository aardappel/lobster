#include <slm/random.h>
#include <time.h>

SLMATH_BEGIN()

random::random()
{
	time_t t;
	time( &t );

	m_seed = (int)(t+clock());
	m_mt.init_genrand( m_seed );
}

random::random( int seed ) :
	m_seed( seed )
{
	m_mt.init_genrand( m_seed );
}

float random::randomFloat()
{
	return (float)m_mt.genrand_real1();
}

float random::randomFloatOpen()
{
	return (float)m_mt.genrand_real2();
}

int random::randomInt()
{
	return m_mt.genrand_int32();
}

int random::seed() const
{
	return m_seed;
}

SLMATH_END()
