#ifndef BUFFER_H
#define BUFFER_H

template <typename type>
class buffer
{
public:
	buffer(unsigned int size);
	~buffer();

	void clear_data();

	type get_item(unsigned int index) const;
	void set_item(unsigned int index, const type &item);

	unsigned int get_length() const
	{
		return length;
	}

private:
	type * buffer_data = nullptr;

	const unsigned int length = 0;
};




template <typename type>
buffer<type>::buffer(unsigned int size) : length(size)
{
	buffer_data = new type[size];
}

template <typename type>
buffer<type>::~buffer()
{
	delete[] buffer_data;
}

template <typename type>
void buffer<type>::clear_data()
{
	for (unsigned int a=0; a<length; a++) {
		buffer_data[a] = type();
	}
}

template <typename type>
type buffer<type>::get_item(unsigned int index) const
{
	if(index >= length)
		return type();

	return buffer_data[index];
}

template <typename type>
void buffer<type>::set_item(unsigned int index, const type & item)
{
	if(index >= length)
		return;

	buffer_data[index] = item;
}

#endif // BUFFER_H
