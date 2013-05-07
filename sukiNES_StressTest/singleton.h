#pragma once

namespace StressTest
{
	template<class T>
	class Singleton
	{
	public:
		static T& self()
		{
			if (!_instance)
			{
				_instance = new T;
			}

			return *_instance;
		}

		static T* _instance;

	protected:
		Singleton() {}
		~Singleton() { }

	private:
		Singleton(const Singleton& copy);
		Singleton& operator=(const Singleton& other);
	};
}
