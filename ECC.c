#include "ECC.h"

bool* encode(bool* data)
{
	bool* code = (bool*) malloc(sizeof(bool)*CODE_SIZE);

	*(code + 0) = *(data + 0) ^ *(data + 1) ^ *(data + 3);
	*(code + 1) = *(data + 0) ^ *(data + 2) ^ *(data + 3);
	*(code + 2) = *(data + 1) ^ *(data + 2) ^ *(data + 3);

	*(code + 3) = *(data + 0);
	*(code + 4) = *(data + 1);
	*(code + 5) = *(data + 2);
	*(code + 6) = *(data + 3);

	*(code + 7) = *(data + 4) ^ *(data + 5) ^ *(data + 7);
	*(code + 8) = *(data + 4) ^ *(data + 6) ^ *(data + 7);
	*(code + 9) = *(data + 5) ^ *(data + 6) ^ *(data + 7);

	*(code + 10) = *(data + 4);
	*(code + 11) = *(data + 5);
	*(code + 12) = *(data + 6);
	*(code + 13) = *(data + 7);

	return code;

}

bool* decode(bool* code)
{
	bool c_0, c_1, c_2;
	int correction;

	c_0 = *(code + 0) ^ *(code + 3) ^ *(code + 4) ^ *(code + 6);
	c_1 = *(code + 1) ^ *(code + 3) ^ *(code + 5) ^ *(code + 6);
	c_2 = *(code + 2) ^ *(code + 4) ^ *(code + 5) ^ *(code + 6);

	correction = (c_2 << 2) | (c_1 << 1) | (c_0);

	switch(correction)
	{
		case 3:
			*(code + 3) ^= true;
			break;

		case 5:
			*(code + 4) ^= true;
			break;

		case 6:
			*(code + 5) ^= true;
			break;

		case 7:
			*(code + 6) ^= true;
			break;

		default:
			break;
	}

	c_0 = *(code + 7) ^ *(code + 10) ^ *(code + 11) ^ *(code + 13);
	c_1 = *(code + 8) ^ *(code + 10) ^ *(code + 12) ^ *(code + 13);
	c_2 = *(code + 9) ^ *(code + 11) ^ *(code + 12) ^ *(code + 13);

	correction = (c_2 << 2) | (c_1 << 1) | (c_0);

	switch(correction)
	{
		case 3:
			*(code + 10) ^= true;
			break;

		case 5:
			*(code + 11) ^= true;
			break;

		case 6:
			*(code + 12) ^= true;
			break;

		case 7:
			*(code + 13) ^= true;
			break;

		default:
			break;
	}

	bool* result = (bool*) malloc(8*sizeof(bool));
	*(result + 0) = *(code + 3);
	*(result + 1) = *(code + 4);
	*(result + 2) = *(code + 5);
	*(result + 3) = *(code + 6);

	*(result + 4) = *(code + 10);
	*(result + 5) = *(code + 11);
	*(result + 6) = *(code + 12);
	*(result + 7) = *(code + 13);

	return result;
}