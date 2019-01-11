//
// table.cpp
//

#include "sol.hpp"
#include "array.hpp"
#include "kernel.hpp"
#include "object.hpp"
#include "pmachine.hpp"

enum {
	TableInit,
	TableAdd,
	TableDelete,
	TableLookup,
	TableLookupObj
};

struct TableNode {
	TableNode() : value(-1),object(0)
	{
	}

	uint  	value;
	SCIWord  object;
};
	
struct TablePtr {
	~TablePtr() 
	{
		delete table;
	};

	TableNode* table;
	uint  	  size;
} tables[10];


void
KTable(argList)
{
	int tIndex = arg(2);

	switch (arg(1)) {
		case TableInit:
			if (tables[tIndex].table == NULL) {
				tables[tIndex].size  = arg(3);
				tables[tIndex].table = New TableNode[arg(3)];
			}
			break;

		case TableAdd:
			if (tables[tIndex].table != NULL) {
				int val, obj;

				for (int i = 0; i < tables[tIndex].size; ++i) {
					if (tables[tIndex].table[i].object == 0) {
						val = arg(3);
						obj = arg(4);
						tables[tIndex].table[i].value  = arg(3);
						tables[tIndex].table[i].object = arg(4);
						break;
					}
				}
			}
			break;

		case TableDelete:
			if (tables[tIndex].table != NULL) {
				int searchVal = arg(3);
				for (int i = 0; i < tables[tIndex].size; ++i) {
					if (tables[tIndex].table[i].value == searchVal) {
						tables[tIndex].table[i].value = -1;
						tables[tIndex].table[i].object = 0;
						break;
					}
				}
			}
			break;

		case TableLookup:
			pm.acc = 0;
			if (tables[tIndex].table != NULL) {
				int searchVal = arg(3);
				for (int i = 0; i < tables[tIndex].size; ++i) {
					if (tables[tIndex].table[i].value == searchVal) {
						pm.acc = tables[tIndex].table[i].object;
						break;
					}
				}
			}
			break;

		case TableLookupObj:
			pm.acc = 0;
			if (tables[tIndex].table != NULL) {
				int searchVal = arg(3);
				for (int i = 0; i < tables[tIndex].size; ++i) {
					if (tables[tIndex].table[i].object == searchVal) {
						pm.acc = tables[tIndex].table[i].value;
						break;
					}
				}
			}
			break;

	}
}

