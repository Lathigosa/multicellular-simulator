#ifndef SAFE_POINTER_H_INCLUDED
#define SAFE_POINTER_H_INCLUDED

#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename type>
class pointer
{
public:
    pointer();
    pointer(const type & object);
    pointer(type* object);
	~pointer();

	const type & get_data() const
	{
        return *data;
	}
	void set_data(const type & object)
	{
        *data = object;
	};

	/// Forbid the use of a copy constructor ( @todo implement the copy constructor)
	//pointer(const pointer&) = delete;
	//void operator=(pointer const &) = delete;

private:
    type * data = nullptr;


};

template <typename type>
pointer<type>::pointer()
{
    data = new type();
}

template <typename type>
pointer<type>::pointer(type* object)
{
	data = object;
}

template <typename type>
pointer<type>::pointer(const type & object)
{
	data = new type(object);
}

template <typename type>
pointer<type>::~pointer()
{
    if(data != nullptr)
        delete data;
}

#endif // SAFE_POINTER_H_INCLUDED
