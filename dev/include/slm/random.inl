inline float randomFloat( random& seed )
{
	return seed.randomFloat();
}

inline float randomFloatSigned( random& seed )
{
	return seed.randomFloat()*2.f - 1.f;
}

inline float randomFloatOpen( random& seed )
{
	return seed.randomFloatOpen();
}

inline int randomInt( random& seed )
{
	return seed.randomInt();
}
