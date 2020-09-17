#pragma once
#include "QString"

class Object
{
public:
	Object();
	virtual ~Object();

	virtual QString GetClassName() const = 0;
};

