#pragma once
#include "QString"

class Object
{
public:
	Object();
	virtual ~Object();

	virtual QString GetStaticClassName() const = 0;
};

