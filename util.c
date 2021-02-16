#include "util.h"

int compare_function(const void* a, const void*b)
{
	return (*(int*)a - *(int*)b);
}

int find_threshold(volatile char* addr)
{

	int times[ROUNDS] = {0};
	uint64_t start;
	int aux = 0;
	int threshold = 0;
	volatile char* test = addr;

	for (int i = 0; i < ROUNDS; i++)
	{
		_mm_clflush((char*)addr);
		_mm_mfence();
		start = __rdtscp(&aux);
		_mm_clflush((char*)addr);
		_mm_mfence();
		times[i] = __rdtscp(&aux) - start;
		sched_yield();
	}
	
	qsort(times, ROUNDS, sizeof(int), compare_function);
	int cut_off = (ROUNDS>>1) + (ROUNDS>>2);
	threshold = times[cut_off];
	return threshold;
}

int time_flush(volatile char* addr)
{
	int time = 0;
	int aux = 0;
	uint64_t start;

	start = __rdtscp(&aux);
	_mm_clflush((char*)addr);
	_mm_mfence();
	time = __rdtscp(&aux) - start;
	
	return time;
}


inline void send_bit(volatile char* channel, bool bit)
{
	if(!bit)
	{
		*(channel);
		_mm_mfence();
	}

	else
	{
		_mm_clflush((char*) channel);
		_mm_mfence();
	}
}

inline bool receive_bit(volatile char* channel, int threshold)
{
	int time = 0;
	time = time_flush(channel);

	if (time > threshold)
		return false;

	return true;
	
}

inline bool* char_to_binary(char character, bool* binary_value)
{
	binary_value[7] = (character & 0b10000000) ? true: false;
	binary_value[6] = (character & 0b01000000) ? true: false;
	binary_value[5] = (character & 0b00100000) ? true: false;
	binary_value[4] = (character & 0b00010000) ? true: false;
	binary_value[3] = (character & 0b00001000) ? true: false;
	binary_value[2] = (character & 0b00000100) ? true: false;
	binary_value[1] = (character & 0b00000010) ? true: false;
	binary_value[0] = (character & 0b00000001) ? true: false;

	return binary_value;
}

inline char binary_to_char(bool* binary_value)
{
	int result = 0;

	for (int i = 0; i < BITS; i++)
	{
		if(binary_value[i])
			result |= (1<<i);
	}
	return (char)result;
}

void demo(char* string, volatile char* channel, int threshold)
{
	bool* send_binary_value; 
	send_binary_value = (bool*) malloc(sizeof(bool) * BITS);

	bool* coded_binary_value;
	coded_binary_value = (bool*) malloc(sizeof(bool) * CODE_SIZE);

	bool* recv_binary_value; 
	recv_binary_value = (bool*) malloc(sizeof(bool) * CODE_SIZE);

	bool* decoded_binary_value;
	decoded_binary_value = (bool*) malloc(sizeof(bool) * BITS);

	bool received_bit = true;
	int index = 0;
	char received_char;
	int count;

	int time;

	while(*(string + index) != '\0')
	{
		send_binary_value = char_to_binary(*(string + index), send_binary_value);

		coded_binary_value = encode(send_binary_value);
		for (int i = 0; i < CODE_SIZE; i++)
		{
			
			count = 0;
			received_bit = true;
			for (int j = 0; j < TRANSMIT_ROUNDS; j++)
				sched_yield();

			for (int j = 0; j < TRANSMIT_ROUNDS; ++j)
			{
				send_bit(channel, *(coded_binary_value + i));
				if(!receive_bit(channel, threshold))
					count--;
				else
					count++;
			}
			if(count <= 0)
				received_bit = false;

			*(recv_binary_value + i) = received_bit;
		}
		decoded_binary_value = decode(recv_binary_value);

		received_char = binary_to_char(decoded_binary_value);
		printf("%c", received_char);
		index++;
	}

	printf("\n");
}

void dump_bitset(bool* bitset, int size)
{
	for (int i = 0; i < size; i++)
		printf("%d", *(bitset + i));
	
	printf("\n");
}

uint64_t synchronise()
{
	int aux = 0;
	while(__rdtscp(&aux) & TIME_MASK < JITTER);
	return __rdtscp(&aux);
}

