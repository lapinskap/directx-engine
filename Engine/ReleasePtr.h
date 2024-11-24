#pragma once

// This is a smart pointer that will automatically call the Release method on the object it is holding when it goes out of scope.
template <typename T>
struct ReleasePtr
{
	ReleasePtr() = default;
	ReleasePtr(const ReleasePtr&) = delete;
	ReleasePtr& operator=(const ReleasePtr&) = delete;
	ReleasePtr(ReleasePtr&& other) noexcept
	{
		_ptr = other._ptr;
		other._ptr = nullptr;
	}
	ReleasePtr& operator=(ReleasePtr&& other) noexcept
	{
		if (this != &other)
		{
			if (_ptr)
				_ptr->Release();

			_ptr = other._ptr;
			other._ptr = nullptr;
		}
		return *this;
	}

	~ReleasePtr()
	{
		if (_ptr)
			_ptr->Release();
	}

	T* operator->()
	{
		return _ptr;
	}

	T* get()
	{
		return _ptr;
	}

	const T* get() const
	{
		return _ptr;
	}

	T** operator&()
	{
		return &_ptr;
	}

	explicit operator bool() const
	{
		return _ptr != nullptr;
	}

private:

	T* _ptr = nullptr;
};