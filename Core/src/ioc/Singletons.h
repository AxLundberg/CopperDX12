#pragma once
#include <any>
#include <format>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <functional>
#include <unordered_map>

#include "Container.h"
#include "Exception.h"
#include <Core/src/utl/Assert.h>
#include <Core/src/utl/String.h>

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
		void RegisterPassThrough()
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
					cpr_check_fail.Msg(std::format(
						L"Failed to resolve Singleton mapped type\nFrom: [{}]\n to: [{}]",
						UTL::ToWide(entry.type().name()), UTL::ToWide(typeid(Generator<T>).name())
					)).Ex();
				}

			}
			else
			{
				throw ServiceNotFound{ std::format(
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