inline void send_preamble(volatile char* channel)
{
	bool* send_binary_value; 
	send_binary_value = (bool*) malloc(sizeof(bool) * BITS);

	bool* coded_binary_value;
	coded_binary_value = (bool*) malloc(sizeof(bool) * CODE_SIZE);

	uint64_t start;
	int aux = 0;


	send_binary_value = char_to_binary((char)PREAMBLE, send_binary_value);
	coded_binary_value = encode(send_binary_value);

	for (int i = 0; i < TRANSMIT_ROUNDS; i++)
		sched_yield();

	for (int i = CODE_SIZE - 1; i >= 0; i--)
	{
		start = synchronise();
		while(__rdtscp(&aux) - start < INTERVAL)
		{
			send_bit(channel, *(coded_binary_value + i));
		}
	}


	return;
}

void demo_child_parent(char* string, volatile char* channel, int threshold)
{
	pid_t child_pid;
	child_pid = fork();

	if(child_pid == 0)
	{
		int sending = 1;
		int index;
		int aux = 0;

		bool* send_binary_value; 
		send_binary_value = (bool*) malloc(sizeof(bool) * BITS);

		bool* coded_binary_value;
		coded_binary_value = (bool*) malloc(sizeof(bool) * CODE_SIZE);

		uint64_t time;

		while(true)
			send_preamble(channel);

		index = 0;
		while(*(string + index) != '\0')
		{
			send_binary_value = char_to_binary(*(string + index), send_binary_value);
			coded_binary_value = encode(send_binary_value);
			for (int i = 0; i < CODE_SIZE; i++)
			{
				for (int i = 0; i < TRANSMIT_ROUNDS; i++)
					sched_yield();
				time = synchronise();
				while((__rdtscp(&aux) - time) < INTERVAL)
					send_bit(channel, *(coded_binary_value + i));
			}
			index++;
		}


		exit(0);
	}

	else
	{
		uint64_t time, start;
		int aux = 0;
		int count = 0;
		int index = 0;
		int strikes = 0;
		bool receiving = true;

		bool bitReceived;
		bool received_bit;
		uint32_t bitSequence = 0;
		uint32_t sequenceMask = ((uint32_t) 1<<15) - 1;
		uint32_t expSequence = 0b10100101011001;

		bool* recv_binary_value; 
		recv_binary_value = (bool*) malloc(sizeof(bool) * CODE_SIZE);

		bool* decoded_binary_value;
		decoded_binary_value = (bool*) malloc(sizeof(bool) * BITS);

		fflush(stdout);
		
		while (receiving) 
		{
			count = 0;

			for (int i = 0; i < TRANSMIT_ROUNDS; i++)
				sched_yield();
			
			start = synchronise();
			while((__rdtscp(&aux) - start) < INTERVAL)
			{
				if(receive_bit(channel, threshold))
					count++;
				else
					count--;
				
			}
			bitReceived = (count >= 0) ? true: false;

			bitSequence = ((uint32_t) bitSequence<<1) | bitReceived;

			if ((bitSequence & sequenceMask) == expSequence) 
			{
				printf("Preamble received\n");
				char received_char = (char) 27;

				while(received_char != '\0')
				{
					index = 0;
					strikes = 0;
					while(index != 14)
					{
						received_bit = true;
						count = 0;
						for (int i = 0; i < TRANSMIT_ROUNDS; i++)
							sched_yield();

						start = synchronise();

						while((__rdtscp(&aux) - start) < INTERVAL)
						{
							if(receive_bit(channel, threshold))
								count++;
							else
								count--;
						}
						if(count > 0)
						{
							strikes++;
							received_bit = false;
						}

						else
							strikes = 0;

						*(recv_binary_value + index) = received_bit;						
						if(index == 13 || strikes >= 14)
							break;

						index++;
					}
					decoded_binary_value = decode(recv_binary_value);
					received_char = binary_to_char(decoded_binary_value);

					if(strikes != 14)
					{
						printf("%c", received_char);
					}
					if(received_char == '\0')
						receiving = false;
					
				}
				
			}
		}
		printf("\n");
		wait(NULL);

	}
	return;
}

