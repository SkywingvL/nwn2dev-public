/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	gui_test_structregress.nss

Abstract:

	This module houses test code for general structure parsing regression
	testing.

--*/

int Struct1;
int Struct3;

struct Struct1
{
	int Value;
};

struct Struct2
{
	int Value;
};

/*
// OK in the BioWare compiler but not in ours.
struct Struct3
{
	struct Struct4 Value;
	int Value2;
};

struct Struct4
{
	int Value;
};*/

// void Struct6() {} // Would cause crash before

//
// The following line causes an error if uncommented in the BioWare compiler,
// but not ours, because the BioWare compiler registers function symbols as at
// least unavailable for use as future variable names during phase 1.
// 

//void Value() {}

struct Struct6
{
	int Value;
};

struct Struct5
{
	struct Struct6 Value;
};


struct Struct2 Function1()
{
	struct Struct2 Variable;
	
	Variable.Value = 0;
	
	return Variable;
}

struct Struct2 Struct2 = Function1();

struct Identifier { int y; };

//Identifier = 0;

#if 1
//struct spell { int x; };
struct spell { int x; };

struct spell func()
{
struct spell sp;

sp.x = 0;

return sp;
}

struct spell spell = func();

void main()
{
PrintInteger(spell.x);
}

#endif

