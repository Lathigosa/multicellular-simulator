#ifndef QUEUE_2D_H
#define QUEUE_2D_H

#include "utilities/queue.h"


//----------------------------------------------------------------//

template <typename type>
class block_queue
{
public:
	block_queue(unsigned int size_of_block);
	~block_queue();

	void clear_data();

	type get_item(unsigned int index) const;
	void set_item(unsigned int index, const type &item);

	void add_item(const type &item);
	void insert_item(unsigned int at, const type & item);
	void remove_item(unsigned int index);

	unsigned int get_length() const
	{
		return used_length;
	}

private:
	type * buffer_data = nullptr;

	unsigned int used_length = 0;
	unsigned int true_length = 0;

	const unsigned int block_size = 0;
};




template <typename type>
block_queue<type>::block_queue(unsigned int size_of_block)
{
    block_size = size_of_block;
	buffer_data = new type[8*block_size];
	true_length = 8;
	used_length = 0;
}

template <typename type>
block_queue<type>::~block_queue()
{
	delete[] buffer_data;
}

template <typename type, unsigned int block_size>
void block_queue<type>::clear_data()
{
	for (unsigned int a=0; a<true_length*block_size; a++) {
		buffer_data[a] = type();
	}
	used_length = 0;
}

template <typename type, unsigned int block_size>
type block_queue<type>::get_item(unsigned int block, unsigned int index) const
{
	if((block*block_size + index) >= used_length)
		return type();

	return buffer_data[block*block_size + index];
}

template <typename type, unsigned int block_size>
void block_queue<type>::set_item(unsigned int block, unsigned int index, const type & item)
{
	if((block*block_size + index) >= used_length)
		return;

	buffer_data[block*block_size + index] = item;
}

template <typename type, unsigned int block_size>
void block_queue<type>::add_item(const type & item)
{
	if(used_length == true_length)
	{
		type * temp_buffer = buffer_data;	// Store the pointer to the old buffer in temp_buffer.

		buffer_data = new type[true_length * 2];

		for (unsigned int a=0; a<true_length; a++)
		{
			buffer_data[a] = temp_buffer[a];
		}

		delete[] temp_buffer;

		true_length = true_length * 2;
	}

	buffer_data[used_length] = item;

	used_length++;
}

template <typename type, unsigned int block_size>
void block_queue<type>::insert_item(unsigned int at, const type &item)
{
	if(at >= used_length)
		return;

	if(used_length == true_length)
	{
		type * temp_buffer = buffer_data;	// Store the pointer to the old buffer in temp_buffer.

		buffer_data = new type[true_length * 2];
		for (unsigned int a=0; a<true_length; a++)
		{
			buffer_data[a] = temp_buffer[a];
		}

		delete[] temp_buffer;

		true_length = true_length * 2;
	}

	for(unsigned int a=used_length; a>at; a--)
	{
		buffer_data[a] = buffer_data[a - 1];
	}

	buffer_data[at] = item;

	used_length++;
}

template <typename type, unsigned int block_size>
void block_queue<type>::remove_item(unsigned int index)
{
	if(index >= used_length)
		return;

	for(unsigned int i = index; i<used_length; i++)
	{
		buffer_data[i] = buffer_data[i + 1];
	}

	used_length--;
}


#endif // QUEUE_H
