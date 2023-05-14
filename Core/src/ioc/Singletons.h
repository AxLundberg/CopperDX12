#pragma once
#include <any>
#include <format>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <functional>
#include <unordered_map>

#include "Container.h"

namespace CPR::IOC
{
	class Singletons
	{
	public:
		// types
		template<class T>
		using Generator = std::function<std::shared_ptr<T>()>;
		// functions
		template<class T>
		void Register(Generator<T> gen)
		{
			mServiceMap[typeid(T)] = gen;
		}
		template<class T>
		void RegisterPassthru()
		{
			Register<T>([] { return IOC::Get().Resolve<T>(); });
		}
		template<class T>
		std::shared_ptr<T> Resolve()
		{
			// TODO: pull this out of template/header
			if (const auto i = mServiceMap.find(typeid(T)); i != mServiceMap.end())
			{
				auto& entry = i->second;
				try {
					// first check if we have an existing instance, return if so
					if (auto ppInstance = std::any_cast<std::shared_ptr<T>>(&entry)) {
						return *ppInstance;
					}
					// if not, generate instance, store, and return
					auto pInstance = std::any_cast<Generator<T>>(entry)();
					entry = pInstance;
					return pInstance;
				}
				catch (const std::bad_any_cast&)
				{
					throw std::logic_error{ std::format(
						L"Could not resolve Singleton mapped type\nfrom: [{}]\n  to: [{}]\n",
						entry.type().name(), typeid(Generator<T>).name())
					};
				}
			}
			else
			{
				throw std::runtime_error{ std::format(
					"Failed to find entry for type [{}] in singleton container",
					typeid(T).name())
				};
			}
		}
	private:
		// data
		std::unordered_map<std::type_index, std::any> mServiceMap;
	};

	Singletons& Sing();
}
